// license:BSD-3-Clause
// copyright-holders:Ramiro Polla, Felipe Sanches
/*
 * Epson LX-810L dot matrix printer emulation
 *
 */

#ifndef MAME_BUS_CENTRONICS_EPSON_LX810L_H
#define MAME_BUS_CENTRONICS_EPSON_LX810L_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a30.h"
#include "machine/eepromser.h"
#include "machine/steppers.h"
#include "sound/dac.h"
#include "screen.h"


/* The printer starts printing at x offset 44 and stops printing at x
 * offset 1009, giving a total of 965 printable pixels. Supposedly, the
 * border at the far right would be at x offset 1053. I've chosen the
 * width for the paper as 1024, since it's a nicer number than 1053, so
 * an offset must be used to centralize the pixels.
 */
#define CR_OFFSET    (-14)
#define PAPER_WIDTH  1024
#define PAPER_HEIGHT 576

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_lx810l_device

class epson_lx810l_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx810l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* Centronics stuff */
	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override { m_e05a30->centronics_input_strobe(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { m_e05a30->centronics_input_data0(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { m_e05a30->centronics_input_data1(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { m_e05a30->centronics_input_data2(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { m_e05a30->centronics_input_data3(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { m_e05a30->centronics_input_data4(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { m_e05a30->centronics_input_data5(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { m_e05a30->centronics_input_data6(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { m_e05a30->centronics_input_data7(state); }

	/* Panel buttons */
	DECLARE_INPUT_CHANGED_MEMBER(online_sw);

protected:
	epson_lx810l_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(portc_w);

	/* fake memory I/O to get past memory reset check */
	DECLARE_READ8_MEMBER(fakemem_r);
	DECLARE_WRITE8_MEMBER(fakemem_w);

	/* Extended Timer Output */
	DECLARE_WRITE_LINE_MEMBER(co0_w);

	/* ADC */
	DECLARE_READ8_MEMBER(an0_r);
	DECLARE_READ8_MEMBER(an1_r);
	DECLARE_READ8_MEMBER(an2_r);
	DECLARE_READ8_MEMBER(an3_r);
	DECLARE_READ8_MEMBER(an4_r);
	DECLARE_READ8_MEMBER(an5_r);
	DECLARE_READ8_MEMBER(an6_r);
	DECLARE_READ8_MEMBER(an7_r);


	/* GATE ARRAY */
	DECLARE_WRITE16_MEMBER(printhead);
	DECLARE_WRITE8_MEMBER(pf_stepper);
	DECLARE_WRITE8_MEMBER(cr_stepper);
	DECLARE_WRITE_LINE_MEMBER(e05a30_ready);

	DECLARE_WRITE_LINE_MEMBER(e05a30_centronics_ack) { output_ack(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a30_centronics_busy) { output_busy(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a30_centronics_perror) { output_perror(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a30_centronics_fault) { output_fault(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a30_centronics_select) { output_select(state); }

	void lx810l_mem(address_map &map);

	/* Video hardware (simulates paper) */
	uint32_t screen_update_lx810l(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

#define uabs(x) ((x) > 0 ? (x) : -(x))
	unsigned int bitmap_line(int i) { return ((uabs(m_pf_pos_abs) / 6) + i) % m_bitmap.height(); }

	required_device<cpu_device> m_maincpu;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<e05a30_device> m_e05a30;
	required_device<screen_device> m_screen;

	int m_93c06_clk;
	int m_93c06_cs;
	uint16_t m_printhead;
	int m_pf_pos_abs;
	int m_cr_pos_abs;
	int m_real_cr_pos;
	int m_real_cr_steps;
	int m_real_cr_dir; /* 1 is going right, -1 is going left */
	uint8_t m_fakemem;
	bitmap_rgb32 m_bitmap;

	enum {
		TIMER_CR
	};

	emu_timer *m_cr_timer;
};

// ======================> epson_ap2000_t

class epson_ap2000_device : public epson_lx810l_device
{
public:
	// construction/destruction
	epson_ap2000_device(const machine_config &mconfig, const char *tag,
					device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_LX810L, epson_lx810l_device)
DECLARE_DEVICE_TYPE(EPSON_AP2000, epson_ap2000_device)

#endif // MAME_BUS_CENTRONICS_EPSON_LX810L_H
