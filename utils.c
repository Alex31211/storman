#include "utils.h"


//Controlla se l'argomento è una potenza di 2
/*Usage:
	- A: block_alloc -> verifica la legittimità dell'allineamento
*/
int is_power_of_two(size_t var){

	short ret = 0;

	if(var != 0){
		unsigned long pow;
		for(pow=1; pow>0; pow=pow<<1){
			if(pow >= var){
				if(pow == var){
					ret = 1;
				}
				break;
			}
		}
	}
	
	return ret;
}

//Recupera l'allineamento di un blocco allocato a partire da start
	//Nota: un indirizzo allineato a n byte ha ALMENO log2(n) bit meno significativi a 0.
	//		se addr è allineato a 16, sarà allineato anche a 32.
/*Usage:
	- E: pointer_assign -> alloca una nuova copia di un blocco
	- block: are_identical_blocks -> verifica se due blocchi sono allineati allo stesso modo
*/
size_t retrieve_alignment(void* start){

	size_t alignment = 0;
	size_t temp = sizeof(void*);

	while(((size_t)start % temp) == 0){
		alignment = temp;
		temp *= 2;
	}

	return alignment;
}