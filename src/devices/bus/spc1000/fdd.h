// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SPC1000_FDD_H
#define MAME_BUS_SPC1000_FDD_H

#pragma once

#include "exp.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spc1000_fdd_exp_device

class spc1000_fdd_exp_device : public device_t, public device_spc1000_card_interface
{
public:
	// construction/destruction
	spc1000_fdd_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	TIMER_CALLBACK_MEMBER(tc_off);

	// internal state
	required_device<z80_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<i8255_device> m_ppi;
	required_device_array<floppy_connector, 2> m_fd;

	emu_timer *m_timer_tc;

	uint8_t m_i8255_0_pc;
	uint8_t m_i8255_1_pc;
	uint8_t m_i8255_portb;

	void i8255_b_w(uint8_t data);
	uint8_t i8255_c_r();
	void i8255_c_w(uint8_t data);

	uint8_t tc_r();
	void control_w(uint8_t data);

	void sd725_io(address_map &map) ATTR_COLD;
	void sd725_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SPC1000_FDD_EXP, spc1000_fdd_exp_device)

#endif // MAME_BUS_SPC1000_FDD_H
