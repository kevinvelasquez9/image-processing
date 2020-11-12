//
// Example plugin: it just swaps the blue and green color component
// values for each pixel in the source image.
//

#include <stdlib.h>
#include "image_plugin.h"

struct Arguments {
	// This plugin doesn't accept any command line arguments;
	// just define a single dummy field.
	int tiles;
};

const char *get_plugin_name(void) {
	return "mirrorv";
}

const char *get_plugin_desc(void) {
	return "mirror image vertically";
}

void *parse_arguments(int num_args, char *args[]) {
	(void) args; // this is just to avoid a warning about an unused parameter

	if (num_args != 1) {
		return NULL;
	}
	return calloc(1, sizeof(struct Arguments));
}

// Helper function to swap the blue and green color component values.
void create_tile(int j, int i, int tw, int th, struct Image *source, struct Image *out) {
	for (j; j < th; j++) {
        for (i; i < tw; i++) {
            int y = j * args->tiles;
            int x = i * args->tiles;
            out->data[j * tileWidth + i] = source->data[y * source->width + x];
        }
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

    int tileWidth = source->width/args->tiles;
    int tileHeight = source->height/args->tiles;

    int excessWidth = source->width % args->tiles;
    int excessHeight = source->height % args->tiles;
    
    int tempW;
    int tempH;
    for (int y = 0; y < args->tiles; y++) {
        if (excessHeight != 0) {
            tempH = tileHeight + 1;
        }
        for (int x = 0; x < args->tiles; x++) {
            create_tile(y * tempH, x * tempW, tempW, 
        }

    }
    


	free(args);

	return out;
}
