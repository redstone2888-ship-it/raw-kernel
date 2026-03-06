// Simple string comparison function
int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++; b++;
    }
    return *a - *b;
}

// Simple string length function
int strlen(const char* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}

// split input line into arguments
int split(char* line, char* argv[]) {
    int argc = 0;

    while (*line) {

        // пропустить пробелы
        while (*line == ' ')
            line++;

        if (*line == 0)
            break;

        argv[argc++] = line;

        // идти до следующего пробела
        while (*line && *line != ' ')
            line++;

        if (*line) {
            *line = 0; // ← РАЗРЕЗАЕМ строку
            line++;
        }
    }

    return argc;
}

int atoi(const char* str) {
    int res = 0;
    int sign = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str) {
        if (*str < '0' || *str > '9') break;
        res = res * 10 + (*str - '0');
        str++;
    }

    return res * sign;
}