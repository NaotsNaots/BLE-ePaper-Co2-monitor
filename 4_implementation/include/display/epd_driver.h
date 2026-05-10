#pragma once

// 2.9inch e-Paper Module BWR (赤/黒/白 3色)
// Amazon互換品 (GDEM029C90/SSD1619A 相当, 296x128px)
// 基板シルク: "waveshare 2.9inch e-Paper Module V4 rev2.1 296x128 Pixel"
// レジスタ: SSD1680 互換 (0x24=B/W, 0x26=赤チャンネル, HIGH=busy)
// GxEPD2_290_C90c を使用 (HIGH=busy, 0x22/0x20 で更新, 0x26 がカラーチャンネル)
// 注意: full_refresh_time=27000ms は BUSY=-1 時の上限値。実ピン使用時は BUSY LOW で抜ける。
#include <GxEPD2_3C.h>
#include <epd3c/GxEPD2_290_C90c.h>

using EpdDriver = GxEPD2_290_C90c;
constexpr uint16_t kEpdHeight = EpdDriver::HEIGHT;
