#include "storman.h"

//Init lists
Pointer* handled_ptrs = NULL;
Zone* available_zones = NULL;

int main(){

	// **** A **** OK
	/*static int* ptr;
	static int* ptr2;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign((void**)&ptr2, (void*)ptr); //ptr2 is not handled
	printf("%p == %p\n", ptr, ptr2);

	block_release((void**)&ptr);
	printf("%p == NULL\n", ptr);

	block_alloc((void**)&ptr, 2*sizeof(void*), 300*sizeof(int));
	pointer_assign((void**)&ptr2, (void*)ptr);
	printf("%p != %p != NULL\n", ptr, ptr2);*/
	
	// **** B ****
	static char** ptr; //usage: block_info, pointer_info, assign
	static char** ptr_2; //usage: block_info

	int* ptr2; //usage: pointer_info, block_realloc
	int* ptr3; //usage: block_realloc

	int i, ret, type1, type2, type3, retval1, retval2, retval3;

	size_t num;
	void* beginning;
	void* end;
	void*** pointer_array;

	//TEST BLOCK_INFO -> OK

	/*block_alloc((void**)(&ptr), 2*sizeof(void*), 5*sizeof(char*)); //alloca un array di 5 char* e lo assegna a ptr -> zona_1 (80/39)	
	pointer_assign((void**)&ptr_2, (void*)ptr);

	pointer_array = block_info((void**)(&ptr), &beginning, &end, &num);
	for(i=0; i<(int)num; i++){
		printf("%p; ", pointer_array[i]);
	}
	printf("\n");
	free(pointer_array);
	*/

	//TEST POINTER_INFO + ASSIGN -> OK
	/*block_alloc((void**)(&ptr), 2*sizeof(void*), 5*sizeof(char*));
	block_alloc((void**)(&ptr2), 2*sizeof(void*), 20*sizeof(int)); //alloca un array di 20 int e lo assegna a ptr2 -> zona_2 (160/79)
	block_alloc((void**)(&ptr[3]), 2*sizeof(void*), 30*sizeof(char)); //alloca un array di 30 char e lo assegna a ptr[3] -> zona_1 (80/9)

	pointer_info((void**)&ptr, &type1); //setta type1 a 0 
	pointer_info((void**)&ptr2, &type2); //setta type2 a 0
	pointer_info((void**)&ptr[3], &type3); //setta type3 a 1

	printf("%d - %d - %d\n", type1, type2, type3); //EXPECTED: 0, 0, 1

	assign(ptr[3][1], 'c', retval1); //retval1 è settato a 0
	assign(i, 2, retval2); //retval2 è settato a 1
	assign(*ptr, NULL, retval3); //retval3 è settato a 2
	
	printf("%c - %d - %d - %d\n", ptr[3][1], retval1, retval2, retval3); //EXPECTED: c, 0, 1, 2
	*/
	
	//TEST BLOCK_REALLOC -> OK
	block_alloc((void**)&ptr2, 2*sizeof(void*), 20*sizeof(int));
	pointer_assign((void**)&ptr3, ptr2+11);
	ret = block_realloc((void**)&ptr2, 10*sizeof(int));
	printf("%d\n", ret);
	
}