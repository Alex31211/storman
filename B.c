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
	if(retrieve_block(*ptr_addr, available_zones, lowaddr, highaddr) == -1){
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
	void* temp;
	while(curr != NULL){
		temp = *(curr->address);
		if(*lowaddr <= temp && temp < *highaddr){
			a[i] = curr->address;
			i++;
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
	//STEP 0
	if(!is_handled(*ptr_addr, handled_ptrs)){
		return 1;
	}

	//STEP 1
	//Sia B il blocco puntato dal puntatore all’indirizzo in ptr_addr	
	void* start;
	void* end;
	retrieve_block(*ptr_addr, available_zones, &start, &end);
	size_t size = (size_t)(end - start);

	if(size < newsize){ 
		//Caso 2: |B| < newsize
		//Se nella zona Z che contiene B c’è spazio a sufficienza sulla destra di B, allora espande B sulla destra e return 0.
		size_t needed_dim = newsize-size;
		if(is_avb_space(*ptr_addr, available_zones, needed_dim)){
			expand_block(end, &available_zones, needed_dim);
		}else{			
			//Alloca un nuovo blocco B' con block_alloc(ptr_addr, align, newsize), stesso align di B, senza deallocare B.
				//NOTE: block_alloc controlla il vincolo di avere alignement divisibile per 8 byte e che sia una potenza di 2!
			size_t alignment = 0; 
			size_t align_temp = sizeof(void*);
			while(((size_t)start % align_temp) == 0){
				alignment = align_temp;
				align_temp *= 2;
			}
			block_alloc(ptr_addr, alignment, newsize);

			//Copia il contenuto di B in B'.
			void* newstart;
			void* newend;
			retrieve_block(*ptr_addr, available_zones, &newstart, &newend);
			copy_block_content(newstart, start, size);
						
			//Aggiorna tutti i puntatori gestiti da storman che puntano a B in modo che puntino a B', mantenendo gli stessi offset.
			size_t num;
			void*** pointer_array = block_info(ptr_addr, start, end, &num);
			if(num != 0){
				insert_corresp_ptrs(pointer_array, start, num, newstart);
			}			
			free(pointer_array);

			//Dealloca B e return 0.
			release_block(*ptr_addr, &available_zones);
		}

	}else if(size > newsize){ 
		//Caso 3: |B| > newsize -> ia B' il sotto-blocco di lunghezza newsize prefisso di B: 
		//Se esistono puntatori gestiti da storman che puntano a B ma non a B' allora return 2.
		void* newend = start + newsize;
		if(has_ptrs_left(start, newend, end, handled_ptrs)){
			return 2;
		}else{
			//Altrimenti contrae B a destra e return 0
			reduce_block(&start, &end, &newend, &available_zones); //STACK SMASHING
		}
	}

	//Caso 1: |B| = newsize || success
	return 0;
}