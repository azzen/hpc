#ifndef PERF_MANAGER_H
#define PERF_MANAGER_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef struct {
    int ctl_fd;
    int ack_fd;
    unsigned char enable;
} perf_manager;

static const char *enable_cmd = "enable";
static const char *disable_cmd = "disable";
static const char *ack_cmd = "ack\n";

void send_command(perf_manager *pm, const char *command) {
    if (pm->enable) {
        write(pm->ctl_fd, command, strlen(command));
        char ack[5];
        read(pm->ack_fd, ack, 5);
        assert(strcmp(ack, ack_cmd) == 0);
    }
}

void perf_manager_init(perf_manager *pm) {
    char *ctl_fd_env = getenv("PERF_CTL_FD");
    char *ack_fd_env = getenv("PERF_ACK_FD");
    if (ctl_fd_env && ack_fd_env) {
        pm->enable = 1;
        pm->ctl_fd = atoi(ctl_fd_env);
        pm->ack_fd = atoi(ack_fd_env);
    } else {
        pm->enable = 0;
        pm->ctl_fd = -1;
        pm->ack_fd = -1;
    }
}

void perf_manager_pause(perf_manager *pm) {
    send_command(pm, disable_cmd);
}

void perf_manager_resume(perf_manager *pm) {
    send_command(pm, enable_cmd);
}

#endif