#include "utils.h"

/*
Controlla se esistono zone allocate con spazio sufficiente per il blocco
@usage: block_alloc (A)
@return: zona su cui operare
*/
void retrieve_available_space(Zone** head, size_t size, Zone** ret){
	Zone* prev = NULL;
	Zone* curr = *head;

	while(curr != NULL){
		if(curr->available >= size){
			*ret = curr;
			return;
		}
		prev = curr;
		curr = curr->next;
	}

	Zone* new_zone = (Zone*)malloc(2*size);
	new_zone->total = 2*size;
	new_zone->available = new_zone->total;

	if(prev == NULL){
        new_zone->next = *head;
        *head = new_zone;
    }else{
        prev->next = new_zone;
        new_zone->next = curr;
    }

	*ret = new_zone;
}

//Aggiunge il nuovo blocco alla lista locale della zona in cui è allocato
int insert_new_block(Zone* zone, void* start, void* end){
	void** s = zone->starting_addr;
	void** e = zone->ending_addr;

	for(int i=0; i<MAX_BLOCKS; i++){
		if(s[i] == NULL){
			s[i] = start;
			e[i] = end;

			return 1;
		}
	}

	return 0;
}

//Controlla se ptr è gestito da storman
int is_handled(void* ptr, Pointer* head){
	Pointer* curr = head;
	while(curr != NULL){
		if(curr->address == ptr){

			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

//Recupera l'intervallo di memoria che rappresenta il blocco puntato da ptr
int retrieve_block(void* ptr, Zone* head, void** start, void** end){
	int i;
	void** s;
	void** e;

	Zone* curr = head;
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(s[i] == NULL){
				break;
			}

			if(s[i]<=ptr && ptr<=e[i]){
				*start = s[i];
				*end = e[i];

				return 1;
			}
		}
		curr = curr->next;
	}

	return 0;
}

//Controlla quanti ptr nella coda puntano a block
int has_multiple_ptr(void* ptr, Pointer* ptr_head, Zone* head){

	void* s = NULL;
	void* e = NULL;
	int count=0;

	if(retrieve_block(ptr, head, &s, &e)){
		Pointer* curr = ptr_head;
		while(curr != NULL){
			if(s <= curr->address && curr->address <= e){
				count++;
			}
			curr = curr->next;
		}
	}

	return count;
}

//Aggiunge il nuovo puntatore alla lista globale dei puntatori gestiti
void insert_new_pointer(void** ptr, Pointer** head){
	Pointer* prev = NULL;
	Pointer* curr = *head;

	while(curr != NULL){
		prev = curr;
		curr = curr->next;
	}

	Pointer* new_ptr = (Pointer*)malloc(sizeof(Pointer));
	if(new_ptr == NULL){
		printf("Errore allocazione nuovo puntatore\n");
		return;
	}

	new_ptr->address = ptr;

	if(prev == NULL){
        new_ptr->next = *head;
        *head = new_ptr;
    }else{
        prev->next = new_ptr;
        new_ptr->next = curr;
    }
}

//Controlla se l'argomento è una potenza di 2
int is_power_of_two(size_t var){
	if(var == 0){
		return 0;
	}
	
	for(unsigned long pow=1; pow>0; pow=pow<<1){
		if(pow == var){
			return 1;
		}
		
		if(pow > var){
			return 0;
		}
	}
	
	return 0;
}

//Rilascia un puntatore
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

//Pulisce la memoria che era riservata al blocco senza deallocare memoria dalla zona
void clear_block(void** start, size_t size){

    for(int i=0; i<(int)size; i++){
    	start[i] = NULL;
    }
}

//Riordina gli array degli indirizzi di inizio e fine dei blocchi nella zona
void order_metadata(Zone** zone, int idx){
	int i;
	void** s = (*zone)->starting_addr;
	void** e = (*zone)->ending_addr;

	for(i=idx; i<MAX_BLOCKS-1; i++){
		s[i] = NULL;
		e[i] = NULL;

		if(s[i+1] == NULL){
			return;
		}

		s[i] = s[i+1];
		e[i] = e[i+1];
	}

	s[i+1] = NULL;
	e[i+1] = NULL;
}

//Rilascia un intero blocco
void release_block(void* ptr, Zone** head){
	int i;
	void** s;
	void** e;

	Zone* curr = *head;
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(s[i] == NULL){
				break;
			}

			if(s[i]<=ptr && ptr<=e[i]){

				void* start = s[i];
				void* end = e[i];

				size_t size = (size_t)(end - start);
	            curr->available += size;

	            clear_block(curr->starting_addr, size);
	            order_metadata(&curr, i);

				return;
			}
		}
		curr = curr->next;
	}

	return;
}

int is_in_block(void* ptr, Zone* head){
	int i;
	void** s;
	void** e;

	Zone* curr = head;
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(s[i] == NULL){
				break;
			}

			if(s[i]<=ptr && ptr<=e[i]){
				return 1;
			}
		}
		curr = curr->next;
	}

	return 0;
}