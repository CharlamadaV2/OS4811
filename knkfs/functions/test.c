// TODO: Figure out block size
// TODO: Figure out how many blocks will be used to store data
// TODO: Figure out how to use SPI 
// TODO: Figure out how to make wear leveling algorithm

//EEPROM Information:
// 32 Mbits of page EEPROM
// 64 Kbyte blocks
// 4 Kbyte sectors
// 512 byte pages

//  8192 erasable pages of 512 bytes

/* 1024 erasable sectors of 4 Kbytes, 
64 erasable blocks of 64 Kbytes, 
or as an entirely erasable array
*/

// The EEPROM can take two modes (CPOL CPHA): '11' or '00'

#include <stdint.h> //for uint
#include <stdio.h> // for debugging
#include <errno.h>  // For errno
#include <stdlib.h>
#include <unistd.h>

//libraries for SPI
#include <string.h> // functions for memory control
#include <fcntl.h> // functions for file control; open devices
#include <sys/ioctl.h> // functions for IO control
#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)


#define num_blocks 16

// File type definitions
#define FILE_TYPE_TXT 1
#define FILE_TYPE_CONFIG 2
#define FILE_TYPE_ASM 3
#define FILE_TYPE_INVALID -1

static int spi_fd = -1;  //Default is -1; no device connected

// The block map tells the user which blocks are free or not
typedef struct {
    uint8_t bitmap[num_blocks];
    // Each bit represents an available block (0 = free, 1 = used)
    // 1 byte = 8 bits, 128 bytes / 8 = 16
    // 16 block rows w/ each bit representing a block
    uint32_t wear_level[num_blocks];
    // Wear level represent amount of time used
    // 4 bits for each block, 15 is lowest priority, 0 is highest priority
    // We want to use blocks with least amount of use
    
} block_map;

//Byte size for the 
typedef struct superblock_properties{
    int64_t time_stamp; // Time Stamp of when volume has changed.
    uint64_t data_size; // Size of Data Area in blocks
    uint64_t index_size; // Size of Index Area in bytes

    uint8_t magic[3]; // signature ‘KNKFS’ (0x534653)
    uint8_t version; // SFS version (0x10 = 1.0, 0x1A = 1.10)
    uint64_t total_blocks; // Total number of blocks in volume (including reserved area)
    uint32_t rsvd_blocks; // Number of reserved blocks | MIGHT REMOVE!
    uint8_t blk_size;  // log(x+7) of block size (x = 2 = 512)
    uint8_t crc; // Zero sum of bytes above 0b11111111 -> 0xFF
} superblock;

typedef struct { //32 byte block
    char name[12];               // File name (up to 11 chars + null) 12 bytes
    uint32_t size;               // File size in bytes 4 bytes, size of entire node
    uint8_t type;                // File type: Config, text, and sensor data
    uint8_t direct_blocks[num_blocks];   // Direct block pointers (up to 16 blocks) 
    extended_inode xtd_inode; //Point to extended node
} inode;

typedef struct {
    uint32_t size;
    uint8_t parent_inode; 
    extended_inode xtd_inode[num_blocks];
} extended_inode;

typedef struct {
    //

} text_file; 

typedef struct {
    //

} config_file; 

//Data structures for type map
typedef struct {
    char *type_name;
    uint8_t type_value;
} TypeEntry;

TypeEntry type_map[] = {
    {"txt", FILE_TYPE_TXT},
    {"config", FILE_TYPE_CONFIG},
    {"asm", FILE_TYPE_ASM},
    {NULL, 0}  // Sentinel value to mark end of map
};

/////////////////////////// Functions ////////////////////////////////

void mount_fs(){ // Mounts driver to file system
    spi_fd = open("/dev/spidev0.0", O_RDWR); //file descripter describes open status
    //The open system call here grants the device in SPI0 read and write acess

    if (spi_fd < 0) { //If the fd is not a positive value it means it didn't open
        perror("Failed to open SPI device");
        exit(1);
    }

    uint8_t mode = SPI_MODE_0; // 0 CPOL, 0 CPHA; samples on rising edge
    uint8_t bits = 8;
    uint32_t speed = 80000000; // 80 MHz; based on the doc

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) { //Tries to set SPI mode to 0
        perror("Failed to set SPI mode");
        exit(1);
    }

    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits); //Binary code length is 8 bits; 2 hex
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    printf("SPI mounted successfully\n");

}

void help(){
    printf("Available commands: write, read, ls, help, quit");
}

/* Store all meta data onto the EEPROM */

/////////////////////// Create Files /////////////////////////////////
void write_command(){
    //Tell user to enter file name
    printf("Enter file name: ");
    char name[12];
    scanf("%11s", name);  // Use %11s to prevent buffer overflow
    
    printf("Enter file type (txt, config, asm): ");
    char type_str[20];
    scanf("%19s", type_str);
    
    // Convert type string to value
    int8_t type_value = get_type_value(type_str);
    
    // Check if conversion was successful
    if (type_value == FILE_TYPE_INVALID) {
        return;
    }

    printf("Creating file '%s' of type '%s'\n", name, type_str);
    
    // Call your read_file function with the validated type
    create_file(name, type_value);
}


void create_file(char name[12], uint8_t type){
//File name, File type, File address (offset)

//Check if file type with name already exists
//Ask if write wants to overwrite
//If yes remove inode pointer, update blockmap vacancy, update table
//If no exit from prompt

//Create wear leveling algorithm here
//Reads block map, selects block based on availability, wear level, itteration
//Ideal block: availability = 0, wear_level = 0, next itteration
//Algorithm will look at every block and hold the current best block,
// If the current best block matches the ideal block it will use it
// else it will store whatever the current best block is

    inode new_file;
    switch (type) {
        //For the test only txt and config will be created
        //Read user inputs until they press enter
        case 1: 
            printf("Creating txt file");
            new_file.type = 1;
            create_txt();
            break;
        case 2: 
            printf("Creating config file");
            new_file.type = 2;
            break;
        case 3:
            //The EEPROM can be used to store code
            printf("Creating ASM file");
            new_file.type = 3;
            break;
        default:
        printf("This file type does not exist...");
    }

}


/////////////////////// Read functions /////////////////////////////////////////////
void read_file(char name[12], uint8_t type){
//Checks if file with name and type is in inode table


}

int8_t get_type_value(const char *type_str) {
    //If input is empty
    if (type_str == NULL) {
        printf("ERROR: Type string is NULL\n");
        return FILE_TYPE_INVALID;
    }
    
    // Search through the type map
    for (int i = 0; type_map[i].type_name != NULL; i++) {
        if (strcmp(type_map[i].type_name, type_str) == 0) {
            return type_map[i].type_value;
        }
    }
    
    // Type not found
    printf("ERROR: Unknown file type '%s'\n", type_str);
    printf("Valid types are: txt, config, asm\n");
    return FILE_TYPE_INVALID;
}

void read_command() {
    printf("Enter file name: ");
    char name[12];
    scanf("%11s", name);  // Use %11s to prevent buffer overflow
    
    printf("Enter file type (txt, config, asm): ");
    char type_str[20];
    scanf("%19s", type_str);
    
    // Convert type string to value
    int8_t type_value = get_type_value(type_str);
    
    // Check if conversion was successful
    if (type_value == FILE_TYPE_INVALID) {
        return;
    }
    
    printf("Reading file '%s' of type '%s'\n", name, type_str);
    
    // Call your read_file function with the validated type
    read_file(name, type_value);
}

//////////////////////////////////////// LIST FILES ////////////////////////////////

void list_files(){ //list files inside Inode Table
    //Read names of all files in Inode table
    //Read types of files in Inode table
    //Types are converted to extension
    //Print out file names with file type extension
}

///////////////////////// System functions ///////////////////////////////////////
int spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
        .speed_hz = 80000000,
        .bits_per_word = 8,
        .cs_change = 0,
    };
    
    return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void eeprom_write(uint32_t address, uint8_t *data, size_t len) {
    // Example: Write Enable command (0x06)
    uint8_t cmd_wren = 0x06;
    spi_transfer(&cmd_wren, NULL, 1);
    
    // Write command (0x02) + 3 byte address + data
    uint8_t tx_buf[4 + len];
    tx_buf[0] = 0x02;  // Page Program command
    tx_buf[1] = (address >> 16) & 0xFF;
    tx_buf[2] = (address >> 8) & 0xFF;
    tx_buf[3] = address & 0xFF;
    memcpy(&tx_buf[4], data, len);
    
    spi_transfer(tx_buf, NULL, 4 + len);
    
    // Wait for write to complete (poll status register)
    usleep(5000);  // Typical write time
}

void eeprom_read(uint32_t address, uint8_t *data, size_t len) {
    uint8_t tx_buf[4 + len];
    uint8_t rx_buf[4 + len];
    
    tx_buf[0] = 0x03;  // Read command
    tx_buf[1] = (address >> 16) & 0xFF;
    tx_buf[2] = (address >> 8) & 0xFF;
    tx_buf[3] = address & 0xFF;
    memset(&tx_buf[4], 0, len);  // Dummy bytes for clock
    
    spi_transfer(tx_buf, rx_buf, 4 + len);
    
    memcpy(data, &rx_buf[4], len);  // Data starts after 4 command bytes
}

/////////////////////////// MAIN///////////////////////////////
int main(){
    mount_fs(); //Keeps trying to run until it can read status
    //Tell user to enter a command
    while(1){
        char command[100];
        printf("Enter a command...");
        //Read user function
        scanf("%s", &command);
        //loop until exit
        //enter command into hash table
        if(command == "write"){
            write_command();
        }
        else if(command == "read"){
            read_command();
        }
        else if(command == "ls"){
            list_files();
        }
        else if(command == "help"){
            help();
        }
        else if(command == "quit"){
            printf("Goodbye!");
            break;
        }
        else{
            printf("%s: command not found", &command);
            printf("Please enter a valid command. Type help for list of commands.");
        }
    }
}