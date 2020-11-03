//
// Plugin data type 
//

#ifndef PLUGIN_H
#define PLUGIN_H

struct Plugin {
    void* handle;
    const char* (*get_plugin_name)(void);
    const char* (*get_plugin_desc)(void);
    void* (*parse_arguments)(int num_args, char* args[]);
    struct Image* (*transform_image)(struct Image* source, void* arg_data);
};

#endif