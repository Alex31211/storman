#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "pointer.h"
#include "zone.h"
#include "block.h"
#include "group.h"

int is_power_of_two(size_t);

size_t retrieve_alignment(void*);