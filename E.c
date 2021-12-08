#include "storman.h"

/*
Acquisisce un puntatore.
@params
	ptr_addr: indirizzo del puntatore di cui si vuole cambiare il valore
	val: valore che si vuole assegnare al puntatore puntato da ptr_addr
	mptr_addr: l’indirizzo di un puntatore gestito
@return
	0: esecuzione corretta
	1: val non è un indirizzo di un blocco gestito da storman
*/
int pointer_assign(void** ptr_addr, void* val, void** mptr_addr){

	void* start;
	void* end;

	//1. Se l’indirizzo in ptr_addr non fa parte di uno dei blocchi gestiti da storman allora chiama pointer_assign_internal
	if((retrieve_block(*ptr_addr, available_zones, &start, &end) == -1)){
		return pointer_assign_internal(ptr_addr, val);
	}

	//2. Sia B il blocco puntato da *ptr_addr, se B non è in uno snapshot allora chiama pointer_assign_internal
	if(!is_a_snapshot(start, end, &available_zones)){
		return pointer_assign_internal(ptr_addr, val);
	}

	int num_ptr = has_multiple_ptrs(start, end, handled_ptrs);
	void** snapshot = retrieve_snapshot(handled_ptrs, start, end, num_ptr);

	//3. Se mptr_addr non è l’indirizzo di un puntatore appartenente allo snapshot di B ritorna 2.	
	if((mptr_addr == NULL) || (!is_in_snapshot(*mptr_addr, snapshot, num_ptr))){
		return 2;
	}

	//4. Mantiene la proprietà degli snapshot
	//alloca un nuovo blocco B' delle stesse dimensioni di B e con lo stesso allineamento e copia il contenuto di B in B'.
	size_t size = (size_t)(end-start);
	void* newstart = copy_block(ptr_addr, &start, &end, size, size);
	void* newend = (void*)((size_t)newstart + size);

	//fa puntare p a B'
	*mptr_addr = *get_corresp_ptr(*ptr_addr, start, end, &newstart);

	//(P − {p}, B) rimane uno snapshot; ({p}, B') invece è un nuovo alias group.
	to_alias(newstart, newend, &available_zones);

	//5. effettua la modifica richiesta su B'
	return pointer_assign_internal(mptr_addr, val);
}

/*
Params:
	ptr_addr:
Return
	0: esecuzione corretta
	1: errore, il puntatore con indirizzo in ptr_addr non è gestito da storman.
*/
int toggle_snapshot(void** ptr_addr){

	//1. Se il puntatore con indirizzo in ptr_addr non è gestito da storman, ritorna 1.
	if(!is_handled(*ptr_addr, handled_ptrs)){
		return 1;
	}

	void* beginning;
	void* end;

	retrieve_block(*ptr_addr, available_zones, &beginning, &end);
	if(is_a_snapshot(beginning, end, &available_zones)){
		//2. Se il puntatore con indirizzo in ptr_addr fa già parte di uno snapshot, trasforma lo snapshot in un alias group e ritorna 0.
		to_alias(beginning, end, &available_zones);
	}else{
		//3. Se il puntatore con indirizzo in ptr_addr fa parte di un alias group allora trasforma l’alias group in snapshot e ritorna 0.
		to_snapshot(beginning, end, &available_zones);
	}
	return 0;
}

/*
Elimina blocchi duplicati, eleggendo un rappresentante e modificando i puntatori gestiti da storman
Params:
	pointers: l’indirizzo di un array di void **, allocato con malloc, contenente indirizzi di puntatori gestiti da storman.
	num_ptrs: il numero di elementi dell’array puntato da pointers.
Return:
	0: esecuzione corretta
	1: errore, esiste almeno un puntatore in pointers non gestito da storman.
	2: errore, esiste almeno un puntatore in pointers che faccia parte di uno snapshot.
*/
int dedup_blocks(void*** pointers, int num_ptrs){

	//1. Se almeno uno dei puntatori in pointers non è fra quelli gestiti da storman, ritorna con errore 1
	int i;
	for(i=0; i<num_ptrs; i++){
		if(!is_handled(*pointers[i], handled_ptrs)){
			return 1;
		}
	}

	//2. Se almeno uno dei puntatori in pointers fa parte di uno snapshot, ritorna con errore 2
	void* start;
	void* end;
	size_t n;
	void* starts[num_ptrs];
	void* ends[num_ptrs];
	int count = 0;

	for(i=0; i<num_ptrs; i++){
		retrieve_block(*pointers[i], available_zones, &start, &end);		
		if(is_a_snapshot(start, end, &available_zones)){
			return 2;
		}

		//Siano (B1, ..., Bk) i blocchi puntati dai puntatori in pointers... 
		starts[count] = start;
		ends[count] = end;
		count++;
	}

	//...ed elencati secondo il loro ordine relativo in memoria
	reorder_addresses(starts, ends, count, num_ptrs);

	//3. Ripartisce (B1, ..., Bk) in (S1, ..., Sh) insiemi tali per cui ogni insieme contiene blocchi duplicati
	int num_set = 0;	
	int* in_set;
	void**** sets = group_duplicates(starts, ends, count, &num_set, &in_set);

	//4. Per ogni insieme, sceglie un blocco Bi in St come rappresentante
	int j;
	void* s;
	void* e;
	void*** pointer_array;

	for(i=0; i<num_set; i++){
		start = sets[i][0][0];
		end = sets[i][1][0];

		//Per ogni altro blocco B dell'insieme
		for(j=1; j<in_set[i]; j++){
			s = sets[i][0][j];
			e = sets[i][1][j];

			//Cambia tutti i puntatori che puntano a B in modo che puntino al rappresentante
			pointer_array = block_info(&s, &s, &e, &n);
			if(n != 0){
				insert_corresp_ptrs(*pointer_array, s, n, start);
			}			
			
			//Dealloca B.
			release_block(s, &available_zones);	
			release_ptrs(pointer_array, n, &handled_ptrs);
			free(pointer_array);		
		}

		free(sets[i]);
	}

	free(sets);
	free(in_set);

	return 0;
}