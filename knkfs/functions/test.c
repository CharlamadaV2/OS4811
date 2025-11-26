// TODO: Figure out block size
// TODO: Figure out how many blocks will be used to store data
// TODO: Figure out how to use SPI 
// TODO: Figure out how to make wear leveling algorithm

#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)
#include <stdint.h>
#include <stdio.h>

#define num_blocks 16

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

typedef struct textfile_properties {
    //

} text_file; 

void write(char name[12], uint8_t type, uint16_t address){
//File name, File type, File address (offset)
    inode new_file;
    switch (type) {
        case 1: 
            printf("Creating txt file");
            new_file.type = 1;
            break;
        case 2: 
            printf("Creating config file");
            new_file.type = 2;
            break;
        case 3:
            printf("Creating data file");
            new_file.type = 3;
            break;
        default:
        printf("This file type does not exist...");
    }



}

void create_txt(){
    //Update inode based on what is written



}