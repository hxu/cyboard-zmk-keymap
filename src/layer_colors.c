#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

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
static struct profile_flash flash;
static struct k_spinlock flash_lock;

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
    int64_t remaining = profile_flash_remaining(&current_flash, k_uptime_get());

    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(index);
    // Studio can remove the default layer, leaving no valid active layer.
    if (!remaining && (layer == ZMK_KEYMAP_LAYER_ID_INVAL || layer >= ARRAY_SIZE(layer_hues))) {
        return;
    }

    struct profile_color color = remaining ? profile_colors[current_flash.profile]
                                           : (struct profile_color){layer_hues[layer], 100};

    struct zmk_behavior_binding_event event = {.timestamp = k_uptime_get()};
    struct zmk_behavior_binding bindings[] = {
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_EFS_CMD,
         .param2 = 0},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_COLOR_HSB_CMD,
         .param2 = RGB_COLOR_HSB_VAL(color.hue, color.saturation,
                                    CONFIG_ZMK_RGB_UNDERGLOW_BRT_START)},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)), .param1 = RGB_ON_CMD},
    };

    for (size_t i = 0; i < ARRAY_SIZE(bindings); i++) {
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

static int layer_color_settings_loaded(void) {
    // Saved RGB settings load in main(), after SYS_INIT. Reapply the active layer
    // here so a saved blue or green color cannot override Qwerty's startup red.
    // Establish the restored profile without flashing a startup indication.
    k_spinlock_key_t key = k_spin_lock(&flash_lock);
    flash = (struct profile_flash){.profile = zmk_ble_active_profile_index()};
    k_spin_unlock(&flash_lock, key);
    k_work_reschedule(&layer_color_work, K_NO_WAIT);
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(layer_colors, "layer_colors", NULL, NULL,
                               layer_color_settings_loaded, NULL);
