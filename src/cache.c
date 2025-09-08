#include "cache.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include <limits.h>
#include <errno.h>

// Forward declarations
static time_t get_mtime(const char *path);
static char* gzip_compress(const char *data, size_t data_len, size_t *compressed_size);
static void ensure_cache_dir_exists();

CacheResult get_cached_or_generate(const char *source_path, content_generator_t generator) {
    CacheResult result = { .content = NULL, .size = 0, .etag = NULL, .last_modified = 0 };
    ensure_cache_dir_exists();

    char cache_path[PATH_MAX];
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

    time_t source_mtime = get_mtime(source_path);
    if (source_mtime == -1) {
        fprintf(stderr, "Error: Cannot get modification time for source file %s\n", source_path);
        return result;
    }
    result.last_modified = source_mtime;

    char etag_buffer[64];
    snprintf(etag_buffer, sizeof(etag_buffer), "\"%lx\"", (unsigned long)source_mtime);
    result.etag = strdup(etag_buffer);
    if (!result.etag) {
        return result;
    }

    time_t cache_mtime = get_mtime(cache_path);

    if (cache_mtime != -1 && cache_mtime >= source_mtime) {
        size_t cached_size;
        char *cached_content = read_file_content(cache_path, &cached_size);
        if (cached_content) {
            result.content = cached_content;
            result.size = cached_size;
            return result;
        }
    }

    size_t content_size = 0;
    char *content = generator(source_path, &content_size);
    if (!content) {
        free_cache_result(result);
        result.etag = NULL;
        return result;
    }

    size_t compressed_size = 0;
    char *compressed_content = gzip_compress(content, content_size, &compressed_size);
    free(content);

    if (!compressed_content) {
        free_cache_result(result);
        result.etag = NULL;
        return result;
    }

    FILE *fp = fopen(cache_path, "wb");
    if (fp) {
        fwrite(compressed_content, 1, compressed_size, fp);
        fclose(fp);
    }

    result.content = compressed_content;
    result.size = compressed_size;
    return result;
}

void free_cache_result(CacheResult result) {
    if (result.content) free(result.content);
    if (result.etag) free(result.etag);
}

static void ensure_cache_dir_exists() {
    char cache_dir_path[PATH_MAX];
    snprintf(cache_dir_path, sizeof(cache_dir_path), "%s/cache", g_project_root);
    struct stat st = {0};
    if (stat(cache_dir_path, &st) == -1) {
        mkdir(cache_dir_path, 0755);
    }
}

static time_t get_mtime(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return -1;
}

static char* gzip_compress(const char *data, size_t data_len, size_t *compressed_size) {
    z_stream strm = {0};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    size_t buffer_size = data_len + 12 + (data_len / 1000) + 5;
    unsigned char *out_buffer = malloc(buffer_size);
    if (!out_buffer) return NULL;

    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        free(out_buffer);
        return NULL;
    }

    strm.next_in = (Bytef *)data;
    strm.avail_in = data_len;
    strm.next_out = out_buffer;
    strm.avail_out = buffer_size;

    if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&strm);
        free(out_buffer);
        return NULL;
    }

    *compressed_size = strm.total_out;
    deflateEnd(&strm);
    return (char *)out_buffer;
}
