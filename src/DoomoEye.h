// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.
//どーもくん (C)NHK・TYO  

#ifndef DoomoEYE_H_
#define DoomoEYE_H_

#include <M5Unified.h>
#include "DrawContext.h"
#include "Drawable.h"

namespace m5avatar {

class DoomoEye final : public Drawable {
 private:
  uint16_t r;
  bool isLeft;

 public:
  // constructor
  DoomoEye() = delete;
  DoomoEye(uint16_t x, uint16_t y, uint16_t r, bool isLeft);  // deprecated
  DoomoEye(uint16_t r, bool isLeft);
  ~DoomoEye() = default;
  DoomoEye(const DoomoEye &other) = default;
  DoomoEye &operator=(const DoomoEye &other) = default;
  void draw(M5Canvas *spi, BoundingRect rect,
            DrawContext *drawContext) override;
  // void draw(M5Canvas *spi, DrawContext *drawContext) override; // deprecated
};

}  // namespace m5avatar

#endif  // DoomoEYE_H_
