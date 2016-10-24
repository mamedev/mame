// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __PCCARD_H__
#define __PCCARD_H__

#include "emu.h"

class pccard_interface
{
public:
	virtual uint16_t read_memory(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_reg(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual void write_memory(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_reg(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual ~pccard_interface() {}
};

extern const device_type PCCARD_SLOT;

class pccard_slot_device : public device_t,
	public device_slot_interface
{
public:
	pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	int read_line_inserted();
	uint16_t read_memory(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t read_reg(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write_memory(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_reg(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;

private:
	// internal state
	pccard_interface *m_pccard;
};

#endif
