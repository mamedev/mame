// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM040-B Remote Telecommunication controller emulation

**********************************************************************/

#pragma once

#ifndef __WANGPC_RTC__
#define __WANGPC_RTC__

#include "emu.h"
#include "wangpc.h"
#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_rtc_device

class wangpc_rtc_device : public device_t,
							public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_wangpcbus_card_interface overrides
	virtual UINT16 wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask);
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data);
	virtual UINT16 wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask);
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data);

private:
	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80dart_device> m_sio;
	optional_shared_ptr<UINT8> m_char_ram;
};


// device type definition
extern const device_type WANGPC_RTC;


#endif
