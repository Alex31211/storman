#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define MAX_BLOCKS 10
//Struttura che implementa il nodo della lista di puntatori acquisiti
typedef struct ptr_node{
	void** address;
	int type; //tipo di storage puntato
	struct ptr_node* next;
}Pointer;

typedef struct zone_node{
	size_t total;
	size_t available;
	void* allocated_memory;
	void* starting_addr[MAX_BLOCKS];
	void* ending_addr[MAX_BLOCKS];
	struct zone_node* next;
}Zone;

extern Pointer* handled_ptrs; //Testa della lista di puntatori acquisiti
extern Zone* available_zones; //Testa della lista di zone

int retrieve_allocation_zone(Zone**, size_t, Zone**);

int new_allocation_zone(Zone**, size_t, Zone**);

int is_handled(void*, Pointer*);

int retrieve_block(void*, Zone*, void**, void**);

int has_multiple_ptr(void*, Pointer*, Zone*);

int is_power_of_two(size_t);

void release_ptr(void*, Pointer**);

void release_block(void*, Zone**);

void insert_new_pointer(void**, Pointer**, int);

int insert_new_block(Zone**, void*, void*);

int is_avb_space(void*, Zone*, size_t);

int retrieve_ptr_type(void*, Pointer*);

int has_ptrs_left(void*, void*, void*, Pointer*);

void reduce_block(void**, void**, void**, Zone**);

void expand_block(void*, Zone**, size_t);

void copy_block_content(void*, void*, size_t);

void** retrieve_snapshot(Pointer*, void*, void*, int);

int is_in_snapshot(void*, void**, int);

int is_a_snapshot(void***, size_t);

void insert_corresp_ptrs(void***, void*, int, void*);

void** get_corresp_ptr(void*, void*, void*, void**);

size_t retrieve_alignment(void*);

int are_identical_blocks(void*, void*, void*, void*);

int get_set(void****, void*, int);

int add_in_set(void***, void*, void*, int, int);

int duplicate_in_set(void**, void*, int);