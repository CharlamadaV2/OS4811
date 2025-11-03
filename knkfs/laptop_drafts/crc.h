#include <stdint.h>
#include <stdio.h>

uint32_t pattern = 0b01101101;

typedef struct{
    //Sender Data
    uint64_t sender;
    uint32_t FCS;
    uint64_t encrypted_data;
} crc_metadata;