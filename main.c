#include "storman.h"

//Init lists
Pointer* handled_ptrs = NULL;
Zone* available_zones = NULL;

int main(){

	//---------------------------------------------BLOCK_RELEASE------------------------------------------------
	/*
	static int* ptr;
	static int* ptr2;
	int i;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign_internal((void**)&ptr2, (void*)ptr);

	i = ptr2 [10];

	block_release((void**)&ptr);
	block_release((void**)&ptr2);
	printf("%p == %p == NULL\n", ptr, ptr2);
	*/

	//----------------------------------------------POINTER_RELEASE---------------------------------------------
	/*
	static int* ptr;
	static int* ptr2;
	int i, res1, res2;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign_internal((void**)&ptr2, (void*)ptr);
	
	i = ptr2[10];

	res1 = pointer_release(ptr);
	res2 = pointer_release(ptr2);

	printf("%d - %p, %d - %p\n", res1, ptr, res2, ptr2); //0 - null, 2 - ptr2
	*/

	//----------------------------------------------POINTER_ASSIGN----------------------------------------------
	/*
	static int* ptr;
	static int* ptr2;
	int i, res1, res2;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));
	pointer_assign_internal((void**)&ptr2, (void*)ptr);
	
	i = ptr2[10];

	block_release((void**)&ptr);
	block_alloc((void**)&ptr, 2*sizeof(void*), 300*sizeof(int));
	pointer_assign_internal((void**)&ptr2, (void*)ptr);

	printf("%p != %p != NULL\n", ptr, ptr2);
	*/
	
	//------------------------------------------------BLOCK_INFO------------------------------------------
	/*
	static int* ptr;
	static int* ptr2;
	size_t num;
	void* beginning;
	void* end;
	void*** pointer_array;

	block_alloc((void**)(&ptr), 2*sizeof(void*), 200*sizeof(int));	
	pointer_assign_internal((void**)&ptr_2, (void*)ptr);

	pointer_array = block_info((void**)(&ptr), &beginning, &end, &num);
	for(i=0; i<(int)num; i++){
		printf("%p; ", pointer_array[i]);
	}
	printf("\n");
	free(pointer_array);
	*/

	//-----------------------------------------------POINTER_INFO-----------------------------------------
	/*
	static char** ptr;
	int* ptr2;
	int type1, type2, type3;

	block_alloc((void**)(&ptr), 2*sizeof(void*), 5*sizeof(char*));
	block_alloc((void**)(&ptr2), 2*sizeof(void*), 20*sizeof(int));
	block_alloc((void**)(&ptr[3]), 2*sizeof(void*), 30*sizeof(char));

	pointer_info((void**)&ptr, &type1); 
	pointer_info((void**)&ptr2, &type2); 
	pointer_info((void**)&ptr[3], &type3);

	printf("%d - %d - %d\n", type1, type2, type3); //0, 0, 1
	*/

	//-------------------------------------ASSIGN---------------------------------------------------
	/*
	static char* ptr;
	int i;
	int retval1, retval2, retval3;

	block_alloc((void**)(&ptr), 2*sizeof(void*), 5*sizeof(char*));

	assign_internal(ptr[1], 'c', retval1); 
	assign_internal(i, 2, retval2);
	assign_internal(ptr, NULL, retval3);
	
	printf("%c - %d - %d - %d\n", ptr[1], retval1, retval2, retval3); //c, 0, 1, 2
	*/
	
	//--------------------------------BLOCK_REALLOC-------------------------------------------------
	/*
	int* ptr2;
	int* ptr3;
	int ret;

	block_alloc((void**)&ptr2, 2*sizeof(void*), 20*sizeof(int));

	pointer_assign_internal((void**)&ptr3, ptr2+11);

	ret = block_realloc((void**)&ptr2, 10*sizeof(int));

	printf("%d\n", ret); //2
	*/

	//---------------------------------------TOGGLE_GROUP--------------------------------------------
	/*
	int* ptr;
	int* ptr2;
	int* ptr3;
	int* ptr4;

	block_alloc((void**)&ptr, 2*sizeof(void*), 20*sizeof(int));

	pointer_assign((void**)&ptr2, ptr, NULL);
	pointer_assign((void**)&ptr3, ptr, NULL);
	pointer_assign((void**)&ptr4, ptr, NULL);

	assign(ptr3[10], 13, ret, NULL);

	toggle_snapshot((void**)&ptr3);

	assign(ptr2[10], 14, ret, (void**)&ptr2);

	printf("%d\n", ptr2[10]); //14
	printf("%d\n", ptr4[10]); //13
	printf("%d\n", ptr3[10]); //13
	printf("%d\n", ptr[10]); //13
	*/

	//------------------------------------DEDUP_BLOCKS----------------------------------------------
	
	int* ptr;
	int* ptr2;
	int* ptr3;
	int* ptr4;
	int i;
	char*** pointers;

	block_alloc((void**)&ptr, 2*sizeof(void *), 20*sizeof(int)); 
	block_alloc((void**)&ptr2, 2*sizeof(void *), 30*sizeof(int)); 
	block_alloc((void**)&ptr3, 2*sizeof(void *), 20*sizeof(int)); 
	block_alloc((void**)&ptr4, 2*sizeof(void *), 30*sizeof(int)); 

	for(i=0; i<20; i++){
		ptr[i] = ptr3[i] = 12;
	}

	for(i=0; i<30; i++){
		ptr2[i] = ptr4[i] = 24;
	}

	pointers = (char***)malloc(4*sizeof(void **));
	pointers[0] = (char**)&ptr;
	pointers[1] = (char**)&ptr2;
	pointers[2] = (char**)&ptr3;
	pointers[3] = (char**)&ptr4;

	dedup_blocks((void***)pointers, 4);

	free(pointers);
	
	return 0;
}