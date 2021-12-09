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
	/*1. Se l’indirizzo di __lv è di uno dei puntatori gestiti da storman allora assegna 2 a __ret.*/ \
	if(is_handled(&__lv, handled_ptrs)){ \
		__ret = 2; \
	}else{ \
		/*2. Se l’indirizzo di __lv non è contenuto in uno dei blocchi gestiti da storman allora assegna 1 a __ret.*/ \
		void* s_temp;\
		void* e_temp;\
		if(retrieve_block(&__lv, available_zones, &s_temp, &e_temp) == -1){ \
			__ret = 1; \
		}else{ \
			/*3. Assegna 0 a __ret e assegna __rv a __lv.*/ \
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
	/*1. Se __lv è uno dei puntatori gestiti da storman ritorna 2*/ \
	if(is_handled(&__lv, handled_ptrs)){ \
		__ret = 2; \
	}else{ \
		/*2. Se __lv non è contenuto in uno (B) dei blocchi gestiti da storman ritorna 1*/ \
		void* start; \
		void* end; \
		if(retrieve_block(&__lv, available_zones, &start, &end) == -1){ \
			__ret = 1; \
		}else{ \
			/*3. Se B non è in uno snapshot, assegna __rv a __lv e ritorna 0.*/ \
			if(!is_a_snapshot(start, end, &available_zones)){ \
				__ret = 0; \
				__lv = __rv; \
			}else{ \
				/*4. Se mptr_addr non appartiene allo snapshot di B, ritorna 3*/ \
				int num_ptr = has_multiple_ptrs(start, end, handled_ptrs); \
				void** snapshot = retrieve_snapshot(handled_ptrs, start, end, num_ptr); \
				if((mptr_addr == NULL) || (!is_in_snapshot(mptr_addr, snapshot, num_ptr))){ \
					__ret = 3;\
				}else{ \
					/*Mantiene la proprietà degli snapshot.*/\
					size_t size = (size_t)(end-start);\
					void* temp = (void*)&__lv;\
					void* newstart = copy_block(&temp, &start, size, size, mptr_addr);\
					void* newend = (void*)((size_t)newstart + size); \
					/*Trova il corrispondente di &__lv in B'*/\
					void** newptr = get_corresp_ptr((void*)&__lv, start, end, &newstart);\
					\
					/*Quindi __rv sarà assegnato non a __lv ma al corrispondente indirizzo in B'.*/\
					*((int*)newptr) = __rv;\
					to_alias(newstart, newend, &available_zones);\
					__ret = 0; \
				} \
			}\
		}\
	}
