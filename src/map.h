/** @file main.h
 * @ingroup Map
 */

#ifndef MAP_h
#define MAP_h

#define MAX_TEXTURES_PER_MAP 5

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <tmx.h>

/**
 * @ingroup Map
 */
typedef struct map_t
{
    tmx_map     *map;
    SDL_Texture *texture[MAX_TEXTURES_PER_MAP];
    double      worldPosX;
    double      worldPosY;
} Map;

uint8_t  mapGIDisType(Map *map, const char *type, uint16_t gid);
void     mapFree(Map *map);
uint16_t mapGetGID(Map *map, double posX, double posY);
Map      *mapInit(const char *filename);
int8_t   mapRender(SDL_Renderer *renderer, Map *map, const char *name, uint8_t bg, uint8_t index, double posX, double posY);

#endif
