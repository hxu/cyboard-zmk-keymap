#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zmk/rgb_underglow.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <dt-bindings/zmk/rgb.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include "profile_flash.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static const uint16_t layer_hues[] = {0, 240, 120};
#define LAYER_FLASH_MS 600
static struct profile_flash flash;
static struct k_spinlock flash_lock;
static zmk_keymap_layer_id_t previous_layer = ZMK_KEYMAP_LAYER_ID_INVAL;
static int64_t layer_until;

BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT == PROFILE_COLOR_COUNT,
             "Each Bluetooth profile must have an indicator color");

BUILD_ASSERT(ZMK_KEYMAP_LAYERS_LEN == ARRAY_SIZE(layer_hues),
             "Each keymap layer must have an indicator color");

static void update_layer_color(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(layer_color_work, update_layer_color);

static void update_layer_color(struct k_work *work) {
    k_spinlock_key_t key = k_spin_lock(&flash_lock);
    struct profile_flash current_flash = flash;
    k_spin_unlock(&flash_lock, key);
    int64_t now = k_uptime_get();
    int64_t profile_remaining = profile_flash_remaining(&current_flash, now);

    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(index);
    bool valid_layer = layer != ZMK_KEYMAP_LAYER_ID_INVAL && layer < ARRAY_SIZE(layer_hues);
    if (layer != previous_layer) {
        previous_layer = layer;
        // Read the final layer after &to's intermediate events. If a profile
        // flash has priority, show the new layer for a full pulse afterwards.
        layer_until = valid_layer ? now + profile_remaining + LAYER_FLASH_MS : 0;
    }
    int64_t remaining = profile_remaining ? profile_remaining
                                         : (valid_layer && layer_until > now ? layer_until - now : 0);

    struct profile_color color = profile_remaining ? profile_colors[current_flash.profile]
                                : (struct profile_color){valid_layer ? layer_hues[layer] : 0, 100};

    struct zmk_behavior_binding_event event = {.timestamp = k_uptime_get()};
    struct zmk_behavior_binding bindings[] = {
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_EFS_CMD,
         .param2 = 0},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_COLOR_HSB_CMD,
         .param2 = RGB_COLOR_HSB_VAL(color.hue, color.saturation,
                                    CONFIG_ZMK_RGB_UNDERGLOW_BRT_START)},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = remaining ? RGB_ON_CMD : RGB_OFF_CMD},
    };

    // Expiry and battery resynchronization only need the off command.
    for (size_t i = remaining ? 0 : 2; i < ARRAY_SIZE(bindings); i++) {
        // Global behaviors update both halves through ZMK's split transport.
        int err = zmk_behavior_invoke_binding(&bindings[i], event, true);
        if (err < 0) {
            LOG_ERR("Failed to update layer color: %d", err);
            break;
        }
    }

    if (remaining) {
        // schedule(), unlike reschedule(), preserves an immediate update that
        // another event queued while this handler was running.
        k_work_schedule(&layer_color_work, K_MSEC(remaining));
    }
}

// &to can emit multiple layer events; read the final active layer in the work item.
static int layer_color_listener(const zmk_event_t *event) {
    const struct zmk_ble_active_profile_changed *profile =
        as_zmk_ble_active_profile_changed(event);
    if (profile) {
        k_spinlock_key_t key = k_spin_lock(&flash_lock);
        bool changed = profile_flash_change(&flash, profile->index, k_uptime_get());
        k_spin_unlock(&flash_lock, key);
        // The same event is also raised on reconnect and bond changes.
        if (!changed) {
            return ZMK_EV_EVENT_BUBBLE;
        }
    }
    k_work_reschedule(&layer_color_work, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_colors, layer_color_listener);
ZMK_SUBSCRIPTION(layer_colors, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(layer_colors, zmk_peripheral_battery_state_changed);
ZMK_SUBSCRIPTION(layer_colors, zmk_ble_active_profile_changed);
#endif

static int layer_color_settings_loaded(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    // Saved RGB settings load in main(), after SYS_INIT. Establish the restored
    // layer and profile without flashing, then override any saved on state.
    k_spinlock_key_t key = k_spin_lock(&flash_lock);
    flash = (struct profile_flash){.profile = zmk_ble_active_profile_index()};
    k_spin_unlock(&flash_lock, key);
    previous_layer = zmk_keymap_layer_index_to_id(zmk_keymap_highest_layer_active());
    layer_until = 0;
    k_work_reschedule(&layer_color_work, K_NO_WAIT);
#endif
    return zmk_rgb_underglow_off();
}

SETTINGS_STATIC_HANDLER_DEFINE(layer_colors, "layer_colors", NULL, NULL,
                               layer_color_settings_loaded, NULL);
