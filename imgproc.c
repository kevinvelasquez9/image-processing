#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include "plugin.h"
#include "image.h"
#include <assert.h>

char* fetch_dir() {
    char* dir = getenv("PLUGIN_DIR");
    return (dir != NULL) ? dir : "./plugins";
}

void fetch_files(char** plugin_files, char* plugin_dir, int num_elements) {
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
            // Realloc if we need to
            if (index == num_elements) {
                plugin_files = realloc(plugin_files, 2*num_elements);
                num_elements *= 2;
            }
            plugin_files[index] = (char*) malloc(sizeof(char*));
            plugin_files[index++] = file;
        }
    }
    if (index == num_elements) {
        plugin_files = realloc(plugin_files, 2*num_elements);
        num_elements *= 2;
    }
    plugin_files[index] = NULL;
}

void fetch_plugins(Plugin** plugins, char** plugin_files, int num_elements) {
    int index = 0;
    // Iterate over the plugin files
    while ((plugin_files)[index] != NULL) {
        // Realloc if we need to
        if (index == num_elements) {
            plugins = realloc(plugins, 2*num_elements);
            num_elements *= 2;
        }
        char* handle = dlopen(plugin_files[index], RTLD_LAZY);
<<<<<<< HEAD
        assert(plugins[index] != NULL);
        plugins[index]->handle = handle;
=======
        (plugins)[index]->handle = handle;
>>>>>>> 42cf6353437225078b72b8ac95ce5492d624e8b3
        // idk if dlsym() works here
        *(void **) (&(plugins[index])->get_plugin_name) = dlsym(handle, "get_plugin_name");
        *(void **) (&(plugins[index])->get_plugin_desc) = dlsym(handle, "get_plugin_desc");
        *(void **) (&(plugins[index])->parse_arguments) = dlsym(handle, "parse_arguments");
        *(void **) (&(plugins[index])->transform_image) = dlsym(handle, "transform_image");
        //plugins[index]->parse_arguments = dlsym(handle, "parse_arguments");
        //plugins[index]->transform_image = (Image* (*)(Image*, void*)) dlsym(handle, "transform_image");
        index++;
    }
}

int main() {
    char* plugin_dir = fetch_dir();
    int num_elements = 10;

    // We make the array of plugin file names
    char** plugin_files = (char**) malloc(num_elements * sizeof(char*));
    fetch_files(plugin_files, plugin_dir, num_elements);

    // We make the array of plugins
    Plugin** plugins = malloc(num_elements * sizeof(Plugin*));
<<<<<<< HEAD
    for (int i = 0; i < 5; i++) {
        plugins[i] = (Plugin*)malloc(sizeof(Plugin));
    }
    fetch_plugins(plugins, plugin_files, num_elements);

    for (int i = 0; i < 5; i++) {
        free(plugins[i]);
    }
    free(plugins);
    free(plugin_files);
    
=======
    for (int i = 0; i < num_elements; i++) {
        plugins[i] = malloc(sizeof(Plugin));
    }
    fetch_plugins(plugins, plugin_files, num_elements);


>>>>>>> 42cf6353437225078b72b8ac95ce5492d624e8b3
    return 0;
}
