#pragma once

// Match physical Waveshare 2.9\" panel via hardware label — swap to GxEPD2_290_C90 if board is (C) revision.
#include <GxEPD2_BW.h>

using EpdDriver = GxEPD2_290_GDEY029T71H;
constexpr uint16_t kEpdHeight = EpdDriver::HEIGHT;
