#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>

#define FIFO_NAME "Fifo"
#define BUFFER_SIZE 300

typedef struct sigaction sigaction_t;

static int32_t fd;

static void sa_usr_1_handler(int sig)
{
    write(fd, "SIGN:1", 7u);
}

static void sa_usr_2_handler(int sig)
{
    write(fd, "SIGN:2", 7u);
}

int main(void)
{
    char output_buffer[BUFFER_SIZE];
    uint32_t bytes_wrote;
    int32_t ret_code;
    sigaction_t sa_usr_1, sa_usr_2;

    sa_usr_1.sa_handler = sa_usr_1_handler;
    sa_usr_1.sa_flags = SA_RESTART;
    sigemptyset(&sa_usr_1.sa_mask);
    sigaction(SIGUSR1, &sa_usr_1, NULL);

    sa_usr_2.sa_handler = sa_usr_2_handler;
    sa_usr_2.sa_flags = SA_RESTART;
    sigemptyset(&sa_usr_2.sa_mask);
    sigaction(SIGUSR2, &sa_usr_2, NULL);

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((ret_code = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", ret_code);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("Waiting for readers...\n");
    if ((fd = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("Got a reader. Type something...\n");

    /* Loop forever */
    while (1)
    {
        /* Get some text from console */
        fgets(output_buffer, BUFFER_SIZE, stdin);

        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
        if ((bytes_wrote = write(fd, output_buffer, strlen(output_buffer) - 1)) == -1)
        {
            perror("Writer");
        }
        else
        {
            printf("Writer: wrote %d bytes\n", bytes_wrote);
        }
    }
    return 0;
}
