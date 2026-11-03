#include <assert.h>
#include "profile_flash.h"

int main(void) {
    struct profile_flash flash = {.profile = 3};
    // A restored profile or a reconnect must not flash at boot.
    assert(!profile_flash_change(&flash, 3, 100));
    assert(profile_flash_remaining(&flash, 100) == 0);

    assert(profile_flash_change(&flash, 4, 200));
    assert(profile_flash_remaining(&flash, 200) == 1500);
    assert(profile_flash_remaining(&flash, 1699) == 1);
    assert(profile_flash_remaining(&flash, 1700) == 0);
    assert(profile_flash_remaining(&flash, 1701) == 0);

    // Wraparound and quick presses restart the full timer for the latest profile.
    assert(profile_flash_change(&flash, 0, 1800));
    assert(profile_flash_change(&flash, 1, 1900));
    assert(flash.profile == 1);
    assert(profile_flash_remaining(&flash, 2000) == 1400);
    // Connection events during a flash must not extend it.
    assert(!profile_flash_change(&flash, 1, 2100));
    assert(profile_flash_remaining(&flash, 2100) == 1300);
    assert(!profile_flash_change(&flash, -1, 2200));
    assert(!profile_flash_change(&flash, 5, 2200));
    assert(flash.profile == 1 && flash.until == 3400);

    // Uptime can exceed 32-bit milliseconds on a keyboard left powered on.
    assert(profile_flash_change(&flash, 2, INT64_C(5000000000)));
    assert(profile_flash_remaining(&flash, INT64_C(5000000500)) == 1000);

    const unsigned layer_hues[] = {0, 240, 120};
    for (unsigned i = 0; i < PROFILE_COLOR_COUNT; i++) {
        for (unsigned j = 0; j < 3; j++) {
            assert(profile_colors[i].saturation == 0 || profile_colors[i].hue != layer_hues[j]);
        }
        for (unsigned j = i + 1; j < PROFILE_COLOR_COUNT; j++) {
            assert(profile_colors[i].hue != profile_colors[j].hue ||
                   profile_colors[i].saturation != profile_colors[j].saturation);
        }
    }
    return 0;
}
