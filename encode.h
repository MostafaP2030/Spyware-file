#ifndef ENCODE
#define ENCODE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>

#define MAX_COMMANDS 64
#define MAX_CMD_LEN  1024

char* commands[MAX_COMMANDS];
int command_count = 0;

int split_commands(const char* input) {
    if (!input || strlen(input) == 0) return 0;

    command_count = 0;
    char* str = strdup(input);
    if (!str) return 0;

    char* token;
    char* rest = str;

    // strtok_r برای thread-safety و کنترل بهتر
    while ((token = strtok_r(rest, "&;", &rest))) {
        // حذف فضاهای اول و آخر
        while (*token && isspace((unsigned char)*token)) token++;
        
        char* end = token + strlen(token) - 1;
        while (end >= token && isspace((unsigned char)*end)) end--;
        *(end + 1) = '\0';

        // اگر دستور خالی نباشه، اضافه کن
        if (strlen(token) > 0) {
            if (command_count < MAX_COMMANDS - 1) {
                commands[command_count] = strdup(token);
                command_count++;
            }
        }
    }

    free(str);
    commands[command_count] = NULL;  // علامت پایان آرایه
    return command_count;
}

void simple_cipher(char *text)
{
    int i;
    for (i = 0; text[i]; i++)
    {
        if (text[i] % 2)
            text[i]--;
        else
            text[i]++;
    }
    text[i] = '\0';
}

char* json_escape(const char* input) {
    if (!input) return strdup("");
    
    size_t len = strlen(input);
    char* output = malloc(len * 6 + 1);  // حداکثر 6 بایت برای \uXXXX
    if (!output) return NULL;
    
    char* p = output;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = input[i];
        if (c == '"' ) { strcpy(p, "\\\""); p += 2; }
        else if (c == '\\') { strcpy(p, "\\\\"); p += 2; }
        else if (c == '\b') { strcpy(p, "\\b"); p += 2; }
        else if (c == '\f') { strcpy(p, "\\f"); p += 2; }
        else if (c == '\n') { strcpy(p, "\\n"); p += 2; }
        else if (c == '\r') { strcpy(p, "\\r"); p += 2; }
        else if (c == '\t') { strcpy(p, "\\t"); p += 2; }
        else if (c < 32 || c >= 127) {  // غیرقابل چاپ یا غیر ASCII
            sprintf(p, "\\u%04x", c);
            p += 6;
        }
        else {
            *p++ = c;
        }
    }
    *p = '\0';
    return output;
}

#endif