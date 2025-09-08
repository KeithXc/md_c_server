#ifndef UTILS_H
#define UTILS_H

#include <stddef.h> // For size_t
#include <limits.h> // For PATH_MAX

// Global variable to store the project's root directory
extern char g_project_root[PATH_MAX];

/**
 * @brief Gets the project's root directory based on the executable's location.
 *
 * This function determines the absolute path of the executable, then navigates
 * up the directory tree to find the project root (assuming the executable is
 * in a subdirectory like 'bin'). The result is stored in the output buffer.
 *
 * @param out A character buffer to store the resulting project root path.
 * @param size The size of the output buffer.
 */
void get_project_root(char *out, size_t size);

/**
 * @brief Reads an entire file into a dynamically allocated string.
 *
 * @param path The path to the file.
 * @param size Pointer to a size_t to store the file size.
 * @return A dynamically allocated string with the file content, or NULL on error.
 *         The caller is responsible for freeing the returned string.
 */
char* read_file_content(const char *path, size_t *size);

/**
 * @brief Replaces all occurrences of a substring in a string.
 *
 * @param orig The original string.
 * @param rep The substring to replace.
 * @param with The string to replace with.
 * @return A new dynamically allocated string with the replacements, or NULL on error.
 *         The caller is responsible for freeing the returned string.
 */
char* str_replace(const char *orig, const char *rep, const char *with);

#endif // UTILS_H
