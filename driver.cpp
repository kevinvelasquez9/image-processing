#include <iostream>
#include <string>
using namespace std;

string fetch_dir() {
    char* dir = getenv("PLUGIN_DIR");
    return (dir != NULL) ? dir : "plugins";
}

int main(int argc, char* argv[]) {
    string plugin_dir = fetch_dir();
}