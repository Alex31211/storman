#pragma once

#include "utils.h"

//Modulo A
int block_alloc(void** ptr_addr, size_t alignment, size_t size);
int block_release (void** ptr_addr);
int pointer_release(void** ptr_addr);
int pointer_assign_internal(void** ptr_addr, void* val);

//Modulo B
void*** block_info(void** ptr_addr, void** lowaddr, void** highaddr, size_t* num_ptr);
int pointer_info(void** ptr_addr, int* type);
#define assign_internal(__lv, __rv, __ret) \
	/*STEP 1 
	Se l’indirizzo di __lv è di uno dei puntatori gestiti da storman allora assegna 2 a __ret.*/ \
	if(is_handled(&__lv, handled_ptrs)){ \
		__ret = 2; \
	}else{ \
		/*STEP 2 
		Se l’indirizzo di __lv non è contenuto in uno dei blocchi gestiti da storman allora assegna 1 a __ret.*/ \
		void* s_temp;\
		void* e_temp;\
		if(retrieve_block(&__lv, available_zones, &s_temp, &e_temp) == -1){ \
			__ret = 1; \
		}else{ \
			/*STEP 3
			Assegna 0 a __ret e assegna __rv a __lv.*/ \
			__ret = 0; \
			__lv = __rv; \
		} \
	}
				
		
int block_realloc(void** ptr_addr, size_t newsize);

//Modulo E
int dedup_blocks(void*** pointers, int num_ptrs);
int toggle_snapshot(void** ptr_addr);
int pointer_assign(void** ptr_addr, void* val, void** mptr_addr);
#define assign(__lv , __rv , __ret , mptr_addr) \
	/*STEP 0*/\
	if(mptr_addr == NULL){ \
		assign_internal(__lv , __rv , __ret);\
	}else{\
		/*STEP 1		
		Se &__lv è l’indirizzo di uno dei puntatori gestiti da storman.*/\
		if(is_handled(&__lv, handled_ptrs)){ \
			__ret = 2; \
		}else{ \
			/*STEP 2
			Se &__lv non è contenuto in uno dei blocchi gestiti da storman.*/ \
			void* start;\
			void* end;\
			if(retrieve_block(&__lv, available_zones, &start, &end) == -1){ \
				__ret = 1; \
			}else{ \
				/*STEP 3
				Se &__lv è contenuto nel blocco B gestito da storman ma B non è in uno snapshot.*/\
				int num_ptr = has_multiple_ptrs(start, end, handled_ptrs); \
				if(num_ptr < 2){ \
					__ret = 0; \
					__lv = __rv; \
				}else{ \
					/*STEP 4
					Se &__lv è contenuto nel blocco B gestito da storman e B è in uno snapshot.
					Verifica che mptr_addr appartiene allo snapshot di B. In caso contrario assegna 3 a __ret.*/\
					void** snapshot = retrieve_snapshot(handled_ptrs, start, end, num_ptr);\
					if(!is_in_snapshot(mptr_addr, snapshot, num_ptr)){\
						__ret = 3;\
					}else{ \
						/*Mantiene la proprietà degli snapshot.*/\
						void* newstart;\
						void* newend;	\
						size_t num, size, alignment;\
						void*** pointer_array;\
						\
						size = (size_t)(end-start);\
						alignment = retrieve_alignment(start);\
						block_alloc(mptr_addr, alignment, size);\
						\
						retrieve_block(mptr_addr, available_zones, &newstart, &newend);\
						copy_block_content(newstart, start, size);\
						\
						pointer_array = block_info((void**)&__lv, &start, &end, &num);\
						if(num != 0){\
							insert_corresp_ptrs(*pointer_array, start, (int)num, mptr_addr);\
						}\
						\
						/*Trova il corrispondente di &__lv in B'*/\
						void** newptr = get_corresp_ptr(&__lv, start, end, newstart);\
						\
						/*Quindi __rv sarà assegnato non a __lv ma al corrispondente indirizzo in B'.*/\
						(**(char**)newptr) = __rv;\
					} \
				}\
			}\
		}\
	}

	

