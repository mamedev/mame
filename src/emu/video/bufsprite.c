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
extern const device_type BUFFERED_SPRITERAM8 = &device_creator<buffered_spriteram8_device>;
extern const device_type BUFFERED_SPRITERAM16 = &device_creator<buffered_spriteram16_device>;
extern const device_type BUFFERED_SPRITERAM32 = &device_creator<buffered_spriteram32_device>;
extern const device_type BUFFERED_SPRITERAM64 = &device_creator<buffered_spriteram64_device>;



/* ----- sprite buffering ----- */

/* buffered sprite RAM write handlers */
WRITE8_HANDLER( buffer_spriteram_w ) { }
WRITE16_HANDLER( buffer_spriteram16_w ) { }
WRITE32_HANDLER( buffer_spriteram32_w ) { }
WRITE8_HANDLER( buffer_spriteram_2_w ) { }
WRITE16_HANDLER( buffer_spriteram16_2_w ) { }
WRITE32_HANDLER( buffer_spriteram32_2_w ) { }

/* perform the actual buffering */
void buffer_spriteram(running_machine &machine, UINT8 *ptr, int length) { }
void buffer_spriteram_2(running_machine &machine, UINT8 *ptr, int length) { }
