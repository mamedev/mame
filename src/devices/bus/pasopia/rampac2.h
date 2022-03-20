// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_PASOPIA_RAMPAC2_H
#define MAME_BUS_PASOPIA_RAMPAC2_H

#pragma once

#include "pac2.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pasopia_rampac2_device

class pasopia_rampac2_device : public device_t, public pac2_card_interface, public device_nvram_interface
{
protected:
	// construction/destruction
	pasopia_rampac2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 ram_size);

	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// pac2_card_interface overrides
	virtual u8 pac2_read(offs_t offset) override;
	virtual void pac2_write(offs_t offset, u8 data) override;

private:
	const u32 m_ram_size;

	// internal state
	std::unique_ptr<u8[]> m_ram;
	u16 m_ram_index;
};

// ======================> pasopia_pa7243_device

class pasopia_pa7243_device : public pasopia_rampac2_device
{
public:
	// device type constructor
	pasopia_pa7243_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> pasopia_pa7245_device

class pasopia_pa7245_device : public pasopia_rampac2_device
{
public:
	// device type constructor
	pasopia_pa7245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> pasopia_pa7248_device

class pasopia_pa7248_device : public pasopia_rampac2_device
{
public:
	// device type constructor
	pasopia_pa7248_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(PASOPIA_PA7243, pasopia_pa7243_device)
DECLARE_DEVICE_TYPE(PASOPIA_PA7245, pasopia_pa7245_device)
DECLARE_DEVICE_TYPE(PASOPIA_PA7248, pasopia_pa7248_device)

#endif // MAME_BUS_PASOPIA_RAMPAC2_H
