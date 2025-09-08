#include "utils.h"
#include <unistd.h> // for readlink
#include <libgen.h> // for dirname
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Definition of the global variable
char g_project_root[PATH_MAX];

void get_project_root(char *out, size_t size) {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);

    if (len != -1) {
        exe_path[len] = '\0';
        // First dirname gets the directory of the executable (e.g., /path/to/project/bin)
        char *exe_dir = dirname(exe_path);
        // Second dirname gets the parent directory (the project root)
        char *project_root = dirname(exe_dir);
        
        strncpy(out, project_root, size - 1);
        out[size - 1] = '\0'; // Ensure null-termination
    } else {
        // Fallback to current working directory if /proc/self/exe is not available
        if (getcwd(out, size) == NULL) {
            perror("getcwd() error");
            // Handle error appropriately, maybe exit or set a default
            strncpy(out, ".", size); 
        }
    }
}

// Helper function to read a file into a dynamically allocated buffer
char* read_file_content(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(*size + 1);
    if (buffer == NULL) {
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, *size, f);
    buffer[*size] = '\0';
    fclose(f);
    return buffer;
}

// Helper function to replace a substring in a string
char* str_replace(const char *orig, const char *rep, const char *with) {
    char *result; 
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep) return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0) return NULL;
    if (!with) with = "";
    len_with = strlen(with);

    ins = (char *) orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = (char *) malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result) return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

// Converts a 64-bit unsigned integer to a string.
void u64_to_str(unsigned long long n, char *out_buf) {
    char *p = out_buf;
    if (n == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    // Extract digits in reverse order
    while (n > 0) {
        *p++ = (n % 10) + '0';
        n /= 10;
    }
    *p = '\0';

    // Reverse the string
    char *start = out_buf;
    char *end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}