// license:BSD-3-Clause
// copyright-holders:hap
/*

  The Final ChessCard by Tasc

*/

#ifndef MAME_BUS_ISA_FINALCHS_H
#define MAME_BUS_ISA_FINALCHS_H

#pragma once

#include "isa.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/gen_latch.h"


class isa8_finalchs_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<m65sc02_device> m_maincpu;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<generic_latch_8_device> m_sublatch;

	bool m_installed;

	DECLARE_READ8_MEMBER(finalchs_r);
	DECLARE_WRITE8_MEMBER(finalchs_w);

	void finalchs_mem(address_map &map);
};


DECLARE_DEVICE_TYPE(ISA8_FINALCHS, isa8_finalchs_device)

#endif // MAME_BUS_ISA_FINALCHS_H
