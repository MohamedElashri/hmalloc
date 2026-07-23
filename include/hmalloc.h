#ifndef HMALLOC_H
#define HMALLOC_H

#include <stddef.h>

void *hmalloc(size_t size);
void hfree(void *ptr);
void *hcalloc(size_t nmemb, size_t size);
void *hrealloc(void *ptr, size_t size);

#endif /* HMALLOC_H */
