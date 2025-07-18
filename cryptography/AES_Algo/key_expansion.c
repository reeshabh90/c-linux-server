
/**
 * 📔key_expansion.0📔
 * @file: key_expansion.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program expands a 128-bit (16 bytes) key using a key scheduling process
 * 1. It starts from a user-provided 128-bit (16-byte) hexadecimal key.
 * 2. Converts the hex string into 4 initial 32-bit words (W0–W3).
 * 3. Then, it applies AES key expansion logic to generate all 44 words (for 11 rounds).
 *
 * AES Key Schedule Overview:
 * - Each round key consists of 4 words (128 bits).
 * - Total 44 words = 4 (original) + 10 * 4 (for each round).
 * - Every 4th word is generated using a function `g()`, which:
 *   a. Rotates the word left by one byte (RotWord).
 *   b. Applies S-box substitution (SubWord).
 *   c. XORs with a round constant (Rcon).
 * 
 * 
 * 🔧 gcc -g key_expansion.c -o key_exp
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "key_expansion.h"


// AES S-box s a 256-element lookup table used to substitute bytes during key expansion and encryption.
static const byte sbox[256] = {
    // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Rcon[i] = (x^(i-1), 0, 0, 0)
// Rcon values are used in key expansion to introduce non-repetition and prevent symmetry in round keys.
const word rcon[11] = {
    0x00000000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1b000000, 0x36000000
};

/// @brief This function converts a single hexadecimal character (0–9, A–F, a–f) into integer value (0–15)
/// @param c
/// @return
byte hex_char_to_int(char c)
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
byte hex_byte(const char *hex)
{
    return (hex_char_to_int(hex[0]) << 4) | hex_char_to_int(hex[1]);
}

/// @brief This function performs a circular rotation on a 32 bit word
/// @details 
/// We first shift the values in 32 bit word to 8 bit positions left, which results in last 9 positions to be 00000000.
/// We then shift the values in original 32 bit word to 24 positions right, which rsults in 1st 24 positions to be 000...0
/// At last we combine both the results using OR operation and return the result.
/// @param w 
/// @return 
word RotWord(word w)
{
    // word is 32 bits
    word leftShift = w << 8;
    word rightShift = w >> 24;
    // combine the result
    return (leftShift | rightShift);
}
/// @brief This function applies S-box substitution to each byte in word
/// @details
/// Each byte is substituted with: 
/// 1. Its multiplicative inverse in GF(2⁸).
/// 2. An affine transformation applied on the 8-bit representation.
/// This process adds non-linearity to prevent linear attacks.
/// @param w 
/// @return 
word SubWord(word w) {
    return (sbox[(w >> 24) & 0xFF] << 24) |
           (sbox[(w >> 16) & 0xFF] << 16) |
           (sbox[(w >> 8)  & 0xFF] << 8)  |
           (sbox[w & 0xFF]);
}

/// @brief This function combines bytes to form words
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
void KeyExpansion(word words[4], byte key_bytes[16])
{
    for (int i = 0; i < AES_NK; i++)
    {
        words[i] = (key_bytes[4 * i] << 24) |
                   (key_bytes[4 * i + 1] << 16) |
                   (key_bytes[4 * i + 2] << 8) |
                   key_bytes[4 * i + 3];
    }

    // Print the 4 words
    printf("\nInitial 4 words (in hex):\n");
    for (int i = 0; i < AES_NK; i++)
    {
        printf("W%d: %08X\n", i, words[i]);
    }
    // Expanding rest of the 40 keys
    for (int i = AES_NK; i < AES_KEY_EXP_SIZE; i++)
    {
        word temp = words[i-1];
        if(i % AES_NK == 0) 
        {
            temp = SubWord(RotWord(temp)) ^ rcon[i / AES_NK];
        }
        words[i] = words[i - AES_NK] ^ temp;
    }

    printf("Next Round Keys:\n");
    for (int i = AES_NK; i < AES_KEY_EXP_SIZE; i++) {
        printf("W[%2d]: %08X\n", i, words[i]);
    }
}

int main(int argc, char const *argv[])
{
    // Input string buffer: 32 hex characters + null terminator
    char input_key[33];
    // AES-128: 128 bits = 16 bytes
    byte key_bytes[AES_KEYLEN];
    // 4 words (each 32 bits) for initial round key
    word words[AES_KEY_EXP_SIZE];

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
    for (int i = 0; i < AES_KEYLEN; ++i)
    {
        // printf("Hex pairs: %c%c\n", input_key[i * 2], input_key[i * 2 + 1]);
        key_bytes[i] = hex_byte(&input_key[i * 2]);
    }

    printf("Converted 16 bytes: ");
    for (int i = 0; i < AES_KEYLEN; i++)
    {
        printf("%X ", key_bytes[i]);
    }
    printf("\n");

    // Step 2: Expand the keys (big-endian)
    KeyExpansion(words, key_bytes);
    return 0;
}
