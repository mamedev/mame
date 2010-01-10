/***************************************************************************

    Atari Shuuz hardware

    driver by Aaron Giles

    Games supported:
        * Shuuz (1990) [2 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/shuuz.h"
#include "sound/okim6295.h"



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	shuuz_state *state = (shuuz_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 4, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Video controller access
 *
 *************************************/

static READ16_HANDLER( shuuz_atarivc_r )
{
	return atarivc_r(space->machine->primary_screen, offset);
}


static WRITE16_HANDLER( shuuz_atarivc_w )
{
	atarivc_w(space->machine->primary_screen, offset, data, mem_mask);
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static MACHINE_START( shuuz )
{
	atarigen_init(machine);
}


static MACHINE_RESET( shuuz )
{
	shuuz_state *state = (shuuz_state *)machine->driver_data;

	atarigen_eeprom_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarivc_reset(machine->primary_screen, state->atarigen.atarivc_eof_data, 1);
}


static WRITE16_HANDLER( latch_w )
{
}



/*************************************
 *
 *  LETA I/O
 *
 *************************************/

static READ16_HANDLER( leta_r )
{
	/* trackball -- rotated 45 degrees? */
	static int cur[2];
	int which = offset & 1;

	/* when reading the even ports, do a real analog port update */
	if (which == 0)
	{
		int dx = (INT8)input_port_read(space->machine, "TRACKX");
		int dy = (INT8)input_port_read(space->machine, "TRACKY");

		cur[0] = dx + dy;
		cur[1] = dx - dy;
	}

	/* clip the result to -0x3f to +0x3f to remove directional ambiguities */
	return cur[which];
}



/*************************************
 *
 *  Additional I/O
 *
 *************************************/

static READ16_HANDLER( special_port0_r )
{
	int result = input_port_read(space->machine, "SYSTEM");

	if ((result & 0x0800) && atarigen_get_hblank(space->machine->primary_screen))
		result &= ~0x0800;

	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_READWRITE(atarigen_eeprom_r, atarigen_eeprom_w) AM_BASE_SIZE_MEMBER(shuuz_state, atarigen.eeprom, atarigen.eeprom_size)
	AM_RANGE(0x101000, 0x101fff) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x102000, 0x102001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x103000, 0x103003) AM_READ(leta_r)
	AM_RANGE(0x105000, 0x105001) AM_READWRITE(special_port0_r, latch_w)
	AM_RANGE(0x105002, 0x105003) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x106000, 0x106001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x107000, 0x107007) AM_NOP
	AM_RANGE(0x3e0000, 0x3e087f) AM_RAM_WRITE(atarigen_666_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3effc0, 0x3effff) AM_READWRITE(shuuz_atarivc_r, shuuz_atarivc_w) AM_BASE_MEMBER(shuuz_state, atarigen.atarivc_data)
	AM_RANGE(0x3f4000, 0x3f5eff) AM_RAM_WRITE(atarigen_playfield_latched_msb_w) AM_BASE_MEMBER(shuuz_state, atarigen.playfield)
	AM_RANGE(0x3f5f00, 0x3f5f7f) AM_RAM AM_BASE_MEMBER(shuuz_state, atarigen.atarivc_eof_data)
	AM_RANGE(0x3f5f80, 0x3f5fff) AM_RAM_WRITE(atarimo_0_slipram_w) AM_BASE(&atarimo_0_slipram)
	AM_RANGE(0x3f6000, 0x3f7fff) AM_RAM_WRITE(atarigen_playfield_upper_w) AM_BASE_MEMBER(shuuz_state, atarigen.playfield_upper)
	AM_RANGE(0x3f8000, 0x3fcfff) AM_RAM
	AM_RANGE(0x3fd000, 0x3fd3ff) AM_RAM_WRITE(atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0x3fd400, 0x3fffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( shuuz )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x07fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x07fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX")
    PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY")
    PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( shuuz2 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Step Debug SW") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0600, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Playfield Debug SW") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Debug SW") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Crosshair Debug SW") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Freeze Debug SW") PORT_CODE(KEYCODE_F)

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Replay Debug SW") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0600, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("TRACKX")
    PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY")
    PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, 0+RGN_FRAC(1,2), 4+RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static GFXDECODE_START( shuuz )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  256, 16 )		/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx2", 0, pfmolayout,    0, 16 )		/* sprites & playfield */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( shuuz )
	MDRV_DRIVER_DATA(shuuz_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_MACHINE_START(shuuz)
	MDRV_MACHINE_RESET(shuuz)
	MDRV_NVRAM_HANDLER(atarigen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(shuuz)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)

	MDRV_VIDEO_START(shuuz)
	MDRV_VIDEO_UPDATE(shuuz)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, ATARI_CLOCK_14MHz/16)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( shuuz )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136083-4010.23p",     0x00000, 0x20000, CRC(1c2459f8) SHA1(4b8daf196e3ba17cf958a3c1af4e4dacfb79b9e7) )
	ROM_LOAD16_BYTE( "136083-4011.13p",     0x00001, 0x20000, CRC(6db53a85) SHA1(7f9b3ea78fa65221931bfdab1aa5f1913ffed753) )

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136083-2030.43x", 0x000000, 0x20000, CRC(8ecf1ed8) SHA1(47143f1eaf43027c5301eb6009d8a56a98328894) )
	ROM_LOAD( "136083-2032.20x", 0x020000, 0x20000, CRC(5af184e6) SHA1(630969466c606d1f51da81911fb365a4cac4685c) )
	ROM_LOAD( "136083-2031.87x", 0x040000, 0x20000, CRC(72e9db63) SHA1(be13830b38c2603bbd6b875abdc1675788a60b24) )
	ROM_LOAD( "136083-2033.65x", 0x060000, 0x20000, CRC(8f552498) SHA1(7fd323f3b30747a8645d7a9676fdf8f973b6632a) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136083-1020.43u", 0x000000, 0x20000, CRC(d21ad039) SHA1(5389745eff6690c1890f98a9630869b1084fb2f3) )
	ROM_LOAD( "136083-1022.20u", 0x020000, 0x20000, CRC(0c10bc90) SHA1(11272757ecad42a4fae49046bd1b01d5ff7f7d4f) )
	ROM_LOAD( "136083-1024.43m", 0x040000, 0x20000, CRC(adb09347) SHA1(5294dfb3d4aa83525795ca03c2f328ab9a666baf) )
	ROM_LOAD( "136083-1026.20m", 0x060000, 0x20000, CRC(9b20e13d) SHA1(726b6fb548c0906a5baa90b9698f99a6af9ecc36) )
	ROM_LOAD( "136083-1021.87u", 0x080000, 0x20000, CRC(8388910c) SHA1(62c6b1885bed042ef72fb62464923a33f9b464f1) )
	ROM_LOAD( "136083-1023.65u", 0x0a0000, 0x20000, CRC(71353112) SHA1(0aab14379e1b562b81cdd52eb209e264a12232c4) )
	ROM_LOAD( "136083-1025.87m", 0x0c0000, 0x20000, CRC(f7b20a64) SHA1(667c539fa809d3ae4a1c127e2044dd3a4e533266) )
	ROM_LOAD( "136083-1027.65m", 0x0e0000, 0x20000, CRC(55d54952) SHA1(73e1a388ea48bab567bde8958ee228432ebfbf67) )

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM data */
	ROM_LOAD( "136083-1040.75b", 0x00000, 0x20000, CRC(0896702b) SHA1(d826bb4812d393889584c7c656c317fd5745a05f) )
	ROM_LOAD( "136083-1041.65b", 0x20000, 0x20000, CRC(b3b07ce9) SHA1(f1128a143b72867c16b9803b0beb0188420cbfb5) )
ROM_END


ROM_START( shuuz2 )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136083-23p.rom",     0x00000, 0x20000, CRC(98aec4e7) SHA1(8cbe6e7835ecf0ef74a2de723ef970a63d3bddd1) )
	ROM_LOAD16_BYTE( "136083-13p.rom",     0x00001, 0x20000, CRC(dd9d5d5c) SHA1(0bde6be55532c232b1d27824c2ce61f33501cbb0) )

	ROM_REGION( 0x080000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136083-2030.43x", 0x000000, 0x20000, CRC(8ecf1ed8) SHA1(47143f1eaf43027c5301eb6009d8a56a98328894) )
	ROM_LOAD( "136083-2032.20x", 0x020000, 0x20000, CRC(5af184e6) SHA1(630969466c606d1f51da81911fb365a4cac4685c) )
	ROM_LOAD( "136083-2031.87x", 0x040000, 0x20000, CRC(72e9db63) SHA1(be13830b38c2603bbd6b875abdc1675788a60b24) )
	ROM_LOAD( "136083-2033.65x", 0x060000, 0x20000, CRC(8f552498) SHA1(7fd323f3b30747a8645d7a9676fdf8f973b6632a) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136083-1020.43u", 0x000000, 0x20000, CRC(d21ad039) SHA1(5389745eff6690c1890f98a9630869b1084fb2f3) )
	ROM_LOAD( "136083-1022.20u", 0x020000, 0x20000, CRC(0c10bc90) SHA1(11272757ecad42a4fae49046bd1b01d5ff7f7d4f) )
	ROM_LOAD( "136083-1024.43m", 0x040000, 0x20000, CRC(adb09347) SHA1(5294dfb3d4aa83525795ca03c2f328ab9a666baf) )
	ROM_LOAD( "136083-1026.20m", 0x060000, 0x20000, CRC(9b20e13d) SHA1(726b6fb548c0906a5baa90b9698f99a6af9ecc36) )
	ROM_LOAD( "136083-1021.87u", 0x080000, 0x20000, CRC(8388910c) SHA1(62c6b1885bed042ef72fb62464923a33f9b464f1) )
	ROM_LOAD( "136083-1023.65u", 0x0a0000, 0x20000, CRC(71353112) SHA1(0aab14379e1b562b81cdd52eb209e264a12232c4) )
	ROM_LOAD( "136083-1025.87m", 0x0c0000, 0x20000, CRC(f7b20a64) SHA1(667c539fa809d3ae4a1c127e2044dd3a4e533266) )
	ROM_LOAD( "136083-1027.65m", 0x0e0000, 0x20000, CRC(55d54952) SHA1(73e1a388ea48bab567bde8958ee228432ebfbf67) )

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM data */
	ROM_LOAD( "136083-1040.75b", 0x00000, 0x20000, CRC(0896702b) SHA1(d826bb4812d393889584c7c656c317fd5745a05f) )
	ROM_LOAD( "136083-1041.65b", 0x20000, 0x20000, CRC(b3b07ce9) SHA1(f1128a143b72867c16b9803b0beb0188420cbfb5) )
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, shuuz,  0,     shuuz, shuuz,  0, ROT0, "Atari Games", "Shuuz (version 8.0)", 0 )
GAME( 1990, shuuz2, shuuz, shuuz, shuuz2, 0, ROT0, "Atari Games", "Shuuz (version 7.1)", 0 )
