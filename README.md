# ESPHome external component for Good Display GDEY075T7 via GxEPD2

ESPHome display platform for the **Good Display GDEY075T7** (7.5 inch, 800×480, monochrome e-ink, UC8179 controller). Wraps the Arduino [GxEPD2](https://github.com/ZinggJM/GxEPD2) library.

**Supported hardware stack:**
- Panel: [GDEY075T7](https://www.good-display.com/product/396.html) — 800×480, 7.5", monochrome
- MCU: [Seeed XIAO ESP32S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
- Expansion board: [Seeed XIAO eInk Expansion Board V2](https://wiki.seeedstudio.com/xiao_eink_expansion_board_v2/)

**Requirements:** ESP32, Arduino framework (not ESP-IDF).

---

## Pin mapping (Seeed eInk Expansion Board V2 + XIAO ESP32S3)

| Signal | XIAO ESP32S3 GPIO |
|--------|-------------------|
| CS     | GPIO2             |
| DC     | GPIO4             |
| RESET  | GPIO1             |
| BUSY   | GPIO3             |
| CLK    | GPIO7             |
| MOSI   | GPIO9             |

---

## Installation

### Option A – local checkout

Clone this repository next to your ESPHome configuration directory:

```bash
git clone https://github.com/ceear/esphome-gdey075t7.git
```

Then reference it in your YAML:

```yaml
external_components:
  - source:
      type: local
      path: ../esphome-gdey075t7/components
    components: [ gdey075t7_gxepd2 ]
```

### Option B – directly from GitHub

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/ceear/esphome-gdey075t7
    components: [ gdey075t7_gxepd2 ]
```

---

## Minimal example

```yaml
esphome:
  name: my-eink

external_components:
  - source:
      type: local
      path: ../esphome-gdey075t7/components
    components: [ gdey075t7_gxepd2 ]

esp32:
  board: seeed_xiao_esp32s3
  framework:
    type: arduino

psram:
  mode: octal

logger:
api:
ota:
  - platform: esphome
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

font:
  - file: "gfonts://Roboto"
    id: font_roboto
    size: 36

display:
  - platform: gdey075t7_gxepd2
    id: epd
    cs_pin: GPIO2
    dc_pin: GPIO4
    reset_pin: GPIO1
    busy_pin: GPIO3
    clk_pin: GPIO7
    mosi_pin: GPIO9
    update_interval: 120s
    lambda: |-
      it.fill(COLOR_OFF);
      it.printf(20, 60, id(font_roboto), COLOR_ON, "Hello World");
      it.rectangle(10, 10, 780, 460);
```

---

## Usage notes

### Display colors

This is a binary (monochrome) display:

| ESPHome constant | Meaning    |
|------------------|------------|
| `COLOR_ON`       | Black ink  |
| `COLOR_OFF`      | White (no ink) |

All standard ESPHome drawing primitives (`print`, `printf`, `rectangle`, `line`, `filled_rectangle`, `image`, …) work as usual.

### Refresh speed

A full refresh takes roughly 3–5 seconds. During refresh, the display cycles through black and white flashes — this is normal for e-ink. A minimum `update_interval` of `60s` is recommended to avoid excessive wear on the panel.

### PSRAM

The XIAO ESP32S3 has 8 MB octal PSRAM. Enable it to ensure there is enough heap for the display buffer (38 400 bytes) and other components:

```yaml
psram:
  mode: octal
```

### Arduino framework

This component relies on the Arduino SPI library and the GxEPD2 Arduino library. The ESP-IDF framework is **not** supported.

---

## Compatibility (verified 2026-04-03, ESPHome 2026.3.2)

### ESPHome Display API vs native Waveshare integration

| API feature | Waveshare (built-in) | GDEY075T7 |
|---|---|---|
| `DisplayBuffer` base class | ✅ | ✅ |
| `draw_absolute_pixel_internal()` | ✅ | ✅ |
| `fill()` — full screen | ✅ | ✅ via `epd_->fillScreen()` |
| `fill()` — with active clipping region | ✅ | ✅ falls back to pixel-by-pixel |
| `get_display_type()` → BINARY | ✅ | ✅ |
| `rotation` | ✅ | ✅ handled by `DisplayBuffer::draw_pixel_at()` |
| `lambda` | ✅ | ✅ |
| `pages` + `on_page_change` automation | ✅ | ✅ |
| `auto_clear_enabled` | ✅ | ✅ |
| `show_test_card` | ✅ | ✅ |
| `update_interval` | ✅ | ✅ |
| `on_safe_shutdown()` → deep sleep | ✅ | ✅ via `epd_->hibernate()` |
| `dump_config()` | ✅ | ✅ |

### ESPHomeDesigner compatibility

ESPHomeDesigner generates standard ESPHome display lambdas. All generated drawing primitives (`print`, `printf`, `image`, `qr_code`, `rectangle`, `line`, …) route through the standard `DisplayBuffer` interface and work identically to any native Waveshare display.

### Architectural differences (not functional gaps)

| Aspect | Waveshare (built-in) | GDEY075T7 |
|---|---|---|
| SPI layer | ESPHome `SPIDevice` (shared bus) | Arduino `SPI.begin()` (dedicated bus) |
| Render pipeline | buffer → `write_array()` single pass | GxEPD2 paged; lambda called once per page |
| Framework support | all ESPHome-supported boards | ESP32 + Arduino framework only |
