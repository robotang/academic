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

#include "cea.h"
#include <math.h>
#include <float.h>

typedef enum {REM_FAIL, REM_INCONSISTENT, REM_GENERAL, REM_SPECIFIC} cea_rem_t;

/* Prototypes of internal functions */
int cea_init_set(cea_t *cea, bool_t init_sset);
int cea_add(cea_t *cea, const bool_t specialise, const char *example, cea_attributes_t attributes);
int cea_remove(list_t *set, int num_attributes, const char *example, const cea_rem_t rem); 
int cea_remove_bubble(list_t *set, int num_attributes, const cea_rem_t rem);
int cea_update_vs(cea_t *cea);
bool_t cea_converged(cea_t *cea);
int cea_evaluate_hypotheses(cea_t *cea, const char *example, float *positive);
bool_t cea_covers(const char *example, const char *element);
bool_t cea_covers_set(list_t *set, const char *example);

int cea_init(cea_t *cea, cea_class_t cea_class, const int num_attributes)
{
    list_init(&cea->sset, free);
    list_init(&cea->gset, free);
    list_init(&cea->vs, free);
    cea->update_vs = B_TRUE;
    cea->cea_class = cea_class;
    cea->num_attributes = num_attributes;

    /* Initialise S to (0,0,...) */    
    cea_init_set(cea, B_TRUE);

    /* Initialise G to (?,?,...) */
    cea_init_set(cea, B_FALSE);

    return 0;
}

int cea_train(cea_t *cea, const char *data, cea_attributes_t attributes)
{
    char *example = NULL;
    char data_class = data[0];
    if((example = (char *) malloc(sizeof(char)*(cea->num_attributes+1))) == NULL)
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    strcpy(example, ++data); /* exclude the class from the example */
    
    /* Positive example */
    if(data_class == cea->cea_class)
    {
        /* Remove from G any hypothesis that fails to cover example */
        cea_remove(&cea->gset, cea->num_attributes, example, REM_FAIL);
        /* Generalise each hypothesis in S that fails to cover d */
        cea_add(cea, B_FALSE, example, attributes);
        /* Remove from S any hypothesis that is more general than another in S */
        cea_remove(&cea->sset, cea->num_attributes, example, REM_GENERAL);
    }
    /* Negative example */
    else
    {
        /* Remove from S any hypothesis incosistent with example */
        cea_remove(&cea->sset, cea->num_attributes, example, REM_INCONSISTENT);
        /* Specialise each hypothesis in G that is not consistent with d */
        cea_add(cea, B_TRUE, example, attributes);
        /* Remove from G any hypothesis that is more specific than another in G */
        cea_remove(&cea->gset, cea->num_attributes, example, REM_SPECIFIC);
    }

    /* For now 'assume' that vs needs to be updated after each training example */
    cea->update_vs = B_TRUE;

    free(example);
    return 0;
}

int cea_classify(cea_t *cea, int num_classes, const char *example, cea_attributes_t attributes, FILE *output)
{
    int class_index, max_index;
    bool_t *converged = NULL;
    bool_t any_converged = B_FALSE;
    float *positive = NULL; 
    float max_positive;

    if(((converged = (bool_t *) malloc(sizeof(bool_t)*(num_classes))) == NULL) ||
        ((positive = (float *) malloc(sizeof(float)*(num_classes))) == NULL))
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    for(class_index = 0; class_index < num_classes; class_index++)
    {
        converged[class_index] = cea_converged(&cea[class_index]);
        if(converged[class_index] == B_TRUE) 
            any_converged = B_TRUE;
        /* No need to enumerate implied hypotheses iff example covers every member of S */
        if(cea_covers_set(&cea[class_index].sset, example) == B_TRUE)
        {
            positive[class_index] = 1.0;
        }
        /* Need to enumerate the implied hypotheses and take votes */
        else
        {
            cea_evaluate_hypotheses(&cea[class_index], example, &positive[class_index]);
        }
    }

    /* Process the results and classify */    
    printf("\tClass classification: ");
    fprintf(output, "\tClass classification: ");

    /* Look for the class with the highest positive value */
    max_index = 0;
    max_positive = positive[max_index];
    for(class_index = 0; class_index < num_classes; class_index++)
    {
        if(positive[class_index] > positive[max_index])
        {
            max_index = class_index;
            max_positive = positive[max_index];
        }
    }

    /* Deal with the case that the example fails to match any VS */
    if(any_converged == B_FALSE || fabs(max_positive) < FLT_EPSILON)
    {
        printf("not classified - example did not match any VS!\r\n");
        fprintf(output, "not classified - example did not match any VS!\r\n");
        goto clean_up;
    }

    /* Look to see if another class exists that also has the same max */
    for(class_index = 0; class_index < num_classes; class_index++)
    {        
        if(class_index != max_index && ((positive[class_index] - FLT_EPSILON) < positive[max_index]) && ((positive[class_index] + FLT_EPSILON) > positive[max_index]))
        {
            /* Find out whether or not the two have converged */
            if(converged[class_index] == B_TRUE && converged[max_index] == B_TRUE)
            {
                printf("error - example matched multiple converged VS!\r\n");
                fprintf(output, "error - example matches multiple converged VS!\r\n");
                goto clean_up;
            }
            else if(converged[class_index] == B_FALSE && converged[max_index] == B_FALSE && any_converged == B_FALSE)
            {
                printf("dont know - example matched multiple non-converged VS!\r\n");
                fprintf(output, "dont know - example matches multiple non converged VS!\r\n");
                goto clean_up;
            }
            else if(converged[class_index] == B_TRUE)
            {
                printf("%c\r\n", cea[class_index].cea_class);
                fprintf(output, "%c\r\n", cea[class_index].cea_class);
                goto clean_up;
            }
            else if(converged[max_index] == B_TRUE)
            {
                printf("%c\r\n", cea[max_index].cea_class);
                fprintf(output, "%c\r\n", cea[max_index].cea_class);
                goto clean_up;
            }
        }
    }

    /* If not already classed, class it! */
    printf("%c\r\n", cea[max_index].cea_class);
    fprintf(output, "%c\r\n", cea[max_index].cea_class);
    goto clean_up;

    /* Jump to clean up allocated variables */
    clean_up:

    free(converged);
    free(positive);

    return 0;
}

void cea_print(const cea_t *cea, FILE *output, bool_t print_sset)
{    
    node_t *elem;
    char *data;
    int i = 0;
    int size;

    if(print_sset == B_TRUE)
    {
        size = list_size(&cea->sset);
        printf("\tS[%d]\t", size);
        fprintf(output, "\tS[%d]\t", size);
        elem = list_head(&cea->sset);        
    }
    else
    {
        size = list_size(&cea->gset);
        printf("\tG[%d]\t", size);
        fprintf(output, "\tG[%d]\t", size);
        elem = list_head(&cea->gset);
    }

    if(size == 0)
    {    
        printf("(EMPTY!)\r\n");
        fprintf(output, "(EMPTY!)\r\n");
    }
    else
    {
        while(size--)
        {
            data = list_data(elem);        
            printf("(%c", data[0]);
            fprintf(output, "(%c", data[0]);
            for(i = 1; i < cea->num_attributes; i++)
            {
                printf(", %c", data[i]);
                fprintf(output, ", %c", data[i]);
            }
            if(size != 0)
            {
                printf("), ");
                fprintf(output, "), ");
            }
            else
            {
                printf(")\r\n");
                fprintf(output, ")\r\n");
            }
            elem = list_next(elem);
        }
    }
}

/* Internal functions */

int cea_init_set(cea_t *cea, bool_t init_sset)
{
    int i;
    char *tmp = NULL;

    if((tmp = (char *) malloc(sizeof(char)*(cea->num_attributes+1))) == NULL)
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    i = 0;
    for(i = 0; i < cea->num_attributes; i++)
    {    
        if(init_sset == B_TRUE)
            tmp[i] = CEA_NONE;
        else
            tmp[i] = CEA_ANY;
    }
    tmp[i] ='\0';
    if(init_sset == B_TRUE)
        list_ins_next(&cea->sset, list_tail(&cea->sset), tmp);
    else
        list_ins_next(&cea->gset, list_tail(&cea->gset), tmp);

    return 0;
}

int cea_add(cea_t *cea, const bool_t specialise, const char *example, cea_attributes_t attributes)
{
    node_t *elem;
    char *list_data;
    int size;

    if(specialise == B_TRUE)
    {
        size = list_size(&cea->gset);
        elem = list_head(&cea->gset);
    }
    else /* Generalise */
    {
        size = list_size(&cea->sset);
        elem = list_head(&cea->sset);
    }

    while(size--)
    {        
        int index;
        bool_t del_elem = B_FALSE;
        bool_t skip_elem = B_FALSE;
        int i;
        list_data = list_data(elem);

        /* Figure out whether or not to skip the element */
        if(specialise == B_FALSE)
            skip_elem = B_TRUE;
        for(i = 0; i < cea->num_attributes; i++)
        {
            if(specialise == B_TRUE && list_data[i] != example[i] && list_data[i] != CEA_ANY && example[i] != CEA_ANY)
            {
                skip_elem = B_TRUE;
                break;                    
            }    
            else if(specialise == B_FALSE && list_data[i] != example[i] && list_data[i] != CEA_ANY)
            {
                skip_elem = B_FALSE;
                break;                    
            }    
        }

        if(skip_elem == B_TRUE)
        {
            elem = list_next(elem);
            continue;
        }

        /* Generate new hypothesis, for each attribute */
        for(index = 0; index < cea->num_attributes; index++)
        {
            int i = 0;
            bool_t add_elem;    
            bool_t consistent;
            char *new_data;

            /* Jump back to test other attributes, in the case that an attribute has more than two classes */
            test_other_attributes:

            add_elem = B_FALSE;    
            new_data = NULL;
            if((new_data = (char *) malloc(sizeof(char)*(cea->num_attributes+1))) == NULL)
            {
                fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
                return -1;
            }

            if(specialise == B_FALSE && list_data[0] == CEA_NONE) /* Take care of special case */
            {
                strcpy(new_data, example);
                list_ins_next(&cea->sset, list_tail(&cea->sset), new_data);
                del_elem = B_TRUE;
                break;
            }
            else
                strcpy(new_data, list_data);

            /* Second iteration loop just incase need to change more than one attribute to make it work! */
            for(; index < cea->num_attributes; index++)
            {
                if(index == cea->num_attributes-1) /* Handle special case (end attribute) */
                    i = 0;
                for(; i < strlen(attributes[index]); i++)
                {                
                    int j;                    
                    if(specialise == B_TRUE)
                        consistent = B_FALSE;
                    else
                        consistent = B_TRUE;
                    
                    /* Skip if the attribute is already specialised or is the same as example */
                    if(specialise == B_TRUE && (new_data[index] != CEA_ANY || attributes[index][i] == example[index]))
                        continue;
                    /* Skip if the attribute is not already specialised or is the same as example */
                    else if(specialise == B_FALSE && (new_data[index] == CEA_ANY || new_data[index] == example[index]))
                        continue;

                    if(specialise == B_TRUE)
                        new_data[index] = attributes[index][i];
                    else
                        new_data[index] = CEA_ANY;

                    /* Check that the new hypothesis is consistent with example */
                    for(j = 0; j < cea->num_attributes; j++)
                    {
                        if(specialise == B_TRUE && (new_data[j] != CEA_ANY && new_data[j] != example[j]))
                        {
                            consistent = B_TRUE;
                            break;
                        }
                        else if(specialise == B_FALSE && (new_data[j] != CEA_ANY && new_data[j] != example[j]))
                        {
                            consistent = B_FALSE;
                            break;
                        }
                    }

                    if(consistent == B_TRUE || specialise == B_FALSE)
                    {
                        i++;
                        break;
                    }
                }
                
                if(consistent == B_TRUE)
                {
                    break;
                }
            }            

            /* Iterate through other set to check that new_data is ok */
            if(consistent == B_TRUE && strcmp(new_data, list_data) != 0) /* Prevent some duplicate hypotheses from occuring (when generalising the S set) */
            {
                node_t *elem2; 
                char *list_data2;
                int size2;

                if(specialise == B_TRUE)
                {
                    size2 = list_size(&cea->sset);
                    elem2 = list_head(&cea->sset);
                }
                else
                {
                    size2 = list_size(&cea->gset);
                    elem2 = list_head(&cea->gset);
                }

                while(size2--)
                {
                    int index2;
                    int count1 = 0;
                    int count2 = 0;
                    int count3 = 0;
                    int count4 = 0;
                    list_data2 = list_data(elem2);
                    
                    for(index2 = 0; index2 < cea->num_attributes; index2++)
                    {
                        if(new_data[index2] != list_data2[index2] && new_data[index2] != CEA_ANY)
                            count1++;
                        if(new_data[index2] != list_data2[index2] && list_data2[index2] != CEA_ANY)
                            count2++;
                        if(list_data2[index2] == CEA_ANY)
                            count3++;
                        else if(list_data2[index2] == CEA_NONE)
                            count4++;
                    }
                    
                    if(specialise == B_TRUE && (count1 == 0 || (count4 == cea->num_attributes)))
                    {                        
                        add_elem = B_TRUE;
                        break;
                    }
                    else if(specialise == B_FALSE && (count2 == 0 || (count3 == cea->num_attributes)))
                    {                        
                        add_elem = B_TRUE;
                        break;
                    }

                    elem2 = list_next(elem2);
                }
            }

            /* Only add element if appropriate */
            if(specialise == B_TRUE && add_elem == B_TRUE)
            {
                list_ins_next(&cea->gset, list_tail(&cea->gset), new_data);
                del_elem = B_TRUE;
            }
            else if(specialise == B_FALSE && add_elem == B_TRUE)
            {
                list_ins_next(&cea->sset, list_tail(&cea->sset), new_data);
                del_elem = B_TRUE;
            }
            else
                free(new_data);

            /* Jump back to test other attributes, in the case that an attribute has more than two classes */
            if(index < cea->num_attributes && specialise == B_TRUE)
                if(i < strlen(attributes[index]) && attributes[index][i] != example[index])
                    goto test_other_attributes;
        }

        if(specialise == B_TRUE) /* && del_elem == B_TRUE) */
            list_rem_elem(&cea->gset, elem);
        else if(specialise == B_FALSE) /* && del_elem == B_TRUE) */
            list_rem_elem(&cea->sset, elem);

        elem = list_next(elem);

        if(elem == NULL)
            break;
    }

    return 0;
}

int cea_remove(list_t *set, int num_attributes, const char *example, const cea_rem_t rem)
{
    node_t *elem;
    char *list_data;
    int size;

    if(rem == REM_GENERAL || rem == REM_SPECIFIC)
    {
        return cea_remove_bubble(set, num_attributes, rem); 
    }

    size = list_size(set);
    elem = list_head(set);

    while(size--)
    {        
        int index;
        bool_t remove = B_FALSE;
        if(rem == REM_INCONSISTENT)
            remove = B_TRUE;
        list_data = list_data(elem);

        for(index = 0; index < num_attributes; index++)
        {
            if(rem == REM_FAIL)
            {
                if(list_data[index] != CEA_ANY && example[index] != list_data[index])
                {
                    remove = B_TRUE;
                    break;
                }
            }
            else /* REM_INCONSISTENT */
            {
                if(example[index] != list_data[index])
                {
                    remove = B_FALSE;
                    break;
                }
            }
        }

        if(remove == B_TRUE)
        {            
            list_rem_elem(set, elem);
        }

        elem = list_next(elem);
        if(elem == NULL)
            break;
    }

    return 0;
}

int cea_remove_bubble(list_t *set, int num_attributes, const cea_rem_t rem)
{
    node_t *elem;
    char *list_data;
    int size;

    size = list_size(set);
    elem = list_head(set);

    while(size--)
    {        
        node_t *elem2;
        char *list_data2;
        int size2;        

        size2 = size;
        elem2 = list_next(elem);

        list_data = list_data(elem);
        
        while(size2--)
        {
            int index;
            int count1 = 0;
            int count2 = 0;
            int count3 = 0;

            if(elem2 == NULL)
                break;

            list_data2 = list_data(elem2);

            for(index = 0; index < num_attributes; index++)
            {
                if(list_data[index] == list_data2[index] && (list_data[index] != CEA_ANY && list_data2[index] != CEA_ANY))
                    count1++;
                else if(list_data[index] == CEA_ANY && list_data2[index] != CEA_ANY)
                    count2++;
                else if(list_data2[index] == CEA_ANY && list_data[index] != CEA_ANY)
                    count3++;
            }

            if(count1 > 0 && count2 > 0 && count3 == 0) 
            {                    
                if(rem == REM_GENERAL)
                {
                    list_rem_elem(set, elem); /* list_data is more general, so remove it */
                    break;
                }
                else /* REM_SPECIFIC */
                    list_rem_elem(set, elem2); /* list_data2 is more specific, so remove it */
            }
            else if(count1 > 0 && count2 == 0 && count3 > 0)
            {
                if(rem == REM_GENERAL)
                    list_rem_elem(set, elem2); /* list_data2 is more general, so remove it */
                else /* REM_SPECIFIC */
                {
                    list_rem_elem(set, elem); /* list_data is more specific, so remove it */
                    break;
                }
            }

            elem2 = list_next(elem2);
        }

        elem = list_next(elem);

        if(elem == NULL)
            break;
    }

    return 0;
}

bool_t cea_converged(cea_t *cea)
{
    node_t *elem;
    char *list_data;
    int size;

    size = list_size(&cea->sset);
    if(size == 0)
        return B_FALSE;

    elem = list_head(&cea->sset);    

    while(size--)
    {        
        node_t *elem2;
        char *list_data2;
        int size2;    

        size2 = list_size(&cea->gset);
        if(size2 == 0)
            return B_FALSE;

        elem2 = list_head(&cea->gset);

        list_data = list_data(elem);        

        while(size2--)
        {
            list_data2 = list_data(elem2);

            if(strcmp(list_data, list_data2) != 0)
                return B_FALSE;

            elem2 = list_next(elem2);
        }
    }

    return B_TRUE;
}

int cea_update_vs(cea_t *cea)
{
    node_t *elem, *elem2;
    char *list_data, *list_data2;
    int size, size2, i, count1 = 0, count2 = 0;
    list_t *level;

    size = list_size(&cea->gset);
    size2 = list_size(&cea->sset);
    if(size == 0 || size2 == 0)
        return 0;

    elem = list_head(&cea->gset);
    list_data = list_data(elem);
    elem2 = list_head(&cea->sset);
    list_data2 = list_data(elem2);    
    
    /* Find out the max num of levels needed for G to be specialised to S */
    for(i = 0; i < cea->num_attributes; i++)
    {
        if(list_data[i] == CEA_ANY)
            count1++;
        if(list_data2[i] == CEA_ANY)
            count2++;
    }

    if((level = (list_t *) malloc(sizeof(list_t)*(count1 - count2))) == NULL) /* Expect count1 > count2 */
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    /* Destroy old version space list */
    list_destroy(&cea->vs);

    /* Generate implied hypotheses */

    /*
        TODO
    */

    /* Transfer the G and S sets, as well as the implied hypotheses to the VS list */
    
    /* Add G set to VS set */
    elem = list_head(&cea->gset);
    size = list_size(&cea->gset);
    while(size--) 
    {
        list_data = list_data(elem);
        list_ins_next(&cea->vs, list_tail(&cea->vs), list_data);
        elem = list_next(elem);
    }
    
    /* Add level to VS set */
    
    /*
        TODO
    */
    
    /* Add S set to VS set */
    elem = list_head(&cea->sset);
    size = list_size(&cea->sset);
    while(size--) 
    {
        list_data = list_data(elem);
        list_ins_next(&cea->vs, list_tail(&cea->vs), list_data);
        elem = list_next(elem);
    }

    free(level);

    return 0;
}

int cea_evaluate_hypotheses(cea_t *cea, const char *example, float *positive)
{
    int positive_count = 0;
    node_t *elem;
    char *list_data;
    int size;
    
    /* Update version space if required */
    if(cea->update_vs == B_TRUE)
        cea_update_vs(cea);

    /* Iterate through version space, counting items that match the example */
    size = list_size(&cea->vs);
    elem = list_head(&cea->vs);

    while(size--)
    {        
        list_data = list_data(elem);

        if(cea_covers(example, list_data) == B_TRUE)
            positive_count++;

        elem = list_next(elem);
    }

    /* Set probability */
    size = list_size(&cea->vs);
    if(size == 0)
        *positive = 0;
    else
        *positive = ((float)positive_count/(float)size);

    return 0;
}

bool_t cea_covers(const char *example, const char *element)
{
    int index;
    for(index = 0; index < strlen(example); index++)
    {
        if(element[index] != CEA_ANY && example[index] != element[index]) 
            return B_FALSE;
    }

    return B_TRUE;
}

bool_t cea_covers_set(list_t *set, const char *example)
{
    node_t *elem;
    char *list_data;
    int size;

    size = list_size(set);
    elem = list_head(set);

    while(size--)
    {        
        list_data = list_data(elem);

        if(cea_covers(example, list_data) == B_FALSE)
            return B_FALSE;
        
        elem = list_next(elem);
    }
    return B_TRUE;
}
