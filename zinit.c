#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define ZEROCORE_BIN  "/sbin/core0-real"

typedef enum procstat_t {
    PROCESS_DIED,
    PROCESS_CORE0_GRACEFULLY,
    PROCESS_CORE0_ERROR,
    SYSTEM_ERROR,

} procstat_t;

void warnp(char *str) {
    fprintf(stderr, "[-] %s: %s\n", str, strerror(errno));
}

void warnpchild(char *str) {
    fprintf(stderr, "[-]   %s: %s\n", str, strerror(errno));
}

pid_t zcore(char **envp) {
    pid_t corepid;
    char *argscore[] = {ZEROCORE_BIN, "-c", "/etc/zero-os/zero-os.toml", NULL};

    printf("[+] forking\n");
    corepid = fork();

    if(corepid == -1) {
        perror("fork");
        return -1;
    }

    if(corepid == 0) {
        printf("[+]   executing core0\n");

        execve(ZEROCORE_BIN, argscore, envp);
        warnpchild(ZEROCORE_BIN);

        printf("[-]   exiting not usable child\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] child started with pid %d\n", corepid);

    return corepid;
}

procstat_t zwait(pid_t corepid) {
    int wstatus;
    pid_t child;

    if((child = wait(&wstatus)) < 0) {
        warnp("wait");
        return SYSTEM_ERROR;
    }

    if(child != corepid) {
        // we catched a died process
        printf("[+] process %d died, cleaning state\n", child);
        return PROCESS_DIED;
    }

    if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus) > 0) {
        printf("[-] abnormal termination of core0\n");
        return PROCESS_CORE0_ERROR;
    }

    return PROCESS_CORE0_GRACEFULLY;
}

pid_t coreinit(char **envp) {
    pid_t corepid;

    if((corepid = zcore(envp)) < 0) {
        printf("[-] cannot prepare core0 environment, cannot do anything more\n");
        exit(EXIT_FAILURE);
        // kernel panic - attempted to kill init, obviously
    }

    return corepid;
}

void sysloop(int argc, char **argv, char **envp) {
    (void) argc;
    (void) argv;

    struct timespec timeout = {
        .tv_sec = 2,
        .tv_nsec = 0,
    };

    pid_t corepid;
    corepid = coreinit(envp);

    while(1) {
        int procstat = zwait(corepid);

        switch(procstat) {
            case PROCESS_DIED:
                // nothing special to do
                break;

            case SYSTEM_ERROR:
                // let's try again, nothing special to do
                break;

            case PROCESS_CORE0_ERROR:
                printf("[-] abnormal termination, waiting for restart\n");
                nanosleep(&timeout, NULL);
                break;

            case PROCESS_CORE0_GRACEFULLY:
                printf("[+] core0 gracefully exited, restarting...\n");
                // post-update process
                return;
        }
    }
}

int main(int argc, char **argv, char **envp) {
    printf("[+] ==============================================\n");
    printf("[+] ==            Zero-OS System Init           ==\n");
    printf("[+] ==============================================\n");
    printf("[+]\n");

    while(1) {
        sysloop(argc, argv, envp);
    }

    return 0;
}
