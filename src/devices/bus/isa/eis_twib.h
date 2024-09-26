// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
#ifndef MAME_BUS_ISA_EIS_TWIB_H
#define MAME_BUS_ISA_EIS_TWIB_H

#pragma once

#include "isa.h"
#include "machine/z80sio.h"
#include "machine/timer.h"
#include "machine/sdlc.h"

class isa8_eistwib_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_eistwib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t twib_r(offs_t offset);
	void twib_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// devices
	required_device<i8274_device> m_uart8274;
	required_device<sdlc_logger_device> m_sdlclogger;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// Timers
	TIMER_DEVICE_CALLBACK_MEMBER(tick_bitclock);
	bool m_bitclock;
	bool m_rts;
	bool m_txd;

	// helpers
	required_ioport m_sw1;
	required_ioport m_isairq;
	bool m_installed;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_EIS_TWIB, isa8_eistwib_device)

#endif  // MAME_BUS_ISA_EIS_TWIB_H
