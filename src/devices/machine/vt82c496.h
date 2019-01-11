// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C496G "Green PC" system chipset

*/

#ifndef MAME_MACHINE_VT82C496_H
#define MAME_MACHINE_VT82C496_H

#pragma once

#include "ram.h"

#define MCFG_VT82C496_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, VT82C496, 0)

#define MCFG_VT82C496_CPU( _tag ) \
	downcast<vt82c496_device &>(*device).set_cpu(_tag);

#define MCFG_VT82C496_REGION( _tag ) \
	downcast<vt82c496_device &>(*device).set_region(_tag);

class vt82c496_device :  public device_t
{
public:
	// construction/destruction
	vt82c496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu(const char *tag) { m_cpu_tag = tag; }
	void set_region(const char *tag) { m_region_tag = tag; }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	const char* m_cpu_tag;
	const char* m_region_tag;
//  cpu_device* m_maincpu;
	address_space* m_space;
	ram_device* m_ram;
	uint8_t* m_rom;

	uint8_t m_reg[0x100];
	uint8_t m_reg_select;

	void update_mem_c0(uint8_t data);
	void update_mem_d0(uint8_t data);
	void update_mem_e0(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(VT82C496, vt82c496_device)

#endif // MAME_MACHINE_VT82C496_H
