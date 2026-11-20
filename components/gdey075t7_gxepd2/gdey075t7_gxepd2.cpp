#include "gdey075t7_gxepd2.h"

#include <esp_sleep.h>

namespace esphome {
namespace gdey075t7_gxepd2 {

#ifdef USE_GDEY075T7_GRAYSCALE
uint8_t GDEY075T7Panel4G::white_boost_frames = 0;

// Copy of the GxEPD2_4G greyscale LUTs for GDEY075T7 (UC8179), rows of
// {levels A|B|C|D (2 bit each: 00 GND, 01 to black, 10 to white), frames A..D, repeat}.
// Row 5 is the added white boost (frames filled in at runtime).
static uint8_t lut_vcom[42] = {
    0x00, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t lut_ww[42] = {  // white -> white (also used for the border)
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x10, 0x14, 0x0A, 0x00, 0x00, 0x01, 0xA0, 0x13, 0x0A, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t lut_bw[42] = {  // light grey
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t lut_wb[42] = {  // dark grey
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t lut_bb[42] = {  // black
    0x80, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x20, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

void GDEY075T7Panel4G::write_boost_luts_() {
  uint8_t *luts[] = {lut_vcom, lut_ww, lut_bw, lut_wb, lut_bb};
  for (uint8_t *l : luts) l[25] = white_boost_frames;  // row 5, phase A: same length in every LUT
  const uint8_t cmds[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25};
  uint8_t *data[] = {lut_vcom, lut_ww, lut_bw, lut_wb, lut_bb, lut_ww};
  for (int i = 0; i < 6; i++) {
    this->_writeCommand(cmds[i]);
    for (int j = 0; j < 42; j++) this->_writeData(data[i][j]);
  }
}

bool GDEY075T7Panel4G::two_pass = false;
uint8_t GDEY075T7Panel4G::light_frames = 4;
uint8_t GDEY075T7Panel4G::dark_frames = 5;

// One LUT row: `level` for `drive` frames, then GND until `total`; rest zero.
void GDEY075T7Panel4G::write_lut_(uint8_t cmd, uint8_t level, uint8_t drive, uint8_t total) {
  this->_writeCommand(cmd);
  const uint8_t row[6] = {level, drive, (uint8_t) (total - drive), 0x00, 0x00, 0x01};
  for (uint8_t b : row) this->_writeData(b);
  for (int i = 6; i < 42; i++) this->_writeData(0x00);
}

// Controller RAM (written by writeImage_4G): white (1,1), black (0,0),
// light grey (old 0, new 1), dark grey (old 1, new 0) — see to_grey_().
void GDEY075T7Panel4G::two_pass_refresh_() {
  // Pass 1: factory OTP waveform (fast full update) -> every pixel becomes its
  // "new" colour: white/light grey -> white, black/dark grey -> black.
  this->_writeCommand(0x00);  // panel setting: KW, LUT from OTP
  this->_writeData(0x1f);
  this->_writeCommand(0x50);  // VCOM/data interval: LUTBD, no N2OCP (keep old plane!)
  this->_writeData(0x31);
  this->_writeData(0x07);
  this->_writeCommand(0xE0);  // cascade setting: TSFIX
  this->_writeData(0x02);
  this->_writeCommand(0xE5);  // force temperature 90 -> fast full update
  this->_writeData(0x5A);
  if (!this->_power_is_on) {
    this->_writeCommand(0x04);
    this->_waitWhileBusy("two_pass power on", 140);
    this->_power_is_on = true;
  }
  this->_writeCommand(0x12);
  this->_waitWhileBusy("two_pass OTP", 1200);

  // Pass 2: register LUTs, only grey pixels are driven.
  const uint8_t total = std::max(light_frames, dark_frames);
  this->_writeCommand(0xE0);  // back to internal temperature
  this->_writeData(0x00);
  this->_writeCommand(0x00);  // panel setting: KW, LUT from registers
  this->_writeData(0x3f);
  this->write_lut_(0x20, 0x00, total, total);         // VCOM: DC level
  this->write_lut_(0x21, 0x00, total, total);         // white: no drive
  this->write_lut_(0x22, 0x40, light_frames, total);  // old 0/new 1 = light grey: black pulse
  this->write_lut_(0x23, 0x80, dark_frames, total);   // old 1/new 0 = dark grey: white pulse
  this->write_lut_(0x24, 0x00, total, total);         // black: no drive
  this->write_lut_(0x25, 0x00, total, total);         // border: no drive
  this->_writeCommand(0x12);
  this->_waitWhileBusy("two_pass grey", 500);
}

void GDEY075T7Panel4G::refresh(bool partial_update_mode) {
  if (!partial_update_mode && two_pass) {
    this->two_pass_refresh_();
    return;
  }
  // Only relevant for the register-LUT (greyscale) refresh; in OTP mode the
  // controller ignores the LUT registers, so writing them is harmless.
  if (!partial_update_mode && white_boost_frames > 0) this->write_boost_luts_();
  GxEPD2_750_GDEY075T7::refresh(partial_update_mode);
}
#endif

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
#ifdef USE_GDEY075T7_GRAYSCALE
          GDEY075T7Panel4G(
#else
          GxEPD2_750_GDEY075T7(
#endif
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
  ESP_LOGCONFIG(TAG, "  Mode: 4-level greyscale (GxEPD2_4G), edges: %s, white boost: %u frames",
                this->light_edges_ ? "light" : "normal", (unsigned) GDEY075T7Panel4G::white_boost_frames);
  if (GDEY075T7Panel4G::two_pass)
    ESP_LOGCONFIG(TAG, "  Refresh: two-pass (OTP b/w + grey pulses light %u / dark %u frames)",
                  (unsigned) GDEY075T7Panel4G::light_frames, (unsigned) GDEY075T7Panel4G::dark_frames);
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