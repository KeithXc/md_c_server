#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <limits.h> // For PATH_MAX

// Declare g_project_root as an external variable
// It is defined in server.c
extern char g_project_root[PATH_MAX];

// Function to find the project root directory
void get_project_root(char *out, size_t size);

char* read_file_content(const char *path, size_t *size);
char* str_replace(const char *orig, const char *rep, const char *with);

#endif // UTILS_H
