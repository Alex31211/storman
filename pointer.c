#include "pointer.h"

//Controlla se ptr è gestito da storman
/*Usage:
	- A: block_alloc
		 block_release
		 pointer_release
		 pointer_assign_internal
	- B: block_info
		 pointer_info
		 block_realloc
	- E: toggle_snapshot
		 dedup_blocks
*/
int is_handled(void* ptr, Pointer* head){
	short ret = 0;

	Pointer* curr = head;
	while(curr != NULL){

		if(*(curr->address) == ptr){
			ret = 1;
			break;
		}
		curr = curr->next;
	}

	return ret;
}

//Aggiunge il nuovo puntatore alla lista globale dei puntatori gestiti
/*Usage:
	- A: block_alloc
		 pointer_assign_internal
*/
void insert_new_pointer(void** ptr, Pointer** head, int type){
	Pointer* prev = NULL;
	Pointer* curr = *head;

	while(curr != NULL){
		prev = curr;
		curr = curr->next;
	}

	Pointer* new_ptr = (Pointer*)malloc(sizeof(Pointer));
	new_ptr->address = ptr;
	new_ptr->type = type;

	if(prev == NULL){
        new_ptr->next = *head;
        *head = new_ptr;
    }else{
        prev->next = new_ptr;
        new_ptr->next = curr;
    }
}

//Aggiunge i nuovi puntatori relativi ad una copia di un blocco corrispondenti ai puntatori del blocco da copiare
/*Usage:
	- B: block_realloc
	- E: pointer_assign
		 dedup_blocks
*/
void insert_corresp_ptrs(void** array, void* start, int dim, void* address){

	void* temp1 = start;
	void* temp2;
	size_t offsets[dim];

	int i=0;
	do{
		temp2 = array[i];
		offsets[i] = (size_t)(temp2 - temp1);
		insert_new_pointer(address + offsets[i], &handled_ptrs, 0);
		temp1 = temp2;

		i++;
	}while(i < dim);
}

//Trova il puntatore corrispondente a src che punta ad un blocco da copiare

void** get_corresp_ptr(void* src_ptr, void* src_start, void* src_end, void** dest_start){

	int i = 0;
	char* temp = (char*)src_start;

	while(temp != src_end){
		if(temp == src_ptr){
			break;
		}
		temp++;
		i++;
	}

	return (dest_start + i);
}


//Rilascia un puntatore
/*
	- A: block_release
		 pointer_release 
*/
void release_ptr(void* ptr, Pointer** head){
	Pointer* prev = NULL;
    Pointer* curr = *head;

    while(curr != NULL){
		if(curr->address == ptr){
			if(prev == NULL){
                *head = curr->next;
            }else{
                prev->next = curr->next;
            }
	  
	        free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
	}
}