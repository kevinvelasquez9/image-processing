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

void fetch_files(char** plugin_files, char* plugin_dir) {
    DIR* dir;
    struct dirent* dirp;
    // Return if we cannot read from directory
    dir = opendir(plugin_dir);
    if (dir == NULL) {
        printf("Error opening %s", plugin_dir);
        return;
    }
    // Iterate through all files and push back those with ".so"
    int index = 0;
    while ((dirp = readdir(dir)) != NULL) {
        char* file = dirp->d_name;
        if (strstr(file, ".so") != NULL) {
            plugin_files[index] = (char*) malloc(sizeof(char*));
            plugin_files[index++] = file;
        }
    }
}

void fetch_plugins(char* dir, Plugin** plugins, char** plugin_files, int *i) {
    int index = 0;
    // Iterate over the plugin files
    while (plugin_files[index] != NULL) {
        char* f = malloc(strlen(dir) + strlen(plugin_files[index]) + 2);
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
    for (int j = 0; j < index; j++) {
        printf(plugins[j]->get_plugin_name());
        printf(": ");
        printf(plugins[j]->get_plugin_desc());
        printf("\n");
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

/*int handle_input(int argc, char* argv[], Plugin** plugins) {
    if (argc > 6) {
        return -1;
    }
    if (argc == 1) {
        printf("Usage: imgproc <command> [<command args...>]\n");
        printf("Commands are:\nlist\nexec <plugin> <input img> <output img> [<plugin args...>]\n");
    } else if (strcmp("list", argv[1]) == 0) {
        int num_plugins = 0;
        for (int i = 0; i < 5; i++) {
            if (plugins[i] != NULL) {
                num_plugins++;
            }
        }
        printf("Loaded %d plugin(s)\n", num_plugins);
    }
    return 0;
} */

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

    char command[100];
    char *handle;
    int numArgs = 0;
    
    if (strcmp("exec", argv[1]) == 0) {
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
        dlclose(command);

    } else if (strcmp("list", argv[1]) == 0) {
        char** plugin_files = (char**) calloc(num_elements, sizeof(char*));
        fetch_files(plugin_files, plugin_dir);
        // We make the array of plugins
        Plugin** plugins = calloc(num_elements, sizeof(Plugin*));
        for (int i = 0; i < num_elements; i++) {
            plugins[i] = calloc(1, sizeof(Plugin));
        }
        int index; 
        fetch_plugins(plugin_dir, plugins, plugin_files, &index);
        for (int i = 0; i < index; i++)  {
            free(plugin_files[i]);
        }
        free(plugin_files);
        for (int i = 0; i < num_elements; i++) {
            dlclose((plugins)[index]->handle);
            free(plugins[i]);
        }
        free(plugins);
    }
    return 0;
}
