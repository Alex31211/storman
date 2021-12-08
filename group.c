#include "group.h"

//Trova lo snapshot a cui appartiene un blocco, ovvero l'insieme di puntatori che punta ad esso
/*Usage:
	- E: pointer_assign
	- assign MACRO
*/
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
		idx++;
	}

	return pointers;
}

//Verifica che un certo puntatore sia nello snapshot di un blocco
/*Usage:
	- E: pointer_assign
	- assign MACRO
*/
int is_in_snapshot(void* addr, void** ptrs, int len){

	for(int i=0; i<len; i++){
		if(ptrs[i] == addr){
			return 1;
		}
	}

	return 0;
}

//Verifica se il blocco è in uno snapshot
/*Usage:
	-E: toggle_snapshot
		dedup_blocks
	- assign MACRO
*/
int is_a_snapshot(void* start, void* end, Zone** head){
	int flag;
	int num_ptrs = has_multiple_ptrs(start, end, handled_ptrs);
	if(num_ptrs == 1){
		flag = 1; //il blocco ha un solo puntatore -> storman si comporta come se fosse un alias
	}

	void** s;
	void** e;
	int i;
	Zone* curr = *head;
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;

		for(i=0; i<MAX_BLOCKS; i++){

			if(s[i] == NULL){
				break;
			}

			if(s[i]==start && e[i]==end){
				if(flag){
					(curr->type)[i] = ALIAS;
					return 0;
				}

				return 1;
			}
		}
		curr = curr->next;
	}

	//il gruppo è uno snapshot: ha più puntatori e non nella lista degli alias
	return -1;
}

//Aggiunge lo snapshot alla lista degli alias
/*Usage:
	- E: toggle_snapshot
*/
void to_alias(void* start, void* end, Zone** head){
	void** s;
	void** e;
	int i;
	Zone* curr = *head;
	
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;

		for(i=0; i<MAX_BLOCKS; i++){

			if(s[i] == NULL){
				break;
			}

			if(s[i]==start && e[i]==end){
				(curr->type)[i] = ALIAS;
				return;
			}
		}
		curr = curr->next;
	}

	return;
}

//Rimuove il gruppo dalla lista degli alias senza deallocarlo: resta come snapshot
/*Usage:
	- E: toggle_snapshot
*/
void to_snapshot(void* start, void* end, Zone** head){
	void** s;
	void** e;
	int i;
	Zone* curr = *head;
	
	while(curr != NULL){
		s = curr->starting_addr;
		e = curr->ending_addr;

		for(i=0; i<MAX_BLOCKS; i++){

			if(s[i] == NULL){
				break;
			}

			if(s[i]==start && e[i]==end){
				(curr->type)[i] = SNAPSHOT;
				return;
			}
		}
		curr = curr->next;
	}

	return;
}

//Ordina in base all'indirizzo di partenza un set di blocchi
/*Usage:
	- E: dedup_blocks
*/
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

//Esclude dal set blocchi già aggiunti
/*Usage:
	- add_in_set
*/
int duplicate_in_set(void** set, void* addr, int dim){
	int i;
	for(i=0; i<dim; i++){
		if(set[i] == addr){
			return 1;
		}
	}

	return 0;
}

//INTERNAL: aggiunge un blocco ad un insieme di blocchi identici
/*Usage:
	- group_duplicates
*/
int add_in_set(void*** set, void* start, void* end, int idx, int dim){
	if(duplicate_in_set(set[0], start, dim)){
		return 0;
	}

	set[0][idx] = start;
	set[1][idx] = end;

	return 1;
}

//Raggruppa in set i blocchi duplicati
/*Usage:
	- E: dedup_blocks
*/
void**** group_duplicates(void** starts, void** ends, int count, int* num_set, int** in_set){
	int i,j;
	int total_sets = 0;   //Contatore degli insiemi costruiti
	int num_block[count]; //Indice dei blocchi per tenere traccia dei duplicati

	//Definizione dei set
	void**** sets = (void****)malloc(count*sizeof(void***));
	int* tot_in_set = (int*)malloc(count*sizeof(int));

	for(i=0; i<count; i++){
		sets[i] = (void***)malloc(2*sizeof(void**)); 		//array esterno: indici dei set
		sets[i][0] = (void**)malloc(count*sizeof(void*));	//array intermedi: indirizzi di inizio blocco
		sets[i][1] = (void**)malloc(count*sizeof(void*));	//array intermedi: indirizzo di fine blocco

		num_block[i] = i; 	//Popola l'array delle etichette in un modo arbitrario diverso da -1
		tot_in_set[i] = 0; 	//Inizializza i contatori del numero di blocchi in ogni set a 0
	}

	//Popolamento
	for(i=0; i<count; i++){ //Fissato un blocco dell'insieme considerato...
		if(num_block[i] == -1){ //Se non etichettato come duplicato di un precedente blocco...
			continue;
		}

		//Aggiunge il blocco all'insieme
		sets[total_sets][0][0] = starts[i]; 
		sets[total_sets][1][0] = ends[i]; 
		num_block[i] = -1; 			  //Etichetta il blocco come analizzato
		tot_in_set[total_sets] += 1;  //Incrementa il numero di blocchi nell'insieme

		for(j=i+1; j<count; j++){ 	//Verifica quali sono i blocchi seguenti che risultano duplicati del blocco fissato
			if(num_block[j] == -1){
				continue;
			}

			//Per ogni duplicato trovato...
			if(are_identical_blocks(starts[i], ends[i], starts[j], ends[j])){ 
				//Aggiunge il blocco all'insieme
				add_in_set(sets[total_sets], starts[j], ends[j], tot_in_set[total_sets], count);			
				num_block[j] = -1; 				//Etichetta il blocco come duplicato
				tot_in_set[total_sets] += 1;	//Incrementa il numero di blocchi nell'insieme
			}
		}

		total_sets++; //Passa al set successivo
	}

	*num_set = total_sets;
	*in_set = tot_in_set;

	return sets;
}