#include <HzMeter_asukiaaa.hpp>

#define PIN_INTERRUPT_HZ_METER 3
#if digitalPinToInterrupt(PIN_INTERRUPT_HZ_METER) < 0
#error needed to assign interrupt pin for PIN_INTERRUPT_HZ_METER
#endif

HzMeter_asukiaaa::Core hzMeter(PIN_INTERRUPT_HZ_METER);

// #define HISTORY_LENGTH 5
// HzMeter_asukiaaa::Core hzMeter(PIN_INTERRUPT_HZ_METER, HISTORY_LENGTH);

void setup() {
  Serial.begin(115200);
  hzMeter.counter->begin([]() { hzMeter.counter->countUp(); });
}

void loop() {
  hzMeter.onInterval();
  auto countInfo = hzMeter.getInfoBundled();
  Serial.print(countInfo.calcHzByFirstAndLast());
  Serial.print("Hz from ");
  Serial.print(countInfo.measuredFrom);
  Serial.print(" to ");
  Serial.println(countInfo.measuredTill);
  delay(1000);
}