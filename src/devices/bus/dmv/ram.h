// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_RAM_H__
#define __DMV_RAM_H__

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
	dmv_ram_device(const machine_config &mconfig, device_type type, uint32_t size, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;

	// dmvcart_interface overrides
	virtual void ram_read(uint8_t cas, offs_t offset, uint8_t &data) override;
	virtual void ram_write(uint8_t cas, offs_t offset, uint8_t data) override;

private:
	uint8_t *     m_ram;
	uint8_t       m_size;
};


class dmv_k200_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class dmv_k202_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class dmv_k208_device :
		public dmv_ram_device
{
public:
	// construction/destruction
	dmv_k208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
extern const device_type DMV_K200;
extern const device_type DMV_K202;
extern const device_type DMV_K208;

#endif  /* __DMV_RAM_H__ */
