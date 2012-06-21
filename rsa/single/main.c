/*
 *  An implementation of the RSA public-key encryption scheme (single precision).
 *
 *  Note that this code was for an academic exercise. If you are after
 *  a proper RSA implementation, see <www.gnu.org/s/gnu-crypto/>.
 *
 *  Copyright (C) 2010, Robert Tang <opensource@robotang.co.nz>
 *
 *  This is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public Licence
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "single.h"
#include "file.h"

void printUsage(void)
{
    printf("There are three modes in this RSA application.\r\n\n");
    printf("1. To generate keys, eg: ./rsa -genkeys\r\n");
    printf("2. To encrypt files, eg: ./rsa -encrypt file\r\n");
    printf("3. To decrypt files, eg: ./rsa -decrypt file\r\n\n");
    printf("Note: you need to generate keys before encryption can be done.\r\n");
}

typedef enum {genkeys, encrypt, decrypt} mode_t;

#if 1
int main(int argc, char *argv[])
{
    FILE *input, *output;
    char fileName[100], fileOut[100]; //should be long enough for a file name!
    unsigned char *readln, *writeln;
    mode_t mode; int i, length;
    single_rsa_t rsa;

    if(argc <= 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "man") == 0 ||
        strcmp(argv[1], "-help") == 0) 
        { printUsage(); exit(1); }

    for(i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-genkeys") == 0)
            mode = genkeys;
        else if(strcmp(argv[i], "-encrypt") == 0)
        {         
            mode = encrypt; 
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            strcpy(fileName, argv[i+1]); 
        }
        else if(strcmp(argv[i], "-decrypt") == 0)
        { 
            mode = decrypt; 
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            strcpy(fileName, argv[i+1]); 
        }
    }

    if(mode == genkeys)
    {
        single_generate_keys(&rsa);
        file_writekeys(&rsa);
    }
    else if(mode == encrypt)
    {
        //Setup file to read from and read keys
        file_init(&input, fileName, "r");
        file_readkeys(&rsa);
        //Get text to encrypt
        length = 0;
        readln = file_read(&input, &length);
        //Encrypt text
        writeln = single_encrypt_message(&rsa, readln);
        //Setup file to write encrypted text to and write to file
        strcpy(fileOut, "encrypt_"); strcat(fileOut, fileName);
        file_init(&output, fileOut, "wb");
        file_write(&output, writeln, 2*(strlen(readln)+1));            
        //Cleanup
        free(readln); /* free(writeln); */ //Doesnt seem to want to free writeln!
        file_close(&input); file_close(&output);
    }
    else if(mode == decrypt)
    {
        //Setup file to read from and read keys
        file_init(&input, fileName, "rb");
        file_readkeys(&rsa);
        //Get text to decrypt
        length = 0;
        readln = file_read(&input, &length);
        //Decrypt text
        writeln = single_decrypt_message(&rsa, readln, length);
        //Setup file to write decrypted text to and write to file
        strcpy(fileOut, "decrypt_"); strcat(fileOut, fileName);
        file_init(&output, fileOut, "w");
        file_writeln(&output, writeln, "");            
        //Cleanup
        free(readln); /* free(writeln); */ //Doesnt seem to want to free writeln!
        file_close(&input); file_close(&output);
    }

    return 1;
}

#else
unsigned char *message = "abcdefghijklmnopqrstuvwxyz";

int main(int argc, char **argv)
{
    single_rsa_t rsa;
    unsigned char *ciphertext, *recovered;

    single_generate_keys(&rsa);

    printf("Message: %s\r\n\n", message);
    ciphertext = single_encrypt_message(&rsa, message);
    //printf("Ciphertext: %s\r\n", ciphertext);
    recovered = single_decrypt_message(&rsa, ciphertext);
    printf("Recover: %s\r\n", recovered);

    getchar();

    return 1;
}
#endif
