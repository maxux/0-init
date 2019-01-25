#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define ZEROCORE_BIN  "/sbin/core0-real"

void warnp(char *str) {
    fprintf(stderr, "[-] %s: %s\n", str, strerror(errno));
}

void warnpchild(char *str) {
    fprintf(stderr, "[-]   %s: %s\n", str, strerror(errno));
}

int zcore(char **envp) {
    pid_t corepid;
    char *argscore[] = {ZEROCORE_BIN, "-c", "/etc/zero-os/zero-os.toml", NULL};

    printf("[+] forking\n");
    corepid = fork();

    if(corepid == -1) {
        perror("fork");
        return 1;
    }

    if(corepid == 0) {
        printf("[+]   executing core0\n");

        execve(ZEROCORE_BIN, argscore, envp);
        warnpchild(ZEROCORE_BIN);

        printf("[-]   exiting not usable child\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] child started with pid %d\n", corepid);

    return 0;
}

int zwait() {
    int wstatus;
    pid_t child;

    if((child = waitpid(-1, &wstatus, 0) < 0)) {
        warnp("waitpid");
        return 1;
    }

    if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus) > 0) {
        printf("[-] abnormal termination of core0\n");
        return 1;
    }

    return 0;
}

int main(int argc, char **argv, char **envp) {
    (void) argc;
    (void) argv;

    struct timespec timeout = {
        .tv_sec = 1,
        .tv_nsec = 0,
    };

    // signal(SIGCHLD, SIG_IGN);

    printf("[+] initializing zero-init bootstrapping\n");
    nanosleep(&timeout, NULL);

    while(1) {
        if(zcore(envp)) {
            printf("[-] could not prepare 0core environment\n");
            nanosleep(&timeout, NULL);

            exit(EXIT_FAILURE);
            // kernel panic
        }

        if(zwait()) {
            printf("[-] abnormal termination, waiting for restart\n");
            nanosleep(&timeout, NULL);
        }

        printf("[+] core0 exited, restarting...\n");
    }

    // never reached

    return 0;
}
