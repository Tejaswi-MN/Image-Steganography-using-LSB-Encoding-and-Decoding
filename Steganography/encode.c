#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // check argv[2] having .bmp
    if (argv[2] != NULL) {
        if (compare_last_n(argv[2],".bmp", 4) != 0) {
            return e_failure;
        } else {
            encInfo->src_image_fname = argv[2];
        }
    }

    // check argv[3] is having extn or not
    if (argv[3] != NULL) {
        if (compare_last_n(argv[3], ".txt", 4) == 0) {
            encInfo->sec_fname = argv[3];
            strcpy(encInfo->extn_secret_file, ".txt");
        } else {
            return e_failure;
        }
    }


    // check argv[4] having .bmp
    if (argv[4] != NULL) {
        if (compare_last_n(argv[4], ".bmp", 4) == 0) {
            encInfo->stego_image_fname = argv[4];
        } else {
            return e_failure;
        }
    } else {
        encInfo->stego_image_fname = "stego.bmp";
    }


    // return e_success;
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->sec_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->sec_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);


    if (encInfo->image_capacity > (strlen(MAGIC_STRING) * 8 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8))) {
        return e_success;
    }
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    //Seek to 0th Position
    fseek(fptr_src_image, 0, SEEK_SET);

    // read 54 bytes from src file pointer
    fread(buffer, 1, 54, fptr_src_image);
    // write 45 bytes to dest file
    fwrite(buffer, 1, 54, fptr_dest_image);

    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i=0; i<strlen(magic_string); i++) {
        // read 8 bytes from src file
        fread(buffer, 1, 8, encInfo->fptr_src_image);

        // encode_byte_to_lsb
        encode_byte_to_lsb(magic_string[i], buffer);

        // write 8 bytes to dest file
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image); //fptr_stego_image is the destination image
    }

    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    // read 32 bytes from src file
    fread(buffer, 1, 32, encInfo->fptr_src_image);

    // encode_size_to_lsb
    encode_size_to_lsb(size, buffer);

    // write 8 bytes to dest file
    fwrite(buffer, 1, 32, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i=0; i<strlen(file_extn); i++) {
        // read 8 bytes from src file
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        // encode_byte_to_lsb
        encode_byte_to_lsb(file_extn[i], buffer);
        // write 8 bytes to dest file
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    // read 32 bytes from src file
    fread(buffer, 1, 32, encInfo->fptr_src_image);
    // encode_size_to_lsb
    encode_size_to_lsb(file_size, buffer);
    // write 32 bytes to dest file
    fwrite(buffer, 1, 32, encInfo->fptr_stego_image);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char image_buffer[8];
    char secret_byte;

    fseek(encInfo->fptr_secret, 0, SEEK_SET);

    while (fread(&secret_byte, 1, 1, encInfo->fptr_secret) == 1)
    {
        fread(image_buffer, 1, 8, encInfo->fptr_src_image);

        encode_byte_to_lsb(secret_byte, image_buffer);

        fwrite(image_buffer, 1, 8, encInfo->fptr_stego_image);
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // write logic to encode data in lsb
    for (int i=0; i<8; i++) {
        image_buffer[i] &= 0xFE; // Clearing the last bit (1111 1110)
        image_buffer[i] |= (data >> (7-i)) & 0x01; // Shifting data bits to POS and extracting the last bit
    }

    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // write logic to encode data in lsb
    for (int i=0; i<32; i++) {
        imageBuffer[i] &= 0xFE; // Clearing the last bit (1111 1110)
        imageBuffer[i] |= (size >> (31-i)) & 0x01; // Shifting data bits to POS and extracting the last bit
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // check open_files(encInfo) is returning e_success or not
    if (open_files(encInfo) == e_failure) {
        printf("Error in Opening Files\n");
        return e_failure;
    } else {
        printf("Successfully Opened Files\n");
    }

    // check check_capacity(encInfo) is returning e_success or not
    if (check_capacity(encInfo) == e_failure) {
        printf("Insufficient Capacity to Encode\n");
        return e_failure;
    } else {
        printf("Sufficient Capacity to Encode\n");
    }

    // check copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) success or not
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) {
        printf("Successfully Copied bmp Header\n");
    }

    encode_magic_string(MAGIC_STRING, encInfo);

    encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);

    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);

    encode_secret_file_size(encInfo->size_secret_file, encInfo);

    encode_secret_file_data(encInfo);

    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    return e_success;

}

