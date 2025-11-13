#include <stdlib.h>
#include <stdint.h>   // for uint values
#include <string.h>   // for memset, strlen, strcpy
#include <EEPROM.h>   // EEPROM library

/*Description:
All numbers in this file is purely for testing purposes
This program will store the file system process on computer memory
This program will create the hello world file in the EEPROM, when user wants to write
After the user can read the hello world file by writing read in the serial monitor
*/

//The eeprom on the Arduino Mega is 4kb 
#define block_size 32 //bytes

typedef struct { //32 byte block
    char name[12];               // File name (up to 11 chars + null)
    uint32_t size;               // File size in bytes
    uint8_t direct_blocks[16];   // Direct block pointers (up to 16 blocks)
    //This means a file can be as large as 16 blocks
} inode;

// The block map tells the user which blocks are free or not
typedef struct {
    uint8_t bitmap[16];
    // Each bit represents an available block (0 = free, 1 = used)
    // 1 byte = 8 bits, 128 bytes / 8 = 16
    // 16 block rows w/ each bit representing a block
} block_map;

typedef struct {
    inode inodes[16]; //16 inodes for the blocks
} inode_table;  // Fixed: Added semicolon

//Creates bm and it in memory
block_map bm;
inode_table it;

void update_bm() { // Update bit map
    //For testing purpose this is blank
    Serial.println("Bitmap updated.");
}

void update_it() { // Update inode table
    //For testing purpose this is blank
    Serial.println("Inode table updated.");
}

void write_hw() { //Write hw file in first free block (inode 0)
    int free_block = -1;
    for (int i = 0; i < 128; i++) {  // 128 blocks total
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = i % 8;
        if ((bm.bitmap[byte_idx] & (1u << bit_idx)) == 0) {
            free_block = i;
            break;
        }
    }

    if (free_block == -1) {
        Serial.println("No free blocks!");
        return;
    }

    //Create hello world file
    const char* hello_str = "Hello, World!";  // 13 chars
    uint32_t file_size = strlen(hello_str);   // 13 bytes
    uint8_t data_block[block_size];           // Full block buffer
    memset(data_block, 0, block_size);        // Create data block
    memcpy(data_block, hello_str, file_size); // Copy string

    // Write to block address
    int data_addr = free_block * block_size;
    for (int j = 0; j < block_size; j++) {
        EEPROM.write(data_addr + j, data_block[j]);
    }

    //Update block map at the address
    uint8_t byte_idx = free_block / 8;
    uint8_t bit_idx = free_block % 8;
    bm.bitmap[byte_idx] |= (1u << bit_idx); //Bit wise or to set bit in bit map as used
    update_bm();  // Call to log, currently does nothing

    //Update inode table (set name, size, block)
    strcpy(it.inodes[0].name, "hello.txt");   // Use name field
    it.inodes[0].size = file_size;
    memset(it.inodes[0].direct_blocks, 0, sizeof(it.inodes[0].direct_blocks));  // Clear
    it.inodes[0].direct_blocks[0] = (uint8_t)free_block;
    update_it();  // Call to log, also does nothing
    
    //Debugging
    Serial.print("Created '");
    Serial.print(it.inodes[0].name);
    Serial.print("' (");
    Serial.print(file_size);
    Serial.print(" bytes) in block ");
    Serial.print(free_block);
    Serial.println(".");
}

void read() { 
    //Reads value at inode[0]
    Serial.println("Demo: Reading from inode 0:");

    if (it.inodes[0].size == 0) {
        Serial.println("No file data in inode 0");
        return;
    }

    //Find in the inode table where the file is
    uint8_t block_num = it.inodes[0].direct_blocks[0];
    int block_addr = block_num * block_size;
    uint32_t file_size = it.inodes[0].size;

    //Debugging
    Serial.print("File: '");
    Serial.print(it.inodes[0].name);
    Serial.print("', Size: ");
    Serial.print(file_size);
    Serial.print(" bytes, Block: ");
    Serial.println(block_num);

    // Had LLSM help me understand this process
    // Read full block and print as string (null-padded)
    char buf[block_size + 1];  // +1 for safe null
    for (int j = 0; j < block_size; j++) {
        buf[j] = EEPROM.read(block_addr + j); //Reads the bytes stored in that address
    }
    buf[file_size] = '\0';  // ends the read
    Serial.print("Content: ");
    Serial.println(buf);
    //To here

    // HEX dump for verification
    Serial.print("HEX (first ");
    Serial.print(min(32, (int)file_size));
    Serial.print(" bytes): ");
    for (int j = 0; j < min(32, (int)file_size); j++) {
        Serial.print((uint8_t)buf[j], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void list_files() {  // List all inodes with names/sizes
    Serial.println("Files in inode table:");
    for (int i = 0; i < 16; i++) {
        if (it.inodes[i].size > 0 && it.inodes[i].name[0] != '\0') {
            Serial.print("  ");
            Serial.print(it.inodes[i].name);
            Serial.print(" (");
            Serial.print(it.inodes[i].size);
            Serial.println(" bytes)");
        }
    }
    if (it.inodes[0].size == 0) Serial.println("  (No files)");
    Serial.println();
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {}  // Wait for serial connection

    //Check EEPROM size
    uint16_t eeprom_size = EEPROM.length();
    Serial.print("The EEPROM is ");
    Serial.print(eeprom_size);
    Serial.println(" bytes long.");

    //How many blocks can fit inside the EEPROM (1 block = 32 bytes)
    uint16_t num_blocks = eeprom_size / block_size;
    Serial.print("Number of blocks we can fit is ");
    Serial.print(num_blocks);
    Serial.println();  // Fixed: Added println

    //Create an empty inode table and block map
    memset(&bm, 0, sizeof(bm));
    memset(&it, 0, sizeof(it));

    Serial.println("File system initialized.");
    Serial.println("Commands: 'write', 'read', 'ls' (list files)");
}

void loop() {
    //Check if user wants to write to eeprom
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input == "write") {
            write_hw();
        } else if (input == "read") {
            read();
        } else if (input == "ls") {
            list_files();
        } else {
            Serial.println("Unknown: Use 'write', 'read', or 'ls'");
        }
    }
    delay(500);
}