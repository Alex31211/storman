#include "utils.h"

//*************ZONE*********************
/*
Controlla se esistono zone allocate con spazio sufficiente per il blocco
@usage: block_alloc (A)
@return: zona su cui operare
*/
int retrieve_allocation_zone(Zone** head, size_t size, Zone** ret){
	Zone* curr = *head;

	while(curr != NULL){

		if(curr->available >= size){
			*ret = curr;
			return 1;
		}

		curr = curr->next;
	}

	return new_allocation_zone(head, size, ret);
}

int new_allocation_zone(Zone** head, size_t size, Zone** ret){

	Zone* new_zone = (Zone*)malloc(sizeof(Zone));
	new_zone->allocated_memory = (void*)malloc(2*size);

	new_zone->total = 2*size;
	new_zone->available = new_zone->total;

	Zone* prev = NULL;
	Zone* curr = *head;

	while(curr != NULL){
		prev = curr;
		curr = curr->next;
	}

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

			if(s[i]<=ptr && ptr<e[i]){
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
		void* temp;
		while(curr != NULL){
			temp = *(curr->address);
			if(s <= temp && temp < e){
				count++;
			}
			curr = curr->next;
		}
	}

	return count;
}

//Controlla se esistono puntatori spuri nella contrazione di un blocco
int has_ptrs_left(void* first, void* inter, void* last, Pointer* ptr_head){
	Pointer* curr = ptr_head;
	void* temp;
	while(curr != NULL){
		temp = *(curr->address);

		if(!(first <= temp && temp <= inter) && (inter <= temp && temp <= last)){
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

//Pulisce la memoria che era riservata al blocco senza deallocare memoria dalla zona
void clear_block(void** start, size_t size){
  	unsigned char *p = *start;
  	while(size){
  		p = 0;
      	p++;
      	size--;
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

			//Aggiorna puntatori ad inizio e fine del blocco, aggiorna spazio disponibile nel blocco, azzera memoria in ecceso
			if(s[i] == *start && e[i] == *end){
				e[i] = *newend;

				size_t spare = (size_t)(*end - *newend);
				curr->available = curr->available + spare;

				unsigned char *p = (*newend)+1;
			  	while(spare){
			  		p = 0;
			      	p++;
			      	spare--;
			  	}
			  	p = 0;
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
		int rem = (alig2 > alig1 ? alig2%alig1 : alig1%alig2); //at LEAST log2(n) 0 bits (if more then it's ok)
		if(rem != 0){
			return 0;
		}
	}

	//content
	char* sc1 = (char*)s1;
	char* sc2 = (char*)s2;
	for(int i=0; i<(int)size1; i++){
		if(*(sc1+i) != *(sc2+i)){
			printf("NOT CONTENTED: %c - %c\n", *(sc1+i), *(sc2+i));
			return 0;
		}
	}

	return 1;
}

//**********PUNTATORI*****************

//Controlla se ptr è gestito da storman
int is_handled(void* ptr, Pointer* head){
	Pointer* curr = head;
	while(curr != NULL){
		//printf("%p - %p\n", *(curr->address), ptr);
		if(*(curr->address) == ptr){
			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

//Aggiunge il nuovo puntatore alla lista globale dei puntatori gestiti
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

void insert_corresp_ptrs(void*** array, void* start, int dim, void* address){
	int i = 0;
	void** temp1 = array[i];
	void** temp2;
	size_t offsets[dim];

	offsets[i] = (size_t)(*temp1 - start);
	for(i=1; i<dim; i++){
		temp2 = array[i];
		offsets[i] = (size_t)(*temp2 - *temp1);
		temp1 = temp2;
	}

	for(i=0; i<dim; i++){
		insert_new_pointer(address + offsets[i], &handled_ptrs, 0);
	}
}

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

	void** dest_ptr = dest_start + i;

	return dest_ptr;
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

//**************GROUPS***************
void** retrieve_snapshot(Pointer* head, void* start, void* end, int dim){
	void** pointers = (void**)malloc(dim*sizeof(void*));
	int idx = 0;

	Pointer* curr = head;
	void* temp;
	while(curr != NULL){
		temp = *(curr->address);
		if(start <= temp && temp <= end){
			pointers[idx] = temp;
		}
		curr = curr->next;
	}

	return pointers;
}

int is_in_snapshot(void* addr, void** ptrs, int len){

	for(int i=0; i<len; i++){
		if(ptrs[i] == addr){
			return 1;
		}
	}

	return 0;

}

int is_a_snapshot(void*** group, size_t num){

	if(num <= 1){
		return 0;
	}

	int count = 0;
	void* temp = *group[0];

	for(int i=1; i<(int)num; i++){
		if(temp != *group[i]){
			count++;
		}
		temp = *group[i];
	}

	if(count > 0){
		return 1;
	}

	return 0;
}

//*************SETS*****************
int add_in_set(void*** set, void* start, void* end, int idx, int dim){
	if(duplicate_in_set(set[0], start, dim)){
		return 0;
	}

	set[0][idx] = start;
	set[1][idx] = end;

	return 1;
}

int duplicate_in_set(void** set, void* addr, int dim){
	int i;
	for(i=0; i<dim; i++){
		if(set[i] == addr){
			return 1;
		}
	}

	return 0;
}

void reorder_addresses(void** starts, void** ends, int count, int num_ptrs){
	int i, flag;
	void* temp;
	do{
		flag = 0;
		for(i=0; i<count-1; i++){
			if(starts[i] > starts[i+1]){
				temp = starts[i];
				starts[i] = starts[i+1];
				starts[i+1] = temp;

				temp = ends[i];
				ends[i] = ends[i+1];
				ends[i+1] = temp;

				flag = 1;
			}
		}
	}while(flag);

	for(i=count+1; i<num_ptrs; i++){
		starts[i] = NULL;
		ends[i] = NULL;
	}
}

void**** group_duplicates(void** starts, void** ends, int count, int* num_set, int** in_set){
	int i,j;
	int total_sets = 0; 
	int num_block[count];

	//Definizione dei sets
	void**** sets = (void****)malloc(count*sizeof(void***));
	int* tot_in_set = (int*)malloc(count*sizeof(int));

	for(i=0; i<count; i++){
		sets[i] = (void***)malloc(2*sizeof(void**));
		sets[i][0] = (void**)malloc(count*sizeof(void*));
		sets[i][1] = (void**)malloc(count*sizeof(void*));

		num_block[i] = i;
		tot_in_set[i] = 0;
	}

	//Popolamento
	for(i=0; i<count; i++){
		if(num_block[i] == -1){
			continue;
		}

		sets[i][0][0] = starts[0];
		sets[i][1][0] = ends[0];
		num_block[i] = -1;
		tot_in_set[total_sets] += 1;

		for(j=i+1; j<count; j++){
			if(num_block[j] == -1){
				continue;
			}

			if(are_identical_blocks(starts[i], ends[i], starts[j], ends[j])){ 
				add_in_set(sets[total_sets], starts[j], ends[j], tot_in_set[total_sets], count);				
				num_block[j] = -1;
				tot_in_set[total_sets] += 1;
			}
		}

		total_sets++;
	}

	*num_set = total_sets;
	*in_set = tot_in_set;

	return sets;
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

//Recupera l'allineamento iniziale di un blocco allocato a partire da start
//	start = (temp + (alignment-1)) & ~(alignment-1);
size_t retrieve_alignment(void* start){

	size_t alignment = 0;
	size_t align = sizeof(void*);

	while(((size_t)start % align) == 0){
		alignment = align;
		align *= 2;
	}

	return alignment;
}