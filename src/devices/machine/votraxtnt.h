// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  Votrax Type 'N Talk
*
******************************************************************************/

#ifndef MAME_MACHINE_VOTRAXTNT_H
#define MAME_MACHINE_VOTRAXTNT_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "sound/votrax.h"


class votraxtnt_device : public device_t
{
public:
	votraxtnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	void write_rxd(int state) { m_acia->write_rxd(state); }
	void write_cts(int state) { m_acia->write_cts(state); }

protected:
	virtual void device_start() override ATTR_COLD { }
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<m6802_cpu_device> m_maincpu;
	required_device<votrax_sc01_device> m_votrax;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_clock;

	devcb_write_line m_txd_handler;
	devcb_write_line m_rts_handler;

	void mem_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(VOTRAXTNT, votraxtnt_device)

#endif // MAME_MACHINE_VOTRAXTNT_H
