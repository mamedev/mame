// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_KM035_H
#define MAME_MACHINE_KM035_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> km035_device

class km035_device : public device_t
{
public:
	// construction/destruction
	km035_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto tx_handler() { return m_tx_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

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
	required_device<i8035_device> m_maincpu;
	required_device<beep_device> m_speaker;

	required_ioport_array<16> m_kbd;

	int m_keylatch;                 // keyboard row latch
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_rx;

	devcb_write_line m_tx_handler;
	devcb_write_line m_rts_handler;

	void bus_w(uint8_t data);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(KM035, km035_device)

#endif // MAME_MACHINE_KM035_H
