#include "cache.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <zlib.h>
#include <limits.h>
#include <errno.h>

#define CHUNK 16384 // zlib chunk size

// Forward declarations for internal functions
static long get_mtime(const char *path);
static char* gzip_compress(const char *data, size_t data_len, size_t *compressed_size);
static void ensure_cache_dir_exists();

// Main cache function implementation
char* get_cached_or_generate(const char *source_path, size_t *compressed_size, content_generator_t generator) {
    ensure_cache_dir_exists();

    char cache_path[PATH_MAX];
    // Create a unique cache filename from the source path by replacing '/' with '_'
    char relative_source_path[PATH_MAX];
    if (strstr(source_path, g_project_root) == source_path) {
        snprintf(relative_source_path, sizeof(relative_source_path), "%s", source_path + strlen(g_project_root));
    } else {
        snprintf(relative_source_path, sizeof(relative_source_path), "%s", source_path);
    }
    for (char *p = relative_source_path; *p; p++) {
        if (*p == '/' || *p == '\\') *p = '_';
    }
    snprintf(cache_path, sizeof(cache_path), "%s/cache/%s.gz", g_project_root, relative_source_path);

    long source_mtime = get_mtime(source_path);
    long cache_mtime = get_mtime(cache_path);

    if (source_mtime == -1) {
        fprintf(stderr, "Error: Cannot get modification time for source file %s\n", source_path);
        return NULL; // Source file not found or error
    }

    // If cache is fresh, read from it
    if (cache_mtime != -1 && cache_mtime >= source_mtime) {
        char *cached_content = read_file_content(cache_path, compressed_size);
        if (cached_content) {
            return cached_content;
        }
    }

    // Cache is stale or doesn't exist, generate new content
    size_t content_size = 0;
    char *content = generator(source_path, &content_size);
    if (!content) {
        fprintf(stderr, "Error: Content generator failed for %s\n", source_path);
        return NULL;
    }

    // Compress the new content
    char *compressed_content = gzip_compress(content, content_size, compressed_size);
    free(content); // free uncompressed content

    if (!compressed_content) {
        fprintf(stderr, "Error: Gzip compression failed for %s\n", source_path);
        return NULL;
    }

    // Write the compressed content to the cache file
    FILE *fp = fopen(cache_path, "wb");
    if (fp) {
        fwrite(compressed_content, 1, *compressed_size, fp);
        fclose(fp);
    } else {
        fprintf(stderr, "Error: Could not write to cache file %s\n", cache_path);
        // We can still serve the content, just couldn't cache it
    }

    return compressed_content;
}


// --- Internal Helper Functions ---

static void ensure_cache_dir_exists() {
    char cache_dir_path[PATH_MAX];
    snprintf(cache_dir_path, sizeof(cache_dir_path), "%s/cache", g_project_root);
    struct stat st = {0};
    if (stat(cache_dir_path, &st) == -1) {
        mkdir(cache_dir_path, 0755);
    }
}

static long get_mtime(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return -1;
}

static char* gzip_compress(const char *data, size_t data_len, size_t *compressed_size) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    // + 12 for header, + 0.1% for overhead, + 5 for footer
    size_t buffer_size = data_len + 12 + (data_len / 1000) + 5;
    unsigned char *out_buffer = malloc(buffer_size);
    if (!out_buffer) return NULL;

    // Use GZIP encoding
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        free(out_buffer);
        return NULL;
    }

    strm.next_in = (Bytef *)data;
    strm.avail_in = data_len;
    strm.next_out = out_buffer;
    strm.avail_out = buffer_size;

    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        free(out_buffer);
        return NULL;
    }

    *compressed_size = strm.total_out;
    deflateEnd(&strm);
    return (char *)out_buffer;
}
