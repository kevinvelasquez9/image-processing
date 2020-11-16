//
// Example plugin: it just swaps the blue and green color component
// values for each pixel in the source image.
//

#include <stdlib.h>
#include <string.h>
#include "image_plugin.h"
#include <stdio.h>

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


void *do_tile(struct Image *source, struct Image *out, 
    unsigned startX, unsigned endX, unsigned startY, 
    unsigned endY, int tiles) {

    int zeroY = 0;
    for (int y = startY; y < endY; y++) {
        int zeroX = 0;
        for (int x = startX; x < endX; x++) {
            out->data[y * source->width + x] = source->data[(tiles * (zeroY * source->width)) + (tiles * zeroX)];
            zeroX++;
        }
        zeroY++;
    }
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

    if (excessWidth !=0 ) {
        tileWidth++;
    } 
    if (excessHeight !=0) {
        tileHeight++;
    }

    uint32_t total_pixels = source->width * source->height;

    uint32_t arraySize = args->tiles + 1;
    uint32_t *xArray = (uint32_t*)calloc(arraySize, sizeof(uint32_t) * arraySize);
    uint32_t *yArray = (uint32_t*)calloc(arraySize, sizeof(uint32_t) * arraySize);

    /* Gets ranges of tile widths/heights and puts them in arrays */
    for (int i = 0; i < arraySize; i++) {
        if (i > excessWidth && excessWidth != 0) {
            xArray[i] = (i * tileWidth) - (i - excessWidth);
        } else {
            xArray[i] = i * tileWidth;
        }
        if (i > excessHeight && excessHeight != 0) {
            yArray[i] = (i * tileHeight) - (i - excessHeight);
        } else {
            yArray[i] = i * tileHeight;
        }
    }

    for (int y = 0; y < arraySize - 1; y++) {
        for (int x = 0; x < arraySize - 1; x++) {
            do_tile(source, out, xArray[x], xArray[x+1], yArray[y], yArray[y+1], args->tiles);
        }
    }
    free(xArray);
    free(yArray);
    
	free(args);

	return out;
}
