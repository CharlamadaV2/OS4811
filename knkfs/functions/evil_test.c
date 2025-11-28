// TODO: Figure out block size
// Using 512 bytes as block size (same as page size)

// TODO: Figure out how many blocks will be used to store data
// Total EEPROM: 32 Mbits = 4 MB = 4096 KB
// With 512 byte blocks: 4096 * 1024 / 512 = 8192 blocks
// Reserve some for metadata:
// - 1 block for superblock
// - 1 block for block map
// - 16 blocks for inode table (assuming max 256 files)
// Total reserved: 18 blocks
// Available for data: 8192 - 18 = 8174 blocks

// TODO: Figure out how to use SPI (partially implemented)
// Added EEPROM-specific commands

// TODO: Figure out how to make wear leveling algorithm
// Implemented in find_best_block function

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <time.h>

// EEPROM constants from doc
#define EEPROM_SIZE_BYTES (32 * 1024 * 1024 / 8)  // 32 Mbits = 4 MB
#define BLOCK_SIZE 512  // Page size
#define TOTAL_BLOCKS (EEPROM_SIZE_BYTES / BLOCK_SIZE)  // 8192 blocks
#define DATA_BLOCKS (TOTAL_BLOCKS - 18)  // 8174 blocks available for data
#define INODE_TABLE_BLOCKS 16  // For up to 256 inodes (16 inodes per block)
#define MAX_INODES (INODE_TABLE_BLOCKS * (BLOCK_SIZE / sizeof(inode)))

// EEPROM commands from doc
#define EEPROM_CMD_WRITE_ENABLE 0x06
#define EEPROM_CMD_WRITE_DISABLE 0x04
#define EEPROM_CMD_READ 0x03
#define EEPROM_CMD_WRITE 0x02
#define EEPROM_CMD_READ_STATUS 0x05
#define EEPROM_CMD_WRITE_STATUS 0x01
#define EEPROM_CMD_SECTOR_ERASE 0x20  // 4KB sector erase
#define EEPROM_CMD_BLOCK_ERASE_32K 0x52
#define EEPROM_CMD_BLOCK_ERASE_64K 0xD8
#define EEPROM_CMD_CHIP_ERASE 0xC7

// File system layout
#define SUPERBLOCK_ADDR 0
#define BLOCK_MAP_ADDR BLOCK_SIZE
#define INODE_TABLE_ADDR (2 * BLOCK_SIZE)
#define DATA_START_ADDR ((2 + INODE_TABLE_BLOCKS) * BLOCK_SIZE)

// File type definitions
#define FILE_TYPE_TXT 1
#define FILE_TYPE_CONFIG 2
#define FILE_TYPE_ASM 3
#define FILE_TYPE_INVALID -1

static int spi_fd = -1;

// The block map tells the user which blocks are free or not
typedef struct {
    uint8_t bitmap[DATA_BLOCKS / 8];  // One bit per data block
    uint32_t wear_level[DATA_BLOCKS];  // Wear level for each block
} block_map;

typedef struct superblock_properties {
    int64_t time_stamp;
    uint64_t data_size;  // Size of Data Area in blocks
    uint64_t index_size;  // Size of Index Area in bytes
    uint8_t magic[3];  // 'SFS' (0x53, 0x46, 0x53)
    uint8_t version;  // 0x10 = 1.0
    uint64_t total_blocks;  // Total number of blocks
    uint32_t rsvd_blocks;  // Number of reserved blocks
    uint8_t blk_size;  // log2(block_size) - 7 = 2 for 512 bytes
    uint8_t crc;  // CRC of bytes above
} superblock;

typedef struct inode {
    char name[12];               // File name (up to 11 chars + null)
    uint32_t size;               // File size in bytes
    uint8_t type;                // File type
    uint8_t direct_blocks[16];   // Direct block pointers
    uint8_t indirect_block;      // Indirect block pointer
    uint8_t double_indirect;     // Double indirect block pointer
    uint8_t reserved[6];         // Reserved for future use
} inode;

typedef struct {
    uint8_t blocks[BLOCK_SIZE / sizeof(uint32_t)];  // Block pointers
} indirect_block;

// Data structures for type map
typedef struct {
    char *type_name;
    uint8_t type_value;
} TypeEntry;

TypeEntry type_map[] = {
    {"txt", FILE_TYPE_TXT},
    {"config", FILE_TYPE_CONFIG},
    {"asm", FILE_TYPE_ASM},
    {NULL, 0}  // Sentinel value
};

// Global variables
block_map fs_block_map;
inode inode_table[MAX_INODES];
int next_free_inode = 0;

/////////////////////////// Functions ////////////////////////////////

void mount_fs() {
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        exit(1);
    }

    uint8_t mode = SPI_MODE_0;  // 0 CPOL, 0 CPHA
    uint8_t bits = 8;
    uint32_t speed = 80000000;  // 80 MHz

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");
        exit(1);
    }

    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    printf("SPI mounted successfully\n");
    
    // Initialize file system if needed
    init_filesystem();
}

void init_filesystem() {
    // Read superblock to check if filesystem exists
    superblock sb;
    eeprom_read(SUPERBLOCK_ADDR, (uint8_t*)&sb, sizeof(superblock));
    
    // Check if filesystem is already initialized
    if (sb.magic[0] == 'S' && sb.magic[1] == 'F' && sb.magic[2] == 'S') {
        printf("Filesystem found\n");
        
        // Load block map
        eeprom_read(BLOCK_MAP_ADDR, (uint8_t*)&fs_block_map, sizeof(block_map));
        
        // Load inode table
        for (int i = 0; i < INODE_TABLE_BLOCKS; i++) {
            eeprom_read(INODE_TABLE_ADDR + i * BLOCK_SIZE, 
                       (uint8_t*)&inode_table[i * (BLOCK_SIZE / sizeof(inode))], 
                       BLOCK_SIZE);
        }
        
        // Find next free inode
        for (int i = 0; i < MAX_INODES; i++) {
            if (inode_table[i].name[0] == '\0') {
                next_free_inode = i;
                break;
            }
        }
    } else {
        printf("Initializing new filesystem\n");
        
        // Initialize superblock
        memset(&sb, 0, sizeof(superblock));
        sb.magic[0] = 'S';
        sb.magic[1] = 'F';
        sb.magic[2] = 'S';
        sb.version = 0x10;
        sb.total_blocks = TOTAL_BLOCKS;
        sb.rsvd_blocks = 2 + INODE_TABLE_BLOCKS;
        sb.blk_size = 2;  // log2(512) - 7 = 2
        sb.data_size = DATA_BLOCKS;
        sb.index_size = INODE_TABLE_BLOCKS * BLOCK_SIZE;
        sb.time_stamp = time(NULL);
        
        // Calculate CRC
        uint8_t crc = 0;
        for (int i = 0; i < sizeof(superblock) - 1; i++) {
            crc ^= ((uint8_t*)&sb)[i];
        }
        sb.crc = crc;
        
        // Write superblock
        eeprom_write(SUPERBLOCK_ADDR, (uint8_t*)&sb, sizeof(superblock));
        
        // Initialize block map
        memset(&fs_block_map, 0, sizeof(block_map));
        eeprom_write(BLOCK_MAP_ADDR, (uint8_t*)&fs_block_map, sizeof(block_map));
        
        // Initialize inode table
        memset(inode_table, 0, sizeof(inode_table));
        for (int i = 0; i < INODE_TABLE_BLOCKS; i++) {
            eeprom_write(INODE_TABLE_ADDR + i * BLOCK_SIZE, 
                        (uint8_t*)&inode_table[i * (BLOCK_SIZE / sizeof(inode))], 
                        BLOCK_SIZE);
        }
        
        next_free_inode = 0;
    }
}

void save_filesystem_state() {
    // Update superblock timestamp
    superblock sb;
    eeprom_read(SUPERBLOCK_ADDR, (uint8_t*)&sb, sizeof(superblock));
    sb.time_stamp = time(NULL);
    
    // Recalculate CRC
    uint8_t crc = 0;
    for (int i = 0; i < sizeof(superblock) - 1; i++) {
        crc ^= ((uint8_t*)&sb)[i];
    }
    sb.crc = crc;
    
    eeprom_write(SUPERBLOCK_ADDR, (uint8_t*)&sb, sizeof(superblock));
    
    // Save block map
    eeprom_write(BLOCK_MAP_ADDR, (uint8_t*)&fs_block_map, sizeof(block_map));
    
    // Save inode table
    for (int i = 0; i < INODE_TABLE_BLOCKS; i++) {
        eeprom_write(INODE_TABLE_ADDR + i * BLOCK_SIZE, 
                    (uint8_t*)&inode_table[i * (BLOCK_SIZE / sizeof(inode))], 
                    BLOCK_SIZE);
    }
}

void help() {
    printf("Available commands:\n");
    printf("  write - Create a new file\n");
    printf("  read  - Read a file\n");
    printf("  ls    - List all files\n");
    printf("  help  - Show this help message\n");
    printf("  quit  - Exit the program\n");
}

// Find the best block for writing based on wear leveling
uint32_t find_best_block() {
    uint32_t best_block = 0;
    uint32_t min_wear = UINT32_MAX;
    
    for (uint32_t i = 0; i < DATA_BLOCKS; i++) {
        // Check if block is free
        uint8_t byte = fs_block_map.bitmap[i / 8];
        uint8_t mask = 1 << (i % 8);
        
        if (!(byte & mask)) {  // Block is free
            if (fs_block_map.wear_level[i] < min_wear) {
                min_wear = fs_block_map.wear_level[i];
                best_block = i;
                
                // If we found a block with 0 wear, we can stop searching
                if (min_wear == 0) {
                    break;
                }
            }
        }
    }
    
    return best_block;
}

// Mark a block as used in the block map
void mark_block_used(uint32_t block_num) {
    uint8_t byte = fs_block_map.bitmap[block_num / 8];
    uint8_t mask = 1 << (block_num % 8);
    fs_block_map.bitmap[block_num / 8] = byte | mask;
    
    // Increment wear level
    fs_block_map.wear_level[block_num]++;
}

// Mark a block as free in the block map
void mark_block_free(uint32_t block_num) {
    uint8_t byte = fs_block_map.bitmap[block_num / 8];
    uint8_t mask = 1 << (block_num % 8);
    fs_block_map.bitmap[block_num / 8] = byte & ~mask;
}

// Erase a sector (4KB)
void erase_sector(uint32_t address) {
    uint8_t cmd_wren = EEPROM_CMD_WRITE_ENABLE;
    spi_transfer(&cmd_wren, NULL, 1);
    
    uint8_t cmd[4];
    cmd[0] = EEPROM_CMD_SECTOR_ERASE;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;
    
    spi_transfer(cmd, NULL, 4);
    
    // Wait for erase to complete (poll status register)
    uint8_t status;
    do {
        eeprom_read_status(&status);
    } while (status & 0x01);  // Check WIP (Write In Progress) bit
}

// Read the status register
void eeprom_read_status(uint8_t *status) {
    uint8_t cmd = EEPROM_CMD_READ_STATUS;
    spi_transfer(&cmd, status, 2);
}

/////////////////////// Create Files /////////////////////////////////
void write_command() {
    printf("Enter file name: ");
    char name[12];
    scanf("%11s", name);
    
    printf("Enter file type (txt, config, asm): ");
    char type_str[20];
    scanf("%19s", type_str);
    
    // Convert type string to value
    int8_t type_value = get_type_value(type_str);
    
    // Check if conversion was successful
    if (type_value == FILE_TYPE_INVALID) {
        return;
    }
    
    printf("Enter file content (end with empty line):\n");
    
    // Read file content
    char content[4096];  // Max 4KB for simplicity
    int content_len = 0;
    char line[256];
    
    // Clear stdin buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    while (fgets(line, sizeof(line), stdin)) {
        if (line[0] == '\n') {
            break;
        }
        
        int line_len = strlen(line);
        if (content_len + line_len < sizeof(content)) {
            strcpy(content + content_len, line);
            content_len += line_len;
        } else {
            printf("File too large, truncating\n");
            break;
        }
    }
    
    printf("Creating file '%s' of type '%s'\n", name, type_str);
    
    // Call create_file with the content
    create_file(name, type_value, (uint8_t*)content, content_len);
}

void create_file(char name[12], uint8_t type, uint8_t *content, uint32_t content_len) {
    // Check if file already exists
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].name[0] != '\0' && strcmp(inode_table[i].name, name) == 0) {
            printf("File '%s' already exists. Overwrite? (y/n): ", name);
            char response;
            scanf(" %c", &response);
            
            if (response != 'y' && response != 'Y') {
                printf("File creation cancelled\n");
                return;
            }
            
            // Free existing file's blocks
            inode *existing = &inode_table[i];
            for (int j = 0; j < 16; j++) {
                if (existing->direct_blocks[j] != 0) {
                    mark_block_free(existing->direct_blocks[j] - 1);
                }
            }
            
            // Handle indirect blocks if needed
            if (existing->indirect_block != 0) {
                indirect_block ind;
                eeprom_read(DATA_START_ADDR + (existing->indirect_block - 1) * BLOCK_SIZE, 
                           (uint8_t*)&ind, sizeof(indirect_block));
                
                for (int j = 0; j < BLOCK_SIZE / sizeof(uint32_t); j++) {
                    if (ind.blocks[j] != 0) {
                        mark_block_free(ind.blocks[j] - 1);
                    }
                }
                
                mark_block_free(existing->indirect_block - 1);
            }
            
            // Reset inode
            memset(existing, 0, sizeof(inode));
            break;
        }
    }
    
    // Find a free inode
    if (next_free_inode >= MAX_INODES) {
        printf("No free inodes available\n");
        return;
    }
    
    inode *new_file = &inode_table[next_free_inode];
    
    // Initialize inode
    strncpy(new_file->name, name, 11);
    new_file->name[11] = '\0';
    new_file->type = type;
    new_file->size = content_len;
    
    // Calculate how many blocks are needed
    uint32_t blocks_needed = (content_len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Allocate blocks and write data
    uint32_t bytes_written = 0;
    
    // Direct blocks
    for (int i = 0; i < 16 && i < blocks_needed; i++) {
        uint32_t block_num = find_best_block();
        
        if (block_num >= DATA_BLOCKS) {
            printf("No free blocks available\n");
            return;
        }
        
        // Mark block as used
        mark_block_used(block_num);
        
        // Store block number in inode (add 1 to distinguish from 0 = unused)
        new_file->direct_blocks[i] = block_num + 1;
        
        // Write data to block
        uint32_t write_size = BLOCK_SIZE;
        if (bytes_written + write_size > content_len) {
            write_size = content_len - bytes_written;
        }
        
        eeprom_write(DATA_START_ADDR + block_num * BLOCK_SIZE, 
                    content + bytes_written, write_size);
        
        bytes_written += write_size;
    }
    
    // Indirect blocks if needed
    if (blocks_needed > 16) {
        uint32_t indirect_block_num = find_best_block();
        
        if (indirect_block_num >= DATA_BLOCKS) {
            printf("No free blocks available for indirect block\n");
            return;
        }
        
        mark_block_used(indirect_block_num);
        new_file->indirect_block = indirect_block_num + 1;
        
        indirect_block ind;
        memset(&ind, 0, sizeof(indirect_block));
        
        uint32_t remaining_blocks = blocks_needed - 16;
        for (int i = 0; i < remaining_blocks && i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            uint32_t block_num = find_best_block();
            
            if (block_num >= DATA_BLOCKS) {
                printf("No free blocks available\n");
                return;
            }
            
            mark_block_used(block_num);
            ind.blocks[i] = block_num + 1;
            
            // Write data to block
            uint32_t write_size = BLOCK_SIZE;
            if (bytes_written + write_size > content_len) {
                write_size = content_len - bytes_written;
            }
            
            eeprom_write(DATA_START_ADDR + block_num * BLOCK_SIZE, 
                        content + bytes_written, write_size);
            
            bytes_written += write_size;
        }
        
        // Write indirect block
        eeprom_write(DATA_START_ADDR + indirect_block_num * BLOCK_SIZE, 
                    (uint8_t*)&ind, sizeof(indirect_block));
    }
    
    // Find next free inode for next time
    for (int i = next_free_inode + 1; i < MAX_INODES; i++) {
        if (inode_table[i].name[0] == '\0') {
            next_free_inode = i;
            break;
        }
    }
    
    // Save filesystem state
    save_filesystem_state();
    
    printf("File '%s' created successfully\n", name);
}

/////////////////////// Delete Function ////////////////////////////////////////////
void delete_file(){
    //Delete file pointers in inode table
    //Sets blockmap value to 0

}

/////////////////////// Read functions /////////////////////////////////////////////
void read_file(char name[12], uint8_t type) {
    // Find the file in the inode table
    int inode_idx = -1;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].name[0] != '\0' && 
            strcmp(inode_table[i].name, name) == 0 && 
            inode_table[i].type == type) {
            inode_idx = i;
            break;
        }
    }
    
    if (inode_idx == -1) {
        printf("File '%s' not found\n", name);
        return;
    }
    
    inode *file = &inode_table[inode_idx];
    
    // Allocate buffer for file content
    uint8_t *content = malloc(file->size);
    if (!content) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Read direct blocks
    uint32_t bytes_read = 0;
    for (int i = 0; i < 16 && file->direct_blocks[i] != 0; i++) {
        uint32_t block_num = file->direct_blocks[i] - 1;
        uint32_t read_size = BLOCK_SIZE;
        
        if (bytes_read + read_size > file->size) {
            read_size = file->size - bytes_read;
        }
        
        eeprom_read(DATA_START_ADDR + block_num * BLOCK_SIZE, 
                   content + bytes_read, read_size);
        
        bytes_read += read_size;
    }
    
    // Read indirect blocks if needed
    if (file->indirect_block != 0 && bytes_read < file->size) {
        uint32_t block_num = file->indirect_block - 1;
        indirect_block ind;
        
        eeprom_read(DATA_START_ADDR + block_num * BLOCK_SIZE, 
                   (uint8_t*)&ind, sizeof(indirect_block));
        
        for (int i = 0; i < BLOCK_SIZE / sizeof(uint32_t) && ind.blocks[i] != 0; i++) {
            uint32_t data_block_num = ind.blocks[i] - 1;
            uint32_t read_size = BLOCK_SIZE;
            
            if (bytes_read + read_size > file->size) {
                read_size = file->size - bytes_read;
            }
            
            eeprom_read(DATA_START_ADDR + data_block_num * BLOCK_SIZE, 
                       content + bytes_read, read_size);
            
            bytes_read += read_size;
            
            if (bytes_read >= file->size) {
                break;
            }
        }
    }
    
    // Print file content
    printf("File '%s' content:\n", name);
    for (uint32_t i = 0; i < file->size; i++) {
        printf("%c", content[i]);
    }
    printf("\n");
    
    free(content);
}

int8_t get_type_value(const char *type_str) {
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
    scanf("%11s", name);
    
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

void list_files() {
    printf("Files in filesystem:\n");
    
    int file_count = 0;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].name[0] != '\0') {
            char *type_str = "unknown";
            
            switch (inode_table[i].type) {
                case FILE_TYPE_TXT:
                    type_str = "txt";
                    break;
                case FILE_TYPE_CONFIG:
                    type_str = "config";
                    break;
                case FILE_TYPE_ASM:
                    type_str = "asm";
                    break;
            }
            
            printf("  %s.%s (%d bytes)\n", inode_table[i].name, type_str, inode_table[i].size);
            file_count++;
        }
    }
    
    if (file_count == 0) {
        printf("  No files found\n");
    }
}

///////////////////////// System functions ///////////////////////////////////////
int spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
        .speed_hz = 8000000,
        .bits_per_word = 8,
        .cs_change = 0,
    };
    
    return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void eeprom_write(uint32_t address, uint8_t *data, size_t len) {
    // Check if write crosses page boundary
    uint32_t page_start = address & ~(BLOCK_SIZE - 1);
    uint32_t page_end = page_start + BLOCK_SIZE;
    
    if (address + len > page_end) {
        // Split into multiple writes
        uint32_t first_len = page_end - address;
        eeprom_write(address, data, first_len);
        eeprom_write(page_end, data + first_len, len - first_len);
        return;
    }
    
    // Write Enable command
    uint8_t cmd_wren = EEPROM_CMD_WRITE_ENABLE;
    spi_transfer(&cmd_wren, NULL, 1);
    
    // Write command + 3 byte address + data
    uint8_t tx_buf[4 + len];
    tx_buf[0] = EEPROM_CMD_WRITE;
    tx_buf[1] = (address >> 16) & 0xFF;
    tx_buf[2] = (address >> 8) & 0xFF;
    tx_buf[3] = address & 0xFF;
    memcpy(&tx_buf[4], data, len);
    
    spi_transfer(tx_buf, NULL, 4 + len);
    
    // Wait for write to complete (poll status register)
    uint8_t status;
    do {
        eeprom_read_status(&status);
    } while (status & 0x01);  // Check WIP (Write In Progress) bit
}

void eeprom_read(uint32_t address, uint8_t *data, size_t len) {
    uint8_t tx_buf[4 + len];
    uint8_t rx_buf[4 + len];
    
    tx_buf[0] = EEPROM_CMD_READ;
    tx_buf[1] = (address >> 16) & 0xFF;
    tx_buf[2] = (address >> 8) & 0xFF;
    tx_buf[3] = address & 0xFF;
    memset(&tx_buf[4], 0, len);  // Dummy bytes for clock
    
    spi_transfer(tx_buf, rx_buf, 4 + len);
    
    memcpy(data, &rx_buf[4], len);
}

/////////////////////////// MAIN///////////////////////////////
int main() {
    mount_fs();
    
    while (1) {
        printf("\nEnter a command: ");
        char command[100];
        scanf("%99s", command);
        
        if (strcmp(command, "write") == 0) {
            write_command();
        } else if (strcmp(command, "read") == 0) {
            read_command();
        } else if (strcmp(command, "ls") == 0) {
            list_files();
        } else if (strcmp(command, "help") == 0) {
            help();
        } else if (strcmp(command, "quit") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("'%s': command not found\n", command);
            printf("Please enter a valid command. Type 'help' for list of commands.\n");
        }
    }
    
    close(spi_fd);
    return 0;
}