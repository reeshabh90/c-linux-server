#ifndef PATTERN_MAP_H
#define PATTERN_MAP_H

#define MAX_LINE 8192
#define MAX_SECTIONS 1024

typedef struct
{
    char *SectionName;
    char *ParentSection;
    char *SectionNumber;
    int Level;
    int SectionLineNumber;
    char *SectionContent;
} Section;

#endif