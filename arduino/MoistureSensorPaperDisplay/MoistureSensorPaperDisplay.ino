// Adafruit SleepyDog Library - Version: Latest
#include <Adafruit_SleepyDog.h>

#include "dht_nonblocking.h"
#include "SparkFun_ePaper_750.h"

#include "common.h"
#include "constants.h"

// GLOBALS
// A temperature and humidity sensor.
DHT_nonblocking dht_sensor(hal_dht::kSensorPin, hal_dht::kSensorType);
// A 640x385 ePaper display.
EPAPER_750 display;

// UTILITIES
void sleep(SleepMode mode, long int for_ms) {
  static constexpr long int MAX_SLEEP_MS = 8000L;
  if (mode == DELAY) {
    delay(for_ms);
    return;
  }
  // Sleep for at least time_ms, accounting for the fact that the watchdog 
  // may not carry out our orders exactly.
  while (for_ms > 0) {
    Serial.print(for_ms);
    Serial.println(" millis yet to sleep");
    Serial.flush();
    for_ms -= Watchdog.sleep(static_cast<int>(for_ms > MAX_SLEEP_MS ? MAX_SLEEP_MS : for_ms));
  }
}

// Indicates if plant has a pin assigned and last measurement indicated that it needs water.
bool needs_water(const PlantReading& reading, const int plant_idx) {
  return kPlantInfo[plant_idx].pin != -1 && reading.needs_water;
}

float pct_between(const int value, const int low, const int high) {
  return max(min(100.0f * static_cast<float>(value - low) / (high - low), 100.0f), 0.0f);
}

// MEASURING
void read_environment(EnvironmentReading* reading) {
  while (!dht_sensor.measure(&reading->temperature, &reading->humidity)) {}
  Serial.print("T = ");
  Serial.print(reading->temperature, 1);
  Serial.print(" deg. C, H = ");
  Serial.print(reading->humidity, 1);
  Serial.println("%");
}

void read_plants(PlantReading* readings/*[kNumPlants]*/) {
  Serial.println("W = [");
  for (int i = 0; i < kNumPlants; ++i) {
    if (kPlantInfo[i].pin >= 0) {
      const int raw_reading = analogRead(kPlantInfo[i].pin);
      Serial.print("  R = ");
      Serial.print(raw_reading);
      PlantReading& reading = readings[i];
      // We compute the % along the range max to min since readings are inverted.
      reading.moisture_pct = pct_between(raw_reading, kMaxMoistureReading, kMinMoistureReading);
      reading.needs_water = reading.moisture_pct < kNeedsWateringBelowPct;
      Serial.print(" (");
      Serial.print(reading.moisture_pct, 1);
      Serial.println("%)");
    }
  }
  Serial.println("]");
}

// RENDERING
void render_environment(const EnvironmentReading& reading, const int start_x, const int start_y, const int end_x, const int end_y) {
  display.rectangle(start_x, start_y, end_x, end_y, BLACK, /*fill=*/ false);
  display.setTextCursor(start_x + 10, start_y + 10);
  display.print("T = ");
  display.print(reading.temperature, 1);
  display.print("C");
  display.setTextCursor(end_x - 60, start_y + 10);
  display.print("H = ");
  display.print(reading.humidity, 1);
  display.print("%");
}


void render_plant(const PlantReading& reading, const int plant_idx, const int start_x, const int start_y, const int end_x, const int end_y) {
  const int center_x = start_x + 15;
  const int center_y = start_y + hal_epaper::kBoxHeight / 2;
  epaper_color_t primary_color = BLACK;
  bool alert = needs_water(reading, plant_idx);
  if (alert) {
    primary_color = RED;
  } 
  display.rectangle(start_x, start_y, end_x, end_y, primary_color, /*fill=*/ false);
  const int text_y = start_y + hal_epaper::kBoxHeight / 2 - 3;
  {
    const int chars = strlen(kPlantInfo[plant_idx].name);
    const int text_x = start_x + hal_epaper::kBoxWidth / 2 - (5*chars) / 2;
    display.setTextCursor(text_x, text_y);
    display.print(kPlantInfo[plant_idx].name);
  }
  
  // Only reflect moisture status if we can expect a valid reading.
  if (kPlantInfo[plant_idx].pin == -1) return;

  display.rectangle(start_x, start_y, start_x + 30, end_y, primary_color, /*fill=*/ true);
  display.circle(center_x, center_y, 10, WHITE, /*fill=*/ true);
  display.circle(center_x - 4, center_y - 2, 1, primary_color, /*fill=*/ true);
  display.circle(center_x + 4, center_y - 2, 1, primary_color, /*fill=*/ true);
  if (alert) {
    display.line(center_x - 6, center_y + 6, center_x, center_y + 4, primary_color);
    display.line(center_x, center_y + 4, center_x + 6, center_y + 6, primary_color);
  } else {
    display.line(center_x - 6, center_y + 4, center_x, center_y + 6, primary_color);
    display.line(center_x, center_y + 6, center_x + 6, center_y + 4, primary_color);
  }
  {
    const int display_pct = static_cast<int>(round(reading.moisture_pct));
    const int chars = ((display_pct > 99) ? 3 : ((display_pct > 9) ? 2 : 1)) + 3;
    display.setTextCursor(start_x + hal_epaper::kBoxWidth - (5*chars), text_y);
    display.print(display_pct);
    display.print("%");
  }
}

void render_readings(const EnvironmentReading& environment, const PlantReading* const plants) {
  unsigned long start_time = millis();
  
  // Reset display.
  display.powerOn();
  display.fillScreen(WHITE);
  
  // Display general information.
  render_environment(environment, 240, 350, 400, 377);
  
  // Display plant-specific information.
  for (int i = 0; i < kNumPlants; ++i) {
    int start_x = hal_epaper::kWidth / 2 - hal_epaper::kBoxWidth / 2;
    int start_y = (i + 1) * hal_epaper::kBoxOffset + i * hal_epaper::kBoxHeight;
    int end_x = start_x + hal_epaper::kBoxWidth;
    int end_y = start_y + hal_epaper::kBoxHeight;
    render_plant(plants[i], i, start_x, start_y, end_x, end_y);
  }
  
  display.updateDisplay();
  display.powerOff();
  Serial.print(millis() - start_time, 1);
  Serial.println(" millis to render");
}

// ARDUINO
void setup() {
  for (int i = 0; i < kNumPlants - 1; ++i) {
    if (kPlantInfo[i].pin >= 0) {
      pinMode(kPlantInfo[i].pin, INPUT);
    }
  }
  Serial.begin(115200);

  display.begin(
      hal_epaper::kBusyPin, hal_epaper::kResetPin, hal_epaper::kSdCSPin,
      hal_epaper::kSrCSPin, hal_epaper::kdCSPin, hal_epaper::kdcPin);
}

void loop() {
  static EnvironmentReading environment;
  static PlantReading plants[kNumPlants];

  sleep(DELAY, kWarmupTimeMs);
  read_environment(&environment);
  read_plants(plants);
  sleep(DELAY, kCooldownTimeMs);

  if (kRunMode == FULL) {
    render_readings(environment, plants);
    sleep(SLEEP, kRefreshIntervalMs);
  }
}
