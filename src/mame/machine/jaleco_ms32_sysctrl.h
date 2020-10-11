// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Jaleco MS32 System Control Unit

***************************************************************************/

#ifndef MAME_MACHINE_JALECO_MS32_SYSCTRL_H
#define MAME_MACHINE_JALECO_MS32_SYSCTRL_H

#pragma once

#include "screen.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jaleco_ms32_sysctrl_device

class jaleco_ms32_sysctrl_device : public device_t,
								   public device_memory_interface,
								   public device_video_interface
{
public:
	// construction/destruction
	jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 read(offs_t offset, u16 mem_mask = ~0);


protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;
	void io_map(address_map &map);
	
private:
	void control_w(u16 data);
	void hblank_w(u16 data);
	void hdisplay_w(u16 data);
	void hbp_w(u16 data);
	void hfp_w(u16 data);
	void vblank_w(u16 data);
	void vdisplay_w(u16 data);
	void vbp_w(u16 data);
	void vfp_w(u16 data);

	int m_flipscreen;
	u8 m_dotclock;
	struct {
		u16 horz_blank, horz_display, vert_blank, vert_display;
	}m_crtc;
	inline u16 crtc_write_reg(u16 raw_data);
	inline void crtc_refresh_screen_params();
};


// device type definition
DECLARE_DEVICE_TYPE(JALECO_MS32_SYSCTRL, jaleco_ms32_sysctrl_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_JALECO_MS32_SYSCTRL_H
