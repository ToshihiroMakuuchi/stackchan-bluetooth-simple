// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef TORAEYE_H_
#define TORAEYE_H_

#include <M5Unified.h>
#include "DrawContext.h"
#include "Drawable.h"

namespace m5avatar {

class ToraEye final : public Drawable {
 private:
  uint16_t r;
  bool isLeft;

 public:
  // constructor
  ToraEye() = delete;
  ToraEye(uint16_t x, uint16_t y, uint16_t r, bool isLeft);  // deprecated
  ToraEye(uint16_t r, bool isLeft);
  ~ToraEye() = default;
  ToraEye(const ToraEye &other) = default;
  ToraEye &operator=(const ToraEye &other) = default;
  void draw(M5Canvas *spi, BoundingRect rect,
            DrawContext *drawContext) override;
  // void draw(M5Canvas *spi, DrawContext *drawContext) override; // deprecated
};

}  // namespace m5avatar

#endif  // TORAEYE_H_
