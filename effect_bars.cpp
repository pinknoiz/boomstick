#include "effect_bars.h"

EffectBars::EffectBars() {
  bgColorBase = CRGB(0, 0, 0);
  bgColorBase.setHue(BACKGROUND_COLOR);
}

void EffectBars::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  uint8_t idx;

  int barLevel = (int)(transformedLevel * (double)LAST_PIXEL);

  if (barLevel < 0) {
    barLevel = -1;
  }
  if (barLevel > LAST_PIXEL) {
    barLevel = LAST_PIXEL;
  }

  if (barLevel > peak) {
    peak = barLevel; // Keep dot on top
  }

  // Draw background
  if (barLevel <= BACKGROUND_CUTOFF && bgLevel < BACKGROUND_MAX) {
    bgLevel += BACKGROUND_INCREASE;
  }
  else if (barLevel > BACKGROUND_CUTOFF && bgLevel > 0) {
    bgLevel -= BACKGROUND_DECREASE;
  }
  bgLevel = max(0, min(bgLevel, BACKGROUND_MAX));

  CRGB bgColor;
  if (bgLevel > 0) {
    bgColor = CRGB(bgColorBase);
    bgColor.nscale8_video((int)(255.0 * bgLevel));
  }
  else {
    bgColor = CRGB(0, 0, 0);
  }

  // Ensure we get no bar at times - makes the output more lively
  if (smoothedLevel < historicLevel * 0.4) {
    barLevel = -1;
  }
  if (barLevel < LAST_PIXEL) {
    lights->pixels()(barLevel+1, LAST_PIXEL).fill_solid(bgColor);
  }

  // Draw bar
  int volumeEffect = (((BAR_COL_RANGE - BAR_COL_VAR)/2) * (double)smoothedLevel * HISTORIC_SCALE / historicLevel) + (BAR_COL_VAR/2);

  if (volumeEffect > (BAR_COL_RANGE - BAR_COL_VAR/2)) {
    volumeEffect = (BAR_COL_RANGE - BAR_COL_VAR/2);
  }
  if (volumeEffect < (BAR_COL_VAR / 2)) {
    volumeEffect = (BAR_COL_VAR / 2);
  }

  if (barLevel >= 0) {
    uint8_t rainbowStart = volumeEffect + BAR_COL_LOW;
    uint8_t rainbowDelta = map(barLevel, 0, N_PIXELS - 1, 0, BAR_COL_VAR) / barLevel;
  #ifdef BAR_COL_INVERT
    rainbowStart = BAR_COL_HIGH - rainbowStart + BAR_COL_LOW;
    rainbowDelta = -rainbowDelta;
  #endif

    lights->pixels()(0, barLevel).fill_rainbow(rainbowStart, rainbowDelta);
  }

  // Draw peak dot
  if (peak > 2) {
    uint8_t pixelHue = volumeEffect + map(peak, 0, N_PIXELS - 1, 0, BAR_COL_VAR) + BAR_COL_LOW;
#ifdef BAR_COL_INVERT
    pixelHue = BAR_COL_HIGH - pixelHue + BAR_COL_LOW;
#endif
    CRGB color = CHSV(pixelHue, 255, 255);

    uint8_t peakI = (uint8_t)peak;

    for (short offset = -PEAK_RADIUS; offset <= PEAK_RADIUS; offset++) {
      idx = peakI + offset;
      if (idx <= barLevel || idx >= N_PIXELS) continue;

      fract8 intensity = 255 / (abs(offset) + 1);
      lights->setPixel(idx, lights->pixel(idx).lerp8(color, intensity));
    }
  }

  // Drop the peak by its fall rate
  peak -= PEAK_FALL_RATE;
}
