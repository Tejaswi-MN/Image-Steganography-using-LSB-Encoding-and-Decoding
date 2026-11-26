#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "decode.h"
#include "types.h"
#include "common.h"

//Read and validate decode arguments

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Check source image
    if (argv[2] == NULL)
    {
        return e_failure;
    }

    if (compare_last_n(argv[2], ".bmp", 4) != 0)
    {
        printf("ERROR: Source image should be a .bmp file\n");
        return e_failure;
    }

    decInfo->src_image_fname = argv[2];


    //Check output file name
    if (argv[3] == NULL)
    {
        printf("ERROR: Output file name is missing\n");
        return e_failure;
    }

    decInfo->sec_fname = argv[3];

    return e_success;
}

//Open stego image

Status open_files_for_decoding(DecodeInfo *decInfo)
{
    decInfo->fptr_src_image =
    fopen(decInfo->src_image_fname, "rb");

    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");

        fprintf(stderr,
                "ERROR: Unable to open file %s\n",
                decInfo->src_image_fname);

        return e_failure;
    }

    printf("Successfully opened stego image\n");

    return e_success;
}

//Decode one byte from 8 image bytes

Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    *data = 0;

    for (int i = 0; i < 8; i++)
    {
        //Shift existing bits left
        *data = *data << 1;

        //Take LSB from image byte
        *data = *data | (image_buffer[i] & 1);
    }

    return e_success;
}


//Decode a 32-bit integer from 32 image bytes

Status decode_size_from_lsb(char *image_buffer, int *size)
{
    *size = 0;

    for (int i = 0; i < 32; i++)
    {
        //Shift previous bits left
        *size = *size << 1;

        //Extract LSB from image byte
        *size = *size | (image_buffer[i] & 1);
    }

    return e_success;
}


//Decode magic string

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char image_buffer[8];
    char decoded_char;

    int magic_length = strlen(magic_string);

    for (int i = 0; i < magic_length; i++)
    {
        //Read 8 bytes from stego image
        if (fread(image_buffer, 1, 8,
            decInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        //Extract one character
        decode_byte_from_lsb(image_buffer, &decoded_char);

        //Compare decoded character with expected magic character
        if (decoded_char != magic_string[i])
        {
            return e_failure;
        }
    }

    return e_success;
}

//Decode secret file extension size

Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    char image_buffer[32];

    //Read 32 bytes from stego image
    if (fread(image_buffer, 1, 32,
        decInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    //Extract integer
    decode_size_from_lsb(image_buffer, size);

    return e_success;
}

//Decode secret file extension
Status decode_secret_file_extn(int size, DecodeInfo *decInfo)
{
    char image_buffer[8];
    char decoded_char;

    if (size > 4)
    {
        printf("ERROR: Extension size is too large\n");
        return e_failure;
    }

    for (int i = 0; i < size; i++)
    {
       //Read 8 image bytes
        if (fread(image_buffer, 1, 8,
            decInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        //Extract one character
        decode_byte_from_lsb(image_buffer, &decoded_char);

        //Store decoded character
        decInfo->extn_secret_file[i] = decoded_char;
    }

    //Null terminate string
    decInfo->extn_secret_file[size] = '\0';

    return e_success;
}


//Decode secret file size

Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    char image_buffer[32];
    int decoded_size;

    //Read 32 image bytes
    if (fread(image_buffer, 1, 32,
        decInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    //Decode integer
    decode_size_from_lsb(image_buffer, &decoded_size);

    //Store as long
    *file_size = decoded_size;

    return e_success;
}


//Decode secret file data

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char secret_byte;

    for (long i = 0;
         i < decInfo->size_secret_file;
    i++)
         {
             //Read 8 bytes from stego image
             if (fread(image_buffer, 1, 8,
                 decInfo->fptr_src_image) != 8)
             {
                 return e_failure;
             }
             decode_byte_from_lsb(image_buffer,
                                  &secret_byte);
             if (fwrite(&secret_byte, 1, 1,
                 decInfo->fptr_secret) != 1)
             {
                 return e_failure;
             }
         }
         return e_success;
}


//Copy remaining image data

Status copy_remaining_img_data(FILE *src, FILE *dest)
{
    char buffer[1024];
    size_t bytes;

    while ((bytes = fread(buffer, 1,
        sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, bytes, dest);
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    int extn_size;

    //STEP 1: Open stego image
    if (open_files_for_decoding(decInfo) == e_failure)
    {
        printf("Error in opening files\n");
        return e_failure;
    }
    printf("Successfully opened stego image\n");


    //STEP 2: Skip BMP header
    fseek(decInfo->fptr_src_image,
          54,
          SEEK_SET);


   //STEP 3: Decode and verify magic string
    printf("Decoding magic string...\n");

    if (decode_magic_string(MAGIC_STRING,
        decInfo) == e_failure)
    {
        printf("ERROR: Magic string mismatch\n");
        printf("This may not be a valid stego image\n");

        fclose(decInfo->fptr_src_image);

        return e_failure;
    }

    printf("Magic string verified successfully\n");


    //STEP 4: Decode extension size
    if (decode_secret_file_extn_size(&extn_size,
        decInfo) == e_failure)
    {
        printf("ERROR: Unable to decode extension size\n");
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    printf("Extension size = %d\n", extn_size);


    //STEP 5: Decode extension
    if (decode_secret_file_extn(extn_size,
        decInfo) == e_failure)
    {
        printf("ERROR: Unable to decode extension\n");

        fclose(decInfo->fptr_src_image);

        return e_failure;
    }

    printf("Secret file extension = %s\n",
           decInfo->extn_secret_file);


    //STEP 6: Decode secret file size
    if (decode_secret_file_size(
        &decInfo->size_secret_file,
        decInfo) == e_failure)
    {
        printf("ERROR: Unable to decode secret file size\n");

        fclose(decInfo->fptr_src_image);

        return e_failure;
    }

    printf("Secret file size = %ld bytes\n",
           decInfo->size_secret_file);


    //STEP 7: Open output secret file
    decInfo->fptr_secret =
    fopen(decInfo->sec_fname, "wb");

    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");

        fprintf(stderr,
                "ERROR: Unable to create output file %s\n",
                decInfo->sec_fname);

        fclose(decInfo->fptr_src_image);

        return e_failure;
    }
    printf("Successfully created output file %s\n",
           decInfo->sec_fname);

    //STEP 8: Decode secret file data
    printf("Decoding secret file data...\n");

    if (decode_secret_file_data(decInfo)
        == e_failure)
    {
        printf("ERROR: Unable to decode secret file data\n");

        fclose(decInfo->fptr_secret);
        fclose(decInfo->fptr_src_image);

        return e_failure;
    }

    //STEP 9: Close files
    fclose(decInfo->fptr_secret);
    fclose(decInfo->fptr_src_image);

    printf("Secret file decoded successfully\n");

    return e_success;
}
