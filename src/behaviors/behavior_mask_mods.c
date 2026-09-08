/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mask_mods

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/hid.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_mask_mods_config {
    struct zmk_behavior_binding binding;
    zmk_mod_flags_t mods;
};

struct behavior_mask_mods_data {
    bool active;
};

static int on_mask_mods_binding_pressed(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event) {

    const struct device *dev =
        zmk_behavior_get_binding(binding->behavior_dev);

    const struct behavior_mask_mods_config *cfg = dev->config;
    struct behavior_mask_mods_data *data = dev->data;

    if (data->active) {
        LOG_ERR("mask-mods already active");
        return -ENOTSUP;
    }

    /*
     * Ctrl / Shift をHIDレポートからマスクする。
     *
     * 物理キーそのものは離さない。
     */
    zmk_hid_masked_modifiers_set(cfg->mods);

    data->active = true;

    /*
     * 指定されたbehaviorを実行する。
     */
    return zmk_behavior_invoke_binding(
        &cfg->binding,
        event,
        true
    );
}

static int on_mask_mods_binding_released(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event) {

    const struct device *dev =
        zmk_behavior_get_binding(binding->behavior_dev);

    struct behavior_mask_mods_data *data = dev->data;

    if (!data->active) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    data->active = false;

    /*
     * Aを離した時点でmodifier maskを解除。
     */
    zmk_hid_masked_modifiers_clear();

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api
    behavior_mask_mods_driver_api = {
        .binding_pressed =
            on_mask_mods_binding_pressed,

        .binding_released =
            on_mask_mods_binding_released,
};

#define MASK_MODS_INST(n)                                             \
    static struct behavior_mask_mods_data                             \
        behavior_mask_mods_data_##n = {};                             \
                                                                       \
    static const struct behavior_mask_mods_config                     \
        behavior_mask_mods_config_##n = {                              \
          /* nをDT_DRV_INST(n)でラップする */                        \
          .binding = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),  \
          .mods = DT_INST_PROP(n, mods),                            \
        };                                                             \
                                                                       \
    BEHAVIOR_DT_INST_DEFINE(                                          \
        n,                                                             \
        NULL,                                                          \
        NULL,                                                          \
        &behavior_mask_mods_data_##n,                                  \
        &behavior_mask_mods_config_##n,                                \
        POST_KERNEL,                                                   \
        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                           \
        &behavior_mask_mods_driver_api                                 \
    );

DT_INST_FOREACH_STATUS_OKAY(MASK_MODS_INST)

#endif