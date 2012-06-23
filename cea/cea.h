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

#ifndef CEA_H
#define CEA_H

#include "global.h"
#include "list.h"

#define CEA_ANY        '?'
#define CEA_NONE    '0'

typedef char *cea_classes_t;
typedef char cea_class_t;
typedef char *cea_attributes_t[];

typedef struct
{
    list_t sset;
    list_t gset;
    list_t vs;
    cea_class_t cea_class;
    int num_attributes;
    bool_t update_vs;
} cea_t;

int cea_init(cea_t *cea, cea_class_t cea_class, const int num_attributes);
int cea_train(cea_t *cea, const char *data, cea_attributes_t attributes);
int cea_classify(cea_t *cea, int num_classes, const char *example, cea_attributes_t attributes, FILE *output);
void cea_print(const cea_t *cea, FILE *output, bool_t print_sset);

#endif
