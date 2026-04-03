import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_CONFIG,
    UNIT_PERCENT,
)

from .. import CONF_BS_POOL_ID, BSPoolComponent, bs_pool_ns

BSPoolNumberComponent = bs_pool_ns.class_("BSPoolNumberComponent", cg.Component)

WritableNumber = bs_pool_ns.class_("WritableNumber", number.Number)

CONF_POWER = "power"
CONF_POOL_VOLUME = "pool_volume"
CONF_CLEANING_CYCLE = "cleaning_cycle"
CONF_PH_TARGET = "ph_target"
CONF_ORP_TARGET = "orp_target"
CONF_RELAY_DELAY = "relay_delay"
CONF_PROGRAM_1_START = "program_1_start"
CONF_PROGRAM_1_STOP = "program_1_stop"
CONF_PROGRAM_2_START = "program_2_start"
CONF_PROGRAM_2_STOP = "program_2_stop"

TYPES = [
    CONF_POWER,
    CONF_POOL_VOLUME,
    CONF_CLEANING_CYCLE,
    CONF_PH_TARGET,
    CONF_ORP_TARGET,
    CONF_RELAY_DELAY,
    CONF_PROGRAM_1_START,
    CONF_PROGRAM_1_STOP,
    CONF_PROGRAM_2_START,
    CONF_PROGRAM_2_STOP,
]

# function_code, min, max, step, is_16bit, scale, unit, icon
NUMBER_CONFIG = {
    CONF_POWER: (ord('T'), 0, 100, 1, False, 1.0, UNIT_PERCENT, "mdi:flash"),
    CONF_POOL_VOLUME: (ord('v'), 0, 65535, 1, True, 1.0, "m\u00b3", "mdi:pool"),
    CONF_CLEANING_CYCLE: (ord('b'), 0, 2550, 10, False, 0.1, "min", "mdi:broom"),
    CONF_PH_TARGET: (ord('P'), 0.0, 14.0, 0.01, True, 100.0, "pH", "mdi:ph"),
    CONF_ORP_TARGET: (ord('O'), 0, 2047, 1, True, 1.0, "mV", "mdi:water-check"),
    CONF_RELAY_DELAY: (ord('D'), 0, 255, 1, False, 1.0, "min", "mdi:timer-outline"),
    CONF_PROGRAM_1_START: (201, 0, 1439, 1, False, 1.0, "min", "mdi:clock-start"),
    CONF_PROGRAM_1_STOP: (202, 0, 1439, 1, False, 1.0, "min", "mdi:clock-end"),
    CONF_PROGRAM_2_START: (203, 0, 1439, 1, False, 1.0, "min", "mdi:clock-start"),
    CONF_PROGRAM_2_STOP: (204, 0, 1439, 1, False, 1.0, "min", "mdi:clock-end"),
}


def _number_schema(key):
    code, min_val, max_val, step, is_16bit, scale, unit, icon = NUMBER_CONFIG[key]
    return number.number_schema(
        WritableNumber,
        entity_category=ENTITY_CATEGORY_CONFIG,
        unit_of_measurement=unit,
        icon=icon,
    )


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BSPoolNumberComponent),
            cv.GenerateID(CONF_BS_POOL_ID): cv.use_id(BSPoolComponent),
            **{cv.Optional(key): _number_schema(key) for key in TYPES},
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    bspool_number = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(bspool_number, config)

    await cg.register_parented(bspool_number, config[CONF_BS_POOL_ID])

    bspool = await cg.get_variable(config[CONF_BS_POOL_ID])
    cg.add(bspool.register_listener(bspool_number))

    for key in TYPES:
        if key in config:
            code, min_val, max_val, step, is_16bit, scale, unit, icon = NUMBER_CONFIG[key]
            n = await number.new_number(
                config[key], min_value=min_val, max_value=max_val, step=step
            )
            await cg.register_parented(n, config[CONF_ID])
            cg.add(n.set_function_code(code))
            cg.add(n.set_is_16bit(is_16bit))
            cg.add(n.set_scale(scale))
            cg.add(n.set_is_program_time(key in [
                CONF_PROGRAM_1_START, CONF_PROGRAM_1_STOP,
                CONF_PROGRAM_2_START, CONF_PROGRAM_2_STOP,
            ]))
            cg.add(getattr(bspool_number, f"set_{key}_number")(n))
