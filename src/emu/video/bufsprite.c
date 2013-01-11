/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
