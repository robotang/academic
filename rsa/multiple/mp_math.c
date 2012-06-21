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
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "mp_math.h"

void mp_init(mp_ptr n, int max_length, int zero)
{    
    n->max_len = max_length;
    n->value = NULL;
    if((n->value = (int *)malloc(n->max_len*sizeof(int))) == NULL)
        { printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
    n->len = 0;
    if(zero == 1)
        mp_zero(n);
}

void mp_zero(mp_ptr n)
{
    int i;
    for(i = 0; i < n->max_len; i++)
        n->value[i] = 0;
    n->len = 0;
}

void mp_free(mp_ptr n)
{
    free(n->value);
}

void mp_free_n(int num, ...)
{
    int i; va_list ap; va_start(ap, num); 
    for(i = 0; i < num; i++) mp_free(va_arg(ap, mp_ptr));
    va_end(ap);
}

void mp_swap(mp_ptr a, mp_ptr b)
{
    mp_t tmp;
    mp_init(tmp, a->max_len > b->max_len ? a->max_len : b->max_len, 0);
    mp_assign(tmp, a);
    mp_assign(a, b);
    mp_assign(b, tmp);
    mp_free(tmp);
}

void mp_char2numIO(mp_ptr dst, char *string)
{
    int i, j, len = ((int) strlen(string) + 1) / (MAX_LEN_RADIX + 1);
    if(dst->value == NULL) //need to setup
    {
        mp_init(dst, len, 0);
    }
    if(dst->max_len < len) //reallocate
    {
        if((dst->value = (int *)realloc(dst->value, len*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else dst->max_len = len;
    }
    mp_zero(dst);
    for(i = 0, j = len-1; i < (int) strlen(string) && j >= 0; i += MAX_LEN_RADIX + 1, j--)
    {
        char tmp[MAX_LEN_RADIX+1];
        strncpy(tmp, string + i, MAX_LEN_RADIX);
        tmp[MAX_LEN_RADIX] = '\0';
        dst->value[j] = atoi(tmp);
    }
    mp_length(dst);
}

char *mp_num2charIO(mp_ptr n)
{
    int i, j;
    char *string = NULL;
    if((string = (char *)malloc(n->max_len*(MAX_LEN_RADIX+1) + 1)) == NULL)
        { printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
    for(i = n->max_len-1, j = 0; i >= 0; i--, j += MAX_LEN_RADIX + 1) //print out leading zeros as well, and print MSB first
    {
        if(i > 0)
            sprintf(string + j, "%05d-", n->value[i]);
        else
            sprintf(string + j, "%05d\n", n->value[i]);
    }
    return string;
}

void mp_print(mp_ptr n)
{
    //Prints array out, in its original radix, with dashes separating the elements
    char *string = mp_num2charIO(n);
    printf("%s", string);
    free(string);
}

void mp_print_n(int num, ...)
{
    int i; va_list ap; va_start(ap, num); 
    for(i = 0; i < num; i++) 
        mp_print(va_arg(ap, mp_ptr));
    va_end(ap);
    printf("\r\n");
}

void mp_assign(mp_ptr dst, mp_ptr src)
{
    int i;
    if(dst->max_len < src->len) //reallocate
    {
        if((dst->value = (int *)realloc(dst->value, src->len*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else dst->max_len = src->len;
    }
    mp_zero(dst);
    dst->len = src->len;
    for(i = 0; i < src->len; i++)
        dst->value[i] = src->value[i];
}

void mp_assign_s64(mp_ptr dst, s64_t src) //Note: does not check if dst is large enough!
{
    int i, tmp; s64_t cpy;
    mp_zero(dst); 
    cpy = src;
    i = 0;
    while(cpy > 0)
    {
        tmp = (int) (cpy % RADIX);
        dst->value[i++] = tmp;
        cpy /= RADIX;
    }
    dst->len = i;
}

int mp_is_zero(mp_ptr n)
{
    int i;
    for(i = n->len-1; i >= 0; i--) { if(n->value[i] != 0) break; }
    if(i < 0) return 1;
    else return 0;
}

int mp_is_even(mp_ptr n)
{
    if(n->value[0] % 2 == 0) return 1;
    else return 0;
}

int mp_length(mp_ptr n)
{
    int i;
    for(i = n->max_len-1; i >= 0; i--) 
        { if(n->value[i] != 0) break; }
    n->len = i+1;
    return n->len;
}

int mp_compare(mp_ptr a, mp_ptr b)
{
    int i;
    int ret = 0;
    for(i = (a->len > b->len) ? a->len-1: b->len-1; i >= 0; i--)
    {
        if(a->value[i] > b->value[i]) { ret = 1; break; }
        else if(a->value[i] < b->value[i]) { ret = -1; break; }
    }
    return ret;
}

void mp_increment(mp_ptr n, int increment)
{
    int i, j;    
    j = increment;
    for(i = 0; i < n->len; i++)
    {
        j += n->value[i];
        if(j >= RADIX) { n->value[i] = j - RADIX; j = 1; }
        else if(j < 0) { n->value[i] = RADIX - abs(j); j = -1; }
        else { n->value[i] = j; j = 0; break; }
    }
    if(j == 1) { n->value[i] = j; n->len++; }
    else if(j == 0 && increment < 0 && n->value[n->len-1] == 0) n->len--;
}

void mp_add(mp_ptr dst, mp_ptr a, mp_ptr b)
{
    int i, j, res, len;
    len = (a->len > b->len) ? a->len : b->len;
    if(dst->max_len < len + 1) //reallocate
    {
        if((dst->value = (int *)realloc(dst->value, (len + 1)*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else dst->max_len = len + 1;
    }
    mp_zero(dst);
    j = 0;
    for(i = 0; i < len || (a->value[i] == 0 && b->value[i] == 0); i++)
    {
        j += a->value[i] + b->value[i];
        if(j >= RADIX) { dst->value[i] = j - RADIX; j = 1; }
        else { dst->value[i] = j; j = 0; }
    }
    if(j != 0) dst->value[i] = j;
    res = mp_length(dst);
}

void mp_multiply(mp_ptr dst, mp_ptr a, mp_ptr b)
{
    int i,j, res;    
    if((a->len+b->len) > dst->max_len) //reallocate
    {
        if((dst->value = (int *)realloc(dst->value, (a->len + b->len)*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else dst->max_len = a->len + b->len;
    }
    mp_zero(dst);
    for(i = 0; i < a->len; i++) 
    {  
        for (j = 0; j < b->len; j++) 
        {
            dst->value[i+j+1] = dst->value[i+j+1] + (dst->value[i+j] + a->value[i] * b->value[j]) / RADIX;
            dst->value[i+j] = (dst->value[i+j] + a->value[i] * b->value[j]) % RADIX;
        }
    }
    res = mp_length(dst);
}

//Adapted from course notes
void mp_divide(mp_ptr dst, mp_ptr a, mp_ptr b)
{
    int i, j, q, x, res;
    int a1[100];
    while(b->value[b->len-1] == 0) b->len--;
    if(a->len == a->max_len) //reallocate
    {
        if((a->value = (int *)realloc(a->value, (a->max_len + 1)*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else a->max_len += 1;
    }
    a->value[a->len] = 0; 
    a->len++;
    if(dst->max_len < a->len) //reallocate
    {
        if((dst->value = (int *)realloc(dst->value, a->len*sizeof(int))) == NULL)
            { printf("realloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
        else dst->max_len = a->len;
    }
    mp_zero(dst);
    for(j = a->len - b->len - 1; j >= 0; j--) 
    {
        x = RADIX * a->value[j + b->len] + a->value[j + b->len-1];   //     when a1[j+i] is negative, d = -a1[j+i] deficit  
        a->value[j + b->len-1] = x;            //     ..-d....-2r...-r....0....r.......
        a->value[j + b->len] = 0;              //     |  | |    |    |    |    |
        q = x / b->value[b->len - 1] + 1; //Guess q
        do
        {
            for(i = b->len - 1; i >= 0; i--) a1[j+i] = a->value[j+i];
            q--; // if q is too large, decrease it by 1
            for(i = b->len - 1; i >= 0; i--) a1[j+i] = a1[j+i] - q*b->value[i];
            for(i = 0; i <= b->len - 2;i++) // Try to subtract q*b from a
            { 
                if(a1[j+i] < 0)
                {
                    a1[j+i+1] = a1[j+i+1] - (RADIX-1 - a1[i+j])/RADIX; //borrow from left
                    a1[j+i] = a1[j+i] + ((RADIX-1 - a1[i+j])/RADIX)*RADIX; //remaining at i+j
                }
            }
        } while(a1[j+b->len - 1] < 0);
        dst->value[j] = q;
        for(i = b->len - 1; i >= 0; i--) a->value[j+i] = a1[j+i];
    }
    res = mp_length(a); //update length of remainder
    res = mp_length(dst);
}

void mp_mod(mp_ptr dst, mp_ptr a, mp_ptr b)
{
    mp_t tmp;
    mp_init(tmp, a->max_len, 0);
    mp_assign(tmp, a);
    mp_divide(dst, tmp, b);
    mp_assign(dst, tmp);
    mp_free(tmp);
}

int mp_is_coprime(mp_ptr a, mp_ptr b) //finds if gcd(a, b) == 1
{
    int ret = 0;
    int max_len = (a->max_len > b->max_len) ? a->max_len : b->max_len;
    mp_t q, w0, w1;
    mp_init(q, max_len, 0); 
    mp_init(w0, max_len, 0); mp_assign(w0, a);
    mp_init(w1, max_len, 0); mp_assign(w1, b);
    while(w1->len > 0 && !(w1->len == 1 && w1->value[0] == 1))
        { mp_divide(q, w0, w1); mp_swap(w0, w1); }
    if(w1->value[0] == 1) ret = 1;
    mp_free_n(3, q, w0, w1);
    return ret;
}

void mp_modexp(mp_ptr dst, mp_ptr x, mp_ptr e, mp_ptr n)
{
    int i, m, b_len; int *b = NULL; 
    mp_t e_cpy, tmp1, tmp2;
    mp_init(e_cpy, dst->max_len, 0); mp_assign(e_cpy, e);
    mp_init(tmp1, dst->max_len, 0); 
    mp_init(tmp2, dst->max_len, 0); mp_assign_s64(tmp2, 2);

    i = 0;
    //Allocate memory
    b_len = (int) (log10((float) RADIX) / log10(2.0)) * dst->max_len + 10; //should be long enough!
    if((b = (int *)malloc(b_len*sizeof(int))) == NULL)
        { printf("malloc failed: [%s, %d]\n", __FILE__, __LINE__); exit(0); }
    while(e_cpy->len > 0) // fast way of checking that e > 0
    {
        if(mp_is_even(e_cpy) == 1) b[i] = 0; 
        else b[i] = 1;
        mp_divide(tmp1, e_cpy, tmp2);
        mp_assign(e_cpy, tmp1);
        i++;
    }
    m = i - 1;
    mp_assign_s64(dst, 1);
    for(i = m; i >= 0; i--)
    {
        mp_assign(tmp1, dst);        
        mp_multiply(tmp2, tmp1, dst);
        mp_mod(dst, tmp2, n);
        if(b[i]==1)
        {
            mp_assign(tmp1, dst);
            mp_multiply(tmp2, dst, x);
            mp_mod(dst, tmp2, n);
        }
    }
    mp_free_n(3, e_cpy, tmp1, tmp2);
    free(b);
}

int _J(mp_ptr a, mp_ptr n)
{
    if(a->len == 0) //fast way to check if a == 0
        return 0;
    else if(a->len == 1 && a->value[0] == 1) //fast way to check if a == 1
        return 1;
    else if(mp_is_even(a))
    {
        int sign, max_len;
        mp_t tmp1, tmp2, tmp3;
        max_len = (a->max_len > n->max_len) ? a->max_len : n->max_len;
        mp_init(tmp1, max_len, 0); mp_assign(tmp1, n);
        mp_init(tmp2, max_len, 0); mp_assign_s64(tmp2, 8);
        mp_init(tmp3, max_len, 0); 
        /* Find sign */
        mp_multiply(tmp3, tmp1, n);
        mp_increment(tmp3, -1);        
        mp_divide(tmp1, tmp3, tmp2);
        sign = (mp_is_even(tmp1) == 1) ? 1 : -1;
        /* Find a/2 */
        mp_assign_s64(tmp1, 2);
        mp_assign(tmp2, a);
        mp_divide(a, tmp2, tmp1);
        mp_free_n(3, tmp1, tmp2, tmp3);
        return _J(a, n)*sign;
    }
    else
    {
        int sign, max_len;
        mp_t tmp1, tmp2, tmp3;
        max_len = (a->max_len > n->max_len) ? a->max_len : n->max_len;
        mp_init(tmp1, max_len, 0); mp_assign(tmp1, a);
        mp_init(tmp2, max_len, 0); mp_assign(tmp2, n);
        mp_init(tmp3, max_len, 0); 
        /* Find sign */
        mp_increment(tmp1, -1);
        mp_increment(tmp2, -1);
        mp_multiply(tmp3, tmp1, tmp2);
        mp_assign_s64(tmp2, 4);
        mp_divide(tmp1, tmp3, tmp2);
        sign = (mp_is_even(tmp1) == 1) ? 1 : -1;
        /* Find n%a */
        mp_assign(tmp1, a);
        mp_assign(tmp2, n);
        mp_mod(n, tmp2, tmp1);
        mp_free_n(3, tmp1, tmp2, tmp3);
        return _J(n, a)*sign;
    }
}

int mp_J(mp_ptr a, mp_ptr n)
{
    mp_t a_cpy, n_cpy;
    int result;
    mp_init(a_cpy, a->max_len, 0); mp_assign(a_cpy, a);
    mp_init(n_cpy, n->max_len, 0); mp_assign(n_cpy, n);
    result = _J(a_cpy, n_cpy);
    mp_free_n(2, a_cpy, n_cpy);
    return result;
}
