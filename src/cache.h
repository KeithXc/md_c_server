#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

/**
 * @brief A structure to hold the result of a cache retrieval or generation.
 */
typedef struct {
    char *content;          // Pointer to the (compressed) content buffer
    size_t size;            // Size of the content buffer
    char *etag;             // A unique identifier for the content (e.g., based on timestamp)
    time_t last_modified;   // The modification timestamp of the source file
} CacheResult;


/**
 * @brief A function pointer type for a function that generates content from a source file.
 */
typedef char* (*content_generator_t)(const char *source_path, size_t *content_size);

/**
 * @brief Retrieves compressed content from cache or generates it if missing/stale.
 *
 * @param source_path The absolute path to the original source file.
 * @param generator A function pointer to the content generator.
 * @return A CacheResult struct. The pointers within the struct must be freed by the caller.
 *         If an error occurs, the pointers in the returned struct will be NULL.
 */
CacheResult get_cached_or_generate(
    const char *source_path,
    content_generator_t generator
);

/**
 * @brief Frees the memory allocated for a CacheResult's members.
 */
void free_cache_result(CacheResult result);


#endif // CACHE_H
