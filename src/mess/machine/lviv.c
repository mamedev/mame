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

#define LVIV_SNAPSHOT_SIZE	82219





static void lviv_update_memory (running_machine &machine)
{
	lviv_state *state = machine.driver_data<lviv_state>();
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (state->m_ppi_port_outputs[0][2] & 0x02)
	{
		state->membank("bank1")->set_base(ram);
		state->membank("bank2")->set_base(ram + 0x4000);
	}
	else
	{
		state->membank("bank1")->set_base(ram + 0x8000);
		state->membank("bank2")->set_base(ram + 0xc000);
	}
}

static TIMER_CALLBACK( lviv_reset )
{
	machine.schedule_soft_reset();
}

DIRECT_UPDATE_MEMBER(lviv_state::lviv_directoverride)
{
	if (ioport("RESET")->read() & 0x01)
		machine().scheduler().timer_set(attotime::from_usec(10), FUNC(lviv_reset));
	return address;
}

static READ8_DEVICE_HANDLER ( lviv_ppi_0_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( lviv_ppi_0_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( lviv_ppi_0_portc_r )
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	UINT8 data = state->m_ppi_port_outputs[0][2] & 0x0f;
	if (device->machine().device<cassette_image_device>(CASSETTE_TAG)->input() > 0.038)
		data |= 0x10;
	if (state->m_ppi_port_outputs[0][0] & state->ioport("JOY")->read())
		data |= 0x80;
	return data;
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_0_porta_w )
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	state->m_ppi_port_outputs[0][0] = data;
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_0_portb_w )
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	state->m_ppi_port_outputs[0][1] = data;
	lviv_update_palette(device->machine(), data&0x7f);
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_0_portc_w )	/* tape in/out, video memory on/off */
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	device_t *speaker = device->machine().device(SPEAKER_TAG);
	state->m_ppi_port_outputs[0][2] = data;
	if (state->m_ppi_port_outputs[0][1]&0x80)
		speaker_level_w(speaker, data&0x01);
	device->machine().device<cassette_image_device>(CASSETTE_TAG)->output((data & 0x01) ? -1.0 : 1.0);
	lviv_update_memory(device->machine());
}

static READ8_DEVICE_HANDLER ( lviv_ppi_1_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( lviv_ppi_1_portb_r )	/* keyboard reading */
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	return	((state->m_ppi_port_outputs[1][0] & 0x01) ? 0xff : device->machine().root_device().ioport("KEY0")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x02) ? 0xff : device->machine().root_device().ioport("KEY1")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x04) ? 0xff : device->machine().root_device().ioport("KEY2")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x08) ? 0xff : device->machine().root_device().ioport("KEY3")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x10) ? 0xff : device->machine().root_device().ioport("KEY4")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x20) ? 0xff : device->machine().root_device().ioport("KEY5")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x40) ? 0xff : device->machine().root_device().ioport("KEY6")->read()) &
		((state->m_ppi_port_outputs[1][0] & 0x80) ? 0xff : state->ioport("KEY7")->read());
}

static READ8_DEVICE_HANDLER ( lviv_ppi_1_portc_r )     /* keyboard reading */
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	return	((state->m_ppi_port_outputs[1][2] & 0x01) ? 0xff : device->machine().root_device().ioport("KEY8")->read()) &
		((state->m_ppi_port_outputs[1][2] & 0x02) ? 0xff : device->machine().root_device().ioport("KEY9" )->read()) &
		((state->m_ppi_port_outputs[1][2] & 0x04) ? 0xff : device->machine().root_device().ioport("KEY10")->read()) &
		((state->m_ppi_port_outputs[1][2] & 0x08) ? 0xff : state->ioport("KEY11")->read());
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_1_porta_w )	/* kayboard scaning */
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	state->m_ppi_port_outputs[1][0] = data;
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_1_portb_w )
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	state->m_ppi_port_outputs[1][1] = data;
}

static WRITE8_DEVICE_HANDLER ( lviv_ppi_1_portc_w )	/* kayboard scaning */
{
	lviv_state *state = device->machine().driver_data<lviv_state>();
	state->m_ppi_port_outputs[1][2] = data;
}


/* I/O */
READ8_MEMBER(lviv_state::lviv_io_r)
{
	if (m_startup_mem_map)
	{
		return 0;	/* ??? */
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
			return 0;	/* ??? */
		}
	}
}

WRITE8_MEMBER(lviv_state::lviv_io_w)
{
	address_space *cpuspace = machine().device("maincpu")->memory().space(AS_PROGRAM);
	if (m_startup_mem_map)
	{
		UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();

		m_startup_mem_map = 0;

		cpuspace->install_write_bank(0x0000, 0x3fff, "bank1");
		cpuspace->install_write_bank(0x4000, 0x7fff, "bank2");
		cpuspace->install_write_bank(0x8000, 0xbfff, "bank3");
		cpuspace->unmap_write(0xC000, 0xffff);

		membank("bank1")->set_base(ram);
		membank("bank2")->set_base(ram + 0x4000);
		membank("bank3")->set_base(ram + 0x8000);
		membank("bank4")->set_base(machine().root_device().memregion("maincpu")->base() + 0x010000);
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


I8255A_INTERFACE( lviv_ppi8255_interface_0 )
{
	DEVCB_HANDLER(lviv_ppi_0_porta_r),
	DEVCB_HANDLER(lviv_ppi_0_porta_w),
	DEVCB_HANDLER(lviv_ppi_0_portb_r),
	DEVCB_HANDLER(lviv_ppi_0_portb_w),
	DEVCB_HANDLER(lviv_ppi_0_portc_r),
	DEVCB_HANDLER(lviv_ppi_0_portc_w)
};

I8255A_INTERFACE( lviv_ppi8255_interface_1 )
{
	DEVCB_HANDLER(lviv_ppi_1_porta_r),
	DEVCB_HANDLER(lviv_ppi_1_porta_w),
	DEVCB_HANDLER(lviv_ppi_1_portb_r),
	DEVCB_HANDLER(lviv_ppi_1_portb_w),
	DEVCB_HANDLER(lviv_ppi_1_portc_r),
	DEVCB_HANDLER(lviv_ppi_1_portc_w)
};

void lviv_state::machine_reset()
{
	address_space *space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *mem;

	space->set_direct_update_handler(direct_update_delegate(FUNC(lviv_state::lviv_directoverride), this));

	m_video_ram = machine().device<ram_device>(RAM_TAG)->pointer() + 0xc000;

	m_startup_mem_map = 1;

	space->unmap_write(0x0000, 0x3fff);
	space->unmap_write(0x4000, 0x7fff);
	space->unmap_write(0x8000, 0xbfff);
	space->unmap_write(0xC000, 0xffff);

	mem = memregion("maincpu")->base();
	membank("bank1")->set_base(mem + 0x010000);
	membank("bank2")->set_base(mem + 0x010000);
	membank("bank3")->set_base(mem + 0x010000);
	membank("bank4")->set_base(mem + 0x010000);

	/*machine().scheduler().timer_pulse(TIME_IN_NSEC(200), FUNC(lviv_draw_pixel));*/

	/*memset(machine().device<ram_device>(RAM_TAG)->pointer(), 0, sizeof(unsigned char)*0xffff);*/
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

static void lviv_setup_snapshot (running_machine &machine,UINT8 * data)
{
	lviv_state *state = machine.driver_data<lviv_state>();
	unsigned char lo,hi;

	/* Set registers */
	lo = data[0x14112] & 0x0ff;
	hi = data[0x14111] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_BC, (hi << 8) | lo);
	lo = data[0x14114] & 0x0ff;
	hi = data[0x14113] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_DE, (hi << 8) | lo);
	lo = data[0x14116] & 0x0ff;
	hi = data[0x14115] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_HL, (hi << 8) | lo);
	lo = data[0x14118] & 0x0ff;
	hi = data[0x14117] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_AF, (hi << 8) | lo);
	lo = data[0x14119] & 0x0ff;
	hi = data[0x1411a] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_SP, (hi << 8) | lo);
	lo = data[0x1411b] & 0x0ff;
	hi = data[0x1411c] & 0x0ff;
	machine.device("maincpu")->state().set_state_int(I8085_PC, (hi << 8) | lo);

	/* Memory dump */
	memcpy (machine.device<ram_device>(RAM_TAG)->pointer(), data+0x0011, 0xc000);
	memcpy (machine.device<ram_device>(RAM_TAG)->pointer()+0xc000, data+0x10011, 0x4000);

	/* Ports */
	state->m_ppi_port_outputs[0][0] = data[0x14011+0xc0];
	state->m_ppi_port_outputs[0][1] = data[0x14011+0xc1];
	lviv_update_palette(machine, state->m_ppi_port_outputs[0][1]&0x7f);
	state->m_ppi_port_outputs[0][2] = data[0x14011+0xc2];
	lviv_update_memory(machine);
}

static void dump_registers(running_machine &machine)
{
	logerror("PC   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_PC));
	logerror("SP   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_SP));
	logerror("AF   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_AF));
	logerror("BC   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_BC));
	logerror("DE   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_DE));
	logerror("HL   = %04x\n", (unsigned) machine.device("maincpu")->state().state_int(I8085_HL));
}

static int lviv_verify_snapshot (UINT8 * data, UINT32 size)
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

SNAPSHOT_LOAD( lviv )
{
	UINT8 *lviv_snapshot_data;

	lviv_snapshot_data = (UINT8*)malloc(LVIV_SNAPSHOT_SIZE);
	if (!lviv_snapshot_data)
	{
		logerror ("Unable to load snapshot file\n");
		return IMAGE_INIT_FAIL;
	}

	image.fread( lviv_snapshot_data, LVIV_SNAPSHOT_SIZE);

	if( lviv_verify_snapshot(lviv_snapshot_data, snapshot_size) == IMAGE_VERIFY_FAIL)
	{
		free(lviv_snapshot_data);
		return IMAGE_INIT_FAIL;
	}

	lviv_setup_snapshot (image.device().machine(),lviv_snapshot_data);

	dump_registers(image.device().machine());

	free(lviv_snapshot_data);

	logerror("Snapshot file loaded\n");
	return IMAGE_INIT_PASS;
}

