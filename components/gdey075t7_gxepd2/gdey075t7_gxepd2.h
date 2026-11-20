#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display_buffer.h"

#include "esphome/core/defines.h"

#ifdef USE_GDEY075T7_GRAYSCALE
#include <GxEPD2_4G_4G.h>
#else
#include <GxEPD2_BW.h>
#endif
#include <SPI.h>

namespace esphome {
namespace gdey075t7_gxepd2 {

#ifdef USE_GDEY075T7_GRAYSCALE
// Panel class of GxEPD2_4G with an optional "white boost": before every full
// refresh the greyscale LUTs are rewritten with one extra phase in which only
// white pixels (and the border) are driven to white again. The library LUTs end
// with a slightly grey white on this panel.
class GDEY075T7Panel4G : public GxEPD2_750_GDEY075T7 {
 public:
  using GxEPD2_750_GDEY075T7::GxEPD2_750_GDEY075T7;
  using GxEPD2_750_GDEY075T7::refresh;
  void refresh(bool partial_update_mode = false) override;
  static uint8_t white_boost_frames;  // 0 = unchanged library waveform
  static bool two_pass;               // OTP black/white pass + short grey pass
  static uint8_t light_frames, dark_frames;

 protected:
  void write_boost_luts_();
  void two_pass_refresh_();
  void write_lut_(uint8_t cmd, uint8_t level, uint8_t drive, uint8_t total);
};

// 4 grey levels (2 bit per pixel); full refresh uses the greyscale waveform.
using GDEY075T7Epd = GxEPD2_4G_4G<GDEY075T7Panel4G, GDEY075T7Panel4G::HEIGHT / 2>;
#else
using GDEY075T7Epd = GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2>;
#endif

class GDEY075T7GxEPD2 : public display::DisplayBuffer {
 public:
  void set_cs_pin(InternalGPIOPin *pin) { this->cs_pin_ = pin; }
  void set_dc_pin(InternalGPIOPin *pin) { this->dc_pin_ = pin; }
  void set_reset_pin(InternalGPIOPin *pin) { this->reset_pin_ = pin; }
  void set_busy_pin(InternalGPIOPin *pin) { this->busy_pin_ = pin; }
  void set_clk_pin(InternalGPIOPin *pin) { this->clk_pin_ = pin; }
  void set_mosi_pin(InternalGPIOPin *pin) { this->mosi_pin_ = pin; }
  // Greyscale mode only: two-pass refresh (OTP black/white, then grey pulses).
  void set_two_pass(uint8_t light_frames, uint8_t dark_frames) {
#ifdef USE_GDEY075T7_GRAYSCALE
    GDEY075T7Panel4G::two_pass = true;
    GDEY075T7Panel4G::light_frames = light_frames;
    GDEY075T7Panel4G::dark_frames = dark_frames;
#endif
  }
  // Greyscale mode only: extra white drive frames at the end of the waveform.
  void set_white_boost(uint8_t frames) {
#ifdef USE_GDEY075T7_GRAYSCALE
    GDEY075T7Panel4G::white_boost_frames = frames;
#endif
  }
  // Greyscale mode only: map 1/3-covered edge pixels to white (slimmer text).
  void set_light_edges(bool light) {
#ifdef USE_GDEY075T7_GRAYSCALE
    this->light_edges_ = light;
#endif
  }

  void setup() override;
  void update() override;
  void dump_config() override;
  void on_safe_shutdown() override;
  void fill(Color color) override;

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  display::DisplayType get_display_type() override {
#ifdef USE_GDEY075T7_GRAYSCALE
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
#else
    return display::DisplayType::DISPLAY_TYPE_BINARY;
#endif
  }

 protected:
  static const int WIDTH = 800;
  static const int HEIGHT = 480;

  int get_width_internal() override { return WIDTH; }
  int get_height_internal() override { return HEIGHT; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  InternalGPIOPin *cs_pin_{nullptr};
  InternalGPIOPin *dc_pin_{nullptr};
  InternalGPIOPin *reset_pin_{nullptr};
  InternalGPIOPin *busy_pin_{nullptr};
  InternalGPIOPin *clk_pin_{nullptr};
  InternalGPIOPin *mosi_pin_{nullptr};

#ifdef USE_GDEY075T7_GRAYSCALE
  // ESPHome colour -> grey value for GxEPD2_4G (0 = black, 255 = white).
  // COLOR_ON means ink, so more colour means darker; anti-aliased font pixels
  // arrive as gradients between background and foreground.
  uint8_t to_grey_(Color color) const {
    const uint8_t ink = std::max(std::max(color.r, color.g), std::max(color.b, color.w));
    const uint8_t level = (ink * 3 + 127) / 255;  // 0 = white .. 3 = black
    static const uint8_t NORMAL[4] = {255, 170, 85, 0};
    static const uint8_t LIGHT[4] = {255, 255, 170, 0};
    uint8_t grey = (this->light_edges_ ? LIGHT : NORMAL)[level];
    // Two-pass: GxEPD2_4G stores level 170 as (old 1, new 0) and 85 as (old 0, new 1).
    // The OTP pass turns pixels into their "new" colour, so swap the greys: light grey
    // starts from white, dark grey from black (the closest base for each).
    if (GDEY075T7Panel4G::two_pass && (grey == 170 || grey == 85)) grey = 255 - grey;
    return grey;
  }
  bool light_edges_{false};
#endif

  GDEY075T7Epd *epd_{nullptr};
};

}  // namespace gdey075t7_gxepd2
}  // namespace esphome