/*
 *  Copyright (C) 2008, Robert Tang <opensource@robotang.co.nz>
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

#ifndef LIST_H
#define LIST_H

#include "global.h"

/* Define the structure of the each of the nodes in a list */
typedef struct node_
{
    void *data;
    struct node_ *next;
} node_t;

/* Definition of the structure of the linked list */
typedef struct
{
    int size;
    int (*match)(const void *, const void *);
    void (*destroy)(void *);
    node_t *head;
    node_t *tail;
} list_t;

/*
Initialises a linked list.

Inputs: *list - a pointer to an instance of a linked list structure to initialise
        (*destroy)(void *data) - destroys/frees up the data it points to
Returns: none
*/
extern void list_init(list_t *list, void (*destroy)(void *data));

/*
Destroys the linked list. Also frees this memory that it occupied

Inputs: *list - the pointer to the linked list to destroy
Returns: none
*/
extern void list_destroy(list_t *list);

/*
Inserts the element into the list, after it creates a node that contains the data specified. 

Inputs: *list - a pointer to an instance of a list to add to
        *element - specify where to add the data to the linked list.  
        *data - the data to add to the node of the linked list
Returns: -1 if the memory allocation fails
         0 if the insertion into the list was a success
*/
extern int list_ins_next(list_t *list, node_t *element, const void *data);

/*
Retrieves the next element element in the list, and writes its contents to the **data. 
If *element is NULL, it removes this node from the list.

Inputs: *list - a pointer to an instance of a list to remove data.
        *element - specify whether or not to remove the data from the linked list.
        **data - the pointer to the data which to write the contents of the node to.
Returns: -1 if the size of the list is equal to zero
         0 if otherwise
*/
extern int list_rem_next(list_t *list, node_t *element, void **data);

extern void list_rem_elem(list_t *list, node_t *element);

/* 
Various linked list macros 
*/

/*
Returns the size (number of nodes) of the list.
*/
#define list_size(list) ((list)->size)

/*
Returns the head node of the list.
*/
#define list_head(list) ((list)->head)

/*
Returns the tail node of the list.
*/
#define list_tail(list) ((list)->tail)

/*
Returns 1 if the element is the head of the list, otherwise 0 if not. 
*/
#define list_is_head(list, elem) ((elem) == (list)->head ? 1 : 0)

/*
Returns 1 if the element is the tail of the list, else 0 if not.
*/
#define list_is_tail(elem) ((elem)->next == NULL ? 1 : 0)

/*
Retrieves the data from the list that the element is currently pointing to.
*/
#define list_data(elem) ((elem)->data)

/*
Makes the element point to the next node in the list
*/
#define list_next(elem) ((elem)->next)

#endif
