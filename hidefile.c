#define _GNU_SOURCE
#include <string.h>
#include <stdarg.h> // handle variable args in open
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>  // for setting an error code
#include <dlfcn.h>  // for dynamic loading

#define MAX_TOKENS 256

int ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;


    int str_len = strlen(str);
    int suffix_len = strlen(suffix);


    if (suffix_len > str_len) return 0;


    if (strcmp(str + str_len - suffix_len, suffix) == 0) {
        return 1;
    } else {
        return 0;
    }
}


// interpose the open call
// open takes an optional third argument - the permissions for creating a file
// we don't care what it's set to but will have to handle the variable # of args
int open(const char *pathname, int flags, ...) {
   
    // get the BLOCKED env var
    char *b = getenv("BLOCKED");
    // printf("b: %s\n", b);

    char *blocked_files[MAX_TOKENS];
    char* b_copy = NULL;
    char* token = NULL;
    int num_blocked_files = 0;
    int i = 0;
   
    // if there are BLOCKED files:
    if (b != NULL) {
        // duplicate the BLOCKED env var
        b_copy = strdup(b);
        if (b_copy == NULL) {
            perror("strdup() failed");
            exit(EXIT_FAILURE);
        }
       
        // place each file name in hidden_files array
        token = strtok(b_copy, ":");
        while (token != NULL) {
            blocked_files[i++] = token;
            token = strtok(NULL, ":");
        }
   
        num_blocked_files = i;
    } else {
        // wrap the real open call with va_list
        int (*the_real_open)(const char* pathname, int flags, ...);
        the_real_open = dlsym(RTLD_NEXT, "open");
        if (the_real_open == NULL) {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        va_list args;
        va_start(args, flags);
        int fd = (*the_real_open)(pathname, flags, va_arg(args, int)); // call the real open
        va_end(args);
        return fd;
    }

    int result = 0;
    for (int j = 0; j < num_blocked_files; j++) {
        result = ends_with(pathname, blocked_files[j]);
        if (result == 1) {
            errno = EACCES;
            return -1;
        }
    }

    // wrap the real open call with va_list
    int (*the_real_open)(const char* pathname, int flags, ...);
    the_real_open = dlsym(RTLD_NEXT, "open");
    if (the_real_open == NULL) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, flags);
    int fd = (*the_real_open)(pathname, flags, va_arg(args, int)); // call the real open
    va_end(args);


    return fd;
}


// interpose the readdir call
// this isn't the system call but is used by many programs that need to read directories,
// like find and ls.
struct dirent *readdir(DIR *dirp) {
   
    // get the HIDDEN env var
    char *h = getenv("HIDDEN");
    char *hidden_files[MAX_TOKENS];
    char* h_copy = NULL;
    char* token = NULL;
    int num_hidden_files = 0;
    int i = 0;
   
    // if there are HIDDEN files:
    if (h != NULL) {
        // duplicate the HIDDEN env var
        h_copy = strdup(h);
        if (h_copy == NULL) {
            perror("strdup() failed");
            exit(EXIT_FAILURE);
        }
       
        // place each file name in hidden_files array
        token = strtok(h_copy, ":");
        while (token != NULL) {
            hidden_files[i++] = token;
            token = strtok(NULL, ":");
        }
   
        num_hidden_files = i;
    } else {
        struct dirent *(*real_readdir)(DIR* );
        real_readdir = dlsym(RTLD_NEXT, "readdir");
        if (real_readdir == NULL) {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        struct dirent* entry = real_readdir(dirp);
        return entry;
    }


    // call the real_readdir() to variable real_readdir
    struct dirent *(*real_readdir)(DIR* );
    real_readdir = dlsym(RTLD_NEXT, "readdir");
    if (real_readdir == NULL) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
   
    // call the real readdir() and save the result in variable entry
    struct dirent* entry;

    while ((entry = real_readdir(dirp)) != NULL) {


        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }


        int match = 0;
        for (int j = 0; j < num_hidden_files; j++) {
            if (strcmp(entry->d_name, hidden_files[j]) == 0) {
                match = 1;
                break;
            }
        }
        if (!match) {
            free(h_copy);
            return entry;
        }
    }

    free(h_copy);
    return NULL;
}
