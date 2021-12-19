#pragma once

#include <Arduino.h>

#define HZ_METER_ASUKIAAA_VERSION_MAJOR 1
#define HZ_METER_ASUKIAAA_VERSION_MINOR 1
#define HZ_METER_ASUKIAAA_VERSION_PATCH 0

namespace HzMeter_asukiaaa {

const int DEFAULT_HISTORY_LENGTH = 2;

class CountInfo {
 public:
  unsigned long firstAt = 0;
  unsigned long lastAt = 0;
  unsigned long measuredFrom = 0;
  unsigned long measuredTill = 0;
  int counts = 0;

  void prepend(CountInfo info) {
    if (info.counts > 0) {
      firstAt = info.firstAt;
      if (counts == 0) {
        lastAt = info.lastAt;
      }
    }
    measuredFrom = info.measuredFrom;
    counts += info.counts;
  }

  void append(CountInfo info) {
    if (info.counts > 0) {
      lastAt = info.lastAt;
      if (counts == 0) {
        firstAt = info.firstAt;
      }
    }
    measuredTill = info.measuredTill;
    counts += info.counts;
  }

  void begin() {
    counts = firstAt = lastAt = 0;
    measuredFrom = measuredTill = millis();
  }

  void end() { measuredTill = millis(); }

  float calcHzByFirstAndLast() const {
    if (counts <= 0 || counts == 1) return 0;
    return (double)(counts - 1) * 1000 / (float)(lastAt - firstAt);
  }

  float calcHzByMeasuredFrom(float targetMeasuredTill = 0) const {
    if (counts <= 0) return 0;
    if (targetMeasuredTill == 0) {
      targetMeasuredTill = measuredTill;
    }
    return (double)counts * 1000 / (float)(measuredFrom - targetMeasuredTill);
  }

  void countUp() {
    lastAt = millis();
    if (counts == 0) {
      firstAt = lastAt;
    }
    ++counts;
  }
};

class Counter {
 public:
  CountInfo info;
  void begin() { info.begin(); }

  void popAndReset(CountInfo* pInfo) {
    info.end();
    *pInfo = info;
    info.begin();
  }

  void countUp() { info.countUp(); }
};

class CountInfoBundler {
 public:
  const int historyLength;

  CountInfoBundler(int historyLength = DEFAULT_HISTORY_LENGTH)
      : historyLength(historyLength) {
    this->infoArr = new CountInfo[historyLength];
  }

  ~CountInfoBundler() { delete[] infoArr; }

  void begin() {
    for (int i = 0; i < historyLength; ++i) {
      infoArr[i].begin();
    }
  }

  void pushCountInfo(CountInfo info) {
    if (historyCurrentIndex >= historyLength - 1) {
      historyCurrentIndex = 0;
    } else {
      ++historyCurrentIndex;
    }
    infoArr[historyCurrentIndex] = info;
  }

  void bundle(CountInfo* resultInfo) {
    int targetIndex = historyCurrentIndex;
    CountInfo bundledInfo = infoArr[targetIndex];
    for (int i = 0; i < historyLength - 1; ++i) {
      if (targetIndex == 0) {
        targetIndex = historyLength;
      }
      --targetIndex;
      bundledInfo.prepend(infoArr[targetIndex]);
    }
    *resultInfo = bundledInfo;
  }

 private:
  CountInfo* infoArr;
  int historyCurrentIndex = 0;
};

class Core {
 public:
  Counter* counter;

  Core(const int historyLength = DEFAULT_HISTORY_LENGTH) {
    counter = new Counter();
    infoBundler = new CountInfoBundler(historyLength);
  }

  ~Core() {
    delete counter;
    delete infoBundler;
  }

  void begin() { counter->begin(); }
  void countUp() { counter->countUp(); }

  void onInterval() {
    CountInfo info;
    counter->popAndReset(&info);
    infoBundler->pushCountInfo(info);
    infoBundler->bundle(&infoBundled);
  }

  // float getHz() { return infoBundled.calcHzByFirstAndLast(); }
  // float getHzMeasuredAt() { return infoBundled.measuredTill; }

  const CountInfo* const pInfoBundledP() { return &infoBundled; }
  CountInfo const getInfoBundled() { return infoBundled; }

 private:
  CountInfoBundler* infoBundler;
  CountInfo infoBundled;
};

}  // namespace HzMeter_asukiaaa
