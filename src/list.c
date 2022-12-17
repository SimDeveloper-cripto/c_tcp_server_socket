#include <stdio.h>
#include <stdlib.h>
#include "../include/list.h"

void artifacts_list_print(artifacts_list* artifacts_list) {
    struct list_node_t* curr = *artifacts_list;

    while(curr != NULL) {
        printf("list -> { artifact_id: %s, n_description: %s, y_description: %s, e_description: %s, area: %s }\n", 
            curr->artifact_id, curr->n_description, curr->y_description, curr->e_description, curr->area);
        curr = curr->next;
    }
    puts("");
}