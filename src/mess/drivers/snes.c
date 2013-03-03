/***************************************************************************

  snes.c

  Driver file to handle emulation of the Nintendo Super NES.

  R. Belmont
  Anthony Kruize
  Angelo Salese
  Fabio Priuli
  byuu
  Based on the original MESS driver by Lee Hammerton (aka Savoury Snax)

  Driver is preliminary right now.

  The memory map included below is setup in a way to make it easier to handle
  Mode 20 and Mode 21 ROMs.

  Todo (in no particular order):
    - Fix additional sound bugs
    - Emulate extra chips - superfx, dsp2, sa-1 etc.
    - Add horizontal mosaic, hi-res. interlaced etc to video emulation.
    - Fix support for Mode 7. (In Progress)
    - Handle interleaved roms (maybe even multi-part roms, but how?)
    - Add support for running at 3.58 MHz at the appropriate time.
    - I'm sure there's lots more ...

***************************************************************************/

#include "emu.h"
#include "audio/snes_snd.h"
#include "cpu/spc700/spc700.h"
#include "cpu/superfx/superfx.h"
#include "cpu/g65816/g65816.h"
#include "cpu/upd7725/upd7725.h"
#include "includes/snes.h"
#include "machine/snescart.h"

#include "crsshair.h"

#define MAX_SNES_CART_SIZE 0x600000


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static READ8_DEVICE_HANDLER( spc_ram_100_r )
{
	return spc_ram_r(device, space, offset + 0x100);
}

static WRITE8_DEVICE_HANDLER( spc_ram_100_w )
{
	spc_ram_w(device, space, offset + 0x100, data);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( snes_map, AS_PROGRAM, 8, snes_state )
	AM_RANGE(0x000000, 0x2fffff) AM_READWRITE_LEGACY(snes_r_bank1, snes_w_bank1)    /* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE_LEGACY(snes_r_bank2, snes_w_bank2)    /* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x400000, 0x5fffff) AM_READ_LEGACY(snes_r_bank3)       /* ROM (and reserved in Mode 20) */
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE_LEGACY(snes_r_bank4, snes_w_bank4)    /* used by Mode 20 DSP-1 */
	AM_RANGE(0x700000, 0x7dffff) AM_READWRITE_LEGACY(snes_r_bank5, snes_w_bank5)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM                 /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE_LEGACY(snes_r_bank6, snes_w_bank6)    /* Mirror and ROM */
	AM_RANGE(0xc00000, 0xffffff) AM_READWRITE_LEGACY(snes_r_bank7, snes_w_bank7)    /* Mirror and ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( superfx_map, AS_PROGRAM, 8, snes_state )
	AM_RANGE(0x000000, 0x3fffff) AM_READWRITE_LEGACY(superfx_r_bank1, superfx_w_bank1)
	AM_RANGE(0x400000, 0x5fffff) AM_READWRITE_LEGACY(superfx_r_bank2, superfx_w_bank2)
	AM_RANGE(0x600000, 0x7dffff) AM_READWRITE_LEGACY(superfx_r_bank3, superfx_w_bank3)
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE_LEGACY(superfx_r_bank1, superfx_w_bank1)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE_LEGACY(superfx_r_bank2, superfx_w_bank2)
	AM_RANGE(0xe00000, 0xffffff) AM_READWRITE_LEGACY(superfx_r_bank3, superfx_w_bank3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spc_map, AS_PROGRAM, 8, snes_state )
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE_LEGACY("spc700", spc_ram_r, spc_ram_w) /* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE_LEGACY("spc700", spc_io_r, spc_io_w)   /* spc io */
	AM_RANGE(0x0100, 0xffff) AM_DEVREADWRITE_LEGACY("spc700", spc_ram_100_r, spc_ram_100_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_prg_map, AS_PROGRAM, 32, snes_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("dspprg", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_data_map, AS_DATA, 16, snes_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("dspdata", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( setadsp_prg_map, AS_PROGRAM, 32, snes_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("dspprg", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( setadsp_data_map, AS_DATA, 16, snes_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("dspdata", 0)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static CUSTOM_INPUT( snes_mouse_speed_input )
{
	snes_state *state = field.machine().driver_data<snes_state>();
	int port = (FPTR)param;

	if (snes_ram[OLDJOY1] & 0x1)
	{
		state->m_mouse[port].speed++;
		if ((state->m_mouse[port].speed & 0x03) == 0x03)
			state->m_mouse[port].speed = 0;
	}

	return state->m_mouse[port].speed;
}

static CUSTOM_INPUT( snes_superscope_offscreen_input )
{
	snes_state *state = field.machine().driver_data<snes_state>();
	int port = (FPTR)param;
	static const char *const portnames[2][3] =
			{
				{ "SUPERSCOPE1", "SUPERSCOPE1_X", "SUPERSCOPE1_Y" },
				{ "SUPERSCOPE2", "SUPERSCOPE2_X", "SUPERSCOPE2_Y" },
			};

	INT16 x = field.machine().root_device().ioport(portnames[port][1])->read();
	INT16 y = field.machine().root_device().ioport(portnames[port][2])->read();

	/* these are the theoretical boundaries, but we currently are always onscreen... */
	if (x < 0 || x >= SNES_SCR_WIDTH || y < 0 || y >= snes_ppu.beam.last_visible_line)
		state->m_scope[port].offscreen = 1;
	else
		state->m_scope[port].offscreen = 0;

	return state->m_scope[port].offscreen;
}

static TIMER_CALLBACK( lightgun_tick )
{
	if ((machine.root_device().ioport("CTRLSEL")->read() & 0x0f) == 0x03 || (machine.root_device().ioport("CTRLSEL")->read() & 0x0f) == 0x04)   {
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
	}

	if ((machine.root_device().ioport("CTRLSEL")->read() & 0xf0) == 0x30 || (machine.root_device().ioport("CTRLSEL")->read() & 0xf0) == 0x40)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_NONE);
	}
}


static INPUT_PORTS_START( snes_joypads )
	PORT_START("SERIAL1_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_START("SERIAL1_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P1 Select") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("SERIAL2_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_START("SERIAL2_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P2 Select") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)

	PORT_START("SERIAL1_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL1_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL2_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( snes_mouse )
	PORT_START("MOUSE1")
	/* bits 0,3 = mouse signature (must be 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 4,5 = mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused */
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_mouse_speed_input, (void *)0)
	/* bits 6,7 = mouse buttons */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Mouse Button Left") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Mouse Button Right") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MOUSE1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MOUSE1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MOUSE2")
	/* bits 0,3 = mouse signature (must be 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 4,5 = mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused */
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_mouse_speed_input, (void *)1)
	/* bits 6,7 = mouse buttons */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Mouse Button Left") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Mouse Button Right") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("MOUSE2_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("MOUSE2_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
INPUT_PORTS_END

static INPUT_PORTS_START( snes_superscope )
	PORT_START("SUPERSCOPE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    // Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_superscope_offscreen_input, (void *)0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port1 Superscope Pause") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port1 Superscope Turbo") PORT_TOGGLE PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port1 Superscope Cursor") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port1 Superscope Fire") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port1 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port1 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    // Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_superscope_offscreen_input, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port2 Superscope Pause") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port2 Superscope Turbo") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port2 Superscope Cursor") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port2 Superscope Fire") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)

	PORT_START("SUPERSCOPE2_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port2 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)

	PORT_START("SUPERSCOPE2_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port2 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
INPUT_PORTS_END

static INPUT_PORTS_START( snes )
	PORT_START("CTRLSEL")  /* Select Controller Type */
	PORT_CONFNAME( 0x0f, 0x01, "P1 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x01, "Gamepad" )
	PORT_CONFSETTING(  0x02, "Mouse" )
	PORT_CONFSETTING(  0x03, "Superscope" )
//  PORT_CONFSETTING(  0x04, "Justfier" )
//  PORT_CONFSETTING(  0x05, "Multitap" )
	PORT_CONFNAME( 0xf0, 0x10, "P2 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x10, "Gamepad" )
	PORT_CONFSETTING(  0x20, "Mouse" )
	PORT_CONFSETTING(  0x30, "Superscope" )
//  PORT_CONFSETTING(  0x40, "Justfier" )
//  PORT_CONFSETTING(  0x50, "Multitap" )

	PORT_INCLUDE(snes_joypads)
	PORT_INCLUDE(snes_mouse)
	PORT_INCLUDE(snes_superscope)

	PORT_START("OPTIONS")
	PORT_CONFNAME( 0x01, 0x00, "Hi-Res pixels blurring (TV effect)")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


/*************************************
 *
 *  Input callbacks
 *
 *************************************/

static void snes_gun_latch( running_machine &machine, INT16 x, INT16 y )
{
	/* these are the theoretical boundaries */
	if (x < 0)
		x = 0;
	if (x > (SNES_SCR_WIDTH - 1))
		x = SNES_SCR_WIDTH - 1;

	if (y < 0)
		y = 0;
	if (y > (snes_ppu.beam.last_visible_line - 1))
		y = snes_ppu.beam.last_visible_line - 1;

	snes_ppu.beam.latch_horz = x;
	snes_ppu.beam.latch_vert = y;
	snes_ram[STAT78] |= 0x40;
}

static void snes_input_read_joy( running_machine &machine, int port )
{
	snes_state *state = machine.driver_data<snes_state>();
	static const char *const portnames[2][4] =
			{
				{ "SERIAL1_DATA1_L", "SERIAL1_DATA1_H", "SERIAL1_DATA2_L", "SERIAL1_DATA2_H" },
				{ "SERIAL2_DATA1_L", "SERIAL2_DATA1_H", "SERIAL2_DATA2_L", "SERIAL2_DATA2_H" },
			};

	state->m_data1[port] = machine.root_device().ioport(portnames[port][0])->read() | (machine.root_device().ioport(portnames[port][1])->read() << 8);
	state->m_data2[port] = machine.root_device().ioport(portnames[port][2])->read() | (machine.root_device().ioport(portnames[port][3])->read() << 8);

	// avoid sending signals that could crash games
	// if left, no right
	if (state->m_data1[port] & 0x200)
		state->m_data1[port] &= ~0x100;
	// if up, no down
	if (state->m_data1[port] & 0x800)
		state->m_data1[port] &= ~0x400;

	state->m_joypad[port].buttons = state->m_data1[port];
}

static void snes_input_read_mouse( running_machine &machine, int port )
{
	snes_state *state = machine.driver_data<snes_state>();
	INT16 var;
	static const char *const portnames[2][3] =
			{
				{ "MOUSE1", "MOUSE1_X", "MOUSE1_Y" },
				{ "MOUSE2", "MOUSE2_X", "MOUSE2_Y" },
			};

	state->m_mouse[port].buttons = machine.root_device().ioport(portnames[port][0])->read();
	state->m_mouse[port].x = machine.root_device().ioport(portnames[port][1])->read();
	state->m_mouse[port].y = machine.root_device().ioport(portnames[port][2])->read();
	var = state->m_mouse[port].x - state->m_mouse[port].oldx;

	if (var < -127)
	{
		state->m_mouse[port].deltax = 0xff;
		state->m_mouse[port].oldx -= 127;
	}
	else if (var < 0)
	{
		state->m_mouse[port].deltax = 0x80 | (-var);
		state->m_mouse[port].oldx = state->m_mouse[port].x;
	}
	else if (var > 127)
	{
		state->m_mouse[port].deltax = 0x7f;
		state->m_mouse[port].oldx += 127;
	}
	else
	{
		state->m_mouse[port].deltax = var & 0xff;
		state->m_mouse[port].oldx = state->m_mouse[port].x;
	}

	var = state->m_mouse[port].y - state->m_mouse[port].oldy;

	if (var < -127)
	{
		state->m_mouse[port].deltay = 0xff;
		state->m_mouse[port].oldy -= 127;
	}
	else if (var < 0)
	{
		state->m_mouse[port].deltay = 0x80 | (-var);
		state->m_mouse[port].oldy = state->m_mouse[port].y;
	}
	else if (var > 127)
	{
		state->m_mouse[port].deltay = 0x7f;
		state->m_mouse[port].oldy += 127;
	}
	else
	{
		state->m_mouse[port].deltay = var & 0xff;
		state->m_mouse[port].oldy = state->m_mouse[port].y;
	}

	state->m_data1[port] = state->m_mouse[port].buttons | (0x00 << 8);
	state->m_data2[port] = 0;
}

static void snes_input_read_superscope( running_machine &machine, int port )
{
	snes_state *state = machine.driver_data<snes_state>();
	static const char *const portnames[2][3] =
			{
				{ "SUPERSCOPE1", "SUPERSCOPE1_X", "SUPERSCOPE1_Y" },
				{ "SUPERSCOPE2", "SUPERSCOPE2_X", "SUPERSCOPE2_Y" },
			};
	UINT8 input;

	/* first read input bits */
	state->m_scope[port].x = machine.root_device().ioport(portnames[port][1])->read();
	state->m_scope[port].y = machine.root_device().ioport(portnames[port][2])->read();
	input = machine.root_device().ioport(portnames[port][0])->read();

	/* then start elaborating input bits: only keep old turbo value */
	state->m_scope[port].buttons &= 0x20;

	/* set onscreen/offscreen */
	state->m_scope[port].buttons |= BIT(input, 1);

	/* turbo is a switch; toggle is edge sensitive */
	if (BIT(input, 5) && !state->m_scope[port].turbo_lock)
	{
		state->m_scope[port].buttons ^= 0x20;
		state->m_scope[port].turbo_lock = 1;
	}
	else if (!BIT(input, 5))
		state->m_scope[port].turbo_lock = 0;

	/* fire is a button; if turbo is active, trigger is level sensitive; otherwise it is edge sensitive */
	if (BIT(input, 7) && (BIT(state->m_scope[port].buttons, 5) || !state->m_scope[port].fire_lock))
	{
		state->m_scope[port].buttons |= 0x80;
		state->m_scope[port].fire_lock = 1;
	}
	else if (!BIT(input, 7))
		state->m_scope[port].fire_lock = 0;

	/* cursor is a button; it is always level sensitive */
	state->m_scope[port].buttons |= BIT(input, 6);

	/* pause is a button; it is always edge sensitive */
	if (BIT(input, 4) && !state->m_scope[port].pause_lock)
	{
		state->m_scope[port].buttons |= 0x10;
		state->m_scope[port].pause_lock = 1;
	}
	else if (!BIT(input, 4))
		state->m_scope[port].pause_lock = 0;

	/* If we have pressed fire or cursor and we are on-screen and SuperScope is in Port2, then latch video signal.
	Notice that we only latch Port2 because its IOBit pin is connected to bit7 of the IO Port, while Port1 has
	IOBit pin connected to bit6 of the IO Port, and the latter is not detected by the H/V Counters. In other
	words, you can connect SuperScope to Port1, but there is no way SNES could detect its on-screen position */
	if ((state->m_scope[port].buttons & 0xc0) && !(state->m_scope[port].buttons & 0x02) && port == 1)
		snes_gun_latch(machine, state->m_scope[port].x, state->m_scope[port].y);

	state->m_data1[port] = 0xff | (state->m_scope[port].buttons << 8);
	state->m_data2[port] = 0;
}

static void snes_input_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 ctrl1 = machine.root_device().ioport("CTRLSEL")->read() & 0x0f;
	UINT8 ctrl2 = (machine.root_device().ioport("CTRLSEL")->read() & 0xf0) >> 4;

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine.scheduler().timer_set(attotime::zero, FUNC(lightgun_tick));

	switch (ctrl1)
	{
	case 1: /* SNES joypad */
		snes_input_read_joy(machine, 0);
		break;
	case 2: /* SNES Mouse */
		snes_input_read_mouse(machine, 0);
		break;
	case 3: /* SNES Superscope */
		snes_input_read_superscope(machine, 0);
		break;
	case 0: /* no controller in port1 */
	default:
		state->m_data1[0] = 0;
		state->m_data2[0] = 0;
		break;
	}

	switch (ctrl2)
	{
	case 1: /* SNES joypad */
		snes_input_read_joy(machine, 1);
		break;
	case 2: /* SNES Mouse */
		snes_input_read_mouse(machine, 1);
		break;
	case 3: /* SNES Superscope */
		snes_input_read_superscope(machine, 1);
		break;
	case 0: /* no controller in port2 */
	default:
		state->m_data1[1] = 0;
		state->m_data2[1] = 0;
		break;
	}

	// is automatic reading on? if so, copy port data1/data2 to joy1l->joy4h
	// this actually works like reading the first 16bits from oldjoy1/2 in reverse order
	if (snes_ram[NMITIMEN] & 1)
	{
		state->m_joy1l = (state->m_data1[0] & 0x00ff) >> 0;
		state->m_joy1h = (state->m_data1[0] & 0xff00) >> 8;
		state->m_joy2l = (state->m_data1[1] & 0x00ff) >> 0;
		state->m_joy2h = (state->m_data1[1] & 0xff00) >> 8;
		state->m_joy3l = (state->m_data2[0] & 0x00ff) >> 0;
		state->m_joy3h = (state->m_data2[0] & 0xff00) >> 8;
		state->m_joy4l = (state->m_data2[1] & 0x00ff) >> 0;
		state->m_joy4h = (state->m_data2[1] & 0xff00) >> 8;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		state->m_read_idx[0] = 16;
		state->m_read_idx[1] = 16;
	}

}

static UINT8 snes_oldjoy1_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 ctrl1 = machine.root_device().ioport("CTRLSEL")->read() & 0x0f;
	UINT8 res = 0;

	switch (ctrl1)
	{
	case 1: /* SNES joypad */
		if (state->m_read_idx[0] >= 16)
			res = 0x01;
		else
			res = (state->m_joypad[0].buttons >> (15 - state->m_read_idx[0]++)) & 0x01;
		break;
	case 2: /* SNES Mouse */
		if (state->m_read_idx[0] >= 32)
			res = 0x01;
		else if (state->m_read_idx[0] >= 24)
			res = (state->m_mouse[0].deltax >> (31 - state->m_read_idx[0]++)) & 0x01;
		else if (state->m_read_idx[0] >= 16)
			res = (state->m_mouse[0].deltay >> (23 - state->m_read_idx[0]++)) & 0x01;
		else if (state->m_read_idx[0] >= 8)
			res = (state->m_mouse[0].buttons >> (15 - state->m_read_idx[0]++)) & 0x01;
		else
			res = 0;
		break;
	case 3: /* SNES Superscope */
		if (state->m_read_idx[0] >= 8)
			res = 0x01;
		else
			res = (state->m_scope[0].buttons >> (7 - state->m_read_idx[0]++)) & 0x01;
		break;
	case 0: /* no controller in port2 */
	default:
		break;
	}

	return res;
}

static UINT8 snes_oldjoy2_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 ctrl2 = (machine.root_device().ioport("CTRLSEL")->read() & 0xf0) >> 4;
	UINT8 res = 0;

	switch (ctrl2)
	{
	case 1: /* SNES joypad */
		if (state->m_read_idx[1] >= 16)
			res = 0x01;
		else
			res = (state->m_joypad[1].buttons >> (15 - state->m_read_idx[1]++)) & 0x01;
		break;
	case 2: /* SNES Mouse */
		if (state->m_read_idx[1] >= 32)
			res = 0x01;
		else if (state->m_read_idx[1] >= 24)
			res = (state->m_mouse[1].deltax >> (31 - state->m_read_idx[1]++)) & 0x01;
		else if (state->m_read_idx[1] >= 16)
			res = (state->m_mouse[1].deltay >> (23 - state->m_read_idx[1]++)) & 0x01;
		else if (state->m_read_idx[1] >= 8)
			res = (state->m_mouse[1].buttons >> (15 - state->m_read_idx[1]++)) & 0x01;
		else
			res = 0;
		break;
	case 3: /* SNES Superscope */
		if (state->m_read_idx[1] >= 8)
			res = 0x01;
		else
			res = (state->m_scope[1].buttons >> (7 - state->m_read_idx[1]++)) & 0x01;
		break;
	case 0: /* no controller in port2 */
	default:
		break;
	}

	return res;
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_RESET( snes_mess )
{
	snes_state *state = machine.driver_data<snes_state>();

	MACHINE_RESET_CALL(snes);

	state->m_io_read = snes_input_read;
	state->m_oldjoy1_read = snes_oldjoy1_read;
	state->m_oldjoy2_read = snes_oldjoy2_read;
}

static MACHINE_CONFIG_START( snes_base, snes_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", _5A22, MCLK_NTSC)   /* 2.68 MHz, also 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(snes_map)

	MCFG_CPU_ADD("soundcpu", SPC700, 1024000)   /* 1.024 MHz */
	MCFG_CPU_PROGRAM_MAP(spc_map)

	//MCFG_QUANTUM_TIME(attotime::from_hz(48000))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_MACHINE_START(snes_mess)
	MCFG_MACHINE_RESET(snes_mess)

	/* video hardware */
	MCFG_VIDEO_START(snes)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE_DRIVER( snes_state, snes_screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snes, snes_base )

	MCFG_FRAGMENT_ADD(snes_cartslot)
MACHINE_CONFIG_END


static SUPERFX_CONFIG( snes_superfx_config )
{
	DEVCB_DRIVER_LINE_MEMBER(snes_state,snes_extern_irq_w)  /* IRQ line from cart */
};

static MACHINE_CONFIG_DERIVED( snessfx, snes )

	MCFG_CPU_ADD("superfx", SUPERFX, 21480000)  /* 21.48MHz */
	MCFG_CPU_PROGRAM_MAP(superfx_map)
	MCFG_CPU_CONFIG(snes_superfx_config)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snesdsp, snes )

	MCFG_CPU_ADD("dsp", UPD7725, 8000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snesst10, snes )

	MCFG_CPU_ADD("setadsp", UPD96050, 10000000)
	MCFG_CPU_PROGRAM_MAP(setadsp_prg_map)
	MCFG_CPU_DATA_MAP(setadsp_data_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snesst11, snes )

	MCFG_CPU_ADD("setadsp", UPD96050, 15000000)
	MCFG_CPU_PROGRAM_MAP(setadsp_prg_map)
	MCFG_CPU_DATA_MAP(setadsp_data_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespal, snes_base )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( MCLK_PAL )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_PAL, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL)

	MCFG_FRAGMENT_ADD(snesp_cartslot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespsfx, snespal )

	MCFG_CPU_ADD("superfx", SUPERFX, 21480000)  /* 21.48MHz */
	MCFG_CPU_PROGRAM_MAP(superfx_map)
	MCFG_CPU_CONFIG(snes_superfx_config)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespdsp, snespal )

	MCFG_CPU_ADD("dsp", UPD7725, 8000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snesst, snes_base )

	MCFG_MACHINE_START(snesst)

	MCFG_FRAGMENT_ADD(sufami_cartslot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snesbsx, snes_base )

	MCFG_FRAGMENT_ADD(bsx_cartslot)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( snes )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )      /* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesdsp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "dsp1b.bin", SNES_DSP1B_OFFSET, 0x002800, CRC(453557e0) SHA1(3a218b0e4572a8eba6d0121b17fdac9529609220) )
	ROM_LOAD( "dsp1.bin",  SNES_DSP1_OFFSET,  0x002800, CRC(2838f9f5) SHA1(0a03ccb1fd2bea91151c745a4d1f217ae784f889) )
	ROM_LOAD( "dsp2.bin",  SNES_DSP2_OFFSET,  0x002800, CRC(8e9fbd9b) SHA1(06dd9fcb118d18f6bbe234e013cb8780e06d6e63) )
	ROM_LOAD( "dsp3.bin",  SNES_DSP3_OFFSET,  0x002800, CRC(6b86728a) SHA1(1b133741fad810eb7320c21ecfdd427d25a46da1) )
	ROM_LOAD( "dsp4.bin",  SNES_DSP4_OFFSET,  0x002800, CRC(ce0c7783) SHA1(76fd25f7dc26c3b3f7868a3aa78c7684068713e5) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesst10 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x11000, "addons", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesst11 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x11000, "addons", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "st011.bin",    0x000000, 0x011000, CRC(34d2952c) SHA1(1375b8c1efc8cae4962b57dfe22f6b78e1ddacc8) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snessfx )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )      /* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snespal )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )      /* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snespdsp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "dsp1b.bin", SNES_DSP1B_OFFSET, 0x002800, CRC(453557e0) SHA1(3a218b0e4572a8eba6d0121b17fdac9529609220) )
	ROM_LOAD( "dsp1.bin",  SNES_DSP1_OFFSET,  0x002800, CRC(2838f9f5) SHA1(0a03ccb1fd2bea91151c745a4d1f217ae784f889) )
	ROM_LOAD( "dsp2.bin",  SNES_DSP2_OFFSET,  0x002800, CRC(8e9fbd9b) SHA1(06dd9fcb118d18f6bbe234e013cb8780e06d6e63) )
	ROM_LOAD( "dsp3.bin",  SNES_DSP3_OFFSET,  0x002800, CRC(6b86728a) SHA1(1b133741fad810eb7320c21ecfdd427d25a46da1) )
	ROM_LOAD( "dsp4.bin",  SNES_DSP4_OFFSET,  0x002800, CRC(ce0c7783) SHA1(76fd25f7dc26c3b3f7868a3aa78c7684068713e5) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snespsfx )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )      /* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesst )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )      /* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( 0x40000, "sufami", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "shvc-qh-0.bin", 0,   0x40000, CRC(9b4ca911) SHA1(ef86ea192eed03d5c413fdbbfd46043be1d7a127) )

	ROM_REGION( MAX_SNES_CART_SIZE, "slot_a", ROMREGION_ERASE00 )
	ROM_REGION( MAX_SNES_CART_SIZE, "slot_b", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesbsx )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )      /* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "dsp1b.bin", SNES_DSP1B_OFFSET, 0x002800, CRC(453557e0) SHA1(3a218b0e4572a8eba6d0121b17fdac9529609220) )
	ROM_LOAD( "dsp1.bin",  SNES_DSP1_OFFSET,  0x002800, CRC(2838f9f5) SHA1(0a03ccb1fd2bea91151c745a4d1f217ae784f889) )
	ROM_LOAD( "dsp2.bin",  SNES_DSP2_OFFSET,  0x002800, CRC(8e9fbd9b) SHA1(06dd9fcb118d18f6bbe234e013cb8780e06d6e63) )
	ROM_LOAD( "dsp3.bin",  SNES_DSP3_OFFSET,  0x002800, CRC(6b86728a) SHA1(1b133741fad810eb7320c21ecfdd427d25a46da1) )
	ROM_LOAD( "dsp4.bin",  SNES_DSP4_OFFSET,  0x002800, CRC(ce0c7783) SHA1(76fd25f7dc26c3b3f7868a3aa78c7684068713e5) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( MAX_SNES_CART_SIZE, "flash", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  INIT          COMPANY     FULLNAME                                      FLAGS */
CONS( 1989, snes,     0,      0,      snes,     snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespal,  snes,   0,      snespal,  snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

// FIXME: the "hacked" drivers below, currently needed due to limitations in the core device design, should eventually be removed

// These would require CPU to be added/removed depending on the cart which is loaded
CONS( 1989, snesdsp,  snes,   0,      snesdsp,  snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/DSP-x)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespdsp, snes,   0,      snespdsp, snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/DSP-x)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snessfx,  snes,   0,      snessfx,  snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/SuperFX)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespsfx, snes,   0,      snespsfx, snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/SuperFX)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snesst10,  snes,   0,      snesst10,  snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/ST-010)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snesst11,  snes,   0,      snesst11,  snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/ST-011)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
//CONS( 1989, snessa1,  snes,   0,      snessa1,  snes, XXX_CLASS,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/SA-1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
//CONS( 1991, snespsa1, snes,   0,      snespsa1, snes, XXX_CLASS,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/SA-1)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

// These would require cartslot to be added/removed depending on the cart which is loaded
CONS( 1989, snesst,   snes,   0,      snesst,  snes, snes_state,  snesst,       "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/Sufami Turbo)", GAME_NOT_WORKING )
CONS( 1989, snesbsx,  snes,   0,      snesbsx, snes, snes_state,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/BS-X Satellaview slotted cart)",  GAME_NOT_WORKING )



// WIP for slot-ified cart devices

#include "machine/sns_slot.h"
#include "machine/sns_rom.h"
#include "machine/sns_rom21.h"
#include "machine/sns_bsx.h"
#include "machine/sns_sdd1.h"
#include "machine/sns_sfx.h"
#include "machine/sns_spc7110.h"
#include "machine/sns_sufami.h"
#include "machine/sns_upd.h"

class snsnew_state : public snes_state
{
public:
	snsnew_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag),
		m_slotcart(*this, "snsslot")
	{ }	
	
	optional_device<sns_cart_slot_device> m_slotcart;
	int m_type;
};


// FIXME: merge these add-on specific maps into something more sane!

// In general LoROM games have perfect mirror between 0x00-0x7d and 0x80-0xff
// But BSX+LoROM games use different read handlers (to access ROM beyond 2MB)
// so we use two different set of handlers...

// LoROM

static READ8_HANDLER( snes20_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
			return snes_r_io(space, address);
		if (address >= 0x6000 && address < 0x8000)
			return snes_open_bus_r(space, 0);
		if (address >= 0x8000)			
			return state->m_slotcart->m_cart->read_h(space, offset);
	}
	else if (offset < 0x700000)
	{
		if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return state->m_slotcart->m_cart->read_h(space, offset);
	}
	
	// ROM & NVRAM access
	return state->m_slotcart->m_cart->read_h(space, offset);
}	

static WRITE8_HANDLER( snes20_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
			snes_w_io(space, address, data);
	}
	else if (offset >= 0x700000)	// NVRAM access
	{
		state->m_slotcart->m_cart->write_h(space, offset, data);
	}
}	

static READ8_HANDLER( snes20_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
			return snes_r_io(space, address);
		if (address >= 0x6000 && address < 0x8000)
			return snes_open_bus_r(space, 0);
		if (address >= 0x8000)			
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	else if (offset < 0x700000)
	{
		if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	
	// ROM & NVRAM access
	return state->m_slotcart->m_cart->read_l(space, offset);
}	

static WRITE8_HANDLER( snes20_lo_w )
{
	snes20_hi_w(space, offset, data, 0xff);
}	


// HiROM

static READ8_HANDLER( snes21_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
			return snes_r_io(space, address);
		if (address >= 0x6000 && address < 0x8000)
		{
			if (state->m_slotcart->m_cart->get_nvram_size())
			{
				// read NVRAM, instead
				if (offset >= 0x300000)
					return state->m_slotcart->m_cart->read_l(space, offset);
			}
			return snes_open_bus_r(space, 0);
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	
	// ROM & NVRAM access
	return state->m_slotcart->m_cart->read_l(space, offset);
}	

static WRITE8_HANDLER( snes21_lo_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
			snes_w_io(space, address, data);
		if (address >= 0x6000 && address < 0x8000)
		{
			if (state->m_slotcart->m_cart->get_nvram_size())
			{
				// write to NVRAM, in this case
				if (offset >= 0x300000)
					state->m_slotcart->m_cart->write_l(space, offset, data);
			}
		}
	}
	else if (offset >= 0x700000)	// NVRAM access
		state->m_slotcart->m_cart->write_l(space, offset, data);
}	

static READ8_HANDLER( snes21_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
			return snes_r_io(space, address);
		if (address >= 0x6000 && address < 0x8000)
		{
			if (state->m_slotcart->m_cart->get_nvram_size())
			{
				// read NVRAM, instead
				if (offset >= 0x300000)
					return  state->m_slotcart->m_cart->read_h(space, offset);
			}
			return snes_open_bus_r(space, 0);
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_h(space, offset);
	}
	
	// ROM & NVRAM access
	return state->m_slotcart->m_cart->read_h(space, offset);
}	

static WRITE8_HANDLER( snes21_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
			snes_w_io(space, address, data);
		if (address >= 0x6000 && address < 0x8000)
		{
			if (state->m_slotcart->m_cart->get_nvram_size())
			{
				// write to NVRAM, in this case
				if (offset >= 0x300000)
					state->m_slotcart->m_cart->write_h(space, offset, data);
			}
		}
	}
	else if (offset >= 0x700000)	// NVRAM access
		state->m_slotcart->m_cart->write_h(space, offset, data);
}	

// SuperFX / GSU

static READ8_HANDLER( snesfx_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				return state->m_slotcart->m_cart->chip_read(space, offset);
			else
				return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			return state->m_slotcart->m_cart->read_h(space, offset);	//RAM
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_h(space, offset);	//ROM
	}
	if (offset < 0x600000)
		return state->m_slotcart->m_cart->read_h(space, offset);	//ROM
	
	return state->m_slotcart->m_cart->read_h(space, offset);	//RAM
}	

static READ8_HANDLER( snesfx_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				return state->m_slotcart->m_cart->chip_read(space, offset);
			else
				return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			return state->m_slotcart->m_cart->read_l(space, offset);	//RAM
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_l(space, offset);	//ROM
	}
	if (offset < 0x600000)
		return state->m_slotcart->m_cart->read_l(space, offset);	//ROM
	
	return state->m_slotcart->m_cart->read_l(space, offset);	//RAM
}	

static WRITE8_HANDLER( snesfx_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				state->m_slotcart->m_cart->chip_write(space, offset, data);
			else
				snes_w_io(space, address, data);
		}
		if (address >= 0x6000 && address < 0x8000)
			state->m_slotcart->m_cart->write_h(space, offset, data);
	}
	else
		state->m_slotcart->m_cart->write_h(space, offset, data);
}	

static WRITE8_HANDLER( snesfx_lo_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				state->m_slotcart->m_cart->chip_write(space, offset, data);
			else
				snes_w_io(space, address, data);
		}
		if (address >= 0x6000 && address < 0x8000)
			state->m_slotcart->m_cart->write_l(space, offset, data);
	}
	else
		state->m_slotcart->m_cart->write_l(space, offset, data);
}	

// SPC-7110

static READ8_HANDLER( snespc7110_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			UINT16 limit = (state->m_slotcart->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
				return state->m_slotcart->m_cart->chip_read(space, address);
			
			return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				return state->m_slotcart->m_cart->read_h(space, offset);
			if (offset >= 0x300000 && offset < 0x310000)
				return state->m_slotcart->m_cart->read_h(space, offset);
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_h(space, offset);
	}
	return state->m_slotcart->m_cart->read_h(space, offset);
}	

static READ8_HANDLER( snespc7110_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			UINT16 limit = (state->m_slotcart->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
				return state->m_slotcart->m_cart->chip_read(space, address);
			
			return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				return state->m_slotcart->m_cart->read_l(space, offset);
			if (offset >= 0x300000 && offset < 0x310000)
				return state->m_slotcart->m_cart->read_l(space, offset);
		}
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	if (offset >= 0x500000 && offset < 0x510000)
		return state->m_slotcart->m_cart->chip_read(space, 0x4800);

	return snes_open_bus_r(space, 0);
}	

static WRITE8_HANDLER( snespc7110_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			UINT16 limit = (state->m_slotcart->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
			{
				state->m_slotcart->m_cart->chip_write(space, address, data);
				return;
			}
			snes_w_io(space, address, data);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				state->m_slotcart->m_cart->write_l(space, offset, data);
			if (offset >= 0x300000 && offset < 0x310000)
				state->m_slotcart->m_cart->write_l(space, offset, data);
		}
	}
}	

static WRITE8_HANDLER( snespc7110_lo_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			UINT16 limit = (state->m_slotcart->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
			{
				state->m_slotcart->m_cart->chip_write(space, address, data);
				return;
			}
			snes_w_io(space, address, data);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset < 0x10000)
				state->m_slotcart->m_cart->write_l(space, offset, data);
			if (offset >= 0x300000 && offset < 0x310000)
				state->m_slotcart->m_cart->write_l(space, offset, data);
		}
	}
}	


// S-DD1

static READ8_HANDLER( snesdd1_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x4800 && address < 0x4808)
				return state->m_slotcart->m_cart->chip_read(space, address);
			
			return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
			return snes_open_bus_r(space, 0);
		if (address >= 0x8000)
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	
	// ROM & NVRAM access
	return state->m_slotcart->m_cart->read_l(space, offset);
}	

static WRITE8_HANDLER( snesdd1_lo_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x4300 && address < 0x4380)
			{
				state->m_slotcart->m_cart->chip_write(space, address, data);
				// here we don't return, but we let the w_io happen...
			}
			if (address >= 0x4800 && address < 0x4808)
			{
				state->m_slotcart->m_cart->chip_write(space, address, data);
				return;
			}
			snes_w_io(space, address, data);
		}
	}
	if (offset >= 0x700000 && address < 0x8000 && state->m_slotcart->m_cart->get_nvram_size())
		return state->m_slotcart->m_cart->write_l(space, offset, data);
}	

static READ8_HANDLER( snesdd1_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	
	if (offset >= 0x400000)
		return state->m_slotcart->m_cart->read_h(space, offset);
	else		
		return snesdd1_lo_r(space, offset, 0xff);
}	

static WRITE8_HANDLER( snesdd1_hi_w )
{
	snesdd1_lo_w(space, offset, data, 0xff);
}	


// BS-X

static READ8_HANDLER( snesbsx_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x2188 && address < 0x21a0)
				return state->m_slotcart->m_cart->chip_read(space, offset);
			if (address >= 0x5000)
				return state->m_slotcart->m_cart->chip_read(space, offset);
			return snes_r_io(space, address);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset >= 0x200000)			
				return state->m_slotcart->m_cart->read_l(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
		if (address >= 0x8000)			
			return state->m_slotcart->m_cart->read_l(space, offset);
	}
	return state->m_slotcart->m_cart->read_l(space, offset);
}	

static WRITE8_HANDLER( snesbsx_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		if (address >= 0x2000 && address < 0x6000)
		{
			if (address >= 0x2188 && address < 0x21a0)
			{
				state->m_slotcart->m_cart->chip_write(space, offset, data);
				return;
			}
			if (address >= 0x5000)
			{
				state->m_slotcart->m_cart->chip_write(space, offset, data);
				return;
			}
			snes_w_io(space, address, data);
		}
		if (address >= 0x6000 && address < 0x8000)
		{
			if (offset >= 0x200000)			
				return state->m_slotcart->m_cart->write_l(space, offset, data);
		}
		if (address >= 0x8000)			
			return state->m_slotcart->m_cart->write_l(space, offset, data);
	}
	return state->m_slotcart->m_cart->write_l(space, offset, data);
}	

static READ8_HANDLER( snesbsx_lo_r )
{
	return snesbsx_hi_r(space, offset, 0xff);
}	

static WRITE8_HANDLER( snesbsx_lo_w )
{
	snesbsx_hi_w(space, offset, data, 0xff);
}	


static READ8_HANDLER( snesnew_lo_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();

	// take care of add-on IO
	if (state->m_slotcart->get_type() == SNES_DSP 
		&& (offset >= 0x200000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x7fff);
	else if (state->m_slotcart->get_type() == SNES_DSP_MODE21 
			 && (offset < 0x200000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x1fff);
	else if (state->m_slotcart->get_type() == SNES_DSP4 
			 && (offset >= 0x300000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x7fff);
	else if (state->m_slotcart->get_type() == SNES_OBC1 
			 && (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset);
	else if ((state->m_slotcart->get_type() == SNES_ST010 || state->m_slotcart->get_type() == SNES_ST011) 
			 && (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x1000))
		return state->m_slotcart->m_cart->chip_read(space, offset);
	else if (state->m_slotcart->get_type() == SNES_SRTC
			 && (offset < 0x400000 && (offset & 0xffff) == 0x2800))
			return state->m_slotcart->m_cart->chip_read(space, offset & 0xffff);
	else
	{
		switch (state->m_type)
		{
			case SNES_MODE20:
			case SNES_ST010:
			case SNES_ST011:
			case SNES_DSP:
			case SNES_DSP4:
			case SNES_OBC1:
			case SNES_SUFAMITURBO:
			case SNES_STROM:
			case SNES_BSXLO:
			case SNES_POKEMON:
			case SNES_BANANA:
				return snes20_lo_r(space, offset, 0xff);
				
			case SNES_MODE21:
			case SNES_DSP_MODE21:
			case SNES_SRTC:
			case SNES_BSXHI:
				return snes21_lo_r(space, offset, 0xff);
				
			case SNES_SFX:
				return snesfx_lo_r(space, offset, 0xff);
				
			case SNES_SPC7110:
			case SNES_SPC7110_RTC:
				return snespc7110_lo_r(space, offset, 0xff);
				
			case SNES_SDD1:
				return snesdd1_lo_r(space, offset, 0xff);
				
			case SNES_BSX:
				return snesbsx_lo_r(space, offset, 0xff);
		}
	}
	return snes_open_bus_r(space, 0);
}	

static READ8_HANDLER( snesnew_hi_r )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	
	// take care of add-on IO
	if (state->m_slotcart->get_type() == SNES_DSP
		&& (offset >= 0x200000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x7fff);
	else if (state->m_slotcart->get_type() == SNES_DSP_MODE21 
			 && (offset < 0x200000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x1fff);
	else if (state->m_slotcart->get_type() == SNES_DSP4 
			 && (offset >= 0x300000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset & 0x7fff);
	else if (state->m_slotcart->get_type() == SNES_OBC1 
			 && (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		return state->m_slotcart->m_cart->chip_read(space, offset);
	else if ((state->m_slotcart->get_type() == SNES_ST010 || state->m_slotcart->get_type() == SNES_ST011) 
			 && (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x1000))
		return state->m_slotcart->m_cart->chip_read(space, offset);
	else if (state->m_slotcart->get_type() == SNES_SRTC 
			 && (offset < 0x400000 && (offset & 0xffff) == 0x2800))
			return state->m_slotcart->m_cart->chip_read(space, offset & 0xffff);
	else if ((state->m_slotcart->get_type() == SNES_POKEMON || state->m_slotcart->get_type() == SNES_BANANA)
			 && (offset & 0x70000) == 0x0000)
	{
//		printf("hi read %x\n", offset);
		return state->m_slotcart->m_cart->chip_read(space, offset);
	}
	else
	{
		switch (state->m_type)
		{
			case SNES_MODE20:
			case SNES_ST010:
			case SNES_ST011:
			case SNES_DSP:
			case SNES_DSP4:
			case SNES_OBC1:
			case SNES_SUFAMITURBO:
			case SNES_STROM:
			case SNES_BSXLO:
			case SNES_POKEMON:
			case SNES_BANANA:
				return snes20_hi_r(space, offset, 0xff);
				
			case SNES_MODE21:
			case SNES_DSP_MODE21:
			case SNES_SRTC:
			case SNES_BSXHI:
				return snes21_hi_r(space, offset, 0xff);
				
			case SNES_SFX:
				return snesfx_hi_r(space, offset, 0xff);
				
			case SNES_SPC7110:
			case SNES_SPC7110_RTC:
				return snespc7110_hi_r(space, offset, 0xff);
				
			case SNES_SDD1:
				return snesdd1_hi_r(space, offset, 0xff);
				
			case SNES_BSX:
				return snesbsx_hi_r(space, offset, 0xff);
		}
	}
	return snes_open_bus_r(space, 0);
}	

static WRITE8_HANDLER( snesnew_lo_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	
	// take care of add-on IO
	if (state->m_slotcart->get_type() == SNES_DSP
		&& (offset >= 0x200000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x7fff, data);
	else if (state->m_slotcart->get_type() == SNES_DSP_MODE21 
			 && (offset < 0x200000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x1fff, data);
	else if (state->m_slotcart->get_type() == SNES_DSP4 
			 && (offset >= 0x300000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x7fff, data);
	else if (state->m_slotcart->get_type() == SNES_OBC1 
			 && (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset, data);
	else if ((state->m_slotcart->get_type() == SNES_ST010 || state->m_slotcart->get_type() == SNES_ST011) 
			 && (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x1000))
		state->m_slotcart->m_cart->chip_write(space, offset, data);
	else if (state->m_slotcart->get_type() == SNES_SRTC
			 && (offset < 0x400000 && (offset & 0xffff) == 0x2801))
		state->m_slotcart->m_cart->chip_write(space, offset & 0xffff, data);
	else if (state->m_slotcart->get_type() == SNES_BANANA
			 && (offset & 0x78000) == 0x8000)
	{	
//		printf("lo write %x\n", offset);
		state->m_slotcart->m_cart->chip_write(space, offset, data);
	}
	else
	{
		switch (state->m_type)
		{
			case SNES_MODE20:
			case SNES_ST010:
			case SNES_ST011:
			case SNES_DSP:
			case SNES_DSP4:
			case SNES_OBC1:
			case SNES_SUFAMITURBO:
			case SNES_STROM:
			case SNES_BSXLO:
			case SNES_POKEMON:
			case SNES_BANANA:
				snes20_lo_w(space, offset, data, 0xff);
				break;
				
			case SNES_MODE21:
			case SNES_DSP_MODE21:
			case SNES_SRTC:
			case SNES_BSXHI:
				snes21_lo_w(space, offset, data, 0xff);
				break;
				
			case SNES_SFX:
				snesfx_lo_w(space, offset, data, 0xff);
				break;
				
			case SNES_SPC7110:
			case SNES_SPC7110_RTC:
				snespc7110_lo_w(space, offset, data, 0xff);
				break;
				
			case SNES_SDD1:
				snesdd1_lo_w(space, offset, data, 0xff);
				break;
				
			case SNES_BSX:
				snesbsx_lo_w(space, offset, data, 0xff);
				break;
		}
	}
}	

static WRITE8_HANDLER( snesnew_hi_w )
{
	snsnew_state *state = space.machine().driver_data<snsnew_state>();
	
	// take care of add-on IO
	if (state->m_slotcart->get_type() == SNES_DSP
		&& (offset >= 0x200000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x7fff, data);
	else if (state->m_slotcart->get_type() == SNES_DSP_MODE21 
			 && (offset < 0x200000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x1fff, data);
	else if (state->m_slotcart->get_type() == SNES_DSP4 
			 && (offset >= 0x300000 && offset < 0x400000 && (offset & 0x8000) == 0x8000))
		state->m_slotcart->m_cart->chip_write(space, offset & 0x7fff, data);
	else if (state->m_slotcart->get_type() == SNES_OBC1 
			 && (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))
		return state->m_slotcart->m_cart->chip_write(space, offset, data);
	else if ((state->m_slotcart->get_type() == SNES_ST010 || state->m_slotcart->get_type() == SNES_ST011) 
			 && (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x1000))
		state->m_slotcart->m_cart->chip_write(space, offset, data);
	else if (state->m_slotcart->get_type() == SNES_SRTC
			 && (offset < 0x400000 && (offset & 0xffff) == 0x2801))
		state->m_slotcart->m_cart->chip_write(space, offset & 0xffff, data);
	else if ((state->m_slotcart->get_type() == SNES_POKEMON)
			 && (offset & 0x70000) == 0x0000)
	{	
//		printf("hi write %x\n", offset);
		state->m_slotcart->m_cart->chip_write(space, offset, data);
	}
	else
	{
		switch (state->m_type)
		{
			case SNES_MODE20:
			case SNES_ST010:
			case SNES_ST011:
			case SNES_DSP:
			case SNES_DSP4:
			case SNES_OBC1:
			case SNES_SUFAMITURBO:
			case SNES_STROM:
			case SNES_BSXLO:
			case SNES_POKEMON:
			case SNES_BANANA:
				snes20_hi_w(space, offset, data, 0xff);
				break;
				
			case SNES_MODE21:
			case SNES_DSP_MODE21:
			case SNES_SRTC:
			case SNES_BSXHI:
				snes21_hi_w(space, offset, data, 0xff);
				break;
				
			case SNES_SFX:
				snesfx_hi_w(space, offset, data, 0xff);
				break;
				
			case SNES_SPC7110:
			case SNES_SPC7110_RTC:
				snespc7110_hi_w(space, offset, data, 0xff);
				break;
				
			case SNES_SDD1:
				snesdd1_hi_w(space, offset, data, 0xff);
				break;
				
			case SNES_BSX:
				snesbsx_hi_w(space, offset, data, 0xff);
				break;
		}
	}
}	


static ADDRESS_MAP_START( snesnew_map, AS_PROGRAM, 8, snsnew_state )
	AM_RANGE(0x000000, 0x7dffff) AM_READWRITE_LEGACY(snesnew_lo_r, snesnew_lo_w)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM                 /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE_LEGACY(snesnew_hi_r, snesnew_hi_w)
ADDRESS_MAP_END

static SLOT_INTERFACE_START(snes_cart)
	SLOT_INTERFACE_INTERNAL("lorom",         SNS_LOROM)
	SLOT_INTERFACE_INTERNAL("lorom_bsx",     SNS_LOROM_BSX)	// LoROM + BS-X slot - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_cx4",     SNS_LOROM)	// Cart + CX4 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_dsp",     SNS_LOROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("lorom_dsp4",    SNS_LOROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("lorom_obc1",    SNS_LOROM_OBC1)
	SLOT_INTERFACE_INTERNAL("lorom_sa1",     SNS_LOROM)	// Cart + SA1 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_sdd1",    SNS_LOROM_SDD1)
	SLOT_INTERFACE_INTERNAL("lorom_sfx",     SNS_LOROM_SUPERFX)
	SLOT_INTERFACE_INTERNAL("lorom_sgb",     SNS_LOROM)	// SuperGB base cart - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_st010",   SNS_LOROM_SETA10)
	SLOT_INTERFACE_INTERNAL("lorom_st011",   SNS_LOROM_SETA11)
	SLOT_INTERFACE_INTERNAL("lorom_st018",   SNS_LOROM)	// Cart + ST018 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_sufami",  SNS_LOROM_SUFAMI)	// Sufami Turbo base cart
	SLOT_INTERFACE_INTERNAL("hirom",         SNS_HIROM)
	SLOT_INTERFACE_INTERNAL("hirom_bsx",     SNS_HIROM_BSX)	// HiROM + BS-X slot - unsupported
	SLOT_INTERFACE_INTERNAL("hirom_dsp",     SNS_HIROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("hirom_spc7110", SNS_HIROM_SPC7110)
	SLOT_INTERFACE_INTERNAL("hirom_spcrtc",  SNS_HIROM_SPC7110_RTC)
	SLOT_INTERFACE_INTERNAL("hirom_srtc",    SNS_HIROM_SRTC)
	SLOT_INTERFACE_INTERNAL("bsxrom",        SNS_ROM_BSX)	// BS-X base cart - partial support only
	// pirate carts
	SLOT_INTERFACE_INTERNAL("lorom_poke",    SNS_LOROM_POKEMON)
SLOT_INTERFACE_END


static MACHINE_START( snesnew )
{
	snsnew_state *state = machine.driver_data<snsnew_state>();

	state->m_type = state->m_slotcart->get_type();

	MACHINE_START_CALL(snes);

	// FIXME: why installing handlers here does not work? it would allow to clean up handlers above...
	switch (state->m_type)
	{
		case SNES_MODE21:
		case SNES_MODE25:
		case SNES_DSP_MODE21:
//			machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x000000, 0x7dffff, FUNC(snes21_lo_r), FUNC(snes21_lo_w));
//			machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x800000, 0xffffff, FUNC(snes21_hi_r), FUNC(snes21_hi_w));
			break;
	}
}

static MACHINE_CONFIG_START( snesnew, snsnew_state )
	MCFG_FRAGMENT_ADD( snes_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(snesnew_map)

	MCFG_MACHINE_START(snesnew)

	MCFG_SNS_CARTRIDGE_ADD("snsslot", snes_cart, NULL, NULL)
	MCFG_SOFTWARE_LIST_ADD("cart_list","snes")
	MCFG_SOFTWARE_LIST_ADD("bsx_list","snes_bspack")
	MCFG_SOFTWARE_LIST_ADD("st_list","snes_strom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespnew, snesnew )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( MCLK_PAL )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_PAL, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL)
MACHINE_CONFIG_END


#define rom_snesnew rom_snes
#define rom_snespnew rom_snespal

CONS( 1989, snesnew,   snes,   0,      snesnew,  snes, snes_state,  snes_mess,       "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, Test)", GAME_NOT_WORKING )
CONS( 1989, snespnew,  snes,   0,      snespnew, snes, snes_state,  snes_mess,       "Nintendo", "Super Nintendo Entertainment System (PAL, Test)", GAME_NOT_WORKING )
