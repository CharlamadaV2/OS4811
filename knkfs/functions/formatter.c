//This code was written by Kimson Lam

//Description:
/*
The purpose of this code is to build the file system in the EEPROM.

Note: All structures, variables, and headers used will be stored in the formatter header.
*/



#include <stdint.h> // for uint values
#include <stdio.h>
#include <string.h>   // for memset, strlen, strcpy 
#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)

//The is EEPROM 4MB (2^22 = 4 MB) since (2^20 = 1 MB) and (2^2 = 4)
#define block_size 32 //bytes (may remove)
#define num_blocks 16
#define num_inodes 16

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

typedef struct {
    inode inodes[num_inodes]; //16 inodes for the blocks
} inode_table;  

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

