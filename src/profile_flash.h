#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PROFILE_FLASH_MS 1500
#define PROFILE_COLOR_COUNT 5

struct profile_color {
    uint16_t hue;
    uint8_t saturation;
};

// Cyan, magenta, yellow, orange, white; separate from the layer RGB colors.
static const struct profile_color profile_colors[PROFILE_COLOR_COUNT] = {
    {180, 100}, {300, 100}, {60, 100}, {30, 100}, {0, 0},
};

struct profile_flash {
    int profile;
    int64_t until;
};

static inline bool profile_flash_change(struct profile_flash *flash, int profile, int64_t now) {
    if (profile < 0 || profile >= PROFILE_COLOR_COUNT || profile == flash->profile) {
        return false;
    }
    flash->profile = profile;
    flash->until = now + PROFILE_FLASH_MS;
    return true;
}

static inline int64_t profile_flash_remaining(const struct profile_flash *flash, int64_t now) {
    return flash->until > now ? flash->until - now : 0;
}
