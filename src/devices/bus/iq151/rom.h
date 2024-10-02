// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_IQ151_ROM_H
#define MAME_BUS_IQ151_ROM_H

#pragma once

#include "iq151.h"
#include "machine/i8255.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iq151_rom_device

class iq151_rom_device :
		public device_t,
		public device_iq151cart_interface
{
public:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// construction/destruction
	iq151_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// iq151cart_interface overrides
	virtual uint8_t* get_cart_base() override;

	required_region_ptr<uint8_t> m_rom;
};


// ======================> iq151_basic6_device

class iq151_basic6_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_basic6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
};

// ======================> iq151_basicg_device

class iq151_basicg_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_basicg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
};


// ======================> iq151_amos1_device

class iq151_amos1_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

	bool m_active;
};


// ======================> iq151_amos2_device

class iq151_amos2_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

	bool m_active;
};


// ======================> iq151_amos3_device

class iq151_amos3_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

	bool m_active;
};


// device type definition
DECLARE_DEVICE_TYPE(IQ151_BASIC6, iq151_basic6_device)
DECLARE_DEVICE_TYPE(IQ151_BASICG, iq151_basicg_device)
DECLARE_DEVICE_TYPE(IQ151_AMOS1,  iq151_amos1_device)
DECLARE_DEVICE_TYPE(IQ151_AMOS2,  iq151_amos2_device)
DECLARE_DEVICE_TYPE(IQ151_AMOS3,  iq151_amos3_device)

#endif  // MAME_BUS_IQ151_ROM_H
