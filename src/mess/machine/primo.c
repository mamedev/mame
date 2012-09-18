/*******************************************************************************

    primo.c

    Functions to emulate general aspects of Microkey Primo computers
    (RAM, ROM, interrupts, I/O ports)

    Krzysztof Strzecha

*******************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/primo.h"

/* Components */
#include "cpu/z80/z80.h"
#include "machine/cbmiec.h"
#include "sound/speaker.h"

/* Devices */
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "imagedev/cartslot.h"



/*******************************************************************************

    Interrupt callback

*******************************************************************************/

INTERRUPT_GEN_MEMBER(primo_state::primo_vblank_interrupt)
{
	if (m_nmi)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/*******************************************************************************

    Memory banking

*******************************************************************************/

static void primo_update_memory(running_machine &machine)
{
	primo_state *state = machine.driver_data<primo_state>();
	address_space& space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	switch (state->m_port_FD & 0x03)
	{
		case 0x00:	/* Original ROM */
			space.unmap_write(0x0000, 0x3fff);
			state->membank("bank1")->set_base(machine.root_device().memregion("maincpu")->base()+0x10000);
			break;
		case 0x01:	/* EPROM extension 1 */
			space.unmap_write(0x0000, 0x3fff);
			state->membank("bank1")->set_base(machine.root_device().memregion("maincpu")->base()+0x14000);
			break;
		case 0x02:	/* RAM */
			space.install_write_bank(0x0000, 0x3fff, "bank1");
			state->membank("bank1")->set_base(machine.root_device().memregion("maincpu")->base());
			break;
		case 0x03:	/* EPROM extension 2 */
			space.unmap_write(0x0000, 0x3fff);
			state->membank("bank1")->set_base(state->memregion("maincpu")->base()+0x18000);
			break;
	}
	logerror ("Memory update: %02x\n", state->m_port_FD);
}

/*******************************************************************************

    IO read/write handlers

*******************************************************************************/

READ8_MEMBER(primo_state::primo_be_1_r)
{
	UINT8 data = 0x00;
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };

	// bit 7, 6 - not used

	// bit 5 - VBLANK
	data |= (machine().primary_screen->vblank()) ? 0x20 : 0x00;

	// bit 4 - I4 (external bus)

	// bit 3 - I3 (external bus)

	// bit 2 - cassette
	data |= ((machine().device<cassette_image_device>(CASSETTE_TAG))->input() < 0.1) ? 0x04 : 0x00;

	// bit 1 - reset button
	data |= (ioport("RESET")->read()) ? 0x02 : 0x00;

	// bit 0 - keyboard
	data |= (ioport(portnames[(offset & 0x0030) >> 4])->read() >> (offset&0x000f)) & 0x0001 ? 0x01 : 0x00;

	return data;
}

READ8_MEMBER(primo_state::primo_be_2_r)
{
	UINT8 data = 0xff;

	// bit 7, 6 - not used

	// bit 5 - SCLK
	if (!m_iec->clk_r())
		data &= ~0x20;

	// bit 4 - SDATA
	if (!m_iec->data_r())
		data &= ~0x10;

	// bit 3 - SRQ
	if (!m_iec->srq_r())
		data &= ~0x08;

	// bit 2 - joystic 2 (not implemeted yet)

	// bit 1 - ATN
	if (!m_iec->atn_r())
		data &= ~0x02;

	// bit 0 - joystic 1 (not implemeted yet)

	logerror ("IOR BE-2 data:%02x\n", data);
	return data;
}

WRITE8_MEMBER(primo_state::primo_ki_1_w)
{
	device_t *speaker = machine().device(SPEAKER_TAG);
	// bit 7 - NMI generator enable/disable
	m_nmi = (data & 0x80) ? 1 : 0;

	// bit 6 - joystick register shift (not emulated)

	// bit 5 - V.24 (2) / tape control (not emulated)

	// bit 4 - speaker
	speaker_level_w(speaker, (data&0x10)>>4);

	// bit 3 - display buffer
	if (data & 0x08)
		m_video_memory_base |= 0x2000;
	else
		m_video_memory_base &= 0xdfff;

	// bit 2 - V.24 (1) / tape control (not emulated)

	// bit 1, 0 - cassette output
	switch (data & 0x03)
	{
		case 0:
			machine().device<cassette_image_device>(CASSETTE_TAG)->output(-1.0);
			break;
		case 1:
		case 2:
			machine().device<cassette_image_device>(CASSETTE_TAG)->output(0.0);
			break;
		case 3:
			machine().device<cassette_image_device>(CASSETTE_TAG)->output(1.0);
			break;
	}
}

WRITE8_MEMBER(primo_state::primo_ki_2_w)
{

	// bit 7, 6 - not used

	// bit 5 - SCLK
	m_iec->clk_w(!BIT(data, 5));

	// bit 4 - SDATA
	m_iec->data_w(!BIT(data, 4));

	// bit 3 - not used

	// bit 2 - SRQ
	m_iec->srq_w(!BIT(data, 2));

	// bit 1 - ATN
	m_iec->atn_w(!BIT(data, 1));

	// bit 0 - not used

//  logerror ("IOW KI-2 data:%02x\n", data);
}

WRITE8_MEMBER(primo_state::primo_FD_w)
{
	if (!ioport("MEMORY_EXPANSION")->read())
	{
		m_port_FD = data;
		primo_update_memory(machine());
	}
}

/*******************************************************************************

    Driver initialization

*******************************************************************************/

static void primo_common_driver_init (primo_state *state)
{
	state->m_port_FD = 0x00;
}

DRIVER_INIT_MEMBER(primo_state,primo32)
{
	primo_common_driver_init(this);
	m_video_memory_base = 0x6800;
}

DRIVER_INIT_MEMBER(primo_state,primo48)
{
	primo_common_driver_init(this);
	m_video_memory_base = 0xa800;
}

DRIVER_INIT_MEMBER(primo_state,primo64)
{
	primo_common_driver_init(this);
	m_video_memory_base = 0xe800;
}

/*******************************************************************************

    Machine initialization

*******************************************************************************/

static void primo_common_machine_init (running_machine &machine)
{
	primo_state *state = machine.driver_data<primo_state>();
	if (machine.root_device().ioport("MEMORY_EXPANSION")->read())
		state->m_port_FD = 0x00;
	primo_update_memory(machine);
	machine.device("maincpu")->set_clock_scale(machine.root_device().ioport("CPU_CLOCK")->read() ? 1.5 : 1.0);
}

void primo_state::machine_reset()
{
	primo_common_machine_init(machine());
}

MACHINE_RESET_MEMBER(primo_state,primob)
{
	primo_common_machine_init(machine());

//removed   cbm_drive_0_config(SERIAL, 8);
//removed   cbm_drive_1_config(SERIAL, 9);
}

/*******************************************************************************

    Snapshot files (.pss)

*******************************************************************************/

static void primo_setup_pss (running_machine &machine, UINT8* snapshot_data, UINT32 snapshot_size)
{
	primo_state *state = machine.driver_data<primo_state>();
	int i;
	device_t *speaker = machine.device(SPEAKER_TAG);

	/* Z80 registers */

	machine.device("maincpu")->state().set_state_int(Z80_BC, snapshot_data[4] + snapshot_data[5]*256);
	machine.device("maincpu")->state().set_state_int(Z80_DE, snapshot_data[6] + snapshot_data[7]*256);
	machine.device("maincpu")->state().set_state_int(Z80_HL, snapshot_data[8] + snapshot_data[9]*256);
	machine.device("maincpu")->state().set_state_int(Z80_AF, snapshot_data[10] + snapshot_data[11]*256);
	machine.device("maincpu")->state().set_state_int(Z80_BC2, snapshot_data[12] + snapshot_data[13]*256);
	machine.device("maincpu")->state().set_state_int(Z80_DE2, snapshot_data[14] + snapshot_data[15]*256);
	machine.device("maincpu")->state().set_state_int(Z80_HL2, snapshot_data[16] + snapshot_data[17]*256);
	machine.device("maincpu")->state().set_state_int(Z80_AF2, snapshot_data[18] + snapshot_data[19]*256);
	machine.device("maincpu")->state().set_state_int(Z80_PC, snapshot_data[20] + snapshot_data[21]*256);
	machine.device("maincpu")->state().set_state_int(Z80_SP, snapshot_data[22] + snapshot_data[23]*256);
	machine.device("maincpu")->state().set_state_int(Z80_I, snapshot_data[24]);
	machine.device("maincpu")->state().set_state_int(Z80_R, snapshot_data[25]);
	machine.device("maincpu")->state().set_state_int(Z80_IX, snapshot_data[26] + snapshot_data[27]*256);
	machine.device("maincpu")->state().set_state_int(Z80_IY, snapshot_data[28] + snapshot_data[29]*256);


	/* IO ports */

	// KI-1 bit 7 - NMI generator enable/disable
	state->m_nmi = (snapshot_data[30] & 0x80) ? 1 : 0;

	// KI-1 bit 4 - speaker
	speaker_level_w(speaker, (snapshot_data[30]&0x10)>>4);


	/* memory */

	for (i=0; i<0xc000; i++)
		machine.device("maincpu")->memory().space(AS_PROGRAM)->write_byte( i+0x4000, snapshot_data[i+38]);
}

SNAPSHOT_LOAD( primo )
{
	UINT8 *snapshot_data;

	if (!(snapshot_data = (UINT8*) malloc(snapshot_size)))
		return IMAGE_INIT_FAIL;

	if (image.fread( snapshot_data, snapshot_size) != snapshot_size)
	{
		free(snapshot_data);
		return IMAGE_INIT_FAIL;
	}

	if (strncmp((char *)snapshot_data, "PS01", 4))
	{
		free(snapshot_data);
		return IMAGE_INIT_FAIL;
	}

	primo_setup_pss(image.device().machine(),snapshot_data, snapshot_size);

	free(snapshot_data);
	return IMAGE_INIT_PASS;
}

/*******************************************************************************

    Quicload files (.pp)

*******************************************************************************/


static void primo_setup_pp (running_machine &machine,UINT8* quickload_data, UINT32 quickload_size)
{
	int i;

	UINT16 load_addr;
	UINT16 start_addr;

	load_addr = quickload_data[0] + quickload_data[1]*256;
	start_addr = quickload_data[2] + quickload_data[3]*256;

	for (i=4; i<quickload_size; i++)
		machine.device("maincpu")->memory().space(AS_PROGRAM)->write_byte(start_addr+i-4, quickload_data[i]);

	machine.device("maincpu")->state().set_state_int(Z80_PC, start_addr);

	logerror ("Quickload .pp l: %04x r: %04x s: %04x\n", load_addr, start_addr, quickload_size-4);
}

QUICKLOAD_LOAD( primo )
{
	UINT8 *quickload_data;

	if (!(quickload_data = (UINT8*) malloc(quickload_size)))
		return IMAGE_INIT_FAIL;

	if (image.fread( quickload_data, quickload_size) != quickload_size)
	{
		free(quickload_data);
		return IMAGE_INIT_FAIL;
	}

	primo_setup_pp(image.device().machine(), quickload_data, quickload_size);

	free(quickload_data);
	return IMAGE_INIT_PASS;
}
