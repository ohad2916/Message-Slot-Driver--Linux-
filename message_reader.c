#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define BUF_LEN 128
#define MAJOR_NUMBER 235
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUMBER, 0, unsigned long)


int main(int argc, char* args[])
{
    if (argc != 3)
    {
        perror("Wrong amount of arguments!\n");
        exit(1);
    }

    
    char* filepath = args[1];
    unsigned long channel_id = atoi(args[2]);
    char msg[BUF_LEN+1];
    msg[BUF_LEN] = '\0';

    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
    {
        perror("Could not open File!\n");
        exit(1);
    }

    // set channel id
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0)
    {
        perror("IOCTL Failed!\n");
        close(fd);
        exit(1);
    }

    //read from file
    if (read(fd, msg, BUF_LEN) < 0)
    {
        perror("Error Reading from file!\n");
        close(fd);
        exit(1);
    }

    // print the thing
    printf("%s\n", msg);

    close(fd);
    exit(0);
}