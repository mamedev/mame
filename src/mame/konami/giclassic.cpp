// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 GI Classic / GI Classic EX
 (c) 1995, 1998 Konami
 Preliminary driver by R. Belmont

 GI Classic EX Main PCB:
 Main CPU: 68000
 Video: 056832 + 058143 (GX tilemaps)
 Video: 055673(x2) + 053246 (GX sprites)
 Video: 055555 (Mixer)
 053252 (x2) (CRTC?)

 GI Classic EX Satellite PCB:
 Main CPU: 68000-12
 Video: 056832 / 058143 (GX tilemaps)
 Video: 000907 LCD Controller

 WANTED: main PCB and any other PCBs for GI Classic EX, plus any and all
 PCBs for other games also believed to be on this h/w:
 - GI-Classic (1995)
 - GI-Classic Special (1996)
 - GI-Classic WINDS (1996)
 - GI-Classic WINDS EX (1998)

 Other "GI" games, list from http://www.konami.jp/am/g1/
 - GI-LEADING SIRE (1999)
 - GI-LEADING SIRE Ver. 2 (2000)
 - GI-LEADING SIRE Ver. 3 (2001)
 - GI-WINNING SIRE (2002)
 - GI-TURFWILD (2003)
 - GI-WINNING SIRE Ver. 2 (2003)
 - GI-TURFWILD 2 (2004)
 - GI-TURFWILD 3 (2005)
 - GI-HORSEPARK (2005)
 - GI-HORSEPARK EX (2006)
 - GI-HORSEPARK EX STD (2006)
 - GI-HORSEPARK GX STD (2009)
 - GI-HORSEPARK GX (2009)
 - GI-Turf TV (2010)

***************************************************************************/

#include "emu.h"

#include "k055555.h"
#include "k054156_k054157_k056832.h"
#include "k053246_k053247_k055673.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "machine/k053252.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class giclassic_state : public driver_device
{
public:
	giclassic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_palette(*this, "palette")
	{ }

	void giclassic(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<palette_device> m_palette;

	INTERRUPT_GEN_MEMBER(giclassic_interrupt);

	uint32_t screen_update_giclassic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K056832_CB_MEMBER(tile_callback);

	void control_w(uint16_t data);
	uint16_t vrom_r(offs_t offset);

	void satellite_main(address_map &map) ATTR_COLD;

	uint8_t m_control = 0;
};

// --------------------------------------------------------------------------------------------------------------
// Client portion
// --------------------------------------------------------------------------------------------------------------

K056832_CB_MEMBER(giclassic_state::tile_callback)
{
	*color = (*color & 0xf);
}

void giclassic_state::video_start()
{
}

uint32_t giclassic_state::screen_update_giclassic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 4);
//  m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);

	return 0;
}

INTERRUPT_GEN_MEMBER(giclassic_state::giclassic_interrupt)
{
	if (m_control & 2)
	{
		m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE);
		m_maincpu->set_input_line(M68K_IRQ_3, HOLD_LINE);
	}
}

void giclassic_state::control_w(uint16_t data)
{
	// bits:
	// 0 = ?
	// 1 = IRQ enable
	// 2 = ?
	// 3 = extra VROM readback bank
	// 4 = screen on?

	m_control = data & 0xff;
}

uint16_t giclassic_state::vrom_r(offs_t offset)
{
	if (m_control & 8)
	{
		return m_k056832->piratesh_rom_r(offset + 0x1000);
	}

	return m_k056832->piratesh_rom_r(offset);
}

void giclassic_state::satellite_main(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x801fff).ram().rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0x900000, 0x90003f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));
	map(0xb00000, 0xb01fff).r(FUNC(giclassic_state::vrom_r));
	map(0xc00000, 0xc00001).w(FUNC(giclassic_state::control_w));
	map(0xd00000, 0xd0003f).ram(); // these must read/write or 26S (LCD controller) fails
	map(0xe00000, 0xe00007).w(m_k056832, FUNC(k056832_device::b_w)).umask16(0xff00); // ?
	map(0xf00000, 0xf00001).noprw(); // watchdog reset
}

static INPUT_PORTS_START( giclassic )
INPUT_PORTS_END

void giclassic_state::machine_start()
{
}

void giclassic_state::machine_reset()
{
}

// --------------------------------------------------------------------------------------------------------------
// Server portion
// --------------------------------------------------------------------------------------------------------------

class giclassicsvr_state : public driver_device
{
public:
	giclassicsvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k055673(*this, "k055673"),
		m_palette(*this, "palette")
	{ }

	void giclassvr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055673_device> m_k055673;
	required_device<palette_device> m_palette;

	INTERRUPT_GEN_MEMBER(giclassicsvr_interrupt);

	uint32_t screen_update_giclassicsvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K056832_CB_MEMBER(tile_callback);
	K055673_CB_MEMBER(sprite_callback);

	void control_w(uint16_t data);
	uint16_t control_r();

	void server_main(address_map &map) ATTR_COLD;

	uint16_t m_control = 0;
};

void giclassicsvr_state::control_w(uint16_t data)
{
	m_control = data;
}

uint16_t giclassicsvr_state::control_r()
{
	return m_control;
}

INTERRUPT_GEN_MEMBER(giclassicsvr_state::giclassicsvr_interrupt)
{
	//if (m_control & 2)
	{
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
		m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
	}
}

K056832_CB_MEMBER(giclassicsvr_state::tile_callback)
{
}

K055673_CB_MEMBER(giclassicsvr_state::sprite_callback)
{
	int c = *color;

	*color = (c & 0x001f);
	//int pri = (c >> 5) & 7;
	// .... .... ...x xxxx - Color
	// .... .... xxx. .... - Priority?
	// .... ..x. .... .... - ?
	// ..x. .... .... .... - ?

	*priority_mask = 0;

	// 0 - Sprites over everything
	// f0 -
	// f0 cc -
	// f0 cc aa -

	// 1111 0000
	// 1100 1100
	// 1010 1010
}


uint32_t giclassicsvr_state::screen_update_giclassicsvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);

	return 0;
}

void giclassicsvr_state::server_main(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x080000, 0x08ffff).ram();
	map(0x090000, 0x093fff).ram();
	map(0x100000, 0x107fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x183fff).ram();
	map(0x280000, 0x281fff).ram().rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0x300000, 0x300007).w(m_k055673, FUNC(k055673_device::k053246_w)); // SPRITES
	map(0x300060, 0x30006f).r(m_k055673, FUNC(k055673_device::k055673_ps_rom_word_r)); // SPRITES
	map(0x308000, 0x30803f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));
	map(0x320000, 0x32001f).rw("k053252a", FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff); // CRTC 1
	map(0x320000, 0x32001f).rw("k053252b", FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0xff00); // CRTC 2
	map(0x380000, 0x380001).nopw();    // watchdog reset
	map(0x398000, 0x398001).rw(FUNC(giclassicsvr_state::control_r), FUNC(giclassicsvr_state::control_w));
	map(0x400000, 0x41ffff).ram();
}

static INPUT_PORTS_START( giclassvr )
INPUT_PORTS_END

void giclassicsvr_state::machine_start()
{
}

void giclassicsvr_state::machine_reset()
{
}

void giclassic_state::giclassic(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000) / 2); // PCB is marked "68000 12 MHz", but only visible osc is 20 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &giclassic_state::satellite_main);
	m_maincpu->set_vblank_int("screen", FUNC(giclassic_state::giclassic_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(600, 384);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(giclassic_state::screen_update_giclassic));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 2048);
	m_palette->enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(giclassic_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4PIRATESH, 1, 0);
	m_k056832->set_palette(m_palette);
}

void giclassicsvr_state::giclassvr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000)); // unknown speed
	m_maincpu->set_addrmap(AS_PROGRAM, &giclassicsvr_state::server_main);
	m_maincpu->set_vblank_int("screen", FUNC(giclassicsvr_state::giclassicsvr_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(giclassicsvr_state::screen_update_giclassicsvr));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 16384);
	m_palette->enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(giclassicsvr_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4PIRATESH, 0, 0);
	m_k056832->set_palette(m_palette);

	K055673(config, m_k055673, 0);
	m_k055673->set_sprite_callback(FUNC(giclassicsvr_state::sprite_callback));
	m_k055673->set_config(K055673_LAYOUT_PS, -60, 24);
	m_k055673->set_palette(m_palette);

	K053252(config, "k053252a", XTAL(32'000'000)/4).set_offsets(40, 16); // TODO
	K053252(config, "k053252b", XTAL(32'000'000)/4).set_offsets(40, 16); // TODO
}

ROM_START( giclasex )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "gsgu760ae01.12t", 0x000000, 0x080000, CRC(f0f9c118) SHA1(1753d53946bc0703d329e4a09c452713b260da75) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "gsgu760ae03.14c", 0x000000, 0x080000, CRC(1663d327) SHA1(98c1a9653d38f4918f78b3a11af0c29c658201f5) )
	ROM_LOAD( "gsgu760ae02.14e", 0x080000, 0x080000, CRC(2b9fe163) SHA1(f60190a9689a70d6c5bb14fb46b7ac2267cf0969) )
ROM_END

ROM_START( giclassvr )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "gsgu_760_fd01.34e.bin", 0x000000, 0x080000, CRC(da89c1d7) SHA1(551d050a9b6e54fbf98e966eb37924b644037893) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "gsgu_760_ad04.25q", 0x080000, 0x080000, CRC(71a45742) SHA1(fbddd54f5fb236662f7cc7e9b350723bc5404f72) )
	ROM_LOAD( "gsgu_760_ad05.25r", 0x000000, 0x080000, CRC(44221eec) SHA1(966452e606e828b536ed11cbdd626a2fe3165199) )

	ROM_REGION( 0x100000, "k055673", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "gsgu_760_ad02.34j", 0x000000, 0x080000, CRC(6d33c720) SHA1(35da3e1f0133a76480d2078fae89ea87b841ffc7) )
	ROM_LOAD32_WORD( "gsgu_760_ad02.34k", 0x000002, 0x080000, CRC(8057a417) SHA1(82d4a1d84729e9f0a8aff4c219a19601b89caf15) )
ROM_END

} // anonymous namespace


GAME( 1998, giclasex, 0, giclassic, giclassic, giclassic_state,    empty_init, 0, "Konami", "GI-Classic EX (satellite terminal)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW)
GAME( 1998, giclassvr,0, giclassvr, giclassvr, giclassicsvr_state, empty_init, 0, "Konami", "GI-Classic EX (server)",             MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW)
