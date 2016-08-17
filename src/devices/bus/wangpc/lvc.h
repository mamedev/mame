// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC Low-Resolution Video Controller emulation

**********************************************************************/

#pragma once

#ifndef __WANGPC_LVC__
#define __WANGPC_LVC__

#include "emu.h"
#include "wangpc.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_lvc_device

class wangpc_lvc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	MC6845_UPDATE_ROW( crtc_update_row );
	DECLARE_WRITE_LINE_MEMBER( vsync_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_wangpcbus_card_interface overrides
	virtual UINT16 wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask) override;
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data) override;
	virtual UINT16 wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask) override;
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data) override;

private:
	inline void set_irq(int state);

	required_device<mc6845_device> m_crtc;
	optional_shared_ptr<UINT16> m_video_ram;

	rgb_t m_palette[16];
	UINT8 m_option;
	UINT16 m_scroll;
	int m_irq;
};


// device type definition
extern const device_type WANGPC_LVC;


#endif
