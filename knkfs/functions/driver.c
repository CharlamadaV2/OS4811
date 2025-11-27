//This code was written by Kimson Lam

//Description:
/*
The purpose of this code is to give functionality like read, write (edit), ls, delete, rename
Write will use an algorithm to find unused blocks with priority over least used blocks

Note: All structures, variables, and headers used will be stored in the formatter header.
*/

#include <stdint.h> // for uint values
#include <stdio.h>
#include <string.h>   // for memset, strlen, strcpy 
#include <linux/spi/spidev.h> // functions for SPI devices (for EEPROM)

//On driver start up it should set chip select to low 
