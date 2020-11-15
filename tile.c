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
    
    int input = atoi(args[5]);
    struct Image *i = img_read_png(args[3]);
    if (input < 0 || input == 0 || input > (int) i->width || input > (int) i->height) {
        img_destroy(i);
        return NULL;
    }
    img_destroy(i);
	struct Arguments *a = calloc(1, sizeof(struct Arguments));
    a->tiles = input;
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
    int curHeight = tileHeight;
    for (unsigned int i = 0; i < out->height; i++) {
        if (excessHeight == 0) {
            curHeight = tileHeight - 1;
        }
        int curWidth = tileWidth;
        int widthHolder = excessWidth;
        for (unsigned int j = 0; j  < out->width; j++) {
            if (widthHolder == 0) {
                curWidth = tileWidth - 1;
            }
            out->data[i * out->width + j] = intermediate->data[(i % curHeight) * curWidth + (j % curWidth)];
            //out->data[i * source->width + j] = intermediate->data[(i % tileHeight) * tileWidth + (j % tileWidth)];
            if (j % tileWidth == 0 && widthHolder != 0) {
                widthHolder--;
            }
        }
        if (i % tileHeight == 0 && excessHeight != 0) {
            excessHeight--;
        }
    }
    

    img_destroy(intermediate);
	free(args);

	return out;
}
