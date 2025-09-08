#include "routes_post.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Serves a markdown file by injecting it into a template
void serve_post(struct mg_connection *c, struct mg_http_message *hm) {
    char md_path[PATH_MAX];
    // The URI is like "/post/subdir/file.md", we need "subdir/file.md"
    const char *relative_md_path = hm->uri.buf + strlen("/post/");
    snprintf(md_path, sizeof(md_path), "%s/md/%s", g_project_root, relative_md_path);
    // 在空格的位置截断
    for (char *p = md_path; *p; p++) {
        if (*p == ' ') {
            *p = '\0';
            break;
        }
    }

    if (strstr(md_path, "..") != NULL) {
        mg_http_reply(c, 400, "Content-Type: text/plain; charset=utf-8\r\n", "Bad Request\n");
        return;
    }

    size_t md_size;
    char *md_content = read_file_content(md_path, &md_size);
    if (md_content == NULL) {
        mg_http_reply(c, 404, "Content-Type: text/plain; charset=utf-8\r\n", "File Not Found\n");
        return;
    }

    size_t template_size;
    char template_path[PATH_MAX];
    snprintf(template_path, sizeof(template_path), "%s/templates/post.html", g_project_root);
    char *template_content = read_file_content(template_path, &template_size);
    if (template_content == NULL) {
        free(md_content);
        mg_http_reply(c, 500, "Content-Type: text/plain; charset=utf-8\r\n", "Internal Server Error: Could not read post template\n");
        return;
    }

    char *final_html = str_replace(template_content, "{{POST_CONTENT}}", md_content);

    mg_http_reply(c, 200, "Content-Type: text/html; charset=utf-8\r\n", "%s", final_html);

    free(md_content);
    free(template_content);
    free(final_html);
}
