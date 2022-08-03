#pragma once

#include <Arduino.h>

#define HZ_METER_ASUKIAAA_VERSION_MAJOR 1
#define HZ_METER_ASUKIAAA_VERSION_MINOR 1
#define HZ_METER_ASUKIAAA_VERSION_PATCH 4

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

  void countUp(unsigned long msIgnoreLessThan = 0) {
    if (counts != 0 && millis() - lastAt < msIgnoreLessThan) {
      return;
    }
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
  unsigned long msIgnoreLessThan = 0;
  void begin() { info.begin(); }

  void popAndReset(CountInfo* pInfo) {
    info.end();
    *pInfo = info;
    info.begin();
  }

  void countUp() { info.countUp(msIgnoreLessThan); }
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
    historyCurrentIndex = normalizeIndex(historyCurrentIndex + 1);
    infoArr[historyCurrentIndex] = info;
  }

  CountInfo bundle(int length = -1, int from = 0) const {
    if (from < 0) {
      from = 0;
    }
    if (length < 0 || length > historyLength) {
      length = historyLength;
    }
    int targetIndex = normalizeIndex(historyCurrentIndex - from);
    CountInfo bundledInfo = infoArr[targetIndex];
    for (int i = 0; i < length - 1; ++i) {
      targetIndex = normalizeIndex(targetIndex - 1);
      bundledInfo.prepend(infoArr[targetIndex]);
    }
    return bundledInfo;
  }

 private:
  CountInfo* infoArr;
  int historyCurrentIndex = 0;

  int normalizeIndex(int i) const {
    while (i < 0 || historyLength <= i) {
      if (i < 0) {
        i += historyLength;
      } else {
        i -= historyLength;
      }
    }
    return i;
  }
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
    infoBundled = infoBundler->bundle();
  }

  // float getHz() { return infoBundled.calcHzByFirstAndLast(); }
  // float getHzMeasuredAt() { return infoBundled.measuredTill; }

  const CountInfo* const getInfoBundledP() { return &infoBundled; }
  CountInfo const getInfoBundled() { return infoBundled; }
  const CountInfoBundler* const getBundlerP() { return infoBundler; }

 private:
  CountInfoBundler* infoBundler;
  CountInfo infoBundled;
};

}  // namespace HzMeter_asukiaaa
