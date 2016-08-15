// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis High Resolution Graphics adapter emulation

**********************************************************************/

#pragma once

#ifndef __COMPIS_HRG__
#define __COMPIS_HRG__

#include "emu.h"
#include "graphics.h"
#include "video/upd7220.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_fdc_device

class compis_hrg_t : public device_t,
					 public device_compis_graphics_card_interface
{
public:
	// construction/destruction
	compis_hrg_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	UPD7220_DISPLAY_PIXELS_MEMBER( display_pixels );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_compis_graphics_card_interface overrides
	virtual UINT8 pcs6_6_r(address_space &space, offs_t offset) override;
	virtual void pcs6_6_w(address_space &space, offs_t offset, UINT8 data) override;

private:
	required_device<upd7220_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_video_ram;

	UINT8 m_unk_video;
};


// device type definition
extern const device_type COMPIS_HRG;


#endif
