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
	//STEP 1
	//Se ptr_addr non è l’indirizzo di un puntatore di storman return NULL.
	if(!is_handled(*ptr_addr, handled_ptrs)){
		return NULL;
	}

	//STEP 2
	//Setta *lowaddr e *highaddr con gli indirizzi di partenza e fine del blocco puntato da *ptr_addr.
	if(!retrieve_block(*ptr_addr, available_zones, lowaddr, highaddr)){
		return NULL;
	}

	//STEP 3
	//Trova tutti i puntatori p1, ..., pn gestiti da storman che puntano a B
	int n = has_multiple_ptr(*ptr_addr, handled_ptrs, available_zones);

	//STEP 4
	//Alloca con malloc un array a di n void **.
	void*** a = malloc(n*sizeof(void**));
	
	//STEP 5
	//Setta *num_ptr a n.
	*num_ptr = n;

	//STEP 6
	//Copia gli indirizzi di p1, ..., pn in a.
	int i = 0;
	Pointer* curr = handled_ptrs;
	while(curr != NULL){
		if(*lowaddr <= curr->address && curr->address <= *highaddr){
			a[i++] = &(curr->address);
		}
		curr = curr->next;
	}

	//STEP 7
	//Restituisce l’indirizzo di a.
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
int pointer_info(void** ptr_addr, unsigned int* type){
	if(!is_handled(*ptr_addr, handled_ptrs)){
		return 1;
	}

	//get type and set pointer -> return 0
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
/*int block_realloc(void** ptr_addr, size_t newsize){
	Block* block_ptr = NULL;
	if(!is_handled(*ptr_addr, d_ptr, &block_ptr)){
		return 1;
	}

	//Caso 1
	if(block_ptr->block_dim == newsize){
		return 0;
	}

	//Caso 2
	if(block_ptr->block_dim < newsize){
		//Se nella zona Z che contiene B c’è spazio a sufficienza sulla destra di B, allora espande B sulla destra e return 0.
		//Se in Z non c’è spazio per espandere B:
			//1. Alloca un nuovo blocco B 0 con lo stesso procedimento di block_alloc(ptr_addr, align, newsize) (dove align è lo stesso allineamento di B) ma senza deallocare B.
			//2. Copia il contenuto di B in B 0.
			//3. Aggiorna tutti i puntatori gestiti da storman che puntano a B in modo che puntino a B 0 , mantenendo gli stessi offset.
			//4. Dealloca B e return 0.
	}

	//Caso 3 
	if(block_ptr->block_dim > newsize){
		//Deve contrarre B dall’estremità destra, sia B0 il sotto-blocco di lunghezza newdim prefisso di B.
			//Se esistono puntatori gestiti da storman che puntano a B ma non a B0 (e che quindi sarebbero “tagliati fuori” dalla contrazione) allora return 2.
			//Altrimenti contrae B a destra (liberando spazio nella zona Z di B) e return 0.
	}
}*/