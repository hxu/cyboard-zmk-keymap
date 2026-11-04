#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IS_ENABLED(config) (config)
#define CONFIG_ZMK_RGB_UNDERGLOW_BRT_START 25
#define ZMK_BLE_PROFILE_COUNT 5
#define ZMK_KEYMAP_LAYERS_LEN 3
#define ZMK_KEYMAP_LAYER_ID_INVAL UINT8_MAX
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define BUILD_ASSERT(condition, message) _Static_assert(condition, message)
#define LOG_MODULE_DECLARE(...)
#define LOG_ERR(...)
#define DT_NODELABEL(node) node
#define DEVICE_DT_NAME(node) #node
#define RGB_EFS_CMD 1
#define RGB_COLOR_HSB_CMD 2
#define RGB_ON_CMD 3
#define RGB_COLOR_HSB_VAL(h, s, b) (((h) << 16) | ((s) << 8) | (b))
#define ZMK_EV_EVENT_BUBBLE 0
#define K_NO_WAIT 0
#define K_MSEC(value) (value)
#define ZMK_SUBSCRIPTION(...)
#define ZMK_LISTENER(name, callback) \
    static int (*name##_listener)(const zmk_event_t *) = callback
#define SETTINGS_STATIC_HANDLER_DEFINE(name, path, get, set, commit, export) \
    static int (*settings_commit)(void) = commit

typedef uint8_t zmk_keymap_layer_index_t;
typedef uint8_t zmk_keymap_layer_id_t;
typedef int k_spinlock_key_t;
struct k_spinlock { int unused; };
struct k_work { int unused; };
struct k_work_delayable { void (*handler)(struct k_work *); };
#define K_WORK_DELAYABLE_DEFINE(name, callback) \
    static struct k_work_delayable name = {.handler = callback}

struct zmk_ble_active_profile_changed { int index; };
typedef struct {
    bool is_profile;
    struct zmk_ble_active_profile_changed profile;
} zmk_event_t;

struct zmk_behavior_binding_event { int64_t timestamp; };
struct zmk_behavior_binding {
    const char *behavior_dev;
    uint32_t param1;
    uint32_t param2;
};

static int power_on_calls;
int zmk_rgb_underglow_on(void) {
    power_on_calls++;
    return 0;
}

int64_t now;
uint8_t active_layer;
struct k_work_delayable *pending_work;
uint32_t color;
int effect;

int64_t k_uptime_get(void) { return now; }
k_spinlock_key_t k_spin_lock(struct k_spinlock *lock) { return 0; }
void k_spin_unlock(struct k_spinlock *lock, k_spinlock_key_t key) {}
int zmk_ble_active_profile_index(void) { return 0; }
zmk_keymap_layer_index_t zmk_keymap_highest_layer_active(void) {
    return active_layer;
}
zmk_keymap_layer_id_t zmk_keymap_layer_index_to_id(uint8_t index) {
    return index;
}
const struct zmk_ble_active_profile_changed *
as_zmk_ble_active_profile_changed(const zmk_event_t *event) {
    return event->is_profile ? &event->profile : NULL;
}
int k_work_reschedule(struct k_work_delayable *work, int64_t delay) {
    pending_work = work;
    return 0;
}
int k_work_schedule(struct k_work_delayable *work, int64_t delay) {
    return 0;
}
int zmk_behavior_invoke_binding(struct zmk_behavior_binding *binding,
                                struct zmk_behavior_binding_event event, bool pressed) {
    assert(pressed);
    switch (binding->param1) {
    case RGB_EFS_CMD:
        effect = binding->param2;
        break;
    case RGB_COLOR_HSB_CMD:
        color = binding->param2;
        break;
    case RGB_ON_CMD:
        power_on_calls++;
        break;
    default:
        assert(false);
    }
    return 0;
}

void run_pending_work(void) {
    assert(pending_work);
    struct k_work_delayable *work = pending_work;
    pending_work = NULL;
    work->handler(NULL);
}

#include "../src/layer_colors.c"

int main(void) {
    // Idle saves RGB off. Boot and deep-sleep wake must restore startup lighting.
    assert(settings_commit() == 0);
#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    run_pending_work();
    assert(color == RGB_COLOR_HSB_VAL(0, 100, 25));
#endif
    assert(power_on_calls == 1);

#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    power_on_calls = 0;
    // Layer and battery events have no profile payload. While lighting is off,
    // both must still update the color without issuing a power command.
    zmk_event_t event = {0};
    active_layer = 1;
    layer_colors_listener(&event);
    run_pending_work();
    assert(color == RGB_COLOR_HSB_VAL(240, 100, 25));
    assert(effect == 0);
    assert(power_on_calls == 0);

    // Profile feedback and its delayed return also leave lighting power alone.
    now = 100;
    event = (zmk_event_t){.is_profile = true, .profile = {.index = 1}};
    layer_colors_listener(&event);
    run_pending_work();
    assert(color == RGB_COLOR_HSB_VAL(300, 100, 25));
    assert(power_on_calls == 0);

    active_layer = 2;
    now = 1600;
    layer_color_work.handler(NULL);
    assert(color == RGB_COLOR_HSB_VAL(120, 100, 25));
    assert(power_on_calls == 0);
#endif
    return 0;
}
