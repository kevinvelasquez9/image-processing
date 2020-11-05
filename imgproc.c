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
            plugin_files[index++] = file;
        }
    }
}

void fetch_plugins(Plugin** plugins, char** plugin_files, int num_elements) {
    int index = 0;
    // Iterate over the plugin files
    while (plugin_files[index] != NULL) {
        // Realloc if we need to
        if (index == num_elements) {
            plugins = realloc(plugins, 2*num_elements);
            num_elements *= 2;
        }
        char* handle = dlopen(plugin_files[index], RTLD_LAZY);
        plugins[index]->handle = handle;
        // idk if dlsym() works here
        plugins[index]->get_plugin_name = (const char * (*)(void)) dlsym(handle, "get_plugin_name");
        plugins[index]->get_plugin_desc = (const char * (*)(void)) dlsym(handle, "get_plugin_desc");
        plugins[index]->parse_arguments = dlsym(handle, "parse_arguments");
        plugins[index]->transform_image = (Image* (*)(Image*, void*)) dlsym(handle, "transform_image");
        index++;
    }
}

int main() {
    char* plugin_dir = fetch_dir();
    int num_elements = 10;

    char** plugin_files = malloc(num_elements * sizeof(char*));
    fetch_files(plugin_files, plugin_dir, num_elements);

    Plugin** plugins = malloc(num_elements * sizeof(Plugin*));
    fetch_plugins(plugins, plugin_files, num_elements);

    return 0;
}