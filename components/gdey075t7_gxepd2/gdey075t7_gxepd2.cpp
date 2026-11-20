#include "gdey075t7_gxepd2.h"

#include <esp_sleep.h>

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
      new GDEY075T7Epd(
          GxEPD2_750_GDEY075T7(
              this->cs_pin_->get_pin(),
              this->dc_pin_->get_pin(),
              this->reset_pin_->get_pin(),
              this->busy_pin_->get_pin()));

  this->epd_->init(0, true, 2, false);

  // Waking from deep sleep: the panel still shows the last image. Skip the
  // blank frame so the old image stays visible until the next update() replaces
  // it in a single full refresh (avoids a visible white phase and one refresh).
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
    ESP_LOGD(TAG, "Woke from deep sleep - keeping previous image");
    return;
  }

  // Cold boot: initial blank frame so the panel starts in a known state.
  this->epd_->firstPage();
  do {
    this->epd_->fillScreen(GxEPD_WHITE);
  } while (this->epd_->nextPage());
}

void GDEY075T7GxEPD2::dump_config() {
  ESP_LOGCONFIG(TAG, "GDEY075T7 GxEPD2 Display");
#ifdef USE_GDEY075T7_GRAYSCALE
  ESP_LOGCONFIG(TAG, "  Mode: 4-level greyscale (GxEPD2_4G)");
#else
  ESP_LOGCONFIG(TAG, "  Mode: black/white");
#endif
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
#ifdef USE_GDEY075T7_GRAYSCALE
  static const uint16_t LEVELS[4] = {GxEPD_BLACK, GxEPD_DARKGREY, GxEPD_LIGHTGREY, GxEPD_WHITE};
  this->epd_->fillScreen(LEVELS[to_grey_(color) >> 6]);
#else
  this->epd_->fillScreen(color.is_on() ? GxEPD_BLACK : GxEPD_WHITE);
#endif
}

void GDEY075T7GxEPD2::update() {
  if (this->epd_ == nullptr) return;
  this->epd_->setFullWindow();
  this->epd_->firstPage();
  do {
    this->do_update_();
  } while (this->epd_->nextPage());
}

void GDEY075T7GxEPD2::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
  if (this->epd_ == nullptr) return;
#ifdef USE_GDEY075T7_GRAYSCALE
  this->epd_->drawGreyPixel(x, y, to_grey_(color));
#else
  this->epd_->drawPixel(x, y, color.is_on() ? GxEPD_BLACK : GxEPD_WHITE);
#endif
}

}  // namespace gdey075t7_gxepd2
}  // namespace esphome