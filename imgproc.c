#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include "plugin.h"
#include "image.h"

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

void fetch_plugins(Plugin** plugins, char** plugin_files) {
    int index = 0;
    // Iterate over the plugin files
    while (plugin_files[index] != NULL) {
        char* handle = dlopen(plugin_files[index], RTLD_LAZY);
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
}

int handle_input(int argc, char* argv[], Plugin** plugins) {
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
        printf("%d\n", num_plugins);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    char* plugin_dir = fetch_dir();
    int num_elements = 5;

    // We make the array of plugin file names
    char** plugin_files = (char**) calloc(num_elements, sizeof(char*));
    fetch_files(plugin_files, plugin_dir);

    // We make the array of plugins
    Plugin** plugins = calloc(num_elements, sizeof(Plugin*));
    for (int i = 0; i < num_elements; i++) {
        plugins[i] = calloc(1, sizeof(Plugin));
    }
    fetch_plugins(plugins, plugin_files);

    int error_code = handle_input(argc, argv, plugins);

    return 0;
}
