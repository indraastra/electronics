#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "common.h"

namespace hal_dht {
static constexpr uint8_t kSensorType = DHT_TYPE_11;
static constexpr uint8_t kSensorPin = 2;
}

namespace hal_epaper {
static constexpr uint8_t kBusyPin = 9;
static constexpr uint8_t kResetPin = 8;
static constexpr uint8_t kSdCSPin = 7;
static constexpr uint8_t kSrCSPin = 6;
static constexpr uint8_t kdCSPin = 5;
static constexpr uint8_t kdcPin = 4;

static constexpr int kWidth = 640;
static constexpr int kHeight = 385;
static constexpr int kBoxHeight = 45;
static constexpr int kBoxWidth = 200;
static constexpr int kBoxOffset = 20;
}

// Constants to tune and tweak.
static constexpr int kWarmupTimeMs = 1000;
static constexpr int kCooldownTimeMs = 1000;
static constexpr long int kRefreshIntervalMs =  * 60 * 1000L;
// 175 is the theoretical minimum based on dipping the sensor in water.
// 275 is the practical minimum based on fully saturated succulent soil.
static constexpr int kMinMoistureReading = 275;
static constexpr int kMaxMoistureReading = 1023;
static constexpr int kNeedsWateringBelowPct = 15;

// Constants to modify per setup.
static constexpr int kNumPlants = 4;
static constexpr Plant kPlantInfo[kNumPlants] = {
  // Format: {<name>, <pin>}
  {"Shorkle", -1},
  {"Atlas", -1},
  {"Big Daddy", -1},
  {"<SUBJECT X>", A0}
};

static constexpr RunMode kRunMode = FULL;

#endif  // CONSTANTS_H
