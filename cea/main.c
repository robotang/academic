/*
 *  An implementation of 'Mitchell's Candidate Elimination Algorithm' (CEA)
 *
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

#include "global.h"
#include "cea.h"

cea_classes_t classes =  "123"; /* Compact representation of the classes */
int num_classes = 3;
cea_attributes_t attributes = {"123", "12", "12", "12"}; /* Compact representation of the attributes */
int num_attributes = 4;

char *training_data[] = {
    "31111",
    "21112",
    "31121",
    "11122",
    "31211",
    "21212",
    "31221",
    "11222",
    "32111",
    "22112",
    "32121",
    "12122",
    "32211",
    "22212",
    "32221",
    "32222",
    "33111",
    "33121",
    "13122",
    "33211",
    "23212",
    "33221",
    "33222"
}; /* "<class><attributes...>" */
int num_training = 23;

int main(void)
{
    FILE *output;
    int class_index, training_index;
    cea_t *cea;
    output = fopen("output.txt", "w");    
    
    if((cea = (cea_t *) malloc(sizeof(cea_t)*(num_classes))) == NULL)
    {
        fprintf(stderr, "malloc: %s [%s, %d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    /* Train CEA */
    for(class_index = 0; class_index < num_classes; class_index++)
    {
        cea_init(&cea[class_index], classes[class_index], num_attributes);
        printf("Running CEA with class[%d] = %c\r\n\n", class_index, classes[class_index]);
        fprintf(output, "Running CEA with class[%d] = %c\r\n\n", class_index, classes[class_index]);
        for(training_index = 0; training_index < 10; training_index++)
        {
            printf("\tTraining with training_data[%d] = %s\r\n", training_index, training_data[training_index]);
            fprintf(output, "\tTraining with training_data[%d] = %s\r\n", training_index, training_data[training_index]);
            cea_train(&cea[class_index], training_data[training_index], attributes);
            cea_print(&cea[class_index], output, B_TRUE);
            cea_print(&cea[class_index], output, B_FALSE);
            printf("\r\n");
            fprintf(output, "\r\n");
        }
    }
    
    /* Use trained CEA to classify unseen examples */
    printf("Classifying unseen examples\r\n\n");
    fprintf(output, "Classifying unseen examples\r\n\n");
    for(; training_index < num_training; training_index++)
    {
        printf("\tClassifying example[%d] = %s\r\n", training_index-10, ++training_data[training_index]);
        fprintf(output, "\tClassifying example[%d] = %s\r\n", training_index-10, training_data[training_index]);
        cea_classify(cea, num_classes, training_data[training_index], attributes, output);
        printf("\r\n");
        fprintf(output, "\r\n");
    }

    /* Clean up */
    for(class_index = 0; class_index < num_classes; class_index++)
    {
        list_destroy(&cea[class_index].sset);
        list_destroy(&cea[class_index].gset);
    }
    
    printf("Finished!");
    fprintf(output, "Finished!");

    fclose(output);
    free(cea);

    /* Wait for user to press enter before closing program */
    getchar();

    return 0;
}
