// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C496G "Green PC" system chipset

*/

#ifndef MAME_MACHINE_VT82C496_H
#define MAME_MACHINE_VT82C496_H

#pragma once

#include "ram.h"


class vt82c496_device :  public device_t
{
public:
	// construction/destruction
	vt82c496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cputag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ramtag(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_isatag(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_cpu;
	address_space* m_space;
	required_device<ram_device> m_ram;
	required_region_ptr<uint32_t> m_rom;

	uint8_t m_reg[0x100];
	uint8_t m_reg_select;

	void update_mem_c0(uint8_t data);
	void update_mem_d0(uint8_t data);
	void update_mem_e0(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(VT82C496, vt82c496_device)

#endif // MAME_MACHINE_VT82C496_H
