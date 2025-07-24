#pragma once

#define AES_KEYLEN 16       // 128-bit key = 16 bytes
#define AES_NK 4            // 4 words
#define AES_NR 10           // 10 rounds
#define AES_NB 4            // 4 words per round
#define AES_KEY_EXP_SIZE 44 // 4 words * (10 rounds + 1)
#define BLOCK_SIZE 16       // AES input block = 16 bytes

typedef uint8_t byte;
typedef uint32_t word;
byte hex_char_to_int(char c);
byte hex_byte(const char *hex);
word RotWord(word w);
word SubWord(word w);
void KeyExpansion(word words[4], byte key_bytes[16]);
void apply_pkcs7_padding(unsigned char *block, int data_len);
unsigned char *padding_function(char *input_array, int num_blocks, size_t input_len);
void encryption_main(word *words, unsigned char *padded_data, int num_blocks);
byte *aes_encrypt(unsigned char *block, word *words, byte *ciphertext);
void add_state_round(word *words, byte state_matrix[4][4], int round);
static inline byte xtime(uint8_t x);
void sub_bytes(byte state_matrix[4][4]);
void shift_rows(byte state_matrix[4][4]);
void mix_columns(byte state_matrix[4][4]);
