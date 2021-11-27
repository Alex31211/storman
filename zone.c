#include "zone.h"

//Alloca una nuova zona quando non c'è spazio dispobile già allocato
/*Usage (internal): 
	- retrieve_allocation_zone
*/
int zone_alloc(Zone** head, Zone* prev, Zone* curr, size_t size, Zone** ret){

	Zone* new_zone = (Zone*)malloc(sizeof(Zone));

	new_zone->allocated_memory = (void*)malloc(2*size);
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

//Controlla se esistono zone allocate con spazio sufficiente per il blocco
/*Usage: 
	- A: block_alloc
*/
int retrieve_allocation_zone(Zone** head, size_t size, size_t slack, Zone** ret){

	Zone* prev = NULL;
	Zone* curr = *head;
	size_t tot_size = size + slack;

	while(curr != NULL){

		if(curr->available >= tot_size){
			*ret = curr;
			return 1;
		}

		prev = curr;
		curr = curr->next;
	}

	return zone_alloc(head, prev, curr, size, ret);
}

//Riordina gli array degli indirizzi di inizio e fine dei blocchi nella zona
/*Usage:
	- block: release_block
*/
void order_metadata(Zone** zone, int idx){
	int i;
	void** s = (*zone)->starting_addr;
	void** e = (*zone)->ending_addr;

	for(i=idx; i<MAX_BLOCKS-1; i++){
		s[i] = e[i] = NULL;

		if(s[i+1] == NULL){
			return;
		}

		s[i] = s[i+1];
		e[i] = e[i+1];
	}

	s[i+1] = e[i+1] = NULL;
}