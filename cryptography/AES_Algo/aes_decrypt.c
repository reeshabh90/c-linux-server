
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

// Rcon[i] = (x^(i-1), 0, 0, 0)
// Rcon values are used in key expansion to introduce non-repetition and prevent symmetry in round keys.
const word rcon[11] = {
    0x00000000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1b000000, 0x36000000};

/// @brief This function creates a padding over the input
/// @details The function performs following steps:
/// 1. Removes trailing new line
/// 2. Calculate number of blocks text need to be divided into, as AES works on 16 bytes (128 bits) block only.
/// 3. Last block is padded using pcks7 so that during decryption program knows when data has ended.
/// @param input string array
/// @return
unsigned char *padding_function(char *input_array, int num_blocks, size_t input_len)
{

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
    return padded_data;
    // free(padded_data);
}

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

// Multiply by 2 in GF(2^8)
static inline byte xtime(byte x)
{
    return (x << 1) ^ ((x & 0x80) ? 0x1B : 0x00);
}

// Multiply by 9, 11, 13, 14 using xtime chaining
static inline byte mul9(byte x)
{
    return xtime(xtime(xtime(x))) ^ x; // (((x * 2) * 2) * 2) ^ x
}

static inline byte mul11(byte x)
{
    return xtime(xtime(xtime(x)) ^ x) ^ x; // (((x*2)*2)*2 ^ x)*2 ^ x
}

static inline byte mul13(byte x)
{
    return xtime(xtime(xtime(x) ^ x)) ^ x; // (((x*2)*2)^x)*2 ^ x
}

static inline byte mul14(byte x)
{
    return xtime(xtime(xtime(x) ^ x) ^ x); // (((x*2)*2)^x ^ x)*2
}

void inv_mix_columns(byte state[4][4])
{
    for (int j = 0; j < 4; j++)
    {
        byte s0 = state[0][j];
        byte s1 = state[1][j];
        byte s2 = state[2][j];
        byte s3 = state[3][j];

        state[0][j] = mul14(s0) ^ mul11(s1) ^ mul13(s2) ^ mul9(s3);
        state[1][j] = mul9(s0) ^ mul14(s1) ^ mul11(s2) ^ mul13(s3);
        state[2][j] = mul13(s0) ^ mul9(s1) ^ mul14(s2) ^ mul11(s3);
        state[3][j] = mul11(s0) ^ mul13(s1) ^ mul9(s2) ^ mul14(s3);
    }
}

/**
 * @brief This function performs AES encryption on a block of data using the expanded key words.
 * @param block Pointer to the block of data to be encrypted (16 bytes).
 * @param words Pointer to the expanded key words (40 words for AES-128).
 */
byte *aes_decrypt(unsigned char *block, word *words, byte *ciphertext)
{
    printf("AES encryption function called with block and words.\n");
    byte state_matrix[4][4]; // AES state matrix (4x4)

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state_matrix[i][j] = block[i + 4 * j];
            printf("State[%d][%d]: %02X\n", i, j, state_matrix[i][j]);
        }
    }

    // Round (1-9)
    for (int round = 9; round <= 1; round--)
    {
        // SubBytes
        inv_sub_bytes(state_matrix);
        // ShiftRows
        inv_shift_rows(state_matrix);
        // AddRoundKey
        add_state_round(words, state_matrix, round);
        // MixColumns
        inv_mix_columns(state_matrix);
    }

    // Final round: round 0 (no InvMixColumns)
    inv_shift_rows(state_matrix);
    inv_sub_bytes(state_matrix);
    add_state_round(words, state_matrix, 0);

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            ciphertext[col * 4 + row] = state_matrix[row][col];
        }
    }
    return ciphertext;
}

/**
 * @brief This function handles the encryption process for each block of data.
 * @details
 * It processes the padded data, encrypts each 16-byte block using the AES algorithm,
 * and prints the encrypted blocks.
 * @param words Pointer to the expanded key words (40 words for AES-128).
 * @param padded_data Pointer to the padded data to be encrypted.
 */
void decryption_main(word *words, unsigned char *padded_data, int num_blocks)
{
    printf("DEBUG: num_blocks = %d\n", num_blocks);
    if (num_blocks <= 0 || num_blocks > 1000)
    { // sanity check
        printf("Invalid num_blocks: %d\n", num_blocks);
        exit(1);
    }
    byte final_ciphertext[BLOCK_SIZE * num_blocks];
    for (int i = 0; i < num_blocks; i++)
    {
        unsigned char *block = padded_data + i * BLOCK_SIZE;
        printf("\nEncrypting block %d:\n", i + 1);
        // Print the block before encryption
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%02X ", block[j]);
        }
        printf("\n");
        byte ciphertext[BLOCK_SIZE];
        // Call the AES encryption function (not implemented here)
        aes_decrypt(block, words, ciphertext);

        memcpy(final_ciphertext + i * BLOCK_SIZE, ciphertext, BLOCK_SIZE);
    }

    printf("Encrypted ciphertext:\n");
    for (int i = 0; i < (BLOCK_SIZE * num_blocks); i++)
    {
        printf("%02X", final_ciphertext[i]);
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{
    /* code */
    // text: Hello Cipher World
    // Encrypted Cipher Text: B146A131F3F0EA0E26D8DABFB39A8112A15FA380EECEB335CBEA134A8602CF6A
    // AES key: 1CDFAABAB7B9BA7E0EE939035F8165AA

    char input_key[33] = "1CDFAABAB7B9BA7E0EE939035F8165AA";
    char encrypted_text[65] = "B146A131F3F0EA0E26D8DABFB39A8112A15FA380EECEB335CBEA134A8602CF6A";
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

    // Calculate Input Length
    size_t input_len = strlen(encrypted_text);

    // Remove trailing newline from fgets
    if (encrypted_text[input_len - 1] == '\n')
    {
        encrypted_text[--input_len] = '\0';
    }

    // Calculate number of 16-byte blocks needed
    int num_blocks = input_len / BLOCK_SIZE;
    if (input_len % BLOCK_SIZE != 0)
    {
        num_blocks += 1;
    }

    printf("\nTotal blocks (with padding): %d\n", num_blocks);
    unsigned char *padded_data = padding_function(encrypted_text, num_blocks, input_len);

    // Step 4: Encryption Process
    decryption_main(words, padded_data, num_blocks);
    free(padded_data);
    return 0;
}
