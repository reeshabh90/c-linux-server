/**
 * emailparser V1.0📔
 * @file: emailparser.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program parses emails from a sample text file.
 *  Program Flow:
 * 1. Read the input file.
 * 2. Remove noise
 * 3. Apply logic for converting raw text to standard email format.
 * 4. Do parsing
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include "emailparser.h"

/**
 * @brief Clean input content by removing noise
 * @param file_content Input string
 * @details Removes spaces, numbering, labels, and junk symbols
 */
void clean_content(char *file_content)
{
    char *ptr = file_content;

    // 1. Remove spaces
    while (isspace(*ptr))
        ptr++;

    // 2. Remove numbering
    if (isdigit(*ptr))
    {
        while (isdigit(*ptr))
            ptr++;
        if (*ptr == '.' || *ptr == ')')
            ptr++;
    }

    // 3. Remove labels
    if (!strncmp(ptr, "Email:", 6) ||
        !strncmp(ptr, "Email:-", 7) ||
        !strncmp(ptr, "ID:", 3))
    {

        while (*ptr && *ptr != ':')
            ptr++;
        if (*ptr == ':')
            ptr++;
    }

    // 4. Remove leading junk symbols
    while (*ptr == '-' || *ptr == ':' || isspace(*ptr))
        ptr++;

    // Shift cleaned content to start
    memmove(file_content, ptr, strlen(ptr) + 1);
}

/**
 * @brief Convert string to lowercase
 * @param str Input string
 */
void convert_to_lowercase(char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}

void replace_all(char *str, const char *old, const char *updated)
{
    char buffer[MAX_LINE];
    char *ptr;

    while ((ptr = strstr(str, old)) != NULL)
    {
        buffer[0] = '\0';

        strncat(buffer, str, ptr - str);
        strcat(buffer, updated);
        strcat(buffer, ptr + strlen(old));

        strcpy(str, buffer);
    }
}

/**
 * @brief Normalize email content
 * @param file_content Input string
 * @details This is the main rules engine to normalize text to email format
 */
void normalize(char *file_content)
{

    convert_to_lowercase(file_content);

    replace_all(file_content, "_at_the_rate_", "@");
    replace_all(file_content, "-at-the-rate-", "@");
    replace_all(file_content, " at the rate ", "@");
    replace_all(file_content, "attherate", "@");
    replace_all(file_content, "(attherate)", "@");
    replace_all(file_content, "(at)", "@");
    replace_all(file_content, "[at]", "@");
    replace_all(file_content, " at ", "@");
    replace_all(file_content, "(@)", "@");


    replace_all(file_content, "_dot_", ".");
    replace_all(file_content, " dot ", ".");
    replace_all(file_content, "(dot)", ".");
    replace_all(file_content, "[dot]", ".");
    replace_all(file_content, " period ", ".");
    replace_all(file_content, "period ", ".");
    replace_all(file_content, "(period)", ".");

    replace_all(file_content, " underscore ", "_");
    replace_all(file_content, "underscore", "_");

    replace_all(file_content, " gml ", " gmail ");
    replace_all(file_content, " gmai ", " gmail ");
    replace_all(file_content, " google mail ", " gmail ");   
    // replace_all(file_content, "gm", "gmail"); 
    replace_all(file_content, " gm ", " gmail ");
    replace_all(file_content, "gm ", "gmail");
    

     // Cleanup spaces around symbols
    replace_all(file_content, " @ ", "@");
    replace_all(file_content, " . ", ".");
    replace_all(file_content, " .com", ".com");
}

/**
 * @brief Extract and print email if present
 * @param text Normalized input string
 * @return 1 if email found, 0 otherwise
 * @details Uses regex to find email patterns
 */
int extract_email(const char *text)
{
    regex_t regex;
    regmatch_t match;
    // pattern for email 
    const char *email_pattern =
        "[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,}";

    if (regcomp(&regex, email_pattern, REG_EXTENDED | REG_ICASE))
    {
        fprintf(stderr, "Failed to compile regex\n");
        return 0;
    }

    int ret = regexec(&regex, text, 1, &match, 0);

    if (ret == 0)
    {
        // Extract matched email
        int start = match.rm_so;
        int end   = match.rm_eo;

        char email[MAX_LINE];
        int len = end - start;

        strncpy(email, text + start, len);
        email[len] = '\0';

        printf("Email Extracted: %s\n", email);

        regfree(&regex);
        return 1;
    }

    regfree(&regex);
    return EXIT_SUCCESS;
}



int main(int argc, char const *argv[])
{
    // Read File
    FILE *fp;
    char file_content[MAX_LINE];

    fp = fopen("email.txt", "r");
    if (!fp)
    {
        perror("Error opening file");
        return 1;
    }

    while (fgets(file_content, sizeof(file_content), fp))
    {
        file_content[strcspn(file_content, "\n")] = '\0';
        clean_content(file_content);
        normalize(file_content);

        if (strlen(file_content) == 0)
            continue;

        printf("NORMALIZED: %s\n", file_content);
        extract_email(file_content);
    }

    fclose(fp);
    return EXIT_SUCCESS;
}