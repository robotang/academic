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

#ifndef SINGLE_H
#define SINGLE_H

typedef long long int sp_t;

typedef struct
{
    int p; //A prime number, is meant to be kept secret
    int q; //Another prime number, is meant to be kept secret
    sp_t n; //The modulus, part of the public and private keys
    sp_t e; //The exponent, part of the public key
    sp_t d; //The exponent, part of the private key, is meant to be kept secret
    int numChar; //The number of chars that can be packed
} single_rsa_t;

void single_generate_keys(single_rsa_t *rsa);
char *single_encrypt_message(single_rsa_t *rsa, unsigned char *message);
char *single_decrypt_message(single_rsa_t *rsa, unsigned char *ciphertext, int length);

sp_t char2numIO(unsigned char *string);
unsigned char *num2charIO(sp_t num);

#endif
