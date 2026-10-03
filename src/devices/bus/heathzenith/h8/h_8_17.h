// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H-17 Floppy Disk Controller


  Model number: H-8-17

  The controller itself is shared with the H89's H-88-1 card, and is in
  bus/heathzenith/h8x_common/h17_fdc_base.{h,cpp}.  Only the bus attachment is here.

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H8_H_8_17_H
#define MAME_BUS_HEATHZENITH_H8_H_8_17_H

#pragma once

#include "h8bus.h"

DECLARE_DEVICE_TYPE(H8BUS_H_8_17, device_h8bus_card_interface)

#endif // MAME_BUS_HEATHZENITH_H8_H_8_17_H
