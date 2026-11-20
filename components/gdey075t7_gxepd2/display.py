import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_PAGES
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
CONF_GRAYSCALE = "grayscale"
CONF_GRAYSCALE_EDGES = "grayscale_edges"
CONF_GRAYSCALE_WHITE_BOOST = "grayscale_white_boost"

# 4-level greyscale needs ZinggJM/GxEPD2_4G (not in the PlatformIO registry, pinned commit)
GXEPD2_4G_REPO = "https://github.com/ZinggJM/GxEPD2_4G.git#de006169dba1d905f618d70423f0e1a9215b20c7"

gdey075t7_ns = cg.esphome_ns.namespace("gdey075t7_gxepd2")
GDEY075T7GxEPD2 = gdey075t7_ns.class_(
    "GDEY075T7GxEPD2",
    cg.PollingComponent,
    display.DisplayBuffer,
)


def _validate_arduino(config):
    if not (CORE.is_esp32 and CORE.using_arduino):
        raise cv.Invalid(
            "gdey075t7_gxepd2 requires an ESP32 board with the Arduino framework"
        )
    return config


CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(GDEY075T7GxEPD2),
            cv.Required(CONF_CS_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_DC_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_BUSY_PIN): pins.internal_gpio_input_pin_schema,
            cv.Required(CONF_CLK_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_MOSI_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_GRAYSCALE, default=False): cv.boolean,
            # How anti-aliased edge pixels map to the 4 levels:
            #   normal: 1/3 -> light grey, 2/3 -> dark grey   (smoothest, looks bolder on the panel)
            #   light:  1/3 -> white,      2/3 -> light grey  (slimmer strokes, still smooth)
            cv.Optional(CONF_GRAYSCALE_EDGES, default="normal"): cv.one_of("normal", "light", lower=True),
            # Experimental: extra frames (~20 ms each) at the end of the greyscale
            # waveform that drive only white pixels (and the border) to white again.
            # Counters the grey veil of the GxEPD2_4G waveform. 0 = library waveform.
            cv.Optional(CONF_GRAYSCALE_WHITE_BOOST, default=0): cv.int_range(min=0, max=255),
        }
    ).extend(cv.polling_component_schema("60s")),
    _validate_arduino,
)

async def to_code(config):
    cg.add_library("SPI", None)
    cg.add_library("Wire", None)
    cg.add_library("adafruit/Adafruit BusIO", None)
    cg.add_library("adafruit/Adafruit GFX Library", None)
    if config[CONF_GRAYSCALE]:
        # GxEPD2_4G and GxEPD2 define the same panel classes — only one may be linked.
        cg.add_define("USE_GDEY075T7_GRAYSCALE")
        cg.add_library("GxEPD2_4G", None, GXEPD2_4G_REPO)
    else:
        cg.add_library("zinggjm/GxEPD2", None)

    var = cg.new_Pvariable(config[CONF_ID])
    if config[CONF_GRAYSCALE] and config[CONF_GRAYSCALE_EDGES] == "light":
        cg.add(var.set_light_edges(True))
    if config[CONF_GRAYSCALE] and config[CONF_GRAYSCALE_WHITE_BOOST] > 0:
        cg.add(var.set_white_boost(config[CONF_GRAYSCALE_WHITE_BOOST]))

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

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))