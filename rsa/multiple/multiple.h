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

#ifndef MULTIPLE_H
#define MULTIPLE_H

#include "mp_math.h"

typedef struct
{
    mp_t p; //A prime number, is meant to be kept secret
    mp_t q; //Another prime number, is meant to be kept secret
    mp_t n; //The modulus, part of the public and private keys
    mp_t e; //The exponent, part of the public key
    mp_t d; //The exponent, part of the private key, is meant to be kept secret
    int numChar; //The number of chars that can be packed.
} multiple_rsa_t;

void random_prime(mp_ptr dst, int seed, int iterations);
void multiple_generate_keys(multiple_rsa_t *rsa);
char *multiple_encrypt_message(multiple_rsa_t *rsa, char *message, int length_in, int *length_out);
char *multiple_decrypt_message(multiple_rsa_t *rsa, char *ciphertext, int length_in, int *length_out);

#endif
