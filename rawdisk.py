#!/usr/bin/env python3
"""
rawdisk.py — disk utility for raw-kernel FAT12 images
(c) 2026, Redstone2888

Usage:
  python3 rawdisk.py <disk.img> ls
  python3 rawdisk.py <disk.img> cat  <NAME.EXT>
  python3 rawdisk.py <disk.img> put  <host_file> [NAME.EXT]
  python3 rawdisk.py <disk.img> get  <NAME.EXT> [host_file]
  python3 rawdisk.py <disk.img> rm   <NAME.EXT>
  python3 rawdisk.py <disk.img> info
"""

import sys
import os
import struct

SECTOR_SIZE  = 512
FAT_START    = 1          # sector where FAT begins
FAT_SECTORS  = 9          # sectors per FAT copy
FAT_COPIES   = 2
ROOT_START   = FAT_START + FAT_SECTORS * FAT_COPIES   # = 19
ROOT_SECTORS = 14
ROOT_ENTRIES = 224
DATA_START   = ROOT_START + ROOT_SECTORS               # = 33

# ── low-level helpers ───────────────────────────────────────────────────────

def read_sector(img, n):
    img.seek(n * SECTOR_SIZE)
    return bytearray(img.read(SECTOR_SIZE))

def write_sector(img, n, data):
    img.seek(n * SECTOR_SIZE)
    img.write(bytes(data))

def read_fat(img):
    fat = bytearray()
    for i in range(FAT_SECTORS):
        fat += read_sector(img, FAT_START + i)
    return fat

def write_fat(img, fat):
    for copy in range(FAT_COPIES):
        for i in range(FAT_SECTORS):
            write_sector(img, FAT_START + copy * FAT_SECTORS + i,
                         fat[i*SECTOR_SIZE:(i+1)*SECTOR_SIZE])

def fat_get(fat, cluster):
    idx = cluster + cluster // 2
    if cluster & 1:
        return ((fat[idx] >> 4) | (fat[idx+1] << 4)) & 0xFFF
    else:
        return (fat[idx] | ((fat[idx+1] & 0x0F) << 8))

def fat_set(fat, cluster, value):
    idx = cluster + cluster // 2
    if cluster & 1:
        fat[idx]   = (fat[idx] & 0x0F) | ((value & 0x0F) << 4)
        fat[idx+1] = (value >> 4) & 0xFF
    else:
        fat[idx]   = value & 0xFF
        fat[idx+1] = (fat[idx+1] & 0xF0) | ((value >> 8) & 0x0F)

def fat_alloc(fat):
    for c in range(2, 2848):
        if fat_get(fat, c) == 0:
            return c
    return None

def fat_free_chain(fat, start):
    cur = start
    while 0x002 <= cur < 0xFF8:
        nxt = fat_get(fat, cur)
        fat_set(fat, cur, 0)
        cur = nxt

# ── directory helpers ────────────────────────────────────────────────────────

DIR_ENTRY_SIZE = 32

def read_root(img):
    raw = bytearray()
    for i in range(ROOT_SECTORS):
        raw += read_sector(img, ROOT_START + i)
    return raw

def write_root(img, raw):
    for i in range(ROOT_SECTORS):
        write_sector(img, ROOT_START + i, raw[i*SECTOR_SIZE:(i+1)*SECTOR_SIZE])

def parse_entries(raw):
    entries = []
    for i in range(ROOT_ENTRIES):
        off = i * DIR_ENTRY_SIZE
        e = raw[off:off+DIR_ENTRY_SIZE]
        first = e[0]
        if first == 0x00:
            break
        if first == 0xE5:
            entries.append(None)   # deleted slot
            continue
        attr = e[11]
        if attr & 0x08 or attr & 0x10:
            entries.append(None)   # label or directory, skip
            continue
        name = e[0:8].decode('ascii', errors='replace').rstrip()
        ext  = e[8:11].decode('ascii', errors='replace').rstrip()
        cluster = struct.unpack_from('<H', e, 26)[0]
        size    = struct.unpack_from('<I', e, 28)[0]
        entries.append({'idx': i, 'name': name, 'ext': ext,
                        'cluster': cluster, 'size': size})
    return entries

def find_entry(entries, name83):
    """name83: 'NAME    EXT' — 8+3 uppercase space-padded"""
    name_part = name83[:8].upper()
    ext_part  = name83[8:11].upper()
    for e in entries:
        if e is None:
            continue
        en = e['name'].upper().ljust(8)
        ee = e['ext'].upper().ljust(3)
        if en == name_part and ee == ext_part:
            return e
    return None

def to_83(filename):
    """Convert 'hello.txt' → 'HELLO   TXT'"""
    filename = os.path.basename(filename).upper()
    if '.' in filename:
        n, x = filename.rsplit('.', 1)
    else:
        n, x = filename, ''
    return (n[:8].ljust(8) + x[:3].ljust(3))

def fmt_name(e):
    if e['ext']:
        return f"{e['name']}.{e['ext']}"
    return e['name']

# ── data read/write ─────────────────────────────────────────────────────────

def read_file(img, fat, entry):
    data = bytearray()
    cluster = entry['cluster']
    remaining = entry['size']
    while 0x002 <= cluster < 0xFF8 and remaining > 0:
        sector = DATA_START + (cluster - 2)
        chunk = read_sector(img, sector)
        take = min(SECTOR_SIZE, remaining)
        data += chunk[:take]
        remaining -= take
        cluster = fat_get(fat, cluster)
    return bytes(data)

def write_file_data(img, fat, data):
    """Write data bytes, allocate clusters. Returns first_cluster or None."""
    chunks = [data[i:i+SECTOR_SIZE] for i in range(0, max(len(data),1), SECTOR_SIZE)]
    if not chunks:
        chunks = [b'']
    first = None
    prev  = None
    for chunk in chunks:
        c = fat_alloc(fat)
        if c is None:
            return None
        fat_set(fat, c, 0xFFF)
        if prev is not None:
            fat_set(fat, prev, c)
        else:
            first = c
        sector_data = bytearray(SECTOR_SIZE)
        sector_data[:len(chunk)] = chunk
        write_sector(img, DATA_START + (c - 2), sector_data)
        prev = c
    return first

def set_entry_in_raw(raw, idx, name8, ext3, cluster, size, attr=0x20):
    off = idx * DIR_ENTRY_SIZE
    raw[off:off+8]  = name8.encode('ascii').ljust(8)[:8]
    raw[off+8:off+11] = ext3.encode('ascii').ljust(3)[:3]
    raw[off+11] = attr
    raw[off+12:off+26] = bytes(14)   # reserved
    struct.pack_into('<H', raw, off+26, cluster)
    struct.pack_into('<H', raw, off+28, 0)           # cluster hi
    struct.pack_into('<I', raw, off+28, size)

def mark_deleted(raw, idx):
    raw[idx * DIR_ENTRY_SIZE] = 0xE5

def find_free_slot(raw):
    for i in range(ROOT_ENTRIES):
        off = i * DIR_ENTRY_SIZE
        b = raw[off]
        if b == 0x00 or b == 0xE5:
            return i
    return None

# ── commands ─────────────────────────────────────────────────────────────────

def cmd_info(img):
    img.seek(0, 2)
    size = img.tell()
    sectors = size // SECTOR_SIZE
    fat = read_fat(img)
    used = sum(1 for c in range(2, 2848) if fat_get(fat, c) != 0)
    free = sum(1 for c in range(2, 2848) if fat_get(fat, c) == 0)
    print(f"Image size : {size} bytes ({sectors} sectors)")
    print(f"Filesystem : FAT12")
    print(f"Clusters   : {used} used, {free} free ({free * SECTOR_SIZE // 1024} KB free)")

def cmd_ls(img):
    raw = read_root(img)
    entries = [e for e in parse_entries(raw) if e is not None]
    if not entries:
        print("(empty)")
        return
    print(f"{'NAME':<12}  {'SIZE':>8}")
    print("-" * 22)
    for e in entries:
        print(f"{fmt_name(e):<12}  {e['size']:>8} bytes")
    print(f"\n{len(entries)} file(s)")

def cmd_cat(img, filename):
    fat = read_fat(img)
    raw = read_root(img)
    entries = parse_entries(raw)
    entry = find_entry(entries, to_83(filename))
    if entry is None:
        print(f"Error: '{filename}' not found")
        sys.exit(1)
    data = read_file(img, fat, entry)
    sys.stdout.buffer.write(data)

def cmd_get(img, filename, dest=None):
    fat = read_fat(img)
    raw = read_root(img)
    entries = parse_entries(raw)
    entry = find_entry(entries, to_83(filename))
    if entry is None:
        print(f"Error: '{filename}' not found")
        sys.exit(1)
    data = read_file(img, fat, entry)
    if dest is None:
        dest = fmt_name(entry).replace(' ', '')
    with open(dest, 'wb') as f:
        f.write(data)
    print(f"Extracted {len(data)} bytes → {dest}")

def cmd_put(img, host_path, dest_name=None):
    if not os.path.exists(host_path):
        print(f"Error: '{host_path}' not found")
        sys.exit(1)
    with open(host_path, 'rb') as f:
        data = f.read()
    if len(data) > 65535:
        print("Error: file too large (max 64KB for FAT12 in this kernel)")
        sys.exit(1)

    name83 = to_83(dest_name if dest_name else host_path)
    name8  = name83[:8].rstrip()
    ext3   = name83[8:11].rstrip()

    fat = read_fat(img)
    raw = read_root(img)
    entries = parse_entries(raw)

    # overwrite if exists
    existing = find_entry(entries, name83)
    if existing:
        fat_free_chain(fat, existing['cluster'])
        slot_idx = existing['idx']
    else:
        slot_idx = find_free_slot(raw)
        if slot_idx is None:
            print("Error: root directory full")
            sys.exit(1)

    first_cluster = write_file_data(img, fat, data)
    if first_cluster is None:
        print("Error: disk full")
        sys.exit(1)

    # patch the dir entry manually
    off = slot_idx * DIR_ENTRY_SIZE
    raw[off:off+8]    = name8.upper().encode('ascii').ljust(8)[:8]
    raw[off+8:off+11] = ext3.upper().encode('ascii').ljust(3)[:3]
    raw[off+11] = 0x20
    raw[off+12:off+26] = bytes(14)
    struct.pack_into('<H', raw, off+26, first_cluster)
    struct.pack_into('<H', raw, off+28, 0)
    struct.pack_into('<I', raw, off+28, len(data))

    write_fat(img, fat)
    write_root(img, raw)
    print(f"Written {len(data)} bytes → {name8.strip()}.{ext3.strip()}")

def cmd_rm(img, filename):
    fat = read_fat(img)
    raw = read_root(img)
    entries = parse_entries(raw)
    entry = find_entry(entries, to_83(filename))
    if entry is None:
        print(f"Error: '{filename}' not found")
        sys.exit(1)
    fat_free_chain(fat, entry['cluster'])
    mark_deleted(raw, entry['idx'])
    write_fat(img, fat)
    write_root(img, raw)
    print(f"Deleted {fmt_name(entry)}")

# ── main ─────────────────────────────────────────────────────────────────────

USAGE = __doc__

def main():
    if len(sys.argv) < 3:
        print(USAGE)
        sys.exit(1)

    img_path = sys.argv[1]
    cmd      = sys.argv[2].lower()
    args     = sys.argv[3:]

    if not os.path.exists(img_path):
        print(f"Error: '{img_path}' not found")
        sys.exit(1)

    mode = 'r+b' if cmd in ('put', 'rm') else 'rb'
    with open(img_path, mode) as img:
        if cmd == 'info':
            cmd_info(img)
        elif cmd == 'ls':
            cmd_ls(img)
        elif cmd == 'cat':
            if not args: print("Usage: cat <NAME.EXT>"); sys.exit(1)
            cmd_cat(img, args[0])
        elif cmd == 'get':
            if not args: print("Usage: get <NAME.EXT> [dest]"); sys.exit(1)
            cmd_get(img, args[0], args[1] if len(args)>1 else None)
        elif cmd == 'put':
            if not args: print("Usage: put <host_file> [NAME.EXT]"); sys.exit(1)
            cmd_put(img, args[0], args[1] if len(args)>1 else None)
        elif cmd == 'rm':
            if not args: print("Usage: rm <NAME.EXT>"); sys.exit(1)
            cmd_rm(img, args[0])
        else:
            print(f"Unknown command: {cmd}")
            print(USAGE)
            sys.exit(1)

if __name__ == '__main__':
    main()
