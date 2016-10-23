// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson

#pragma once

#ifndef __C116_H__
#define __C116_H__


//***************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************

class namco_c116_device :
	public device_t,
	public device_gfx_interface,
	public device_video_interface
{
public:
	//construction/destruction
	namco_c116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	//read/write handlers
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	//getters
	uint16_t get_reg(int reg) { return m_regs[reg]; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	std::vector<uint8_t> m_ram_r;
	std::vector<uint8_t> m_ram_g;
	std::vector<uint8_t> m_ram_b;
	uint16_t m_regs[8];
};

extern const device_type NAMCO_C116;

#endif
