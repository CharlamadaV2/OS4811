#include <read_disk.h>

char *read_disk(){
    // Take address of raw disk
    static char disk_address[MAX_ADDRESS]; // Static so it persists after return
    printf("Enter a disk address: ");
    scanf("%255s", disk_address);
    return disk_address;
}

int validate_disk(const char *address){
    int min_MB = min_size_MB;
    long long disk_size_bytes;
    int disk_size_MB;
    
    // Open the raw disk device
    int fd = open(address, O_RDONLY);
    if (fd < 0) {
        perror("Cannot open disk device");
        return 0;
    }
    
    // Get the disk size using ioctl
    if (ioctl(fd, BLKGETSIZE64, &disk_size_bytes) < 0) {
        perror("Cannot get disk size");
        close(fd);
        return 0;
    }
    
    close(fd);
    
    // Convert bytes to MB
    disk_size_MB = disk_size_bytes / (1024 * 1024);
    
    // If raw disk is less than min_thresh
    if(disk_size_MB < min_MB){
        perror("Disk size too small");
        return 0;
    }
    
    // Get block size if needed
    // int block_size = disk_size_bytes / num_blocks;
    
    return 1; // Success
}

//Reads disk and returns it if valid
char *get_disk(){
    char *disk_address[MAX_ADDRESS] = read_disk();

    //Runs exception catcher
    if (validate_disk(disk_address)){
        return disk_address;
    }else{
    return NULL;
    }
}