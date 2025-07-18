#pragma once
#define AES_KEYLEN 16       // 128-bit key = 16 bytes
#define AES_NK 4            // 4 words
#define AES_NR 10           // 10 rounds
#define AES_NB 4            // 4 words per round
#define AES_KEY_EXP_SIZE 44 // 4 words * (10 rounds + 1)

typedef uint8_t byte;
typedef uint32_t word;
byte hex_char_to_int(char c);
byte hex_byte(const char *hex);
word RotWord(word w);
word SubWord(word w);
void KeyExpansion(word words[4], byte key_bytes[16]);
