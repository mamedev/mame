// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_RAM_H__
#define __DMV_RAM_H__

#include "emu.h"
#include "dmvbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================>

class dmv_ram_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_ram_device(const machine_config &mconfig, device_type type, UINT32 size, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

protected:
	// device-level overrides
	virtual void device_start() override;

	// dmvcart_interface overrides
	virtual void ram_read(UINT8 cas, offs_t offset, UINT8 &data) override;
	virtual void ram_write(UINT8 cas, offs_t offset, UINT8 data) override;

private:
	UINT8 *     m_ram;
	UINT8       m_size;
};


class dmv_k200_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k200_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class dmv_k202_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k202_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class dmv_k208_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k208_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type DMV_K200;
extern const device_type DMV_K202;
extern const device_type DMV_K208;

#endif  /* __DMV_RAM_H__ */
