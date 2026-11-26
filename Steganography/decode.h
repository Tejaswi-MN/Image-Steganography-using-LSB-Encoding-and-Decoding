#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Decode Information */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *src_image_fname;
    FILE *fptr_src_image;

    /* Secret File Info */
    char *sec_fname;
    FILE *fptr_secret;

    char extn_secret_file[5];
    long size_secret_file;

    /* Image Info */
    uint image_capacity;

} DecodeInfo;


/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open required files */
Status open_files_for_decoding(DecodeInfo *decInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(int size, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Extract one byte from 8 image bytes */
Status decode_byte_from_lsb(char *image_buffer, char *data);

/* Extract 32-bit integer from 32 image bytes */
Status decode_size_from_lsb(char *image_buffer, int *size);

/* Copy remaining image data - not required for decoding */
Status copy_remaining_img_data(FILE *src, FILE *dest);

/* Compare last n characters */
int compare_last_n(char *str1, char *str2, int n);

#endif
