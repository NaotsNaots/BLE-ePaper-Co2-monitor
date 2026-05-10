#pragma once

#include <GxEPD2_BW.h>
#include "app/types.h"
#include "display/epd_driver.h"

class EpdPresenter {
public:
  EpdPresenter();
  bool begin();
  void show(const MeasurementFrame& m);
  void showError(const char* line1, const char* line2);
  void hibernatePanel();

private:
  GxEPD2_BW<EpdDriver, kEpdHeight> display_;
  bool ok_ = false;
};
