#include <stdio.h>
#define ARRAY_SIZE 10

int main(int argc, char const *argv[])
{
    int num[ARRAY_SIZE];
    int total = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        scanf("%d", &num[i]);
        total += num[i];
    }

    printf("Total: %d\n", total);
    return 0;
}
