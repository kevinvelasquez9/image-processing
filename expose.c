//
// Example plugin: it just swaps the blue and green color component
// values for each pixel in the source image.
//

#include <stdlib.h>
#include "image_plugin.h"
#include <string.h>

struct Arguments {
	// This plugin doesn't accept any command line arguments;
	// just define a single dummy field.
	float expose;
};

const char *get_plugin_name(void) {
	return "expose";
}

const char *get_plugin_desc(void) {
	return "adjust the intensity of all pixels";
}

void *parse_arguments(int num_args, char *args[]) {
	if (num_args != 1) {
		return NULL;
	}
    struct Arguments *a = calloc(1, sizeof(struct Arguments));
    a->expose = atof(args[0]);
    return (void*)a;
}

// Helper function to swap the blue and green color component values.
static uint32_t expose_pixel(uint32_t pix, float expose) {
	uint8_t r, g, b, a;
	img_unpack_pixel(pix, &r, &g, &b, &a);
    float rf = (float)r * expose;
    if (rf > 255.0f) {
        rf = 255.0f;
    }
    r = (uint8_t)rf;

    float gf = (float)g * expose;
    if (gf > 255.0f) {
        gf = 255.0f;
    }
    g = (uint8_t)gf;
    
    float bf = (float)b * expose;
    if (bf > 255.0f) {
        bf = 255.0f;
    }
    b = (uint8_t)bf;
	return img_pack_pixel(r, b, g, a);
}

struct Image *transform_image(struct Image *source, void *arg_data) {
	struct Arguments *args = arg_data;

	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
		free(args);
		return NULL;
	}

    if (args->expose <= 0.0f) {
        free(args);
        return NULL;
    }

    for (unsigned int i = 0; i < source->height * source->width; i++) {
        out->data[i] = expose_pixel(source->data[i], args->expose);
    }
	

	free(args);

	return out;
}
