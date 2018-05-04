#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LEN 50
#define BYTE_LEN 16
static char receive[BUFFER_LEN];

int main(int argc, char *argv[])
{
        int ret, fd, count = 0;
        char stringToSend[BYTE_LEN  + 1];
        printf("Starting device test code example...\n");
        fd = open("/dev/decdev", O_RDWR);
        if (fd < 0)
        {
                perror("Failed to open the device...");
                return errno;
        }
        char *inputFile = argv[1];
        FILE *in;
        in = fopen(inputFile, "r");
        if (in == NULL)
        {
                 printf( "Could not open file %s\n", inputFile);
                 return 1;
        }

        char readline[BYTE_LEN + 1];
        while (fgets(readline, BYTE_LEN + 1, in) != NULL)
        {
                strcpy(stringToSend, readline);
                printf("Writing message to the device: %s\n", stringToSend);
                getchar();
                ret = write(fd, stringToSend, strlen(stringToSend));
                if (ret < 0)
                {
                        perror("Failed to write the message to the device.");
                        return errno;
                }
                count = count + 1;
        }
        fclose(in);

        printf("Press Enter once.\n");
        getchar();

        printf("Reading from the device.\n");
        ret = read(fd, receive, BYTE_LEN);
        if (ret < 0)
        {
                perror("Failed to read the message from the device.");
                return errno;
        }
        printf("Key is: %s\n", receive);
        while (count > 1)
        {
                ret = read(fd, receive, BYTE_LEN);
                if (ret < 0)
                {
                        perror("Failed to read the message from the device.");
                        return errno;
                }
                printf("Message is: %s\n", receive);
                count--;
        }
        printf("End of the program\n");
        return 0;
}
