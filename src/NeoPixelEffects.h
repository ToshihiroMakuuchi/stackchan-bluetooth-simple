/*--------------------------------------------------------------------
  This file is part of the NeoPixel Effects library.
  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.
  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  --------------------------------------------------------------------*/

#ifndef NEOPIXELEFFECTS_H
#define NEOPIXELEFFECTS_H

#include <FastLED.h>

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#define FORWARD true
#define REVERSE false

enum EffectType {
  RAINBOWWAVE,
  COMET,
  LARSON,
  CHASE,
  STATIC,
  STROBE,
  SINEWAVE,
  RANDOM,
  FADEINOUT,                // 2023-11-08 追加
  NANAIRO,                  // 2023-11-08 追加
  MERAMERA,                 // 2023-11-13 追加
  NONE
};

enum EffectStatus {
  INACTIVE,
  ACTIVE
};

class NeoPixelEffects {
  public:
    NeoPixelEffects(CRGB *pix, EffectType effect, int pixstart, int pixend, int aoe, unsigned long delay, CRGB color_crgb, bool looping, bool dir);
    NeoPixelEffects();
    ~NeoPixelEffects();

    void setEffect(EffectType effect);  // Sets effect
    EffectType getEffect();
    void setStatus(EffectStatus status);
    EffectStatus getStatus();
    void setColor(CRGB color_crgb);
    void setBackgroundColor(CRGB color_crgb);
    void setRange(int pixstart, int pixend);
    void setAreaOfEffect(int aoe);
    void setDelay(unsigned long delay_ms);
    void setDelayHz(int delay_hz);
    void setRepeat(bool repeat);
    void setDirection(bool direction);

    void setAll(byte red, byte green, byte blue);         // 2023-11-08 追加
    void showStrip();                                     // 2023-11-08 追加
    void setParameters(unsigned long delay, CRGB color_crgb, bool dir); // 2024-08-03 追加

    void update(); // Process effect
    void stop();
    void pause();
    void play();

    void clear();
    void clearRange(int start, int end);                  // 2023-11-08 追加
	
    void fill_solid(CRGB color_crgb);
    void fill_gradient(CRGB color_crgb1, CRGB color_crgb2);

  private:
    void updateCometEffect(int subtype);
    void updateChaseEffect();
    void updateStaticEffect(int subtype);
    void updateSolidEffect();
    void updateRainbowWaveEffect();
    void updateStrobeEffect();
    void updateWaveEffect(int subtype);
    void updateFadeInOutEffect();  // 2023-11-08 追加
    void updateNanairoEffect();    // 2023-11-08 追加
    void updateMerameraEffect();   // 2023-11-13 追加

    CRGB *_pixset;          // A reference to the one created in the user code
    CRGB _color_fg;
    CRGB _color_bg;
    EffectType _effect;     // Your silly or awesome effect!
    EffectStatus _status;

    int
      fadeValue,            // フェードの値を追跡する   // 2023-11-08 追加
      _pixstart,            // First NeoPixel in range of effect
      _pixend,              // Last NeoPixel in range of effect
      _pixrange,            // Length of effect area
      _pixaoe,              // The length of the effect that takes place within the range
      _pixcurrent,          // Head pixel that indicates current pixel to base effect on
      _counter;
    uint8_t _subtype;          // Defines sub type to be used
    bool
      effectFinished,       // エフェクトが完了したかどうかを追跡する    // 2023-11-08 追加
      _repeat,              // Whether or not the effect loops in area
      _direction;           // Whether or not the effect moves from start to end pixel
    unsigned long
      _lastupdate,          // Last update time, in milliseconds since sys reboot
      _delay;               // Period at which effect should update, in milliseconds
};

#endif
