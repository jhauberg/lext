#pragma once

#include <stddef.h> // size_t

void lxt_gen(char * result, size_t length, char const * pattern);
void lxt_genk(char * result, size_t length, char const * pattern, char const * name);
