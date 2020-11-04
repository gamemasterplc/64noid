#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <vector>
#include <string>
#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tinyxml2.h"
#include "exoquant.h"

#define IMAGE_FORMAT_I4 0
#define IMAGE_FORMAT_I8 1
#define IMAGE_FORMAT_IA4 2
#define IMAGE_FORMAT_IA8 3
#define IMAGE_FORMAT_IA16 4
#define IMAGE_FORMAT_CI4 5
#define IMAGE_FORMAT_CI8 6
#define IMAGE_FORMAT_RGBA16 7
#define IMAGE_FORMAT_RGBA32 8
#define IMAGE_FORMAT_MAX 9
#define IMAGE_FORMAT_AUTO 9

#define PALETTE_FORMAT_RGBA16 0
#define PALETTE_FORMAT_IA16 1
#define PALETTE_FORMAT_MAX 2
#define PALETTE_FORMAT_AUTO 2
#define PALETTE_FORMAT_INVALID 2

struct XMLSpriteFrame {
    uint32_t image_idx;
    unsigned int delay;
};

struct XMLSpriteAnimation {
    std::string id;
    std::vector<XMLSpriteFrame> frames;
};

struct XMLSpriteImage {
    uint8_t *data_rgba32;
    int width;
    int height;
    std::string id;
    uint32_t format;
    uint32_t pal_format;
    float origin_x;
    float origin_y;
};

struct XMLSprite {
    std::vector<XMLSpriteAnimation> animations;
    std::vector<XMLSpriteImage> images;
};

struct SpriteAnimFrame {
    uint16_t image;
    uint16_t delay;
};

struct SpriteAnim {
    uint32_t name_hash;
    uint32_t num_frames;
    uint32_t frame_ofs;
};

struct SpriteImage {
    uint32_t id_hash;
    uint32_t data_ofs;
    uint32_t pal_data_ofs;
    uint32_t spr_ofs;
    uint32_t gfx_ofs;
    int16_t origin_x;
    int16_t origin_y;
    uint16_t format;
    uint16_t pal_format;
    uint16_t w;
    uint16_t h;
};

struct SpriteHeader {
    uint16_t magic;
    uint16_t ref_count;
    uint16_t num_animations;
    uint16_t num_images;
    uint32_t animations_ofs;
    uint32_t images_ofs;
};

static char *prog_name;
static XMLSprite xml_sprite;

int bpp_table[IMAGE_FORMAT_MAX] = { 4, 8, 4, 8, 16, 4, 8, 16, 32 };
int slice_pixel_count[IMAGE_FORMAT_MAX] = { 8192, 4096, 8192, 4096, 2048, 4096, 2048, 2048, 1024 };

uint8_t color_8bit_to_3bit[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x07,0x07,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07
};

uint8_t color_8bit_to_4bit[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08,
    0x08,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09,
    0x09,0x09,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a,
    0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b,
    0x0b,0x0b,0x0b,0x0b, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0f, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f
};

uint8_t color_8bit_to_5bit[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x08,0x08,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x0a,
    0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f,
    0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x11,0x11,0x11,0x11, 0x11,0x11,0x11,0x11,
    0x12,0x12,0x12,0x12, 0x12,0x12,0x12,0x12, 0x12,0x13,0x13,0x13, 0x13,0x13,0x13,0x13,
    0x13,0x14,0x14,0x14, 0x14,0x14,0x14,0x14, 0x14,0x15,0x15,0x15, 0x15,0x15,0x15,0x15,
    0x15,0x16,0x16,0x16, 0x16,0x16,0x16,0x16, 0x16,0x16,0x17,0x17, 0x17,0x17,0x17,0x17,
    0x17,0x17,0x18,0x18, 0x18,0x18,0x18,0x18, 0x18,0x18,0x19,0x19, 0x19,0x19,0x19,0x19,
    0x19,0x19,0x1a,0x1a, 0x1a,0x1a,0x1a,0x1a, 0x1a,0x1a,0x1b,0x1b, 0x1b,0x1b,0x1b,0x1b,
    0x1b,0x1b,0x1b,0x1c, 0x1c,0x1c,0x1c,0x1c, 0x1c,0x1c,0x1c,0x1d, 0x1d,0x1d,0x1d,0x1d,
    0x1d,0x1d,0x1d,0x1e, 0x1e,0x1e,0x1e,0x1e, 0x1e,0x1e,0x1e,0x1f, 0x1f,0x1f,0x1f,0x1f
};

void PrintUsage()
{
    printf("Usage: %s in [out]\n", prog_name);
    printf("In is the input XML.\n");
    printf("Out is the Output Sprite File.\n" "This will be calculated from in's filename by default.\n");
    getchar();
    exit(1);
}

void PrintError(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    getchar();
    exit(1);
    va_end(args);
}

int strcmpins(const char *a, const char *b)
{
    while ((*a != 0) && (*b != 0)) {
        int diff = ::tolower(*a) - ::tolower(*b);
        if (diff) {
            return diff;
        }
        a++;
        b++;
    }
    return ::tolower(*a) - ::tolower(*b);
}

uint32_t FindImageId(std::string string, XMLSpriteFrame *frame)
{
    for (uint32_t i = 0; i < xml_sprite.images.size(); i++) {
        if (xml_sprite.images[i].id == string) {
            return i;
        }
    }
    return UINT32_MAX;
}

uint32_t GetImageFormat(std::string name)
{
    const char *format_names[IMAGE_FORMAT_MAX] = { "i4", "i8", "ia4", "ia8", "ia16", "ci4", "ci8", "rgba16", "rgba32" };
    for (uint32_t i = 0; i < IMAGE_FORMAT_MAX; i++) {
        if (!strcmpins(name.c_str(), format_names[i])) {
            return i;
        }
    }
    return UINT32_MAX;
}

uint32_t GetPaletteFormat(std::string name)
{
    const char *format_names[PALETTE_FORMAT_MAX] = { "rgba16", "ia16" };
    for (uint32_t i = 0; i < PALETTE_FORMAT_MAX; i++) {
        if (!strcmpins(name.c_str(), format_names[i])) {
            return i;
        }
    }
    return UINT32_MAX;
}

void ParseSpriteAnims(tinyxml2::XMLElement *element)
{
    tinyxml2::XMLElement *anim_element = element->FirstChildElement("animation");
    while (anim_element) {
        tinyxml2::XMLElement *frame_element = anim_element->FirstChildElement("frame");
        if (frame_element) {
            XMLSpriteAnimation animation;
            while (frame_element) {
                XMLSpriteFrame frame;
                const char *image_name = frame_element->Attribute("image");
                if (!image_name) {
                    PrintError("ERROR: Missing image attribute in frame element.\n");
                }
                uint32_t image_idx = FindImageId(image_name, &frame);
                if (image_idx == UINT32_MAX) {
                    PrintError("Failed to Find Image with ID %s.\n", image_name);
                }
                frame.image_idx = image_idx;
                frame.delay = frame_element->UnsignedAttribute("delay", 6);
                frame_element = frame_element->NextSiblingElement("frame");
                animation.frames.push_back(frame);
            }
            xml_sprite.animations.push_back(animation);
        }
        const char *anim_id = anim_element->Attribute("id");
        if (!anim_id) {
            printf("WARNING: Missing animation ID.\n" "Using animation ID %d instead.\n", xml_sprite.animations.size() - 1);
            xml_sprite.animations.back().id = std::to_string(xml_sprite.animations.size() - 1);
        } else {
            xml_sprite.animations.back().id = anim_id;
        }
        anim_element = anim_element->NextSiblingElement("animation");
    }
}

void ParseSpriteImages(std::string xml_name, tinyxml2::XMLElement *element)
{
    std::string temp;
    if (xml_name.find_last_of("\\/") != std::string::npos) {
        temp = xml_name.substr(0, xml_name.find_last_of("\\/"));
    } else {
        temp = "";
    }
    std::string xml_base_path = temp;
    tinyxml2::XMLElement *image_element = element->FirstChildElement("image");
    while (image_element) {
        XMLSpriteImage image;
        const char *filename = image_element->Attribute("filename");
        if (!filename) {
            PrintError("ERROR: Missing filename attribute in image element.\n");
        }
        temp = filename;
        std::string default_id;
        if (temp.find_last_of("\\/") != std::string::npos) {
            default_id = temp.substr(temp.find_last_of("\\/"), temp.find_last_of("."));
        } else {
            default_id = temp.substr(0, temp.find_last_of("."));
        }
        std::string path;
        if (xml_base_path != "") {
            path = xml_base_path + "\\" + filename;
        } else {
            path = filename;
        }
        int components_temp;
        image.data_rgba32 = stbi_load(path.c_str(), &image.width, &image.height, &components_temp, 4);
        if (!image.data_rgba32) {
            printf("Failed to Load Image %s. Cause %s.\n", path.c_str(), stbi_failure_reason());
            getchar();
            exit(1);
        }
        if (image_element->Attribute("id")) {
            image.id = image_element->Attribute("id");
        } else {
            image.id = default_id;
        }
        image.origin_x = image_element->FloatAttribute("origin_x", 0.5f);
        image.origin_y = image_element->FloatAttribute("origin_y", 0.5f);
        const char *image_format_value = image_element->Attribute("format");
        if(image_format_value) {
            uint32_t format = GetImageFormat(image_format_value);
            if (format == UINT32_MAX) {
                PrintError("Invalid Image Format %s.\n", image_format_value);
            }
            image.format = format;
        } else {
            image.format = IMAGE_FORMAT_AUTO;
        }
        if (image.format != IMAGE_FORMAT_AUTO && bpp_table[image.format] == 4 && (image.width % 2) != 0) {
            PrintError("Image %s has odd width.\n", path.c_str());
        }
        const char *palette_format_value = image_element->Attribute("palette_format");
        if (palette_format_value) {
            uint32_t format = GetPaletteFormat(palette_format_value);
            if (format == UINT32_MAX) {
                printf("Invalid Image Format %s.\n", palette_format_value);
                getchar();
                exit(1);
            }
            image.pal_format = format;
        } else {
            image.pal_format = PALETTE_FORMAT_AUTO;
        }
        xml_sprite.images.push_back(image);
        image_element = image_element->NextSiblingElement("image");
    }
}

void LoadSpriteXML(std::string filename)
{
    tinyxml2::XMLDocument document;
    tinyxml2::XMLError error = document.LoadFile(filename.c_str());
    if(error != tinyxml2::XML_SUCCESS) {
        PrintError("Failed to Load XML File.\n");
    }
    tinyxml2::XMLNode *root = document.FirstChild();
    if (!root) {
        PrintError("Failed to Read XML File.\n");
    }
    tinyxml2::XMLElement *sprite_element = root->NextSiblingElement("sprite");
    if (!sprite_element) {
        PrintError("XML File has missing sprite element.\n");
    }
    XMLSprite *new_sprite = new XMLSprite;
    tinyxml2::XMLElement *images_element = sprite_element->FirstChildElement("images");
    if (images_element) {
        ParseSpriteImages(filename, images_element);
    } else {
        PrintError("XML File has missing images element.\n");
    }
    tinyxml2::XMLElement *anims_element = sprite_element->FirstChildElement("animations");
    if (anims_element) {
        ParseSpriteAnims(anims_element);
    }
    if (xml_sprite.animations.size() > UINT16_MAX) {
        PrintError("Exceeded %d Animations. You Have %d Animations.\n", UINT16_MAX, xml_sprite.animations.size());
    }
    if (xml_sprite.images.size() > UINT16_MAX) {
        PrintError("Exceeded %d Images. You Have %d Images.\n", UINT16_MAX, xml_sprite.images.size());
    }
}

void AlignFile(FILE *file, uint32_t alignment)
{
    uint32_t ofs = ftell(file);
    while (ofs % alignment) {
        uint8_t zero = 0;
        fwrite(&zero, 1, 1, file);
        ofs++;
    }
}

uint32_t AlignNumber(uint32_t ofs, uint32_t alignment)
{
    return ((ofs + alignment - 1) / alignment) * alignment;
}

void WriteFileU8(FILE *file, uint8_t value)
{
    uint8_t temp = value;
    fwrite(&temp, 1, 1, file);
}

void WriteFileU16(FILE *file, uint16_t value)
{
    uint16_t temp = (value & 0xFF) << 8;
    temp |= (value & 0xFF00) >> 8;
    fwrite(&temp, 2, 1, file);
}

void WriteFileS16(FILE *file, int16_t value)
{
    int16_t temp = (value & 0xFF) << 8;
    temp |= (value & 0xFF00) >> 8;
    fwrite(&temp, 2, 1, file);
}

void WriteFileU32(FILE *file, uint32_t value)
{
    uint32_t temp = (value & 0x000000FF) << 24;
    temp |= (value & 0x0000FF00) << 8;
    temp |= (value & 0x00FF0000) >> 8;
    temp |= (value & 0xFF000000) >> 24;
    fwrite(&temp, 4, 1, file);
}

void WriteFileS32(FILE *file, int32_t value)
{
    int32_t temp = (value & 0x000000FF) << 24;
    temp |= (value & 0x0000FF00) << 8;
    temp |= (value & 0x00FF0000) >> 8;
    temp |= (value & 0xFF000000) >> 24;
    fwrite(&temp, 4, 1, file);
}

void WriteFileFloat(FILE *file, float value)
{
    float temp = value;
    WriteFileS32(file, *(int32_t *)&temp);
}

uint32_t GetHash(std::string string)
{
    uint32_t hash = 16777619;
    uint32_t i;
    for (i = 0; i<string.length(); i++) {
        hash ^= string.at(i);
        hash *= 2166136261U;
    }
    return hash;
}

void WriteSpriteAnimations(FILE *file)
{
    uint32_t frame_ofs = AlignNumber(sizeof(SpriteAnim), 4) * xml_sprite.animations.size();
    for (uint16_t i = 0; i < xml_sprite.animations.size(); i++) {
        SpriteAnim anim;
        anim.name_hash = GetHash(xml_sprite.animations[i].id);
        anim.num_frames = xml_sprite.animations[i].frames.size();
        anim.frame_ofs = frame_ofs;
        WriteFileU32(file, anim.name_hash);
        WriteFileU32(file, anim.num_frames);
        WriteFileU32(file, anim.frame_ofs);
        AlignFile(file, 4);
        frame_ofs += xml_sprite.animations[i].frames.size() * AlignNumber(sizeof(SpriteAnimFrame), 2);
    }
    for (uint16_t i = 0; i < xml_sprite.animations.size(); i++) {
        for (uint16_t j = 0; j < xml_sprite.animations[i].frames.size(); j++) {
            SpriteAnimFrame frame;
            frame.image = xml_sprite.animations[i].frames[j].image_idx;
            frame.delay = xml_sprite.animations[i].frames[j].delay;
            WriteFileU16(file, frame.image);
            WriteFileU16(file, frame.delay);
            AlignFile(file, 2);
        }
    }
    AlignFile(file, 8);
}

bool IsImageSemitransparent(uint8_t *data_rgba32, int w, int h)
{
    for (int i = 0; i < w * h; i++) {
        if (data_rgba32[(i * 4) + 3] != 0 && data_rgba32[(i * 4) + 3] != 255) {
            return true;
        }
    }
    return false;
}

bool IsImageGrayscale(uint8_t *data_rgba32, int w, int h)
{
    for (int i = 0; i < w * h; i++) {
        if (data_rgba32[i * 4] != data_rgba32[(i * 4) + 1] || data_rgba32[i * 4] != data_rgba32[(i * 4) + 2]) {
            return false;
        }
    }
    return true;
}

int CountImageColors(uint8_t *data_rgba32, int w, int h)
{
    std::vector<uint32_t> colors;
    colors.reserve(w * h);
    for (int i = 0; i < w * h; i++) {
        colors.push_back((data_rgba32[i * 4]) | (data_rgba32[(i * 4) + 1] << 8) | (data_rgba32[(i * 4) + 2] << 16) | (data_rgba32[(i * 4) + 3] << 24));
    }
    return std::unordered_set<uint32_t>(colors.begin(), colors.end()).size();
}

uint32_t GetBestImageFormat(uint8_t *data_rgba32, int w, int h)
{
    if (IsImageGrayscale(data_rgba32, w, h)) {
        int num_colors = CountImageColors(data_rgba32, w, h);
        if (num_colors <= 16) {
            if ((w % 2) != 0) {
                return IMAGE_FORMAT_CI8;
            } else {
                return IMAGE_FORMAT_CI4;
            }
        } else if (num_colors <= 256) {
            return IMAGE_FORMAT_CI8;
        } else {
            return IMAGE_FORMAT_IA16;
        }
    } else {
        if (IsImageSemitransparent(data_rgba32, w, h)) {
            return IMAGE_FORMAT_RGBA32;
        } else {
            int num_colors = CountImageColors(data_rgba32, w, h);
            if (num_colors <= 16) {
                if ((w % 2) != 0) {
                    return IMAGE_FORMAT_CI8;
                } else {
                    return IMAGE_FORMAT_CI4;
                }
            } else if (num_colors <= 256) {
                return IMAGE_FORMAT_CI8;
            } else {
                return IMAGE_FORMAT_RGBA16;
            }
        }
    }
}

uint32_t GetBestPaletteFormat(uint8_t *data_rgba32, int w, int h)
{
    if (IsImageGrayscale(data_rgba32, w, h)) {
        return PALETTE_FORMAT_IA16;
    } else {
        return PALETTE_FORMAT_RGBA16;
    }
}

uint32_t GetImageDataSize(int w, int h, int format)
{
    return AlignNumber(((w * h * bpp_table[format]) / 8), 8);
}

void WriteImageI4(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    uint8_t dst_value = 0;
    int i;
    for (i = 0; i < (w * h); i++) {
        float temp_r = data_rgba32[i * 4] * 0.3f;
        float temp_g = data_rgba32[(i * 4) + 1] * 0.59f;
        float temp_b = data_rgba32[(i * 4) + 2] * 0.11f;
        uint8_t new_value = color_8bit_to_4bit[(uint8_t)(((temp_r + temp_g + temp_b) * data_rgba32[(i * 4) + 3]) / 255)];
        if (i % 2) {
            dst_value |= new_value;
            fwrite(&dst_value, 1, 1, file);
            dst_value = 0;
        } else {
            dst_value |= (new_value << 4);
        }
    }
    if (i % 2) {
        fwrite(&dst_value, 1, 1, file);
    }
    AlignFile(file, 8);
}

void WriteImageI8(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    int i;
    for (i = 0; i < (w * h); i++) {
        float temp_r = data_rgba32[i * 4] * 0.3f;
        float temp_g = data_rgba32[(i * 4) + 1] * 0.59f;
        float temp_b = data_rgba32[(i * 4) + 2] * 0.11f;
        uint8_t new_value = (uint8_t)(((temp_r + temp_g + temp_b) * data_rgba32[(i * 4) + 3]) / 255);
        fwrite(&new_value, 1, 1, file);
    }
    AlignFile(file, 8);
}

void WriteImageIA4(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    uint8_t dst_value = 0;
    int i;
    for (i = 0; i < (w * h); i++) {
        float temp_r = data_rgba32[i * 4] * 0.3f;
        float temp_g = data_rgba32[(i * 4) + 1] * 0.59f;
        float temp_b = data_rgba32[(i * 4) + 2] * 0.11f;
        uint8_t new_value = color_8bit_to_3bit[(uint8_t)(temp_r + temp_g + temp_b)] << 1;
        if (data_rgba32[(i * 4) + 3] > 192) {
            new_value |= 1;
        }
        if (i % 2) {
            dst_value |= new_value;
            fwrite(&dst_value, 1, 1, file);
            dst_value = 0;
        }
        else {
            dst_value |= (new_value << 4);
        }
    }
    if (i % 2) {
        fwrite(&dst_value, 1, 1, file);
    }
    AlignFile(file, 8);
}

void WriteImageIA8(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    int i;
    for (i = 0; i < (w * h); i++) {
        float temp_r = data_rgba32[i * 4] * 0.3f;
        float temp_g = data_rgba32[(i * 4) + 1] * 0.59f;
        float temp_b = data_rgba32[(i * 4) + 2] * 0.11f;
        uint8_t new_value = color_8bit_to_4bit[(uint8_t)(temp_r + temp_g + temp_b)] << 4;
        new_value |= color_8bit_to_4bit[data_rgba32[(i * 4) + 3]];
        fwrite(&new_value, 1, 1, file);
    }
    AlignFile(file, 8);
}

void WriteColorIA16(FILE *file, uint8_t *color_rgba32)
{
    float temp_r = color_rgba32[0] * 0.3f;
    float temp_g = color_rgba32[1] * 0.59f;
    float temp_b = color_rgba32[2] * 0.11f;
    uint8_t intensity = (uint8_t)(temp_r + temp_g + temp_b);
    fwrite(&intensity, 1, 1, file);
    fwrite(&color_rgba32[3], 1, 1, file);
}

void WriteImageIA16(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    uint8_t dst_value = 0;
    int i;
    for (i = 0; i < (w * h); i++) {
        WriteColorIA16(file, &data_rgba32[i * 4]);
    }
    AlignFile(file, 8);
}

void WriteColorRGBA16(FILE *file, uint8_t *color_rgba32)
{
    uint8_t r = color_8bit_to_5bit[color_rgba32[0]];
    uint8_t g = color_8bit_to_5bit[color_rgba32[1]];
    uint8_t b = color_8bit_to_5bit[color_rgba32[2]];
    uint16_t value = (r << 11) | (g << 6) | (b << 1);
    if (color_rgba32[3] > 192) {
        value |= 1;
    }
    WriteFileU16(file, value);
}

void WritePalette(FILE *file, uint8_t *pal_buf, int num_colors, int pal_format)
{
    switch (pal_format) {
        case PALETTE_FORMAT_RGBA16:
            for (int i = 0; i < num_colors; i++) {
                WriteColorRGBA16(file, &pal_buf[i * 4]);
            }
            break;

        case PALETTE_FORMAT_IA16:
            for (int i = 0; i < num_colors; i++) {
                WriteColorIA16(file, &pal_buf[i * 4]);
            }
            break;

        default:
            break;
    }
}

void WriteImageCI(FILE *file, uint8_t *data_rgba32, int w, int h, bool is_8bpp, int pal_format)
{
    int num_colors = 16;
    if (is_8bpp) {
        num_colors = 256;
    }
    size_t data_len = w * h;
    uint8_t *data_buf = new uint8_t[data_len]();
    uint8_t *pal_buf = new uint8_t[num_colors * 4]();
    exq_data *exq_data = exq_init();
    exq_feed(exq_data, data_rgba32, data_len);
    exq_quantize_hq(exq_data, num_colors);
    exq_get_palette(exq_data, pal_buf, num_colors);
    exq_map_image_ordered(exq_data, w, h, data_rgba32, data_buf);
    exq_free(exq_data);
    if (is_8bpp) {
        int i;
        fwrite(data_buf, 1, data_len, file);
    } else {
        uint8_t dst_value = 0;
        int i;
        for (i = 0; i < (w*h); i++) {
            uint8_t new_value = data_buf[i] & 0xF;
            if (i % 2) {
                dst_value |= new_value;
                fwrite(&dst_value, 1, 1, file);
                dst_value = 0;
            }
            else {
                dst_value |= (new_value << 4);
            }
        }
        if (i % 2) {
            fwrite(&dst_value, 1, 1, file);
        }
    }
    AlignFile(file, 8);
    WritePalette(file, pal_buf, num_colors, pal_format);
    delete[] pal_buf;
    delete[] data_buf;
}

void WriteImageCI4(FILE *file, uint8_t *data_rgba32, int w, int h, int pal_format)
{
    WriteImageCI(file, data_rgba32, w, h, false, pal_format);
}

void WriteImageCI8(FILE *file, uint8_t *data_rgba32, int w, int h, int pal_format)
{
    WriteImageCI(file, data_rgba32, w, h, true, pal_format);
}

void WriteImageRGBA16(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    for (int i = 0; i < (w * h); i++) {
        WriteColorRGBA16(file, &data_rgba32[i * 4]);
    }
    AlignFile(file, 8);
}

void WriteImageRGBA32(FILE *file, uint8_t *data_rgba32, int w, int h)
{
    fwrite(data_rgba32, 4, (w * h), file);
    AlignFile(file, 8);
}

void WriteImageData(FILE *file, uint8_t *data_rgba32, int w, int h, int format, int pal_format)
{
    switch (format) {
        case IMAGE_FORMAT_I4:
            WriteImageI4(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_I8:
            WriteImageI8(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_IA4:
            WriteImageIA4(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_IA8:
            WriteImageIA8(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_IA16:
            WriteImageIA16(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_CI4:
            WriteImageCI4(file, data_rgba32, w, h, pal_format);
            break;

        case IMAGE_FORMAT_CI8:
            WriteImageCI8(file, data_rgba32, w, h, pal_format);
            break;

        case IMAGE_FORMAT_RGBA16:
            WriteImageRGBA16(file, data_rgba32, w, h);
            break;

        case IMAGE_FORMAT_RGBA32:
            WriteImageRGBA32(file, data_rgba32, w, h);
            break;

        default:
            break;
    }
}

void WriteSpriteImage(FILE *file)
{
    uint32_t image_ofs = AlignNumber(AlignNumber(sizeof(SpriteImage), 4) * xml_sprite.images.size(), 8);
    for (uint16_t i = 0; i < xml_sprite.images.size(); i++) {
        SpriteImage image;
        image.id_hash = GetHash(xml_sprite.images[i].id);
        image.data_ofs = image_ofs;
        image.spr_ofs = 0;
        image.gfx_ofs = 0;
        image.w = xml_sprite.images[i].width;
        image.h = xml_sprite.images[i].height;
        if (xml_sprite.images[i].format == IMAGE_FORMAT_AUTO) {
            xml_sprite.images[i].format = GetBestImageFormat(xml_sprite.images[i].data_rgba32, image.w, image.h);
        }
        image.format = xml_sprite.images[i].format;
        if (image.format == IMAGE_FORMAT_CI4 || image.format == IMAGE_FORMAT_CI8) {
            uint32_t pal_ofs = image_ofs + GetImageDataSize(xml_sprite.images[i].width, xml_sprite.images[i].height, image.format);
            image.pal_data_ofs = pal_ofs;
            if (xml_sprite.images[i].pal_format == PALETTE_FORMAT_AUTO) {
                xml_sprite.images[i].pal_format = GetBestPaletteFormat(xml_sprite.images[i].data_rgba32, image.w, image.h);
            }
            image.pal_format = xml_sprite.images[i].pal_format;
            image_ofs = pal_ofs + (sizeof(uint16_t) << bpp_table[image.format]);
        } else {
            image.pal_data_ofs = 0;
            image.pal_format = PALETTE_FORMAT_INVALID;
            image_ofs += GetImageDataSize(xml_sprite.images[i].width, xml_sprite.images[i].height, image.format);
        }
        image.origin_x = (image.w * xml_sprite.images[i].origin_x * 4.0f);
        image.origin_y = (image.h * xml_sprite.images[i].origin_y * 4.0f);
        WriteFileU32(file, image.id_hash);
        WriteFileU32(file, image.data_ofs);
        WriteFileU32(file, image.pal_data_ofs);
        WriteFileU32(file, image.spr_ofs);
        WriteFileU32(file, image.gfx_ofs);
        WriteFileS16(file, image.origin_x);
        WriteFileS16(file, image.origin_y);
        WriteFileU16(file, image.format);
        WriteFileU16(file, image.pal_format);
        WriteFileU16(file, image.w);
        WriteFileU16(file, image.h);
        AlignFile(file, 4);
    }
    AlignFile(file, 8);
    for (uint16_t i = 0; i < xml_sprite.images.size(); i++) {
        XMLSpriteImage *image = &xml_sprite.images[i];
        WriteImageData(file, image->data_rgba32, image->width, image->height, image->format, image->pal_format);
    }
}

void WriteSprite(std::string path)
{
    SpriteHeader header;
    FILE *file = fopen(path.c_str(), "wb");
    if (!file) {
        printf("Failed to open %s for writing.", path.c_str());
        exit(1);
    }
    fseek(file, AlignNumber(sizeof(SpriteHeader), 4), SEEK_SET);
    header.magic = 'SP';
    header.ref_count = 0;
    header.num_animations = xml_sprite.animations.size();
    header.num_images = xml_sprite.images.size();
    if (header.num_animations != 0) {
        header.animations_ofs = AlignNumber(sizeof(SpriteHeader), 4);
    } else {
        header.animations_ofs = 0;
    }
    WriteSpriteAnimations(file);
    header.images_ofs = ftell(file);
    WriteSpriteImage(file);
    fseek(file, 0, SEEK_SET);
    WriteFileU16(file, header.magic);
    WriteFileU16(file, header.ref_count);
    WriteFileU16(file, header.num_animations);
    WriteFileU16(file, header.num_images);
    WriteFileU32(file, header.animations_ofs);
    WriteFileU32(file, header.images_ofs);
    fclose(file);
}

int main(int argc, char **argv)
{
    prog_name = argv[0];
    if (argc < 2) {
        PrintUsage();
    }
    LoadSpriteXML(argv[1]);
    if (argc > 2) {
        WriteSprite(argv[2]);
    } else {
        std::string orig_name = argv[1];
        if (orig_name.find_last_of(".") != std::string::npos) {
            std::string name = orig_name.substr(0, orig_name.find_last_of("."));
            WriteSprite(name + ".spr");
        } else {
            WriteSprite(orig_name + ".spr");
        }
    }
    return 0;
}