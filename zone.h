#pragma once

#include <stdlib.h>

#define MAX_BLOCKS 10

typedef enum groupType{
	SNAPSHOT = 0,
	ALIAS = 1
}Type;

typedef struct zone_node{
	size_t total;
	size_t available;
	void* allocated_memory;
	void* starting_addr[MAX_BLOCKS];
	void* ending_addr[MAX_BLOCKS];
	Type type[MAX_BLOCKS];
	struct zone_node* next;
}Zone;

extern Zone* available_zones; //Testa della lista di zone

//int zone_alloc(Zone**, Zone*, Zone*, size_t, Zone**); -> INTERNAL

int retrieve_allocation_zone(Zone**, size_t, size_t, Zone**);

void order_metadata(Zone**, int);