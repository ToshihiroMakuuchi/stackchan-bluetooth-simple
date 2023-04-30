// スタックチャンをBluetoothスピーカーとして使うサンプルアプリです。
// https://github.com/m5stack/M5Unified のBluetooth_with_ESP32A2DP.inoをベースにサーボの動きを追加しています。
// Copyright (c) 2022 Takao Akaki

#include <Arduino.h>

#include <Ticker.h>
#include <SD.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "Stackchan_servo.h"
#include "BluetoothA2DPSink_M5Speaker.hpp"
#include "Avatar.h"
#include "Avatar.h"
#include "AtaruFace.h"
#include "RamFace.h"
#include "DannFace.h"
#include "DogFace.h"
#include "ToraFace.h"
#include "DoomoFace.h"
#include "PaletteColor.h"
#include <nvs.h>
#include "NeoPixelEffects.h"            // FastLED関連ライブラリ
#include "FastLED.h"                    // FastLED

using namespace m5avatar;
Avatar avatar;
//Avatar* avatar;
StackchanSERVO servo;

#include "Stackchan_system_config.h"

// --------------------
// サーボ関連の初期設定
#define START_DEGREE_VALUE_X 90         // Xサーボの初期位置（変更しないでください。） Start angle of ServoX
#define START_DEGREE_VALUE_Y 90         // Yサーボの初期位置（変更しないでください。） Start angle of ServoY

fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
StackchanSystemConfig system_config;
const char* stackchan_system_config_yaml = "/yaml/SC_Config.yaml";

bool bluetooth_mode = false; 

// --------------------
// Avatar関連の初期設定
#define LIPSYNC_LEVEL_MAX 10.0f
static float lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンクの上限初期値
float mouth_ratio = 0.0f;
bool sing_happy = true;
int BatteryLevel = -1;
// Avatar関連の設定 end
// --------------------

// --------------------
// FastLED関連の初期設定
#define DATA_PIN 25
#define NUM_LEDS 10

CRGB leds[NUM_LEDS];

NeoPixelEffects effect = NeoPixelEffects(
  leds,
  COMET,          // エフェクトの種類      effect
  0,              // エフェクト開始位置    pixstart
  9,              // エフェクト終了位置    pixend
  3,              // 点灯する範囲(COMET等) aoe
  50,             // エフェクトの間隔      delay_ms
  CRGB::Green,    // 色(FastLED 指定色)    color_crgb
  true,           // ループするかどうか？  looping
  FORWARD         // エフェクトの方向      direction
);
// 詳細はこちら: https://github.com/nolanmoore/NeoPixelEffects
// FastLED関連の設定 end
// --------------------

uint32_t last_discharge_time = 0;  // USB給電が止まったときの時間(msec)

/// set M5Speaker virtual channel (0-7)
static constexpr uint8_t m5spk_virtual_channel = 0;

static constexpr size_t WAVE_SIZE = 320;
static BluetoothA2DPSink_M5Speaker a2dp_sink = { &M5.Speaker, m5spk_virtual_channel };
static fft_t fft;
static bool fft_enabled = false;
static bool wave_enabled = false;
static uint16_t prev_y[(FFT_SIZE / 2)+1];
static uint16_t peak_y[(FFT_SIZE / 2)+1];
static int16_t wave_y[WAVE_SIZE];
static int16_t wave_h[WAVE_SIZE];
static int16_t raw_data[WAVE_SIZE * 2];
static int header_height = 0;

static int px;  // draw volume bar
static int prev_x[2];
static int peak_x[2];
    

uint32_t bgcolor(LGFX_Device* gfx, int y)
{
  auto h = gfx->height()/4;
  auto dh = h - header_height;
  int v = ((h - y)<<5) / dh;
  if (dh > 44)
  {
    int v2 = ((h - y - 1)<<5) / dh;
    if ((v >> 2) != (v2 >> 2))
    {
      return 0x666666u;
    }
  }
  return gfx->color888(v + 2, v, v + 6);
}

void gfxSetup(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }
  if (gfx->width() < gfx->height())
  {
    gfx->setRotation(gfx->getRotation()^1);
  }
  gfx->setFont(&fonts::lgfxJapanGothic_12);
  gfx->setEpdMode(epd_mode_t::epd_fastest);
  gfx->setCursor(0, 8);
  gfx->print("BT A2DP : ");
//  gfx->println(bt_device_name);
  gfx->println(system_config.getBluetoothSetting()->device_name);
  gfx->setTextWrap(false);
  gfx->fillRect(0, 6, gfx->width(), 2, TFT_BLACK);

//  header_height = (gfx->height() > 80) ? 45 : 21;
  header_height = (gfx->height() > 80) ? 33 : 21;
  fft_enabled = !gfx->isEPD();
  if (fft_enabled)
  {
    wave_enabled = (gfx->getBoard() != m5gfx::board_M5UnitLCD);

    for (int y = header_height; y < gfx->height()/4; ++y)
    {
      gfx->drawFastHLine(0, y, gfx->width(), bgcolor(gfx, y));
    }
  }

  for (int x = 0; x < (FFT_SIZE/2)+1; ++x)
  {
//    prev_y[x] = INT16_MAX;
    prev_y[x] = gfx->height()/4;
    peak_y[x] = INT16_MAX;
  }
  for (int x = 0; x < WAVE_SIZE; ++x)
  {
    wave_y[x] = gfx->height()/4;
    wave_h[x] = 0;
  }

  px = 0;  // draw volume bar
  prev_x[0] = prev_x[1] = 0;
  peak_x[0] = peak_x[1] = 0;

}

void gfxLoop(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }
  if (header_height > 32)
  {
    auto bits = a2dp_sink.getMetaUpdateInfo();
    if (bits)
    {
      gfx->startWrite();
      for (int id = 0; id < a2dp_sink.metatext_num; ++id)
      {
        if (0 == (bits & (1<<id))) { continue; }
        size_t y = id * 12;
        if (y+12 >= header_height) { continue; }
        gfx->setCursor(4, 8 + y);
        gfx->fillRect(0, 8 + y, gfx->width(), 12, gfx->getBaseColor());
        gfx->print(a2dp_sink.getMetaData(id));
        gfx->print(" "); // Garbage data removal when UTF8 characters are broken in the middle.
      }
      gfx->display();
      gfx->endWrite();
    }
  }
  else
  {
    static int title_x;
    static int title_id;
    static int wait = INT16_MAX;

    if (a2dp_sink.getMetaUpdateInfo())
    {
      gfx->fillRect(0, 8, gfx->width(), 12, TFT_BLACK);
      a2dp_sink.clearMetaUpdateInfo();
      title_x = 4;
      title_id = 0;
      wait = 0;
    }

    if (--wait < 0)
    {
      int tx = title_x;
      int tid = title_id;
      wait = 3;
      gfx->startWrite();
      uint_fast8_t no_data_bits = 0;
      do
      {
        if (tx == 4) { wait = 255; }
        gfx->setCursor(tx, 8);
        const char* meta = a2dp_sink.getMetaData(tid, false);
        if (meta[0] != 0)
        {
          gfx->print(meta);
          gfx->print("  /  ");
          tx = gfx->getCursorX();
          if (++tid == a2dp_sink.metatext_num) { tid = 0; }
          if (tx <= 4)
          {
            title_x = tx;
            title_id = tid;
          }
        }
        else
        {
          if ((no_data_bits |= 1 << tid) == ((1 << a2dp_sink.metatext_num) - 1))
          {
            break;
          }
          if (++tid == a2dp_sink.metatext_num) { tid = 0; }
        }
      } while (tx < gfx->width());
      --title_x;
      gfx->display();
      gfx->endWrite();
    }
  }
  if (!gfx->displayBusy())
  { // draw volume bar
//    static int px;
    uint8_t v = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    int x = v * (gfx->width()) >> 8;
    if (px != x)
    {
      gfx->fillRect(x, 6, px - x, 2, px < x ? 0xAAFFAAu : 0u);
      gfx->display();
      px = x;
    }
  }

  if (fft_enabled && !gfx->displayBusy())
  {
    // static int prev_x[2];
    // static int peak_x[2];
    static bool prev_conn;
    bool connected = a2dp_sink.is_connected();
    if (prev_conn != connected)
    {
      prev_conn = connected;
      if (!connected)
      {
        a2dp_sink.clear();
      }
    }

    auto buf = a2dp_sink.getBuffer();
    if (buf)
    {
      memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t)); // stereo data copy
      gfx->startWrite();

      // draw stereo level meter
      for (size_t i = 0; i < 2; ++i)
      {
        int32_t level = 0;
        for (size_t j = i; j < 640; j += 32)
        {
          uint32_t lv = abs(raw_data[j]);
          if (level < lv) { level = lv; }
        }

        int32_t x = (level * gfx->width()) / INT16_MAX;
        int32_t px = prev_x[i];
        if (px != x)
        {
          gfx->fillRect(x, i * 3, px - x, 2, px < x ? 0xFF9900u : 0x330000u);
          prev_x[i] = x;
        }
        px = peak_x[i];
        if (px > x)
        {
          gfx->writeFastVLine(px, i * 3, 2, TFT_BLACK);
          px--;
        }
        else
        {
          px = x;
        }
        if (peak_x[i] != px)
        {
          peak_x[i] = px;
          gfx->writeFastVLine(px, i * 3, 2, TFT_WHITE);
        }
      }
      gfx->display();

      // draw FFT level meter
      fft.exec(raw_data);
      size_t bw = gfx->width() / 60;
      if (bw < 3) { bw = 3; }
      int32_t dsp_height = gfx->height()/4;
      int32_t fft_height = dsp_height - header_height - 1;
      size_t xe = gfx->width() / bw;
      if (xe > (FFT_SIZE/2)) { xe = (FFT_SIZE/2); }
      int32_t wave_next = ((header_height + dsp_height) >> 1) + (((256 - (raw_data[0] + raw_data[1])) * fft_height) >> 17);

      uint32_t bar_color[2] = { 0x000033u, 0x99AAFFu };

//      int32_t lipsync_temp = 0;
      for (size_t bx = 0; bx <= xe; ++bx)
      {
        size_t x = bx * bw;
        if ((x & 7) == 0) { gfx->display(); taskYIELD(); }
        int32_t f = fft.get(bx);
        int32_t y = (f * fft_height) >> 18;
//         if (x > 0 and x < 10) { // 0〜31の範囲でlipsyncでピックアップしたい音域を選びます。
//           int32_t f1 = f * 100;
//           lipsync_temp = std::max(lipsync_temp, f1 >> 19); // 指定した範囲で最も高い音量を設定。
// //            lipsync_temp += (f1 >> 18);
//         }
        if (y > fft_height) { y = fft_height; }
        y = dsp_height - y;
        int32_t py = prev_y[bx];
        if (y != py)
        {
          gfx->fillRect(x, y, bw - 1, py - y, bar_color[(y < py)]);
          prev_y[bx] = y;
        }
        py = peak_y[bx] + 1;
        if (py < y)
        {
          gfx->writeFastHLine(x, py - 1, bw - 1, bgcolor(gfx, py - 1));
        }
        else
        {
          py = y - 1;
        }
        if (peak_y[bx] != py)
        {
          peak_y[bx] = py;
          gfx->writeFastHLine(x, py, bw - 1, TFT_WHITE);
        }


        if (wave_enabled)
        {
          for (size_t bi = 0; bi < bw; ++bi)
          {
            size_t i = x + bi;
            if (i >= gfx->width() || i >= WAVE_SIZE) { break; }
            y = wave_y[i];
            int32_t h = wave_h[i];
            bool use_bg = (bi+1 == bw);
            if (h>0)
            { /// erase previous wave.
              gfx->setAddrWindow(i, y, 1, h);
              h += y;
              do
              {
                uint32_t bg = (use_bg || y < peak_y[bx]) ? bgcolor(gfx, y)
                            : (y == peak_y[bx]) ? 0xFFFFFFu
                            : bar_color[(y >= prev_y[bx])];
                gfx->writeColor(bg, 1);
              } while (++y < h);
            }
            size_t i2 = i << 1;
            int32_t y1 = wave_next;
            wave_next = ((header_height + dsp_height) >> 1) + (((256 - (raw_data[i2] + raw_data[i2 + 1])) * fft_height) >> 17);
            int32_t y2 = wave_next;
            if (y1 > y2)
            {
              int32_t tmp = y1;
              y1 = y2;
              y2 = tmp;
            }
            y = y1;
            h = y2 + 1 - y;
            wave_y[i] = y;
            wave_h[i] = h;
            if (h>0)
            { /// draw new wave.
              gfx->setAddrWindow(i, y, 1, h);
              h += y;
              do
              {
                uint32_t bg = (y < prev_y[bx]) ? 0xFFCC33u : 0xFFFFFFu;
                gfx->writeColor(bg, 1);
              } while (++y < h);
            }
          }
        }
      }
//      lipsync_level = lipsync_temp; // リップシンクを設定
      gfx->display();
      gfx->endWrite();
    }
  }
}

bool servo_home = false;
bool levelMeter = true;
bool balloon = false;

void servoLoop(void *args) {
  long move_time = 0;
  long interval_time = 0;
  long move_x = 0;
  long move_y = 0;
  float gaze_x = 0.0f;
  float gaze_y = 0.0f;
  bool sing_mode = false;
  for (;;) {
    if(!servo_home)
    {

    if (mouth_ratio == 0.0f) {
      // 待機時の動き
      interval_time = random(system_config.getServoInterval(AvatarMode::NORMAL)->interval_min
                           , system_config.getServoInterval(AvatarMode::NORMAL)->interval_max);
      move_time = random(system_config.getServoInterval(AvatarMode::NORMAL)->move_min
                       , system_config.getServoInterval(AvatarMode::NORMAL)->move_max);
      lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンク上限の初期化
      sing_mode = false;

    } else {
      // 歌うモードの動き
      interval_time = random(system_config.getServoInterval(AvatarMode::SINGING)->interval_min
                           , system_config.getServoInterval(AvatarMode::SINGING)->interval_max);
      move_time = random(system_config.getServoInterval(AvatarMode::SINGING)->move_min
                       , system_config.getServoInterval(AvatarMode::SINGING)->move_max);
      sing_mode = true;
    } 
    avatar.getGaze(&gaze_y, &gaze_x);
//    avatar.setRotation(gaze_x * 5);
    
//    Serial.printf("x:%f:y:%f\n", gaze_x, gaze_y);
    // X軸は90°から+-で左右にスイング
    if (gaze_x < 0) {
      move_x = START_DEGREE_VALUE_X - mouth_ratio * 15 + (int)(30.0 * gaze_x);
    } else {
      move_x = START_DEGREE_VALUE_X + mouth_ratio * 15 + (int)(30.0 * gaze_x);
    }
    // Y軸は90°から上にスイング（最大35°）
    move_y = START_DEGREE_VALUE_Y - mouth_ratio * 10 - abs(25.0 * gaze_y);
    servo.moveXY(move_x, move_y, move_time);
    } else {
      servo.moveXY(START_DEGREE_VALUE_X, START_DEGREE_VALUE_Y, 500);
    }

    if (!bluetooth_mode) {
      int lyric_no = random(system_config.getLyrics_num());
      int exp2 = random(2);
      avatar.setMouthOpenRatio(1.0f);
      avatar.setSpeechText((const char*)system_config.getLyric(lyric_no)->c_str());
      if (exp2 % 2) {
        avatar.setExpression(Expression::Neutral);
      } else {
        avatar.setExpression(Expression::Happy);
      }
      avatar.setMouthOpenRatio(0.5f);
    }
    //avatar.setExpression((Expression)exp);
    if (sing_mode && !servo_home) {
      // 歌っているときはうなずく
      servo.moveXY(move_x, move_y + 10, 400);
    }

    // } else {
    //   servo.moveXY(START_DEGREE_VALUE_X, START_DEGREE_VALUE_Y, 500);
    // }
    vTaskDelay(interval_time/portTICK_PERIOD_MS);

  }
}

static fft_t fft1;
static int16_t raw_data1[WAVE_SIZE * 2];

void lipSync(void *args)
{
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  for (;;)
  {
    uint64_t level = 0;
    auto buf = a2dp_sink.getBuffer();
    if (buf) {
      memcpy(raw_data1, buf, WAVE_SIZE * 2 * sizeof(int16_t));
      fft1.exec(raw_data1);
      for (size_t bx = 5; bx <= 60; ++bx) { // リップシンクで抽出する範囲はここで指定(低音)0〜64（高音）
        int32_t f = fft1.get(bx);
        level += abs(f);
        //Serial.printf("bx:%d, f:%d\n", bx, f) ;
      }
      //Serial.printf("level:%d\n", level >> 16);
    }

    // スレッド内でログを出そうとすると不具合が起きる場合があります。
    //Serial.printf("data=%d\n\r", level >> 16);
    mouth_ratio = (float)(level >> 16)/lipsync_level_max;
    if (mouth_ratio > 1.2f) {
      if (mouth_ratio > 1.5f) {
        lipsync_level_max += 10.0f; // リップシンク上限を大幅に超えるごとに上限を上げていく。
      }
      mouth_ratio = 1.2f;
    }
    avatar->setMouthOpenRatio(mouth_ratio);
    vTaskDelay(30/portTICK_PERIOD_MS);
  }
}

void hvt_event_callback(int avatar_expression, const char* text) {
  avatar.setExpression((Expression)avatar_expression);
  if(!levelMeter && bluetooth_mode) avatar.setSpeechText(text);
}

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2)
{
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
  if (sing_happy) {
    avatar.setExpression(Expression::Happy);
  } else {
    avatar.setExpression(Expression::Neutral);
  }
  sing_happy = !sing_happy;

}

Face* faces[8];
const int facesSize = sizeof(faces) / sizeof(Face*);
//int faceIdx = 1;
int16_t faceIdx = 1;
ColorPalette* cps[8];
const int cpsSize = sizeof(cps) / sizeof(ColorPalette*);
int cpsIdx = 0;
const uint16_t color_table[facesSize] = {
  TFT_BLACK,  //Default
  TFT_WHITE,  //AtaruFace
  TFT_WHITE,  //RamFace
  0xef55,     //DannFace
  TFT_WHITE,  //DogFace
  0xef55,     //DannFace
  TFT_YELLOW,  //ToraFace
  TFT_BROWN,  //DoomoFce
};

void Avatar_setup(bool fullScreen) {
//  avatar = new Avatar();
  faces[0] = avatar.getFace();
  faces[1] = new AtaruFace();
  faces[2] = new RamFace();
  faces[3] = new DannFace();
  faces[4] = new DogFace();
  faces[5] = avatar.getFace();
  faces[6] = new ToraFace();
  faces[7] = new DoomoFace();

  cps[0] = new ColorPalette();
  cps[1] = new ColorPalette();
  cps[2] = new ColorPalette();
  cps[3] = new ColorPalette();
  cps[4] = new ColorPalette();
  cps[5] = new ColorPalette();
  cps[6] = new ColorPalette();
  cps[7] = new ColorPalette();
  cps[1]->set(COLOR_PRIMARY, PC_BLACK);  //AtaruFace
  cps[1]->set(COLOR_SECONDARY, PC_WHITE);
  cps[1]->set(COLOR_BACKGROUND, PC_WHITE);
  cps[2]->set(COLOR_PRIMARY, PC_BLACK);  //RamFace
  cps[2]->set(COLOR_SECONDARY, PC_WHITE);
  cps[2]->set(COLOR_BACKGROUND, PC_WHITE);
  cps[3]->set(COLOR_PRIMARY, PC_BLACK); //DannFace
  cps[3]->set(COLOR_BACKGROUND, 9);
  cps[3]->set(COLOR_SECONDARY, PC_WHITE);
  cps[4]->set(COLOR_PRIMARY, PC_BLACK);  //DogFace
  cps[4]->set(COLOR_SECONDARY, PC_WHITE);
  cps[4]->set(COLOR_BACKGROUND, PC_WHITE);
  cps[5] = cps[3];
  cps[6]->set(COLOR_PRIMARY, PC_BLACK);  //DogFace
  cps[6]->set(COLOR_SECONDARY, PC_WHITE);
  cps[6]->set(COLOR_BACKGROUND, PC_YELLOW);
  cps[7]->set(COLOR_PRIMARY, PC_BLACK);  //DoomoFce
  cps[7]->set(COLOR_BACKGROUND, PC_BROWN);
  cps[7]->set(COLOR_SECONDARY, PC_WHITE);

  avatar.setFace(faces[faceIdx]);
  avatar.setColorPalette(*cps[faceIdx]);
  if(!fullScreen)
  {
    switch (M5.getBoard())
    {
      case m5::board_t::board_M5Stack:
      case m5::board_t::board_M5StackCore2:
      case m5::board_t::board_M5Tough:
        avatar.setScale(0.80);
        avatar.setOffset(0, 52);
        break;
      default:
        avatar.setScale(1.00);
        avatar.setOffset(0, 0);
      break;
    }
  }
  avatar.init(4); // start drawing
//  avatar.draw();
  avatar.addTask(lipSync, "lipSync");
}
void select_face(int idx){
  avatar.setFace(faces[1]);
  avatar.setColorPalette(*cps[1]);
}

struct box_t
{
  int x;
  int y;
  int w;
  int h;
  int touch_id = -1;

  void setupBox(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
  bool contain(int x, int y)
  {
    return this->x <= x && x < (this->x + this->w)
        && this->y <= y && y < (this->y + this->h);
  }
};

static box_t box_level;
static box_t box_servo;
static box_t box_balloon;
static box_t box_face;

void displevelMeter(bool levelMeter)
{
  if(levelMeter)
  {
    M5.Display.clear();
    gfxSetup(&M5.Display);
    M5.Display.fillRect(0, M5.Display.height()/4+1, M5.Display.width(), M5.Display.height(), color_table[faceIdx]); //
    avatar.setScale(0.80);
    avatar.setOffset(0, 52);
    if(balloon) {
      avatar.setSpeechText("");
      balloon = false;
    }
  } else {
    M5.Display.fillScreen(color_table[faceIdx]); //
    avatar.setScale(1.0);
    avatar.setOffset(0, 0);
  }
}

void setup(void)
{
  auto cfg = M5.config();

  cfg.external_spk = true;    /// use external speaker (SPK HAT / ATOMIC SPK)
//cfg.external_spk_detail.omit_atomic_spk = true; // exclude ATOMIC SPK
//cfg.external_spk_detail.omit_spk_hat    = true; // exclude SPK HAT

  M5.begin(cfg);
  if (M5.getBoard() == m5::board_t::board_M5Stack) {
    M5.In_I2C.release();
  }

  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
    spk_cfg.sample_rate = 96000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    // spk_cfg.task_priority = configMAX_PRIORITIES - 2;
    spk_cfg.dma_buf_count = 20;
    //spk_cfg.stereo = true;
    // spk_cfg.dma_buf_len = 512;
    M5.Speaker.config(spk_cfg);
  }


  M5.Speaker.begin();

  // BASICとFIREのV2.6で25MHzだと読み込めないため10MHzまで下げています。
  SD.begin(GPIO_NUM_4, SPI, 15000000);
  
  delay(1000);
  system_config.loadConfig(json_fs, stackchan_system_config_yaml);
  
  M5.Speaker.setVolume(system_config.getBluetoothSetting()->start_volume);
  M5.Speaker.setChannelVolume(system_config.getBluetoothSetting()->start_volume, m5spk_virtual_channel);

  bluetooth_mode = system_config.getBluetoothSetting()->starting_state;
  Serial.printf("Bluetooth_mode:%s\n", bluetooth_mode ? "true" : "false");
  
  servo.begin(system_config.getServoInfo()->servo_pin_x, START_DEGREE_VALUE_X,
              system_config.getServoInfo()->servo_offset_x,
              system_config.getServoInfo()->servo_pin_y, START_DEGREE_VALUE_Y,
              system_config.getServoInfo()->servo_offset_y);
  delay(2000);

  {
    uint32_t nvs_handle;
    if (ESP_OK == nvs_open("Avatar", NVS_READONLY, &nvs_handle)) {
      nvs_get_i16(nvs_handle, "faceIdx", &faceIdx);
      if(faceIdx < 0 || faceIdx >= facesSize) {
        faceIdx = 0;
      }
      nvs_close(nvs_handle);
    }
  }
  {
    uint32_t nvs_handle;
    if (ESP_OK == nvs_open("BTSPK", NVS_READONLY, &nvs_handle)) {
      size_t volume;
      nvs_get_u32(nvs_handle, "volume", &volume);
      M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
      nvs_close(nvs_handle);
    }
  }

  levelMeter = bluetooth_mode;
  if(bluetooth_mode){
    gfxSetup(&M5.Display);
    M5.Display.fillRect(0, M5.Display.height()/4+1, M5.Display.width(), M5.Display.height(), color_table[faceIdx]); //
  }
  Avatar_setup(!bluetooth_mode);

  // avatar.addTask(lipSync, "lipSync");
  avatar.addTask(servoLoop, "servoLoop");
  avatar.setExpression(Expression::Neutral);
  avatar.setSpeechFont(system_config.getFont());

  if (bluetooth_mode) {
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.setHvtEventCallback(hvt_event_callback);
    a2dp_sink.start(system_config.getBluetoothSetting()->device_name.c_str(), true);
    avatar.setExpression(Expression::Sad);
    avatar.setSpeechText("Bluetooth Mode");
    delay(1000);
    avatar.setSpeechText("");
  } else {
    avatar.setSpeechText("Normal Mode");
  }
  box_level.setupBox(0, 0, M5.Display.width(), 60);
  box_servo.setupBox(80, 100, 80, 60);
  box_face.setupBox(280, 100, 40, 60);
  box_balloon.setupBox(0, 160, M5.Display.width(), 80);

  FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUM_LEDS);        // FastLED関連
  FastLED.setBrightness(15);                                 // FastLED関連 ※この値は20以上設定してはいけません！
  Serial.begin(9600);                                        // FastLED関連

}

void loop(void)
{
  static unsigned long long saveSettings = 0;
  if(levelMeter) gfxLoop(&M5.Display);
  avatar.draw();
  if(!levelMeter && balloon && bluetooth_mode)   avatar.setSpeechText(a2dp_sink.getMetaData(0, false));

  {
    static int prev_frame;
    int frame;
    do
    {
      vTaskDelay(1);
    } while (prev_frame == (frame = millis() >> 3)); /// 8 msec cycle wait
    prev_frame = frame;
  }
  static int lastms = 0;
  if (millis()-lastms > 1000) {
    lastms = millis();
    BatteryLevel = M5.Power.getBatteryLevel();
//    printf("%d\n\r",BatVoltage);
   }

  M5.update();
  auto count = M5.Touch.getCount();
  if (count)
  {
    auto t = M5.Touch.getDetail();
    if (t.wasPressed())
    {    
      if (box_level.contain(t.x, t.y) && bluetooth_mode)
      {
        levelMeter = !levelMeter;
        displevelMeter(levelMeter);
        M5.Speaker.tone(1000, 100);
      }
      if (box_servo.contain(t.x, t.y))
      {
        servo_home = !servo_home;
        M5.Speaker.tone(1000, 100);
      }
      if (box_balloon.contain(t.x, t.y) && !levelMeter && bluetooth_mode)
      {
        balloon = !balloon;
        if(!balloon) avatar.setSpeechText("");
        M5.Speaker.tone(1000, 100);
      }
      if (box_face.contain(t.x, t.y))
      {
        faceIdx = (faceIdx + 1) % facesSize;
        if(levelMeter)
        {
          M5.Display.fillRect(0, M5.Display.height()/4+1, M5.Display.width(), M5.Display.height(), color_table[faceIdx]); //Dann
        } else {
          M5.Display.fillScreen(color_table[faceIdx]); //Dann
        }
        avatar.setFace(faces[faceIdx]);
        avatar.setColorPalette(*cps[faceIdx]);
        {
          uint32_t nvs_handle;
          if (ESP_OK == nvs_open("Avatar", NVS_READWRITE, &nvs_handle)) {
            nvs_set_i16(nvs_handle, "faceIdx", faceIdx);
            nvs_close(nvs_handle);
          }
        }
        M5.Speaker.tone(1000, 100);
      }
    }
  }

  if (M5.BtnA.wasDecideClickCount())
  {
    switch (M5.BtnA.getClickCount())
    {
    case 1:
      if (!bluetooth_mode) {
        a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
        a2dp_sink.setHvtEventCallback(hvt_event_callback);
        a2dp_sink.start(system_config.getBluetoothSetting()->device_name.c_str(), true);
        avatar.setExpression(Expression::Sad);
//        avatar.setSpeechText("Bluetooth Mode");
        avatar.setSpeechText("");
        M5.Speaker.tone(1000, 100);
        bluetooth_mode = true;
        levelMeter = true;
        displevelMeter(true);
      }
      break;

    case 2:
      if (bluetooth_mode) {
        bluetooth_mode = false;
        displevelMeter(false);
        avatar.setExpression(Expression::Neutral);
        avatar.setSpeechText("Normal Mode");
        avatar.draw();
        M5.Speaker.tone(800, 100);
        a2dp_sink.stop();
        a2dp_sink.end(true);
        delay(1000);
        avatar.setSpeechText("");
        levelMeter = false;
//        servo_home = false;
//        bluetooth_mode = false;
      }
      break;
    }
  }
  if (M5.BtnB.wasPressed()) {
    uint8_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    volume = volume - 10;
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(2000, 100);
    delay(200);
    M5.Speaker.tone(1000, 100);
    saveSettings = millis() + 5000;
  }
  if (M5.BtnC.wasPressed()) {
    uint8_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    volume = volume + 10;
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(1000, 100);
    delay(200);
    M5.Speaker.tone(2000, 100);
    saveSettings = millis() + 5000;
  }
  if (saveSettings > 0 && millis() > saveSettings)
  {
    uint32_t nvs_handle;
    if (ESP_OK == nvs_open("BTSPK", NVS_READWRITE, &nvs_handle)) {
      size_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
      nvs_set_u32(nvs_handle, "volume", volume);
      nvs_close(nvs_handle);
    }
    saveSettings = 0;
  }

#if not(defined(ARDUINO_M5STACK_FIRE) || defined(ARDUINO_M5Stack_Core_ESP32)) // FireはAxp192ではないのとI2Cが使えないので制御できません。
  if (M5.Power.Axp192.getACINVoltage() < 3.0f) {
    // USBからの給電が停止したとき
    // Serial.println("USBPowerUnPlugged.");
    M5.Power.setLed(0);
    if ((system_config.getAutoPowerOffTime() > 0) and (last_discharge_time == 0)) {
      last_discharge_time = millis();
    } else if ((system_config.getAutoPowerOffTime() > 0) 
               and ((millis() - last_discharge_time) > system_config.getAutoPowerOffTime())) {
      M5.Power.powerOff();
    }
  } else {
    //Serial.println("USBPowerPlugged.");
    M5.Power.setLed(80);
    if (last_discharge_time > 0) {
      last_discharge_time = 0;
    }
  }
#endif

effect.update();        // FastLED関連
FastLED.show();         // FastLED関連

}

#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif