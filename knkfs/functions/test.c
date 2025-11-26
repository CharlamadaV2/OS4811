// TODO: Figure out block size
// TODO: Figure out how many blocks will be used to store data
// TODO: Figure out how to use SPI 
// TODO: Figure out how to make wear leveling algorithm

#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)
#include <stdint.h>
#include <stdio.h>
#include <errno.h>  // For errno

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

typedef struct {
    //

} text_file; 

typedef struct {
    //

} config_file; 


/* Store all meta data onto the EEPROM */

void create_file(char name[12], uint8_t type, uint16_t address){
//File name, File type, File address (offset)

//Check if file type with name already exists
//Ask if write wants to overwrite
//If yes remove inode pointer, update blockmap vacancy, update table
//If no exit from prompt

    inode new_file;
    switch (type) {
        //For the test only txt and config will be created
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
            printf("Creating data file");
            new_file.type = 3;
            break;
        default:
        printf("This file type does not exist...");
    }

}

void read_file(char name[12], uint8_t type){
    //Check if file exist with name and type
    // If not; error message
    // Exit program
    // If exists; read value inside file
    // Print out file properties
    // Exit program
}

void list_files(){ //Read files inside Inode Table




}

void write(){ //This function will contain SPI processes
    
    


}

void read(){ //This function will contain SPI processes




}