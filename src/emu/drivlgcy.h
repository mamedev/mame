// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drivlgcy.h

    Legacy driver helpers.

***************************************************************************/

#pragma once

#ifndef __DRIVLGCY_H__
#define __DRIVLGCY_H__

//**************************************************************************
//  MACROS
//**************************************************************************

#define MCFG_MACHINE_START(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_MACHINE_START, MACHINE_START_NAME(_func));

#define MCFG_MACHINE_RESET(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_MACHINE_RESET, MACHINE_RESET_NAME(_func));

#define MCFG_VIDEO_START(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_VIDEO_START, VIDEO_START_NAME(_func));

#define MCFG_PALETTE_INIT_LEGACY(_func) \
	palette_device::static_set_init(*device, palette_init_delegate(PALETTE_INIT_NAME(_func), "palette_init_" #_func, downcast<palette_device *>(device)));
	


#define VIDEO_START_CALL(name)      VIDEO_START_NAME(name)(machine)

#endif  /* __DRIVLGCY_H__ */
