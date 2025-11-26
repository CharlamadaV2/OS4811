#include <crc.h>
#include <stdio.h>

/*
Reference(s):
  [1] https://www.youtube.com/watch?v=6gbkoFciryA

*/
//Figuring out how to create the crc algorithm for storing meta data into buffer
//The whole point of this is error detection

//The buffer will encrypt
//The crc for the superblock will encrypt: magic, version, total blocks, rsvd blocks, and block size
crc_metadata metadata;

void encrypt(){
  // We have a buffer 
  // Send k bits to the buffer
  // Frame contains n bits (n > k)
  // Frame check sequence (FCS) n-k bits
  // We have a key, the FCS, 
  // Given the data we want to send to the buffer is k bits
  metadata.encrypted_data = metadata.sender + metadata.FCS;
  // We append the 0 FCS-bits to the LSM, shifting the original bits left
  //Then use Modulo 2 Arithmetic (XOR) dividing our value with the pattern
  //The remainder from the MOD2 arthimetic is the value of the FCS
  // Depending on the result, we might need to pad the value to the left to match the FCS bits

}

void decrypt(){




}

void store_buffer(){
//Store every 8 bits


}

//What is expected to come out of this is to have 
// Using the same data we wanted to send to the buffer
// We append the actual FCS to the LSM of the data
// The expectation is that when we do MOD2 with the same pattern
// The remainder of the MOD2 should be all 000.

//To be aware, in extreme cases this algorithm can still fail   