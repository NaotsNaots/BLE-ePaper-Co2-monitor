#include "EnvBleService.h"
#include <cstring>

namespace {

BLEUuid uuid128(const char* s) { return BLEUuid(s); }

constexpr char kSvc[] = "11111101-2222-4333-8444-A55555550001";
constexpr char kChrVoc[] = "11111101-2222-4333-8444-A55555551101";
constexpr char kChrEco2[] = "11111101-2222-4333-8444-A55555551102";
constexpr char kChrSeq[] = "11111101-2222-4333-8444-A55555551103";
constexpr char kChrFw[] = "11111101-2222-4333-8444-A55555551104";

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

  setupChr(chrVoc_, kChrVoc, 2, true, true);
  setupChr(chrEco2_, kChrEco2, 2, true, true);
  setupChr(chrSeq_, kChrSeq, 4, true, true);
  setupChr(chrFw_, kChrFw, 16, true, false);

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
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(0);
  Bluefruit.Advertising.start(0);
}

void EnvBleService::stopAdvertising() { Bluefruit.Advertising.stop(); }

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
