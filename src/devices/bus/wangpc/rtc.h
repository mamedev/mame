// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM040-B Remote Telecommunication controller emulation

**********************************************************************/

#ifndef MAME_BUS_WANGPC_RTC_H
#define MAME_BUS_WANGPC_RTC_H

#pragma once

#include "wangpc.h"
#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_rtc_device

class wangpc_rtc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_wangpcbus_card_interface overrides
	virtual uint16_t wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) override;
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) override;

private:
	required_device<z80_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80sio_device> m_sio;
	memory_share_creator<uint8_t> m_char_ram;

	void wangpc_rtc_io(address_map &map) ATTR_COLD;
	void wangpc_rtc_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_RTC, wangpc_rtc_device)

#endif // MAME_BUS_WANGPC_RTC_H
