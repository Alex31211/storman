#pragma once

#include "utils.h"

void** retrieve_snapshot(Pointer*, void*, void*, int);

int is_in_snapshot(void*, void**, int);

int is_a_snapshot(void***, size_t);

//int add_in_set(void***, void*, void*, int, int); --> internal

int duplicate_in_set(void**, void*, int);

void reorder_addresses(void**, void**, int, int);

void**** group_duplicates(void**, void**, int, int*, int**);