// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
#ifndef MAME_VIDEO_NAMCO_C116_H
#define MAME_VIDEO_NAMCO_C116_H

#pragma once



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
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

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

DECLARE_DEVICE_TYPE(NAMCO_C116, namco_c116_device)

#endif // MAME_VIDEO_NAMCO_C116_H
