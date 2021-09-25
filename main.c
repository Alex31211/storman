#include "storman.h"

//Init lists
Pointer* handled_ptrs = NULL;
Zone* available_zones = NULL;

int main(){

	//**********A****************
	/*static int* ptr;
	static int* ptr2;
	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign((void**)&ptr2, (void*)ptr);

	block_release((void**)&ptr);
	block_alloc((void**)&ptr, 2*sizeof(void*), 300*sizeof(int));
	pointer_assign((void**)&ptr2, (void*)ptr);*/

	//************B**************
	//*********block_info*************
	/*static int* ptr;
	static int* ptr2;
	size_t num;
	void* beginning;
	void* end;
	void*** pointer_array;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign((void**)&ptr2, (void*)ptr); //ptr e ptr2 puntano all'array allocato
	pointer_array = block_info((void**)(&ptr), &beginning, &end, &num); //puntatore all'array contenente ptr e ptr2

	free(pointer_array);*/

	//**********pointer_info**********
	static char** ptr;
	int* ptr2;
	int type1, type2, type3;
	block_alloc((void**)(ptr), 2*sizeof(void*), 5*sizeof(char*)); //alloca un array di 5 char* e lo assegna a ptr
	block_alloc((void**)(&ptr2), 2*sizeof(void*), 20*sizeof(int)); //alloca un array di 20 int e lo assegna a ptr2
	block_alloc((void**)(&ptr[3]), 2*sizeof(void*), 30*sizeof(char)); //alloca un array di 30 char e lo assegna a ptr[3]
	
	Zone* curr = available_zones;
	while(curr != NULL){
		printf("%lu\n", curr->available);
		curr = curr->next;
	}
	
	
	/*pointer_info(&ptr, &type1); //setta type1 a 0 
	pointer_info(&ptr2, &type2); //setta type2 a 0
	pointer_info(&ptr[3], &type3); //setta type3 a 1

	printf("%d - %d - %d\n", type1, type2, type3);*/

	//**********assign**********
	/*
	static char* ptr;
	int i;
	int retval1, retval2, retval3;
	block_alloc(&ptr, 2*sizeof(void*), 5*sizeof(char));
	assign(ptr[1], 'c', retval1); //retval1 è settato a 0 e ’c’ viene assegnato a ptr[1]
	assign(i, 2, retval2); //retval2 è settato a 1
	assign(ptr, NULL, retval3); //retval3 è settato a 2
	*/

	//************block_realloc*************
	/*
	int* ptr;
	int* ptr2;
	int ret;
	block_alloc(&ptr, 2*sizeof(void*), 20*sizeof(int)); //alloca un array di 20 int e lo assegna a ptr
	pointer_assign(&ptr2, ptr+11); //assegna a ptr2 l’indirizzo dell’elemento di indice 11 nell’array
	ret = block_realloc(&ptr, 10*sizeof(int)); //prova a contrarre l’array a 10 elementi invece degli originari 20 e quindi block_realloc restituisce 2
	*/

}