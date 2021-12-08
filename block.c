#include "block.h"
#include "storman.h"

//Aggiunge il nuovo blocco alla lista locale della zona in cui è allocato
/* Usage:
	- A: block_alloc
*/
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
/*Usage:
	- release_block
	- has_multiple_ptrs
	- A: block_release
		 pointer_assign_internal
	- B: block_info
		 block_realloc
	- E: pointer_assign
*/
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

			if(s[i]<=ptr && ptr<e[i]){
				*start = s[i];
				*end = e[i];

				return 1;
			}
		}
		curr = curr->next;
	}

	return -1;
}

//Pulisce la memoria che era riservata al blocco senza deallocare memoria dalla zona
/*INTERNAL*/
void clear_block(void** start, size_t size){
  	unsigned char *p = *start;
  	while(size){
  		p = 0;
      	p++;
      	size--;
  	}
}

//Rilascia un intero blocco
/*Usage:
	- A: block_alloc
		 block_release
		 pointer_assign_internal
	- B: block_realloc
	- E: dedup_blocks

*/
void release_block(void* ptr, Zone** head){

	int idx;
	void* s;
	void* e;
	if((idx = retrieve_block(ptr, *head, &s, &e)) == -1){
		return;
	}

	size_t size = (size_t)(e - s);

	Zone* curr = *head;
	int temp = idx;
	while(temp != 0){
		curr = curr->next;
		temp--;
	}

    curr->available += size;
    clear_block(curr->starting_addr, size);
    order_metadata(&curr, idx);

    
}

//Controlla quanti ptr gestiti puntano a block
/*Usage:
	- A: block_alloc
		 block_release
		 pointer_release
		 pointer_assign_internal
	- B: block_info
		 block_realloc
	- E: pointer_assign
*/
int has_multiple_ptrs(void* s, void* e, Pointer* ptr_head){

	int count = 0;

	Pointer* curr = ptr_head;
	while(curr != NULL){

		if(s <= *(curr->address) && *(curr->address) < e){
			count++;
		}
		
		curr = curr->next;
	}

	return count;
}

//Contrae un blocco a destra, riducendone la dimensione e liberando spazio nella zona
/*Usage:
	- B: block_realloc
*/
void reduce_block(void** start, void** end, void** newend, Zone** head){
	int i;
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

			//Aggiorna puntatore alla fine del blocco, aggiorna spazio disponibile nel blocco, azzera memoria in ecceso
			if(s[i] == *start && e[i] == *end){
				e[i] = *newend;

				size_t spare = (size_t)(*end - *newend);
				curr->available = curr->available + spare;

				clear_block(newend, spare);
				
				return;
			}
		}
		curr = curr->next;
	}
}

//Controlla che ci sia abbastanza spazio alla destra di un blocco per poterlo espandere fino a needed_dim
/*Usage:
	- B: block_realloc
*/
int avb_space(void* ptr, Zone* head, size_t needed_dim){

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

			if(s[i]<=ptr && ptr<e[i]){
				if((s[i+1] == NULL) && (curr->available >= needed_dim)){
					return 1;
				}
			}
		}
		curr = curr->next;
	}

	return 0;
}

//Espande un blocco
/*Usage:
	- B: block_realloc
*/
void expand_block(void* end, Zone** head, size_t needed_dim){
	int i;
	void** e;

	Zone* curr = *head;
	while(curr != NULL){
		e = curr->ending_addr;
		for(i=0; i<MAX_BLOCKS; i++){
			if(e[i] == end){
				e[i] = end + needed_dim;
			}
		}
		curr = curr->next;
	}
}

//Copia il contenuto di un blocco in un altro
/*INTERNAL*/
void copy_block_content(void* new, void* old, size_t size){
	char* tmp_src = (char*)old;
	char* tmp_dest = (char*)new;

	for(int i=0; i<(int)size; i++){
		tmp_dest[i] = tmp_src[i];
	}
}

//Copia un blocco
/*Usage:
	- B: block_realloc
	- E: pointer_assign
*/
void* copy_block(void** ptr_addr, void** start, void** end, size_t size, size_t newsize){
	//Allineamento
	size_t alignment = retrieve_alignment(*start);
	block_alloc(ptr_addr, alignment, newsize);

	//Contenuto
	void* newstart;
	void* newend;
	retrieve_block(*ptr_addr, available_zones, &newstart, &newend);
	copy_block_content(newstart, *start, size);
				
	//Puntatori
	size_t num;
	void*** pointer_array = block_info(ptr_addr, *start, *end, &num);
	if(num != 0){
		insert_corresp_ptrs(*pointer_array, *start, num, newstart);
	}			
	free(pointer_array);

	return newstart;
}

//Controlla se due blocchi sono uguali per dimensione, allineamento e contenuto
/*Usage:
	- group: group_duplicates
*/
int are_identical_blocks(void* s1, void* e1, void* s2, void* e2){
	
	//size
	size_t size1 = (size_t)(e1-s1);
	size_t size2 = (size_t)(e2-s2);

	if(size1 != size2){
		return 0;
	}

	//alignment
	size_t alig1 = retrieve_alignment(s1);
	size_t alig2 = retrieve_alignment(s2);

	if(alig1 != alig2){
		int rem = (alig2 > alig1 ? alig2%alig1 : alig1%alig2); //at LEAST log2(n) 0 bits
		if(rem != 0){
			return 0;
		}
	}

	//content
	char* sc1 = (char*)s1;
	char* sc2 = (char*)s2;
	for(int i=0; i<(int)size1; i++){
		if(*(sc1+i) != *(sc2+i)){
			return 0;
		}
	}

	return 1;
}