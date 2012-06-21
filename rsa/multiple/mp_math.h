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

#ifndef MP_MATH_H
#define MP_MATH_H

#define RADIX            16384 //2^14, i.e pack two 7 bit chars into one RADIX number
#define MAX_LEN_RADIX    5 //a 5 digit decimal number can represent any single RADIX number

typedef struct 
{
    int *value; //Dynamically allocated. Least significant val is stored in element[0]
    int max_len;
    int len;
} mp_struct;

typedef mp_struct mp_t[1];
typedef mp_struct *mp_ptr;
typedef long long int s64_t;

void mp_init(mp_ptr n, int max_length, int zero);
void mp_zero(mp_ptr n);
void mp_free(mp_ptr n);
void mp_free_n(int num, ...);
void mp_swap(mp_ptr a, mp_ptr b);
void mp_char2numIO(mp_ptr dst, char *string);
char *mp_num2charIO(mp_ptr n);
void mp_print(mp_ptr n);
void mp_print_n(int num, ...);
void mp_assign(mp_ptr dst, mp_ptr src);
void mp_assign_s64(mp_ptr dst, s64_t src);
int mp_is_zero(mp_ptr n);
int mp_is_even(mp_ptr n);
int mp_length(mp_ptr n);
int mp_compare(mp_ptr a, mp_ptr b);
void mp_increment(mp_ptr n, int increment); //has bugs when going from < 0 to > 0
void mp_add(mp_ptr dst, mp_ptr a, mp_ptr b);
void mp_multiply(mp_ptr dst, mp_ptr a, mp_ptr b);
void mp_divide(mp_ptr dst, mp_ptr a, mp_ptr b); //NOTE: a is changed to the remainder! // dst = a / b, remainder is put in a
void mp_mod(mp_ptr dst, mp_ptr a, mp_ptr b);
int mp_is_coprime(mp_ptr a, mp_ptr b); //finds if gcd(a, b) = 1
void mp_modexp(mp_ptr dst, mp_ptr x, mp_ptr e, mp_ptr n);
int mp_J(mp_ptr a, mp_ptr n);

#endif
