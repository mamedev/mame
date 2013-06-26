/***************************************************************************

    dinvram.h

    Device NVRAM interfaces.

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
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DINVRAM_H__
#define __DINVRAM_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_nvram_interface

// class representing interface-specific live nvram
class device_nvram_interface : public device_interface
{
public:
	// construction/destruction
	device_nvram_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nvram_interface();

	// public accessors... for now
	void nvram_reset() { nvram_default(); }
	void nvram_load(emu_file &file) { nvram_read(file); }
	void nvram_save(emu_file &file) { nvram_write(file); }

protected:
	// derived class overrides
	virtual void nvram_default() = 0;
	virtual void nvram_read(emu_file &file) = 0;
	virtual void nvram_write(emu_file &file) = 0;
};

// iterator
typedef device_interface_iterator<device_nvram_interface> nvram_interface_iterator;


#endif  /* __DINVRAM_H__ */
