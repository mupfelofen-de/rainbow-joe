/** @file main.c
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include "main.h"

int main()
{
    int execStatus  = EXIT_SUCCESS;

    Config config  = configInit("default.ini");
    Video  *video  = NULL;
    Map    *map    = NULL;
    Entity *player = NULL;
    Mixer  *mixer  = NULL;
    Music  *music  = NULL;
    Music  *dead   = NULL;

    video = videoInit(
        "Rainbow Joe",
        config.video.width,
        config.video.height,
        config.video.fullscreen,
        2
    );
    if (NULL == video)
    {
        execStatus = EXIT_FAILURE;
        goto quit;
    }
    atexit(SDL_Quit);

    map = mapInit("res/maps/01.tmx");
    if (NULL == map)
    {
        execStatus = EXIT_FAILURE;
        goto quit;
    }

    player = entityInit("player");
    if (NULL == player)
    {
        execStatus = EXIT_FAILURE;
        goto quit;
    }
    if (-1 == entityLoadSprite(player, video->renderer, "res/sprites/characters.png"))
    {
        execStatus = EXIT_FAILURE;
        goto quit;
    }
    player->frameYoffset =  32;
    player->worldPosX    =  32;
    player->worldPosY    = 608;
    player->worldWidth   = map->map->width  * map->map->tile_width;
    player->worldHeight  = map->map->height * map->map->tile_height;

    /* Note: The error handling isn't missing here.  There is simply no need to
     * quit the program if the music can't be played by some reason. */
    mixer = mixerInit();
    music = musicInit("res/music/enchanted-tiki-86.ogg");
    dead  = musicInit("res/sfx/05.ogg");
    if (config.audio.enabled)
        if (mixer) musicFadeIn(music, -1, 5000);

    double   cameraPosX = 0;
    double   cameraPosY = map->map->height * map->map->tile_height - video->windowHeight;
    double   timeA      = SDL_GetTicks();

    while (1)
    {
        double timeB  = SDL_GetTicks();
        double dTime  = (timeB - timeA) / 1000;
        timeA         = timeB;

        entityFrame(player, dTime);

        if ((player->flags >> IS_DEAD) & 1)
        {
            player->flags &= ~(1 << IS_DEAD);
            player->worldPosX =  32;
            player->worldPosY = 608;
            if (config.audio.enabled)
                if (mixer) musicPlay(dead, 1);
            SDL_Delay(2000);
            if (config.audio.enabled)
                if (mixer) musicFadeIn(music, -1, 5000);
        }

        // Handle keyboard input.
        const uint8_t *keyState;
        SDL_PumpEvents();
        if (SDL_PeepEvents(0, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT) > 0)
            goto quit;
        keyState = SDL_GetKeyboardState(NULL);
        // Reset IN_MOTION flag (in case no key is pressed).
        player->flags &= ~(1 << IN_MOTION);

        if (keyState[SDL_SCANCODE_LSHIFT])
        {
            // Allow running only when not in mid-air.
            if (0 == ((player->flags >> IN_MID_AIR) & 1))
            {
                player->velocityMax  = 250;
                player->frameStart   = RUN;
                player->frameEnd     = RUN_MAX;
            }
        }
        else
        {
            // Don't allow to slow down in mid-air.
            if (0 == ((player->flags >> IN_MID_AIR) & 1))
            {
                player->velocityMax  = 100;
                player->frameStart   = WALK;
                player->frameEnd     = WALK_MAX;
            }
        }
        if (keyState[SDL_SCANCODE_Q]) goto quit;
        if (keyState[SDL_SCANCODE_A])
        {
            if (0 == ((player->flags >> DIRECTION) & 1))
                player->velocity = -player->velocity;
            player->flags |= 1 << IN_MOTION;
            player->flags |= 1 << DIRECTION;
        }
        if (keyState[SDL_SCANCODE_D])
        {
            if ((player->flags >> DIRECTION) & 1)
                player->velocity = -player->velocity;

            player->flags |= 1   << IN_MOTION;
            player->flags &= ~(1 << DIRECTION);
        }

        if (0 == ((player->flags >> IN_MID_AIR) & 1))
            if (keyState[SDL_SCANCODE_SPACE])
            {
                player->flags        |= 1 << IS_JUMPING;
                player->velocityJump  = player->velocity;
            }

        if (keyState[SDL_SCANCODE_1])
            videoSetZoomLevel(video, video->zoomLevelInital);
        if (keyState[SDL_SCANCODE_2])
            videoSetZoomLevel(video, video->zoomLevel - dTime);
        if (keyState[SDL_SCANCODE_3])
            videoSetZoomLevel(video, video->zoomLevel + dTime);

        if (keyState[SDL_SCANCODE_F])
        {
            if (keyState[SDL_SCANCODE_UP])    cameraPosY -= (250 * dTime);
            if (keyState[SDL_SCANCODE_DOWN])  cameraPosY += (250 * dTime);
            if (keyState[SDL_SCANCODE_LEFT])  cameraPosX -= (250 * dTime);
            if (keyState[SDL_SCANCODE_RIGHT]) cameraPosX += (250 * dTime);
        }
        else
        {
            cameraPosX = player->worldPosX - video->windowWidth  / (video->zoomLevel * 2) + (player->width  / 2);
            cameraPosY = player->worldPosY - video->windowHeight / (video->zoomLevel * 2) + (player->height / 2);
        }

        // Set up collision detection.
        if (mapCoordIsType(map, "floor", player->worldPosX, player->worldPosY + player->height))
            player->flags &= ~(1 << IN_MID_AIR);
        else
            player->flags |= 1 << IN_MID_AIR;

        // Set camera boundaries to map size.
        int32_t cameraMaxX = (map->map->width  * map->map->tile_width)  - (video->windowWidth  / video->zoomLevel);
        int32_t cameraMaxY = (map->map->height * map->map->tile_height) - (video->windowHeight / video->zoomLevel);
        if (cameraPosX < 0) cameraPosX = 0;
        if (cameraPosY < 0) cameraPosY = 0;
        if (cameraPosX > cameraMaxX) cameraPosX = cameraMaxX;
        if (cameraPosY > cameraMaxY) cameraPosY = cameraMaxY;

        // Render scene.
        if (-1 == mapRender(video->renderer, map, "Background", 1, 0, map->worldPosX - cameraPosX, map->worldPosY - cameraPosY))
        {
            execStatus = EXIT_FAILURE;
            goto quit;
        }

        if (-1 == mapRender(video->renderer, map, "World", 1, 1, map->worldPosX - cameraPosX, map->worldPosY - cameraPosY))
        {
            execStatus = EXIT_FAILURE;
            goto quit;
        }

        if (-1 == entityRender(video->renderer, player, player->worldPosX - cameraPosX, player->worldPosY - cameraPosY))
        {
            execStatus = EXIT_FAILURE;
            goto quit;
        }

        if (-1 == mapRender(video->renderer, map, "Overlay", 1, 2, map->worldPosX - cameraPosX, map->worldPosY - cameraPosY))
        {
            execStatus = EXIT_FAILURE;
            goto quit;
        }

        SDL_RenderPresent(video->renderer);
        SDL_RenderClear(video->renderer);
    }

    quit:
    musicFree(dead);
    musicFree(music);
    mixerFree(mixer);
    entityFree(player);
    mapFree(map);
    videoTerminate(video);
    //configFree(config);
    return execStatus;
}
