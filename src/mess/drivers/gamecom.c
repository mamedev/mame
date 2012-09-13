/***************************************************************************

Driver file to handle emulation of the Tiger Game.com by
  Wilbert Pol

Todo:
  everything
  - Finish memory map, fill in details
  - Finish input ports
  - Finish palette code
  - Finish machine driver struct
  - Finish cartslot code
  - Etc, etc, etc.


***************************************************************************/

#include "includes/gamecom.h"

static ADDRESS_MAP_START(gamecom_mem_map, AS_PROGRAM, 8, gamecom_state)
	AM_RANGE( 0x0000, 0x0013 )  AM_RAM
	AM_RANGE( 0x0014, 0x0017 )  AM_READWRITE( gamecom_pio_r, gamecom_pio_w )        // buttons
	AM_RANGE( 0x0018, 0x001F )  AM_RAM
	AM_RANGE( 0x0020, 0x007F )  AM_READWRITE( gamecom_internal_r, gamecom_internal_w )/* CPU internal register file */
	AM_RANGE( 0x0080, 0x03FF )  AM_RAM                                                /* RAM */
	AM_RANGE( 0x0400, 0x0FFF )  AM_NOP                                                /* Nothing */
	AM_RANGE( 0x1000, 0x1FFF )  AM_ROM                                                /* Internal ROM (initially), or External ROM/Flash. Controlled by MMU0 (never swapped out in game.com) */
	AM_RANGE( 0x2000, 0x3FFF )  AM_ROMBANK("bank1")                                   /* External ROM/Flash. Controlled by MMU1 */
	AM_RANGE( 0x4000, 0x5FFF )  AM_ROMBANK("bank2")                                   /* External ROM/Flash. Controlled by MMU2 */
	AM_RANGE( 0x6000, 0x7FFF )  AM_ROMBANK("bank3")                                   /* External ROM/Flash. Controlled by MMU3 */
	AM_RANGE( 0x8000, 0x9FFF )  AM_ROMBANK("bank4")                                   /* External ROM/Flash. Controlled by MMU4 */
	AM_RANGE( 0xA000, 0xDFFF )  AM_RAM AM_SHARE("p_videoram")			  /* VRAM */
	AM_RANGE( 0xE000, 0xFFFF )  AM_RAM AM_SHARE("p_nvram")// AM_SHARE("nvram")           /* Extended I/O, Extended RAM */
ADDRESS_MAP_END

static const SM8500_CONFIG gamecom_cpu_config = {
	gamecom_handle_dma,
	gamecom_update_timers
};

static INPUT_PORTS_START( gamecom )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME( "Up" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME( "Down" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME( "Left" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME( "Right" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Menu" ) PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( DEF_STR(Pause) ) PORT_CODE( KEYCODE_V )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Sound" ) PORT_CODE( KEYCODE_B )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Button A" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Button B" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Button C" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Reset" ) PORT_CODE( KEYCODE_N )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Button D" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Stylus press" ) PORT_CODE( KEYCODE_Z ) PORT_CODE( MOUSECODE_BUTTON1 )

	PORT_START("STYX")
	PORT_BIT( 0xff, 100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0,199) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)

	PORT_START("STYY")
	PORT_BIT( 0xff, 80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0,159) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
INPUT_PORTS_END

static const unsigned char palette[] =
{
	0xDF, 0xFF, 0x8F,	/* White */
	0x8F, 0xCF, 0x8F,	/* Gray 3 */
	0x6F, 0x8F, 0x4F,	/* Gray 2 */
	0x0F, 0x4F, 0x2F,	/* Gray 1 */
	0x00, 0x00, 0x00,	/* Black */
};

void gamecom_state::palette_init()
{
	int index;
	for ( index = 0; index < 5; index++ )
	{
		palette_set_color_rgb(machine(),  4-index, palette[index*3+0], palette[index*3+1], palette[index*3+2] );
	}
}

UINT32 gamecom_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

static INTERRUPT_GEN( gamecom_interrupt )
{
	device->machine().device("maincpu")->execute().set_input_line(LCDC_INT, ASSERT_LINE );
}

static MACHINE_CONFIG_START( gamecom, gamecom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", SM8500, XTAL_11_0592MHz/2 )   /* actually it's an sm8521 microcontroller containing an sm8500 cpu */
	MCFG_CPU_PROGRAM_MAP( gamecom_mem_map)
	MCFG_CPU_CONFIG( gamecom_cpu_config )
	MCFG_CPU_VBLANK_INT("screen", gamecom_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	//MCFG_NVRAM_ADD_0FILL("nvram")


	/* video hardware */

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE( 59.732155 )
	MCFG_SCREEN_VBLANK_TIME(500)
	MCFG_SCREEN_UPDATE_DRIVER(gamecom_state, screen_update)
	MCFG_SCREEN_SIZE( 200, 200 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 199, 0, 159 )

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_LENGTH(5)

	/* sound hardware */
#if 0
	MCFG_SPEAKER_STANDARD_STEREO( "left", "right" )
	/* MCFG_SOUND_ADD( "custom", CUSTOM, 0 ) */
	/* MCFG_SOUND_CONFIG */
	MCFG_SOUND_ROUTE( 0, "left", 0.50 )
	MCFG_SOUND_ROUTE( 1, "right", 0.50 )
#endif

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,tgc")
	MCFG_CARTSLOT_INTERFACE("gamecom_cart")
	MCFG_CARTSLOT_LOAD(gamecom_cart1)
	MCFG_SOFTWARE_LIST_ADD("cart_list","gamecom")
	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,tgc")
	MCFG_CARTSLOT_INTERFACE("gamecom_cart")
	MCFG_CARTSLOT_LOAD(gamecom_cart2)
MACHINE_CONFIG_END

ROM_START( gamecom )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "internal.bin", 0x1000,  0x1000, CRC(a0cec361) SHA1(03368237e8fed4a8724f3b4a1596cf4b17c96d33) )

	ROM_REGION( 0x40000, "kernel", 0 )
	ROM_LOAD( "external.bin", 0x00000, 0x40000, CRC(e235a589) SHA1(97f782e72d738f4d7b861363266bf46b438d9b50) )

	ROM_REGION( 0x200000, "cart1", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart2", ROMREGION_ERASEFF )
ROM_END

/*    YEAR  NAME     PARENT COMPAT MACHINE  INPUT    INIT    COMPANY  FULLNAME */
CONS( 1997, gamecom, 0,     0,     gamecom, gamecom, gamecom_state, gamecom,"Tiger", "Game.com", GAME_NOT_WORKING | GAME_NO_SOUND)
