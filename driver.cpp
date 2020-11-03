#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
using namespace std;

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

int main(int argc, char* argv[]) {
    string plugin_dir = fetch_dir();
    vector<string> plugin_files = fetch_files(plugin_dir);
    return 0;
}