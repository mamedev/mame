// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II Shugart SASI host-adapter daughterboard (9R80758)

    The SASI personality of the disk daughterboard.  The Z80 reaches an
    SA1403D over SASI through the "u8" Z80PIO (ports 0x10-0x13): port A
    is the SASI data bus (PARDY clocks a 74LS74 to generate the per-byte
    ACK pulse on a read or a write), port B carries the control / status
    lines.  The SA1403D serves the floppies on LUN 0-2 and an ST-506
    rigid disk on LUN 3.

**********************************************************************/

#ifndef MAME_BUS_XEROX820_SASI_H
#define MAME_BUS_XEROX820_SASI_H

#pragma once

#include "dbslot.h"

#include "machine/nscsi_bus.h"
#include "machine/z80pio.h"


// ======================> x820_sasi_host_device

//  Line-level bridge between the "u8" Z80PIO and the nscsi bus (the
//  bigbord2 SASI host pattern).

class x820_sasi_host_device : public device_t, public nscsi_device_interface
{
public:
	x820_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t data_r();           // PIO port A in: bus data, ACK pulse (U11 via PARDY)
	void data_w(uint8_t data);  // PIO port A out: drive bus data, ACK pulse (U11 via PARDY)
	uint8_t ctrl_r();           // PIO port B in: phase lines
	void ctrl_w(uint8_t data);  // PIO port B out: SEL (bit 5), RST (bit 7)

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void scsi_ctrl_changed() override;

private:
	TIMER_CALLBACK_MEMBER(ack_off);

	static constexpr attotime SASI_PULSE = attotime::from_nsec(500);

	emu_timer *m_ack_timer;
};

DECLARE_DEVICE_TYPE(X820_SASI_HOST, x820_sasi_host_device)


// ======================> xerox820_sasi_device

class xerox820_sasi_device : public device_t, public device_xerox820_dbslot_card_interface
{
public:
	xerox820_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_xerox820_dbslot_card_interface implementation
	virtual uint8_t io_r(offs_t offset) override;
	virtual void io_w(offs_t offset, uint8_t data) override;
	virtual uint8_t personality() override { return 0x02; } // SASI host adapter

	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<z80pio_device> m_pio;
	required_device<x820_sasi_host_device> m_host;
	required_device<nscsi_bus_device> m_sasibus;
};


// device type declarations
DECLARE_DEVICE_TYPE(XEROX820_SASI, xerox820_sasi_device)

#endif // MAME_BUS_XEROX820_SASI_H
