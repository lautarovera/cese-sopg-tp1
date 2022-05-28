#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "Fifo"
#define SIGN_FILE_NAME "Sign.txt"
#define DATA_FILE_NAME "Log.txt"
#define PREFIX_SIZE 5u
#define BUFFER_SIZE 300u

int main(void)
{
    uint8_t input_buffer[BUFFER_SIZE];
    int32_t bytes_read, ret_code, fd_fifo;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((ret_code = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", ret_code);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("Waiting for writers...\n");
    if ((fd_fifo = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd_fifo);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("Got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        if ((bytes_read = read(fd_fifo, input_buffer, BUFFER_SIZE)) == -1)
        {
            perror("Reader");
        }
        else
        {
            int32_t fd_file;
            uint32_t bytes_wrote;
            const char *file_name;
            input_buffer[bytes_read] = '\0';
            char *msg = malloc(strlen(input_buffer) - PREFIX_SIZE);

            printf("Reader: read %d bytes: \"%s\"\n", bytes_read, input_buffer);

            if (sscanf(input_buffer, "SIGN:%s", msg) == 1)
            {
                file_name = SIGN_FILE_NAME;
            }
            else if (sscanf(input_buffer, "DATA:%s", msg) == 1)
            {
                file_name = DATA_FILE_NAME;
            }
            else
            {
                printf("Format must be 'DATA:XXXXXXXX'\n");
                continue;
            }

            /* Create regular file. -1 means already exists so no action if already exists */
            if ((ret_code = mknod(file_name, S_IFREG | 0666, 0)) < -1)
            {
                printf("Error creating regular file: %d\n", ret_code);
                exit(1);
            }

            if ((fd_file = open(file_name, O_WRONLY | O_APPEND)) < 0)
            {
                printf("Error opening regular file: %d\n", fd_file);
                exit(1);
            }

            /* Write buffer to regular file */
            if ((bytes_wrote = write(fd_file, msg, strlen(msg))) == -1)
            {
                perror("Reader");
            }
            else
            {
                printf("Reader: wrote %d bytes in %s\n", bytes_wrote, file_name);
                free(msg);
            }

            if (close(fd_file) < 0)
            {
                printf("Error closing regular file: %d\n", fd_file);
                exit(1);
            }
        }
    } while (bytes_read > 0);

    return 0;
}
