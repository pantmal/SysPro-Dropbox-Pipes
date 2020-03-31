#ifndef ID_H
#define ID_H

//List struct which holds the new IDs spotted

#define MAX_SIZE 50

struct id_list{ 

	char ID[MAX_SIZE];
	int to_delete;
	struct id_list* next;	
	
};

//The use of each function will be explained in the .c file

struct id_list* new_list(char* ID);

struct id_list* get_last(struct id_list* it);

struct id_list* Add_Node(char* temp);

void Removal(struct id_list* itemp, struct id_list* iroot, int allgood );

void DestroyIDList(struct id_list* iroot);



#endif