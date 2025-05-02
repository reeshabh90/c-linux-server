
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "key_expansion.h"

/// @brief This function converts a single hexadecimal character (0–9, A–F, a–f) into integer value (0–15)
/// @param c
/// @return
uint8_t hex_char_to_int(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    return 0; // fallback if invalid char
}

/// @brief Convert 2 hexadecimal characters to a single byte (e.g., "3A" -> 0x3A)
/// @details 1 byte = 8 bits
/// Example, Charachter pair: 3A
/// Step 1. Both charachters are being converted to integer, hex_char_to_int('3') = 3
/// 3 in binary: 0000 0011
/// Step 2. Left Shift by 4 to 1st charachter
/// 0000 0011 << 4 = 0011 0000, Reason: First hex char represents higher 4 bits & Second hex char represents lower 4 bits.
/// Step 3. Combine using bitwise OR, byte = high | low ; Example: 0011 0000 | 0000 1010 =  0011 1010 = 0x3A
/// @param hex
/// @return
uint8_t hex_byte(const char *hex)
{
    return (hex_char_to_int(hex[0]) << 4) | hex_char_to_int(hex[1]);
}

/// @brief Ths function combines bytes to form words
/// @details
/// Each byte is 8 bits long, and we need to combine 4 bytes to form a 32-bit word.
/// The bit-shifting operation helps us place each byte into its correct position in the final 32-bit word.
/// Example: If the first 4 bytes of the AES key are:
/// key_bytes[0] = 0x59
/// key_bytes[1] = 0x1B
/// key_bytes[2] = 0xE4
/// key_bytes[3] = 0x03
///------------------------------------
/// W0 = (0x59 << 24) | (0x1B << 16) | (0xE4 << 8) | 0x03
/// = 0x59000000 | 0x1B0000 | 0xE400 | 0x03
/// = 0x591BE403
/// @param words
/// @param key_bytes
void WordFormation(uint32_t words[4], uint8_t key_bytes[16])
{
    for (int i = 0; i < 4; ++i)
    {
        words[i] = (key_bytes[4 * i] << 24) |
                   (key_bytes[4 * i + 1] << 16) |
                   (key_bytes[4 * i + 2] << 8) |
                   key_bytes[4 * i + 3];
    }

    // Print the 4 words
    printf("\nInitial 4 words (in hex):\n");
    for (int i = 0; i < 4; ++i)
    {
        printf("W%d: %08X\n", i, words[i]);
    }
}

int main(int argc, char const *argv[])
{
    // Input string buffer: 32 hex characters + null terminator
    char input_key[33];
    // AES-128: 128 bits = 16 bytes
    uint8_t key_bytes[16];
    // 4 words (each 32 bits) for initial round key
    uint32_t words[4];

    printf("Enter 128-bit AES key in hex (32 hex digits):\n");
    scanf("%32s", input_key);

    // Validate input length
    if (strlen(input_key) != 32)
    {
        printf("Error: You must enter exactly 32 hexadecimal characters (128 bits).\n");
        return 1;
    }
    /**
     * Expected Output:
     * Enter 128-bit AES key in hex (32 hex digits):
       1CDFAABAB7B9BA7E0EE939035F8165AA
       Converted 16 bytes: 1C DF AA BA B7 B9 BA 7E 0E E9 39 03 5F 81 65 AA
       NOTE: The input is string and hence we get charachters and their ASCII value, not numbers, hence byte conversion.
     */
    // Step 1: Convert each pair of hex characters to a byte
    for (int i = 0; i < 16; ++i)
    {
        // printf("Hex pairs: %c%c\n", input_key[i * 2], input_key[i * 2 + 1]);
        key_bytes[i] = hex_byte(&input_key[i * 2]);
    }

    printf("Converted 16 bytes: ");
    for (int i = 0; i < 16; i++)
    {
        printf("%X ", key_bytes[i]);
    }
    printf("\n");

    // Step 2: Split into 4 32-bit words (big-endian)
    WordFormation(words, key_bytes);
    return 0;
}
