// license:BSD-3-Clause
// copyright-holders: Manuel Abadia

/***************************************************************************

Aliens (c) 1990 Konami Co. Ltd
GX875 PCB

Preliminary driver by:
    Manuel Abadia <emumanu+mame@gmail.com>

***************************************************************************/

#include "emu.h"

#include "k051960.h"
#include "k052109.h"
#include "konamipt.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class aliens_state : public driver_device
{
public:
	aliens_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_soundlatch(*this, "soundlatch"),
		m_paletteram_view(*this, "paletteram_view"),
		m_rombank(*this, "rombank")
	{ }

	void aliens(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<generic_latch_8_device> m_soundlatch;

	memory_view m_paletteram_view;
	required_memory_bank m_rombank;

	void coin_counter_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	void snd_bankswitch_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(aliens_state::tile_callback)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer * 4 + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(aliens_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	/* The PROM allows for mixed priorities, where sprites would have
	   priority over text but not on one or both of the other two planes. */
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0x00; break;                                    // over ABF
		case 0x00: *priority = GFX_PMASK_4; break;                             // over AB, not F
		case 0x40: *priority = GFX_PMASK_4 | GFX_PMASK_2; break;               // over A, not BF
		case 0x20:
		case 0x60: *priority = GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1; break; // over -, not ABF
		case 0x50: *priority = GFX_PMASK_2; break;                             // over AF, not B
		case 0x30:
		case 0x70: *priority = GFX_PMASK_2 | GFX_PMASK_1; break;               // over F, not AB
	}
	*code |= (*color & 0x80) << 6;
	*color = sprite_colorbase + (*color & 0x0f);
	*shadow = 0;    // shadows are not used by this game
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t aliens_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	// The background color is always from layer 1
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}


void aliens_state::coin_counter_w(uint8_t data)
{
	// bits 0-1 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 5 = select work RAM or palette
	m_paletteram_view.select((data & 0x20) >> 5);

	// bit 6 = enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	// other bits unknown
}

void aliens_state::snd_bankswitch_w(uint8_t data)
{
	// b1: bank for channel A
	// b0: bank for channel B

	int const bank_a = BIT(data, 1);
	int const bank_b = BIT(data, 0);

	m_k007232->set_bank(bank_a, bank_b);
}


uint8_t aliens_state::k052109_051960_r(offs_t offset)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(offset);
		else
			return m_k051960->k051960_r(offset - 0x3c00);
	}
	else
		return m_k052109->read(offset);
}

void aliens_state::k052109_051960_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(offset, data);
	else
		m_k051960->k051960_w(offset - 0x3c00, data);
}

void aliens_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).view(m_paletteram_view);
	m_paletteram_view[0](0x0000, 0x03ff).ram();
	m_paletteram_view[1](0x0000, 0x03ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x0400, 0x1fff).ram();
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0x7fff).rw(FUNC(aliens_state::k052109_051960_r), FUNC(aliens_state::k052109_051960_w));
	map(0x5f80, 0x5f80).portr("DSW3");
	map(0x5f81, 0x5f81).portr("P1");
	map(0x5f82, 0x5f82).portr("P2");
	map(0x5f83, 0x5f83).portr("DSW2");
	map(0x5f84, 0x5f84).portr("DSW1");
	map(0x5f88, 0x5f88).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w(FUNC(aliens_state::coin_counter_w));
	map(0x5f8c, 0x5f8c).w("soundlatch", FUNC(generic_latch_8_device::write)); // cause interrupt on audio CPU
	map(0x8000, 0xffff).rom().region("maincpu", 0x28000);  // ROM e24_j02.bin
}

void aliens_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                                     // ROM g04_b03.bin
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc000, 0xc000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( aliens )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Credits", SW1)
	// "No Credits" = both coin slots open, but no effect on coin counters

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )    // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )    // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )    // Listed as "Unused"
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )        // Listed as "Unused"
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        // Listed as "Unused"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_B12_COIN_START(1)

	PORT_START("P2")
	KONAMI8_B12_COIN_START(2)
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

void aliens_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data & 0x0f) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data >> 4) * 0x11);
}

void aliens_state::machine_start()
{
	m_rombank->configure_entries(0, 24, memregion("maincpu")->base(), 0x2000);
	m_rombank->set_entry(0);
}

void aliens_state::aliens(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000) / 2); // 052001 (verified on PCB)
	m_maincpu->set_addrmap(AS_PROGRAM, &aliens_state::main_map);
	m_maincpu->line().set_membank(m_rombank).mask(0x1f);

	Z80(config, m_audiocpu, XTAL(3'579'545)); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &aliens_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000) / 3, 528, 112, 400, 256, 16, 240); // measured 59.17
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  screen.set_raw(XTAL(24'000'000) / 4, 396, hbend, hbstart, 256, 16, 240);
	screen.set_screen_update(FUNC(aliens_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 512).enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette("palette");
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(aliens_state::tile_callback));

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(aliens_state::sprite_callback));
	m_k051960->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545)));  // verified on PCB
	ymsnd.port_write_handler().set(FUNC(aliens_state::snd_bankswitch_w));
	ymsnd.add_route(0, "mono", 0.60);
	ymsnd.add_route(1, "mono", 0.60);

	K007232(config, m_k007232, XTAL(3'579'545));    // verified on PCB
	m_k007232->port_write().set(FUNC(aliens_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.20);
	m_k007232->add_route(1, "mono", 0.20);
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( aliens )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_j01.c24", 0x00000, 0x20000, CRC(6a529cd6) SHA1(bff6dee33141d8ed2b2c28813cf49f52dceac364) )
	ROM_LOAD( "875_j02.e24", 0x20000, 0x10000, CRC(56c20971) SHA1(af272e146705e97342466a208c64d823ebc83d83) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14",  0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) ) // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliens2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_n01.c24", 0x00000, 0x20000, CRC(106cf59c) SHA1(78622adc02055d31cd587c83b23a6cde30c9bc22) )
	ROM_LOAD( "875_p02.e24", 0x20000, 0x10000, CRC(4edd707d) SHA1(02b39068e5fd99ecb5b35a586335b65a20fde490) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty


	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14",  0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) ) // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliens3 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_c01.c24",  0x00000, 0x20000, CRC(3c0006fb) SHA1(e8730d50c358e7321dd676c74368fe44b9bbe5b2) )
	ROM_LOAD( "875_w3_2.e24", 0x20000, 0x10000, CRC(f917f7b5) SHA1(ab95ad40c82f11572bbaa03d76dae35f76bacf0c) ) // Needs correct ROM label

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliens4 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_c01.c24", 0x00000, 0x20000, CRC(3c0006fb) SHA1(e8730d50c358e7321dd676c74368fe44b9bbe5b2) )
	ROM_LOAD( "875_d02.e24", 0x20000, 0x10000, CRC(1dc46780) SHA1(85dbd7c0fe5bfbf22274d9b4238588a2c48c535e) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensu )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_n01.c24", 0x00000, 0x20000, CRC(106cf59c) SHA1(78622adc02055d31cd587c83b23a6cde30c9bc22) )
	ROM_LOAD( "875_n02.e24", 0x20000, 0x10000, CRC(24dd612e) SHA1(35bceb3045cd0bd9d107312b371fb60dcf3f1272) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_b03.g04", 0x00000, 0x08000, CRC(1ac4d283) SHA1(2253f1f39c7edb6cef438b3d97f3af67a1f491ff) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensj )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_m01.c24",  0x00000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )
	ROM_LOAD( "875_m02.e24",  0x20000, 0x10000, CRC(54a774e5) SHA1(b6413b2199f863cae1c6fcef766989162cd4b95e) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensj2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_m01.c24",  0x00000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )
	ROM_LOAD( "875_j2_2.e24", 0x20000, 0x10000, CRC(4bb84952) SHA1(ca40a7181f11d6c34c26b65f8d4a1d1df2c7fb48) ) // Needs correct ROM label

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

ROM_START( aliensa )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "875_m01.c24", 0x00000, 0x20000, CRC(1663d3dc) SHA1(706bdf3daa3bda372d94263f3405d67a7ef8dc69) )
	ROM_LOAD( "875_r02.e24", 0x20000, 0x10000, CRC(973e4f11) SHA1(a4f65ef4c84b1dcac591dc348ebbb96d35ef5f93) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "875_k03.g04", 0x00000, 0x08000, CRC(bd86264d) SHA1(345fd666daf8a29ef314b14306c1a976cb159bed) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "875b11.k13", 0x000000, 0x80000, CRC(89c5c885) SHA1(02a1581579b6ef816e04bec312a7b3ae7c7e84f8) )
	ROM_LOAD32_WORD( "875b12.k19", 0x000002, 0x80000, CRC(ea6bdc17) SHA1(a7c22370f8adc5b479283f1ff831f493df78282f) )
	ROM_LOAD32_WORD( "875b07.j13", 0x100000, 0x40000, CRC(e9c56d66) SHA1(1f58949d5391aef002a6e1ee7034e57bf99cee61) )
	// second half empty
	ROM_LOAD32_WORD( "875b08.j19", 0x100002, 0x40000, CRC(f9387966) SHA1(470ecc4a5a3edd08d5e0ab10b0c590db1968fb0a) )
	// second half empty

	ROM_REGION( 0x200000, "k051960", 0 )    // sprites
	ROM_LOAD32_WORD( "875b10.k08", 0x000000, 0x80000, CRC(0b1035b1) SHA1(db04020761386e79249762cd1540208375c38c7f) )
	ROM_LOAD32_WORD( "875b09.k02", 0x000002, 0x80000, CRC(e76b3c19) SHA1(6838e07460b3eaaeb129208ad0696c8019bd63d9) )
	ROM_LOAD32_WORD( "875b06.j08", 0x100000, 0x40000, CRC(081a0566) SHA1(3a4aa14178fe76a030224743c9e9cd974e08bd79) )
	// second half empty
	ROM_LOAD32_WORD( "875b05.j02", 0x100002, 0x40000, CRC(19a261f2) SHA1(b0518fad833b3e613e0201d5d9cab73dc5e78e1d) )
	// second half empty

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.h14", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )  // priority encoder (not used)

	ROM_REGION( 0x40000, "k007232", 0 ) // samples
	ROM_LOAD( "875b04.e05",  0x00000, 0x40000, CRC(4e209ac8) SHA1(09d9eaae61bfd04bf318555ccd44d7371571d86d) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1990, aliens,   0,      aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (World set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliens2,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (World set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliens3,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (World set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliens4,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (World set 4)", MACHINE_SUPPORTS_SAVE ) // revision D
GAME( 1990, aliensu,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (US)",          MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliensj,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliensj2, aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, aliensa,  aliens, aliens, aliens, aliens_state, empty_init, ROT0, "Konami", "Aliens (Asia)",        MACHINE_SUPPORTS_SAVE )
