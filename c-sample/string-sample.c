#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /* code */
    char str1[30] = "Reeshabh is here";
    char str2[30] = "bh";
    int index = -1;
    for (int i = 0; i < strlen(str1); i++)
    {
        if (str1[i] == str2[0])
        {
            int j = 0;
            for (int j = 0; j < strlen(str2); j++)
            {
                if (str1[i + j] != str2[j])
                {
                    break;
                }
                if (j == strlen(str2) - 1)
                {
                    index = i;
                    printf("Substring found at index: %d\n", index);
                    return 0;
                }
            }
        }
    }
    return 0;
}
