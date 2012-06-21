/*
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

#include "file.h"

#define MAXLEN 100

void file_init(FILE **file, char *fileName, char *params)
{
    *file = fopen(fileName, params);
    if(*file == NULL)
    {
        printf("File '%s' does not exist!\r\n", fileName);
        exit(0);
    }
}

void file_writekeys(single_rsa_t *rsa)
{
    FILE *file;
    char *tmp;
    
    //Write public key, in format "<e>\n<n>"
    file_init(&file, "publickey", "w");
    tmp = num2charIO(rsa->e); file_writeln(&file, tmp, "\n"); free(tmp);
    tmp = num2charIO(rsa->n); file_writeln(&file, tmp, "\n"); free(tmp);
    file_close(&file);

    //Write private key, in format "<d>\n<n>"
    file_init(&file, "privatekey", "w");
    tmp = num2charIO(rsa->d); file_writeln(&file, tmp, "\n"); free(tmp);
    tmp = num2charIO(rsa->n); file_writeln(&file, tmp, "\n"); free(tmp);
    file_close(&file);

    //Write additional information (prime numbers etc)
    //Format "<p>\n<q>\n<numChar>\n"
    file_init(&file, "misc", "w");
    tmp = num2charIO(rsa->p); file_writeln(&file, tmp, "\n"); free(tmp);
    tmp = num2charIO(rsa->q); file_writeln(&file, tmp, "\n"); free(tmp);
    tmp = num2charIO(rsa->numChar); file_writeln(&file, tmp, "\n"); free(tmp);
    file_close(&file);
}

void file_readkeys(single_rsa_t *rsa)
{
    FILE *file;
    char *tmp;
    
    //Read public key, in format "<e>\n<n>"
    file_init(&file, "publickey", "r");
    tmp = file_readln(&file); rsa->e = char2numIO(tmp); free(tmp);
    tmp = file_readln(&file); rsa->n = char2numIO(tmp); free(tmp);
    file_close(&file);
    //Read private key, in format "<d>\n<n>"
    file_init(&file, "privatekey", "r");
    tmp = file_readln(&file); rsa->d = char2numIO(tmp); free(tmp);
    file_close(&file);
}

void file_writeln(FILE **file, unsigned char *line, char *termination)
{
    fprintf(*file, "%s%s", line, termination);
}

//Binary write to file
void file_write(FILE **file, unsigned char *line, int length)
{
    fwrite(line, 1, length, *file);
}

/* 
This function "file_readln" currently reads the entire file and returns it as a big string!
This is done for convienence, particularly since no termination chars are being
inserted for encrypted messages. The only downside to this method is that it uses
up a lot of memory if the files its dealing with are large! 
*/
unsigned char *file_read(FILE **file, int *length)
{
    unsigned char *readln;
    int fileSize = 0;
    
    fseek(*file, 0, SEEK_END);
    fileSize = ftell(*file);
    rewind(*file);
    
    readln = (unsigned char *) calloc(fileSize+1, sizeof(unsigned char));
    fread(readln, 1, fileSize, *file);

    *length = fileSize;
    return readln;
}

unsigned char *file_readln(FILE **file)
{
    unsigned char *line; char *tmp;
    if((line = (unsigned char *)malloc(MAXLEN)) == NULL)
        printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__);
    tmp = fgets(line, MAXLEN, *file);
    if(tmp == NULL) line = NULL;
    return line;
}

void file_close(FILE **file)
{    
    fclose(*file);
}
