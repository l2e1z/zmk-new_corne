/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mask_mods

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_mask_mods_config {
    struct zmk_behavior_binding binding;
    zmk_mod_flags_t mods;
};

struct behavior_mask_mods_data {
    bool pressed;
};

static int on_mask_mods_binding_pressed(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_mask_mods_config *cfg = dev->config;
    struct behavior_mask_mods_data *data = dev->data;

    if (data->pressed) {
        return -ENOTSUP;
    }

    data->pressed = true;

    zmk_hid_masked_modifiers_set(cfg->mods);

    return zmk_behavior_invoke_binding(&cfg->binding, event, true);
}

static int on_mask_mods_binding_released(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_mask_mods_config *cfg = dev->config;
    struct behavior_mask_mods_data *data = dev->data;

    if (!data->pressed) {
        return -ENOTSUP;
    }

    data->pressed = false;

    int err = zmk_behavior_invoke_binding(&cfg->binding, event, false);

    zmk_hid_masked_modifiers_clear();

    return err;
}

static const struct behavior_driver_api behavior_mask_mods_driver_api = {
    .binding_pressed = on_mask_mods_binding_pressed,
    .binding_released = on_mask_mods_binding_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
};

#define MASK_MODS_INST(n)                                                               \
    static struct behavior_mask_mods_config behavior_mask_mods_config_##n = {           \
        .binding = {                                                                    \
            .behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, 0)),     \
            .param1 = COND_CODE_0(                                                       \
                DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, 0, param1),                    \
                (0),                                                                    \
                (DT_INST_PHA_BY_IDX(n, bindings, 0, param1))),                         \
            .param2 = COND_CODE_0(                                                       \
                DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, 0, param2),                    \
                (0),                                                                    \
                (DT_INST_PHA_BY_IDX(n, bindings, 0, param2))),                         \
        },                                                                              \
        .mods = DT_INST_PROP(n, mods),                                                   \
    };                                                                                   \
    static struct behavior_mask_mods_data behavior_mask_mods_data_##n = {};             \
    BEHAVIOR_DT_INST_DEFINE(                                                            \
        n, NULL, NULL,                                                                  \
        &behavior_mask_mods_data_##n,                                                    \
        &behavior_mask_mods_config_##n,                                                  \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                               \
        &behavior_mask_mods_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MASK_MODS_INST)

#endif