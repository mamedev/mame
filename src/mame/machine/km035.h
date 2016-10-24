// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#pragma once

#ifndef __KM035_H__
#define __KM035_H__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_KM035_TX_HANDLER(_cb) \
	devcb = &km035_device::set_tx_handler(*device, DEVCB_##_cb);

#define MCFG_KM035_RTS_HANDLER(_cb) \
	devcb = &km035_device::set_rts_handler(*device, DEVCB_##_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> km035_device

class km035_device : public device_t
{
public:
	// construction/destruction
	km035_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_tx_handler(device_t &device, _Object wr) { return downcast<km035_device &>(device).m_tx_handler.set_callback(wr); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object wr) { return downcast<km035_device &>(device).m_rts_handler.set_callback(wr); }

	void bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void write_rxd(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;

	required_ioport_array<16> m_kbd;

	int m_keylatch;                 // keyboard row latch
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_rx;

	devcb_write_line m_tx_handler;
	devcb_write_line m_rts_handler;
};

// device type definition
extern const device_type KM035;

#endif
