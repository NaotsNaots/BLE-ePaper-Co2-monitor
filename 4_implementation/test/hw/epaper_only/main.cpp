// Waveshare 2.9inch e-Paper（BWR 3色）配線確認用スケッチ。
// ビルド: pio run -e test_epaper_only
// ピン: MOSI D10, SCK D8, CS D1, DC D3, RST D6, BUSY D7（hardware_spec.md と同一）
//
// パネル: 赤/黒/白 3色 (GDEM029C90/SSD1619A相当, SSD1680互換レジスタ, HIGH=busy)
// GxEPD2_290_C90c を使用 (full_refresh_time=27000ms, 実ピン使用時はBUSY LOWで抜ける)
// BUSY 極性: HIGH=busy, LOW=idle
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <GxEPD2_3C.h>
#include <SPI.h>

#include "app/pin_config.h"
#include "display/epd_driver.h"

namespace {

// BUSY=D7 を有効化（HIGH=busy, LOW=idle で GxEPD2_290_C90c と一致）
// 実際のリフレッシュ完了（BUSY LOW）を待つので 27000ms タイムアウト前に終わる
GxEPD2_3C<EpdDriver, kEpdHeight> display(
    EpdDriver(pin::kEpdCs, pin::kEpdDc, pin::kEpdRst, pin::kEpdBusy));

void fullRefreshWhite() {
  display.setRotation(3);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
}

void fullRefreshBlack() {
  display.setRotation(3);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());
}

void fullRefreshMessage(const char* line1, const char* line2) {
  display.setRotation(1); // landscape: W=296, H=128
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    // 4隅に印（どちらが上か判別用）
    display.fillRect(0, 0, 8, 8, GxEPD_BLACK);       // 左上
    display.fillRect(0, 120, 8, 8, GxEPD_BLACK);     // 左下
    display.fillRect(288, 0, 8, 8, GxEPD_BLACK);     // 右上
    // 右下なし → 左上・左下・右上の3点で向きを判別
    display.setCursor(16, 48);
    display.setTextSize(4);
    display.print(line1);
    display.setCursor(16, 100);
    display.setTextSize(2);
    display.print(line2);
  } while (display.nextPage());
}

} // namespace

void setup() {
  Serial.begin(115200);
  delay(2000); // USB CDC 待ちをやめる（シリアルモニタなしでも動作）

  SPI.begin();
  display.init(0, true);
  Serial.println("[epd] init done");

  fullRefreshMessage("EPD", "BWR test OK");
  Serial.println("[epd] first refresh done");
}

void loop() {
  static uint32_t tick = 0;
  delay(18000); // 15s refresh + 余裕
  ++tick;
  if ((tick % 2u) == 0u) {
    fullRefreshWhite();
    Serial.println("[epd] white");
  } else {
    fullRefreshBlack();
    Serial.println("[epd] black");
  }
}
