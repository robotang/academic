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

#include "single.h"

#define MAXLEN 100

#define MESSAGE_TERMINATION            '\0' //Null character is used to terminate readable strings

//Function prototypes
int random_prime(int seed, int iterations, int min, int max);
sp_t J(sp_t a, sp_t n); 
sp_t modexp(sp_t x, sp_t e, sp_t n);
int charPackingMax(sp_t n);
sp_t char2num(unsigned char *string, int *index, int numChar, int isMessage);
void num2char(sp_t num, unsigned char *string, int *index, int numChar, int isMessage);
int gcd(int a, int b);

//External functions
void single_generate_keys(single_rsa_t *rsa)
{
    int i; int phi, tmp;

    //Generate prime numbers p and q. They must be different!
    rsa->p = random_prime((int)time(NULL), 100, 10000, 12000);
    rsa->q = random_prime(rsa->p, 100, 10000, 12000);
    while(rsa->q == rsa->p)
        rsa->q = random_prime(rsa->q, 100, 10000, 12000);

    //Calculate the modulus, n
    rsa->n = (rsa->p)*(rsa->q);

    rsa->numChar = charPackingMax(rsa->n);

    //printf("p: %d, q: %d, n: %lld, num: %d\r\n", rsa->p, rsa->q, rsa->n, rsa->numChar);

    //Find the totients of product
    phi = rsa->n - (rsa->p + rsa->q) + 1; //Or (rsa->p - 1) * (rsa->q - 1);

    //Find exponent e, which is part of the public key
    tmp = 15; //Some initial value
    while(gcd(tmp, phi) != 1)
        tmp += 2;
    rsa->e = tmp;

    //Find exponent d, which is part of the private key
    i = 0;
    while((i * phi + 1) % rsa->e != 0)
        i++;
    rsa->d = (i * phi + 1) / rsa->e;
}

char *single_encrypt_message(single_rsa_t *rsa, unsigned char *message)
{
    int index1, index2; int m, c;
    unsigned char *ciphertext = NULL;
    //Allocate memory
    if((ciphertext = (unsigned char *)malloc(2*strlen(message+1))) == NULL)
        printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__);

    rsa->numChar = charPackingMax(rsa->n);
    
    index1 = 0; index2 = 0;
    do
    {
        //Turn characters into integer
        m = char2num(message, &index1, rsa->numChar, 1);
        //Encrypt integer
        c = modexp(m, rsa->e, rsa->n);
        //Convert integer to ciphertext
        num2char(c, ciphertext, &index2, 2*(rsa->numChar), 0);
    } while(message[index1-1] != MESSAGE_TERMINATION); 

    return ciphertext;
}

char *single_decrypt_message(single_rsa_t *rsa, unsigned char *ciphertext, int length)
{
    int index1, index2; int m, c;
    unsigned char *message = NULL;
    //Allocate memory
    if((message = (unsigned char *)malloc(length / 2)) == NULL)
        printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__);

    rsa->numChar = charPackingMax(rsa->n);
    
    index1 = 0; index2 = 0;
    do
    {
        //Turn characters into integer
        c = char2num(ciphertext, &index1, 2*(rsa->numChar), 0);
        //Decrypt integer
        m = modexp(c, rsa->d, rsa->n);
        //Convert integer to message
        num2char(m, message, &index2, rsa->numChar, 1);
    } while(m != MESSAGE_TERMINATION);

    //Terminate message
    message[index2] = MESSAGE_TERMINATION;

    return message;
}

//Internal functions
//Adapted from course notes
int random_prime(int seed, int iterations, int min, int max)
{
    int i, x, y, a, n;
    int primality;

    srand(seed);
    n=rand()%(max-min) + min; // Choose an initial candidate for prime
    if((n/2)*2==n)n++; // If n is even, add 1
    do
    {
        primality=1; //Suppose n is prime tentatively
        for(i=1; i<=100; i++)
        {
            a=rand()%n;
            x=J(a,n);
            if(x!=0)
            {
                y=modexp(a, (n-1)/2, n);
                if(y==n-1)y=-1;
                if(x!=y) { primality=0; break; }
            } 
            else { primality=0; break; }
        } 
        n=n+2; //Take the next odd number
    } while(primality==0);

    return n - 2;    
}

//Adapted from course notes
sp_t J(sp_t a, sp_t n)
{
    if(a==0)return 0;
    else if(a==1)return 1;
    else if((a/2)*2==a)return J(a/2, n)*modexp(-1, (n*n-1)/8, n);
    else return J(n % a, a)*modexp(-1, (a-1)*(n-1)/4, n);
}

//Adapted from course notes
sp_t modexp(sp_t x, sp_t e, sp_t n)
{
    int i, m; int b[100]; sp_t y;
    i=0;
    while(e>0)
    {
        if((e/2)*2==e) b[i]=0; else b[i]=1;
        e=e/2; i++;
    }
    
    m=i-1; y=1; 
    for(i=m;i>=0;i--)
    {
        y=y*y; y=y%n; if(b[i]==1){y=y*x; y=y%n;}
    }
    return y;
}

int charPackingMax(sp_t n)
{    
    int num = 0;
    //Find the number of chars that can be packed into an integer    
    while((n >> 8) > 0)
    {
        n = n >> 8;
        num++;
    }
    return num;
}

sp_t char2num(unsigned char *string, int *index, int numChar, int isMessage)
{
    int i; sp_t m; m = 0;
    for(i = 0; i < numChar; i++)
    {
        m += (string[*index] << 8*i);
        *index = *index + 1;
        if(string[*index-1] == MESSAGE_TERMINATION && isMessage > 0)
            break;        
    }
    return m;
}

void num2char(sp_t num, unsigned char *string, int *index, int numChar, int isMessage)
{
    int i;
    for(i = numChar-1; i >= 0; i--) //Go through all -> automatically zero pack string
    {        
        string[*index+i] = (num >> 8*i); //+i to put in correct order
        num -= ((int)string[*index+i] << 8*i);
        //if(num == MESSAGE_TERMINATION && isMessage > 0) //Done after adding to terminate string if is a message
        //    break; /* Commented out - it doesnt matter if the string is a message is 'padded' by null terminators! */
    }
    *index = *index + numChar;
}

int gcd(int a, int b)
{
    if(b==0)
        return a;
    else
        return gcd(b, a%b);
}

//For importing sp_t numbers from strings, used in file IO 
sp_t char2numIO(unsigned char *string)
{
    return atol(string);
}

//For exporting sp_t numbers as strings, used in file IO
unsigned char *num2charIO(sp_t num)
{
    unsigned char *string;
    if((string = (unsigned char *)malloc(MAXLEN)) == NULL)
        printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__);
    sprintf(string, "%lld", num); //ltoa(num, string, 10);
    return string;
}
