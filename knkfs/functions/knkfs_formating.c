#include <knkfs_indexarea.h>
#include <knkfs_superblock.h>
#include <knkfs.h>
#include <read_disk.h> //Functions to read and validate disk

#include <errno.h> //For error catching
#include <pthread.h> //For threading and fork
#include <unistd.h> //For writing to disk
#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>


//This will format the file system inside of the raw disk
// Read the superblock

// Allocate memory in raw disk based on meta data found in superblock

// Assuming the disk is already partitioned
// Open raw disk

// Create superblock at 0x00
    // Overwrite certain locations for fixed value
    
// Update superblock
    //After buffer validation
    //Fill superblock with meta data
    //Data should be structured as a hashmap

// Create Reserved Area
    // Overwrite certain locations for fixed value
    // Store meta data in buffer

// Create Data Area: Location for unused and used data addresses
    // Overwrite certain locations for fixed value
    // Store meta data in buffer

// Create Index Area
    // Overwrite certain locations for fixed value
    // Store meta data in buffer

// Meta data buffer for superblock
// The buffer will be used for validation
// Superblock buffer
    //Meta data list for location for all areas
    //Hash map write

int main() {
    char *disk_address = get_disk(); 

    //Parent Process
        //Create superblock in raw disk
        //Fork()

    //Child Process
        //Thread for Reserved Area
            //Create area
            //Send meta data for 
        //Thread for Data Area
        //Thread for Free Area
        //Thread for Index Area

    //After Child Process finish running
    //Take buffer from child process
    //Validate the meta data
    //If validate:
        //Write hashmap with buffer information
    //Else catch exception
        //Clear all formatting

    return 0;    
}
