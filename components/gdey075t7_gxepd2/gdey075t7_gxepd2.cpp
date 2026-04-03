#include "gdey075t7_gxepd2.h"

namespace esphome {
namespace gdey075t7_gxepd2 {

static const char *const TAG = "gdey075t7_gxepd2";

void GDEY075T7GxEPD2::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GDEY075T7 GxEPD2 display");

  // 1 bit per pixel
  this->init_internal_(WIDTH * HEIGHT / 8);

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

void GDEY075T7GxEPD2::update() {
  this->do_update_();
  this->flush_to_panel_();
}

bool GDEY075T7GxEPD2::get_pixel_(int x, int y) const {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) {
    return false;
  }

  const uint32_t pos = (x + y * WIDTH) / 8u;
  const uint8_t bit = 0x80 >> (x & 0x07);
  return (this->buffer_[pos] & bit) != 0;
}

void GDEY075T7GxEPD2::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) {
    return;
  }

  const uint32_t pos = (x + y * WIDTH) / 8u;
  const uint8_t bit = 0x80 >> (x & 0x07);

  // color.is_on() => black pixel
  if (color.is_on()) {
    this->buffer_[pos] |= bit;
  } else {
    this->buffer_[pos] &= ~bit;
  }
}

void GDEY075T7GxEPD2::flush_to_panel_() {
  if (this->epd_ == nullptr) {
    ESP_LOGE(TAG, "Display object not initialized");
    return;
  }

  this->epd_->setFullWindow();
  this->epd_->firstPage();
  do {
    this->epd_->drawBitmap(0, 0, (const uint8_t *) this->buffer_, WIDTH, HEIGHT,
                            GxEPD_BLACK, GxEPD_WHITE);
  } while (this->epd_->nextPage());
}

}  // namespace gdey075t7_gxepd2
}  // namespace esphome