#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief A function pointer type for a function that generates content from a source file.
 *
 * @param source_path The path to the source file (e.g., a Markdown file).
 * @param content_size A pointer to a size_t variable to store the size of the generated content.
 * @return A dynamically allocated string containing the generated content, or NULL on failure.
 *         The caller is responsible for freeing this memory.
 */
typedef char* (*content_generator_t)(const char *source_path, size_t *content_size);

/**
 * @brief Retrieves a compressed file from the cache or generates and caches it if it's missing or stale.
 *
 * This function checks for a valid cached version of the content for the given source_path.
 * A cache is valid if the cached entry exists and the source file has not been modified since it was cached.
 *
 * If a valid cache is found, its compressed content is read and returned.
 * If not, the `generator` function is called to produce the fresh content, which is then
 * compressed using Gzip, saved to the cache, and the compressed content is returned.
 *
 * @param source_path The absolute path to the original source file (e.g., /path/to/file.md).
 * @param compressed_size A pointer to a size_t variable where the size of the returned compressed data will be stored.
 * @param generator A function pointer to the content generator to be called if the cache is invalid.
 * @return A dynamically allocated buffer containing the Gzip-compressed content. The caller is responsible for freeing this buffer.
 *         Returns NULL if any step (file reading, content generation, compression, memory allocation) fails.
 */
char* get_cached_or_generate(
    const char *source_path,
    size_t *compressed_size,
    content_generator_t generator
);

#endif // CACHE_H
