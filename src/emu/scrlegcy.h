/***************************************************************************

    scrlegcy.h

    Legacy screen helpers.

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

***************************************************************************/

#pragma once

#ifndef __SCRLEGCY_H__
#define __SCRLEGCY_H__

//**************************************************************************
//  SCREEN DEVICE CONFIGURATION MACROS
//**************************************************************************

#define SCREEN_UPDATE16_CALL(name)      SCREEN_UPDATE_NAME(name)(NULL, screen, bitmap, cliprect)
#define SCREEN_UPDATE32_CALL(name)      SCREEN_UPDATE_NAME(name)(NULL, screen, bitmap, cliprect)

#define SCREEN_VBLANK_CALL(name)        SCREEN_VBLANK_NAME(name)(NULL, screen, vblank_on)

#define MCFG_SCREEN_UPDATE_STATIC(_func) \
	screen_device::static_set_screen_update(*device, screen_update_delegate_smart(&screen_update_##_func, "screen_update_" #_func));
#define MCFG_SCREEN_VBLANK_STATIC(_func) \
	screen_device::static_set_screen_vblank(*device, screen_vblank_delegate(&screen_vblank_##_func, "screen_vblank_" #_func));


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  screen_update_delegate_smart - collection of
//  inline helpers which create the appropriate
//  screen_update_delegate based on the input
//  function type
//-------------------------------------------------

inline screen_update_ind16_delegate screen_update_delegate_smart(UINT32 (*callback)(device_t *, screen_device &, bitmap_ind16 &, const rectangle &), const char *name)
{
	return screen_update_ind16_delegate(callback, name);
}

inline screen_update_rgb32_delegate screen_update_delegate_smart(UINT32 (*callback)(device_t *, screen_device &, bitmap_rgb32 &, const rectangle &), const char *name)
{
	return screen_update_rgb32_delegate(callback, name);
}

#endif  /* __SCRLEGCY_H__ */
