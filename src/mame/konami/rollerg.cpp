// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Rollergames (GX999) (c) 1991 Konami

    driver by Nicola Salmoria


    2009-03:
    Added dsw locations and verified factory settings based on Guru's notes

***************************************************************************/

#include "emu.h"

#include "k053244_k053245.h"
#include "konami_helper.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/k053252.h"
#include "machine/watchdog.h"
#include "sound/k053260.h"
#include "sound/ymopl.h"
#include "video/k051316.h"

#include "emupal.h"
#include "speaker.h"


// configurable logging
#define LOG_UNKWRITE     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE)

#include "logmacro.h"

#define LOGUNKWRITE(...)     LOGMASKED(LOG_UNKWRITE,     __VA_ARGS__)


namespace {

class rollerg_state : public driver_device
{
public:
	rollerg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053244(*this, "k053244"),
		m_k051316(*this, "k051316"),
		m_mainbank(*this, "mainbank"),
		m_zoomroms_view(*this, "zoomroms_view")
	{ }

	void rollerg(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k05324x_device> m_k053244;
	required_device<k051316_device> m_k051316;

	required_memory_bank m_mainbank;

	memory_view m_zoomroms_view;

	// misc
	emu_timer *m_nmi_blocked = nullptr;

	void ext_enable_w(uint8_t data);
	void soundirq_w(uint8_t data);
	void sound_arm_nmi_w(uint8_t data);
	void z80_nmi_w(int state);
	uint8_t pip_r();
	void irq_ack_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K05324X_CB_MEMBER(sprite_callback);
	K051316_CB_MEMBER(zoom_callback);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

K05324X_CB_MEMBER(rollerg_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };
#if 0
	if (machine().input().code_pressed(KEYCODE_Q) && (*color & 0x80)) *color = machine().rand();
	if (machine().input().code_pressed(KEYCODE_W) && (*color & 0x40)) *color = machine().rand();
	if (machine().input().code_pressed(KEYCODE_E) && (*color & 0x20)) *color = machine().rand();
	if (machine().input().code_pressed(KEYCODE_R) && (*color & 0x10)) *color = machine().rand();
#endif
	*priority = (*color & 0x10) ? 0 : 0x02;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(rollerg_state::zoom_callback)
{
	*code |= ((*color & 0x0f) << 8);
	*color = ((*color & 0x30) >> 4);
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t rollerg_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_colorbase = 16;

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 1);
	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}


void rollerg_state::ext_enable_w(uint8_t data)
{
	LOGUNKWRITE("%04x: write %02x to 0010\n", m_maincpu->pc(), data);

	// bits 0/1 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 2 enables 051316 ROM reading
	BIT(data, 2) ? m_zoomroms_view.select(0) : m_zoomroms_view.disable();

	// bit 5 enables 051316 wraparound
	m_k051316->wraparound_enable(data & 0x20);

	// other bits unknown
}

void rollerg_state::soundirq_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void rollerg_state::sound_arm_nmi_w(uint8_t data)
{
	// see notes in simpsons driver
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_blocked->adjust(m_audiocpu->cycles_to_attotime(4));
}

void rollerg_state::z80_nmi_w(int state)
{
	if (state && !m_nmi_blocked->enabled())
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t rollerg_state::pip_r()
{
	return 0x7f;
}

void rollerg_state::main_map(address_map &map)
{
	map(0x0010, 0x0010).w(FUNC(rollerg_state::ext_enable_w));
	map(0x0020, 0x0020).rw("watchdog", FUNC(watchdog_timer_device::reset_r), FUNC(watchdog_timer_device::reset_w));
	map(0x0030, 0x0031).rw("k053260", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write));
	map(0x0040, 0x0040).w(FUNC(rollerg_state::soundirq_w));
	map(0x0050, 0x0050).portr("P1");
	map(0x0051, 0x0051).portr("P2");
	map(0x0052, 0x0052).portr("DSW3");
	map(0x0053, 0x0053).portr("DSW1");
	map(0x0060, 0x0060).portr("DSW2");
	map(0x0061, 0x0061).r(FUNC(rollerg_state::pip_r));             // ?????
	map(0x0100, 0x010f).rw("k053252", FUNC(k053252_device::read), FUNC(k053252_device::write));      // 053252?
	map(0x0200, 0x020f).w(m_k051316, FUNC(k051316_device::ctrl_w));
	map(0x0300, 0x030f).rw(m_k053244, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w));
	map(0x0800, 0x0fff).rw(m_k051316, FUNC(k051316_device::read), FUNC(k051316_device::write));
	map(0x0800, 0x0fff).view(m_zoomroms_view);
	m_zoomroms_view[0](0x0800, 0x0fff).r(m_k051316, FUNC(k051316_device::rom_r));
	map(0x1000, 0x17ff).rw(m_k053244, FUNC(k05324x_device::k053245_r), FUNC(k05324x_device::k053245_w));
	map(0x1800, 0x1fff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2000, 0x3aff).ram();
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}

void rollerg_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa02f).rw("k053260", FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xfc00, 0xfc00).w(FUNC(rollerg_state::sound_arm_nmi_w));
}

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( rollerg )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Credits" )
	// No Credits = both coin slots open, but no effect on coin counters

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )           // Manual says it's unused
	PORT_DIPNAME( 0x18, 0x10, "Bonus Energy" )          PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, "1/2 for Stage Winner" )
	PORT_DIPSETTING(    0x08, "1/4 for Stage Winner" )
	PORT_DIPSETTING(    0x10, "1/4 for Cycle Winner" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )           // Manual says it's unused
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )           // Manual says it's unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

void rollerg_state::irq_ack_w(int state)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void rollerg_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 6, &ROM[0x00000], 0x4000);
	m_mainbank->configure_entries(6, 2, &ROM[0x00000], 0x4000);
	m_mainbank->set_entry(0);

	m_nmi_blocked = timer_alloc(timer_expired_delegate());
}

void rollerg_state::machine_reset()
{
	// Z80 _NMI goes low at same time as reset
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


void rollerg_state::rollerg(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, 24_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &rollerg_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(rollerg_state::irq0_line_assert));
	m_maincpu->line().set_membank(m_mainbank).mask(0x07);

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &rollerg_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(rollerg_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 1024).enable_shadows();

	K053244(config, m_k053244, 0);
	m_k053244->set_palette("palette");
	m_k053244->set_offsets(-3, -1);
	m_k053244->set_sprite_callback(FUNC(rollerg_state::sprite_callback));

	K051316(config, m_k051316, 0);
	m_k051316->set_palette("palette");
	m_k051316->set_offsets(22, 1);
	m_k051316->set_zoom_callback(FUNC(rollerg_state::zoom_callback));

	k053252_device &k053252(K053252(config, "k053252", 3000000 * 2));
	k053252.int1_ack().set(FUNC(rollerg_state::irq_ack_w));
	k053252.set_offsets(14*8, 2*8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);

	k053260_device &k053260(K053260(config, "k053260", 3.579545_MHz_XTAL));
	k053260.add_route(ALL_OUTPUTS, "mono", 0.70);
	k053260.sh1_cb().set(FUNC(rollerg_state::z80_nmi_w));
}



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( rollerg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "999m02.g7",  0x00000, 0x20000, CRC(3df8db93) SHA1(10c46d53d11b12b8f7cc6417601baef4638c1efe) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "k053244", 0 ) // sprites
	ROM_LOAD32_WORD( "999h06.k2", 0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) )
	ROM_LOAD32_WORD( "999h05.k8", 0x000002, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "k051316", 0 ) // zoom
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) )
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "k053260", 0 ) // samples
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END

ROM_START( rollergj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "999v02.bin", 0x00000, 0x20000, CRC(0dd8c3ac) SHA1(4c3d5514dec317c6640ceaaa06411766632f4412) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "k053244", 0 ) // sprites
	ROM_LOAD32_WORD( "999h06.k2", 0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) )
	ROM_LOAD32_WORD( "999h05.k8", 0x000002, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "k051316", 0 ) // zoom
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) )
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "k053260", 0 ) // samples
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1991, rollerg,  0,       rollerg, rollerg, rollerg_state, empty_init, ROT0, "Konami", "Rollergames (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, rollergj, rollerg, rollerg, rollerg, rollerg_state, empty_init, ROT0, "Konami", "Rollergames (Japan)", MACHINE_SUPPORTS_SAVE )
