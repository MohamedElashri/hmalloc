#ifndef HMALLOC_H
#define HMALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard POSIX allocator APIs */
void *hmalloc(size_t size);
void hfree(void *ptr);
void *hcalloc(size_t nmemb, size_t size);
void *hrealloc(void *ptr, size_t size);

/* Advanced / Alignment APIs */
int hposix_memalign(void **memptr, size_t alignment, size_t size);
void *hmemalign(size_t alignment, size_t size);
void *haligned_alloc(size_t alignment, size_t size);
void *hvalloc(size_t size);

#ifdef __cplusplus
}
#endif

#endif /* HMALLOC_H */
