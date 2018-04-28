/** @file main.c
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include "main.h"

int main()
{
    Video *video = videoInit("Rainbow Joe", 800, 600, 0);
    if (NULL == video) return EXIT_FAILURE;

    atexit(SDL_Quit);

    Map *map = mapInit("res/maps/forest.tmx");
    if (NULL == map) return EXIT_FAILURE;
    map->worldPosY = video->height - (map->map->height * map->map->tile_height);

    Entity *player = entityInit("player");
    if (NULL == player) return EXIT_FAILURE;
    if (-1 == entityLoadSprite(player, video->renderer, "res/sprites/characters.png"))
        return EXIT_FAILURE;
    player->frameYoffset = 32;
    player->worldPosY = 312;

    int32_t cameraPosX = 0;
    int32_t cameraPosY = 0;

    while (1)
    {
        // Handle keyboard input.
        const uint8_t *keyState;
        SDL_PumpEvents();
        if (SDL_PeepEvents(0, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT) > 0)
            goto quit;

        player->flags &= ~(1 << IN_MOTION);

        keyState = SDL_GetKeyboardState(NULL);

        if (keyState[SDL_SCANCODE_Q]) goto quit;
        if (keyState[SDL_SCANCODE_A])
        {
            player->flags |= 1 << IN_MOTION;
            player->flags |= 1 << DIRECTION;
            player->worldPosX--;
        }
        if (keyState[SDL_SCANCODE_D])
        {
            player->flags |= 1   << IN_MOTION;
            player->flags &= ~(1 << DIRECTION);
            player->worldPosX++;
        }
        if (keyState[SDL_SCANCODE_I]) cameraPosY--;
        if (keyState[SDL_SCANCODE_K]) cameraPosY++;
        if (keyState[SDL_SCANCODE_J]) cameraPosX--;
        if (keyState[SDL_SCANCODE_L]) cameraPosX++;

        if (-1 == mapRender(video->renderer, map, "Background", 1, cameraPosX, cameraPosY))
            return EXIT_FAILURE;

        if (-1 == entityRender(video->renderer, player, player->worldPosX - cameraPosX, player->worldPosY - cameraPosY))
            return EXIT_FAILURE;

        if (-1 == mapRender(video->renderer, map, "Level", 0, map->worldPosX - cameraPosX, map->worldPosY - cameraPosY))
            return EXIT_FAILURE;

        SDL_RenderPresent(video->renderer);
        SDL_RenderClear(video->renderer);
    }

    quit:
    entityFree(player);
    mapFree(map);
    videoTerminate(video);
    return EXIT_SUCCESS;
}
