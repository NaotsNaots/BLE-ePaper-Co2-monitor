#include "EpdPresenter.h"
#include <SPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "app/pin_config.h"

EpdPresenter::EpdPresenter()
    : display_(EpdDriver(pin::kEpdCs, pin::kEpdDc, pin::kEpdRst, pin::kEpdBusy)) {}

bool EpdPresenter::begin() {
  SPI.begin();
  display_.init(0, true);
  ok_ = true;
  return true;
}

namespace {

// 296 x 128（横向き）。左に eCO2 の大きな数字とトレンド矢印、右に換気の判定、下に VOC と測定番号。
constexpr int16_t kPanelX = 192;  // 右側の判定パネルの左端

void printCentered(Adafruit_GFX& d, const char* s, int16_t cx, int16_t baseline) {
  int16_t x1, y1;
  uint16_t w, h;
  d.getTextBounds(s, 0, baseline, &x1, &y1, &w, &h);
  d.setCursor(cx - (int16_t)w / 2 - x1, baseline);
  d.print(s);
}

void drawTrend(Adafruit_GFX& d, TrendClass t, int16_t x, int16_t cy) {
  switch (t) {
    case TrendClass::Up:
      d.fillTriangle(x, cy + 9, x + 20, cy + 9, x + 10, cy - 11, GxEPD_BLACK);
      break;
    case TrendClass::Down:
      d.fillTriangle(x, cy - 9, x + 20, cy - 9, x + 10, cy + 11, GxEPD_BLACK);
      break;
    default:  // 横ばい: 右向き矢印
      d.fillRect(x, cy - 2, 12, 5, GxEPD_BLACK);
      d.fillTriangle(x + 11, cy - 9, x + 11, cy + 9, x + 21, cy, GxEPD_BLACK);
      break;
  }
}

}  // namespace

void EpdPresenter::show(const MeasurementFrame& m) {
  if (!ok_) {
    return;
  }
  // VOC Index は学習が終わるまで 0 を返す（正常値は 1〜500）。その間は判定しない。
  const bool warming = m.vocIndex <= 0;

  const char* l1;
  const char* l2;
  uint16_t panelFill = GxEPD_WHITE;
  uint16_t panelText = GxEPD_BLACK;
  if (warming) {
    l1 = "WARM";
    l2 = "UP";
  } else if (m.vent == VentClass::High) {
    l1 = "OPEN";
    l2 = "WINDOW";
    panelFill = GxEPD_RED;  // 実機ではオレンジに発色
    panelText = GxEPD_WHITE;
  } else if (m.vent == VentClass::Mild) {
    l1 = "VENT";
    l2 = "SOON";
    panelFill = GxEPD_BLACK;
    panelText = GxEPD_WHITE;
  } else {
    l1 = "AIR";
    l2 = "OK";
  }

  char num[12];
  char line[40];
  display_.setRotation(1);
  display_.setFullWindow();
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);

    // 左: eCO2
    display_.setTextColor(GxEPD_BLACK);
    display_.setFont(&FreeSansBold24pt7b);
    if (warming) {
      snprintf(num, sizeof(num), "---");
    } else {
      snprintf(num, sizeof(num), "%u", (unsigned)m.eco2Ppm);
    }
    display_.setCursor(8, 56);
    display_.print(num);
    int16_t x1, y1;
    uint16_t w, h;
    display_.getTextBounds(num, 8, 56, &x1, &y1, &w, &h);
    if (!warming) {
      drawTrend(display_, m.trend, x1 + (int16_t)w + 10, 38);
    }
    display_.setFont(&FreeSans9pt7b);
    display_.setCursor(10, 80);
    display_.print("ppm eCO2 (est.)");

    // 下: VOC と測定番号
    display_.drawFastHLine(0, 96, kPanelX - 6, GxEPD_BLACK);
    display_.setCursor(10, 118);
    snprintf(line, sizeof(line), "VOC %d   #%lu", (int)m.vocIndex, (unsigned long)m.sampleIndex);
    display_.print(line);

    // 右: 換気の判定
    const int16_t pw = 296 - kPanelX - 4;
    if (panelFill == GxEPD_WHITE) {
      display_.drawRoundRect(kPanelX, 4, pw, 120, 8, GxEPD_BLACK);
      display_.drawRoundRect(kPanelX + 1, 5, pw - 2, 118, 7, GxEPD_BLACK);
    } else {
      display_.fillRoundRect(kPanelX, 4, pw, 120, 8, panelFill);
    }
    display_.setTextColor(panelText);
    display_.setFont(&FreeSansBold12pt7b);
    const int16_t cx = kPanelX + pw / 2;
    printCentered(display_, l1, cx, 58);
    printCentered(display_, l2, cx, 90);
  } while (display_.nextPage());
}

void EpdPresenter::showError(const char* line1, const char* line2) {
  if (!ok_) {
    return;
  }
  display_.setRotation(1);
  display_.setFont(&FreeMonoBold9pt7b);
  display_.setTextColor(GxEPD_BLACK);
  display_.setFullWindow();
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    display_.setCursor(8, 40);
    display_.print(line1);
    display_.setCursor(8, 64);
    display_.print(line2);
  } while (display_.nextPage());
}

void EpdPresenter::hibernatePanel() {
  if (!ok_) {
    return;
  }
  display_.hibernate();
}
