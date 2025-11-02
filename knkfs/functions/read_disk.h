#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>

#define min_size_MB 32
#define MAX_ADDRESS 260

char *read_disk();
int validate_disk(const char *address);
char *get_disk();