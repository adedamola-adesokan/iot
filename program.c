#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_URL_LEN 2048
#define MAX_DATA_LEN 4096

// Function to write the response data to a string
size_t write_callback(void *ptr, size_t size, size_t nmemb, char *data) {
    strncat(data, ptr, size * nmemb);
    return size * nmemb;
}

// Function to print help message
void print_help() {
    printf("Usage: hw [options] <data>\n");
    printf("Options:\n");
    printf("  -u, --url      URL to send the request to (including port number)\n");
    printf("  -o, --post     Use HTTP POST method\n");
    printf("  -g, --get      Use HTTP GET method\n");
    printf("  -p, --put      Use HTTP PUT method\n");
    printf("  -d, --delete   Use HTTP DELETE method\n");
    printf("  -h, --help     Print this help message\n");
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_help();
        return 1;
    }

    CURL *curl;
    CURLcode res;
    char url[MAX_URL_LEN] = {0};
    char data[MAX_DATA_LEN] = {0};
    int method = 0; // 1 = POST, 2 = GET, 3 = PUT, 4 = DELETE

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--url") == 0) {
            if (i + 1 < argc) {
                strncpy(url, argv[++i], MAX_URL_LEN - 1);
            } else {
                fprintf(stderr, "URL missing after %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--post") == 0) {
            method = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--get") == 0) {
            method = 2;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--put") == 0) {
            method = 3;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            method = 4;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else {
            // Collect remaining arguments as data
            strncat(data, argv[i], MAX_DATA_LEN - strlen(data) - 1);
            if (i < argc - 1) {
                strncat(data, " ", MAX_DATA_LEN - strlen(data) - 1);
            }
        }
    }

    if (strlen(url) == 0) {
        fprintf(stderr, "URL is required\n");
        return 1;
    }

    if (method == 0) {
        fprintf(stderr, "HTTP method is required\n");
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        char response[MAX_DATA_LEN] = {0};
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        switch (method) {
            case 1:
                if (strlen(data) == 0) {
                    fprintf(stderr, "POST data is required\n");
                    return 1;
                }
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
                break;
            case 2:
                // GET is the default method, no extra setup needed
                break;
            case 3:
                if (strlen(data) == 0) {
                    fprintf(stderr, "PUT data is required\n");
                    return 1;
                }
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
                break;
            case 4:
                if (strlen(data) != 0) {
                    fprintf(stderr, "DELETE does not require data\n");
                    return 1;
                }
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            default:
                fprintf(stderr, "Unknown method\n");
                return 1;
        }

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return 1;
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("%ld %s\n", response_code, response);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
