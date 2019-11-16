#ifndef COMMON_H
#define COMMON_H

enum SleepMode {
  DELAY,
  SLEEP
};

enum RunMode {
  FULL,      // Renders (slowly) on ePaper.
  CALIBRATE  // Renders (quickly) on Serial console only.
};

struct Plant {
  const char* name;
  const int pin;
};

struct PlantReading {
  // A value in the range [0, 100] indicating the soil moisture percentage.
  // High values mean more moisture in the soil.
  float moisture_pct;
  // Indicates the plant needs watering.
  bool needs_water;
};

struct EnvironmentReading {
  float temperature;
  float humidity;
};

#endif  // COMMON_H
