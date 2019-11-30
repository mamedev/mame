// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Centuri Aztarac hardware

    driver by Mathis Rosenhauer
    Thanks to David Fish for additional hardware information.

    Games supported:
        * Aztarac

    Known bugs:
        * none at this time

***************************************************************************/

#include "emu.h"
#include "includes/aztarac.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "speaker.h"


/*************************************
 *
 *  Machine init
 *
 *************************************/

void aztarac_state::machine_start()
{
	save_item(NAME(m_sound_status));
}

void aztarac_state::machine_reset()
{
	m_nvram->recall(1);
	m_nvram->recall(0);
}



/*************************************
 *
 *  NVRAM handler
 *
 *************************************/

void aztarac_state::nvram_store_w(uint16_t data)
{
	m_nvram->store(1);
	m_nvram->store(0);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

READ16_MEMBER(aztarac_state::joystick_r)
{
	return (((ioport("STICKZ")->read() - 0xf) << 8) |
			((ioport("STICKY")->read() - 0xf) & 0xff));
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void aztarac_state::main_map(address_map &map)
{
	map(0x000000, 0x00bfff).rom();
	map(0x021000, 0x021001).w(FUNC(aztarac_state::nvram_store_w));
	map(0x022000, 0x0221ff).rw(m_nvram, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0x00ff);
	map(0x027000, 0x027001).r(FUNC(aztarac_state::joystick_r));
	map(0x027004, 0x027005).portr("INPUTS");
	map(0x027008, 0x027009).rw(FUNC(aztarac_state::sound_r), FUNC(aztarac_state::sound_w));
	map(0x02700c, 0x02700d).portr("DIAL");
	map(0x02700e, 0x02700f).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0xff8000, 0xffafff).ram().share("vectorram");
	map(0xffb000, 0xffb001).nopr();
	map(0xffb001, 0xffb001).w(FUNC(aztarac_state::ubr_w));
	map(0xffe000, 0xffffff).ram();
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void aztarac_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8800).r(FUNC(aztarac_state::snd_command_r));
	map(0x8c00, 0x8c01).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x8c02, 0x8c03).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x8c04, 0x8c05).rw("ay3", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x8c06, 0x8c07).rw("ay4", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x9000, 0x9000).rw(FUNC(aztarac_state::snd_status_r), FUNC(aztarac_state::snd_status_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( aztarac )
	PORT_START("STICKZ")
	PORT_BIT( 0x1f, 0xf, IPT_AD_STICK_Z ) PORT_MINMAX(0,0x1e) PORT_SENSITIVITY(100) PORT_KEYDELTA(1)

	PORT_START("STICKY")
	PORT_BIT( 0x1f, 0xf, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x1e) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_REVERSE

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE

	PORT_START("INPUTS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_LOW)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void aztarac_state::aztarac(machine_config &config)
{
	/* basic machine hardware */
	m68000_device &maincpu(M68000(config, m_maincpu, 16_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &aztarac_state::main_map);
	maincpu.set_cpu_space(AS_PROGRAM);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &aztarac_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(aztarac_state::snd_timed_irq), attotime::from_hz(100));

	X2212(config, m_nvram);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	VECTOR(config, m_vector, 0);
	SCREEN(config, m_screen, SCREEN_TYPE_VECTOR);
	m_screen->set_refresh_hz(40);
	m_screen->set_size(400, 300);
	m_screen->set_visarea(0, 1024-1, 0, 768-1);
	m_screen->set_screen_update("vector", FUNC(vector_device::screen_update));
	m_screen->screen_vblank().set(FUNC(aztarac_state::video_interrupt));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "ay1", 16_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15);

	AY8910(config, "ay2", 16_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15);

	AY8910(config, "ay3", 16_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15);

	AY8910(config, "ay4", 16_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( aztarac )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6.l8", 0x000000, 0x001000, CRC(25f8da18) SHA1(e8179ba3683e39c8225b549ead74c8af2d0a0b3e) )
	ROM_LOAD16_BYTE( "0.n8", 0x000001, 0x001000, CRC(04e20626) SHA1(2b6a04992037257830df2c01a6da748fb4449f79) )
	ROM_LOAD16_BYTE( "7.l7", 0x002000, 0x001000, CRC(230e244c) SHA1(42283a368144acf2aad2ef390e312e0951c3ea64) )
	ROM_LOAD16_BYTE( "1.n7", 0x002001, 0x001000, CRC(37b12697) SHA1(da288b077902e3205600a021c3fac5730f9fb832) )
	ROM_LOAD16_BYTE( "8.l6", 0x004000, 0x001000, CRC(1293fb9d) SHA1(5a8d512372fd38f1a55f990f5c3eb51833c463d8) )
	ROM_LOAD16_BYTE( "2.n6", 0x004001, 0x001000, CRC(712c206a) SHA1(eb29f161189c14d84896502940e3ab6cc3bd1cd0) )
	ROM_LOAD16_BYTE( "9.l5", 0x006000, 0x001000, CRC(743a6501) SHA1(da83a8f756466bcd94d4b0cc28a1a1858e9532f3) )
	ROM_LOAD16_BYTE( "3.n5", 0x006001, 0x001000, CRC(a65cbf99) SHA1(dd06f98c0989604bd4ac6317e545e1fcf6722e75) )
	ROM_LOAD16_BYTE( "a.l4", 0x008000, 0x001000, CRC(9cf1b0a1) SHA1(dd644026f49d8430c0b4cf4c750dc33c013c19fc) )
	ROM_LOAD16_BYTE( "4.n4", 0x008001, 0x001000, CRC(5f0080d5) SHA1(fb1303f9a02067faea2ac4d523051c416de9cf35) )
	ROM_LOAD16_BYTE( "b.l3", 0x00a000, 0x001000, CRC(8cc7f7fa) SHA1(fefb9a4fdd63878bc5d8138e3e8456cb6638425a) )
	ROM_LOAD16_BYTE( "5.n3", 0x00a001, 0x001000, CRC(40452376) SHA1(1d058b7ecd2bbff3393950aab9215b262908475b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c.j4", 0x0000, 0x1000, CRC(e897dfcd) SHA1(750df3d08512d8098a13ec62677831efa164c126) )
	ROM_LOAD( "d.j3", 0x1000, 0x1000, CRC(4016de77) SHA1(7232ec003f1b9d3623d762f3270108a1d1837846) )

	ROM_REGION( 0x3000, "proms", 0 ) /* not hooked up */
	ROM_LOAD( "l5.l5", 0x0000, 0x0020, CRC(317fb438) SHA1(3130e1dbde06228707ba46ae85d8df8cc8f32b67) )
	ROM_LOAD( "k8.k8", 0x0000, 0x1000, CRC(596ad8d9) SHA1(7e2d2d3e02712911ef5ef55d1df5740f6ec28bcb) )
	ROM_LOAD( "k9.k9", 0x0000, 0x1000, CRC(b8544823) SHA1(78ff1fcb7e640929765533592015cfccef690179) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, aztarac, 0, aztarac, aztarac, aztarac_state, empty_init, ROT0, "Centuri", "Aztarac", MACHINE_SUPPORTS_SAVE )
