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

#include "list.h"

/* In order to initialise, call this function, i.e list_init(&list, free)
where &list is dereferencing the pointer to an instance of the list */  
void list_init(list_t *list, void (*destroy)(void *data))
{
    list->size = 0;
    list->destroy = destroy;
    list->head = NULL;
    list->tail = NULL;
}

void list_destroy(list_t *list)
{
    void *data;
    
    /* Keep looping until there are no longer any elements in the list */
    while (list_size(list) > 0)
    {
        if (list_rem_next(list, NULL, (void **) &data) == 0 && list->destroy != NULL)
        {
            list->destroy(data);
        }
    }
    /* Sets the <sizeof(list_t)> number of bytes pointed to by the arguement *list to be zero */
    memset(list, 0, sizeof(list_t));
}

int list_ins_next(list_t *list, node_t *element, const void *data)
{
    node_t *new_element;
    if ((new_element = (node_t *) malloc(sizeof(node_t))) == NULL)
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return (-1);
    }
    new_element->data = (void *) data;
    
    /* Insert the element to the head of the list */
    if (element == NULL)
    {
        /* Also if the list is empty, set the tail to point to the element */
        if (list_size(list) == 0)
        {
            list->tail = new_element;
        }    
        new_element->next = list->head;
        list->head = new_element;
    }
    /* Otherwise insert the element into the list */
    else
    {
        if (element->next == NULL)
        {
            list->tail = new_element;
        }
        new_element->next = element->next;
        element->next = new_element;
    }
    
    list->size++;
    return (0);
}

int list_rem_next(list_t *list, node_t *element, void **data)
{
    node_t *old_element;
    if (list_size(list) == 0)
    {
        return (-1);
    }
    
    /* Retrieve the element from the head of the list */
    if (element == NULL)
    {
        *data = list->head->data;
        old_element = list->head;
        list->head = list->head->next;
        
        /* If the list now only contains one element, tail points to NULL */
        if (list_size(list) == 1)
        {
            list->tail = NULL;
        }
    }
    
    /* Otherwise retrieve the element from the list */
    else
    {
        if (element->next == NULL)
        {
            return (-1);
        }
        
        *data = element->next->data;
        old_element = element->next;
        
        element->next = element->next->next;
        /* If the pointer to the next element is null, make the tail now point to the element */
        if (element->next == NULL)
        {
            list->tail = element;
        }
    }
    
    free(old_element);
    list->size--;
    return (0);
}

/* Todo - return pointer to next, so can free element */
void list_rem_elem(list_t *list, node_t *element)
{
    node_t *tmp, *prev;
    tmp = element;
    prev = list_head(list);

    if(tmp == prev)
    {
        list->head = list->head->next;
        /* free(tmp); dont free tmp as it will also free element! */
        list->size--;
    }
    else
    {
        while(prev->next != tmp)
        {
            prev = prev->next;
        }

        prev->next = tmp->next;
        /* free(tmp); dont free tmp as it will also free element! */
        if (prev->next == NULL)
        {
            list->tail = prev;
        }

        list->size--;
    }
}
