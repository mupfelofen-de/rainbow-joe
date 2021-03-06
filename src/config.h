/** @file config.h
 * @ingroup Config
 */

#ifndef CONFIG_h
#define CONFIG_h

#include <stdint.h>

/**
 * @ingroup Config
 */
typedef struct audioConfig_t {
    int8_t enabled;
} AudioConfig;

/**
 * @ingroup Config
 */
typedef struct videoConfig_t {
    int32_t height;
    int32_t width;
    int8_t  fullscreen;
    int8_t  limitFPS;
    int8_t  fps;
} VideoConfig;

/**
 * @ingroup Config
 */
typedef struct cfg_t
{
    AudioConfig audio;
    VideoConfig video;
} Config;

Config configInit(const char *filename);

#endif
