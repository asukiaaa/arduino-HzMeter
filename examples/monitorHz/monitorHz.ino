#include <HzMeter_asukiaaa.hpp>

#define PIN_INTERRUPT_HZ_METER 3
#if digitalPinToInterrupt(PIN_INTERRUPT_HZ_METER) < 0
#error needed to assign interrupt pin for PIN_INTERRUPT_HZ_METER
#endif

HzMeter_asukiaaa::Core hzMeter;

// #define HISTORY_LENGTH 5
// HzMeter_asukiaaa::Core hzMeter(HISTORY_LENGTH);

void setup() {
  Serial.begin(115200);
  hzMeter.begin();
  pinMode(PIN_INTERRUPT_HZ_METER, INPUT_PULLUP);
  attachInterrupt(
      digitalPinToInterrupt(PIN_INTERRUPT_HZ_METER),
      []() { hzMeter.countUp(); }, RISING);
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
