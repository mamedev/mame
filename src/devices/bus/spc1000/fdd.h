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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

private:
	static constexpr device_timer_id TIMER_TC = 0;

	// internal state
	required_device<z80_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<i8255_device> m_ppi;

	floppy_image_device *m_fd0;
	floppy_image_device *m_fd1;

	emu_timer *m_timer_tc;

	uint8_t m_i8255_0_pc;
	uint8_t m_i8255_1_pc;
	uint8_t m_i8255_portb;

	DECLARE_WRITE8_MEMBER(i8255_b_w);
	DECLARE_READ8_MEMBER(i8255_c_r);
	DECLARE_WRITE8_MEMBER(i8255_c_w);

	DECLARE_READ8_MEMBER(tc_r);
	DECLARE_WRITE8_MEMBER(control_w);

	void sd725_io(address_map &map);
	void sd725_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(SPC1000_FDD_EXP, spc1000_fdd_exp_device)

#endif // MAME_BUS_SPC1000_FDD_H
