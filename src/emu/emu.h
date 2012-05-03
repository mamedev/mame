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

// device_delegate is a delegate that wraps with a device tag and can be easily
// late bound without replicating logic everywhere
template<typename _Signature>
class device_delegate : public delegate<_Signature>
{
	typedef delegate<_Signature> basetype;

public:
	// provide same set of constructors as the base class, with additional device name
	// parameter
	device_delegate() : basetype(), m_device_name(NULL) { }
	device_delegate(const basetype &src) : basetype(src), m_device_name(src.m_device_name) { }
	device_delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object), m_device_name(src.m_device_name) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), m_device_name(devname) { }
	device_delegate(typename basetype::template traits<device_t>::static_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), m_device_name(NULL) { }
	device_delegate(typename basetype::template traits<device_t>::static_ref_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), m_device_name(NULL) { }
	device_delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; m_device_name = src.m_device_name; return *this; }

	// perform the binding
	void bind_relative_to(device_t &search_root);

private:
	// internal state
	const char *m_device_name;
};

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
#include "drivers/xtal.h"
#include "machine/generic.h"
#include "video/generic.h"

#endif	/* __EMU_H__ */
