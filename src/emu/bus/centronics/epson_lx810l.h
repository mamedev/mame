/*
 * Epson LX-810L dot matrix printer emulation
 *
 * Copyright: 2014 Ramiro Polla
 *                 Felipe Sanches
 * License: BSD-3-Clause
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
#include "sound/beep.h"
#include "sound/speaker.h"


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
					device_t *owner, UINT32 clock);
	epson_lx810l_t(const machine_config &mconfig, device_type type,
					const char *name, const char *tag, device_t *owner,
					UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(portc_w);

	/* Extended Timer Output */
	DECLARE_WRITE_LINE_MEMBER(co0_w);
	DECLARE_WRITE_LINE_MEMBER(co1_w);

	/* ADC */
	DECLARE_READ8_MEMBER(an0_r);
	DECLARE_READ8_MEMBER(an1_r);
	DECLARE_READ8_MEMBER(an2_r);
	DECLARE_READ8_MEMBER(an3_r);
	DECLARE_READ8_MEMBER(an4_r);
	DECLARE_READ8_MEMBER(an5_r);
	DECLARE_READ8_MEMBER(an6_r);
	DECLARE_READ8_MEMBER(an7_r);

	/* fake memory I/O to get past memory reset check */
	DECLARE_READ8_MEMBER(fakemem_r);
	DECLARE_WRITE8_MEMBER(fakemem_w);

	/* GATE ARRAY */
	DECLARE_WRITE16_MEMBER(printhead);
	DECLARE_WRITE8_MEMBER(pf_stepper);
	DECLARE_WRITE8_MEMBER(cr_stepper);
	DECLARE_WRITE_LINE_MEMBER(e05a30_ready);

	/* Panel buttons */
	DECLARE_INPUT_CHANGED_MEMBER(online_sw);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<speaker_sound_device> m_speaker;

	int m_93c06_clk;
	int m_93c06_cs;
	UINT16 m_printhead;
	int m_pf_pos_abs;
	int m_cr_pos_abs;
	int m_last_fire; /* HACK to get fire positions for motor in movement */
	UINT8 m_fakemem;
};

// ======================> epson_ap2000_t

class epson_ap2000_t : public epson_lx810l_t
{
public:
	// construction/destruction
	epson_ap2000_t(const machine_config &mconfig, const char *tag,
					device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
};


// device type definition
extern const device_type EPSON_LX810L;
extern const device_type EPSON_AP2000;

#endif
