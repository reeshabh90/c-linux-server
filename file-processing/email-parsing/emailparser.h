#ifndef EMAILPARSER_H
#define EMAILPARSER_H

#define MAX_LINE 8192

void clean_content(char *file_content);
void convert_to_lowercase(char *str);
void replace_all(char *str, const char *old, const char *updated);
void normalize(char *file_content);
int extract_email(const char *text);

#endif