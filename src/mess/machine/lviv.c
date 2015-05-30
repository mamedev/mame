// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  machine.c

  Functions to emulate general aspects of PK-01 Lviv (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "includes/lviv.h"
#include "sound/speaker.h"
#include "machine/ram.h"

#define LVIV_SNAPSHOT_SIZE  82219





void lviv_state::lviv_update_memory ()
{
	UINT8 *ram = m_ram->pointer();

	if (m_ppi_port_outputs[0][2] & 0x02)
	{
		membank("bank1")->set_base(ram);
		membank("bank2")->set_base(ram + 0x4000);
	}
	else
	{
		membank("bank1")->set_base(ram + 0x8000);
		membank("bank2")->set_base(ram + 0xc000);
	}
}

INPUT_CHANGED_MEMBER(lviv_state::lviv_reset)
{
	machine().schedule_soft_reset();
}

READ8_MEMBER(lviv_state::lviv_ppi_0_porta_r)
{
	return 0xff;
}

READ8_MEMBER(lviv_state::lviv_ppi_0_portb_r)
{
	return 0xff;
}

READ8_MEMBER(lviv_state::lviv_ppi_0_portc_r)
{
	UINT8 data = m_ppi_port_outputs[0][2] & 0x0f;
	if (m_cassette->input() > 0.038)
		data |= 0x10;
	if (m_ppi_port_outputs[0][0] & ioport("JOY")->read())
		data |= 0x80;
	return data;
}

WRITE8_MEMBER(lviv_state::lviv_ppi_0_porta_w)
{
	m_ppi_port_outputs[0][0] = data;
}

WRITE8_MEMBER(lviv_state::lviv_ppi_0_portb_w)
{
	m_ppi_port_outputs[0][1] = data;
	lviv_update_palette(data&0x7f);
}

WRITE8_MEMBER(lviv_state::lviv_ppi_0_portc_w)/* tape in/out, video memory on/off */
{
	m_ppi_port_outputs[0][2] = data;
	if (m_ppi_port_outputs[0][1]&0x80)
		m_speaker->level_w(data & 0x01);
	m_cassette->output((data & 0x01) ? -1.0 : 1.0);
	lviv_update_memory();
}

READ8_MEMBER(lviv_state::lviv_ppi_1_porta_r)
{
	return 0xff;
}

READ8_MEMBER(lviv_state::lviv_ppi_1_portb_r)/* keyboard reading */
{
	return  ((m_ppi_port_outputs[1][0] & 0x01) ? 0xff : ioport("KEY0")->read()) &
		((m_ppi_port_outputs[1][0] & 0x02) ? 0xff : ioport("KEY1")->read()) &
		((m_ppi_port_outputs[1][0] & 0x04) ? 0xff : ioport("KEY2")->read()) &
		((m_ppi_port_outputs[1][0] & 0x08) ? 0xff : ioport("KEY3")->read()) &
		((m_ppi_port_outputs[1][0] & 0x10) ? 0xff : ioport("KEY4")->read()) &
		((m_ppi_port_outputs[1][0] & 0x20) ? 0xff : ioport("KEY5")->read()) &
		((m_ppi_port_outputs[1][0] & 0x40) ? 0xff : ioport("KEY6")->read()) &
		((m_ppi_port_outputs[1][0] & 0x80) ? 0xff : ioport("KEY7")->read());
}

READ8_MEMBER(lviv_state::lviv_ppi_1_portc_r)/* keyboard reading */
{
	return  ((m_ppi_port_outputs[1][2] & 0x01) ? 0xff : ioport("KEY8")->read()) &
		((m_ppi_port_outputs[1][2] & 0x02) ? 0xff : ioport("KEY9" )->read()) &
		((m_ppi_port_outputs[1][2] & 0x04) ? 0xff : ioport("KEY10")->read()) &
		((m_ppi_port_outputs[1][2] & 0x08) ? 0xff : ioport("KEY11")->read());
}

WRITE8_MEMBER(lviv_state::lviv_ppi_1_porta_w)/* kayboard scaning */
{
	m_ppi_port_outputs[1][0] = data;
}

WRITE8_MEMBER(lviv_state::lviv_ppi_1_portb_w)
{
	m_ppi_port_outputs[1][1] = data;
}

WRITE8_MEMBER(lviv_state::lviv_ppi_1_portc_w)/* kayboard scaning */
{
	m_ppi_port_outputs[1][2] = data;
}


/* I/O */
READ8_MEMBER(lviv_state::lviv_io_r)
{
	if (m_startup_mem_map)
	{
		return 0;   /* ??? */
	}
	else
	{
		switch ((offset >> 4) & 0x3)
		{
		case 0:
			return machine().device<i8255_device>("ppi8255_0")->read(space, offset & 3);

		case 1:
			return machine().device<i8255_device>("ppi8255_1")->read(space, offset & 3);

		case 2:
		case 3:
		default:
			/* reserved for extension? */
			return 0;   /* ??? */
		}
	}
}

WRITE8_MEMBER(lviv_state::lviv_io_w)
{
	address_space &cpuspace = m_maincpu->space(AS_PROGRAM);
	if (m_startup_mem_map)
	{
		UINT8 *ram = m_ram->pointer();

		m_startup_mem_map = 0;

		cpuspace.install_write_bank(0x0000, 0x3fff, "bank1");
		cpuspace.install_write_bank(0x4000, 0x7fff, "bank2");
		cpuspace.install_write_bank(0x8000, 0xbfff, "bank3");
		cpuspace.unmap_write(0xC000, 0xffff);

		membank("bank1")->set_base(ram);
		membank("bank2")->set_base(ram + 0x4000);
		membank("bank3")->set_base(ram + 0x8000);
		membank("bank4")->set_base(memregion("maincpu")->base() + 0x010000);
	}
	else
	{
		switch ((offset >> 4) & 0x3)
		{
		case 0:
			machine().device<i8255_device>("ppi8255_0")->write(space, offset & 3, data);
			break;

		case 1:
			machine().device<i8255_device>("ppi8255_1")->write(space, offset & 3, data);
			break;

		case 2:
		case 3:
			/* reserved for extension? */
			break;
		}
	}
}


void lviv_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *mem;

	m_video_ram = m_ram->pointer() + 0xc000;

	m_startup_mem_map = 1;

	space.unmap_write(0x0000, 0x3fff);
	space.unmap_write(0x4000, 0x7fff);
	space.unmap_write(0x8000, 0xbfff);
	space.unmap_write(0xC000, 0xffff);

	mem = memregion("maincpu")->base();
	membank("bank1")->set_base(mem + 0x010000);
	membank("bank2")->set_base(mem + 0x010000);
	membank("bank3")->set_base(mem + 0x010000);
	membank("bank4")->set_base(mem + 0x010000);

	/*machine().scheduler().timer_pulse(TIME_IN_NSEC(200), FUNC(lviv_draw_pixel));*/

	/*memset(m_ram->pointer(), 0, sizeof(unsigned char)*0xffff);*/
}


/*******************************************************************************
Lviv snapshot files (SAV)
-------------------------

00000 - 0000D:  'LVOV/DUMP/2.0/' (like LVT-header)
0000E - 0000F:  'H+' (something additional)
00010           00h
00011 - 0C010:  RAM (0000 - BFFF)
0C011 - 10010:  ROM (C000 - FFFF)
10011 - 14010:  Video RAM (4000 - 7FFF)
14011 - 14110:  Ports map (00 - FF)
14111 - 1411C:  Registers (B,C,D,E,H,L,A,F,SP,PC)
1411D - 1412A:  ??? (something additional)
*******************************************************************************/

void lviv_state::lviv_setup_snapshot (UINT8 * data)
{
	unsigned char lo,hi;

	/* Set registers */
	lo = data[0x14112] & 0x0ff;
	hi = data[0x14111] & 0x0ff;
	m_maincpu->set_state_int(I8085_BC, (hi << 8) | lo);
	lo = data[0x14114] & 0x0ff;
	hi = data[0x14113] & 0x0ff;
	m_maincpu->set_state_int(I8085_DE, (hi << 8) | lo);
	lo = data[0x14116] & 0x0ff;
	hi = data[0x14115] & 0x0ff;
	m_maincpu->set_state_int(I8085_HL, (hi << 8) | lo);
	lo = data[0x14118] & 0x0ff;
	hi = data[0x14117] & 0x0ff;
	m_maincpu->set_state_int(I8085_AF, (hi << 8) | lo);
	lo = data[0x14119] & 0x0ff;
	hi = data[0x1411a] & 0x0ff;
	m_maincpu->set_state_int(I8085_SP, (hi << 8) | lo);
	lo = data[0x1411b] & 0x0ff;
	hi = data[0x1411c] & 0x0ff;
	m_maincpu->set_state_int(I8085_PC, (hi << 8) | lo);

	/* Memory dump */
	memcpy (m_ram->pointer(), data+0x0011, 0xc000);
	memcpy (m_ram->pointer()+0xc000, data+0x10011, 0x4000);

	/* Ports */
	m_ppi_port_outputs[0][0] = data[0x14011+0xc0];
	m_ppi_port_outputs[0][1] = data[0x14011+0xc1];
	lviv_update_palette(m_ppi_port_outputs[0][1]&0x7f);
	m_ppi_port_outputs[0][2] = data[0x14011+0xc2];
	lviv_update_memory();
}

void lviv_state::dump_registers()
{
	logerror("PC   = %04x\n", (unsigned) m_maincpu->state_int(I8085_PC));
	logerror("SP   = %04x\n", (unsigned) m_maincpu->state_int(I8085_SP));
	logerror("AF   = %04x\n", (unsigned) m_maincpu->state_int(I8085_AF));
	logerror("BC   = %04x\n", (unsigned) m_maincpu->state_int(I8085_BC));
	logerror("DE   = %04x\n", (unsigned) m_maincpu->state_int(I8085_DE));
	logerror("HL   = %04x\n", (unsigned) m_maincpu->state_int(I8085_HL));
}

int lviv_state::lviv_verify_snapshot (UINT8 * data, UINT32 size)
{
	const char* tag = "LVOV/DUMP/2.0/";

	if( strncmp( tag, (char*)data, strlen(tag) ) )
	{
		logerror("Not a Lviv snapshot\n");
		return IMAGE_VERIFY_FAIL;
	}

	if (size != LVIV_SNAPSHOT_SIZE)
	{
		logerror ("Incomplete snapshot file\n");
		return IMAGE_VERIFY_FAIL;
	}

	logerror("returning ID_OK\n");
	return IMAGE_VERIFY_PASS;
}

SNAPSHOT_LOAD_MEMBER( lviv_state, lviv )
{
	dynamic_buffer lviv_snapshot_data(LVIV_SNAPSHOT_SIZE);

	image.fread( &lviv_snapshot_data[0], LVIV_SNAPSHOT_SIZE);

	if(lviv_verify_snapshot(&lviv_snapshot_data[0], snapshot_size) == IMAGE_VERIFY_FAIL)
	{
		return IMAGE_INIT_FAIL;
	}

	lviv_setup_snapshot (&lviv_snapshot_data[0]);

	dump_registers();

	logerror("Snapshot file loaded\n");
	return IMAGE_INIT_PASS;
}
