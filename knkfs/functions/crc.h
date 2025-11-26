#include <stdint.h>
#include <stdio.h>

uint32_t pattern = 0b011011011;

typedef struct{
    //Sender Data
    uint8_t sender[18];
    uint8_t FCS; //Frame Check Sequencer
    uint8_t encrypted_data[18];
} crc_metadata;