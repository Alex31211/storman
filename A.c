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

	//STEP 0
	//Controlla i vincoli su alignment
	if((alignment%sizeof(void*)!=0) && (!is_power_of_two(alignment))){
		return 2;
	}
	
	//STEP 1
	//Controlla spazio per l'allocazione del blocco
	Zone* zone;
	retrieve_available_space(&available_zones, size, &zone);

	size_t busy_space = zone->total - zone->available;
	void* start = (void*)zone + busy_space;

	int slack = 0;
	while(!((size_t)start & (alignment-1))){
		start += 1;
		slack++;
	}

	//Alloca il nuovo blocco ad un indirizzo che sia multiplo di alignment	
	void* end = start + size;
	if(!insert_new_block(zone, start, end)){
		return 1;
	}

	//Aggiorna i parametri di zone
	zone->available -= size + slack;

	//STEP 2
	/*Se l’indirizzo in ptr_addr è di un puntatore già gestito allora applica block_release(ptr_addr) 
	senza rilasciare il puntatore all’indirizzo in ptr_addr.*/
	
	if(is_handled(ptr_addr, handled_ptrs)){
		int count = has_multiple_ptr(*ptr_addr, handled_ptrs, available_zones);

		*ptr_addr = NULL;
		if(count > 1){
			return 1;
		}
		return 0;
	}

	//STEP 3
	//Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito allora lo acquisisce
	//STEP 4
	//Assegna l’indirizzo di partenza del nuovo blocco a *ptr_addr e return 0
	*ptr_addr = start;
	insert_new_pointer(ptr_addr, &handled_ptrs);

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

	//STEP 1
	//Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito da storman allora return 2
	if(!is_handled(ptr_addr, handled_ptrs)){
		return 2;
	}

	//STEP 2
	/*Se il blocco B puntato dal puntatore in ptr_addr è puntato anche da altri puntatori gestiti da storman:
		Rilascia il puntatore in ptr_addr ma non B.
		Assegna NULL a *ptr_addr e return 1.*/
	int count = has_multiple_ptr(*ptr_addr, handled_ptrs, available_zones);

	if(count > 1){
		release_ptr(*ptr_addr, &handled_ptrs);
		*ptr_addr = NULL;
		return 1;
	}else if(count == 1){
		//STEP 3
		/*Se invece B è puntato solo dal puntatore in ptr_addr:
			Rilascia sia il puntatore in ptr_addr che B.
			Assegna NULL a *ptr_addr e return 0.*/
		void* start;
		void* end;
		if(retrieve_block(*ptr_addr, available_zones, &start, &end)){
			release_block(*ptr_addr, &available_zones);
			release_ptr(*ptr_addr, &handled_ptrs);
			*ptr_addr = NULL;
			return 0;
		}
	}

	return 2;
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
	//STEP 1
	//Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito da storman allora return 1
	if(!is_handled(ptr_addr, handled_ptrs)){
		return 1;
	}

	//STEP 2
	//Se il blocco a cui punta p non è puntato da altri puntatori allora return 2
	if(has_multiple_ptr(*ptr_addr, handled_ptrs, available_zones) <= 1){
		return 2;
	}

	//STEP 3
	//Rilascia il puntatore
	release_ptr(*ptr_addr, &handled_ptrs);

	//STEP 4
	//Assegna NULL a *ptr_addr e return 0
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
int pointer_assign(void** ptr_addr, void* val){
	
	//STEP 1
	//Se val non è un indirizzo di un blocco gestito da storman return 1.
	if(!is_in_block(val, available_zones)){
		return 1;
	}

	//Se ptr_addr è di un puntatore gestito
	if(is_handled(ptr_addr, handled_ptrs)){
		//STEP 2
		//Se val == *ptr_addr allora return 0
		if(val == *ptr_addr){
			return 0;
		}else{
			//STEP 3
			/*Se val non è interno al blocco puntato da *ptr_addr allora applica block_release(ptr_addr) 
				senza rilasciare il puntatore all’indirizzo in ptr_addr*/
			void* start;
			void* end;
			if(retrieve_block(*ptr_addr, available_zones, &start, &end)){
				if(!(start<=val && val<=end)){
					int count = has_multiple_ptr(*ptr_addr, handled_ptrs, available_zones);
					*ptr_addr = NULL;
					if(count > 1){
						return 1;
					}
				}
			}
			/*NOT HANDLED: val è interno al blocco puntato da *ptr_addr ma val!=*ptr_addr*/	
		}	

	}else{
		//STEP 4
		//Se l’indirizzo in ptr_addr non è quello di un puntatore già gestito allora lo acquisisce

		//STEP 5
		//Assegna l’indirizzo in val a *ptr_addr e return 0

		*ptr_addr = val;
		insert_new_pointer(ptr_addr, &handled_ptrs);		
	}

	return 0;
}