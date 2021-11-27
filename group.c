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

//Verifica se 
/*Usage:
	-E: toggle_snapshot
		dedup_blocks
*/
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

//
/*Usage:

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

//
/*Usage:

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