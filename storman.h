#include "utils.h"

//Modulo A
int block_alloc(void** ptr_addr, size_t alignment, size_t size);
int block_release (void** ptr_addr);
int pointer_release(void** ptr_addr);
int pointer_assign(void** ptr_addr, void* val);

//Modulo B
void*** block_info(void** ptr_addr, void** lowaddr, void** highaddr, size_t* num_ptr);
int pointer_info(void** ptr_addr, unsigned int* type);
/*#define assign (__lv, __rv, __ret)
		(//Se l’indirizzo di __lv è l’indirizzo di uno dei puntatori gestiti da
		//storman allora assegna 2 a __ret e non fa altro.
			if(is_handled(__lv, d_ptr, &block_ptr)){
				__ret = 2;
			}else{
				//Se l’indirizzo di __lv non è contenuto in uno dei blocchi gestiti da
				//storman allora assegna 1 a __ret e non fa altro.

				//Altrimenti, assegna 0 a __ret e assegna __rv a __lv.
			}
		)*/
//int block_realloc(void** ptr_addr, size_t newsize);

//Modulo E
//int dedup_blocks(void*** pointers, int num_ptrs);
//int toggle_snapshot(void** ptr_addr);
//int pointer_assign(void** ptr_addr, void* val, void** mptr_addr);
/*#define assign ( __lv , __rv , __ret , mptr_addr)
		(*/
			/*
			Se l’indirizzo di __lv è l’indirizzo di uno dei puntatori gestiti da
			storman allora assegna 2 a __ret e non fa altro.

			Se l’indirizzo di __lv non è contenuto in uno dei blocchi gestiti da
			storman allora assegna 1 a __ret e non fa altro.

			Se l’indirizzo di __lv è contenuto nel blocco B gestito da storman
			ma B non è in uno snapshot allora ignora mptr_addr, assegna 0 a
			__ret e assegna __rv a __lv.

			Se l’indirizzo di __lv è contenuto nel blocco B gestito da storman e
			B è in uno snapshot allora opera come segue.
				1. Verifica che mptr_addr sia l’indirizzo di un puntatore appartenente allo
					snapshot di B. In caso contrario assegna 3 a __ret e non fa altro.
				2. Mantiene la proprietà degli snapshot (copia B in B 0 , modifica B 0 ecc).
				3. Quindi __rv sarà assegnato non a __lv ma al corrispondente indirizzo in B0.
			*/
/*		)*/

