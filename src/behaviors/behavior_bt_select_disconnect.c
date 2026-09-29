/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bt_select_disconnect

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/ble.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint8_t profile = binding->param1;
    int ret;

    ret = zmk_ble_prof_select(profile);
    if (ret < 0) {
        return ret;
    }

    for (uint8_t i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (i != profile && zmk_ble_profile_is_connected(i)) {
            ret = zmk_ble_prof_disconnect(i);
            if (ret < 0) {
                return ret;
            }
        }
    }

    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bt_select_disconnect_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_bt_select_disconnect_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */