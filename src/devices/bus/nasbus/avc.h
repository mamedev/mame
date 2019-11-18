// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom Advanced Video Card

***************************************************************************/

#ifndef MAME_BUS_NASBUS_AVC_H
#define MAME_BUS_NASBUS_AVC_H

#pragma once

#include "nasbus.h"
#include "video/mc6845.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nascom_avc_device

class nascom_avc_device : public device_t, public device_nasbus_card_interface
{
public:
	// construction/destruction
	nascom_avc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_WRITE8_MEMBER(control_w);

	READ8_MEMBER(vram_r);
	WRITE8_MEMBER(vram_w);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	std::vector<uint8_t> m_r_ram;
	std::vector<uint8_t> m_g_ram;
	std::vector<uint8_t> m_b_ram;

	uint8_t m_control;
};

// device type definition
DECLARE_DEVICE_TYPE(NASCOM_AVC, nascom_avc_device)

#endif // MAME_BUS_NASBUS_AVC_H
