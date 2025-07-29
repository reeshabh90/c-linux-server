
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "aes_decrypt.h"
#include "key_expansion.h"

int main(int argc, char const *argv[])
{
    /* code */
    // text: Hello Cipher World
    // Encrypted Cipher Text: B146A131F3F0EA0E26D8DABFB39A8112A15FA380EECEB335CBEA134A8602CF6A
    // AES key: 1CDFAABAB7B9BA7E0EE939035F8165AA

    char input_key[33] = "1CDFAABAB7B9BA7E0EE939035F8165AA";
    // char encrypted_text[65] = "B146A131F3F0EA0E26D8DABFB39A8112A15FA380EECEB335CBEA134A8602CF6A";
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
    return 0;
}
