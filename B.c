#include "storman.h"

/*
Trova tutti i puntatori gestiti da storman che puntano allo stesso blocco di *ptr_addr
@params
	ptr_addr: indirizzo di un puntatore gestito da storman (blocco B)
	lowaddr: indirizzo di partenza di B
	highaddr: indirizzo finale di B
	num_ptr: numero di puntatori a B
@return
	NULL: errore o ptr_addr non è l’indirizzo di un puntatore gestito da storman
	void***: indirizzo di un array contenente tutti gli indirizzi dei puntatori gestiti da storman che puntano a B
*/
void*** block_info (void** ptr_addr, void** lowaddr, void** highaddr, size_t* num_ptr){
	//1. Se ptr_addr non è l’indirizzo di un puntatore di storman, allora ritorna con errore 0 (NULL).
	if((!is_handled(*ptr_addr, handled_ptrs)) || (retrieve_block(*ptr_addr, available_zones, lowaddr, highaddr) == -1)){
		return NULL;
	}

	//2. Trova tutti i puntatori gestiti da storman che puntano a B e li inserisce in a[]. Inserisce la dimensione di a[] in num_ptr.
	int n;
	void*** a;
	int i = 0;
	Pointer* curr;
	void* temp;

	n = has_multiple_ptrs(*lowaddr, *highaddr, handled_ptrs);
	a = malloc(n*sizeof(void**));
	*num_ptr = n;
	
	curr = handled_ptrs;	
	while(curr != NULL){
		temp = *(curr->address);
		if(*lowaddr <= temp && temp < *highaddr){
			a[i] = curr->address;
			i++;
		}
		curr = curr->next;
	}

	//3. Restituisce l’indirizzo di a.
	return a;
}

/*
@params
	ptr_addr: indirizzo di un puntatore gestito da storman
	type: conterrà il tipo dello storage del puntatore con indirizzo in ptr_addr: 
		  0 se statico o automatico, 
		  1 se dinamico.
@return
	0: esecuzione corretta
	1: ptr_addr non è l’indirizzo di un puntatore gestito da storman
*/
int pointer_info(void** ptr_addr, int* type){
	if(!is_handled(*ptr_addr, handled_ptrs)){
		return 1;
	}

	Pointer* curr = handled_ptrs;
	while(curr != NULL){
		if(*(curr->address) == *ptr_addr){
			*type = curr->type;
			break;
		}
		curr = curr->next;
	}

	return 0;
}

/*
@params
	ptr_addr: l’indirizzo di un puntatore gestito da storman
	newsize: la nuova grandezza del blocco puntato dal puntatore all’indirizzo in ptr_addr
@return
	0: esecuzione corretta
	1: ptr_addr non è l’indirizzo di un puntatore gestito da storman
	2: errore, newsize è troppo piccola
*/

int block_realloc(void** ptr_addr, size_t newsize){
	//1. Se *ptr_addr non è l’indirizzo di un puntatore gestito da storman
	void* start;
	void* end;
	if((!is_handled(*ptr_addr, handled_ptrs)) || (retrieve_block(*ptr_addr, available_zones, &start, &end) == -1)){
		return 1;
	}

	//2. |B| < newsize
	size_t size = (size_t)(end - start);
	if(size < newsize){ 		
		
		size_t needed_dim = newsize-size;
		//Se nella zona Z che contiene B c’è spazio a sufficienza sulla destra di B, allora espande B sulla destra e ritorna 0.
		if(avb_space(*ptr_addr, available_zones, needed_dim)){
			expand_block(end, &available_zones, needed_dim);
			return 0;
		}

		//Altrimenti, alloca un nuovo blocco B', dealloca B e ritorna 0.			
		copy_block(ptr_addr, &start, &end, size, newsize);
		release_block(*ptr_addr, &available_zones);
		return 0;
	}

	//3. |B| > newsize
	if(size > newsize){ 
		//Sia B' il sotto-blocco di lunghezza newsize prefisso di B: 
		void* newend = start + newsize;
		
		//Se esistono puntatori gestiti da storman che puntano a B ma non a B' allora ritorna con errore 2.	
		if(has_multiple_ptrs(newend, end, handled_ptrs) >= 1){
			return 2;
		}

		//Altrimenti contrae B a destra e ritorna 0
		reduce_block(&start, &end, &newend, &available_zones); 
	}

	//4. |B| = newsize
	return 0;
}