// license:BSD-3-Clause
// copyright-holders:Ramiro Polla, Felipe Sanches
/*
 * Epson LX-810L dot matrix printer emulation
 *
 */

#pragma once

#ifndef __EPSON_LX810L__
#define __EPSON_LX810L__

#include "emu.h"
#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a30.h"
#include "machine/eepromser.h"
#include "machine/steppers.h"
#include "sound/dac.h"


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

// ======================> epson_lx810l_t

class epson_lx810l_t : public device_t,
						public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx810l_t(const machine_config &mconfig, const char *tag,
					device_t *owner, uint32_t clock);
	epson_lx810l_t(const machine_config &mconfig, device_type type,
					const char *name, const char *tag, device_t *owner,
					uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* Extended Timer Output */
	void co0_w(int state);

	/* ADC */
	uint8_t an0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t an7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	/* fake memory I/O to get past memory reset check */
	uint8_t fakemem_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fakemem_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* GATE ARRAY */
	void printhead(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pf_stepper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cr_stepper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void e05a30_ready(int state);

	/* Centronics stuff */
	virtual void input_strobe(int state) override { if (m_e05a30) m_e05a30->centronics_input_strobe(state); }
	virtual void input_data0(int state) override { if (m_e05a30) m_e05a30->centronics_input_data0(state); }
	virtual void input_data1(int state) override { if (m_e05a30) m_e05a30->centronics_input_data1(state); }
	virtual void input_data2(int state) override { if (m_e05a30) m_e05a30->centronics_input_data2(state); }
	virtual void input_data3(int state) override { if (m_e05a30) m_e05a30->centronics_input_data3(state); }
	virtual void input_data4(int state) override { if (m_e05a30) m_e05a30->centronics_input_data4(state); }
	virtual void input_data5(int state) override { if (m_e05a30) m_e05a30->centronics_input_data5(state); }
	virtual void input_data6(int state) override { if (m_e05a30) m_e05a30->centronics_input_data6(state); }
	virtual void input_data7(int state) override { if (m_e05a30) m_e05a30->centronics_input_data7(state); }
	void e05a30_centronics_ack(int state) { output_ack(state); }
	void e05a30_centronics_busy(int state) { output_busy(state); }
	void e05a30_centronics_perror(int state) { output_perror(state); }
	void e05a30_centronics_fault(int state) { output_fault(state); }
	void e05a30_centronics_select(int state) { output_select(state); }

	/* Panel buttons */
	void online_sw(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	/* Video hardware (simulates paper) */
	uint32_t screen_update_lx810l(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
#define uabs(x) ((x) > 0 ? (x) : -(x))
	unsigned int bitmap_line(int i) { return ((uabs(m_pf_pos_abs) / 6) + i) % m_bitmap.height(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
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
};

// ======================> epson_ap2000_t

class epson_ap2000_t : public epson_lx810l_t
{
public:
	// construction/destruction
	epson_ap2000_t(const machine_config &mconfig, const char *tag,
					device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// device type definition
extern const device_type EPSON_LX810L;
extern const device_type EPSON_AP2000;

#endif
