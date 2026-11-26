//Name:- Tejaswi M N 26010_092 , Steganography is the practice of hiding secret information within images, audio, videos, or text to keep communication private and undetected.

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{


    if (argc < 4) {
        printf("Usage: ./a.out <operation_flag> <src_path> <secret_path> (optional <dest_path>)\n");
        return 0;
    }

    if (argv[1] != NULL) {
        if (check_operation_type(argv[1]) == e_encode) {
            EncodeInfo encInfo;

            if(read_and_validate_encode_args(argv, &encInfo) == e_success) {
                printf("Successfully Read and Validated the User Input\n");

                //encode operation
                Status flag = do_encoding(&encInfo);
                if (flag == e_success) {
                    printf("Successfully Encoded the User Input\n");
                } else if(flag == e_failure) {
                    printf("Error in Encoding the User Input\n");
                }

            } else {
                printf("Error in Reading or Validating the User Input\n");
            }

        } else if (check_operation_type(argv[1]) == e_decode) {
            //decode logic
            DecodeInfo decInfo;

            if(read_and_validate_decode_args(argv, &decInfo) == e_success) {
                printf("Successfully Read and Validated the User Input\n");

                //encode operation
                Status flag = do_decoding(&decInfo);
                if (flag == e_success) {
                    printf("Successfully Decoded the User Input\n");
                } else if(flag == e_failure) {
                    printf("Error in Decoding the User Input\n");
                }

            } else {
                printf("Error in Reading or Validating the User Input\n");
            }

        } else if(check_operation_type(argv[1]) == e_unsupported) {
            //unsupported logic
        }
    }
    return 0;
}

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e")==0) {
        return e_encode;
    }

    else if (strcmp(symbol, "-d")==0) {
        return e_decode;
    }

    return e_unsupported;
}
