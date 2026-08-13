#include "texture_loader.h"

#include "platform.h"
#include "errors.h"
#include "unwind.h"
#include <stdio.h>
#include <string.h>

#ifdef PLATFORM_CALCULATOR

/**
 * load textures from texture file
 */
textures_t load_textures(void) {

    header_t header = parse_header(eadk_external_data);
    check_for_missing_sections(&header);

    const char *wall_ptr = eadk_external_data + header.wall_offset;
    const char *titl_ptr = eadk_external_data + header.titl_offset;
    const char *sky_ptr  = eadk_external_data + header.sky_offset ;
    const char *enty_ptr = eadk_external_data + header.enty_offset;
    (void)next_u16(&titl_ptr); // ignore title screen dimentions
    (void)next_u16(&titl_ptr); // ignore title screen dimentions
    (void)next_u16(&sky_ptr); // ignore sky dimentions
    (void)next_u16(&sky_ptr); // ignore sky dimentions

    textures_t textures = {
        .wall_tex_w    = (uint16_t)next_u16(&wall_ptr),
        .wall_tex_h    = (uint16_t)next_u16(&wall_ptr),
        .entities_tex_w = (uint16_t)next_u16(&enty_ptr),
        .entities_tex_h = (uint16_t)next_u16(&enty_ptr),
        .wall_textures     = (color_t *)wall_ptr,
        .entities_textures = (color_t *)enty_ptr,
        .title_screen      = (color_t *)titl_ptr,
        .sky_texture       = (color_t *)sky_ptr,
    };
    return textures;
}

void unload_textures(textures_t *textures) {
    (void)textures;
    /* no texture unloading needed on calculator */
}

#define SECTION_NAME_LEN 4

/**
 * load a texture offset in the header by its name
 */
static void load_texture_offset(header_t *header, char *name, uint32_t offset) {
    if (strncmp(name, "wall", SECTION_NAME_LEN) == 0) {
        header->wall_offset = offset;
    }
    else if (strncmp(name, "titl", SECTION_NAME_LEN) == 0) {
        header->titl_offset = offset;
    }
    else if (strncmp(name, "sky ", SECTION_NAME_LEN) == 0) {
        header->sky_offset = offset;
    }
    else if (strncmp(name, "enty", SECTION_NAME_LEN) == 0) {
        header->enty_offset = offset;
    }
    else {
        /**
         * TODO: display or log warning
         */
    }
}

/**
 * check for any missing section
 */
void check_for_missing_sections(header_t *header) {
    if (header->wall_offset == 0)
        error_screen("unable to load wall texture:\n'wall' section not found");
    else if (header->titl_offset == 0) 
        error_screen("unable to load title screen texture:\n'titl' section not found");
    else if (header->sky_offset == 0)
        error_screen("unable to load sky texture:\n'sky ' section not found");
    else if (header->enty_offset == 0)
        error_screen("unable to load entity textures:\n'enty' section not found");

    // No problemo
}

/**
 * parses the header of the texture file
 */
header_t parse_header(const char *file_content) {
    header_t header = {
        .wall_offset = 0,
        .enty_offset = 0,
        .titl_offset = 0,
        .sky_offset  = 0,
    };

    // parse the header length
    uint32_t header_len = next_u32(&file_content);
    char name[SECTION_NAME_LEN + 1] = "";

    for (uint16_t i = 0; i < header_len; i += 8) {
        // read section name
        next_str(&file_content, name, SECTION_NAME_LEN);
        uint32_t offset = next_u32(&file_content);

        load_texture_offset(&header, name, offset);
    }

    return header;
}

/**
 * reads the next uint32 from the file and increament the pointer by 4 bytes
 */
uint32_t next_u32(const char **file_ptr) {
    if (file_ptr == NULL) return 0;
    if (*file_ptr == NULL) return 0;

    uint32_t u32 = ((**file_ptr) & 0xff)
    | ((*(*file_ptr + 1) & 0xff) << 8 )
    | ((*(*file_ptr + 2) & 0xff) << 16)
    | ((*(*file_ptr + 3) & 0xff) << 24);

    (*file_ptr) += 4;

    return u32;
}

/**
 * reads the next uint16 from the file and increament the pointer by 2 bytes
*/
uint16_t next_u16(const char **file_ptr) {
    if (file_ptr == NULL) return 0;
    if (*file_ptr == NULL) return 0;

    uint16_t u16 = ((**file_ptr) & 0xff)
    | ((*(*file_ptr + 1) & 0xff) << 8);

    (*file_ptr) += 2;

    return u16;
}

/**
 * reads the next string of length len
 * @warning assums the str is large enough
 */
void next_str(const char **file_ptr, char *str, uint16_t len) {
    if (file_ptr == NULL) return;
    if (*file_ptr == NULL) return;

    // copy the string
    memcpy(str, *file_ptr, len);

    // advance the pointer
    *file_ptr += len;
}

#else
#error PLATFROM NOT SUPPOTED
#endif