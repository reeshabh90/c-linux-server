/**
 * 📔string_input_processing.0📔
 * @file: string_input_processing.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program processes an input string and splits into 16-byte blocks.
 *
 * 1. Why? Break the string into blocks of 128 bits = 16 bytes.
 * 2. If the last block (not 16 bytes) is smaller, apply padding (eg. PKCS#7 ).
 * 3. Padding Logic:
 * -- If the final block is n bytes (where n < 16), pad the remaining bytes with the value (16 - n).
 * -- If the block is already 16 bytes, add an entire new block with value 0x10 (i.e., 16).
 * --- Reason: To make it unambiguous how much padding was added during encryption, so it can be reliably removed during decryption.
 * --- Let’s say message is: "16_bytes_of_txt"  --> exactly 16 bytes
 * --- If we don’t add padding, the ciphertext would decrypt to exactly this,
 *      however, if the last byte happened to be 0x01,decryption logic might wrongly strip it off thinking it is padding.
 * --- If the message is exactly 16 bytes, a full new block of padding.
 * --- The padded message becomes: [ original 16 bytes ] + [ 16 bytes of 0x10 ]
 * --- On decryption, we see a final block of 16 × 0x10, and confidently remove it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 16

// Function to apply PKCS#7 padding
void apply_pkcs7_padding(unsigned char *block, int data_len)
{
    int padding_len = BLOCK_SIZE - data_len;
    for (int i = data_len; i < BLOCK_SIZE; i++)
    {
        // We added padding length as padding so that while decryption program knows exactly how many bytes were padded
        block[i] = (unsigned char)padding_len;
    }
}

/// @brief This function creates a padding over the input
/// @details The function performs following steps:
/// 1. Removes trailing new line
/// 2. Calculate number of blocks text need to be divided into, as AES works on 16 bytes (128 bits) block only.
/// 3. Last block is padded using pcks7 so that during decryption program knows when data has ended.
/// @param input string array
/// @return
void padding_function(char *input_array)
{
    size_t input_len = strlen(input_array);

    // Remove trailing newline from fgets
    if (input_array[input_len - 1] == '\n')
    {
        input_array[--input_len] = '\0';
    }

    // Calculate number of 16-byte blocks needed
    int num_blocks = input_len / BLOCK_SIZE;
    if (input_len % BLOCK_SIZE != 0)
    {
        num_blocks += 1;
    }

    printf("\nTotal blocks (with padding): %d\n", num_blocks);

    unsigned char *padded_data = (unsigned char *)calloc(num_blocks * BLOCK_SIZE, sizeof(unsigned char));
    memcpy(padded_data, input_array, input_len);

    // Apply PKCS#7 padding to the last block
    int last_block_len = input_len % BLOCK_SIZE;
    if (last_block_len == 0)
    {
        // Full block, add new block with padding
        // When decrypting, the program must know where the real data ends and padding begins.
        // If a 16-byte block is the exact length of the data,  to avoid ambiguity,
        // we add a full block of padding (e.g., 0x10 0x10 ... 0x10 for PKCS#7).
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            padded_data[input_len + i] = BLOCK_SIZE;
        }
        num_blocks += 1;
    }
    else
    {
        // `padded_data` points to the start of the full padded buffer
        // first argument points to the last 16-byte block (hence, num_blocks -1).
        apply_pkcs7_padding(padded_data + (num_blocks - 1) * BLOCK_SIZE, last_block_len);
    }

    // Display the blocks
    printf("\nPadded 16-byte blocks (in hex):\n");
    for (int i = 0; i < num_blocks; i++)
    {
        printf("Block %d: ", i + 1);
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%02X ", padded_data[i * BLOCK_SIZE + j]);
        }
        printf("\n");
    }

    free(padded_data);
}

/// @brief Main function of the program which processes a string input and divides in 16 blocks
/// @details Input string is explictly casted to `unsigned char*` and stored in `const unsigned char*`
/// 1. char is signed or unsigned (platform-dependent), but unsigned char guarantees values in the range 0–255.
/// 2. unsigned char* --> “this is a pointer to binary-safe byte data.”
/// @param argc
/// @param argv
/// @return
int main(int argc, char const *argv[])
{
    char input_array[1024];
    printf("Enter plaintext: ");
    fgets(input_array, sizeof(input_array), stdin);
    padding_function(input_array);
    return 0;
}
