#include "core/lna_memory.h"

size_t lna_kilobytes(size_t value) { return value * 1024LL; }
size_t lna_megabytes(size_t value) { return lna_kilobytes(value) * 1024LL; }
size_t lna_gigabytes(size_t value) { return lna_megabytes(value) * 1024LL; }
