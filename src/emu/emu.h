// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emu.h

    Core header file to be included by most files.

    NOTE: The contents of this file are designed to meet the needs of
    drivers and devices. In addition to this file, you will also need
    to include the headers of any CPUs or other devices that are required.

    If you find yourself needing something outside of this file in a
    driver or device, think carefully about what you are doing.

***************************************************************************/

#ifndef __EMU_H__
#define __EMU_H__

#include <list>
#include <forward_list>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>

// core emulator headers -- must be first (profiler needs attotime, attotime needs xtal)
#include "emucore.h"
#include "eminline.h"
#include "xtal.h"
#include "attotime.h"
#include "profiler.h"

// http interface helpers
#include "http.h"

// commonly-referenced utilities imported from lib/util
#include "palette.h"
#include "strformat.h"
#include "vecstream.h"

// emulator-specific utilities
#include "hash.h"
#include "fileio.h"
#include "delegate.h"
#include "devdelegate.h"

// memory and address spaces
#include "emumem.h"

// machine-wide utilities
#include "romentry.h"
#include "save.h"

// I/O
#include "input.h"
#include "ioport.h"
#include "output.h"

// devices and callbacks
#include "device.h"
#include "devfind.h"
#include "addrmap.h" // Needs optional_device<> and required_device<>
#include "distate.h"
#include "dimemory.h"
#include "opresolv.h"
#include "dipalette.h"
#include "digfx.h"
#include "diimage.h"
#include "dislot.h"
#include "disound.h"
#include "divideo.h"
#include "dinvram.h"
#include "schedule.h"
#include "dinetwork.h"

// machine and driver configuration
#include "mconfig.h"
#include "gamedrv.h"
#include "parameters.h"

// the running machine
#include "main.h"
#include "machine.h"
#include "driver.h"

// common device interfaces
#include "diexec.h"
#include "devcpu.h"

// video-related
#include "drawgfx.h"
#include "video.h"

// sound-related
#include "sound.h"

// generic helpers
#include "devcb.h"
#include "bookkeeping.h"
#include "video/generic.h"

// member templates that don't like incomplete types
#include "device.ipp"

#endif // __EMU_H__
