// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    CCS Model 7710 Asynchronous Serial Interface

*********************************************************************/

#ifndef MAME_BUS_A2BUS_CCS7710_H
#define MAME_BUS_A2BUS_CCS7710_H

#pragma once

#include "a2bus.h"
#include "machine/6850acia.h"
#include "machine/f4702.h"

class ccs7710_device : public device_t, public device_a2bus_card_interface
{
public:
	// device type constructor
	ccs7710_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2bus_card_interface overrides
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual bool take_c800() override { return false; }

private:
	// miscellaneous handlers
	void acia_irq_w(int state);
	void external_clock_w(int state);
	u8 baud_select_r(offs_t offset);

	// object finders
	required_device<acia6850_device> m_acia;
	required_device<f4702_device> m_brg;
	required_region_ptr<u8> m_firmware;
	required_ioport m_baud_select;

	// internal state
	bool m_external_clock;
};

// device type declaration
DECLARE_DEVICE_TYPE(A2BUS_CCS7710, ccs7710_device)

#endif // MAME_BUS_A2BUS_CCS7710_H
