#include "gdey075t7_gxepd2.h"

namespace esphome {
namespace gdey075t7_gxepd2 {

static const char *const TAG = "gdey075t7_gxepd2";

void GDEY075T7GxEPD2::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GDEY075T7 GxEPD2 display");

  SPI.begin(
      this->clk_pin_->get_pin(),
      -1,
      this->mosi_pin_->get_pin(),
      this->cs_pin_->get_pin());

  this->epd_ =
      new GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2>(
          GxEPD2_750_GDEY075T7(
              this->cs_pin_->get_pin(),
              this->dc_pin_->get_pin(),
              this->reset_pin_->get_pin(),
              this->busy_pin_->get_pin()));

  this->epd_->init(0, true, 2, false);

  // Initial blank frame so the panel starts in a known state.
  this->epd_->firstPage();
  do {
    this->epd_->fillScreen(GxEPD_WHITE);
  } while (this->epd_->nextPage());
}

void GDEY075T7GxEPD2::dump_config() {
  ESP_LOGCONFIG(TAG, "GDEY075T7 GxEPD2 Display");
  LOG_UPDATE_INTERVAL(this);
  if (this->cs_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  CS pin: GPIO%d", this->cs_pin_->get_pin());
  }
  if (this->dc_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  DC pin: GPIO%d", this->dc_pin_->get_pin());
  }
  if (this->reset_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  RESET pin: GPIO%d", this->reset_pin_->get_pin());
  }
  if (this->busy_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  BUSY pin: GPIO%d", this->busy_pin_->get_pin());
  }
  if (this->clk_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  CLK pin: GPIO%d", this->clk_pin_->get_pin());
  }
  if (this->mosi_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  MOSI pin: GPIO%d", this->mosi_pin_->get_pin());
  }
}

void GDEY075T7GxEPD2::on_safe_shutdown() {
  if (this->epd_ != nullptr) {
    this->epd_->hibernate();
  }
}

void GDEY075T7GxEPD2::fill(Color color) {
  if (this->get_clipping().is_set()) {
    Display::fill(color);
    return;
  }
  if (this->epd_ == nullptr) return;
  this->epd_->fillScreen(color.is_on() ? GxEPD_BLACK : GxEPD_WHITE);
}

void GDEY075T7GxEPD2::update() {
  if (this->epd_ == nullptr) return;
  this->epd_->setFullWindow();
  this->epd_->firstPage();
  do {
    // Start each page with a white background, then let the lambda draw on top.
    this->epd_->fillScreen(GxEPD_WHITE);
    this->do_update_();
  } while (this->epd_->nextPage());
}

void GDEY075T7GxEPD2::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
  if (this->epd_ == nullptr) return;
  this->epd_->drawPixel(x, y, color.is_on() ? GxEPD_BLACK : GxEPD_WHITE);
}

}  // namespace gdey075t7_gxepd2
}  // namespace esphome