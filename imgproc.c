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

char* fetch_dir() {
    char* dir = getenv("PLUGIN_DIR");
    return (dir != NULL) ? dir : "./plugins";
}

/*void fetch_files(char** plugin_files, char* plugin_dir, uint8_t* index) {
    DIR* dir;
    struct dirent* dirp;
    // Return if we cannot read from directory
    dir = opendir(plugin_dir);
    if (dir == NULL) {
        printf("Error opening %s", plugin_dir);
        closedir(dir);
        return;
    }
    // Iterate through all files and push back those with ".so"
    while ( ( dirp = readdir(dir) ) != NULL) {
        char* file = dirp->d_name;
        if (strstr(file, ".so") != NULL) {
            plugin_files[index] = (char*) malloc(strlen(dirp->d_name) + 1);
            strncpy(plugin_files[index], dirp->d_name, strlen(dir->d_name) );
            /*plugin_files[index] = (char*) malloc(sizeof(char*));
            plugin_files[index] = file; 
            index++;
        }
    }
    rewinddir(dir);
    closedir(dir);
} */

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

void fetch_plugins(char* dir, Plugin** plugins, char** plugin_files, int *i) {
    int index = 0;
    // Iterate over the plugin files
    while (plugin_files[index] != NULL) {
        char f[100];
        strcpy(f, dir);
        strcat(f, "/");
        strcat(f, plugin_files[index]);
        char* handle = dlopen(f, RTLD_LAZY);
        (plugins)[index]->handle = handle;
        // idk if dlsym() works here
        *(void **) (&(plugins[index])->get_plugin_name) = dlsym(handle, "get_plugin_name");
        *(void **) (&(plugins[index])->get_plugin_desc) = dlsym(handle, "get_plugin_desc");
        *(void **) (&(plugins[index])->parse_arguments) = dlsym(handle, "parse_arguments");
        *(void **) (&(plugins[index])->transform_image) = dlsym(handle, "transform_image");
        //plugins[index]->parse_arguments = dlsym(handle, "parse_arguments");
        //plugins[index]->transform_image = (Image* (*)(Image*, void*)) dlsym(handle, "transform_image");
        index++;
        if (index == 5) {
            break;
        }
    }
    printf("Loaded %d plugin(s)\n", index);
    for (int j = 0; j < index; j++) {
        printf(plugins[j]->get_plugin_name());
        printf(": ");
        printf(plugins[j]->get_plugin_desc());
        printf("\n");
        dlclose(plugins[j]->handle);
    }

    *i = index;
    
}

void fetch_plugin(Plugin *plugin, char* handle) {
    plugin->handle = handle;

    *(void **) (&(plugin)->get_plugin_name) = dlsym(handle, "get_plugin_name");

    *(void **) (&(plugin)->get_plugin_desc) = dlsym(handle, "get_plugin_desc");

    *(void **) (&(plugin)->parse_arguments) = dlsym(handle, "parse_arguments");

    *(void **) (&(plugin)->transform_image) = dlsym(handle, "transform_image");

}

int main(int argc, char* argv[]) {
    if (argc > 6) {
        return -1;
    }
    if (argc == 1) {
        printf("Usage: imgproc <command> [<command args...>]\n");
        printf("Commands are:\nlist\nexec <plugin> <input img> <output img> [<plugin args...>]\n");
        return 0;
    }

    char* plugin_dir = fetch_dir();
    int num_elements = 5;

    //char command[100];
    char *handle;
    int numArgs = 0;
    
    if (strcmp("exec", argv[1]) == 0) {
        char command[100];
        assert(argc >=5);
        Plugin *plugin = (Plugin*)malloc(sizeof(Plugin));
        strcpy(command, plugin_dir);
        strcat(command, "/");
        if (strcmp("mirrorh", argv[2]) == 0) {
            strcat(command, "mirrorh.so");
        } else if (strcmp("mirrorv", argv[2]) == 0) {
            printf("mirrov called");
            strcat(command, "mirrorv.so");
        } else if (strcmp("swapbg", argv[2]) == 0) {
            strcat(command, "swapbg.so");
        } else if (strcmp("tile", argv[2]) == 0) {
            assert(argc == 6);
            strcat(command, "tile.so");
            numArgs++;
        } else if (strcmp("expose", argv[2]) == 0) {
            assert(argc == 6);
            strcat(command, "expose.so");
            numArgs++;
        } else {
            return -1;
        }
        handle = dlopen(command, RTLD_LAZY);
        fetch_plugin(plugin, handle);
        if (access(argv[3], F_OK) == -1) {
            printf("Input image doesn't exist");
            return -1;
        }
        struct Image *i = img_read_png(argv[3]);
        void* arg_memory = plugin->parse_arguments(numArgs, argv);
        struct Image *result = plugin->transform_image(i, arg_memory);
        img_write_png(result, argv[4]);
        img_destroy(i);
        img_destroy(result);
        free(plugin);
        dlclose(handle);

    } else if (strcmp("list", argv[1]) == 0) {
        uint8_t fileIndex = 0;
        DIR* dir;
        struct dirent* dirp;
        // Return if we cannot read from directory
        dir = opendir(plugin_dir);
        if (dir == NULL) {
            printf("Error opening %s", plugin_dir);
            closedir(dir);
            return -1;
        }
        int numFiles = get_num_files(dir, dirp);
        printf("Loaded %d plugin(s)\n", numFiles);
        // Iterate through all files and push back those with ".so"
        while ( ( dirp = readdir(dir) ) != NULL) {
            char command[100];
            Plugin *p = (Plugin*)malloc(sizeof(Plugin));
            char* file = dirp->d_name;
            if (strstr(file, ".so") != NULL) {
                strcpy(command, plugin_dir);
                strcat(command, "/");
                strcat(command, file);
                handle = dlopen(command, RTLD_LAZY);
                fetch_plugin(p, handle);
                printf(p->get_plugin_name());
                printf(": ");
                printf(p->get_plugin_desc());
                printf("\n");
                dlclose(handle);
            }
            free(p);
        }
        rewinddir(dir);
        closedir(dir);
        /* char** plugin_files = (char**) calloc(num_elements, sizeof(char*));
        fetch_files(plugin_files, plugin_dir, &fileIndex);
        // We make the array of plugins
        Plugin** plugins = calloc(num_elements, sizeof(Plugin*));
        for (int i = 0; i < num_elements; i++) {
            plugins[i] = calloc(1, sizeof(Plugin));
        }
        int index; 
        fetch_plugins(plugin_dir, plugins, plugin_files, &index);
        for (int i = 0; i < fileIndex; fileIndex++) {
            if (plugin_files[fileIndex] != NULL) {
                free(plugin_files[fileIndex]);
            }
        }
        free(plugin_files);
        for (int i = 0; i < num_elements; i++) {
            //dlclose((plugins)[index]->handle);
            free(plugins[i]);
        }
        free(plugins); */
    } 
    return 0;
}
