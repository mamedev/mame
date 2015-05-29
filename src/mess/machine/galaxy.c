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

READ8_MEMBER(galaxy_state::galaxy_keyboard_r)
{
	if (offset == 0)
	{
		double level = m_cassette->input();
		return (level >  0) ? 0xfe : 0xff;
	}
	else
	{
		return m_io_ports[(offset>>3) & 0x07]->read() & (0x01<<(offset & 0x07)) ? 0xfe : 0xff;
	}
}

WRITE8_MEMBER(galaxy_state::galaxy_latch_w)
{
	double val = (((data >>6) & 1 ) + ((data >> 2) & 1) - 1) * 32000;
	m_latch_value = data;
	m_cassette->output(val);
}



/***************************************************************************
  Interrupts
***************************************************************************/

INTERRUPT_GEN_MEMBER(galaxy_state::galaxy_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

IRQ_CALLBACK_MEMBER(galaxy_state::galaxy_irq_callback)
{
	galaxy_set_timer();
	m_interrupts_enabled = TRUE;
	return 0xff;
}

/***************************************************************************
  Snapshot files (GAL)
***************************************************************************/

#define GALAXY_SNAPSHOT_V1_SIZE 8268
#define GALAXY_SNAPSHOT_V2_SIZE 8244

void galaxy_state::galaxy_setup_snapshot (const UINT8 * data, UINT32 size)
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
			m_maincpu->set_state_int(Z80_IFF2, (UINT64)0);

			m_maincpu->set_state_int(Z80_HALT, (UINT64)0);

			m_maincpu->set_state_int(Z80_IM,   (data[0x18] >> 1) & 0x03);

			m_maincpu->set_state_int(Z80_I,    data[0x19]);
			m_maincpu->set_state_int(Z80_R,    data[0x1a]);

			memcpy (m_ram->pointer(), data + 0x0834, (m_ram->size() < 0x1800) ? m_ram->size() : 0x1800);

			break;
	}

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

SNAPSHOT_LOAD_MEMBER( galaxy_state, galaxy )
{
	UINT8* snapshot_data;

	switch (snapshot_size)
	{
		case GALAXY_SNAPSHOT_V1_SIZE:
		case GALAXY_SNAPSHOT_V2_SIZE:
			snapshot_data = auto_alloc_array(machine(), UINT8, snapshot_size);
			break;
		default:
			return IMAGE_INIT_FAIL;
	}

	image.fread( snapshot_data, snapshot_size);

	galaxy_setup_snapshot(snapshot_data, snapshot_size);

	return IMAGE_INIT_PASS;
}


/***************************************************************************
  Driver Initialization
***************************************************************************/

DRIVER_INIT_MEMBER(galaxy_state,galaxy)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7" };

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_bank( 0x2800, 0x2800 + m_ram->size() - 1, "bank1");
	membank("bank1")->set_base(m_ram->pointer());

	if (m_ram->size() < (6 + 48) * 1024)
	{
		space.nop_readwrite( 0x2800 + m_ram->size(), 0xffff);
	}

	for ( int i = 0; i < 8; i++ )
	{
		m_io_ports[i] = ioport( keynames[i] );
	}
}

/***************************************************************************
  Machine Initialization
***************************************************************************/

MACHINE_RESET_MEMBER(galaxy_state,galaxy)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* ROM 2 enable/disable */
	if (ioport("ROM2")->read()) {
		space.install_read_bank(0x1000, 0x1fff, "bank10");
	} else {
		space.nop_read(0x1000, 0x1fff);
	}
	space.nop_write(0x1000, 0x1fff);

	if (ioport("ROM2")->read())
		membank("bank10")->set_base(memregion("maincpu")->base() + 0x1000);

	m_interrupts_enabled = TRUE;
}

DRIVER_INIT_MEMBER(galaxy_state,galaxyp)
{
	DRIVER_INIT_CALL(galaxy);
}

MACHINE_RESET_MEMBER(galaxy_state,galaxyp)
{
	UINT8 *ROM = memregion("maincpu")->base();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	ROM[0x0037] = 0x29;
	ROM[0x03f9] = 0xcd;
	ROM[0x03fa] = 0x00;
	ROM[0x03fb] = 0xe0;

	space.install_read_bank(0xe000, 0xefff, "bank11");
	space.nop_write(0xe000, 0xefff);
	membank("bank11")->set_base(memregion("maincpu")->base() + 0xe000);
	m_interrupts_enabled = TRUE;
}
