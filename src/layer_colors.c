#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <dt-bindings/zmk/rgb.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static const uint16_t layer_hues[] = {0, 240, 120};

BUILD_ASSERT(ZMK_KEYMAP_LAYERS_LEN == ARRAY_SIZE(layer_hues),
             "Each keymap layer must have an indicator color");

static void update_layer_color(struct k_work *work) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(index);
    // Studio can remove the default layer, leaving no valid active layer.
    if (layer == ZMK_KEYMAP_LAYER_ID_INVAL) {
        return;
    }

    struct zmk_behavior_binding_event event = {.timestamp = k_uptime_get()};
    struct zmk_behavior_binding bindings[] = {
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_EFS_CMD,
         .param2 = 0},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)),
         .param1 = RGB_COLOR_HSB_CMD,
         .param2 = RGB_COLOR_HSB_VAL(layer_hues[layer], 100,
                                    CONFIG_ZMK_RGB_UNDERGLOW_BRT_START)},
        {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(rgb_ug)), .param1 = RGB_ON_CMD},
    };

    for (size_t i = 0; i < ARRAY_SIZE(bindings); i++) {
        // Global behaviors update both halves through ZMK's split transport.
        int err = zmk_behavior_invoke_binding(&bindings[i], event, true);
        if (err < 0) {
            LOG_ERR("Failed to update layer color: %d", err);
            return;
        }
    }
}

// &to can emit multiple layer events; read the final active layer in the work item.
K_WORK_DEFINE(layer_color_work, update_layer_color);

static int layer_color_listener(const zmk_event_t *event) {
    k_work_submit(&layer_color_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_colors, layer_color_listener);
ZMK_SUBSCRIPTION(layer_colors, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(layer_colors, zmk_peripheral_battery_state_changed);

static int layer_color_settings_loaded(void) {
    // Saved RGB settings load in main(), after SYS_INIT. Reapply the active layer
    // here so a saved blue or green color cannot override Qwerty's startup red.
    k_work_submit(&layer_color_work);
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(layer_colors, "layer_colors", NULL, NULL,
                               layer_color_settings_loaded, NULL);
