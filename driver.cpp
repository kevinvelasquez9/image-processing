#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include "plugin.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

string fetch_dir() {
    char* dir = getenv("PLUGIN_DIR");
    return (dir != NULL) ? dir : "plugins";
}

vector<string> fetch_files(string plugin_dir) {
    vector<string> files;
    DIR* dir;
    struct dirent* dirp;
    // Return if we cannot read from directory
    if ((dir = opendir(plugin_dir.c_str())) == NULL) {
        cout << "Error " << errno << " opening " << plugin_dir << endl;
        return files;
    }
    // Iterate through all files and push back those with ".so"
    while ((dirp = readdir(dir)) != NULL) {
        string file = dirp->d_name;
        if (file.find(".so") != string::npos) {
            files.push_back(file);
        }
    }
    return files;
}

vector<Plugin> fetch_plugins(vector<string> plugin_files) {
    vector<Plugin> plugins;
    // Iterate over the plugin files
    for (vector<string>::iterator it = plugin_files.begin(); it != plugin_files.end(); it++) {
        Plugin plugin;
        plugin.handle = dlopen((*it).c_str(), RTLD_LAZY);
    }
    return plugins;
}

int main(int argc, char* argv[]) {
    string plugin_dir = fetch_dir();
    vector<string> plugin_files = fetch_files(plugin_dir);
    vector<Plugin> plugins = fetch_plugins(plugin_files);
    return 0;
}