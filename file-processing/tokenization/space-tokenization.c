#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief This function tokenizes the input string based on space delimiter (without modifying input string).
 * @param str Input string to be tokenized
 * @param delim Delimiter character (space in this case)
 * @param tokens Array of string pointers to hold the tokens
 * @param max_tokens_size Maximum number of tokens that can be stored
 * @param buffer Pointer to a character buffer to hold the actual token strings 🔴 Remember: To free memory of buffer
 * @return Number of tokens found
 */
size_t tokenize_by_space(char *str, char *delim, char **tokens, size_t max_tokens_size, char **buffer)
{
    size_t len = strlen(str);
    size_t token_index = 0;
    size_t start = 0;
    size_t buffer_index = 0;
    printf("Create a buffer of size w.r.t input string: %zu\n", len + 1);
    *buffer = malloc(len + 1);
    if (!*buffer)
        return 0;

    for (size_t i = 0; i <= len; i++)
    {
        if (str[i] == delim[0] || str[i] == '\0')
        {
            // variable to store length of the token to be copied
            size_t tok_len = i - start;
            if (tok_len > 0 && token_index < max_tokens_size)
            {
                // copy portion of the input string upto the point delimiter or null value was encountered
                memcpy(*buffer + buffer_index, &str[start], tok_len);
                (*buffer)[buffer_index + tok_len] = '\0'; // null-terminate the token
                // adding pointer to the token in tokens array
                tokens[token_index] = *buffer + buffer_index;
                buffer_index += tok_len + 1;
                token_index++;
            }
            // setting up start for the next token
            start = i + 1;
        }
    }

    return token_index;
}

int main(int argc, char const *argv[])
{
    /* code */
    size_t size = 10;
    char *tokens[10];
    char *buffer;
    // 🔴 Remember: To free memory of buffer
    char string_to_tokenize[] = "This is a sample string to be tokenized";
    char *delim = " ";
    printf("Original string: %s\n", string_to_tokenize);
    size_t token_count = tokenize_by_space(string_to_tokenize, delim, tokens, size, &buffer);
    printf("Tokenized string:\n");
    for (size_t i = 0; i < token_count; i++)
    {
        printf("Token %zu: %s\n", i + 1, tokens[i]);
    }
    // 🟢 Done: Memory free of buffer
    free(buffer);
    return 0;
}
