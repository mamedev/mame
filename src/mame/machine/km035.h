// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_KM035_H
#define MAME_MACHINE_KM035_H

#pragma once

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

	DECLARE_WRITE_LINE_MEMBER( write_rxd );

	void km035_map(address_map &map);
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

	required_ioport_array<16> m_kbd;

	int m_keylatch;                 // keyboard row latch
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_rx;

	devcb_write_line m_tx_handler;
	devcb_write_line m_rts_handler;

	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_READ_LINE_MEMBER( t0_r );
	DECLARE_READ_LINE_MEMBER( t1_r );
};

// device type definition
DECLARE_DEVICE_TYPE(KM035, km035_device)

#endif // MAME_MACHINE_KM035_H
