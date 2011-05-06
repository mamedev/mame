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
	return spc_ram_r(device, offset + 0x100);
}

static WRITE8_DEVICE_HANDLER( spc_ram_100_w )
{
	spc_ram_w(device, offset + 0x100, data);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( snes_map, AS_PROGRAM, 8)
	AM_RANGE(0x000000, 0x2fffff) AM_READWRITE(snes_r_bank1, snes_w_bank1)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE(snes_r_bank2, snes_w_bank2)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x400000, 0x5fffff) AM_READ(snes_r_bank3)		/* ROM (and reserved in Mode 20) */
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE(snes_r_bank4, snes_w_bank4)	/* used by Mode 20 DSP-1 */
	AM_RANGE(0x700000, 0x7dffff) AM_READWRITE(snes_r_bank5, snes_w_bank5)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM					/* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE(snes_r_bank6, snes_w_bank6)	/* Mirror and ROM */
	AM_RANGE(0xc00000, 0xffffff) AM_READWRITE(snes_r_bank7, snes_w_bank7)	/* Mirror and ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( superfx_map, AS_PROGRAM, 8)
	AM_RANGE(0x000000, 0x3fffff) AM_READWRITE(superfx_r_bank1, superfx_w_bank1)
	AM_RANGE(0x400000, 0x5fffff) AM_READWRITE(superfx_r_bank2, superfx_w_bank2)
	AM_RANGE(0x600000, 0x7dffff) AM_READWRITE(superfx_r_bank3, superfx_w_bank3)
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE(superfx_r_bank1, superfx_w_bank1)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(superfx_r_bank2, superfx_w_bank2)
	AM_RANGE(0xe00000, 0xffffff) AM_READWRITE(superfx_r_bank3, superfx_w_bank3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spc_map, AS_PROGRAM, 8)
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE("spc700", spc_ram_r, spc_ram_w)	/* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE("spc700", spc_io_r, spc_io_w)  	/* spc io */
	AM_RANGE(0x0100, 0xffff) AM_DEVWRITE("spc700", spc_ram_100_w)
	AM_RANGE(0x0100, 0xffbf) AM_DEVREAD("spc700", spc_ram_100_r)
	AM_RANGE(0xffc0, 0xffff) AM_DEVREAD("spc700", spc_ipl_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_prg_map, AS_PROGRAM, 32 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("dspprg", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_data_map, AS_DATA, 16 )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("dspdata", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( setadsp_prg_map, AS_PROGRAM, 32 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("dspprg", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( setadsp_data_map, AS_DATA, 16 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("dspdata", 0)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static CUSTOM_INPUT( snes_mouse_speed_input )
{
	snes_state *state = field->port->machine().driver_data<snes_state>();
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
	snes_state *state = field->port->machine().driver_data<snes_state>();
	int port = (FPTR)param;
	static const char *const portnames[2][3] =
			{
				{ "SUPERSCOPE1", "SUPERSCOPE1_X", "SUPERSCOPE1_Y" },
				{ "SUPERSCOPE2", "SUPERSCOPE2_X", "SUPERSCOPE2_Y" },
			};

	INT16 x = input_port_read(field->port->machine(), portnames[port][1]);
	INT16 y = input_port_read(field->port->machine(), portnames[port][2]);

	/* these are the theoretical boundaries, but we currently are always onscreen... */
	if (x < 0 || x >= SNES_SCR_WIDTH || y < 0 || y >= snes_ppu.beam.last_visible_line)
		state->m_scope[port].offscreen = 1;
	else
		state->m_scope[port].offscreen = 0;

	return state->m_scope[port].offscreen;
}

static TIMER_CALLBACK( lightgun_tick )
{
	if ((input_port_read(machine, "CTRLSEL") & 0x0f) == 0x03 || (input_port_read(machine, "CTRLSEL") & 0x0f) == 0x04)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
	}

	if ((input_port_read(machine, "CTRLSEL") & 0xf0) == 0x30 || (input_port_read(machine, "CTRLSEL") & 0xf0) == 0x40)
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_START("SERIAL1_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CATEGORY(11)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CATEGORY(11)

	PORT_START("SERIAL2_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_START("SERIAL2_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CATEGORY(21)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CATEGORY(21)

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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Mouse Button Left") PORT_PLAYER(1) PORT_CATEGORY(12)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Mouse Button Right") PORT_PLAYER(1) PORT_CATEGORY(12)

	PORT_START("MOUSE1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CATEGORY(12)

	PORT_START("MOUSE1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CATEGORY(12)

	PORT_START("MOUSE2")
	/* bits 0,3 = mouse signature (must be 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 4,5 = mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused */
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_mouse_speed_input, (void *)1)
	/* bits 6,7 = mouse buttons */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Mouse Button Left") PORT_PLAYER(2) PORT_CATEGORY(22)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Mouse Button Right") PORT_PLAYER(2) PORT_CATEGORY(22)

	PORT_START("MOUSE2_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2) PORT_CATEGORY(22)

	PORT_START("MOUSE2_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2) PORT_CATEGORY(22)
INPUT_PORTS_END

static INPUT_PORTS_START( snes_superscope )
	PORT_START("SUPERSCOPE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )	// Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_superscope_offscreen_input, (void *)0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port1 Superscope Pause") PORT_PLAYER(1) PORT_CATEGORY(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port1 Superscope Turbo") PORT_TOGGLE PORT_PLAYER(1) PORT_CATEGORY(13)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port1 Superscope Cursor") PORT_PLAYER(1) PORT_CATEGORY(13)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port1 Superscope Fire") PORT_PLAYER(1) PORT_CATEGORY(13)

	PORT_START("SUPERSCOPE1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port1 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CATEGORY(13)

	PORT_START("SUPERSCOPE1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port1 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CATEGORY(13)

	PORT_START("SUPERSCOPE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )	// Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snes_superscope_offscreen_input, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port2 Superscope Pause") PORT_PLAYER(2) PORT_CATEGORY(23)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port2 Superscope Turbo") PORT_PLAYER(2) PORT_CATEGORY(23)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port2 Superscope Cursor") PORT_PLAYER(2) PORT_CATEGORY(23)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port2 Superscope Fire") PORT_PLAYER(2) PORT_CATEGORY(23)

	PORT_START("SUPERSCOPE2_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port2 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CATEGORY(23)

	PORT_START("SUPERSCOPE2_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port2 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CATEGORY(23)
INPUT_PORTS_END

static INPUT_PORTS_START( snes )
	PORT_START("CTRLSEL")  /* Select Controller Type */
	PORT_CATEGORY_CLASS( 0x0f, 0x01, "P1 Controller")
	PORT_CATEGORY_ITEM(  0x00, "Unconnected",		10 )
	PORT_CATEGORY_ITEM(  0x01, "Gamepad",		11 )
	PORT_CATEGORY_ITEM(  0x02, "Mouse",			12 )
	PORT_CATEGORY_ITEM(  0x03, "Superscope",		13 )
//  PORT_CATEGORY_ITEM(  0x04, "Justfier",      14 )
//  PORT_CATEGORY_ITEM(  0x05, "Multitap",      15 )
	PORT_CATEGORY_CLASS( 0xf0, 0x10, "P2 Controller")
	PORT_CATEGORY_ITEM(  0x00, "Unconnected",		20 )
	PORT_CATEGORY_ITEM(  0x10, "Gamepad",		21 )
	PORT_CATEGORY_ITEM(  0x20, "Mouse",			22 )
	PORT_CATEGORY_ITEM(  0x30, "Superscope",		23 )
//  PORT_CATEGORY_ITEM(  0x40, "Justfier",      24 )
//  PORT_CATEGORY_ITEM(  0x50, "Multitap",      25 )

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

	state->m_data1[port] = input_port_read(machine, portnames[port][0]) | (input_port_read(machine, portnames[port][1]) << 8);
	state->m_data2[port] = input_port_read(machine, portnames[port][2]) | (input_port_read(machine, portnames[port][3]) << 8);

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

	state->m_mouse[port].buttons = input_port_read(machine, portnames[port][0]);
	state->m_mouse[port].x = input_port_read(machine, portnames[port][1]);
	state->m_mouse[port].y = input_port_read(machine, portnames[port][2]);
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
	state->m_scope[port].x = input_port_read(machine, portnames[port][1]);
	state->m_scope[port].y = input_port_read(machine, portnames[port][2]);
	input = input_port_read(machine, portnames[port][0]);

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
	UINT8 ctrl1 = input_port_read(machine, "CTRLSEL") & 0x0f;
	UINT8 ctrl2 = (input_port_read(machine, "CTRLSEL") & 0xf0) >> 4;

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine.scheduler().timer_set(attotime::zero, FUNC(lightgun_tick));

	switch (ctrl1)
	{
	case 1:	/* SNES joypad */
		snes_input_read_joy(machine, 0);
		break;
	case 2:	/* SNES Mouse */
		snes_input_read_mouse(machine, 0);
		break;
	case 3:	/* SNES Superscope */
		snes_input_read_superscope(machine, 0);
		break;
	case 0:	/* no controller in port1 */
	default:
		state->m_data1[0] = 0;
		state->m_data2[0] = 0;
		break;
	}

	switch (ctrl2)
	{
	case 1:	/* SNES joypad */
		snes_input_read_joy(machine, 1);
		break;
	case 2:	/* SNES Mouse */
		snes_input_read_mouse(machine, 1);
		break;
	case 3:	/* SNES Superscope */
		snes_input_read_superscope(machine, 1);
		break;
	case 0:	/* no controller in port2 */
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
	UINT8 ctrl1 = input_port_read(machine, "CTRLSEL") & 0x0f;
	UINT8 res = 0;

	switch (ctrl1)
	{
	case 1:	/* SNES joypad */
		if (state->m_read_idx[0] >= 16)
			res = 0x01;
		else
			res = (state->m_joypad[0].buttons >> (15 - state->m_read_idx[0]++)) & 0x01;
		break;
	case 2:	/* SNES Mouse */
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
	case 3:	/* SNES Superscope */
		if (state->m_read_idx[0] >= 8)
			res = 0x01;
		else
			res = (state->m_scope[0].buttons >> (7 - state->m_read_idx[0]++)) & 0x01;
		break;
	case 0:	/* no controller in port2 */
	default:
		break;
	}

	return res;
}

static UINT8 snes_oldjoy2_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 ctrl2 = (input_port_read(machine, "CTRLSEL") & 0xf0) >> 4;
	UINT8 res = 0;

	switch (ctrl2)
	{
	case 1:	/* SNES joypad */
		if (state->m_read_idx[1] >= 16)
			res = 0x01;
		else
			res = (state->m_joypad[1].buttons >> (15 - state->m_read_idx[1]++)) & 0x01;
		break;
	case 2:	/* SNES Mouse */
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
	case 3:	/* SNES Superscope */
		if (state->m_read_idx[1] >= 8)
			res = 0x01;
		else
			res = (state->m_scope[1].buttons >> (7 - state->m_read_idx[1]++)) & 0x01;
		break;
	case 0:	/* no controller in port2 */
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
	MCFG_CPU_ADD("maincpu", _5A22, MCLK_NTSC)	/* 2.68 MHz, also 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(snes_map)

	MCFG_CPU_ADD("soundcpu", SPC700, 1024000)	/* 1.024 MHz */
	MCFG_CPU_PROGRAM_MAP(spc_map)

	//MCFG_QUANTUM_TIME(attotime::from_hz(48000))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_MACHINE_START(snes_mess)
	MCFG_MACHINE_RESET(snes_mess)

	/* video hardware */
	MCFG_VIDEO_START(snes)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE(snes)

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
	DEVCB_LINE(snes_extern_irq_w)	/* IRQ line from cart */
};

static MACHINE_CONFIG_DERIVED( snessfx, snes )

	MCFG_CPU_ADD("superfx", SUPERFX, 21480000)	/* 21.48MHz */
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

static MACHINE_CONFIG_DERIVED( snespal, snes )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( MCLK_PAL )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_PAL, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespsfx, snespal )

	MCFG_CPU_ADD("superfx", SUPERFX, 21480000)	/* 21.48MHz */
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

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )		/* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesdsp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
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

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x11000, "addons", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "st010.bin",    0x000000, 0x011000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesst11 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x11000, "addons", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "st011.bin",    0x000000, 0x011000, CRC(34d2952c) SHA1(1375b8c1efc8cae4962b57dfe22f6b78e1ddacc8) )

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x1000, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snessfx )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )		/* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snespal )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )		/* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snespdsp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
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

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )		/* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( MAX_SNES_CART_SIZE, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesst )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", ROMREGION_ERASE00 )		/* add-on chip ROMs (DSP, SFX, etc) */

	ROM_REGION( 0x40000, "sufami", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
	ROM_LOAD( "shvc-qh-0.bin", 0,	0x40000, CRC(9b4ca911) SHA1(ef86ea192eed03d5c413fdbbfd46043be1d7a127) )

	ROM_REGION( MAX_SNES_CART_SIZE, "slot_a", ROMREGION_ERASE00 )
	ROM_REGION( MAX_SNES_CART_SIZE, "slot_b", ROMREGION_ERASE00 )
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF)
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)
ROM_END

ROM_START( snesbsx )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "user5", 0 )		/* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) )	/* boot rom */

	ROM_REGION( 0x10000, "addons", 0 )		/* add-on chip ROMs (DSP, SFX, etc) */
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
CONS( 1989, snes,     0,      0,      snes,     snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespal,  snes,   0,      snespal,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

// FIXME: the "hacked" drivers below, currently needed due to limitations in the core device design, should eventually be removed

// These would require CPU to be added/removed depending on the cart which is loaded
CONS( 1989, snesdsp,  snes,   0,      snesdsp,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/DSP-x)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespdsp, snes,   0,      snespdsp, snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/DSP-x)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snessfx,  snes,   0,      snessfx,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/SuperFX)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1991, snespsfx, snes,   0,      snespsfx, snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/SuperFX)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snesst10,  snes,   0,      snesst10,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/ST-010)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
CONS( 1989, snesst11,  snes,   0,      snesst11,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/ST-011)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
//CONS( 1989, snessa1,  snes,   0,      snessa1,  snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/SA-1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
//CONS( 1991, snespsa1, snes,   0,      snespsa1, snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System (PAL, w/SA-1)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

// These would require cartslot to be added/removed depending on the cart which is loaded
CONS( 1989, snesst,   snes,   0,      snesst,  snes,  snesst,       "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/Sufami Turbo)", GAME_NOT_WORKING )
CONS( 1989, snesbsx,  snes,   0,      snesbsx, snes,  snes_mess,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC, w/BS-X Satellaview slotted cart)",  GAME_NOT_WORKING )
