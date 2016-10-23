// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM001 Winchester Disk Controller emulation

**********************************************************************/

#pragma once

#ifndef __WANGPC_WDC__
#define __WANGPC_WDC__

#include "emu.h"
#include "wangpc.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"
#include "machine/z80ctc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_wdc_device

class wangpc_wdc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_wdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	uint8_t port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctc_ch0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_ch0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctc_ch1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_ch1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctc_ch2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_ch2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctc_ch3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_ch3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_mrdc_r(address_space &space, offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(address_space &space, offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint8_t wangpcbus_dack_r(address_space &space, int line) override;
	virtual void wangpcbus_dack_w(address_space &space, int line, uint8_t data) override;
	virtual bool wangpcbus_have_dack(int line) override;

private:
	inline void set_irq(int state);

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;

	uint8_t m_status;
	uint8_t m_option;
	int m_irq;
};


// device type definition
extern const device_type WANGPC_WDC;


#endif
