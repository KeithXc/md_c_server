#include "routes_index.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

// Helper struct for dynamic string buffer
struct str_buffer {
    char *str;
    size_t len;
    size_t capacity;
};

// Function prototypes for static helpers
static void str_buffer_init(struct str_buffer *buf);
static void str_buffer_append(struct str_buffer *buf, const char *s);
static void str_buffer_free(struct str_buffer *buf);
static char* find_files_recursive_html(const char *base_path, const char *relative_path);

// Serves an HTML page with a clickable list of all .md files
void serve_index(struct mg_connection *c) {
    size_t template_size;
    char template_path[PATH_MAX];
    snprintf(template_path, sizeof(template_path), "%s/templates/index.html", g_project_root);
    char *template_content = read_file_content(template_path, &template_size);
    if (template_content == NULL) {
        mg_http_reply(c, 500, "Content-Type: text/plain; charset=utf-8\r\n", "Internal Server Error: Could not read index template\n");
        return;
    }

    char md_dir_path[PATH_MAX];
    snprintf(md_dir_path, sizeof(md_dir_path), "%s/md", g_project_root);
    char *file_list_html = find_files_recursive_html(md_dir_path, "");
    if (file_list_html == NULL) {
        free(template_content);
        mg_http_reply(c, 500, "Content-Type: text/plain; charset=utf-8\r\n", "Internal Server Error: Could not generate file list\n");
        return;
    }

    char *final_html = str_replace(template_content, "{{FILE_LIST}}", file_list_html);

    mg_http_reply(c, 200, "Content-Type: text/html; charset=utf-8\r\n", "%s", final_html);

    free(template_content);
    free(file_list_html);
    free(final_html);
}

// Initialize a string buffer
static void str_buffer_init(struct str_buffer *buf) {
    buf->len = 0;
    buf->capacity = 1024; // Initial capacity
    buf->str = (char *) malloc(buf->capacity);
    buf->str[0] = '\0';
}

// Append a string to the buffer, reallocating if necessary
static void str_buffer_append(struct str_buffer *buf, const char *s) {
    size_t s_len = strlen(s);
    if (buf->len + s_len + 1 > buf->capacity) {
        buf->capacity = (buf->len + s_len + 1) * 2;
        buf->str = (char *) realloc(buf->str, buf->capacity);
    }
    strcat(buf->str, s);
    buf->len += s_len;
}

// Free the string buffer
static void str_buffer_free(struct str_buffer *buf) {
    free(buf->str);
}

// Recursively finds .md files and builds an HTML list string
static char* find_files_recursive_html(const char *base_path, const char *relative_path) {
    struct str_buffer buf;
    str_buffer_init(&buf);

    char current_path[1024];
    snprintf(current_path, sizeof(current_path), "%s/%s", base_path, relative_path);
    
    DIR *d = opendir(current_path);
    if (d == NULL) return buf.str;

    struct dirent *dir;
    char temp_buffer[2048];

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", current_path, dir->d_name);

        struct stat path_stat;
        stat(file_path, &path_stat);

        char next_relative_path[1024];
        snprintf(next_relative_path, sizeof(next_relative_path), "%s%s", relative_path, dir->d_name);

        if (S_ISDIR(path_stat.st_mode)) {
            snprintf(temp_buffer, sizeof(temp_buffer), "<li class=\"tree-dir-li\"><span class=\"tree-dir\">%s</span><ul>", dir->d_name);
            str_buffer_append(&buf, temp_buffer);

            char new_relative_for_recursion[1024];
            snprintf(new_relative_for_recursion, sizeof(new_relative_for_recursion), "%s/", next_relative_path); 
            
            char *subdir_html = find_files_recursive_html(base_path, new_relative_for_recursion);
            str_buffer_append(&buf, subdir_html);
            free(subdir_html);

            str_buffer_append(&buf, "</ul></li>");
        } else if (strstr(dir->d_name, ".md") != NULL) {
            snprintf(temp_buffer, sizeof(temp_buffer), "<li class=\"tree-file\"><a href=\"/post/%s\">%s</a></li>", next_relative_path, dir->d_name);
            str_buffer_append(&buf, temp_buffer);
        }
    }
    closedir(d);
    return buf.str;
}
