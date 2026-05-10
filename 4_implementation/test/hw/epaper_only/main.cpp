// Waveshare 2.9inch e-Paper（BWR 3色）配線確認用スケッチ。
// ビルド: pio run -e test_epaper_only
// ピン: MOSI D10, SCK D8, CS D1, DC D3, RST D6, BUSY D7（hardware_spec.md と同一）
//
// パネル: 赤/黒/白 3色 (GDEM029C90/SSD1619A相当, SSD1680互換レジスタ, HIGH=busy)
// GxEPD2_290_C90c を使用 (full_refresh_time=27000ms, 実ピン使用時はBUSY LOWで抜ける)
// BUSY 極性: HIGH=busy, LOW=idle
// rotation=1: landscape W=296, H=128（実機確認済み）
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <GxEPD2_3C.h>
#include <SPI.h>

#include "app/pin_config.h"
#include "display/epd_driver.h"

namespace {

GxEPD2_3C<EpdDriver, kEpdHeight> display(
    EpdDriver(pin::kEpdCs, pin::kEpdDc, pin::kEpdRst, pin::kEpdBusy));

// rotation=1: W=296, H=128
// 四隅座標: 左上(0,0), 右上(288,0), 左下(0,120), 右下(288,120)
void drawCorners(uint16_t color) {
  display.fillRect(0,   0,   8, 8, color); // 左上
  display.fillRect(288, 0,   8, 8, color); // 右上
  display.fillRect(0,   120, 8, 8, color); // 左下
  display.fillRect(288, 120, 8, 8, color); // 右下
}

void refreshScreen(uint16_t bg, uint16_t fg, const char* label, uint32_t tick) {
  char sub[24];
  snprintf(sub, sizeof(sub), "%s %lu", label, (unsigned long)tick);

  display.setRotation(1);
  display.setTextColor(fg);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(bg);
    drawCorners(fg);
    display.setCursor(16, 52);
    display.setTextSize(4);
    display.print("EPD");
    display.setCursor(16, 104);
    display.setTextSize(2);
    display.print(sub);
  } while (display.nextPage());
}

} // namespace

void setup() {
  Serial.begin(115200);
  delay(2000); // USB CDC 待ちをやめる（シリアルモニタなしでも動作）

  SPI.begin();
  display.init(0, true);
  Serial.println("[epd] init done");

  refreshScreen(GxEPD_WHITE, GxEPD_BLACK, "START", 0);
  Serial.println("[epd] start screen done");
}

void loop() {
  static uint32_t tick = 0;
  delay(20000); // 15s refresh + 余裕
  ++tick;

  switch (tick % 3u) {
    case 0:
      refreshScreen(GxEPD_WHITE, GxEPD_BLACK, "WHITE", tick);
      Serial.println("[epd] white");
      break;
    case 1:
      refreshScreen(GxEPD_BLACK, GxEPD_WHITE, "BLACK", tick);
      Serial.println("[epd] black");
      break;
    case 2:
      // 赤発色前に白でリセット → 粒子を初期位置に戻してから赤を駆動
      refreshScreen(GxEPD_WHITE, GxEPD_BLACK, "RED..", tick);
      delay(1000);
      refreshScreen(GxEPD_WHITE, GxEPD_RED, "RED", tick);
      Serial.println("[epd] red");
      break;
  }
}
