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

/// @brief This function creates a padding over the input
/// @param input 
/// @return 
unsigned char *padding_function(const unsigned char *input)
{
    int input_length = strlen(input);
    int padding_amount = BLOCK_SIZE - (input_length % BLOCK_SIZE);
    printf("Padding needed: %d\n", padding_amount);
    int padded_input_length = input_length + padding_amount;

    // create heap memory for padding
    unsigned char *padded_memory = (unsigned char *)malloc(padded_input_length);
    if (!padded_memory)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    memcpy(padded_memory, input, input_length);
    // Add padding bytes to the padded memory
    for (int i = 0; i < padding_amount; i++)
    {
        // We add padding amount as Byte value so that during decryptionthe algorithm knows
        // exactly how many padding bytes to strip, by reading the last byte’s value.
        padded_memory[input_length + i] = padding_amount;
    }
    return padded_memory;
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
    /* code */
    const unsigned char *input = (unsigned char *)"Test String";
    printf("Input data: %s\n", input);
    // Note: earlier created heap memory 'padded_memory' in padding_function is just being renamed to 'padded_input'.
    unsigned char *padded_input = padding_function(input);
    printf("Padded data: %s\n", padded_input);

    // Print padded input as 16-byte blocks in hex

    for (size_t i = 0; i < strlen(padded_input); i++)
    {
        // printing in hexadecimal format
        printf("%02x ", padded_input[i]);
        if ((i + 1) % BLOCK_SIZE == 0)
            printf("\n");
    }
    
    // Heap memory clean up
    free(padded_input);
    return 0;
}
