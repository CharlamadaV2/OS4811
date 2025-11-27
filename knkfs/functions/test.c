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

#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)
#include <stdint.h> //for uint
#include <stdio.h> // for debugging
#include <errno.h>  // For errno
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h> // for erasing, turning on write protect
#include <unistd.h> 

#define num_blocks 16

// File type definitions
#define FILE_TYPE_TXT 1
#define FILE_TYPE_CONFIG 2
#define FILE_TYPE_ASM 3
#define FILE_TYPE_INVALID -1

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
    //Set select to 0
    //try to Read status register (RDSR)
    //if failed; tell user
    //tell user we are trying again
    //if mounted; set mount to 1
    //tell user file system is mounted
}

/* Store all meta data onto the EEPROM */

/////////////////////// Create Files /////////////////////////////////

void create_file(char name[12], uint8_t type, uint16_t address){
//File name, File type, File address (offset)


//Check if file type with name already exists
//Ask if write wants to overwrite
//If yes remove inode pointer, update blockmap vacancy, update table
//If no exit from prompt

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
    
    printf("Reading file '%s' of type '%s' (value: %d)\n", name, type_str, type_value);
    
    // Call your read_file function with the validated type
    read_file(name, type_value);
}

//////////////////////////////////////// LIST FILES ////////////////////////////////

void list_files(){ //list files inside Inode Table
    //Assuming we are mounted
    //Read names of all files in Inode table
    //Read types of files in Inode table
    //Print out file names with file type extension
}

///////////////////////// System functions ///////////////////////////////////////
void write(){ //This function will contain SPI processes (refer to page 17 for registers)
    //Turn off write protect
    //Write enable instruction
    


    //Turn on write protect
}

void read(){ //This function will contain SPI processes
    //Take user input
    scanf("");
    //Search inode table if file exists
    //If file doesn't exist


}

void help(){
    printf("Available commands: write, read, ls, help, quit");
}

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
            printf("ERROR: Please enter a valid command. Type help for list of commands.");
        }
    }
}