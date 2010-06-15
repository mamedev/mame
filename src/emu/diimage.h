/***************************************************************************

    diimage.h

    Device image interfaces.

****************************************************************************

    Copyright Miodrag Milanovic
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

#ifndef __DIIMAGE_H__
#define __DIIMAGE_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_config_image_interface

// class representing interface-specific configuration image
class device_config_image_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_image_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_image_interface();
};



// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
public:
	// construction/destruction
	device_image_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_image_interface();

	// configuration access	
	const device_config_image_interface &image_config() const { return m_image_config; }

	// public accessors... for now

protected:
	// derived class overrides

	// configuration
	const device_config_image_interface &m_image_config;	// reference to our device_config_execute_interface
};


#endif	/* __DIIMAGE_H__ */
