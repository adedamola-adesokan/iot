// gcc -o secure_time_daemond secure_time_daemond.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define ERROR_FORMAT "%s"

volatile sig_atomic_t keep_running = 1;

static void _signal_handler(int signal) {
    switch (signal) {
        case SIGHUP:
            syslog(LOG_INFO, "received SIGHUP, reloading configuration.");
            // Reload configuration if necessary
            break;
        case SIGTERM:
            syslog(LOG_INFO, "received SIGTERM, exiting.");
            keep_running = 0;
            break;
        default:
            syslog(LOG_INFO, "received unhandled signal");
    }
}

static void _do_work(void) {
    while (keep_running) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char buffer[26];
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        syslog(LOG_INFO, "Current time: %s", buffer);
        sleep(1);
    }
}

int main(void) {
    pid_t pid, sid;

    // Open syslog
    openlog("time_daemond", LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
    syslog(LOG_INFO, "starting time_daemond");

    // Fork the parent process
    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return EXIT_FAILURE;
    }
    if (pid > 0) {
        return EXIT_SUCCESS;
    }

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return EXIT_FAILURE;
    }

    // Change the current working directory
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return EXIT_FAILURE;
    }

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Signal handling
    struct sigaction sa;
    sa.sa_handler = _signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error: cannot handle SIGTERM");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error: cannot handle SIGHUP");
        exit(EXIT_FAILURE);
    }

    // Do the work of the daemon
    _do_work();

    // Clean up
    syslog(LOG_INFO, "Daemon exiting.");
    closelog();

    return EXIT_SUCCESS;
}
