// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC PM-001B Medium-Resolution Video Controller emulation

**********************************************************************/

#pragma once

#ifndef __WANGPC_MVC__
#define __WANGPC_MVC__

#include "wangpc.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_mvc_device

class wangpc_mvc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_mvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual uint16_t wangpcbus_mrdc_r(address_space &space, offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(address_space &space, offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) override;

private:
	inline void set_irq(int state);

	required_device<mc6845_device> m_crtc;
	optional_shared_ptr<uint16_t> m_video_ram;
	optional_shared_ptr<uint16_t> m_char_ram;
	optional_shared_ptr<uint16_t> m_bitmap_ram;

	uint8_t m_option;
	int m_irq;
};


// device type definition
extern const device_type WANGPC_MVC;


#endif
