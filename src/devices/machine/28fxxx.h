// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_28FXXX_H
#define MAME_MACHINE_28FXXX_H

#pragma once

class base_28fxxx_device : public device_t, public device_nvram_interface
{
public:
	enum commands
	{
		READ_MEMORY         = 0x00,
		ERASE               = 0x20,
		PROGRAM             = 0x40,
		READ_IDENTIFIER_ALT = 0x80, // defined in AMD datasheet, but not Intel
		READ_IDENTIFIER     = 0x90,
		ERASE_VERIFY        = 0xa0,
		PROGRAM_VERIFY      = 0xc0,
		RESET               = 0xff
	};

	void vpp(int state) { m_program_power = state; }
	u8 read(address_space &space, offs_t offset, u8 mem_mask = ~0);
	void write(offs_t offset, u8 data);

protected:
	base_28fxxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 size, u8 manufacturer_code, u8 device_code);

	virtual void device_start() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	optional_memory_region m_region;

private:
	void erase();

	// device specific parameters
	const u32 m_size;
	const u8 m_manufacturer_code;
	const u8 m_device_code;

	// accessible device state
	int m_program_power;
	std::unique_ptr<u8[]> m_data;

	// internal state
	enum state : u8
	{
		STATE_READ_MEMORY,
		STATE_READ_IDENTIFIER,
		STATE_ERASE_SETUP,
		STATE_ERASE,
		STATE_ERASE_RESET,
		STATE_ERASE_VERIFY,
		STATE_PROGRAM_SETUP,
		STATE_PROGRAM,
		STATE_PROGRAM_VERIFY
	};
	state m_state;

	u32 m_address_latch;
};

class intel_28f010_device : public base_28fxxx_device
{
public:
	intel_28f010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class amd_28f010_device : public base_28fxxx_device
{
public:
	amd_28f010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class amd_28f020_device : public base_28fxxx_device
{
public:
	amd_28f020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(INTEL_28F010, intel_28f010_device)
DECLARE_DEVICE_TYPE(AMD_28F010, amd_28f010_device)
DECLARE_DEVICE_TYPE(AMD_28F020, amd_28f020_device)

#endif // MAME_MACHINE_28FXXX_H
