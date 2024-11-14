// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat_fdc.h

    Implementation of the Agat 840K floppy controller card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_AGAT_FDC_H
#define MAME_BUS_A2BUS_AGAT_FDC_H

#pragma once

#include "a2bus.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"


#define MXCSR_SYNC      0x40
#define MXCSR_TR        0x80


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agat_fdc_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_agat_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t d14_i_b();
	uint8_t d15_i_a();
	uint8_t d15_i_c();
	void d14_o_c(uint8_t data);
	void d15_o_b(uint8_t data);
	void d15_o_c(uint8_t data);

	static void floppy_formats(format_registration &fr);

protected:
	// construction/destruction
	a2bus_agat_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	required_device<i8255_device> m_d14;
	required_device<i8255_device> m_d15;

private:
	required_device<floppy_connector> floppy0;
	required_device<floppy_connector> floppy1;

	uint64_t time_to_cycles(const attotime &tm);
	attotime cycles_to_time(uint64_t cycles);

	void lss_start();
	TIMER_CALLBACK_MEMBER(lss_sync);

	TIMER_CALLBACK_MEMBER(motor_off);

	floppy_image_device *floppy;
	int active, bits;
	uint8_t data_reg;
	uint16_t address;
	uint64_t cycles;

	u8 m_mxcs;
	int m_unit;
	int m_state;

	int m_seektime;
	int m_waittime;

	emu_timer *m_timer_lss;
	emu_timer *m_timer_seek;
	emu_timer *m_timer_motor;

	uint8_t *m_rom;
	uint8_t *m_rom_d6;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_AGAT_FDC, a2bus_agat_fdc_device)

#endif  // MAME_BUS_A2BUS_AGAT_FDC_H
