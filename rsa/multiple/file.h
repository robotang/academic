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

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiple.h"

void file_init(FILE **file, char *fileName, char *params);
void file_writekeys(multiple_rsa_t *rsa, char *publickey, char *privatekey);
void file_read_publickey(multiple_rsa_t *rsa, char *publickey);
void file_read_privatekey(multiple_rsa_t *rsa, char *privatekey);
void file_writeln(FILE **file, char *line, char *termination);
void file_write(FILE **file, char *line, int length);
char *file_read(FILE **file, int *length);
char *file_readln(FILE **file);
void file_close(FILE **file);

#endif
