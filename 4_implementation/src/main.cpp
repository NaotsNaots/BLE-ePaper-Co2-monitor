#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "app/ApplicationController.h"

namespace {
ApplicationController app;
}

void setup() { app.setup(); }

void loop() { app.loop(); }
