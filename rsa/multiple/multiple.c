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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h> 
#include "multiple.h"

#define MAX_CHAR            128 //7 bit representation
#define LENGTH_DECIMAL      50 //i.e. a decimal number with 50 digits
#define LENGTH              ((LENGTH_DECIMAL / ((int) log10((float) RADIX))))

//Internal function prototypes
void random_number(mp_ptr dst, int max_len, int seed);
void char2num(mp_ptr dst, char *string, int *index, int max_index, int numChar);
void num2char(mp_ptr n, char *string, int *index, int numChar);

//External functions
void multiple_generate_keys(multiple_rsa_t *rsa)
{
    mp_t phi, tmp1, tmp2, i;
    //Generate prime numbers p and q. They must be different!
    mp_init(rsa->p, LENGTH, 0); random_prime(rsa->p, (int)time(NULL), 100);
    mp_init(rsa->q, LENGTH, 0); random_prime(rsa->q, rand() % rsa->p->value[0], 100);
    while(mp_compare(rsa->q, rsa->p) == 0)
        random_prime(rsa->q, rand() % 100, 100);

    //Calculate the modulus, n
    mp_init(rsa->n, 2*LENGTH, 0); 
    mp_multiply(rsa->n, rsa->p, rsa->q);

    rsa->numChar = 2*(rsa->n->len - 1);

    //Find the totients of product
    mp_init(phi, 2*LENGTH, 0);
    mp_increment(rsa->p, -1);
    mp_increment(rsa->q, -1);
    mp_multiply(phi, rsa->p, rsa->q);
    mp_increment(rsa->p, 1); //Restore original prime
    mp_increment(rsa->q, 1); //Restore original prime

    //Find exponent e, which is part of the public key
    mp_init(tmp1, 2*LENGTH, 0);
    mp_assign_s64(tmp1, 15); //Some initial value
    while(mp_is_coprime(phi, tmp1) != 1)
        mp_increment(tmp1, 2);
    mp_init(rsa->e, tmp1->len, 0); mp_assign(rsa->e, tmp1);

    //Find exponent d, which is part of the private key
    mp_init(i, 2*LENGTH, 0);
    mp_init(tmp2, 2*LENGTH, 0);
    mp_assign_s64(i, 0);
    do
    {        
        mp_increment(i, 1);
        mp_multiply(tmp1, i, phi);
        mp_increment(tmp1, 1);        
        mp_mod(tmp2, tmp1, rsa->e);
    } while(tmp2->len > 0);    //fast way of checking if tmp2 != 0
    mp_divide(tmp2, tmp1, rsa->e);
    mp_init(rsa->d, tmp2->len, 0); 
    mp_assign(rsa->d, tmp2);

    mp_free_n(4, phi, tmp1, tmp2, i);
}

char *multiple_encrypt_message(multiple_rsa_t *rsa, char *message, int length_in, int *length_out)
{
    int index1, index2;
    char *ciphertext = NULL;
    //Allocate memory
    if((ciphertext = (char *)malloc(2*length_in)) == NULL) //Not actually 2xlength_in, but needs to be bigger than length_in due to "rsa->numChar + 1" in num2char hack
        { printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
    rsa->numChar = 2*(rsa->n->len - 1);
    index1 = 0; index2 = 0;
    while(index1 < length_in) //encode terminating character as well??
    {
        mp_t m, c; mp_init(m, rsa->n->max_len, 1); mp_init(c, rsa->n->max_len, 1);
        //Turn characters into integer
        char2num(m, message, &index1, length_in, rsa->numChar);
        //Encrypt integer
        mp_modexp(c, m, rsa->e, rsa->n);
        //Convert integer to ciphertext
        num2char(c, ciphertext, &index2, rsa->numChar + 1);
        mp_free_n(2, m, c);
    } 
    *length_out = index2;
    return ciphertext;
}

char *multiple_decrypt_message(multiple_rsa_t *rsa, char *ciphertext, int length_in, int *length_out)
{
    int index1, index2;
    char *message = NULL;
    //Allocate memory
    if((message = (char *)malloc(length_in)) == NULL)
        { printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
    rsa->numChar = 2*(rsa->n->len - 1);
    index1 = 0; index2 = 0;
    while(index1 < length_in)
    {
        mp_t m, c; mp_init(m, rsa->n->max_len, 1); mp_init(c, rsa->n->max_len, 1);
        //Turn characters into integer
        char2num(c, ciphertext, &index1, length_in, rsa->numChar + 1);
        //Decrypt integer        
        mp_modexp(m, c, rsa->d, rsa->n);
        //Convert integer to message
        num2char(m, message, &index2, rsa->numChar);
        mp_free_n(2, c, m);
    };
    *length_out = index2;
    return message;
}

//Internal functions
void random_number(mp_ptr dst, int max_len, int seed)
{
    int n;
    srand(seed);
    mp_zero(dst);
    max_len = (dst->max_len < max_len) ? dst->max_len : max_len; //make sure dst->max_len > max_len
    while(dst->len < max_len)
    {
        n = rand()%RADIX;
        while(n > 0 && dst->len < max_len) 
        {
            dst->value[dst->len++] = n % RADIX;
            n /= RADIX;
        }
    }
    mp_length(dst);
    if(dst->len == 0) dst->value[dst->len++] = 1; //dont want generate a zero valued random number
}

void random_prime(mp_ptr dst, int seed, int iterations)
{
    int i, primality, x;
    random_number(dst, dst->max_len, seed);
    if(mp_is_even(dst) == 1) mp_increment(dst, 1); //Make random number odd
    do
    {        
        primality = 1; //Suppose n is prime tentatively
        for(i = 0; i < iterations; i++)
        {
            mp_t a; int len;
            mp_init(a, dst->max_len, 0);
            do
            {
                len = rand() % dst->len;
            } while(len == 0); //ensure dont create a zero length random number!
            random_number(a, len, i);
            x = mp_J(a, dst);
            if(x != 0) //check for primality
            {
                mp_t tmp1, tmp2, tmp3;
                mp_init(tmp1, dst->max_len, 0);
                mp_init(tmp2, dst->max_len, 0);
                mp_init(tmp3, dst->max_len, 0);
                mp_assign(tmp1, dst);
                mp_increment(tmp1, -1);
                mp_assign_s64(tmp2, 2);
                mp_divide(tmp3, tmp1, tmp2);
                mp_assign(tmp1, dst); //Do again to restore tmp1
                mp_increment(tmp1, -1); //Do again to restore tmp1
                mp_modexp(tmp2, a, tmp3, dst);
                if(! ((mp_compare(tmp2, tmp1) == 0 && x == -1) || (x == 1 && (tmp2->len == 1 && tmp2->value[0] == 1))) ) //if x != y
                    { primality = 0; mp_free_n(3, tmp1, tmp2, tmp3); break; }
                mp_free_n(3, tmp1, tmp2, tmp3);
            }
            else { primality = 0; mp_free(a); break; }
            mp_free(a);
         }
        mp_increment(dst, 2); //Find next odd number
    } while(primality == 0);
    mp_increment(dst, -2);
}

void char2num(mp_ptr dst, char *string, int *index, int max_index, int numChar)
{
    int i, run = 1;
    mp_zero(dst);
    for(i = 0; i < numChar && *index < max_index; i += 2)
    {
        char a = string[*index];
        *index = *index + 1;
        if(*index == max_index)
            dst->value[dst->len] = a;
        else
        {
            char b = string[*index];
            *index = *index + 1;
            dst->value[dst->len] = a + MAX_CHAR * b;
        }    
        dst->len++;
    }
}

void num2char(mp_ptr n, char *string, int *index, int numChar)
{
    int i, j;
    for(i = 0, j = 0; i < numChar; i += 2, j++)
    {
        int tmp = n->value[j];
        string[*index] = (char) (tmp % MAX_CHAR);
        *index = *index + 1;
        tmp /= MAX_CHAR;
        string[*index] = (char) (tmp % MAX_CHAR);
        *index = *index + 1;
    }
}
