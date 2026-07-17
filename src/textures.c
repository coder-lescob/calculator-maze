#include "textures.h"

eadk_color_t *textures;

void load_textures(void) {
    textures = (eadk_color_t *)eadk_external_data;
}