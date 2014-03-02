// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    scrlegcy.h

    Legacy screen helpers.

***************************************************************************/

#pragma once

#ifndef __SCRLEGCY_H__
#define __SCRLEGCY_H__

//**************************************************************************
//  SCREEN DEVICE CONFIGURATION MACROS
//**************************************************************************

#define SCREEN_UPDATE32_CALL(name)      SCREEN_UPDATE_NAME(name)(NULL, screen, bitmap, cliprect)

#define MCFG_SCREEN_UPDATE_STATIC(_func) \
	screen_device::static_set_screen_update(*device, screen_update_delegate_smart(&screen_update_##_func, "screen_update_" #_func));


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
