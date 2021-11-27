#include "storman.h"

/*
Alloca un nuovo blocco
@params
	ptr_addr: è l’indirizzo del puntatore a cui assegnare il nuovo blocco
	alignment: deve essere una potenza del due e multiplo di sizeof(void*)
	size: è la grandezza in byte del blocco da allocare
@return
	0: esecuzione corretta
	1: errore, problemi allocazione del nuovo blocco
	2: errore, alignment non rispetta il vincolo
*/
int block_alloc(void** ptr_addr, size_t alignment, size_t size){

	//1. Controlla i vincoli sull'allineamento
	if((alignment%sizeof(void*)!=0) && (!is_power_of_two(alignment))){
		return 2;
	}
	
	//2. Alloca il nuovo blocco
	Zone* zone;
	int type;
	size_t busy_space, slack;
	void* start;
	void* temp;
	void* end;

	//Trova la zona di allocazione a seconda dello spazio necessario
	type = retrieve_allocation_zone(&available_zones, size, 0, &zone); 

	busy_space = zone->total - zone->available;
	start = (zone->allocated_memory) + busy_space;	

	//Sistema l'allineamento del blocco
	temp = start;
	start = (void*)((size_t)(temp + (alignment-1)) & ~(alignment-1));

	//Controlla che l'allineamento non abbia compromesso lo spazio disponibile
	slack = (size_t)(start-temp);	
	if((size+slack) > zone->available){		
		//Se l'allineamento richiesto rende la zona target non più valida, effettua una nuova ricerca con i nuovi parametri di slack
		zone = NULL;
		type = retrieve_allocation_zone(&available_zones, size, slack, &zone);

		//Nuovo allineamento
		temp = zone->allocated_memory;
		start = (void*)((size_t)(temp + (alignment-1)) & ~(alignment-1));
	}

	//Alloca il blocco	
	end = start + size;
	if(!insert_new_block(&zone, start, end)){
		return 1;
	}
	zone->available -= size;
	
	//3. Se l’indirizzo in ptr_addr è di un puntatore già gestito allora applica block_release senza rilasciare il puntatore
	int count;
	void* s;
	void* e;
	if(is_handled(*ptr_addr, handled_ptrs) && (retrieve_block(*ptr_addr, available_zones, &s, &e) != -1)){
		count = has_multiple_ptrs(s, e, handled_ptrs);
		if(count > 1){
			return 1;
		}else if (count == 1){
			release_block(*ptr_addr, &available_zones);
			return 0;
		}
	}

	//4. Altrimenti, lo acquisisce e assegna l’indirizzo di partenza del nuovo blocco a *ptr_addr
	insert_new_pointer(ptr_addr, &handled_ptrs, type);
	*ptr_addr = start;

	return 0;	
}

/*
Rilascia il puntatore in ptr_addr ed eventualmente il blocco a cui questo punta.
@params
	ptr_addr: è l’indirizzo del puntatore che punta al blocco B da (eventualmente) rilasciare.
@return
	0: esecuzione corretta
	1: esecuzione corretta, ma solo il puntatore in ptr_addr è stato rilasciato.
	2: errore, l’indirizzo in ptr_addr non è quello di un puntatore gestito da storman
*/
int block_release (void** ptr_addr){

	//1. Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito da storman allora ritorna con errore
	void* start;
	void* end;
	if((!is_handled(*ptr_addr, handled_ptrs)) || (retrieve_block(*ptr_addr, available_zones, &start, &end) == -1)){
		return 2;
	}

	//2. Se il blocco B puntato dal puntatore in ptr_addr è puntato anche da altri puntatori gestiti, 
	//	 allora rilascia il puntatore in ptr_addr ma non B.
	int count = has_multiple_ptrs(start, end, handled_ptrs);
	if(count > 1){
		release_ptr(*ptr_addr, &handled_ptrs);
		*ptr_addr = NULL;
		return 1;

	}

	//3. Se invece B è puntato solo dal puntatore in ptr_addr, allora rilascia sia il puntatore in ptr_addr che B.
	release_block(*ptr_addr, &available_zones);
	release_ptr(*ptr_addr, &handled_ptrs);
	*ptr_addr = NULL;
	return 0;
}

/*
Rilascia il puntatore in ptr_addr.
@params
	ptr_addr è l’indirizzo del puntatore da (eventualmente) rilasciare
@return
	0: esecuzione corretta
	1: errore, l’indirizzo in ptr_addr non è quello di un puntatore gestito da storman
	2: errore, il blocco puntato dal puntatore all’indirizzo in ptr_addr non è puntato da altri puntatori (gestiti da storman)
*/
int pointer_release(void** ptr_addr){

	//1. Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito da storman allora ritorna con errore 1
	void* start;
	void* end;
	if((!is_handled(*ptr_addr, handled_ptrs)) || (retrieve_block(*ptr_addr, available_zones, &start, &end) == -1)){
		return 1;
	}

	//2. Se il blocco a cui punta p non è puntato da altri puntatori allora ritorna con errore 2
	if(has_multiple_ptrs(start, end, handled_ptrs) == 1){
		return 2;
	}

	//3. Rilascia il puntatore
	release_ptr(*ptr_addr, &handled_ptrs);
	*ptr_addr = NULL;

	return 0;
}

/*
Acquisisce un puntatore.
@params
	ptr_addr: indirizzo del puntatore di cui si vuole cambiare il valore
	val: valore che si vuole assegnare al puntatore puntato da ptr_addr
@return
	0: esecuzione corretta
	1: val non è un indirizzo di un blocco gestito da storman
*/
int pointer_assign_internal(void** ptr_addr, void* val){

	//1. Se val non è un indirizzo di un blocco gestito da storman allora ritorna con errore 1
	void* start;
	void* end;
	if(retrieve_block(val, available_zones, &start, &end) == -1){
		return 1;
	}

	 
	if(is_handled(*ptr_addr, handled_ptrs)){
		//2. Altrimenti,
		if(val == *ptr_addr){			
			return 0;
		}else{
			//3. Se val non è interno al blocco puntato da *ptr_addr allora applica block_release senza rilasciare il puntatore
			int count;
			if(retrieve_block(*ptr_addr, available_zones, &start, &end) != -1){
				if(!(start<=val && val<end)){
					count = has_multiple_ptrs(start, end, handled_ptrs);
					if(count > 1){
						return 1;
					}else if (count == 1){
						release_block(*ptr_addr, &available_zones);
					}
				}
			}
		}	

	}else{
		//STEP 4
		//Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito allora lo acquisisce
		//Assegna l’indirizzo in val a *ptr_addr e return 0
		int type = (*ptr_addr == NULL) ? 0 : 1;
		
		*ptr_addr = val;
		insert_new_pointer(ptr_addr, &handled_ptrs, type);		
	}

	return 0;
}