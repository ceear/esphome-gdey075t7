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

### Greyscale mode (optional)

```yaml
display:
  - platform: gdey075t7_gxepd2
    grayscale: true
    grayscale_edges: normal   # or: light (slimmer text)
    grayscale_white_boost: 0  # experimental, e.g. 20: extra white drive against a grey veil
    grayscale_refresh: lut    # or: two_pass (experimental, see below)
    grayscale_light_frames: 4 # two_pass only
    grayscale_dark_frames: 5  # two_pass only
    # ...
font:
  - file: "gfonts://Roboto"
    size: 17
    bpp: 2          # anti-aliased glyphs -> 4 grey levels on the panel
```

With `grayscale: true` the component uses [GxEPD2_4G](https://github.com/ZinggJM/GxEPD2_4G) instead of GxEPD2 and reports itself as a greyscale display. Colours are mapped to 4 levels (white, light grey, dark grey, black) by their brightness, so anti-aliased fonts (`bpp: 2`) get smooth edges. Text and layout geometry are unchanged.

`grayscale_edges` controls how partially covered edge pixels are drawn: `normal` maps 1/3 and 2/3 coverage to light and dark grey (smoothest), `light` maps 1/3 to white and 2/3 to light grey — strokes look slimmer while curves stay smooth. Grey levels tend to look darker on the panel than on a monitor.

`grayscale_white_boost` (experimental, default 0) appends N frames (≈20 ms each) to the greyscale waveform in which only white pixels and the border are driven to white again. With the library waveform the white background can look slightly grey on this panel. Grey levels and black are not changed; the refresh takes N × ≈20 ms longer.

`grayscale_refresh: two_pass` (experimental) first runs the panel's factory black/white waveform — clean white and black like in black/white mode — and then a short register-LUT pass that only drives the grey pixels: light grey gets a black pulse of `grayscale_light_frames` from white, dark grey a white pulse of `grayscale_dark_frames` from black (frames ≈ 20 ms). White and black pixels are not driven in the second pass. The pulse lengths depend on the panel and temperature; tune them visually.

Tip: TrueType fonts with their own hinting (e.g. Roboto from Google Fonts) are only hinted vertically when anti-aliased, so vertical stems can land between pixels and some letters look thinner than others. A copy of the font with the hinting instructions removed makes FreeType use its auto-hinter, which snaps all stems to whole pixels.

Trade-offs: every update is a greyscale full refresh (≈2.5 s instead of ≈1.2 s), the frame buffer doubles, and fonts with `bpp: 2` need twice the flash. For white text on a black area pass the background colour, e.g. `it.printf(x, y, font, COLOR_OFF, COLOR_ON, TextAlign::TOP_LEFT, "…")`, otherwise anti-aliased edges are blended against white.

### Refresh speed

A full refresh takes roughly 3–5 seconds. During refresh, the display cycles through black and white flashes — this is normal for e-ink. A minimum `update_interval` of `60s` is recommended to avoid excessive wear on the panel.

On a cold boot (power-on, reset, flashing) the component clears the panel to white once during `setup()`. When the ESP32 wakes from deep sleep, this blank frame is skipped: the previous image stays visible until the first `update()` replaces it in a single refresh.

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
| `get_display_type()` → BINARY | ✅ | ✅ (GRAYSCALE with `grayscale: true`) |
| `rotation` | ✅ | ✅ handled by `DisplayBuffer::draw_pixel_at()` |
| `lambda` | ✅ | ✅ |
| `pages` + `on_page_change` automation | ✅ | ✅ |
| `auto_clear_enabled` | ✅ | ✅ |
| `show_test_card` | ✅ | ✅ |
| `update_interval` | ✅ | ✅ |
| `on_safe_shutdown()` → deep sleep | ✅ | ✅ via `epd_->hibernate()` |
| Wake from deep sleep without blank frame | — | ✅ previous image kept until first `update()` |
| `dump_config()` | ✅ | ✅ |

### ESPHomeDesigner compatibility

ESPHomeDesigner generates standard ESPHome display lambdas. All generated drawing primitives (`print`, `printf`, `image`, `qr_code`, `rectangle`, `line`, …) route through the standard `DisplayBuffer` interface and work identically to any native Waveshare display.

### Architectural differences (not functional gaps)

| Aspect | Waveshare (built-in) | GDEY075T7 |
|---|---|---|
| SPI layer | ESPHome `SPIDevice` (shared bus) | Arduino `SPI.begin()` (dedicated bus) |
| Render pipeline | buffer → `write_array()` single pass | GxEPD2 paged; lambda called once per page |
| Framework support | all ESPHome-supported boards | ESP32 + Arduino framework only |
