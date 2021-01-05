// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/galaxy.h"

/***************************************************************************
  I/O devices
***************************************************************************/

uint8_t galaxy_state::keyboard_r(offs_t offset)
{
	if (offset == 0)
	{
		double level = m_cassette->input();
		return (level >  0) ? 0xfe : 0xff;
	}
	else
		return m_io_keyboard[(offset>>3) & 0x07]->read() & (0x01<<(offset & 0x07)) ? 0xfe : 0xff;
}

void galaxy_state::latch_w(uint8_t data)
{
	double val = (((data >>6) & 1 ) + ((data >> 2) & 1) - 1) * 32000;
	m_latch_value = data;
	m_cassette->output(val);
}



/***************************************************************************
  Interrupts
***************************************************************************/

IRQ_CALLBACK_MEMBER(galaxy_state::irq_callback)
{
	set_timer();
	m_interrupts_enabled = true;
	return 0xff;
}

/***************************************************************************
  Snapshot files (GAL)
***************************************************************************/

#define GALAXY_SNAPSHOT_V1_SIZE 8268
#define GALAXY_SNAPSHOT_V2_SIZE 8244

void galaxy_state::setup_snapshot(const uint8_t * data, uint32_t size)
{
	switch (size)
	{
		case GALAXY_SNAPSHOT_V1_SIZE:
			m_maincpu->set_state_int(Z80_AF,   data[0x00] | data[0x01] << 8);
			m_maincpu->set_state_int(Z80_BC,   data[0x04] | data[0x05] << 8);
			m_maincpu->set_state_int(Z80_DE,   data[0x08] | data[0x09] << 8);
			m_maincpu->set_state_int(Z80_HL,   data[0x0c] | data[0x0d] << 8);
			m_maincpu->set_state_int(Z80_IX,   data[0x10] | data[0x11] << 8);
			m_maincpu->set_state_int(Z80_IY,   data[0x14] | data[0x15] << 8);
			m_maincpu->set_state_int(Z80_PC,   data[0x18] | data[0x19] << 8);
			m_maincpu->set_state_int(Z80_SP,   data[0x1c] | data[0x1d] << 8);
			m_maincpu->set_state_int(Z80_AF2,  data[0x20] | data[0x21] << 8);
			m_maincpu->set_state_int(Z80_BC2,  data[0x24] | data[0x25] << 8);
			m_maincpu->set_state_int(Z80_DE2,  data[0x28] | data[0x29] << 8);
			m_maincpu->set_state_int(Z80_HL2,  data[0x2c] | data[0x2d] << 8);
			m_maincpu->set_state_int(Z80_IFF1, data[0x30]);
			m_maincpu->set_state_int(Z80_IFF2, data[0x34]);
			m_maincpu->set_state_int(Z80_HALT, data[0x38]);
			m_maincpu->set_state_int(Z80_IM,   data[0x3c]);
			m_maincpu->set_state_int(Z80_I,    data[0x40]);
			m_maincpu->set_state_int(Z80_R,    (data[0x44] & 0x7f) | (data[0x48] & 0x80));

			memcpy (m_ram->pointer(), data + 0x084c, (m_ram->size() < 0x1800) ? m_ram->size() : 0x1800);

			break;
		case GALAXY_SNAPSHOT_V2_SIZE:
			m_maincpu->set_state_int(Z80_AF,   data[0x00] | data[0x01] << 8);
			m_maincpu->set_state_int(Z80_BC,   data[0x02] | data[0x03] << 8);
			m_maincpu->set_state_int(Z80_DE,   data[0x04] | data[0x05] << 8);
			m_maincpu->set_state_int(Z80_HL,   data[0x06] | data[0x07] << 8);
			m_maincpu->set_state_int(Z80_IX,   data[0x08] | data[0x09] << 8);
			m_maincpu->set_state_int(Z80_IY,   data[0x0a] | data[0x0b] << 8);
			m_maincpu->set_state_int(Z80_PC,   data[0x0c] | data[0x0d] << 8);
			m_maincpu->set_state_int(Z80_SP,   data[0x0e] | data[0x0f] << 8);
			m_maincpu->set_state_int(Z80_AF2,  data[0x10] | data[0x11] << 8);
			m_maincpu->set_state_int(Z80_BC2,  data[0x12] | data[0x13] << 8);
			m_maincpu->set_state_int(Z80_DE2,  data[0x14] | data[0x15] << 8);
			m_maincpu->set_state_int(Z80_HL2,  data[0x16] | data[0x17] << 8);

			m_maincpu->set_state_int(Z80_IFF1, data[0x18] & 0x01);
			m_maincpu->set_state_int(Z80_IFF2, (uint64_t)0);

			m_maincpu->set_state_int(Z80_HALT, (uint64_t)0);

			m_maincpu->set_state_int(Z80_IM,   (data[0x18] >> 1) & 0x03);

			m_maincpu->set_state_int(Z80_I,    data[0x19]);
			m_maincpu->set_state_int(Z80_R,    data[0x1a]);

			memcpy (m_ram->pointer(), data + 0x0834, (m_ram->size() < 0x1800) ? m_ram->size() : 0x1800);

			break;
	}

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

SNAPSHOT_LOAD_MEMBER(galaxy_state::snapshot_cb)
{
	uint8_t* snapshot_data;

	switch (snapshot_size)
	{
		case GALAXY_SNAPSHOT_V1_SIZE:
		case GALAXY_SNAPSHOT_V2_SIZE:
			snapshot_data = auto_alloc_array(machine(), uint8_t, snapshot_size);
			break;
		default:
			return image_init_result::FAIL;
	}

	image.fread( snapshot_data, snapshot_size);

	setup_snapshot(snapshot_data, snapshot_size);

	return image_init_result::PASS;
}


/***************************************************************************
  Driver Initialization
***************************************************************************/

void galaxy_state::init_galaxy()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram( 0x2800, 0x2800 + m_ram->size() - 1, m_ram->pointer());

	if (m_ram->size() < (6 + 48) * 1024)
		space.nop_readwrite( 0x2800 + m_ram->size(), 0xffff);
}

void galaxy_state::init_galaxyp()
{
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x0037] = 0x29;
	ROM[0x03f9] = 0xcd;
	ROM[0x03fa] = 0x00;
	ROM[0x03fb] = 0xe0;
}

/***************************************************************************
  Machine Initialization
***************************************************************************/

void galaxy_state::machine_reset()
{
	m_interrupts_enabled = true;
}

