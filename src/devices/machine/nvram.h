// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    nvram.h

    Generic non-volatile RAM.

***************************************************************************/

#ifndef MAME_MACHINE_NVRAM_H
#define MAME_MACHINE_NVRAM_H

#pragma once



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
	nvram_device::static_set_custom_handler(*device, nvram_device::init_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

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
	nvram_device::static_set_custom_handler(*device, nvram_device::init_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nvram_device

class nvram_device :    public device_t,
						public device_nvram_interface
{
public:
	// custom initialization for default state
	typedef device_delegate<void (nvram_device &, void *, size_t)> init_delegate;

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
	nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	static void static_set_default_value(device_t &device, default_value value);
	static void static_set_custom_handler(device_t &device, init_delegate &&callback);

	// controls
	void set_base(void *base, size_t length) { m_base = base; m_length = length; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	// internal helpers
	void determine_final_base();

	// configuration state
	optional_memory_region      m_region;
	default_value               m_default_value;
	init_delegate         m_custom_handler;

	// runtime state
	void *                      m_base;
	size_t                      m_length;
};


// device type definition
DECLARE_DEVICE_TYPE(NVRAM, nvram_device)


#endif // MAME_DEVICES_MACHINE_NVRAM_H
