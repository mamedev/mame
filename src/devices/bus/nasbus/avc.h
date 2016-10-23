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
	nascom_avc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	MC6845_UPDATE_ROW(crtc_update_row);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t vram_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	std::vector<uint8_t> m_r_ram;
	std::vector<uint8_t> m_g_ram;
	std::vector<uint8_t> m_b_ram;

	uint8_t m_control;
};

// device type definition
extern const device_type NASCOM_AVC;

#endif // __NASBUS_AVC_H__
