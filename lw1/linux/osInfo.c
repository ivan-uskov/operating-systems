#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE * versionFile = fopen("/proc/version", "r");

    if (versionFile == NULL) 
    {
        fprintf(stderr, "Can't open version file!\n");
        return 1;
    }

    int ch;
    while ((ch = fgetc(versionFile)) != EOF)
    {
        putchar(ch);
    }

    return 0;
}

