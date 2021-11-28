#pragma once

#include "utils.h"

int insert_new_block(Zone**, void*, void*);

int retrieve_block(void*, Zone*, void**, void**);

void release_block(void*, Zone**);

int has_multiple_ptrs(void*, void*, Pointer*);

//void clear_block(void**, size_t); -> Internal

void reduce_block(void**, void**, void**, Zone**);

int avb_space(void*, Zone*, size_t);

void expand_block(void*, Zone**, size_t);

//void copy_block_content(void*, void*, size_t); --> Internal

void copy_block(void**, void**, void**, size_t, size_t);

int are_identical_blocks(void*, void*, void*, void*);