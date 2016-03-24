// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom Advanced Video Card

***************************************************************************/

#pragma once

#ifndef __NASBUS_AVC_H__
#define __NASBUS_AVC_H__

#include "emu.h"
#include "nasbus.h"
#include "video/mc6845.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nascom_avc_device

class nascom_avc_device : public device_t, public device_nasbus_card_interface
{
public:
	// construction/destruction
	nascom_avc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_WRITE8_MEMBER(control_w);

	READ8_MEMBER(vram_r);
	WRITE8_MEMBER(vram_w);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	std::vector<UINT8> m_r_ram;
	std::vector<UINT8> m_g_ram;
	std::vector<UINT8> m_b_ram;

	UINT8 m_control;
};

// device type definition
extern const device_type NASCOM_AVC;

#endif // __NASBUS_AVC_H__
