/***************************************************************************

    nvram.h

    Generic non-volatile RAM.

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

#ifndef __NVRAM_H__
#define __NVRAM_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NVRAM_ADD_0FILL(_tag) \
	MCFG_DEVICE_ADD(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_ALL_0);
#define MCFG_NVRAM_ADD_1FILL(_tag) \
	MCFG_DEVICE_ADD(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_ALL_1);
#define MCFG_NVRAM_ADD_RANDOM_FILL(_tag) \
	MCFG_DEVICE_ADD(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_RANDOM);
#define MCFG_NVRAM_ADD_NO_FILL(_tag) \
	MCFG_DEVICE_ADD(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_NONE);
#define MCFG_NVRAM_ADD_CUSTOM_DRIVER(_tag, _class, _method) \
	MCFG_DEVICE_ADD(_tag, NVRAM, 0) \
	nvram_device::static_set_custom_handler(*device, nvram_init_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_NVRAM_REPLACE_0FILL(_tag) \
	MCFG_DEVICE_REPLACE(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_ALL_0);
#define MCFG_NVRAM_REPLACE_1FILL(_tag) \
	MCFG_DEVICE_REPLACE(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_ALL_1);
#define MCFG_NVRAM_REPLACE_RANDOM_FILL(_tag) \
	MCFG_DEVICE_REPLACE(_tag, NVRAM, 0) \
	nvram_device::static_set_default_value(*device, nvram_device::DEFAULT_RANDOM);
#define MCFG_NVRAM_REPLACE_CUSTOM_DRIVER(_tag, _class, _method) \
	MCFG_DEVICE_REPLACE(_tag, NVRAM, 0) \
	nvram_device::static_set_custom_handler(*device, nvram_init_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nvram_device;


// custom initialization for default state
typedef device_delegate<void (nvram_device &, void *, size_t)> nvram_init_delegate;


// ======================> nvram_device

class nvram_device :    public device_t,
						public device_nvram_interface
{
public:
	// values
	enum default_value
	{
		DEFAULT_ALL_0,
		DEFAULT_ALL_1,
		DEFAULT_RANDOM,
		DEFAULT_CUSTOM,
		DEFAULT_NONE
	};

	// construction/destruction
	nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_default_value(device_t &device, default_value value);
	static void static_set_custom_handler(device_t &device, nvram_init_delegate callback);

	// controls
	void set_base(void *base, size_t length) { m_base = base; m_length = length; }

protected:
	// device-level overrides
	virtual void device_start();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	// internal helpers
	void determine_final_base();

	// configuration state
	default_value               m_default_value;
	nvram_init_delegate         m_custom_handler;

	// runtime state
	void *                      m_base;
	size_t                      m_length;
};


// device type definition
extern const device_type NVRAM;


#endif
