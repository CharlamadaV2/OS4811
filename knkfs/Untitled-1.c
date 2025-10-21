#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define DISK_SIZE       (1024 * 1024)   // 1 MB disk
#define BLOCK_SIZE      512             // bytes per block
#define NUM_BLOCKS      (DISK_SIZE / BLOCK_SIZE)

// Layout sizes (in bytes)
#define RESERVED_SIZE   4096
#define SUPERBLOCK_SIZE 512
#define BITMAP_SIZE     512

// Derived layout offsets
#define SUPERBLOCK_OFFSET RESERVED_SIZE
#define BITMAP_OFFSET     (SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE)
#define DATA_OFFSET       (BITMAP_OFFSET + BITMAP_SIZE)

// Disk simulation
unsigned char disk[DISK_SIZE];
unsigned char free_block_bitmap[NUM_BLOCKS / 8];

// ------------------ Helper Functions ------------------

void write_to_disk(int offset, const void *src, size_t size) {
    memcpy(&disk[offset], src, size);
}

void read_from_disk(int offset, void *dst, size_t size) {
    memcpy(dst, &disk[offset], size);
}

// Bitmap helpers
void set_block_used(int block_index) {
    free_block_bitmap[block_index / 8] |= (1 << (block_index % 8));
}

void set_block_free(int block_index) {
    free_block_bitmap[block_index / 8] &= ~(1 << (block_index % 8));
}

bool is_block_free(int block_index) {
    return !(free_block_bitmap[block_index / 8] & (1 << (block_index % 8)));
}

int find_free_block() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (is_block_free(i)) {
            set_block_used(i);
            return i;
        }
    }
    return -1; // No free blocks
}

// ------------------ Initialization ------------------

void init_reserved_area() {
    for (int i = 0; i < RESERVED_SIZE; i++)
        disk[i] = 0xFF; // Just mark reserved bytes
}

void init_free_block_bitmap() {
    memset(free_block_bitmap, 0x00, sizeof(free_block_bitmap));
    // Reserve metadata blocks (first few blocks)
    int reserved_blocks = (DATA_OFFSET / BLOCK_SIZE);
    for (int i = 0; i < reserved_blocks; i++)
        set_block_used(i);
}

void init_superblock() {
    char superblock_data[] = "TinyFS v1.0 - 1MB Disk, 512B Blocks";
    write_to_disk(SUPERBLOCK_OFFSET, superblock_data, sizeof(superblock_data));
}

void init_fs() {
    printf("Initializing Tiny File System...\n");
    init_reserved_area();
    init_free_block_bitmap();
    init_superblock();
    printf("File system initialized.\n");
}

// ------------------ File Allocation Example ------------------

int allocate_block_for_file(const char *data) {
    int block = find_free_block();
    if (block == -1) {
        printf("No free blocks available!\n");
        return -1;
    }

    int offset = block * BLOCK_SIZE;
    write_to_disk(offset, data, strlen(data) + 1);
    printf("Data written to block %d (offset %d)\n", block, offset);
    return block;
}
