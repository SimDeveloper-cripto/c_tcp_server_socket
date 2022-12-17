#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// TODO: DEFINE CORRECTLY THE NUMBER OF ATTRIBUTES THE LIST_NODE HAS TO HAVE
struct list_node_t {
    char* artifact_id;
    char* n_description; // artifact description for 'guest' and 'group'
    char* y_description; // artifact description for 'family' and 'school'
    char* e_description; // artifact description for 'expert'
    char* area; // area defining where the artifact is placed in the museum

    // TODO: DO WE NEED TO ADD DATE AND COMMENT_DATE?
    // TODO: DO WE ADD POINTER TO TAIL (LASD LIKE)?

    struct list_node_t* next;
};

typedef struct list_node_t* artifacts_list;

/** CREATE THE LINKED LIST
artifacts_list* artifacts_list_create(char* artifacts_id, char* n_description, char* y_description,
    char* e_description, char* area);

*/

/** DELETE (FREE) THE LINKED LIST
void artifacts_list_delete(artifacts_list* artifacts_list);

*/

/** INSERT NODE AT FRONT INTO THE LINKED LIST
void artifacts_list_insert_front(artifacts_list* artifacts_list, char* artifacts_id, char* n_description,
    char* y_description, char* e_description, char* area);

*/

/** REMOVE NODE AT FRONT FROM THE LINKED LIST
void artifacts_list_remove_front(artifacts_list* artifacts_list);

*/

// PRINT THE LINKED LIST (FOR DEBUGGING)
void artifacts_list_print(artifacts_list* artifacts_list);

#endif /* LINKED_LIST_H */