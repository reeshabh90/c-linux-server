
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "aes_decrypt.h"
#include "key_expansion.h"

/**
 * * @brief AES inverse S-box is a 256-element lookup table used to substitute bytes during key expansion and decryption.
 */
static const byte inv_sbox[256] = {
    // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D};

/**
 * @brief This function adds the round key to the state matrix.
 * @param words The expanded key words (40 words for AES-128).
 * @param state_matrix The state matrix (4x4) to be transformed.
 * @param round The current round number (0 to 10).
 */
void add_state_round(word *words, byte state_matrix[4][4], int round)
{
    for (int i = 0; i < 4; i++)
    {
        word keyWord = words[round * 4 + i];
        for (int j = 0; j < 4; j++)
        {
            // each word is a Hexadecimal 32-bit value
            // Example: word keyWord = 0xAABBCCDD;
            // Bits:   [31-24] [23-16] [15-8] [7-0]
            // Bytes:    AA      BB      CC    DD
            // keyWord >> 8 = 0x00AABBCC
            // keyWord >> 16 = 0x0000AABB
            // `& 0xFF` is used to mask everything except the lowest byte.
            // This removes any bits that might be left behind in upper bytes.
            state_matrix[j][i] ^= (keyWord >> (8 * (3 - j))) & 0xFF;
        }
    }
}

/**
 * @brief This function performs the ShiftRows transformation in AES.
 * @details
 * Each row of the state matrix is shifted left by:

    0 bytes for row 0

    1 byte for row 1

    2 bytes for row 2

    3 bytes for row 3
 * @param state The state matrix (4x4) to be transformed.
 */
void inv_shift_rows(byte state[4][4])
{
    byte temp[4][4];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[i][j] = state[i][(j - i + 4) % 4];
        }
    }
    // Copy back values
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state[i][j] = temp[i][j];
        }
    }
}

/**
 * @brief This function performs the SubBytes transformation in AES.
 * @param state The state matrix (4x4) to be transformed.
 */
void inv_sub_bytes(byte state[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // SubBytes: This step introduces non-linearity into AES
            state[i][j] = inv_sbox[state[i][j]];
        }
    }
}

/**
 * @brief This function performs the xtime operation used in MixColumns.
 */
byte xtime(byte x)
{
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

/**
 * @brief This function performs multiplication in GF(2^8) used in MixColumns.
 * @details
 * It uses the Russian peasant multiplication algorithm to perform multiplication
 * in the Galois Field GF(2^8) with the irreducible polynomial x^8 + x^4 + x^3 + x + 1.
 * @param a First byte to multiply.
 * @param b Second byte to multiply.
 * @return The result of the multiplication.
 */
byte mul(byte a, byte b)
{
    byte result = 0;
    for (int i = 0; i < 8; i++)
    {
        if (b & 1)
            result ^= a;
        byte hi_bit = a & 0x80;
        a <<= 1;
        if (hi_bit)
            a ^= 0x1b;
        b >>= 1;
    }
    return result;
}

/**
 * @brief This function performs the InvMixColumns transformation in AES.
 * @details
 * It applies the inverse MixColumns transformation to the state matrix.
 * @param state The state matrix (4x4) to be transformed.
 */
void inv_mix_columns(byte state[4][4])
{
    byte temp[4];
    for (int c = 0; c < 4; c++)
    {
        temp[0] = mul(0x0e, state[0][c]) ^ mul(0x0b, state[1][c]) ^
                  mul(0x0d, state[2][c]) ^ mul(0x09, state[3][c]);
        temp[1] = mul(0x09, state[0][c]) ^ mul(0x0e, state[1][c]) ^
                  mul(0x0b, state[2][c]) ^ mul(0x0d, state[3][c]);
        temp[2] = mul(0x0d, state[0][c]) ^ mul(0x09, state[1][c]) ^
                  mul(0x0e, state[2][c]) ^ mul(0x0b, state[3][c]);
        temp[3] = mul(0x0b, state[0][c]) ^ mul(0x0d, state[1][c]) ^
                  mul(0x09, state[2][c]) ^ mul(0x0e, state[3][c]);

        for (int i = 0; i < 4; i++)
            state[i][c] = temp[i];
    }
}

/**
 * @brief This function performs AES encryption on a block of data using the expanded key words.
 * @param block Pointer to the block of data to be encrypted (16 bytes).
 * @param words Pointer to the expanded key words (40 words for AES-128).
 */
char *aes_decrypt(unsigned char *block, word *words, char *plain_block)
{
    printf("AES decryption function called with block and words.\n");
    byte state_matrix[4][4]; // AES state matrix (4x4)
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state_matrix[i][j] = block[i + 4 * j];
        }
    }
    add_state_round(words, state_matrix, 10);
    inv_shift_rows(state_matrix);
    inv_sub_bytes(state_matrix);
    // Round (1-9)
    for (int round = 9; round >= 1; round--)
    {
        // AddRoundKey
        add_state_round(words, state_matrix, round);
        // MixColumns
        inv_mix_columns(state_matrix);
        // ShiftRows
        inv_shift_rows(state_matrix);
        // SubBytes
        inv_sub_bytes(state_matrix);
    }

    // Final round: round 0 (no InvMixColumns)doc
    add_state_round(words, state_matrix, 0);

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            // printf("State[%d][%d]: %02X\n", row, col, state_matrix[row][col]);
            plain_block[col * 4 + row] = state_matrix[row][col];
        }
    }
    return plain_block;
}

/**
 * @brief This function converts a hexadecimal string to bytes.
 */
void hex_string_to_bytes(const char *hex_str, unsigned char *bytes, int length)
{
    for (int i = 0; i < length / 2; i++)
    {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
}

/**
 * @brief This function handles the encryption process for each block of data.
 * @details
 * It processes the padded data, encrypts each 16-byte block using the AES algorithm,
 * and prints the encrypted blocks.
 * @param words Pointer to the expanded key words (40 words for AES-128).
 * @param padded_data Pointer to the padded data to be encrypted.
 */
void decryption_main(word *words, const char *encrypted_text)
{
    // Calculate Input Length
    size_t input_len = strlen(encrypted_text);

    // 32 hex chars per 16-byte block
    int num_blocks = input_len / 32;

    unsigned char *cipher_bytes = malloc(BLOCK_SIZE * num_blocks);
    if (!cipher_bytes)
    {
        printf("Memory allocation failed\n");
        return;
    }

    hex_string_to_bytes(encrypted_text, cipher_bytes, input_len);

    unsigned char *plaintext = malloc(BLOCK_SIZE * num_blocks);
    if (!plaintext)
    {
        printf("Memory allocation failed\n");
        return;
    }
    printf("DEBUG: num_blocks = %d\n", num_blocks);
    if (num_blocks <= 0 || num_blocks > 1000)
    { // sanity check
        printf("Invalid num_blocks: %d\n", num_blocks);
        exit(1);
    }

    for (int i = 0; i < num_blocks; i++)
    {
        unsigned char *block = cipher_bytes + i * BLOCK_SIZE;
        unsigned char *plain_block = plaintext + i * BLOCK_SIZE;
        printf("\nDecrypting block %d:\n", i + 1);
        // Print the block before decryption
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%02X ", block[j]);
        }
        printf("\n");
        // Call the AES decryption function
        aes_decrypt(block, words, plain_block);
    }

    // Remove padding (optional based on your padding logic during encryption)
    int padding = plaintext[BLOCK_SIZE * num_blocks - 1]; // PKCS#7 style
    int final_length = BLOCK_SIZE * num_blocks - padding;

    printf("\nDecrypted Plaintext:\n");
    for (size_t i = 0; i < strlen(plaintext); i++)
    {
        printf("Plaintext[%zu]: %c\n", i, plaintext[i]);
    }
    printf("\n");

    free(cipher_bytes);
    free(plaintext);
    printf("\n");
}


int main(int argc, char const *argv[])
{
    /* code */
    // text: Hello Cipher World
    // Encrypted Cipher Text: B146A131F3F0EA0E26D8DABFB39A8112A15FA380EECEB335CBEA134A8602CF6A
    // AES key: 1CDFAABAB7B9BA7E0EE939035F8165AA

    char input_key[33] = "1CDFAABAB7B9BA7E0EE939035F8165AA";
    char encrypted_text[65] = "91065FB466C4F25EF84CC9E0F7F4F9FA";
    // AES-128: 128 bits = 16 bytes
    byte key_bytes[AES_KEYLEN];
    // 4 words (each 32 bits) for initial round key
    word words[AES_KEY_EXP_SIZE];

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

    KeyExpansion(words, key_bytes);

    // Step 4: Decryption Process
    decryption_main(words, encrypted_text);
    return 0;
}
