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
// 4 grey levels (2 bit per pixel); full refresh uses the greyscale waveform.
using GDEY075T7Epd = GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2>;
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
  static uint8_t to_grey_(Color color) {
    uint8_t ink = std::max(std::max(color.r, color.g), std::max(color.b, color.w));
    return 255 - ink;
  }
#endif

  GDEY075T7Epd *epd_{nullptr};
};

}  // namespace gdey075t7_gxepd2
}  // namespace esphome