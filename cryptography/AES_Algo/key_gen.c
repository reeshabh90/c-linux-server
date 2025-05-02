/**
 * 📔key_generation.0📔
 * @file: key_gen.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program generates a 128-bit (16 bytes) key
 * 1. The program relies on OpenSSL’s crypto library's RAND_bytes() function.
 * 2. The functions in the OpenSSL’s crypto library aren't automatically included by default.
 *    They are external, meaning they are not built into the compiler or executable
 * Note: Execute the program by linking to OpenSSL's crypto library, which has to be pre-installed.
 * Command: gcc -o key_gen key_gen.c -lcrypto 
 * Here, `-lcrypto` flag allows the program to call OpenSSL functions at runtime.
 * Installation: sudo apt install libssl-dev
 */

#include <stdio.h>
#include <openssl/rand.h>

unsigned char* key_generator()
{
    // IN C, if we arrays created inside a function are allocated on the stack and are destroyed when the function exits.
    // Hence, a pointer to a local array directly like this because it will point to invalid memory once the function returns.
    // Either, Dynamically allocate memory for the key (using malloc).
    // Or, pass the key array to the function by reference (i.e., passing the address of the array). e.g. void key_generator(unsigned char *key)    

    unsigned char *key = (unsigned char*)malloc(16); // Allocate memory for 128-bit key
    if (key == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    // Generate secure random bytes
    // Note: RAND_bytes expects unisgned char* in its argument.
    // IN C, when we pass an array to a function, it is automatically treated as a pointer to the first element of the array.
    if (RAND_bytes(key, sizeof(key)) != 1)
    {
        fprintf(stderr, "Error generating random bytes\n");
        free(key);
        return NULL;
    }
    return key;
}

int main(int argc, char const *argv[])
{
    unsigned char *generatedKey = key_generator();
    if (generatedKey == NULL) {
        return 1; // Exit if key generation failed
    }
    printf("128-bit AES Key: ");
    for (int i = 0; i < sizeof(generatedKey); i++)
    {
        printf("%02X", generatedKey[i]);
    }
    printf("\n");
    // Free the dynamically allocated memory
    free(generatedKey);
    // Had we taken the approach of referencing the array in key_generator function, we would have skipped the
    // hard work of memory management.
    return 0;
}
