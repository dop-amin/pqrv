#include <stdint.h>
#include <stdlib.h>

static int cmp_uint64_t(const void *a, const void *b)
{
    return (int)((*((const uint64_t *)a)) - (*((const uint64_t *)b)));
}

uint64_t median(uint64_t *l, size_t len)
{
    qsort(l, len, sizeof(uint64_t), cmp_uint64_t);
    return l[len >> 1];
}