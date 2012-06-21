/*
 *  An implementation of the RSA public-key encryption scheme (multiple precision).
 *
 *  Note that this code was for an academic exercise. If you are after
 *  a proper RSA implementation, see <www.gnu.org/s/gnu-crypto/> (and also the 
 *  GNU Bignum Library <http://gmplib.org/> for multiple precision arithmetic).
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
#include "mp_math.h"
#include "multiple.h"
#include "file.h"
#include "profile.h"

//#define PROFILE

void printUsage(void)
{
    printf("There are three modes in this RSA application.\r\n\n");
    printf("1. To generate keys, eg: ./rsa -genkeys <publickey> <privatekey>\r\n");
    printf("2. To encrypt files, eg: ./rsa -encrypt <file> -out <encrypt_file> -key <publickey>\r\n");
    printf("3. To decrypt files, eg: ./rsa -decrypt <file> -out <decrypt_file> -key <privatekey>\r\n\n");
    printf("Note: you need to generate keys before encryption can be done.\r\n");
    printf("Note: decrypt also has an optional -raw flag. This is to write the file as a binary file, rather than ASCII, which the decrypted text is actually binary.\r\n");
}

typedef enum {genkeys, encrypt, decrypt} mode_t;

#if 1
int main(int argc, char *argv[])
{
    FILE *input, *output;
    char *fileName = NULL, *fileOut = NULL, *publickey = NULL, *privatekey = NULL, *key = NULL;
    char *readln, *writeln;
    mode_t mode; int i, length_in = 0, length_out = 0, raw = 0;
    multiple_rsa_t rsa;
    #ifdef PROFILE
    profile_t p;
    #endif
    
    if(argc <= 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "man") == 0 ||
        strcmp(argv[1], "-help") == 0) 
        { printUsage(); exit(1); }

    for(i = 0; i < argc; i++)
    {        
        if(strcmp(argv[i], "-genkeys") == 0)
        {
            mode = genkeys;
            if(argv[i+1] == NULL || argv[i+2] == NULL) { printUsage(); exit(1); }
            publickey = argv[i+1];
            privatekey = argv[i+2];
        }
        else if(strcmp(argv[i], "-encrypt") == 0)
        {         
            mode = encrypt; 
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            fileName = argv[i+1]; 
        }
        else if(strcmp(argv[i], "-decrypt") == 0)
        { 
            mode = decrypt; 
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            fileName = argv[i+1]; 
        }
        else if(strcmp(argv[i], "-out") == 0)
        {
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            fileOut = argv[i+1];  
        }
        else if(strcmp(argv[i], "-key") == 0)
        {
            if(argv[i+1] == NULL) { printUsage(); exit(1); }
            key = argv[i+1];  
        }
        else if(strcmp(argv[i], "-raw") == 0)
        {
            raw = 1;  
        }
    }

    if(mode == genkeys)
    {
        #ifdef PROFILE
        profile_begin(&p);
        #endif
        multiple_generate_keys(&rsa);
        #ifdef PROFILE
        profile_end(&p, PRINT_MILLISECONDS);
        printf("milliseconds\n");
        #endif
        file_writekeys(&rsa, publickey, privatekey);
    }
    else if(mode == encrypt)
    {
        //Setup file to read from and read keys
        file_init(&input, fileName, "r");
        file_read_publickey(&rsa, key);
        //Get text to encrypt
        readln = file_read(&input, &length_in);
        //Encrypt text
        #ifdef PROFILE
        profile_begin(&p);        
        #endif
        writeln = multiple_encrypt_message(&rsa, readln, length_in, &length_out); //use this length instead
        #ifdef PROFILE
        profile_end(&p, PRINT_MILLISECONDS);
        printf("milliseconds\n");
        #endif
        //Setup file to write encrypted text to and write to file
        file_init(&output, fileOut, "wb");
        file_write(&output, writeln, length_out);            
        //Cleanup
        free(readln); free(writeln); //Doesnt seem to want to free writeln!
        file_close(&input); file_close(&output);
    }
    else if(mode == decrypt)
    {
        //Setup file to read from and read keys
        file_init(&input, fileName, "rb");
        file_read_privatekey(&rsa, key);
        //Get text to decrypt
        readln = file_read(&input, &length_in);
        //Decrypt text
        #ifdef PROFILE
        profile_begin(&p);
        #endif
        writeln = multiple_decrypt_message(&rsa, readln, length_in, &length_out);
        #ifdef PROFILE
        profile_end(&p, PRINT_MILLISECONDS);
        printf("milliseconds\n");
        #endif
        //Setup file to write decrypted text to and write to file
        if(raw == 1)
        {
            file_init(&output, fileOut, "wb");
            file_write(&output, writeln, length_out);
        }
        else
        {
            file_init(&output, fileOut, "w");
            file_writeln(&output, writeln, "");
        }
        
        //Cleanup
        free(readln); free(writeln);
        file_close(&input); file_close(&output);
    }

    return 1;
}
#else

int main(void)
{
    char *message = "abcdefghijklmnopqrstuvwxyz";
    char *ciphertext, *recovered;
    multiple_rsa_t rsa;
    int length;
    multiple_generate_keys(&rsa);
    mp_print_n(5, rsa.p, rsa.q, rsa.e, rsa.d, rsa.n);
    
    /* Encrypt */
    printf("Message: %s\r\n\n", message);
    ciphertext = multiple_encrypt_message(&rsa, message, &length);
    /* Decrypt */
    recovered = multiple_decrypt_message(&rsa, ciphertext, length);
    printf("Recover: %s\r\n", recovered);

    return 1;
}
#endif
