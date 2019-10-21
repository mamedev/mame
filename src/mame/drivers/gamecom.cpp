// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/***************************************************************************

Driver file to handle emulation of the Tiger Game.com by Wilbert Pol.
Various improvements by Robbbert.

Todo:
- Fix cpu and system problems that prevent the games from working fully.
- RS232 port
- Sound ports 1,2 do not sound anything like the real thing
- Sound port 3 (noise channel)
- Sound dac port (mostly works but is the wrong speed in some places).
  dac pitch is controlled by how often TIM1_INT occurs. This same
  interrupt also controls the seconds countdown in some games, such as
  Quiz Wiz and Scrabble. Currently this countdown goes twice as fast
  as it should. If the INT is slowed down to compensate, the dac sound
  is so slow as to be unintelligible. Need to find a way to keep both happy.
- System seems slower than it should. Probably wrong cycle count in the CPU.
  What we have there is a guess as the real info has not been found.
  -speed 1.2 makes the sound more natural
  -speed 1.7 if TIM1_INT is slowed to fix the countdown.

Game Status:
- Inbuilt ROM and PDA functions all work
- Due to an irritating message, the NVRAM is commented out in the machine config
- All carts appear to work, from my limited testing.
-- indy500 skips some speech just before the trial race starts.

***************************************************************************/

#include "emu.h"
#include "includes/gamecom.h"

#include "sound/volt_reg.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "gamecom.lh"


void gamecom_state::gamecom_mem_map(address_map &map)
{
	map(0x0000, 0x0013).ram().region("maincpu", 0x00);
	map(0x0014, 0x0017).rw(FUNC(gamecom_state::gamecom_pio_r), FUNC(gamecom_state::gamecom_pio_w));        // buttons
	map(0x0018, 0x001F).ram().region("maincpu", 0x18);
	map(0x0020, 0x007F).rw(FUNC(gamecom_state::gamecom_internal_r), FUNC(gamecom_state::gamecom_internal_w));/* CPU internal register file */
	map(0x0080, 0x03FF).ram().region("maincpu", 0x80);                     /* RAM */
	map(0x0400, 0x0FFF).noprw();                                                /* Nothing */
	map(0x1000, 0x1FFF).rom();                                                /* Internal ROM (initially), or External ROM/Flash. Controlled by MMU0 (never swapped out in game.com) */
	map(0x2000, 0x3FFF).bankr("bank1");                                   /* External ROM/Flash. Controlled by MMU1 */
	map(0x4000, 0x5FFF).bankr("bank2");                                   /* External ROM/Flash. Controlled by MMU2 */
	map(0x6000, 0x7FFF).bankr("bank3");                                   /* External ROM/Flash. Controlled by MMU3 */
	map(0x8000, 0x9FFF).bankr("bank4");                                   /* External ROM/Flash. Controlled by MMU4 */
	map(0xA000, 0xDFFF).writeonly().share("videoram").nopr();             /* VRAM - writeonly, returns 0 on read, as expected by lostwrld */
	map(0xE000, 0xFFFF).ram().share("nvram");           /* Extended I/O, Extended RAM */
}

static INPUT_PORTS_START( gamecom )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME( "Up" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME( "Down" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME( "Left" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME( "Right" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Menu" ) PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( DEF_STR(Pause) ) PORT_CODE( KEYCODE_V )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Sound" ) PORT_CODE( KEYCODE_S )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Button A" ) PORT_CODE( KEYCODE_A ) PORT_CODE( KEYCODE_LCONTROL )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Button B" ) PORT_CODE( KEYCODE_B ) PORT_CODE( KEYCODE_LALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Button C" ) PORT_CODE( KEYCODE_C ) PORT_CODE( KEYCODE_SPACE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Reset" ) PORT_CODE( KEYCODE_N )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Button D" ) PORT_CODE( KEYCODE_D ) PORT_CODE( KEYCODE_LSHIFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Stylus press" ) PORT_CODE( KEYCODE_Z ) PORT_CODE( MOUSECODE_BUTTON1 )

	// These are used by the "Default Grid" artwork to detect mouse clicks
	PORT_START("GRID.0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.6")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.7")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.8")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.9")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.10")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.11")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.12")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)
	INPUT_PORTS_END

void gamecom_state::gamecom_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0x00, 0x00, 0x00); // Black
	palette.set_pen_color(1, 0x0f, 0x4f, 0x2f); // Gray 1
	palette.set_pen_color(2, 0x6f, 0x8f, 0x4f); // Gray 2
	palette.set_pen_color(3, 0x8f, 0xcf, 0x8f); // Grey 3
	palette.set_pen_color(4, 0xdf, 0xff, 0x8f); // White
}

uint32_t gamecom_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

INTERRUPT_GEN_MEMBER(gamecom_state::gamecom_interrupt)
{
	m_maincpu->set_input_line(sm8500_cpu_device::LCDC_INT, ASSERT_LINE );
}

void gamecom_state::gamecom(machine_config &config)
{
	/* basic machine hardware */
	SM8500(config, m_maincpu, XTAL(11'059'200)/2);   /* actually it's an sm8521 microcontroller containing an sm8500 cpu */
	m_maincpu->set_addrmap(AS_PROGRAM, &gamecom_state::gamecom_mem_map);
	m_maincpu->dma_cb().set(FUNC(gamecom_state::gamecom_handle_dma));
	m_maincpu->timer_cb().set(FUNC(gamecom_state::gamecom_update_timers));
	m_maincpu->set_vblank_int("screen", FUNC(gamecom_state::gamecom_interrupt));

	config.m_minimum_quantum = attotime::from_hz(60);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(59.732155);
	m_screen->set_vblank_time(500);
	m_screen->set_screen_update(FUNC(gamecom_state::screen_update));
	m_screen->set_size(200, 160);
	m_screen->set_visarea_full();
	m_screen->set_palette("palette");

	config.set_default_layout(layout_gamecom);
	PALETTE(config, "palette", FUNC(gamecom_state::gamecom_palette), 5);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	/* TODO: much more complex than this */
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC (Digital audio)
	DAC_4BIT_R2R(config, m_dac0, 0).add_route(ALL_OUTPUTS, "speaker", 0.05); // unknown DAC (Frequency modulation)
	DAC_4BIT_R2R(config, m_dac1, 0).add_route(ALL_OUTPUTS, "speaker", 0.05); // unknown DAC (Frequency modulation)
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac0", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac0", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "dac1", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "dac1", -1.0, DAC_VREF_NEG_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot1", generic_linear_slot, "gamecom_cart", "bin,tgc").set_device_load(FUNC(gamecom_state::cart1_load), this);
	GENERIC_CARTSLOT(config, "cartslot2", generic_linear_slot, "gamecom_cart", "bin,tgc").set_device_load(FUNC(gamecom_state::cart2_load), this);
	SOFTWARE_LIST(config, "cart_list").set_original("gamecom");
}

ROM_START( gamecom )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "internal.bin", 0x1000,  0x1000, CRC(a0cec361) SHA1(03368237e8fed4a8724f3b4a1596cf4b17c96d33) )

	ROM_REGION( 0x40000, "kernel", 0 )
	ROM_LOAD( "external.bin", 0x00000, 0x40000, CRC(e235a589) SHA1(97f782e72d738f4d7b861363266bf46b438d9b50) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY  FULLNAME    FLAGS
CONS( 1997, gamecom, 0,      0,      gamecom, gamecom, gamecom_state, init_gamecom, "Tiger", "Game.com", MACHINE_IMPERFECT_SOUND | MACHINE_CLICKABLE_ARTWORK)
