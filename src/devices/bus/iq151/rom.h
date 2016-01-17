// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __IQ151_ROM_H__
#define __IQ151_ROM_H__

#include "emu.h"
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
	// construction/destruction
	iq151_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// iq151cart_interface overrides
	virtual UINT8* get_cart_base() override;

	UINT8 * m_rom;
};


// ======================> iq151_basic6_device

class iq151_basic6_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_basic6_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
};

// ======================> iq151_basicg_device

class iq151_basicg_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_basicg_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
};


// ======================> iq151_amos1_device

class iq151_amos1_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void io_write(offs_t offset, UINT8 data) override;

	bool m_active;
};


// ======================> iq151_amos2_device

class iq151_amos2_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void io_write(offs_t offset, UINT8 data) override;

	bool m_active;
};


// ======================> iq151_amos3_device

class iq151_amos3_device :
		public iq151_rom_device
{
public:
	// construction/destruction
	iq151_amos3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// iq151cart_interface overrides
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void io_write(offs_t offset, UINT8 data) override;

	bool m_active;
};


// device type definition
extern const device_type IQ151_BASIC6;
extern const device_type IQ151_BASICG;
extern const device_type IQ151_AMOS1;
extern const device_type IQ151_AMOS2;
extern const device_type IQ151_AMOS3;

#endif  /* __IQ151_ROM_H__ */
