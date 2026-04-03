#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display_buffer.h"

#include <GxEPD2_BW.h>
#include <SPI.h>

namespace esphome {
namespace gdey075t7_gxepd2 {

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

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_BINARY;
  }

 protected:
  static const int WIDTH = 800;
  static const int HEIGHT = 480;

  int get_width_internal() override { return WIDTH; }
  int get_height_internal() override { return HEIGHT; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void flush_to_panel_();

  InternalGPIOPin *cs_pin_{nullptr};
  InternalGPIOPin *dc_pin_{nullptr};
  InternalGPIOPin *reset_pin_{nullptr};
  InternalGPIOPin *busy_pin_{nullptr};
  InternalGPIOPin *clk_pin_{nullptr};
  InternalGPIOPin *mosi_pin_{nullptr};

  GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2> *epd_{nullptr};
};

}  // namespace gdey075t7_gxepd2
}  // namespace esphome