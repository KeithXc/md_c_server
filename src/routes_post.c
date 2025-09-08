#include "routes_post.h"
#include "utils.h"
#include "cache.h"
#include "cmark.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Generator function to convert a Markdown file to an HTML string
static char* generate_html_from_md(const char *md_path, size_t *html_size) {
    size_t md_size;
    char *md_content = read_file_content(md_path, &md_size);
    if (md_content == NULL) return NULL;

    char *html_body_content = cmark_markdown_to_html(md_content, md_size, 0);
    free(md_content);
    if (html_body_content == NULL) return NULL;

    size_t template_size;
    char template_path[PATH_MAX];
    snprintf(template_path, sizeof(template_path), "%s/templates/post.html", g_project_root);
    char *template_content = read_file_content(template_path, &template_size);
    if (template_content == NULL) {
        free(html_body_content);
        return NULL;
    }

    char *final_html = str_replace(template_content, "{{POST_CONTENT}}", html_body_content);
    free(template_content);
    free(html_body_content);

    if (final_html) {
        *html_size = strlen(final_html);
    }
    return final_html;
}

#include "http_helpers.h"

// Serves a markdown file, using a cache to provide a compressed response
void serve_post(struct mg_connection *c, struct mg_http_message *hm) {
    char md_path[PATH_MAX];
    const char *relative_md_path = hm->uri.buf + strlen("/post/");
    snprintf(md_path, sizeof(md_path), "%s/md/%s", g_project_root, relative_md_path);
    for (char *p = md_path; *p; p++) {
        if (*p == ' ') { *p = '\0'; break; } // Corrected escaping for null terminator
    }

    if (strstr(md_path, "..") != NULL) {
        mg_http_reply(c, 400, "Content-Type: text/plain; charset=utf-8\r\n", "Bad Request\n");
        return;
    }

    CacheResult cache_result = get_cached_or_generate(md_path, generate_html_from_md);

    if (cache_result.content == NULL) {
        free_cache_result(cache_result);
        if (access(md_path, F_OK) != 0) {
            mg_http_reply(c, 404, "Content-Type: text/plain; charset=utf-8\r\n", "File Not Found\n");
        } else {
            mg_http_reply(c, 500, "Content-Type: text/plain; charset=utf-8\r\n", "Internal Server Error\n");
        }
        return;
    }

    // Use the new helper to handle conditional requests
    if (handle_conditional_request(c, hm, cache_result.etag, cache_result.last_modified)) {
        free_cache_result(cache_result);
        return; // Response (304) was already sent by the helper
    }

    // Format timestamp for Last-Modified header
    char last_modified_str[100];
    struct tm *tm = gmtime(&cache_result.last_modified);
    strftime(last_modified_str, sizeof(last_modified_str), "%a, %d %b %Y %H:%M:%S GMT", tm);

    // Serve the content with all headers
    char headers[512];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=utf-8\r\n"
             "Content-Encoding: gzip\r\n"
             "ETag: %s\r\n"
             "Last-Modified: %s\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             cache_result.etag,
             last_modified_str,
             cache_result.size);

    mg_send(c, headers, strlen(headers));
    mg_send(c, cache_result.content, cache_result.size);

    c->is_draining = 1;
    free_cache_result(cache_result);
}
