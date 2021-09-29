#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX_BLOCKS 10
//Struttura che implementa il nodo della lista di puntatori acquisiti
typedef struct ptr_node{
	void* address;
	struct ptr_node* next;
}Pointer;

typedef struct zone_node{
	size_t total;
	size_t available;
	void* starting_addr[MAX_BLOCKS];
	void* ending_addr[MAX_BLOCKS];
	struct zone_node* next;
}Zone;

extern Pointer* handled_ptrs; //Testa della lista di puntatori acquisiti
extern Zone* available_zones; //Testa della lista di zone

void retrieve_available_space(Zone**, size_t, Zone**);

int is_handled(void*, Pointer*);

int retrieve_block(void*, Zone*, void**, void**);

int has_multiple_ptr(void*, Pointer*, Zone*);

int is_power_of_two(size_t);

void release_ptr(void*, Pointer**);

void release_block(void*, Zone**);

void insert_new_pointer(void**, Pointer**);

int insert_new_block(Zone*, void*, void*);

int is_in_block(void*, Zone*);