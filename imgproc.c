#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include "plugin.h"
#include "image.h"
#include <unistd.h>
#include <assert.h>


/* Finds directory containing pluging shared libraries */
char* fetch_dir() {
    char* dir = getenv("PLUGIN_DIR");
    return (dir != NULL) ? dir : "./plugins";
}


/* Returns number of .so files within a given directory */
int get_num_files(DIR *dir, struct dirent *dirp) {
    int i = 0;
    while ( ( dirp = readdir(dir) ) != NULL) {
        char* file = dirp->d_name;
        if (strstr(file, ".so") != NULL) {
            i++;
        }
    }
    rewinddir(dir);
    return i;
}


/* Links functions from shared object file to a Plugin struct */
void fetch_plugin(Plugin *plugin, char* handle) {
    assert(plugin != NULL);
    plugin->handle = handle;

    *(void **) (&(plugin)->get_plugin_name) = dlsym(handle, "get_plugin_name");

    *(void **) (&(plugin)->get_plugin_desc) = dlsym(handle, "get_plugin_desc");

    *(void **) (&(plugin)->parse_arguments) = dlsym(handle, "parse_arguments");

    *(void **) (&(plugin)->transform_image) = dlsym(handle, "transform_image");

}

/* Iterates through plugins in a directory and prints their information */
int plugin_iterator(DIR *dir, struct dirent* dirp, char *plugin_dir) {
    char *handle;
    while ( ( dirp = readdir(dir) ) != NULL) {
        char command[100];  
        Plugin *p = (Plugin*)malloc(sizeof(Plugin));
        char* file = dirp->d_name;
        if (strstr(file, ".so") != NULL) {
            strcpy(command, plugin_dir);
            strcat(command, "/");
            strcat(command, file);
            handle = dlopen(command, RTLD_LAZY);
            if (handle == NULL) {
                return -1;
            }
            fetch_plugin(p, handle);
            printf(p->get_plugin_name());
            printf(": ");
            printf(p->get_plugin_desc());
            printf("\n");
            dlclose(handle);
        }
        free(p);
    } 
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 6) {
        return -1;
    }
    /* Display possible command line arguments */
    if (argc == 1) {
        printf("Usage: imgproc <command> [<command args...>]\n");
        printf("Commands are:\nlist\nexec <plugin> <input img> <output img> [<plugin args...>]\n");
        return 0;
    }

    char* plugin_dir = fetch_dir();
    if (plugin_dir == NULL) {
        printf("Error: Plugin directory not found\n");
        return -1;
    }
    int num_elements = 5;
    
    /* Dynamically link to a single library */
    if (strcmp("exec", argv[1]) == 0) {
        char *handle;
        int numArgs = 0;
        char command[100];
        assert(argc >=5);
        Plugin *plugin = (Plugin*)malloc(sizeof(Plugin));
        /* Concatenates string that leads to .so file */
        strcpy(command, plugin_dir);
        strcat(command, "/");
        if (strcmp("mirrorh", argv[2]) == 0) {
            if (argc < 5) {
                printf("Error: Invald number of arguments\n");
                return -1;
            }
            strcat(command, "mirrorh.so");
        } else if (strcmp("mirrorv", argv[2]) == 0) {
            if (argc < 5) {
                printf("Error: Invald number of arguments\n");
                return -1;
            }
            strcat(command, "mirrorv.so");
        } else if (strcmp("swapbg", argv[2]) == 0) {
            if (argc < 5) {
                printf("Error: Invald number of arguments\n");
                return -1;
            }
            strcat(command, "swapbg.so");
        } else if (strcmp("tile", argv[2]) == 0) {
            if (argc < 6) {
                printf("Error: Invald number of arguments\n");
                return -1;
            }
            strcat(command, "tile.so");
            numArgs++;
        } else if (strcmp("expose", argv[2]) == 0) {
            if (argc < 6) {
                printf("Error: Invald number of arguments\n");
                return -1;
            }
            strcat(command, "expose.so");
            numArgs++;
        } else {
            printf("Error: Invalid or unknown image manipulation\n");
            return -1;
        }
        handle = dlopen(command, RTLD_LAZY);
        if (handle == NULL) {
            printf("Error: dlopen() could not create an executable object from the input file\n");
            return -1;
        }
        fetch_plugin(plugin, handle);
        if (access(argv[3], F_OK) == -1) {
            printf("Error: Input image doesn't exist\n");
            return -1;
        }
        /* Performs plugin's image manipulations */
        struct Image *i = img_read_png(argv[3]);
        if (i == NULL) {
            printf("Error: File read as NULL");
            return -1;
        }
        void* arg_memory = plugin->parse_arguments(numArgs, argv);
        if (arg_memory == NULL) {
            printf("Error: Incorrect args passed to plugin\n");
            return -1;
        }
        struct Image *result = plugin->transform_image(i, arg_memory);
        img_write_png(result, argv[4]);
        /* Deallocates memory/closes files */
        img_destroy(i);
        img_destroy(result);
        free(plugin);
        dlclose(handle);
        /* Was image created? */
        if (access(argv[4], F_OK) == -1) {
            printf("Error: Output image unsucessfully created\n");
            return -1;
        }

    } else if (strcmp("list", argv[1]) == 0) {
        DIR* dir;
        struct dirent* dirp;
        // Return if we cannot read from directory
        dir = opendir(plugin_dir);
        if (dir == NULL) {
            printf("Error: Error opening %s\n", plugin_dir);
            closedir(dir);
            return -1;
        }
        int numFiles = get_num_files(dir, dirp);
        printf("Error: Loaded %d plugin(s)\n", numFiles);
        int iteratorSuccess = plugin_iterator(dir, dirp, plugin_dir);
        if (iteratorSuccess == -1) {
            printf("Error: dlopen() could not create an executable object from the input file\n");
            return iteratorSuccess;
        }
        rewinddir(dir);
        closedir(dir);
    } else {
        printf("Error: Invalid command\n");
        return -1;
    }
    return 0;
}
