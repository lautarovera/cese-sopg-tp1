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
#define BUFFER_SIZE 300

typedef enum
{
    SIGN,
    DATA,
    FAIL
} parser_t;

static parser_t parser(uint8_t *str)
{
    parser_t ret_code = FAIL;
    char *token;

    if ((token = strtok(str, ":")) != NULL)
    {
        if (!strcmp(token, "SIGN\0"))
        {
            token = strtok(NULL, "\0");
            memset(str, 0, strlen(str));
            sprintf(str, "%s\n", token);
            ret_code = SIGN;
        }
        else if (!strcmp(token, "DATA\0"))
        {
            token = strtok(NULL, "\0");
            memset(str, 0, strlen(str));
            sprintf(str, "%s\n", token);
            ret_code = DATA;
        }
        else
        {
            /* No parsing available */
        }
    }
    else
    {
        /* No parsing available */
    }

    return ret_code;
}

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
            printf("Reader: read %d bytes: \"%s\"\n", bytes_read, input_buffer);

            switch (parser(input_buffer))
            {
            case SIGN:
                file_name = SIGN_FILE_NAME;
                break;
            case DATA:
                file_name = DATA_FILE_NAME;
                break;
            case FAIL:
            default:
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
            if ((bytes_wrote = write(fd_file, input_buffer, strlen(input_buffer))) == -1)
            {
                perror("Reader");
            }
            else
            {
                printf("Reader: wrote %d bytes in %s\n", bytes_wrote, file_name);
                // int l = lseek(fd_file, bytes_wrote, SEEK_CUR);
                // printf("SEEK POINTER: %d\n", l);
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
