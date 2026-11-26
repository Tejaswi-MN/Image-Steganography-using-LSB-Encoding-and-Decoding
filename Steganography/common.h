#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include "types.h"

#define MAGIC_STRING "#*"

uint get_image_size_for_bmp(FILE *fptr_image);

uint get_file_size(FILE *fptr);

int compare_last_n(char *str1, char *str2, int n);

#endif
