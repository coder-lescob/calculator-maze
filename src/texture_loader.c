#include "texture_loader.h"

#include <stdio.h>
#include <string.h>

#ifdef PLATFORM_CALCULATOR

/**
 * load textures from texture file
 */
textures_t load_textures(void) {
    header_t header = parse_header(eadk_external_data);

    const char *wall_ptr = eadk_external_data + header.wall_offset;
    const char *titl_ptr = eadk_external_data + header.titl_offset;
    const char *sky_ptr  = eadk_external_data + header.sky_offset;
    (void)next_u16(&titl_ptr); // ignore title screen dimentions
    (void)next_u16(&titl_ptr); // ignore title screen dimentions
    (void)next_u16(&sky_ptr); // ignore sky dimentions
    (void)next_u16(&sky_ptr); // ignore sky dimentions

    textures_t textures = {
        .wall_tex_w  = (uint16_t)next_u16(&wall_ptr),
        .wall_tex_h  = (uint16_t)next_u16(&wall_ptr),
        .wall_textures = (color_t *)wall_ptr,
        .title_screen = (color_t *)titl_ptr,
        .sky_texture = (color_t *)sky_ptr,
    };

    return textures;
}

/**
 * parses the header of the texture file
 */
header_t parse_header(const char *file_content) {
    header_t header;

    // parse the header length
    uint32_t header_len = next_u32(&file_content);
    char name[5] = {0};

    for (uint16_t i = 0; i < header_len; i += 8) {
        // read section name
        next_str(&file_content, name, 4);
        uint32_t offset = next_u32(&file_content);

        if (strcmp(name, "wall") == 0) {
            header.wall_offset = offset;
        }
        else if (strcmp(name, "titl") == 0) {
            header.titl_offset = offset;
        }
        else if (strcmp(name, "sky ") == 0) {
            header.sky_offset = offset;
        }
    }

    return header;
}

/**
 * reads the next uint32 from the file and increament the pointer by 4 bytes
 */
uint32_t next_u32(const char **file_ptr) {
    if (file_ptr == NULL) return 0;
    if (*file_ptr == NULL) return 0;

    uint32_t u32 = (**file_ptr)
    | (*(*file_ptr + 1) << 8 )
    | (*(*file_ptr + 2) << 16)
    | (*(*file_ptr + 3) << 24);

    (*file_ptr) += 4;

    return u32;
}

/**
 * reads the next uint16 from the file and increament the pointer by 2 bytes
*/
uint16_t next_u16(const char **file_ptr) {
    if (file_ptr == NULL) return 0;
    if (*file_ptr == NULL) return 0;

    uint16_t u16 = (**file_ptr)
    | (*(*file_ptr + 1) << 8 );

    (*file_ptr) += 2;

    return u16;
}

/**
 * reads the next string of length len
 */
void next_str(const char **file_ptr, char *str, uint16_t len) {
    // copy the string
    memcpy(str, *file_ptr, len);

    // advance the pointer
    *file_ptr += 4;
}

#else
#error PLATFROM NOT SUPPOTED
#endif