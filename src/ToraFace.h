// Copyright (c) robo8080. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef FACES_TORAFACE_H_
#define FACES_TORAFACE_H_

#include <M5Unified.h>
#include "ToraEye.h"
#include "ToraMouth.h"
#include "Eyeblow.h"

namespace m5avatar {
class ToraFace : public Face {
 public:
  ToraFace()
      : Face(new ToraMouth(140, 140, 40, 70), new BoundingRect(150, 160),  //Toraface
             new ToraEye(18, false), new BoundingRect(50, 75),
             new ToraEye(18, true),  new BoundingRect(50, 245),
             new Eyeblow(32, 0, false), new BoundingRect(67, 96),
             new Eyeblow(32, 0, true), new BoundingRect(72, 230)) {}
};

}  // namespace m5avatar

#endif  // FACES_TORAFACE_H_
