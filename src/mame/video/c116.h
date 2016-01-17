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
	namco_c116_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	//read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	//getters
	UINT16 get_reg(int reg) { return m_regs[reg]; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	std::vector<UINT8> m_ram_r;
	std::vector<UINT8> m_ram_g;
	std::vector<UINT8> m_ram_b;
	UINT16 m_regs[8];
};

extern const device_type NAMCO_C116;

#endif
