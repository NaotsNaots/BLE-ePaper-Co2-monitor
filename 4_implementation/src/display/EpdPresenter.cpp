#include "EpdPresenter.h"
#include <SPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "app/pin_config.h"

EpdPresenter::EpdPresenter()
    : display_(EpdDriver(pin::kEpdCs, pin::kEpdDc, pin::kEpdRst, pin::kEpdBusy)) {}

bool EpdPresenter::begin() {
  SPI.begin();
  display_.init(0, true);
  ok_ = true;
  return true;
}

void EpdPresenter::show(const MeasurementFrame& m) {
  if (!ok_) {
    return;
  }
  display_.setRotation(1);
  display_.setFont(&FreeMonoBold9pt7b);
  display_.setTextColor(GxEPD_BLACK);
  char line[48];
  display_.setFullWindow();
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    display_.setCursor(8, 32);
    snprintf(line, sizeof(line), "VOC %d", (int)m.vocIndex);
    display_.print(line);
    display_.setCursor(8, 56);
    snprintf(line, sizeof(line), "eCO2~ %u", (unsigned)m.eco2Ppm);
    display_.print(line);
    display_.setCursor(8, 80);
    snprintf(line, sizeof(line), "#%lu", (unsigned long)m.sampleIndex);
    display_.print(line);
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
