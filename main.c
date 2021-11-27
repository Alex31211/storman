#include "storman.h"

//Init lists
Pointer* handled_ptrs = NULL;
Zone* available_zones = NULL;

int main(){

	//A -> 	
	/*
	static int* ptr;
	static int* ptr2;
	int ret1, ret2;

	ret1 = block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));	
	printf("Alloc:\n\tres = %d.\n", ret1); //0
		
	pointer_assign_internal((void**)&ptr2, (void*)ptr);
	*/

	//--BLOCK_ALLOC--//--BLOCK_RELEASE--
	/*
	ret1 = block_release((void**)&ptr);
	ret2 = block_release((void**)&ptr2);

	printf("Block release:\n\tres1 = %d;\n\tres2 = %d;\n", ret1, ret2); //1 - 0
	printf("\t%p == %p == NULL.\n", ptr, ptr2);
	*/

	//--POINTER_RELEASE--	
	/*
	ret1 = pointer_release((void**)&ptr);
	ret2 = pointer_release((void**)&ptr2);

	printf("Pointer release:\n\tres1 = %d - ptr = %p;\n\tres2 = %d - ptr2 = %p.\n", ret1, ptr, ret2, ptr2); //0 - null, 2 - ptr2
	*/

	//--POINTER_ASSIGN--
	/*
	block_release((void**)&ptr);
	block_alloc((void**)&ptr, 2*sizeof(void*), 300*sizeof(int));
	ret1 = pointer_assign_internal((void**)&ptr2, (void*)ptr);

	printf("Pointer assign (A):\n\tres = %d;\n", ret1); //0
	printf("\t%p != %p != NULL.\n", ptr, ptr2);	
	*/
	
	//B -> 

	//--BLOCK_INFO--
	/*	
	static int* ptr;
	static int* ptr2;
	size_t num;
	int i;
	void* beginning;
	void* end;
	void*** pointer_array;

	block_alloc((void**)&ptr, 2*sizeof(void*), 200*sizeof(int));		
	pointer_assign_internal((void**)&ptr2, (void*)ptr);

	pointer_array = block_info((void**)(&ptr), &beginning, &end, &num);
	printf("Block info:\n\t{");
	for(i=0; i<(int)num; i++){
		printf("%p; ", pointer_array[i]);
	}
	printf("}\n");
	free(pointer_array);
	*/

	//--POINTER_INFO--	
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

	printf("Pointer info:\n\ttype1 = %d;\n\ttype2 = %d;\n\ttype3 = %d.\n", type1, type2, type3); //0, 0, 1
	*/

	//--ASSIGN--
	/*
	static char* ptr;
	int i;
	int retval1, retval2, retval3;

	block_alloc((void**)(&ptr), 2*sizeof(void*), 5*sizeof(char*));
	
	assign_internal(ptr[1], 'c', retval1); 
	assign_internal(i, 2, retval2);
	assign_internal(*ptr, 0, retval3);
	
	printf("Assign (B):\n\tptr[1] = %c;\n\tres1 = %d;\n\tres2 = %d;\n\tres3 = %d.\n", ptr[1], retval1, retval2, retval3); //c, 0, 1, 2
	*/

	//--BLOCK_REALLOC--
	/*
	int* ptr2;
	int* ptr3;
	int ret;

	block_alloc((void**)&ptr2, 2*sizeof(void*), 20*sizeof(int));
	pointer_assign_internal((void**)&ptr3, ptr2+11);

	ret = block_realloc((void**)&ptr2, 10*sizeof(int));
	printf("Realloc:\n\tres = %d.\n", ret); //2
	*/

	//E -> 

	//--TOGGLE_GROUP--
	/*
	int* ptr;
	int* ptr2;
	int* ptr3;
	int* ptr4;
	int ret;

	block_alloc((void**)&ptr, 2*sizeof(void*), 20*sizeof(int));

	pointer_assign((void**)&ptr2, ptr, NULL);
	pointer_assign((void**)&ptr3, ptr, NULL);
	pointer_assign((void**)&ptr4, ptr, NULL);

	assign(ptr3[10], 13, ret, NULL);
	toggle_snapshot((void**)&ptr3);
	assign(ptr2[10], 14, ret, (void**)&ptr2);

	printf("Toggle:\n\tptr2[10] = %d;\n\tptr4[10] = %d;\n\tptr3[10] = %d;\n\tptr[10] = %d;\n", ptr2[10], ptr4[10], ptr3[10], ptr[10]); //14, 13, 13, 13
	*/
	//--DEDUP_BLOCKS--
	
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

	int res = dedup_blocks((void***)pointers, 4);
	printf("Dedup:\n\tres = %d.\n", res);

	free(pointers);
	

	return 0;
}