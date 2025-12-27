/**
 * emailp V2.0📔
 * @file: emailparser.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program summarizes documents from a sample text file.
 * It identifies unique words, counts their occurrences,
 * and extracts sentences containing the rarest words.
 * The summary is printed to the console in tabular format and saved in a CSV file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>
#include "summarization.h"

/**
 * @brief This function tokenizes the input string based on space delimiter (without modifying input string).
 * @param str Input string to be tokenized
 * @param delim Delimiter character (space in this case)
 * @param tokens Array of string pointers to hold the tokens
 * @param max_tokens_size Maximum number of tokens that can be stored
 * @param buffer Pointer to a character buffer to hold the actual token strings 🔴 Remember: To free memory of buffer
 * @return Number of tokens found
 */
size_t tokenize_by_space(const char *str, char delim, char **tokens, size_t max_tokens_size, char **buffer)
{
    size_t len = strlen(str);
    size_t token_index = 0;
    size_t start = 0;
    size_t buffer_index = 0;
    *buffer = malloc(len + 1);
    if (!*buffer)
        return 0;

    for (size_t i = 0; i <= len; i++)
    {
        if (str[i] == delim || str[i] == '\0')
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

/**
 * @brief Fill up unique words and their counts from tokens
 * @param num_unique Pointer to the count of unique words found so far
 * @param unique_words Array of WordCount structures to hold unique words and their counts
 * @param tokens Array of tokenized words from the current line
 * @param token_count_per_line Number of tokens in the current line
 */
void fill_up_unqiue_words(size_t *num_unique, struct WordCount *unique_words, char **tokens, size_t token_count_per_line)
{
    for (size_t i = 0; i < token_count_per_line; i++)
    {
        int found = 0;
        // Check if word already exists
        for (size_t j = 0; j < *num_unique; j++)
        {
            if (strcmp(unique_words[j].word, tokens[i]) == 0)
            {
                unique_words[j].count++;
                found = 1;
                break;
            }
        }
        // If not found, add it as a new unique word
        if (!found && *num_unique < MAX_UNIQUE_WORDS)
        {
            // 🔴 2. Remember: free up dynamically created memory on the heap by strdup
            unique_words[*num_unique].word = strdup(tokens[i]);
            unique_words[*num_unique].count = 1;
            (*num_unique)++;
        }
    }
}

/**
 * @brief Comparison function for qsort: sort by count ascending, then by word alphabetically if tie
 * @param a Pointer to first WordCount
 * @param b Pointer to second WordCount
 * @return Negative if a < b, positive if a > b, zero if equal
 */
int compare_wordcount(const void *a, const void *b)
{
    const struct WordCount *wa = a;
    const struct WordCount *wb = b;
    if (wa->count != wb->count)
        return wa->count - wb->count;  // ascending count
    return strcmp(wa->word, wb->word); // stable tie-breaker
}

int main(int argc, char const *argv[])
{
    /* 📖 Read a text file */
    FILE *fp = fopen("doc.txt", "r");
    if (!fp)
    {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    char file_content[MAX_LINE];
    char *tokens[MAX_TOKEN_LIMIT];
    // 🔴 1. Remember: To free memory of buffer
    char *buffer;
    char delim = ' ';
    struct WordCount unique_words[MAX_UNIQUE_WORDS];
    size_t num_unique = 0;
    size_t token_count_per_line;

    while (fgets(file_content, sizeof(file_content), fp))
    {
        // Remove newline character
        file_content[strcspn(file_content, "\n")] = '\0';
        token_count_per_line = tokenize_by_space(file_content, delim, tokens, MAX_TOKEN_LIMIT, &buffer);

        // Process each token: add to unique_words or increment count
        fill_up_unqiue_words(&num_unique, unique_words, tokens, token_count_per_line);

        // 🟢1. Done: Memory free of buffer for this line's tokens
        free(buffer);
        buffer = NULL; // Reset for next line
    }

    FILE *fp1;
    const char *filename = "frequency_table.csv";
    // 📖 Open the file for writing ("w")
    fp1 = fopen(filename, "w");

    if (fp1 == NULL)
    {
        printf("Error: could not open file %s\n", filename);
        return EXIT_FAILURE;
    }
    // Print the unique words and their counts
    printf("Unique words and counts:\n");
    // Column headers
    char *headers[] = {"Words", "Counts"};

    // Field width for alignment
    int width = 60;

    // Print the column headers

    for (int j = 0; j < 2; j++)
    {
        // Use '-' flag for left alignment of strings
        fprintf(fp1, "%-*s,", width, headers[j]);
        printf("%-*s", width, headers[j]);
    }
    fprintf(fp1, "\n"); // Move to the next line after headers
    printf("\n");       // Move to the next line after headers

    // Header Demarkation line in console -----
    for (int j = 0; j < 2; j++)
    {
        printf("%-*s", width, "------");
    }
    printf("\n");
    for (size_t i = 0; i < num_unique; i++)
    {
        fprintf(fp1, "%s,%d\n", unique_words[i].word, unique_words[i].count);
        printf("%-*s %-*d\n", width, unique_words[i].word, width, unique_words[i].count);
        if (i < num_unique - 1)
        {
            fprintf(fp1, "\n");
        }
    }

    // 📕 Close the file
    fclose(fp1);
    printf("Successfully generated %s\n", filename);
    // Sort unique words by count (lowest first)
    qsort(unique_words, num_unique, sizeof(struct WordCount), compare_wordcount);

    // Select top 5 lowest-count words
    int top_n = (num_unique < 5) ? num_unique : 5;
    printf("\nTop %d rarest words:\n", top_n);
    for (int i = 0; i < top_n; i++)
    {
        printf("%d. \"%s\" (appears %d time%s)\n",
               i + 1, unique_words[i].word, unique_words[i].count,
               unique_words[i].count == 1 ? "" : "s");
    }
    // Go back to beginning of file
    rewind(fp);
    // Senetence Extraction for summary
    printf("\nSentences containing these rare words:\n");

    int sentence_number = 1;
    while (fgets(file_content, sizeof(file_content), fp))
    {
        // Remove trailing newline for clean printing
        file_content[strcspn(file_content, "\n")] = '\0';

        // Check if this line contains any of the rare words
        int contains_rare = 0;
        for (int i = 0; i < top_n; i++)
        {
            if (strstr(file_content, unique_words[i].word) != NULL)
            {
                contains_rare = 1;
                break;
            }
        }

        if (contains_rare)
        {
            printf("%d. %s\n", sentence_number, file_content);
        }
        sentence_number++;
    }
    // 🟢2. Clean up: free each strdup'd word since they were dynamically allocated using malloc by strdup
    for (size_t i = 0; i < num_unique; i++)
    {
        free(unique_words[i].word);
    }
    // 📕 Close file pointer
    fclose(fp);
    return EXIT_SUCCESS;
}
