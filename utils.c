#include "utils.h"

//*************ZONE*********************
/*
Controlla se esistono zone allocate con spazio sufficiente per il blocco
@usage: block_alloc (A)
@return: zona su cui operare
*/
int retrieve_allocation_zone(Zone** head, size_t size, Zone** ret){
	Zone* prev = NULL;
	Zone* curr = *head;

	while(curr != NULL){
		if(curr->available >= size){
			*ret = curr;
			return 1;
		}
		prev = curr;
		curr = curr->next;
	}

	Zone* new_zone = (Zone*)malloc(2*size*sizeof(Zone));
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
	return 0;
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

//**********BLOCCHI*****************

//Aggiunge il nuovo blocco alla lista locale della zona in cui è allocato
int insert_new_block(Zone** zone, void* start, void* end){
	void** s = (*zone)->starting_addr;
	void** e = (*zone)->ending_addr;

	for(int i=0; i<MAX_BLOCKS; i++){
		if(s[i] == NULL){
			s[i] = start;
			e[i] = end;
			return 1;
		}
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

				return i;
			}
		}
		curr = curr->next;
	}

	return -1;
}

//Controlla quanti ptr nella coda puntano a block
int has_multiple_ptr(void* ptr, Pointer* ptr_head, Zone* head){

	void* s = NULL;
	void* e = NULL;
	int count=0;

	if(retrieve_block(ptr, head, &s, &e) != -1){
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

//Controlla se esistono puntatori spurii nella contrazione di un blocco
int has_ptrs_left(void* first, void* inter, void* last, Pointer* ptr_head){
	Pointer* curr = ptr_head;
	while(curr != NULL){
		if(!(first <= curr->address && curr->address <= inter) && (inter <= curr->address && curr->address <= last)){
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

//Pulisce la memoria che era riservata al blocco senza deallocare memoria dalla zona
void clear_block(void** start, size_t size){
	void* temp;
    for(int i=0; i<(int)size; i++){
    	temp = *start+i;
    	temp = NULL;
    }
}

//Rilascia un intero blocco
void release_block(void* ptr, Zone** head){
	Zone* curr = *head;
	int idx, temp;
	void* s;
	void* e;

	if((idx = retrieve_block(ptr, *head, &s, &e)) == -1){
		return;
	}

	size_t size = (size_t)(e - s);

	temp = idx;
	while(temp != 0){
		curr = curr->next;
		temp--;
	}

    curr->available += size;
    clear_block(curr->starting_addr, size);
    order_metadata(&curr, idx);
}

//Contrae un blocco a destra, riducendone la dimensione e liberando spazio nella zona
void reduce_block(void** start, void** end, void** newend, Zone** head){
	int i, j;
	void** s;
	void** e;

	//Trova la zona di riferimento
	Zone* curr = *head;
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(s[i] == NULL){
				break;
			}

			//Aggiorna puntatori ad inizio e fine del blocco, aggiorna spazio disponibile nel blocco, azzera memoria in ecceso
			if(s[i] == *start && e[i] == *end){
				e[i] = *newend;

				size_t spare = (size_t)(*end - *newend);
				curr->available = curr->available + spare;

				void* temp;
				for(j=1; j<=(int)spare; j++){
					temp = *newend+j;
					temp = NULL;
				}

				return;
			}
		}
		curr = curr->next;
	}
}

int is_avb_space(void* ptr, Zone* head, size_t needed_dim){

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
				if((s[i+1] == NULL) && (curr->available >= needed_dim)){
					return 1;
				}
			}
		}
		curr = curr->next;
	}

	return 0;
}

void expand_block(void* end, Zone** head, size_t needed_dim){
	int i;
	void** e;

	Zone* curr = *head;
	while(curr != NULL){
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(e[i] == end){
				(curr->ending_addr)[i] = end + needed_dim;
			}
		}
		curr = curr->next;
	}
}

void copy_block_content(void* new, void* old, size_t size){
	char* tmp_src = (char*)old;
	char* tmp_dest = (char*)new;

	for(int i=0; i<(int)size; i++){
		tmp_dest[i] = tmp_src[i];
	}
}

//**********PUNTATORI*****************

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

//Aggiunge il nuovo puntatore alla lista globale dei puntatori gestiti
void insert_new_pointer(void* ptr, Pointer** head, int type){
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
	new_ptr->type = type;

	if(prev == NULL){
        new_ptr->next = *head;
        *head = new_ptr;
    }else{
        prev->next = new_ptr;
        new_ptr->next = curr;
    }
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

int retrieve_ptr_type(void* ptr, Pointer* head){
	Pointer* curr = head;
	while(curr != NULL){
		if(curr->address == ptr){
			return curr->type;
		}
		curr = curr->next;
	}

	return -1;
}

//*************ALTRO******************

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