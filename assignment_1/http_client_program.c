#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void print_usage(const char *progname) {
    printf("Usage: %s [--get | --post | --put | --delete] --url <URL> [data]\n", progname);
}

void perform_request(const char *url, const char *data, const char *method) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        if(strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if(strcmp(method, "PUT") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if(strcmp(method, "DELETE") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            printf("HTTP Code: %ld\n", http_code);
        }

        curl_easy_cleanup(curl);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *url = NULL;
    const char *data = "";
    const char *method = NULL;

    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--url") == 0 || strcmp(argv[i], "-u") == 0) {
            if(i + 1 < argc) {
                url = argv[++i];
            } else {
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(argv[i], "--get") == 0 || strcmp(argv[i], "-g") == 0) {
            method = "GET";
        } else if(strcmp(argv[i], "--post") == 0 || strcmp(argv[i], "-o") == 0) {
            method = "POST";
        } else if(strcmp(argv[i], "--put") == 0 || strcmp(argv[i], "-p") == 0) {
            method = "PUT";
        } else if(strcmp(argv[i], "--delete") == 0 || strcmp(argv[i], "-d") == 0) {
            method = "DELETE";
        } else if (i == argc - 1) {
            data = argv[i];
        }
    }

    if(url == NULL || method == NULL) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    perform_request(url, data, method);

    return EXIT_SUCCESS;
}
