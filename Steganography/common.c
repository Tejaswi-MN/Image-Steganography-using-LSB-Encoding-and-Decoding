#include <stdio.h>
#include <string.h>

#include "common.h"
#include "types.h"


uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width;
    uint height;

    fseek(fptr_image, 18, SEEK_SET);

    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);

    return width * height * 3;
}


uint get_file_size(FILE *fptr)
{
    long current_position;
    long size;

    current_position = ftell(fptr);

    fseek(fptr, 0, SEEK_END);

    size = ftell(fptr);

    fseek(fptr, current_position, SEEK_SET);

    return (uint)size;
}


int compare_last_n(char *str1, char *str2, int n)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if (len1 < n || len2 < n)
    {
        return -1;
    }

    return strcmp(str1 + len1 - n,
                  str2 + len2 - n);
}
