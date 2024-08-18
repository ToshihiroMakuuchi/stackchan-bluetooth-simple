/*-------------------------------------------------------------------------
  Arduino library to provide a variety of effects when using Adafruit's
  NeoPixel library along with NeoPixels and other compatible hardware.
  This library is a work in progress and it's main purpose to help get
  my coding back on track after a long absence. Wish me luck!
  Written by Nolan Moore.
  -------------------------------------------------------------------------
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
  -------------------------------------------------------------------------*/

#include <NeoPixelEffects.h>

NeoPixelEffects::NeoPixelEffects(CRGB *ledset, EffectType effect, int pixstart, int pixend, int aoe, unsigned long delay, CRGB color_crgb, bool repeat, bool dir) :
  _pixset(ledset), _color_fg(color_crgb), _color_bg(CRGB::Black), _repeat(repeat), _direction(dir), _counter(0)
{
  setRange(pixstart, pixend);
  setAreaOfEffect(aoe);
  setDelay(delay);
  setEffect(effect);
  // *assoc_effects = NULL;
}

NeoPixelEffects::NeoPixelEffects()
{
  // *_pixset = NULL;
  *_pixset = CRGB();
  _effect = NONE;
  _status = INACTIVE;
  _pixstart = 0;
  _pixend = 0;
  _pixrange = 1;
  _pixaoe = 1;
  _pixcurrent = _pixstart;
  _counter = 0;
  _delay = 0;
  _lastupdate = 0;
  _color_fg = CRGB::Black;
  _color_bg = CRGB::Black;
  _repeat = true;
  _direction = FORWARD;
  fadeValue = 0;                 // 2023-11-08 追加
  effectFinished = true;         // 2023-11-08 追加
}

NeoPixelEffects::~NeoPixelEffects()
{
  // *_pixset = NULL;
  *_pixset = CRGB();
}

void NeoPixelEffects::setEffect(EffectType effect)
{
  _effect = effect;
  if (_direction == FORWARD) {
    _pixcurrent = _pixstart;
    _counter = 0;
  } else {
    _pixcurrent = _pixend;
    _counter = 100;
  }
  _lastupdate = 0;
  _status = ACTIVE;
}

void NeoPixelEffects::update()
{
  if (_status == ACTIVE) {
    unsigned long now = millis();
    if (now - _lastupdate > _delay) {
      _lastupdate = now;
      switch (_effect) {
        case RAINBOWWAVE:
          updateRainbowWaveEffect();
          break;
        case COMET:
          updateCometEffect(0);
          break;
        case LARSON:
          updateCometEffect(1);
          break;
        case CHASE:
          updateChaseEffect();
          break;
        case STATIC:
          updateStaticEffect(0);
          break;
        case RANDOM:
          updateStaticEffect(1);
          break;        
        case STROBE:
          updateStrobeEffect();
          break;
        case SINEWAVE:
          updateWaveEffect(0);
          break;
        case FADEINOUT:                   // 2023-11-08 追加
          updateFadeInOutEffect();        // 2023-11-08 追加
          break;                          // 2023-11-08 追加
        case NANAIRO:                     // 2023-11-08 追加
          updateNanairoEffect();          // 2023-11-08 追加
          break;                          // 2023-11-08 追加
        case MERAMERA:                    // 2023-11-13 変更
          updateMerameraEffect();         // 2023-11-13 変更
          break;                          // 2023-11-13 変更
        default:
          break;
      }
    }
  }
}

void NeoPixelEffects::updateCometEffect(int subtype)
{
  if (subtype > 0) {
    if (_repeat) _repeat = false;
  }

  for (int j = 0; j <= _pixaoe; j++) {
    int tpx;
    bool showpix = true;
    if (_direction == FORWARD) {
      tpx = _pixcurrent + j;
      if (tpx > _pixend) {
        if (_repeat) {
          tpx = _pixcurrent + j - _pixrange + 1;
        } else {
          showpix = false;
        }
      }
    } else {
      tpx = _pixcurrent - j;
      if (tpx < _pixstart) {
        if (_repeat) {
          tpx = _pixcurrent - j + _pixrange;
        } else {
          showpix = false;
        }
      }
    }

    if (showpix) {
      float ratio = j / (float)_pixaoe;
      CRGB tailcolor = CRGB(_color_fg.r * ratio, _color_fg.g * ratio, _color_fg.b * ratio);

      _pixset[tpx] = tailcolor;
    }
  }

  if (_direction == FORWARD) {
    if (_pixcurrent == _pixend) {
      if (_repeat) {
        _pixcurrent = _pixstart;
      } else {
        if (subtype > 0) {
          _direction = REVERSE;
        } else {
          stop();
        }
      }
    } else {
      _pixcurrent++;
    }
  } else {
    if (_pixcurrent == _pixstart) {
      if (_repeat) {
        _pixcurrent = _pixend;
      } else {
        if (subtype > 0) {
          _direction = FORWARD;
        } else {
          stop();
        }
      }
    } else {
      _pixcurrent--;
    }
  }
}

void NeoPixelEffects::updateChaseEffect()
{
  for (int j = _pixstart; j <= _pixend; j++) {
    if (_counter % 2 == 0) {
      if (j % 2 == 0) {
        _pixset[j] = _color_fg;
      } else {
        _pixset[j] = _color_bg;
      }
    } else {
      if (j % 2 == 0) {
        _pixset[j] = _color_bg;
      } else {
        _pixset[j] = _color_fg;
      }
    }
  }
  _counter++;
}

void NeoPixelEffects::updateStrobeEffect()
{
  CRGB strobecolor;
  if (_counter % 2 == 0) {
    strobecolor = _color_fg;
  } else {
    strobecolor = _color_bg;
  }

  for (int j = _pixstart; j <= _pixend; j++) {
    _pixset[j] = strobecolor;
  }
  _counter++;
}

void NeoPixelEffects::updateStaticEffect(int subtype)
{
  random16_add_entropy(random(65535));

  for (int i = _pixstart; i <= _pixend; i++) {
    CRGB randomcolor;
    if (subtype == 0) {
      float random_ratio = random8(101) / 100.0;
      randomcolor.r = _color_fg.r * random_ratio;
      randomcolor.g = _color_fg.g * random_ratio;
      randomcolor.b = _color_fg.b * random_ratio;
    } else {
      randomcolor.r = 255 * random8(101) / 100.0;
      randomcolor.g = 255 * random8(101) / 100.0;
      randomcolor.b = 255 * random8(101) / 100.0;
    }
    _pixset[i] = randomcolor;
  }
}

void NeoPixelEffects::updateRainbowWaveEffect()
{
  float ratio = 255.0  / _pixrange;

  for (int i = _pixstart; i <= _pixend; i++) {
    CRGB color = CHSV((uint8_t)((_counter + i) * ratio), 255, 255);
    _pixset[i] = color;
  }
  _counter = (_direction) ? _counter + 1 : _counter - 1;
}

void NeoPixelEffects::updateWaveEffect(int subtype)
{
  for (int i = _pixstart; i <= _pixend; i++) {
    float ratio;
    if (!subtype) {
      ratio = cubicwave8((255 * (i - _pixstart) / _pixrange) + _counter) / 255.0;
    } else {
      ratio = triwave8((255 * (i - _pixstart) / _pixrange) + _counter) / 255.0;
    }

    CRGB wavecolor = CRGB(_color_fg.r * ratio, _color_fg.g * ratio, _color_fg.b * ratio);
    _pixset[i] = wavecolor;
  }
  _counter = (_direction) ? _counter + 2 : _counter - 2;
}

// ---------------------------- // 2023-11-08 追加
//                              // フェードのスピードを調整する変数
int fadeIncrement = 7;          // スピードを遅くするにはこの値を小さく、速くするには大きくします。

void NeoPixelEffects::updateFadeInOutEffect() {
    if (effectFinished) {                     // エフェクトが完了していれば初期化
        fadeValue = 0;
        effectFinished = false;
    }

    // Fade in
    if (fadeValue < 255) {
        float r = (fadeValue / 255.0) * _color_fg.r;
        float g = (fadeValue / 255.0) * _color_fg.g;
        float b = (fadeValue / 255.0) * _color_fg.b;
        setAll(r, g, b);                     // 計算した値でLEDを設定
        showStrip();
        fadeValue += fadeIncrement;          // フェード速度を調整
        return;                              // このステップで制御を戻す
    }

    // Fade out
    if (fadeValue >= 255) {
        int newFadeValue = 510 - fadeValue; // 255から0まで減算
        if (newFadeValue < 0) {             // エフェクトが完了
            effectFinished = true;
            return;
        }
        float r = (newFadeValue / 255.0) * _color_fg.r;
        float g = (newFadeValue / 255.0) * _color_fg.g;
        float b = (newFadeValue / 255.0) * _color_fg.b;
        setAll(r, g, b);                    // 計算した値でLEDを設定
        showStrip();
        fadeValue += fadeIncrement;         // フェード速度を調整
        return;                             // このステップで制御を戻す
    }
}

void NeoPixelEffects::updateNanairoEffect() {
    static int colorIndex = 0;              // 現在の色のインデックス
    static CRGB colors[] = {
        CRGB::Red,
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Green,
        CRGB::Blue,
        CRGB::Indigo,
        CRGB::Violet
    }; // 七色の配列

    if (effectFinished) {                    // エフェクトが完了していれば初期化
        fadeValue = 0;
        effectFinished = false;
        // 次の色に移動
        colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
        _color_fg = colors[colorIndex];      // 次の色を設定
    }

    // フェードイン
    if (fadeValue < 255) {
        float r = (fadeValue / 255.0) * _color_fg.r;
        float g = (fadeValue / 255.0) * _color_fg.g;
        float b = (fadeValue / 255.0) * _color_fg.b;
        setAll(r, g, b);                    // 計算した値でLEDを設定
        showStrip();
        fadeValue += fadeIncrement;         // フェード速度を調整
        return;                             // このステップで制御を戻す
    }

    // フェードアウト
    if (fadeValue >= 255) {
        int newFadeValue = 510 - fadeValue; // 255から0まで減算
        if (newFadeValue < 0) {             // エフェクトが完了
            effectFinished = true;
            return;
        }
        float r = (newFadeValue / 255.0) * _color_fg.r;
        float g = (newFadeValue / 255.0) * _color_fg.g;
        float b = (newFadeValue / 255.0) * _color_fg.b;
        setAll(r, g, b);                    // 計算した値でLEDを設定
        showStrip();
        fadeValue += fadeIncrement;         // フェード速度を調整
        return;                             // このステップで制御を戻す
    }
}

void NeoPixelEffects::updateMerameraEffect() {
  // エフェクトがアクティブでない場合は何もしない
  if (_status != ACTIVE) return;

  // 火のエフェクト用のバッファを初期化
  static byte heat[256];

  // 炎を下から上に移動させる
  for (int i = _pixend; i >= _pixstart; i--) {
    // 炎のランダムなクロールをシミュレート
    heat[i] = qsub8(heat[i + 1], random8(0, ((_pixaoe * 10) / _pixrange) + 2));
  }

  // ピクセルに熱を加える
  for (int i = _pixstart; i < _pixend; i++) {
    heat[i] = qadd8(heat[i], random8(130, 255));
  }

  // 熱をもとにピクセルの色を設定する
  for (int i = _pixstart; i < _pixend; i++) {
    // 熱を色に変換する
    CRGB color = HeatColor(heat[i]);

    // ここで色をカスタマイズする
    color.red = qadd8(color.red, 0);        // 赤色成分を強くする
    color.green = qsub8(color.green, 230);  // 緑色と青色成分を減少させる（炎の色をより赤くする）
    color.blue = qsub8(color.blue, 230);    // 緑色と青色成分を減少させる（炎の色をより赤くする）

    // 明るさの減衰をシミュレート
    color.nscale8_video(random8(90, 255));

    // ピクセルを設定
    _pixset[i] = color;
  }

  // ストリップを表示
  showStrip();
}
// ---------------------------- // 2023-11-08 追加

EffectType NeoPixelEffects::getEffect()
{
  return _effect;
}

void NeoPixelEffects::setRange(int pixstart, int pixend)
{
  if (pixstart >= 0 && pixstart <= pixend) {
    _pixstart = pixstart;
    _pixend = pixend;
    _pixrange = _pixend - _pixstart + 1;
  }
  if (_direction == FORWARD) {
    _pixcurrent = _pixstart;
  } else {
    _pixcurrent = _pixend;
  }
  _lastupdate = 0;
}

void NeoPixelEffects::setAreaOfEffect(int aoe)
{
  if (aoe > 0 && aoe <= _pixrange) {
    _pixaoe = aoe;
  }
  _lastupdate = 0;
}

void NeoPixelEffects::setDelayHz(int delay_hz)
{
  if (delay_hz > 0) {
    _delay = (unsigned long)(1.0 / delay_hz * 1000);
  }
  update();
}

void NeoPixelEffects::setDelay(unsigned long delay_ms)
{
  _delay = delay_ms;
  _lastupdate = 0;
  update();
}

void NeoPixelEffects::setColor(CRGB color_crgb)
{
  _color_fg = color_crgb;
  _lastupdate = 0;
}

void NeoPixelEffects::setBackgroundColor(CRGB color_crgb)
{
  _color_bg = color_crgb;
  _lastupdate = 0;
}

void NeoPixelEffects::setStatus(EffectStatus status)
{
  _status = status;
  if (status == ACTIVE) {
    _lastupdate = 0;
  }
}

EffectStatus NeoPixelEffects::getStatus()
{
  return(_status);
}

void NeoPixelEffects::setRepeat(bool repeat)
{
  _repeat = repeat;
  _lastupdate = 0;
}

void NeoPixelEffects::setDirection(bool direction)
{
  _direction = direction;
  _lastupdate = 0;
}

void NeoPixelEffects::stop()
{
  clear();
  setEffect(NONE);
  setStatus(INACTIVE);
}

void NeoPixelEffects::pause()
{
  if (getEffect() != NONE && getStatus() == ACTIVE) {
    setStatus(INACTIVE);
  }
}

void NeoPixelEffects::play()
{
  if (getEffect() != NONE && getStatus() != ACTIVE) {
    setStatus(ACTIVE);
  }
}

void NeoPixelEffects::clear()
{
  fill_solid(CRGB::Black);
}

// ---------------------------- // 2023-11-08 追加
// この関数は特定の範囲のLEDをクリアします。
void NeoPixelEffects::clearRange(int start, int end)
{
    for (int i = start; i <= end; i++) {
        _pixset[i] = CRGB::Black;
    }
}
// ---------------------------- // 2023-11-08 追加

void NeoPixelEffects::fill_solid(CRGB color_crgb)
{
  for (int i = _pixstart; i <= _pixend; i++) {
    _pixset[i] = color_crgb;
  }
}

void NeoPixelEffects::fill_gradient(CRGB color_crgb1, CRGB color_crgb2)
{
  int delta_red = color_crgb1.r - color_crgb2.r;
  int delta_green = color_crgb1.g - color_crgb2.g;
  int delta_blue = color_crgb1.b - color_crgb2.b;

  for (int i = _pixstart; i <= _pixend; i++) {
    float part = (float)(i - _pixstart) / _pixrange;
    uint8_t grad_red = color_crgb1.r - (delta_red * part);
    uint8_t grad_green = color_crgb1.g - (delta_green * part);
    uint8_t grad_blue = color_crgb1.b - (delta_blue * part);
    _pixset[i] = CRGB(grad_red, grad_green, grad_blue);
  }
}

// ---------------------------- // 2023-11-08 追加
void NeoPixelEffects::setAll(byte red, byte green, byte blue) {
    for(int i = _pixstart; i <= _pixend; i++) {
        _pixset[i] = CRGB(red, green, blue);
    }
}

void NeoPixelEffects::showStrip() {
    FastLED.show();
}
// ---------------------------- // 2023-11-08 追加

// ---------------------------- // 2024-08-03 追加
void NeoPixelEffects::setParameters(unsigned long delay, CRGB color_crgb, bool dir) {
  
  setDelay(delay);
  _color_fg = color_crgb;
  _direction = dir;  
}
// ---------------------------- // 2024-08-03 追加
