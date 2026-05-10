// Waveshare 2.9inch e-Paper（SPI）のみの配線確認用スケッチ。
// ビルド: pio run -e test_epaper_only
// ピン: MOSI D10, SCK D8, CS D1, DC D3, RST D6, BUSY D7（hardware_spec.md と同一）
//
// 実機が (C) 改版などで描画がおかしい場合はプロジェクトの display/epd_driver.h の
// GxEPD2 クラスをマニュアルに合わせて差し替えること。
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

#include "app/pin_config.h"
#include "display/epd_driver.h"

namespace {
constexpr uint32_t kUsbReadyMs = 800;

GxEPD2_BW<EpdDriver, kEpdHeight> display(EpdDriver(pin::kEpdCs, pin::kEpdDc,
                                                   pin::kEpdRst, pin::kEpdBusy));

void fullRefreshMessage(const char* line1, const char* line2) {
  display.setRotation(3);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(8, 36);
    display.setTextSize(2);
    display.print(line1);
    display.setCursor(8, 72);
    display.setTextSize(1);
    display.print(line2);
  } while (display.nextPage());
}
} // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // USB CDC 接続待ち
  (void)kUsbReadyMs;

  SPI.begin();
  display.init(0, true);
  fullRefreshMessage("EPD", "test OK");
  Serial.println("[epd] first refresh done");
}

void loop() {
  static uint32_t tick = 0;
  delay(8000);
  ++tick;
  if ((tick % 2u) == 0u) {
    fullRefreshMessage("EPD", "invert ->");
    Serial.println("[epd] white field");
  } else {
    display.setRotation(3);
    display.setFullWindow();
    display.firstPage();
    do {
      display.fillScreen(GxEPD_BLACK);
    } while (display.nextPage());
    Serial.println("[epd] black field");
  }
}
