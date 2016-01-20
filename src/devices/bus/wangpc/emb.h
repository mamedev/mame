// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM031-B Extended Memory Board emulation

**********************************************************************/

#pragma once

#ifndef __WANGPC_EMB__
#define __WANGPC_EMB__

#include "emu.h"
#include "wangpc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_emb_device

class wangpc_emb_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_emb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	optional_shared_ptr<UINT16> m_ram;
	UINT16 m_option;
	int m_parity_error;
	int m_parity_odd;
};


// device type definition
extern const device_type WANGPC_EMB;


#endif
