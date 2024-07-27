#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdbool.h>
#include <curl/curl.h>

#define TEMP_FILENAME "/tmp/temp"
#define STATE_FILENAME "/tmp/status"
#define SERVER_URL "https://34.204.54.137:5000/data"

static const char* DAEMON_NAME = "iotclientd";
static const long SLEEP_DELAY = 10;

static void _exit_process(const char* msg) {
    printf("%s\n", msg);
    syslog(LOG_INFO, "%s", msg);
    closelog();
    exit(EXIT_FAILURE);
}

static void _signal_handler(int signal) {
    switch(signal) {
        case SIGHUP:
            break;
        case SIGTERM:
            _exit_process("Received SIGTERM, exiting.");
            break;
        default:
            syslog(LOG_INFO, "Received unhandled signal");
    }
}

static void _daemonize(void) {
    printf("Starting daemonize\n");
    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        printf("Exiting parent process\n");
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        printf("setsid failed\n");
        exit(EXIT_FAILURE);
    }
    printf("New session created\n");

    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);

    umask(0);
    printf("File mode creation mask set to zero\n");

    if (chdir("/") < 0) {
        printf("Failed to change directory to /\n");
        exit(EXIT_FAILURE);
    }
    printf("Changed directory to /\n");

    printf("Daemonize completed\n");
    openlog(DAEMON_NAME, LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon started");
}

static float read_temperature(void) {
    printf("Reading temperature\n");
    syslog(LOG_INFO, "Reading temperature");
    FILE* fp = fopen(TEMP_FILENAME, "r");
    if (!fp) _exit_process("Failed to open temperature file.");
    float temp;
    fscanf(fp, "%f", &temp);
    fclose(fp);
    syslog(LOG_INFO, "Read temperature: %f", temp);
    return temp;
}

static bool read_heater_state(void) {
    printf("Reading heater state\n");
    syslog(LOG_INFO, "Reading heater state");
    FILE* fp = fopen(STATE_FILENAME, "r");
    if (!fp) _exit_process("Failed to open state file.");
    char state[4];
    fscanf(fp, "%3s", state);
    fclose(fp);
    syslog(LOG_INFO, "Read heater state: %s", state);
    return (strcmp(state, "ON") == 0);
}

static void send_data(float temperature, bool heater_state) {
    printf("Sending data\n");
    syslog(LOG_INFO, "Sending data");
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        char postdata[100];
        snprintf(postdata, sizeof(postdata), "temperature=%.2f&heater_state=%s", temperature, heater_state ? "ON" : "OFF");
        curl_easy_setopt(curl, CURLOPT_URL, SERVER_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            syslog(LOG_ERR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        else
            syslog(LOG_INFO, "Data sent successfully");
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

static void _run_client(void) {
    syslog(LOG_INFO, "Running client loop");
    printf("Running client loop\n");
    while(true) {
        float temperature = read_temperature();
        bool heater_state = read_heater_state();
        send_data(temperature, heater_state);
        sleep(SLEEP_DELAY);
    }
}

int main(void) {
    syslog(LOG_INFO, "Starting main function");
    printf("Starting main function\n");
    _daemonize();
    printf("After daemonize\n");
    _run_client();
    printf("After running client loop\n");
    return EXIT_SUCCESS;
}
