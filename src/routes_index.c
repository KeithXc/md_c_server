#include "routes_index.h"
#include "utils.h"
#include "http_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zlib.h>

// Forward declaration
static char* generate_index_html();

// Serves the homepage with a collapsible file tree of the md/ directory.
void serve_index(struct mg_connection *c, struct mg_http_message *hm) {
    char md_dir_path[PATH_MAX];
    snprintf(md_dir_path, sizeof(md_dir_path), "%s/md", g_project_root);

    time_t latest_mtime = get_latest_mtime_in_dir(md_dir_path);
    if (latest_mtime == 0) {
        mg_http_reply(c, 200, "Content-Type: text/html; charset=utf-8\r\n", "<h1>No markdown files found.</h1>");
        return;
    }

    char etag[64];
    snprintf(etag, sizeof(etag), "\"%lx\"", (unsigned long)latest_mtime);

    if (handle_conditional_request(c, hm, etag, latest_mtime)) {
        return;
    }

    char cache_path[PATH_MAX];
    snprintf(cache_path, sizeof(cache_path), "%s/cache/__index.html.gz", g_project_root);
    
    struct stat st;
    if (stat(cache_path, &st) == 0 && st.st_mtime >= latest_mtime) {
        size_t compressed_size;
        char *compressed_content = read_file_content(cache_path, &compressed_size);
        if (compressed_content) {
            char last_modified_str[100];
            struct tm *tm = gmtime(&latest_mtime);
            strftime(last_modified_str, sizeof(last_modified_str), "%a, %d %b %Y %H:%M:%S GMT", tm);

            char headers[512];
            snprintf(headers, sizeof(headers),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=utf-8\r\n"
                     "Content-Encoding: gzip\r\n"
                     "ETag: %s\r\n"
                     "Last-Modified: %s\r\n"
                     "Content-Length: %zu\r\n"
                     "\r\n",
                     etag, last_modified_str, compressed_size);
            
            mg_send(c, headers, strlen(headers));
            mg_send(c, compressed_content, compressed_size);
            c->is_draining = 1;
            free(compressed_content);
            return;
        }
    }

    char *html_content = generate_index_html();
    if (!html_content) {
        mg_http_reply(c, 500, "", "Failed to generate index.");
        return;
    }

    z_stream strm = {0};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    
    size_t html_len = strlen(html_content);
    size_t compressed_capacity = deflateBound(&strm, html_len);
    char *compressed_content = malloc(compressed_capacity);

    strm.avail_in = html_len;
    strm.next_in = (Bytef *)html_content;
    strm.avail_out = compressed_capacity;
    strm.next_out = (Bytef *)compressed_content;

    deflate(&strm, Z_FINISH);
    size_t compressed_size = strm.total_out;
    deflateEnd(&strm);
    free(html_content);

    FILE *fp = fopen(cache_path, "wb");
    if (fp) {
        fwrite(compressed_content, 1, compressed_size, fp);
        fclose(fp);
    }

    char last_modified_str[100];
    struct tm *tm = gmtime(&latest_mtime);
    strftime(last_modified_str, sizeof(last_modified_str), "%a, %d %b %Y %H:%M:%S GMT", tm);

    char headers[512];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=utf-8\r\n"
             "Content-Encoding: gzip\r\n"
             "ETag: %s\r\n"
             "Last-Modified: %s\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             etag, last_modified_str, compressed_size);

    mg_send(c, headers, strlen(headers));
    mg_send(c, compressed_content, compressed_size);
    c->is_draining = 1;
    free(compressed_content);
}


// --- Private helper functions for HTML generation ---

static void append_string(char **buffer, size_t *capacity, const char *str) {
    size_t current_len = strlen(*buffer);
    size_t str_len = strlen(str);
    while (current_len + str_len + 1 > *capacity) {
        *capacity *= 2;
        *buffer = realloc(*buffer, *capacity);
    }
    strcat(*buffer, str);
}

static void list_files_recursive(const char *base_path, const char *rel_path, char **html_buffer, size_t *capacity) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s%s", base_path, rel_path);

    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0) {
        closedir(dir);
        return;
    }

    append_string(html_buffer, capacity, "<ul>");

    for (int i = 0; i < n; i++) {
        struct dirent *entry = namelist[i];
        if (entry->d_name[0] == '.') {
            free(entry);
            continue;
        }

        char full_rel_path[PATH_MAX];
        snprintf(full_rel_path, sizeof(full_rel_path), "%s%s", rel_path, entry->d_name);

        char entry_full_path[PATH_MAX];
        snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(entry_full_path, &st) != 0) {
            free(entry);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            char temp_buffer[PATH_MAX + 64];
            // Create a unique ID for the details element based on its path
            snprintf(temp_buffer, sizeof(temp_buffer), "<li><details id=\"details-%s\" open><summary>", full_rel_path);
            append_string(html_buffer, capacity, temp_buffer);
            
            append_string(html_buffer, capacity, entry->d_name);
            append_string(html_buffer, capacity, "</summary>");
            
            char next_rel_path[PATH_MAX];
            snprintf(next_rel_path, sizeof(next_rel_path), "%s/", full_rel_path);
            list_files_recursive(base_path, next_rel_path, html_buffer, capacity);
            
            append_string(html_buffer, capacity, "</details></li>");
        } else {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".md") == 0) {
                append_string(html_buffer, capacity, "<li><a href=\"/post/");
                append_string(html_buffer, capacity, full_rel_path);
                append_string(html_buffer, capacity, "\"> ");
                append_string(html_buffer, capacity, entry->d_name);
                append_string(html_buffer, capacity, "</a></li>");
            }
        }
        free(entry);
    }
    free(namelist);
    closedir(dir);
    append_string(html_buffer, capacity, "</ul>");
}

static char* generate_index_html() {
    size_t capacity = 4096;
    char *html_buffer = malloc(capacity);
    strcpy(html_buffer, "");

    char md_dir_path[PATH_MAX];
    snprintf(md_dir_path, sizeof(md_dir_path), "%s/md", g_project_root);
    list_files_recursive(md_dir_path, "/", &html_buffer, &capacity);

    size_t template_size;
    char template_path[PATH_MAX];
    snprintf(template_path, sizeof(template_path), "%s/templates/index.html", g_project_root);
    char *template_content = read_file_content(template_path, &template_size);
    if (!template_content) {
        free(html_buffer);
        return NULL;
    }

    char *final_html = str_replace(template_content, "{{FILE_LIST}}", html_buffer);
    free(template_content);
    free(html_buffer);

    return final_html;
}