#include "routes_post.h"
#include "utils.h"
#include "cache.h"
#include "cmark.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Generator function to convert a Markdown file to an HTML string
static char* generate_html_from_md(const char *md_path, size_t *html_size) {
    size_t md_size;
    char *md_content = read_file_content(md_path, &md_size);
    if (md_content == NULL) {
        return NULL;
    }

    // Convert Markdown to HTML using cmark
    char *html_body_content = cmark_markdown_to_html(md_content, md_size, 0);
    free(md_content);
    if (html_body_content == NULL) {
        return NULL;
    }

    // Read the HTML template
    size_t template_size;
    char template_path[PATH_MAX];
    snprintf(template_path, sizeof(template_path), "%s/templates/post.html", g_project_root);
    char *template_content = read_file_content(template_path, &template_size);
    if (template_content == NULL) {
        free(html_body_content);
        return NULL;
    }

    // Inject the HTML content into the template
    char *final_html = str_replace(template_content, "{{POST_CONTENT}}", html_body_content);
    free(template_content);
    free(html_body_content);

    if (final_html) {
        *html_size = strlen(final_html);
    }
    return final_html;
}

// Serves a markdown file, using a cache to provide a compressed response
void serve_post(struct mg_connection *c, struct mg_http_message *hm) {
    char md_path[PATH_MAX];
    // The URI is like "/post/subdir/file.md", we need "subdir/file.md"
    const char *relative_md_path = hm->uri.buf + strlen("/post/");
    snprintf(md_path, sizeof(md_path), "%s/md/%s", g_project_root, relative_md_path);
    // Truncate at the first space to prevent argument injection
    for (char *p = md_path; *p; p++) {
        if (*p == ' ') {
            *p = '\0';
            break;
        }
    }

    // Basic security check
    if (strstr(md_path, "..") != NULL) {
        mg_http_reply(c, 400, "Content-Type: text/plain; charset=utf-8\r\n", "Bad Request\n");
        return;
    }

    size_t compressed_size = 0;
    char *compressed_content = get_cached_or_generate(md_path, &compressed_size, generate_html_from_md);

    if (compressed_content == NULL) {
        // Check if the file itself doesn't exist
        if (access(md_path, F_OK) != 0) {
            mg_http_reply(c, 404, "Content-Type: text/plain; charset=utf-8\r\n", "File Not Found\n");
        } else {
            mg_http_reply(c, 500, "Content-Type: text/plain; charset=utf-8\r\n", "Internal Server Error: Could not generate or retrieve content\n");
        }
        return;
    }

    // Serve the compressed content with the correct headers
    mg_printf(c,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Content-Encoding: gzip\r\n"
              "Content-Length: %zu\r\n"
              "\r\n",
              compressed_size);
    mg_send(c, compressed_content, compressed_size);

    // Mark the connection to be closed after the reply is sent.
    c->is_draining = 1;

    free(compressed_content);
}
