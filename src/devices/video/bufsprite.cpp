// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

*********************************************************************/

#include "emu.h"
#include "bufsprite.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type BUFFERED_SPRITERAM8 = device_creator<buffered_spriteram8_device>;
const device_type BUFFERED_SPRITERAM16 = device_creator<buffered_spriteram16_device>;
const device_type BUFFERED_SPRITERAM32 = device_creator<buffered_spriteram32_device>;
const device_type BUFFERED_SPRITERAM64 = device_creator<buffered_spriteram64_device>;
