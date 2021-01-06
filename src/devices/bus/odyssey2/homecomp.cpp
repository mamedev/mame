// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac+ C7420 Home Computer Module emulation

This module only works on the Videopac+ G7400, although the cartridge can still
be inserted into the Odyssey 2/G7000.

Hardware notes:
- Z80 @ 3.547MHz
- 2*8KB ROM, 16KB RAM(2*TMS4416, 2 unpopulated sockets)
- optional data recorder

TODO:
- lots of unacknowledged writes to latch 1, probably harmless
- cassette data saved from MAME can be loaded fine, but other WAVs can't, even
  when they are good quality, maybe a filter on the data input?

******************************************************************************/

#include "emu.h"
#include "homecomp.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(O2_ROM_HOMECOMP, o2_homecomp_device, "o2_homecomp", "Odyssey 2 Videopac+ C7420")


//-------------------------------------------------
//  o2_homecomp_device - constructor
//-------------------------------------------------

o2_homecomp_device::o2_homecomp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_HOMECOMP, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_latch(*this, "latch%u", 0),
	m_cass(*this, "cassette")
{ }

void o2_homecomp_device::device_start()
{
	save_item(NAME(m_control));
}

void o2_homecomp_device::cart_init()
{
	if (m_rom_size != 0x800 || m_exrom_size != 0x4000)
		fatalerror("o2_homecomp_device: Wrong ROM region size\n");
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u8 o2_homecomp_device::internal_io_r(offs_t offset)
{
	u8 data = 0;

	// A7: input latch
	if (~offset & 0x80)
		data |= m_latch[1]->read();

	// A6: other i/o
	if (~offset & 0x40)
	{
		// d0: latch 0 status
		data |= m_latch[0]->pending_r() ^ 1;

		if (m_cass->is_playing() && m_cass->motor_on())
		{
			// d6: cass clock
			// d7: cass data
			double level = m_cass->input();
			if (level > 0.04)
				data |= 0xc0;
			else if (level < -0.04)
				data |= 0x80;
		}
	}

	return data;
}

void o2_homecomp_device::internal_io_w(offs_t offset, u8 data)
{
	// A7: output latch
	if (~offset & 0x80)
		m_latch[0]->write(data);

	// A6: other i/o
	if (~offset & 0x40)
	{
		// d7: cass remote control (also with data)
		m_cass->set_motor((data & 0x81) ? 1 : 0);

		// d0: cass data
		// d1: cass clock
		double level = 0.0;
		if (data & 1)
			level = (data & 2) ? 0.8 : -0.8;
		m_cass->output(level);
	}
}

void o2_homecomp_device::homecomp_mem(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(o2_homecomp_device::internal_rom_r));
	map(0x8000, 0xbfff).ram();
}

void o2_homecomp_device::homecomp_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(o2_homecomp_device::internal_io_r), FUNC(o2_homecomp_device::internal_io_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void o2_homecomp_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.547_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &o2_homecomp_device::homecomp_mem);
	m_maincpu->set_addrmap(AS_IO, &o2_homecomp_device::homecomp_io);

	GENERIC_LATCH_8(config, m_latch[0]);
	GENERIC_LATCH_8(config, m_latch[1]);

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	SPEAKER(config, "cass_output").front_center(); // on data recorder
	m_cass->add_route(ALL_OUTPUTS, "cass_output", 0.05);
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

void o2_homecomp_device::write_p1(u8 data)
{
	// P10: Z80 INT pin
	// P10+P11: Z80 RESET pin
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 3) ? CLEAR_LINE : ASSERT_LINE);

	// P11: must be low to access latch 0
	// P14: must be low to access latch 1
	m_control = data;
}

u8 o2_homecomp_device::io_read(offs_t offset)
{
	if ((offset & 0xa0) == 0xa0 && ~m_control & 2)
		return m_latch[0]->read();
	else
		return 0xff;
}

void o2_homecomp_device::io_write(offs_t offset, u8 data)
{
	if (offset & 0x80 && ~m_control & 0x10)
		m_latch[1]->write(data);
}
