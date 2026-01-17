/**
 * pattern-map V1.0📔
 * @file: pattern-map.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program parses a text file to identify sections based on
 *    heading patterns and organizes them into a structured format.
 *    It captures section names, numbering, levels, parent sections,
 *    line numbers, and content.
 */
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <string.h>
#include "pattern-map.h"

Section sections[MAX_SECTIONS];
int section_count = 0;
int section_index_stack[MAX_SECTIONS];
int stack_top = -1;
Section *current_section = NULL;

void add_heading_details(const char *line, int line_num)
{
    if (section_count >= MAX_SECTIONS)
    {
        printf("Maximum section limit reached.\n");
        return;
    }

    const char *separatorPosition = line;
    while (*separatorPosition && *separatorPosition != ' ' && *separatorPosition != '\t')
    {
        separatorPosition++;
    }

    size_t spaceLength = separatorPosition - line;
    // 🔴 1. Dynamic memory Allocation
    char *numberingPattern = malloc(spaceLength + 1);
    memcpy(numberingPattern, line, spaceLength);
    numberingPattern[spaceLength] = '\0';

    const char *sectionNameStart = separatorPosition;
    while (*sectionNameStart && *sectionNameStart == ' ' || *sectionNameStart == '\t')
    {
        sectionNameStart++;
    }

    size_t text_len = strlen(sectionNameStart);
    // 🔴 2. Dynamic memory Allocation
    char *sectionName = malloc(text_len + 1);
    memcpy(sectionName, sectionNameStart, text_len);
    sectionName[text_len] = '\0';

    // Determine level based on numbering pattern
    int level = 1;
    for (size_t i = 0; i < strlen(numberingPattern); i++)
    {
        if (numberingPattern[i] == '.' && isdigit(numberingPattern[i + 1]))
        {
            level++;
        }
    }

    // Determine Parent Section

    while (stack_top >= 0 && sections[section_index_stack[stack_top]].Level >= level)
    {
        stack_top--;
    }

    char *parentSection = (stack_top >= 0)
                              ? sections[section_index_stack[stack_top]].SectionName
                              : "-1";

    Section *sec = &sections[section_count];
    sec->SectionName = sectionName;
    sec->SectionNumber = numberingPattern;
    sec->SectionLineNumber = line_num;
    sec->Level = level;
    sec->ParentSection = parentSection ? parentSection : "-1";
    sec->SectionContent = NULL;
    section_index_stack[++stack_top] = section_count;
    current_section = sec;
    section_count++;
}

void add_section_content(const char *line)
{
    if (current_section == NULL || !line || strlen(line) == 0)
    {
        return;
    }

    size_t line_len = strlen(line);
    char *new_content = malloc(line_len + 2);
    if (!new_content)
    {
        fprintf(stderr, "Failed to allocate memory for content\n");
        return;
    }

    strcpy(new_content, line);
    strcat(new_content, "\n");

    if (current_section->SectionContent == NULL)
    {
        // First line for this section
        current_section->SectionContent = new_content;
    }
    else
    {
        // Append to existing content
        size_t existing_len = strlen(current_section->SectionContent);
        char *updated_content = malloc(existing_len + line_len + 2);
        if (!updated_content)
        {
            fprintf(stderr, "Failed to allocate memory for updated content\n");
            free(new_content);
            return;
        }
        strcpy(updated_content, current_section->SectionContent);
        strcat(updated_content, new_content);
        free(current_section->SectionContent);
        free(new_content);
        // 🔴 3. Dynamic memory Allocation
        current_section->SectionContent = updated_content;
    }
}

void print_all_sections()
{
    printf("Total Sections Parsed: %d\n", section_count);
    for (int i = 0; i < section_count; i++)
    {
        Section sec = sections[i];
        printf("Section Name: %s\n", sec.SectionName);
        printf("Section Number: %s\n", sec.SectionNumber);
        printf("Level: %d\n", sec.Level);
        printf("Parent Section: %s\n", sec.ParentSection);
        printf("Line Number: %d\n", sec.SectionLineNumber);
        printf("Content: %s\n", sec.SectionContent ? sec.SectionContent : "N/A");
        printf("-----------------------\n");
    }
}

void free_all_sections()
{
    for (int i = 0; i < section_count; i++)
    {
        // 🟢  1,2,3 Dynamic Memory free up
        free(sections[i].SectionName);
        free(sections[i].SectionNumber);
        if (sections[i].SectionContent)
        {
            free(sections[i].SectionContent);
        }
    }
    section_count = 0;
    stack_top = -1;
    current_section = NULL;
}

int main(int argc, char const *argv[])
{
    // Read File
    FILE *fp;
    char file_content[MAX_LINE];
    int line_num = 1;
    regex_t regex;
    regmatch_t match;
    // pattern for heading
    const char *heading_pattern =
        "^([0-9].[[:space:]]|[0-9](.[0-9])*[[:space:]])+([A-Z][a-zA-Z0-9]*|[A-Z]+)([[:space:]]+.*)?$";

    if (regcomp(&regex, heading_pattern, REG_EXTENDED))
    {
        fprintf(stderr, "Failed to load regex\n");
        return 0;
    }
    fp = fopen("sample.txt", "r");

    if (!fp)
    {
        perror("Error opening file");
        return 1;
    }
    while (fgets(file_content, sizeof(file_content), fp))
    {
        file_content[strcspn(file_content, "\n")] = '\0';
        // remove empty lines
        if (strlen(file_content) == 0)
        {
            line_num++;
            continue;
        }
        if (regexec(&regex, file_content, 1, &match, 0) == 0)
        {
            // printf("Heading Extracted: %s\n", file_content);
            add_heading_details(file_content, line_num);
        }
        else
        {
            if (current_section == NULL)
            {
                line_num++;
                continue;
            }
            // printf("Content Line: %s\n", file_content);
            add_section_content(file_content);
        }
        line_num++;
    }
    regfree(&regex);
    print_all_sections();
    free_all_sections();
    fclose(fp);
    return EXIT_SUCCESS;
}
