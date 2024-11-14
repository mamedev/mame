// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    GROM emulation
    See tmc0430.cpp for documentation,

    Michael Zapf

    February 2012: Rewritten as class

***************************************************************************/

#ifndef MAME_MACHINE_TMC0430_H
#define MAME_MACHINE_TMC0430_H

#pragma once

DECLARE_DEVICE_TYPE(TMC0430, tmc0430_device)

enum
{
	GROM_M_LINE = 1,
	GROM_MO_LINE = 2
};

class tmc0430_device : public device_t
{
public:
	tmc0430_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *regionname, int offset, int ident) :
		tmc0430_device(mconfig, tag, owner, 0)
	{
		set_region_and_ident(regionname, offset, ident);
	}

	tmc0430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ready_cb() { return m_gromready.bind(); }

	void readz(uint8_t *value);
	void write(uint8_t data);
	void set_lines(line_state mline, line_state moline, line_state gsq);

	void m_line(int state);
	void mo_line(int state);
	void gsq_line(int state);
	void gclock_in(int state);

	void set_region_and_ident(const char *regionname, int offset, int ident)
	{
		m_regionname = regionname;
		m_offset = offset;
		m_ident = ident<<13;
	}

	int debug_get_address();

	// Allow for checking the state of the GROM so we can turn off the clock
	bool idle() { return (m_phase == 0 && m_current_clock_level==CLEAR_LINE); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	// Ready callback. This line is usually connected to the READY pin of the CPU.
	devcb_write_line   m_gromready;

	// Clock line level
	int m_current_clock_level;

	// Currently active GROM ident
	int m_current_ident;

	// Phase of the state machine
	int m_phase;

	// Address or data mode?
	bool m_address_mode;

	// Reading or writing?
	bool m_read_mode;

	// Selected?
	bool    m_selected;

	// Toggle for address loading
	bool        m_address_lowbyte;

	// Region name
	const char* m_regionname;

	// Offset in the region. We cannot rely on the ident because the GROMs
	// in the cartridges begin with ident 3, but start at the beginning of their region.
	int         m_offset;

	// Identification of this GROM (0-7 <<13)
	int         m_ident;

	// The address pointer is always expected to be in the range 0x0000 - 0xffff, even
	// when this GROM is not addressed.
	uint16_t m_address;

	/* GROM data buffer. */
	uint8_t m_buffer;

	/* Pointer to the memory region contained in this GROM. */
	uint8_t *m_memptr;
};

#endif // MAME_MACHINE_TMC0430_H
