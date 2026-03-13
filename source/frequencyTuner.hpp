#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

enum class TuneDirection : int8_t {
  Negative = -1,
  Positive = +1
};

struct FrequencyTunerConfig {
  uint32_t dtIgnoreUs = 500U;
  uint32_t dtIdleResetUs = 300'000U;

  double tauSpeedUs = 50'000.0;
  double speedMaxTicksPerSec = 100.0;
  double speedDeadOnTicksPerSec = 8.0;
  double speedDeadOffTicksPerSec = 6.0;
  double speedFullTicksPerSec = 50.0;
  double speedGainMax = 1.6;

  double tauContinuityUs = 220'000.0;
  double continuityRisePerTick = 0.25;
  double continuityGainMax = 0.8;
  double reverseContinuityDrop = 0.15;
  double reverseSpeedDrop = 0.5;

  double speedExtraStepsMax = 20.0;
  double continuityExtraStepsMax = 6.0;
  uint32_t maxStepsPerTick = 32U;
};

class FrequencyTuner {
public:
  void init(const FrequencyTunerConfig &cfg, const double initialValueHz) {
    cfg_ = cfg;
    speedTicksPerSec_ = 0.0;
    continuity_ = 0.0;
    speedGate_ = false;
    hasLastEvent_ = false;
    lastTimestampUs_ = 0U;
    lastSign_ = 0;
    (void)initialValueHz;
  }

  uint32_t onEncoderEvent(const TuneDirection direction,
                          const uint32_t timestampUs) {
    const int sign = direction == TuneDirection::Positive ? 1 : -1;

    if (hasLastEvent_) {
      const uint32_t dtUs = timestampUs - lastTimestampUs_;

      if (dtUs >= cfg_.dtIdleResetUs) {
        speedTicksPerSec_ = 0.0;
        continuity_ = 0.0;
        speedGate_ = false;
      } else if (dtUs >= cfg_.dtIgnoreUs) {
        updateSpeed(dtUs);
        updateContinuity(sign, dtUs);
      }
    }

    updateSpeedGate();

    const double speedNorm =
        speedGate_ ? smoothstep01(normalize(speedTicksPerSec_,
                                            cfg_.speedDeadOnTicksPerSec,
                                            cfg_.speedFullTicksPerSec))
                   : 0.0;
    const double continuityNorm = continuity_ * speedNorm;
    const double extraSteps =
        (cfg_.speedExtraStepsMax * speedNorm) +
        (cfg_.continuityExtraStepsMax * continuityNorm);
    const uint32_t steps = 1U + static_cast<uint32_t>(std::lround(extraSteps));

    hasLastEvent_ = true;
    lastTimestampUs_ = timestampUs;
    lastSign_ = sign;
    return std::min(steps, cfg_.maxStepsPerTick);
  }

  double speedTicksPerSec(void) const { return speedTicksPerSec_; }
  double continuity(void) const { return continuity_; }

private:
  static double clamp01(const double x) {
    return std::clamp(x, 0.0, 1.0);
  }

  static double normalize(const double x, const double lo, const double hi) {
    if (hi <= lo) {
      return 0.0;
    }
    return clamp01((x - lo) / (hi - lo));
  }

  static double smoothstep01(double x) {
    x = clamp01(x);
    return x * x * (3.0 - 2.0 * x);
  }

  void updateSpeed(const uint32_t dtUs) {
    const double dt = static_cast<double>(dtUs);
    const double instTicksPerSec =
        std::min(1'000'000.0 / dt, cfg_.speedMaxTicksPerSec);
    const double alpha = dt / (cfg_.tauSpeedUs + dt);
    speedTicksPerSec_ += alpha * (instTicksPerSec - speedTicksPerSec_);
  }

  void updateContinuity(const int sign, const uint32_t dtUs) {
    const double decay =
        std::exp(-static_cast<double>(dtUs) / cfg_.tauContinuityUs);
    continuity_ *= decay;

    if (sign == lastSign_) {
      continuity_ += (1.0 - continuity_) * cfg_.continuityRisePerTick;
    } else {
      continuity_ *= cfg_.reverseContinuityDrop;
      speedTicksPerSec_ *= cfg_.reverseSpeedDrop;
    }
  }

  void updateSpeedGate() {
    if (speedGate_) {
      if (speedTicksPerSec_ < cfg_.speedDeadOffTicksPerSec) {
        speedGate_ = false;
      }
    } else if (speedTicksPerSec_ > cfg_.speedDeadOnTicksPerSec) {
      speedGate_ = true;
    }
  }

  FrequencyTunerConfig cfg_ {};
  double speedTicksPerSec_ = 0.0;
  double continuity_ = 0.0;
  bool speedGate_ = false;

  bool hasLastEvent_ = false;
  uint32_t lastTimestampUs_ = 0U;
  int lastSign_ = 0;
};
