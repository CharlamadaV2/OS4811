#include <stdint.h>

//List of offsets for reserved for compatibility (using hex)
#define offset_boot_code1 0x0000
#define offset_bios 0x000B
#define offset_boot_code2 0x0020
#define offset_partition 0x01BE
#define offset_boot_signature 0x01FE

//List of offsets for superblock
#define offset_timestamp 0x0194
#define offset_data 0x019C //Data area
#define offset_index 0x01A4 //Index area

#define offset_magic 0x01AC // Magic number
#define offset_fs_version 0x01AF // SFS version number
#define offset_total_blocks 0x01B0
#define offset_reserved_blocks 0x01B8
#define offset_blocksize 0x01BC

#define offset_checksum 0x01BD

//Byte size for the 
typedef struct superblock_properties{
    int64_t time_stamp; // Time Stamp of when volume has changed.
    uint64_t data_size; // Size of Data Area in blocks
    uint64_t index_size; // Size of Index Area in bytes

    uint8_t magic[3]; // signature ‘SFS’ (0x534653)
    uint8_t version; // SFS version (0x10 = 1.0, 0x1A = 1.10)
    uint64_t total_blocks; // Total number of blocks in volume (including reserved area)
    uint32_t rsvd_blocks; // Number of reserved blocks
    uint8_t block_size; // log(x+7) of block size (x = 2 = 512)
    uint8_t crc; // Zero sum of bytes above 0b11111111 -> 0xFF
} superblock;


