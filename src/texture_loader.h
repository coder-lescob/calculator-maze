#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "platform.h"
#include "rendering.h"
#include "input.h"
#include "timings.h"

#ifdef PLATFORM_CALCULATOR
#include <eadk.h>
#else
#error PLATFORM NOT SUPPORTED
#endif

#define TEXTURE_WIDTH  (64)
#define TEXTURE_HEIGHT (64)
#define TEXTURE_SIZE   (TEXTURE_WIDTH * TEXTURE_HEIGHT)
#define NUM_TEXTURES   (8)

typedef struct {
    uint32_t wall_offset;
    uint32_t titl_offset;
    uint32_t sky_offset;
} header_t;

typedef struct {
    // sizes
    uint16_t wall_tex_w, wall_tex_h;
    //uint16_t entities_tex_w, entities_tex_h;

    // content
    color_t *wall_textures;
    //color_t *entities_textures;
    color_t *sky_texture;       // sky texture must be SCREEN_WIDTH x SCREEN_HEIGHT/2
    color_t *title_screen;
} textures_t;

/**
 * load textures from texture file
 */
textures_t load_textures(void);

/**
 * parses the header of the texture file
 */
header_t parse_header(const char *file_content);

/**
 * reads the next uint32 from the file and increament the pointer by 4 bytes
 */
uint32_t next_u32(const char **file_ptr);

/**
 * reads the next uint16 from the file and increament the pointer by 2 bytes
*/
uint16_t next_u16(const char **file_ptr);

/**
 * reads the next string of length len
 */
void next_str(const char **file_ptr, char *str, uint16_t len);

#endif