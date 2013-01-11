/***************************************************************************

    emu.h

    Core header file to be included by most files.

    NOTE: The contents of this file are designed to meet the needs of
    drivers and devices. In addition to this file, you will also need
    to include the headers of any CPUs or other devices that are required.

    If you find yourself needing something outside of this file in a
    driver or device, think carefully about what you are doing.

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

#ifndef __EMU_H__
#define __EMU_H__

// turn off legacy bitmap addressing macros
#define BITMAP_DISABLE_LEGACY_MACROS

// core emulator headers -- must be first
#include "emucore.h"
#include "emutempl.h"
#include "eminline.h"
#include "profiler.h"

// commonly-referenecd utilities imported from lib/util
#include "palette.h"
#include "unicode.h"

// emulator-specific utilities
#include "attotime.h"
#include "hash.h"
#include "fileio.h" // remove me once NVRAM is implemented as device
#include "delegate.h"
#include "devdelegate.h"

// memory and address spaces
#include "memory.h"
#include "addrmap.h"

// machine-wide utilities
#include "romload.h"
#include "save.h"

// define machine_config_constructor here due to circular dependency
// between devices and the machine config
class machine_config;
typedef device_t * (*machine_config_constructor)(machine_config &config, device_t *owner);

// I/O
#include "input.h"
#include "ioport.h"
#include "output.h"

// diimage requires uimenu
#include "uimenu.h"

// devices and callbacks
#include "device.h"
#include "distate.h"
#include "dimemory.h"
#include "diexec.h"
#include "opresolv.h"
#include "diimage.h"
#include "diserial.h"
#include "dislot.h"
#include "disound.h"
#include "dinvram.h"
#include "dirtc.h"
#include "didisasm.h"
#include "schedule.h"
#include "timer.h"
#include "dinetwork.h"

// timers, CPU and scheduling
#include "devcpu.h"

// machine and driver configuration
#include "mconfig.h"
#include "gamedrv.h"

// image-related
#include "softlist.h"
#include "image.h"

// networking
#include "network.h"

// the running machine
#include "machine.h"
#include "driver.h"
#include "mame.h"

// video-related
#include "drawgfx.h"
#include "tilemap.h"
#include "emupal.h"
#include "screen.h"
#include "video.h"

// sound-related
#include "sound.h"
#include "speaker.h"

// generic helpers
#include "devcb.h"
#include "devcb2.h"
#include "drivers/xtal.h"
#include "machine/generic.h"
#include "video/generic.h"

#endif  /* __EMU_H__ */
