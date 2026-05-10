#include "SleepController.h"
#include <Arduino.h>
#include <nrf.h>
#include "app/pin_config.h"

namespace {

volatile uint8_t gRtcWake = 0;
volatile uint8_t gImuGpioWake = 0;

void imuPinIsr() { gImuGpioWake = 1; }

extern "C" void RTC2_IRQHandler(void) {
  if (NRF_RTC2->EVENTS_COMPARE[0]) {
    NRF_RTC2->EVENTS_COMPARE[0] = 0;
    (void)NRF_RTC2->EVENTS_COMPARE[0];
    gRtcWake = 1;
  }
}

void rtc2InitCompareSeconds(uint32_t sec) {
  NVIC_DisableIRQ(RTC2_IRQn);
  NRF_RTC2->TASKS_STOP = 1;
  NRF_RTC2->TASKS_CLEAR = 1;
  NRF_RTC2->EVTENCLR = 0xFFFFFFFF;
  NRF_RTC2->INTENCLR = 0xFFFFFFFF;

  constexpr uint32_t kPrescaler = 4095;
  constexpr float kHz = 32768.0f / float(kPrescaler + 1);
  NRF_RTC2->PRESCALER = kPrescaler;
  uint32_t ticks = (uint32_t)(sec * kHz + 0.5f);
  if (ticks < 2) {
    ticks = 2;
  }
  NRF_RTC2->CC[0] = ticks;

  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Msk;
  NVIC_ClearPendingIRQ(RTC2_IRQn);
  NVIC_EnableIRQ(RTC2_IRQn);
  NRF_RTC2->TASKS_START = 1;
}

} // namespace

void SleepController::configureGpioSenseWakeup(uint32_t pinNumber) {
  pinNumber_ = pinNumber;
}

WakeReason SleepController::takeLastWakeReason() {
  WakeReason w = lastWake_;
  lastWake_ = WakeReason::Unknown;
  return w;
}

void SleepController::armRtcWakeupSeconds(uint32_t sec) {
  armSeconds_ = sec;
}

void SleepController::enterSystemOffOrDeepSleep() {
  gRtcWake = 0;
  gImuGpioWake = 0;

  rtc2InitCompareSeconds(armSeconds_);

  uint32_t imuPin = (pinNumber_ != 0) ? pinNumber_ : (uint32_t)pin::kImuInt1;
  pinMode((uint32_t)imuPin, INPUT_PULLDOWN_SENSE);
  attachInterrupt(digitalPinToInterrupt((uint32_t)imuPin), imuPinIsr, RISING);

  while (gRtcWake == 0 && gImuGpioWake == 0) {
    waitForEvent();
  }

  detachInterrupt(digitalPinToInterrupt((uint32_t)imuPin));
  NVIC_DisableIRQ(RTC2_IRQn);
  NRF_RTC2->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
  NRF_RTC2->TASKS_STOP = 1;

  if (gImuGpioWake) {
    lastWake_ = WakeReason::ImuMotion;
  } else if (gRtcWake) {
    lastWake_ = WakeReason::Timer10Min;
  } else {
    lastWake_ = WakeReason::Unknown;
  }
  gRtcWake = 0;
  gImuGpioWake = 0;
}
