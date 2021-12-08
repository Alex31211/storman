#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

//Struttura che implementa il nodo della lista di puntatori acquisiti
typedef struct ptr_node{
	void** address;
	int type; //tipo di storage puntato
	struct ptr_node* next;
}Pointer;

extern Pointer* handled_ptrs; //Testa della lista di puntatori acquisiti

int is_handled(void*, Pointer*);

void insert_new_pointer(void**, Pointer**, int);

void insert_corresp_ptrs(void**, void*, int, void*);

void** get_corresp_ptr(void*, void*, void*, void**);

void release_ptr(void*, Pointer**);

void release_ptrs(void***, size_t, Pointer**);

void** retrieve_ptr(Pointer*, void*, void*);

void update_ptrs(void***, void**, int, int, int, int);

void** copy_ptrs(void***, int);
