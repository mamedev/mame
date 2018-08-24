// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    nvram.h

    Generic non-volatile RAM.

***************************************************************************/

#ifndef MAME_MACHINE_NVRAM_H
#define MAME_MACHINE_NVRAM_H

#pragma once

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
	nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, default_value value)
		: nvram_device(mconfig, tag, owner, 0)
	{
		set_default_value(value);
	}
	nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration helpers
	void set_default_value(default_value value) { m_default_value = value; }
	template <typename Object> void set_custom_handler(Object &&cb)
	{
		m_default_value = DEFAULT_CUSTOM;
		m_custom_handler = std::forward<Object>(cb);
	}
	template <class FunctionClass> void set_custom_handler(const char *devname, void (FunctionClass::*callback)(nvram_device &, void *, size_t), const char *name)
	{
		set_custom_handler(init_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_custom_handler(void (FunctionClass::*callback)(nvram_device &, void *, size_t), const char *name)
	{
		set_custom_handler(init_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

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

DECLARE_DEVICE_TYPE(NVRAM, nvram_device)

#endif // MAME_DEVICES_MACHINE_NVRAM_H
