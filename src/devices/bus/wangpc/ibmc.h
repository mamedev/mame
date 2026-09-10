// license:BSD-3-Clause
// copyright-holders:Fausto Pracek
/**********************************************************************

    Wang PC-PM007 IBM PC Color Emulation Card

**********************************************************************/

#ifndef MAME_BUS_WANGPC_IBMC_H
#define MAME_BUS_WANGPC_IBMC_H

#pragma once

#include "wangpc.h"
#include "video/mc6845.h"
#include "screen.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_ibmc_device

class wangpc_ibmc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_ibmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;

private:
	inline bool ram_enabled() const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	memory_share_creator<uint16_t> m_ram;
	memory_share_creator<uint16_t> m_video_ram;
	memory_share_creator<uint16_t> m_char_ram;

	uint8_t m_option;
	uint8_t m_enable;
	uint8_t m_mode;
	uint8_t m_live;
	uint8_t m_control;
	uint8_t m_color;
	uint8_t m_reg30;
	uint8_t m_crtc_idx;
	uint8_t m_crtc_regs[32];
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_IBMC, wangpc_ibmc_device)

#endif // MAME_BUS_WANGPC_IBMC_H
