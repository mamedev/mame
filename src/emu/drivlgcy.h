/***************************************************************************

    drivlgcy.h

    Legacy driver helpers.

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

#ifndef __DRIVLGCY_H__
#define __DRIVLGCY_H__

//**************************************************************************
//  MACROS
//**************************************************************************

#define MCFG_MACHINE_START(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_MACHINE_START, MACHINE_START_NAME(_func));

#define MCFG_MACHINE_RESET(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_MACHINE_RESET, MACHINE_RESET_NAME(_func));
	
#define MCFG_SOUND_START(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_SOUND_START, SOUND_START_NAME(_func));
	
#define MCFG_SOUND_RESET(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_SOUND_RESET, SOUND_RESET_NAME(_func));
	
#define MCFG_PALETTE_INIT(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_PALETTE_INIT, PALETTE_INIT_NAME(_func));
	
#define MCFG_VIDEO_START(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_VIDEO_START, VIDEO_START_NAME(_func));
	
#define MCFG_VIDEO_RESET(_func) \
	driver_device::static_set_callback(*owner, driver_device::CB_VIDEO_RESET, VIDEO_RESET_NAME(_func));

	
#define MACHINE_START_CALL(name)    MACHINE_START_NAME(name)(machine)

#define MACHINE_RESET_CALL(name)    MACHINE_RESET_NAME(name)(machine)

#define PALETTE_INIT_CALL(name)     PALETTE_INIT_NAME(name)(machine)

#define VIDEO_START_CALL(name)      VIDEO_START_NAME(name)(machine)

#define VIDEO_RESET_CALL(name)      VIDEO_RESET_NAME(name)(machine)

#endif  /* __DRIVLGCY_H__ */
