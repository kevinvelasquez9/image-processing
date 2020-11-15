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


    uint32_t tileWidth = source->width/args->tiles;
    uint32_t tileHeight = source->height/args->tiles;

    uint32_t excessWidth = source->width % args->tiles;
    uint32_t excessHeight = source->height % args->tiles;

    uint32_t total_pixels = source->width * source->height;

    for (unsigned i = 0; i < total_pixels; i++) {
        uint32_t curCol = i % source->width;
        uint32_t curRow = i / source->width;
        
        uint32_t tileCol;
        uint32_t tileRow;

        if (curCol < (1 + tileWidth) * excessWidth) {
            tileCol = (curCol % (1 + tileWidth)) * args->tiles;
        } else {
            tileCol = ((curCol - (1 + tileWidth) * excessWidth) % tileWidth) *args->tiles;
        }

        if (curRow < (1 + tileHeight) * excessHeight) {
            tileRow = (curRow % (1 + tileHeight)) * args->tiles;
        } else {
            tileRow = ((curRow - (1 + tileHeight) * excessHeight) % tileHeight) *args->tiles;
        }

        uint32_t pix = tileCol + tileRow * source->width;
        out->data[i] = source->data[pix];
    }
    
    /* if (excessWidth != 0) {
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


    for (unsigned int i = 0; i < out->height; i++) {
        for (unsigned int j = 0; j  < out->width; j++) {
            out->data[i * source->width + j] = intermediate->data[(i % tileHeight) * tileWidth + (j % tileWidth)];
        }

    } */
    
	free(args);

	return out;
}
