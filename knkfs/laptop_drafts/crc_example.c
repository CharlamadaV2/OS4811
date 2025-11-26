#include <crc.h>  // Assuming this provides any necessary CRC primitives; if not, we'll implement inline
#include <stdio.h>
#include <stdint.h>  // For fixed-width types like uint32_t
#include <string.h>  // For memcpy, etc.

/*
Reference(s):
  [1] https://www.youtube.com/watch?v=6gbkoFciryA

//Figuring out how to create the crc algorithm for storing meta data into buffer
//The whole point of this is error detection

//The buffer will encrypt
//The crc for the superblock will encrypt: magic, version, total blocks, rsvd blocks, and block size

// Assumptions for completion (updated for CRC-16):
// - Using CRC-16-CCITT for lighter checksum (16 bits vs. 32).
// - Generator polynomial: 0x1021 (non-reflected, common for metadata).
// - Data is treated as a uint32_t for simplicity (e.g., packed metadata fields; 32 bits).
// - FCS is 16 bits (n - k = 16).
// - "Encrypt" here means framing with CRC (not true encryption).
// - Buffer storage is byte-oriented (8-bit chunks).
// - Decrypt checks integrity by recomputing CRC on full frame.
*/

// Define the metadata struct based on comments (adjust sizes as needed)
typedef struct {
    uint32_t magic;          // e.g., 0xDEADBEEF
    uint16_t version;        // e.g., 1
    uint32_t total_blocks;   // e.g., 1024
    uint32_t rsvd_blocks;    // e.g., 64
    uint32_t block_size;     // e.g., 4096
    uint32_t sender;         // Original data (packed metadata or placeholder; uint32_t for demo, but CRC-16 covers it)
    uint16_t fcs;            // Frame Check Sequence (CRC-16)
    uint32_t encrypted_data; // Framed data: sender (32 bits) + FCS (16 bits); high 16 bits unused/padded
} crc_metadata;

// CRC-16 implementation (bit-by-bit for simplicity; init 0x0000 for CCITT)
uint16_t compute_crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0x0000;  // Initial value for CRC-16-CCITT
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;  // MSB-first
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;  // Polynomial XOR
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;  // No final XOR for this variant
}

// Pack metadata fields into a sender uint32_t (for demo; in reality, serialize to buffer)
uint32_t pack_metadata(const crc_metadata *meta) {
    uint32_t packed = 0;
    packed |= (meta->magic & 0xFF) << 24;
    packed |= (meta->version & 0xFFFF) << 8;  // Overlap example; adjust as needed
    packed |= (meta->total_blocks & 0xFF);    // Simplified packing
    // Add rsvd_blocks, block_size similarly; this is a placeholder for k=32 bits
    return packed;
}

// Unpack would reverse this, but omitted for brevity

void encrypt(crc_metadata *metadata) {
    // Pack the metadata fields into sender (original data, k=32 bits)
    metadata->sender = pack_metadata(metadata);

    // Serialize sender to bytes for CRC computation
    uint8_t data[4];
    data[0] = (metadata->sender >> 24) & 0xFF;
    data[1] = (metadata->sender >> 16) & 0xFF;
    data[2] = (metadata->sender >> 8) & 0xFF;
    data[3] = metadata->sender & 0xFF;

    // Compute FCS: CRC-16 of data (standard append)
    metadata->fcs = compute_crc16(data, 4);

    // Form frame: sender (32 bits) + FCS (16 bits) = 48 bits (padded to uint32_t, high 16 bits 0)
    metadata->encrypted_data = (metadata->sender << 16) | metadata->fcs;

    // Note: In mod-2 division, appending FCS makes full frame divisible by generator (remainder 0)
    printf("Encryption complete. FCS: 0x%04X\n", metadata->fcs);
}

int decrypt(crc_metadata *metadata) {
    // Extract sender and received FCS from frame
    uint32_t received_sender = metadata->encrypted_data >> 16;
    uint16_t received_fcs = metadata->encrypted_data & 0xFFFF;

    // Serialize received_sender to bytes
    uint8_t data[4];
    data[0] = (received_sender >> 24) & 0xFF;
    data[1] = (received_sender >> 16) & 0xFF;
    data[2] = (received_sender >> 8) & 0xFF;
    data[3] = received_sender & 0xFF;

    // To verify: Append received FCS and compute CRC on full frame; should be 0 if valid
    uint8_t full_frame[6];
    memcpy(full_frame, data, 4);
    full_frame[4] = (received_fcs >> 8) & 0xFF;
    full_frame[5] = received_fcs & 0xFF;

    uint16_t computed_crc = compute_crc16(full_frame, 6);
    if (computed_crc == 0) {  // Remainder 0: valid (no errors)
        metadata->sender = received_sender;  // Restore
        printf("Decryption successful: No errors detected.\n");
        return 1;  // Valid
    } else {
        printf("Decryption failed: Errors detected (computed CRC: 0x%04X).\n", computed_crc);
        return 0;  // Invalid
    }
}

void store_buffer(crc_metadata *metadata) {
    // Store every 8 bits (bytes) of the encrypted_data into a conceptual buffer
    // For demo, print hex bytes; in reality, write to file/block device
    // Note: Only 6 bytes used (48 bits); last 2 bytes of uint32_t are padding
    uint8_t *buf = (uint8_t *)&metadata->encrypted_data;
    printf("Storing buffer (8-bit chunks):\n");
    for (int i = 0; i < 6; ++i) {  // 48 bits = 6 bytes
        printf("Byte %d: 0x%02X\n", i, buf[i]);
        // e.g., fwrite(&buf[i], 1, 1, file_ptr); for actual storage
    }
    // Note: In extreme cases (e.g., burst errors longer than FCS), detection may fail
}

// Example usage/demo
int main() {
    crc_metadata meta = {0};
    meta.magic = 0xDEADBEEF;
    meta.version = 1;
    meta.total_blocks = 1024;
    meta.rsvd_blocks = 64;
    meta.block_size = 4096;

    encrypt(&meta);
    store_buffer(&meta);

    // Simulate no error
    if (decrypt(&meta)) {
        // Use meta.sender safely
        printf("Restored sender: 0x%08X\n", meta.sender);
    }

    // Simulate error: Flip a bit in FCS
    meta.encrypted_data ^= 0x00010000U;  // Flip bit in FCS
    decrypt(&meta);  // Should detect error

    return 0;
}