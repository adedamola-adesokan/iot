// Use make to compile. i.e make -f Makefile-arm. Dont forget to update the buildroot directory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>

void print_usage(const char *progname) {
    printf("Usage: %s [--get | --post | --put | --delete] --url <URL> [data]\n", progname);
    printf("Options:\n");
    printf("  -u, --url       This is URL to send the request to\n");
    printf("  -o, --post      Use this to send a POST request\n");
    printf("  -g, --get       Use this to send a GET request\n");
    printf("  -p, --put       Use this to send a PUT request\n");
    printf("  -d, --delete    Use this to send a DELETE request\n");
    printf("  -h, --help      Use this to display this help message\n");
}

void perform_request(const char *url, const char *data, const char *method) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if (strcmp(method, "PUT") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if (strcmp(method, "DELETE") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            printf("HTTP Code: %ld\n", http_code);
        }

        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize curl.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *url = NULL;
    const char *data = "";
    const char *method = NULL;

    static struct option long_options[] = {
        {"url", required_argument, 0, 'u'},
        {"post", no_argument, 0, 'o'},
        {"get", no_argument, 0, 'g'},
        {"put", no_argument, 0, 'p'},
        {"delete", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "u:ogpdh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'u':
                url = optarg;
                break;
            case 'o':
                method = "POST";
                break;
            case 'g':
                method = "GET";
                break;
            case 'p':
                method = "PUT";
                break;
            case 'd':
                method = "DELETE";
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (url == NULL || method == NULL) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if ((strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0 || strcmp(method, "DELETE") == 0) && optind < argc) {
        data = argv[optind];
    }

    perform_request(url, data, method);

    return EXIT_SUCCESS;
}
