#pragma once

// #define AES_KEYLEN 16       // 128-bit key = 16 bytes
// #define AES_NK 4            // 4 words
// #define AES_NR 10           // 10 rounds
// #define AES_NB 4            // 4 words per round
// #define AES_KEY_EXP_SIZE 44 // 4 words * (10 rounds + 1)
#define BLOCK_SIZE 16       // AES input block = 16 bytes

// typedef uint8_t byte;
// typedef uint32_t word;
// byte hex_char_to_int(char c);
// byte hex_byte(const char *hex);