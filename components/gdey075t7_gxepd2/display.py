import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import CONF_ID
from esphome.core import CORE

AUTO_LOAD = ["display"]
DEPENDENCIES = []
CODEOWNERS = []

CONF_CS_PIN = "cs_pin"
CONF_DC_PIN = "dc_pin"
CONF_RESET_PIN = "reset_pin"
CONF_BUSY_PIN = "busy_pin"
CONF_CLK_PIN = "clk_pin"
CONF_MOSI_PIN = "mosi_pin"

gdey075t7_ns = cg.esphome_ns.namespace("gdey075t7_gxepd2")
GDEY075T7GxEPD2 = gdey075t7_ns.class_(
    "GDEY075T7GxEPD2",
    cg.PollingComponent,
    display.DisplayBuffer,
)

CONFIG_SCHEMA = (
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(GDEY075T7GxEPD2),
            cv.Required(CONF_CS_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_DC_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_BUSY_PIN): pins.internal_gpio_input_pin_schema,
            cv.Required(CONF_CLK_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_MOSI_PIN): pins.internal_gpio_output_pin_schema,
        }
    ).extend(cv.polling_component_schema("60s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await display.register_display(var, config)

    cs = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    rst = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    busy = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
    clk = await cg.gpio_pin_expression(config[CONF_CLK_PIN])
    mosi = await cg.gpio_pin_expression(config[CONF_MOSI_PIN])

    cg.add(var.set_cs_pin(cs))
    cg.add(var.set_dc_pin(dc))
    cg.add(var.set_reset_pin(rst))
    cg.add(var.set_busy_pin(busy))
    cg.add(var.set_clk_pin(clk))
    cg.add(var.set_mosi_pin(mosi))

    if CORE.is_esp32 and CORE.using_arduino:
        cg.add_library("SPI", None)

    cg.add_library("zinggjm/GxEPD2", None)