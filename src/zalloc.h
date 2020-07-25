#ifndef ZALLOC_H
#define ZALLOC_H

static inline void *zalloc_r(signed int size, void **zone)
{
	char *ptr = *zone;
	*zone = ptr + size;
	return ptr;
}

#endif
