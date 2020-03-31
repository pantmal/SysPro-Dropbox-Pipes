#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "List.h"

struct id_list* new_list(char* ID){ //Constructor function

	struct id_list* new=malloc(sizeof(struct id_list));
	strcpy(new->ID,ID);
    new->to_delete=0;
    new->next=NULL;
    

	return (new);

	}

struct id_list* get_last(struct id_list* it){ //Getting the last ID of the list

		while(it->next != NULL){
          it = it->next;
        }

        return it;
}

struct id_list* Add_Node(char* temp){ //Adding a new ID in the list
	
	struct id_list* new_node=malloc(sizeof(struct id_list));
      			strcpy(new_node->ID,temp);
      			new_node->next=NULL;
      			new_node->to_delete=0;

      		return new_node;
      			
}

void Removal(struct id_list* itemp, struct id_list* iroot, int allgood ){ //Removal function removes the bygone IDs


	while(1){
			struct id_list* prev=NULL; 
			itemp=iroot;
			while(itemp->next!=NULL && itemp->to_delete!=1){ //If we haven't stumbled upon an ID with its to_delete set to 1 move to the next node
				prev=itemp;
				itemp=itemp->next;
			}

			if(itemp->to_delete==1){ //We found an ID that needs to be deleted
			struct id_list* tnext=itemp->next;
			printf("Let's also remove it from the list \n"); //Removing the ID
			free(itemp);
			prev->next=tnext;
			}

			if(itemp->next==NULL){ //If the next node is NULL we are free to exit
				allgood=1;
			}


			if(allgood==1){ //Print a message and exit the loop
				printf("All IDs accounted for.\n");
				printf("\n");
				break;
			}

		}

}

void DestroyIDList(struct id_list* iroot){ //Destructor function


	  struct id_list* itemp2=iroot; 
      struct id_list* icurr=itemp2;
      struct id_list* inext;

      while(icurr !=NULL){

        inext=icurr->next;
        free(icurr);

        icurr=inext;

    }
}

