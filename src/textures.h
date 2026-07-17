#ifndef TEXTURES_H
#define TEXTURES_H

#include <eadk.h>

#define TEXTURE_WIDTH  (32)
#define TEXTURE_HEIGHT (32)
#define TEXTURE_SIZE   (TEXTURE_WIDTH * TEXTURE_HEIGHT)
#define NUM_TEXTURES   (2)
 
// theses images come from Wolfenstein 3D and are copyright by ID Software.
extern eadk_color_t *textures;

void load_textures(void);

#endif