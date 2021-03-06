/** @file entity.c
 * @ingroup   Entity
 * @defgroup  Entity
 * @brief     Handler to take care of game entities such as the player, enemies,
 *            etc..
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "entity.h"

/**
 * @brief   Update entity.  Thie function has to be called every frame.
 * @param   entity the entity to update.  See @ref struct Entity.
 * @param   dTime  delta time; time passed since last frame in seconds.
 * @ingroup Entity
 */
void entityFrame(Entity *entity, double dTime)
{
    // Update bounding box.
    entity->bb.b = entity->worldPosY + entity->height;
    entity->bb.l = entity->worldPosX;
    entity->bb.r = entity->worldPosX + entity->width;
    entity->bb.t = entity->worldPosY;

    // Increase/decrease vertical velocity if player is in motion.
    if ((entity->flags >> IN_MOTION)  & 1)
    {
        entity->velocity += entity->acceleration * dTime;
    }
    else
    {
        entity->velocity -= entity->deceleration * dTime;
    }

    // Set vertical velocity limits.
    if (entity->velocity > entity->velocityMax)
    {
        entity->velocity = entity->velocityMax;
    }
    if (entity->velocity < 0)
    {
        entity->velocity = 0;
        // Reset frame animation when standing still.
        entity->frame    = entity->frameStart;
    }

    // Update frame.
    if (((entity->flags >> IN_MOTION)  & 1) ||
        ((entity->flags >> IN_MID_AIR) & 1))
    {
        entity->frameTime += dTime;

        if (entity->frameTime > 1 / entity->fps)
        {
            entity->frame++;
            entity->frameTime = 0;
        }
    }

    // Loop frame animation.
    if (entity->frameEnd <= entity->frame)
    {
        entity->frame = entity->frameStart;
    }

    // Set vertical player position.
    if (entity->velocity > 0)
    {
        if ((entity->flags >> DIRECTION) & 1)
        {
            entity->worldPosX -= (entity->velocity * dTime);
        }
        else
        {
            entity->worldPosX += (entity->velocity * dTime);
        }
    }

    // Set horizontal player position.
    if ((entity->flags >> IS_JUMPING) & 1)
    {
        entity->jumpTime += dTime;
        entity->flags    |= 1 << IN_MID_AIR;
    }

    // Handle falling, jumping, gravity, etc.
    if ((entity->flags >> IN_MID_AIR) & 1)
    {
        double g = (entity->worldMeterInPixel * entity->worldGravitation);
        if ((entity->flags >> IS_JUMPING) & 1)
        {
            entity->frameStart = JUMP;
            entity->frameEnd   = JUMP_MAX;
            g += entity->velocityJump;
            g = -g * entity->jumpGravityFactor;

            if (entity->jumpTime > entity->jumpTimeMax)
            {
                entity->jumpTime = 0;
                entity->flags &= ~(1 << IS_JUMPING);
            }
        }
        else
        {
            entity->frameStart = FALL;
            entity->frameEnd   = FALL_MAX;
        }

        entity->distanceFall  = g * dTime * dTime;
        entity->velocityFall += entity->distanceFall;
        entity->worldPosY    += entity->velocityFall;
    }
    else
    {
        entity->frameStart    = WALK;
        entity->frameEnd      = WALK_MAX;
        entity->flags        &= ~(1 << IS_JUMPING);
        entity->velocityFall  = 0;
    }

    // Connect left and right border of the map and vice versa.
    if (entity->worldPosX < 0 - (entity->width / 2))
    {
        entity->worldPosX = entity->worldWidth - (entity->width / 2);
    }

    if (entity->worldPosX > entity->worldWidth - (entity->width / 2))
    {
        entity->worldPosX = 0 - (entity->width / 2);
    }

    // Kill player when he falls out of the map.
    if (entity->worldPosY >= entity->worldHeight + entity->height)
    {
        entity->flags |= 1 << IS_DEAD;
    }

    if (entity->worldPosY > entity->worldHeight + entity->height)
    {
        entity->worldPosY = entity->worldHeight + entity->height;
    }
}

/**
 * @brief   Initialise entity.
 * @return  Entity on success, NULL on error.  See @ref struct Entity.
 * @ingroup Entity
 */
Entity *entityInit()
{
    static Entity *entity;
    entity = malloc(sizeof(struct entity_t));
    if (NULL == entity)
    {
        fprintf(stderr, "entityInit(): error allocating memory.\n");
        return NULL;
    }

    // Default values.
    entity->height            =  32;
    entity->width             =  32;
    entity->bb.b              =   0;
    entity->bb.l              =   entity->height;
    entity->bb.r              =   entity->width;
    entity->bb.t              =   0;
    entity->acceleration      = 400;
    entity->deceleration      = 200;
    entity->distanceFall      =   0;
    entity->flags             =   0;
    entity->fps               =  12;
    entity->frame             =   0;
    entity->frameEnd          =   WALK_MAX;
    entity->frameStart        =   WALK;
    entity->frameTime         =   0.0;
    entity->frameYoffset      =  32;
    entity->jumpGravityFactor =   4.0;
    entity->jumpTime          =   0.0;
    entity->jumpTimeMax       =   0.12;
    entity->respawnPosX       =   0.0;
    entity->respawnPosY       =   0.0;
    entity->sprite            =   NULL;
    entity->velocity          =   0.0;
    entity->velocityFall      =   0.0;
    entity->velocityJump      =   0.0;
    entity->velocityMax       = 100.0;
    entity->worldHeight       =   0;
    entity->worldGravitation  =   9.81;
    entity->worldMeterInPixel =  32;
    entity->worldPosX         =   0.0;
    entity->worldPosY         =   0.0;
    entity->worldWidth        =   0;

    return entity;
}

/**
 * @brief
 * @param   entity
 * @param   renderer
 * @param   filename
 * @return  0 on success, -1 on error.
 * @ingroup Entity
 */
int8_t entityLoadSprite(Entity *entity, SDL_Renderer *renderer, const char *filename)
{
    if (NULL != entity->sprite)
    {
        SDL_DestroyTexture(entity->sprite);
    }

    entity->sprite = IMG_LoadTexture(renderer, filename);
    if (NULL == entity->sprite)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

/**
 * @brief   Render entity on screen.
 * @param   renderer   SDL's rendering context.  See @ref struct Video.
 * @param   entity     the entity to render.  See @ref struct Entity.
 * @param   cameraPosX camera position along the x-axis.
 * @param   cameraPosY camera position along the y-axis.
 * @return  0 on success, -1 on error.
 * @ingroup Entity
 */
int8_t entityRender(SDL_Renderer *renderer, Entity *entity, double cameraPosX, double cameraPosY)
{
    if (NULL == entity->sprite)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return -1;
    }

    double renderPosX = entity->worldPosX - cameraPosX;
    double renderPosY = entity->worldPosY - cameraPosY;

    SDL_Rect dst =
    {
        renderPosX,
        renderPosY,
        entity->width,
        entity->height
    };
    SDL_Rect src =
    {
        entity->frame * entity->width,
        entity->frameYoffset,
        entity->width,
        entity->height
    };

    SDL_RendererFlip flip;

    if ((entity->flags >> DIRECTION) & 1)
    {
        flip = SDL_FLIP_HORIZONTAL;
    }
    else
    {
        flip = SDL_FLIP_NONE;
    }

    if (-1 == SDL_RenderCopyEx(renderer, entity->sprite, &src, &dst, 0, NULL, flip))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

/**
 * @brief   Respawn entity.
 * @param   entity the entity to respawn.  See @ref struct Entity.
 * @ingroup Entity
 */
void entityRespawn(Entity *entity)
{
    entity->flags     &= ~(1 << IS_DEAD);
    entity->flags     &= ~(1 << IN_MOTION);
    entity->worldPosX  = entity->respawnPosX;
    entity->worldPosY  = entity->respawnPosY;
}
