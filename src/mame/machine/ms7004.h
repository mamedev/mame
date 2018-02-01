// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_MS7004_H
#define MAME_MACHINE_MS7004_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "sound/beep.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MS7004_TX_HANDLER(_cb) \
	devcb = &ms7004_device::set_tx_handler(*device, DEVCB_##_cb);

#define MCFG_MS7004_RTS_HANDLER(_cb) \
	devcb = &ms7004_device::set_rts_handler(*device, DEVCB_##_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ms7004_device

class ms7004_device : public device_t //, public device_serial_interface
{
public:
	// construction/destruction
	ms7004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_tx_handler(device_t &device, Object &&wr) { return downcast<ms7004_device &>(device).m_tx_handler.set_callback(std::forward<Object>(wr)); }
	template <class Object> static devcb_base &set_rts_handler(device_t &device, Object &&wr) { return downcast<ms7004_device &>(device).m_rts_handler.set_callback(std::forward<Object>(wr)); }

	DECLARE_WRITE_LINE_MEMBER( write_rxd );

	void ms7004_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;
	required_device<i8243_device> m_i8243;

	required_ioport_array<16> m_kbd;

	int m_keylatch;                 // keyboard row latch
	uint8_t m_p1;
	uint8_t m_p2;

	devcb_write_line m_tx_handler;
	devcb_write_line m_rts_handler;

	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ_LINE_MEMBER( t1_r );
	DECLARE_WRITE8_MEMBER( i8243_port_w );
};

// device type definition
DECLARE_DEVICE_TYPE(MS7004, ms7004_device)

#endif // MAME_MACHINE_MS7004_H
