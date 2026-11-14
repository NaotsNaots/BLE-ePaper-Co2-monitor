#include "EnvBleService.h"
#include <cstring>
#include <Arduino.h>

namespace {

BLEUuid uuid128(const char* s) { return BLEUuid(s); }

constexpr char kSvc[] = "11111101-2222-4333-8444-A55555550001";
constexpr char kChrVoc[] = "11111101-2222-4333-8444-A55555551101";
constexpr char kChrEco2[] = "11111101-2222-4333-8444-A55555551102";
constexpr char kChrSeq[] = "11111101-2222-4333-8444-A55555551103";
constexpr char kChrFw[]        = "11111101-2222-4333-8444-A55555551104";
constexpr char kChrHistCount[] = "11111101-2222-4333-8444-A55555551105";
constexpr char kChrHistData[]  = "11111101-2222-4333-8444-A55555551106";

constexpr char kFwId[] = "0.1.0";

void setupChr(BLECharacteristic& c, const char* uuidstr, uint16_t maxLen, bool read, bool notify) {
  uint8_t props = 0;
  if (read) {
    props |= CHR_PROPS_READ;
  }
  if (notify) {
    props |= CHR_PROPS_NOTIFY;
  }
  c.setUuid(uuid128(uuidstr));
  c.setProperties(props);
  c.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  c.setMaxLen(maxLen);
  c.begin();
}

} // namespace

void EnvBleService::begin(const MeasurementFrame& snap) {
  if (svcBegun_) {
    updateGattSnapshot(snap);
    return;
  }
  service_.setUuid(uuid128(kSvc));
  service_.begin();

  setupChr(chrVoc_,       kChrVoc,       2,  true,  true);
  setupChr(chrEco2_,     kChrEco2,      2,  true,  true);
  setupChr(chrSeq_,      kChrSeq,       4,  true,  true);
  setupChr(chrFw_,       kChrFw,        16, true,  false);
  setupChr(chrHistCount_,kChrHistCount, 2,  true,  false);
  setupChr(chrHistData_, kChrHistData,  9,  false, true);

  updateGattSnapshot(snap);

  uint8_t fwbuf[16] = {};
  strncpy((char*)fwbuf, kFwId, sizeof(fwbuf) - 1);
  chrFw_.write(fwbuf, sizeof(fwbuf));
  svcBegun_ = true;
}

void EnvBleService::updateGattSnapshot(const MeasurementFrame& snap) {
  if (!svcBegun_) {
    return;
  }
  int32_t voc = snap.vocIndex;
  if (voc < 0) {
    voc = 0;
  }
  if (voc > 65535) {
    voc = 65535;
  }
  uint16_t voc16 = (uint16_t)voc;
  chrVoc_.write(&voc16, sizeof(voc16));
  chrEco2_.write(&snap.eco2Ppm, sizeof(snap.eco2Ppm));
  uint32_t idx = snap.sampleIndex;
  chrSeq_.write(&idx, sizeof(idx));
}

void EnvBleService::startAdvertising(uint32_t windowMs) {
  (void)windowMs;
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(service_);
  // 広告パケット（31 バイト）は 128bit UUID で埋まり名前が "EnvEp" に切れるため、
  // 名前はスキャン応答に入れる。
  Bluefruit.ScanResponse.clearData();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(0);
  Bluefruit.Advertising.start(0);
}

void EnvBleService::stopAdvertising() { Bluefruit.Advertising.stop(); }

bool EnvBleService::notifyHistoryBurst(const MeasurementFrame* frames, uint16_t count) {
  if (!svcBegun_ || !Bluefruit.connected()) {
    return false;
  }
  // 件数を READ 特性に書き込む
  chrHistCount_.write(&count, sizeof(count));

  if (!chrHistData_.notifyEnabled()) {
    return false;  // 未購読: 購読されるまで呼び出し側が再試行する
  }
  if (count == 0) {
    return true;  // 送るものがない＝送信完了。false を返すと毎ループ再試行になり通知が連発する
  }

  // 1 フレーム = 9 バイト packed (oldest → newest)
  // [0..3] sampleIndex (uint32 LE)
  // [4..5] vocIndex    (int16  LE, clamp -32768..32767)
  // [6..7] eco2Ppm     (uint16 LE)
  // [8]    flags       bits[1:0]=vent, bits[3:2]=trend
  for (uint16_t i = 0; i < count; ++i) {
    const MeasurementFrame& f = frames[i];
    uint8_t pkt[9];
    memcpy(pkt, &f.sampleIndex, 4);
    int16_t voc16 = (f.vocIndex < -32768) ? -32768
                  : (f.vocIndex >  32767) ?  32767
                  : static_cast<int16_t>(f.vocIndex);
    memcpy(pkt + 4, &voc16, 2);
    memcpy(pkt + 6, &f.eco2Ppm, 2);
    pkt[8] = (static_cast<uint8_t>(f.vent) & 0x03)
           | ((static_cast<uint8_t>(f.trend) & 0x03) << 2);
    chrHistData_.notify(pkt, sizeof(pkt));
    delay(10); // BLE スタックの TX バッファ溢れを防ぐ
  }
  return true;
}

void EnvBleService::notifySnapshotIfPossible(const MeasurementFrame& snap) {
  if (!svcBegun_ || !Bluefruit.connected()) {
    return;
  }
  int32_t voc = snap.vocIndex;
  if (voc < 0) voc = 0;
  if (voc > 65535) voc = 65535;
  uint16_t voc16 = (uint16_t)voc;
  if (chrVoc_.notifyEnabled()) {
    chrVoc_.notify(&voc16, sizeof(voc16));
  }
  if (chrEco2_.notifyEnabled()) {
    chrEco2_.notify(&snap.eco2Ppm, sizeof(snap.eco2Ppm));
  }
  uint32_t idx = snap.sampleIndex;
  if (chrSeq_.notifyEnabled()) {
    chrSeq_.notify(&idx, sizeof(idx));
  }
}
