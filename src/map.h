/** @file main.h
 * @ingroup Map
 */

#ifndef MAP_h
#define MAP_h

#include <SDL2/SDL.h>
#include <stdint.h>
#include "tmx/tmx.h"

/**
 * @def     MAX_TEXTURES_PER_MAP
 *          The maximum number of textures (layers) per map.
 * @ingroup Map
 */
#define MAX_TEXTURES_PER_MAP 5

/**
 * @ingroup Map
 */
typedef struct map_t
{
    tmx_map     *map;
    SDL_Texture *texture[MAX_TEXTURES_PER_MAP];
    uint32_t    height;
    uint32_t    width;
    double      worldPosX;
    double      worldPosY;
} Map;

uint8_t mapCoordIsType(Map *map, const char *type, double xPos, double yPos);
void    mapFree(Map *map);
Map     *mapInit(const char *filename);
int8_t  mapRender(SDL_Renderer *renderer, Map *map, const char *name, uint8_t bg, uint8_t index, double cameraPosX, double cameraPosY);

#endif
