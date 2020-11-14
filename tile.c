//
// Example plugin: it just swaps the blue and green color component
// values for each pixel in the source image.
//

#include <stdlib.h>
#include <string.h>
#include "image_plugin.h"

struct Arguments {
	// This plugin doesn't accept any command line arguments;
	// just define a single dummy field.
	int tiles;
};

const char *get_plugin_name(void) {
	return "tile";
}

const char *get_plugin_desc(void) {
	return "tile source image in an N*N arrangement";
}

void *parse_arguments(int num_args, char *args[]) {
	if (num_args != 1) {
		return NULL;
	}
 
	struct Arguments *a = calloc(1, sizeof(struct Arguments));
    a->tiles = atoi(args[5]);
    return (void*)a;
}


struct Image *transform_image(struct Image *source, void *arg_data) {
	struct Arguments *args = arg_data;
    
	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
		free(args);
		return NULL;
	}

    int tileWidth = source->width/args->tiles;
    int tileHeight = source->height/args->tiles;

    int excessWidth = source->width % args->tiles;
    int excessHeight = source->height % args->tiles;
    
    if (excessWidth != 0) {
        tileWidth++;
    }
    if (excessHeight != 0) {
        tileHeight++;
    }
    struct Image *intermediate = img_create(tileWidth, tileHeight);

    for (int i = 0; i < tileHeight; i++) {
        for (int j = 0; j < tileWidth; j++) {
            intermediate->data[i * tileWidth + j] = 
                source->data[(args->tiles * (i * source->width)) + (args->tiles * j)];
        }
    }

    for (unsigned int i = 0; i < source->height; i++) {
        for (unsigned int j = 0; j  < source->width; j++) {
            out->data[i * source->width + j] = intermediate->data[(i % tileHeight) * tileWidth + (j % tileWidth)];
        }
    }
    

    img_destroy(intermediate);
	free(args);

	return out;
}
