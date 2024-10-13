// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

88 Games (c) 1988 Konami

***************************************************************************/

#include "emu.h"

#include "k051960.h"
#include "k052109.h"
#include "konami_helper.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"
#include "video/k051316.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class _88games_state : public driver_device
{
public:
	_88games_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_upd7759(*this, "upd%d", 1),
		m_bank0000(*this, "bank0000"),
		m_bank1000(*this, "bank1000"),
		m_ram(*this, "ram")
	{ }

	void _88games(machine_config &config);

private:
	/* video-related */
	int          m_k88games_priority = 0;
	int          m_videobank = 0;
	int          m_zoomreadroms = 0;
	int          m_speech_chip = 0;

	/* devices */
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device_array<upd7759_device, 2> m_upd7759;

	/* memory banks */
	required_memory_bank m_bank0000;
	required_memory_bank m_bank1000;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_ram;

	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void k88games_5f84_w(uint8_t data);
	void k88games_sh_irqtrigger_w(uint8_t data);
	void speech_control_w(uint8_t data);
	void speech_msg_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_88games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void banking_callback(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(_88games_state::tile_callback)
{
	static const int layer_colorbase[] = { 1024 / 16, 0 / 16, 256 / 16 };

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(_88games_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	*priority = (*color & 0x20) >> 5;   /* ??? */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(_88games_state::zoom_callback)
{
	enum { zoom_colorbase = 768 / 16 };

	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x38) >> 3) + ((*color & 0x80) >> 4);
}

/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t _88games_state::screen_update_88games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	if (m_k88games_priority)
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);   // tile 0
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0); // tile 2
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0); // tile 1
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);   // tile 2
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 0);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0); // tile 1
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0); // tile 0
	}

	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t _88games_state::bankedram_r(offs_t offset)
{
	if (m_videobank)
		return m_ram[offset];
	else
	{
		if (m_zoomreadroms)
			return m_k051316->rom_r(offset);
		else
			return m_k051316->read(offset);
	}
}

void _88games_state::bankedram_w(offs_t offset, uint8_t data)
{
	if (m_videobank)
		m_ram[offset] = data;
	else
		m_k051316->write(offset, data);
}

void _88games_state::k88games_5f84_w(uint8_t data)
{
	/* bits 0/1 coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 2 enables ROM reading from the 051316 */
	/* also 5fce == 2 read roms, == 3 read ram */
	m_zoomreadroms = data & 0x04;

	if (data & 0xf8)
		popmessage("5f84 = %02x", data);
}

void _88games_state::k88games_sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}


void _88games_state::speech_control_w(uint8_t data)
{
	m_speech_chip = BIT(data, 2);

	m_upd7759[m_speech_chip]->reset_w(BIT(data, 1));
	m_upd7759[m_speech_chip]->start_w(!BIT(data, 0));
}

void _88games_state::speech_msg_w(uint8_t data)
{
	m_upd7759[m_speech_chip]->port_w(data);
}

/* special handlers to combine 052109 & 051960 */
uint8_t _88games_state::k052109_051960_r(offs_t offset)
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

void _88games_state::k052109_051960_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(offset, data);
	else
		m_k051960->k051960_w(offset - 0x3c00, data);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void _88games_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bank0000"); /* banked ROM */
	map(0x1000, 0x1fff).bankr("bank1000"); /* banked ROM + palette RAM */
	map(0x1000, 0x1fff).w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x37ff).ram().share("nvram");
	map(0x3800, 0x3fff).rw(FUNC(_88games_state::bankedram_r), FUNC(_88games_state::bankedram_w)).share("ram");
	map(0x4000, 0x7fff).rw(FUNC(_88games_state::k052109_051960_r), FUNC(_88games_state::k052109_051960_w));
	map(0x5f84, 0x5f84).w(FUNC(_88games_state::k88games_5f84_w));
	map(0x5f88, 0x5f88).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5f8c, 0x5f8c).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x5f90, 0x5f90).w(FUNC(_88games_state::k88games_sh_irqtrigger_w));
	map(0x5f94, 0x5f94).portr("IN0");
	map(0x5f95, 0x5f95).portr("IN1");
	map(0x5f96, 0x5f96).portr("IN2");
	map(0x5f97, 0x5f97).portr("DSW1");
	map(0x5f9b, 0x5f9b).portr("DSW2");
	map(0x5fc0, 0x5fcf).w(m_k051316, FUNC(k051316_device::ctrl_w));
	map(0x8000, 0xffff).rom();
}

void _88games_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(_88games_state::speech_msg_w));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe000, 0xe000).w(FUNC(_88games_state::speech_control_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( 88games )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "World Records" )         PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x20, "Don't Erase" )
	PORT_DIPSETTING(    0x00, "Erase on Reset" )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")   // Listed in the manual as "continue"
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

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
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x04, "Cocktail (A)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, "Upright (D)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void _88games_state::banking_callback(uint8_t data)
{
	logerror("%s: bank select %02x\n", machine().describe_context(), data);

	/* bits 0-2 select ROM bank for 0000-1fff */
	/* bit 3: when 1, palette RAM at 1000-1fff */
	/* bit 4: when 0, 051316 RAM at 3800-3fff; when 1, work RAM at 2000-3fff (NVRAM 3700-37ff) */
	int rombank = data & 0x07;
	m_bank0000->set_entry(rombank);
	m_bank1000->set_entry((data & 0x08) ? 8 : rombank);
	m_videobank = data & 0x10;

	/* bit 5 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 6 is unknown, 1 most of the time */

	/* bit 7 controls layer priority */
	m_k88games_priority = data & 0x80;
}

void _88games_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base() + 0x10000;

	m_bank0000->configure_entries(0, 8, &ROM[0x0000], 0x2000);
	m_bank1000->configure_entries(0, 8, &ROM[0x1000], 0x2000);
	m_bank1000->configure_entry(8, memshare("palette")->ptr());

	save_item(NAME(m_videobank));
	save_item(NAME(m_zoomreadroms));
	save_item(NAME(m_speech_chip));
	save_item(NAME(m_k88games_priority));
}

void _88games_state::machine_reset()
{
	m_videobank = 0;
	m_zoomreadroms = 0;
	m_speech_chip = 0;
	m_k88games_priority = 0;
}

void _88games_state::_88games(machine_config &config)
{
	/* basic machine hardware */
	KONAMI(config, m_maincpu, 12000000); /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &_88games_state::main_map);
	m_maincpu->line().set(FUNC(_88games_state::banking_callback));

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &_88games_state::sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(_88games_state::screen_update_88games));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette("palette");
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(_88games_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(_88games_state::sprite_callback));

	K051316(config, m_k051316, 0);
	m_k051316->set_palette("palette");
	m_k051316->set_zoom_callback(FUNC(_88games_state::zoom_callback));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", 3579545).add_route(0, "mono", 0.75).add_route(1, "mono", 0.75);

	UPD7759(config, m_upd7759[0]).add_route(ALL_OUTPUTS, "mono", 0.30);

	UPD7759(config, m_upd7759[1]).add_route(ALL_OUTPUTS, "mono", 0.30);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( 88games )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "861m01.k18", 0x08000, 0x08000, CRC(4a4e2959) SHA1(95572686bef48b5c1ce1dedf0afc891d92aff00d) )
	ROM_LOAD( "861m02.k16", 0x10000, 0x10000, CRC(e19f15f6) SHA1(6c801b274e87eaff7f40148381ade5b38120cc12) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "861d01.d9", 0x00000, 0x08000, CRC(0ff1dec0) SHA1(749dc98f8740beee1383f85effc9336081315f4b) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "861a08.a", 0x000000, 0x10000, CRC(77a00dd6) SHA1(e3667839f8ae3699236da3e312c20d571db38670) )
	ROM_LOAD32_BYTE( "861a08.c", 0x000001, 0x10000, CRC(b422edfc) SHA1(b3842c8dc60975cc71812df098f29b4571b18120) )
	ROM_LOAD32_BYTE( "861a09.a", 0x000002, 0x10000, CRC(df8917b6) SHA1(3614b78c2100f135ea0701409ce279a423decb23) )
	ROM_LOAD32_BYTE( "861a09.c", 0x000003, 0x10000, CRC(f577b88f) SHA1(7d5d88e1492ed361dc7b2135595393b89b9cb5b1) )
	ROM_LOAD32_BYTE( "861a08.b", 0x040000, 0x10000, CRC(28a8304f) SHA1(6b4037eff6d209fec29d05f1071ed3bf9c2bd098) )
	ROM_LOAD32_BYTE( "861a08.d", 0x040001, 0x10000, CRC(e01a3802) SHA1(3fb5fe512c2497160a66e9de0cd45c38dfe46410) )
	ROM_LOAD32_BYTE( "861a09.b", 0x040002, 0x10000, CRC(4917158d) SHA1(b53da3f29c9aeb59933dc3a8214cc1314e21000b) )
	ROM_LOAD32_BYTE( "861a09.d", 0x040003, 0x10000, CRC(2bb3282c) SHA1(6ca54948a02c91543b7e595641b0edc2564f83ff) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_BYTE( "861a05.a", 0x000000, 0x10000, CRC(cedc19d0) SHA1(6eb2a292d574dee06e214e61c0e08fa233ac68e8) )
	ROM_LOAD32_BYTE( "861a05.e", 0x000001, 0x10000, CRC(725af3fc) SHA1(98ac364db4b2c5682a299f4d2a288ebc8a303b1f) )
	ROM_LOAD32_BYTE( "861a06.a", 0x000002, 0x10000, CRC(85e2e30e) SHA1(11010727db8c71650c5b9df5340f9bc412435d11) )
	ROM_LOAD32_BYTE( "861a06.e", 0x000003, 0x10000, CRC(6f96651c) SHA1(c740a814a3e203348b269a70256e01fe2a914118) )
	ROM_LOAD32_BYTE( "861a05.b", 0x040000, 0x10000, CRC(db2a8808) SHA1(dad6b127761889aac198014139cc524a4cea32e7) )
	ROM_LOAD32_BYTE( "861a05.f", 0x040001, 0x10000, CRC(32d830ca) SHA1(a3f10720151f538cf1bec5953a4212bc96ba42fe) )
	ROM_LOAD32_BYTE( "861a06.b", 0x040002, 0x10000, CRC(ce17eaf0) SHA1(cc121c5742428e2613b7da2d8357f15e897161ca) )
	ROM_LOAD32_BYTE( "861a06.f", 0x040003, 0x10000, CRC(88310bf3) SHA1(77bac66489e7fc2ddd714fc684e79d70b089ee84) )
	ROM_LOAD32_BYTE( "861a05.c", 0x080000, 0x10000, CRC(cf03c449) SHA1(234714212dd7288a5128d36c96cca5b62e86d37d) )
	ROM_LOAD32_BYTE( "861a05.g", 0x080001, 0x10000, CRC(fd51c4ea) SHA1(fc8923819fa7f3d02b4d159aea45cb5d1a80f1b0) )
	ROM_LOAD32_BYTE( "861a06.c", 0x080002, 0x10000, CRC(a568b34e) SHA1(8b69a0ac90f32cea31f8c7fcd985ad58fb6c009e) )
	ROM_LOAD32_BYTE( "861a06.g", 0x080003, 0x10000, CRC(4a55beb3) SHA1(35088bf7f6acd2bc95f673a2816b35238d611308) )
	ROM_LOAD32_BYTE( "861a05.d", 0x0c0000, 0x10000, CRC(97d78c77) SHA1(2c123fd08cb9626cf309e7320fe2eb99e4b483fb) )
	ROM_LOAD32_BYTE( "861a05.h", 0x0c0001, 0x10000, CRC(60d0c8a5) SHA1(c7d3531eb65abd51ae4e6f55244d674353d23d36) )
	ROM_LOAD32_BYTE( "861a06.d", 0x0c0002, 0x10000, CRC(bc70ab39) SHA1(a6fa0502ceb6862e7b1e4815326e268fd6511881) )
	ROM_LOAD32_BYTE( "861a06.h", 0x0c0003, 0x10000, CRC(d906b79b) SHA1(905814ce708d80fd4d1a398f60faa0bc680fccaf) )

	ROM_REGION( 0x040000, "k051316", 0 )    /* zoom/rotate */
	ROM_LOAD( "861a04.a", 0x000000, 0x10000, CRC(092a8b15) SHA1(d98a81bfa4bba73805f0236f8a80da130fcb378d) )
	ROM_LOAD( "861a04.b", 0x010000, 0x10000, CRC(75744b56) SHA1(5133d8f6622796ed6b9e6a0d0f1df28f00331fc7) )
	ROM_LOAD( "861a04.c", 0x020000, 0x10000, CRC(a00021c5) SHA1(f73f88af33387d73b4262e8652507e699926fabe) )
	ROM_LOAD( "861a04.d", 0x030000, 0x10000, CRC(d208304c) SHA1(77dd31163c8431416ab0593f084719c914222912) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "861.g3",   0x0000, 0x0100, CRC(429785db) SHA1(d27e8e180f19d2b160f18c79520a77182a62218c) )    /* priority encoder (not used) */

	ROM_REGION( 0x20000, "upd1", 0 ) /* samples for UPD7759 #0 */
	ROM_LOAD( "861a07.a", 0x000000, 0x10000, CRC(5d035d69) SHA1(9df63e004a4f52768331dfb3c3889301ac174ea1) )
	ROM_LOAD( "861a07.b", 0x010000, 0x10000, CRC(6337dd91) SHA1(74ba58f1664abd1491598c1a9467f470304fa430) )

	ROM_REGION( 0x20000, "upd2", 0 ) /* samples for UPD7759 #1 */
	ROM_LOAD( "861a07.c", 0x000000, 0x10000, CRC(5067a38b) SHA1(b5a8f7122356dd72a97e71b480835ba500116aaf) )
	ROM_LOAD( "861a07.d", 0x010000, 0x10000, CRC(86731451) SHA1(c1410f6c7a23aa0c213878a6531d3e7eb966b0a4) )
ROM_END

ROM_START( konami88 )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "861.e03", 0x08000, 0x08000, CRC(55979bd9) SHA1(d683cc514e2b41fc4033d5dc107ca22ba8981ada) )
	ROM_LOAD( "861.e02", 0x10000, 0x10000, CRC(5b7e98a6) SHA1(39b6e93221d14a4695c79fb39c4eea54ec5ffb0c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "861d01.d9", 0x00000, 0x08000, CRC(0ff1dec0) SHA1(749dc98f8740beee1383f85effc9336081315f4b) )

	ROM_REGION(  0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "861a08.a", 0x000000, 0x10000, CRC(77a00dd6) SHA1(e3667839f8ae3699236da3e312c20d571db38670) )
	ROM_LOAD32_BYTE( "861a08.c", 0x000001, 0x10000, CRC(b422edfc) SHA1(b3842c8dc60975cc71812df098f29b4571b18120) )
	ROM_LOAD32_BYTE( "861a09.a", 0x000002, 0x10000, CRC(df8917b6) SHA1(3614b78c2100f135ea0701409ce279a423decb23) )
	ROM_LOAD32_BYTE( "861a09.c", 0x000003, 0x10000, CRC(f577b88f) SHA1(7d5d88e1492ed361dc7b2135595393b89b9cb5b1) )
	ROM_LOAD32_BYTE( "861a08.b", 0x040000, 0x10000, CRC(28a8304f) SHA1(6b4037eff6d209fec29d05f1071ed3bf9c2bd098) )
	ROM_LOAD32_BYTE( "861a08.d", 0x040001, 0x10000, CRC(e01a3802) SHA1(3fb5fe512c2497160a66e9de0cd45c38dfe46410) )
	ROM_LOAD32_BYTE( "861a09.b", 0x040002, 0x10000, CRC(4917158d) SHA1(b53da3f29c9aeb59933dc3a8214cc1314e21000b) )
	ROM_LOAD32_BYTE( "861a09.d", 0x040003, 0x10000, CRC(2bb3282c) SHA1(6ca54948a02c91543b7e595641b0edc2564f83ff) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_BYTE( "861a05.a", 0x000000, 0x10000, CRC(cedc19d0) SHA1(6eb2a292d574dee06e214e61c0e08fa233ac68e8) )
	ROM_LOAD32_BYTE( "861a05.e", 0x000001, 0x10000, CRC(725af3fc) SHA1(98ac364db4b2c5682a299f4d2a288ebc8a303b1f) )
	ROM_LOAD32_BYTE( "861a06.a", 0x000002, 0x10000, CRC(85e2e30e) SHA1(11010727db8c71650c5b9df5340f9bc412435d11) )
	ROM_LOAD32_BYTE( "861a06.e", 0x000003, 0x10000, CRC(6f96651c) SHA1(c740a814a3e203348b269a70256e01fe2a914118) )
	ROM_LOAD32_BYTE( "861a05.b", 0x040000, 0x10000, CRC(db2a8808) SHA1(dad6b127761889aac198014139cc524a4cea32e7) )
	ROM_LOAD32_BYTE( "861a05.f", 0x040001, 0x10000, CRC(32d830ca) SHA1(a3f10720151f538cf1bec5953a4212bc96ba42fe) )
	ROM_LOAD32_BYTE( "861a06.b", 0x040002, 0x10000, CRC(ce17eaf0) SHA1(cc121c5742428e2613b7da2d8357f15e897161ca) )
	ROM_LOAD32_BYTE( "861a06.f", 0x040003, 0x10000, CRC(88310bf3) SHA1(77bac66489e7fc2ddd714fc684e79d70b089ee84) )
	ROM_LOAD32_BYTE( "861a05.c", 0x080000, 0x10000, CRC(cf03c449) SHA1(234714212dd7288a5128d36c96cca5b62e86d37d) )
	ROM_LOAD32_BYTE( "861a05.g", 0x080001, 0x10000, CRC(fd51c4ea) SHA1(fc8923819fa7f3d02b4d159aea45cb5d1a80f1b0) )
	ROM_LOAD32_BYTE( "861a06.c", 0x080002, 0x10000, CRC(a568b34e) SHA1(8b69a0ac90f32cea31f8c7fcd985ad58fb6c009e) )
	ROM_LOAD32_BYTE( "861a06.g", 0x080003, 0x10000, CRC(4a55beb3) SHA1(35088bf7f6acd2bc95f673a2816b35238d611308) )
	ROM_LOAD32_BYTE( "861a05.d", 0x0c0000, 0x10000, CRC(97d78c77) SHA1(2c123fd08cb9626cf309e7320fe2eb99e4b483fb) )
	ROM_LOAD32_BYTE( "861a05.h", 0x0c0001, 0x10000, CRC(60d0c8a5) SHA1(c7d3531eb65abd51ae4e6f55244d674353d23d36) )
	ROM_LOAD32_BYTE( "861a06.d", 0x0c0002, 0x10000, CRC(bc70ab39) SHA1(a6fa0502ceb6862e7b1e4815326e268fd6511881) )
	ROM_LOAD32_BYTE( "861a06.h", 0x0c0003, 0x10000, CRC(d906b79b) SHA1(905814ce708d80fd4d1a398f60faa0bc680fccaf) )

	ROM_REGION( 0x040000, "k051316", 0 )    /* zoom/rotate */
	ROM_LOAD( "861a04.a", 0x000000, 0x10000, CRC(092a8b15) SHA1(d98a81bfa4bba73805f0236f8a80da130fcb378d) )
	ROM_LOAD( "861a04.b", 0x010000, 0x10000, CRC(75744b56) SHA1(5133d8f6622796ed6b9e6a0d0f1df28f00331fc7) )
	ROM_LOAD( "861a04.c", 0x020000, 0x10000, CRC(a00021c5) SHA1(f73f88af33387d73b4262e8652507e699926fabe) )
	ROM_LOAD( "861a04.d", 0x030000, 0x10000, CRC(d208304c) SHA1(77dd31163c8431416ab0593f084719c914222912) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "861.g3",   0x0000, 0x0100, CRC(429785db) SHA1(d27e8e180f19d2b160f18c79520a77182a62218c) )    /* priority encoder (not used) */

	ROM_REGION( 0x20000, "upd1", 0 ) /* samples for UPD7759 #0 */
	ROM_LOAD( "861a07.a", 0x000000, 0x10000, CRC(5d035d69) SHA1(9df63e004a4f52768331dfb3c3889301ac174ea1) )
	ROM_LOAD( "861a07.b", 0x010000, 0x10000, CRC(6337dd91) SHA1(74ba58f1664abd1491598c1a9467f470304fa430) )

	ROM_REGION( 0x20000, "upd2", 0 ) /* samples for UPD7759 #1 */
	ROM_LOAD( "861a07.c", 0x000000, 0x10000, CRC(5067a38b) SHA1(b5a8f7122356dd72a97e71b480835ba500116aaf) )
	ROM_LOAD( "861a07.d", 0x010000, 0x10000, CRC(86731451) SHA1(c1410f6c7a23aa0c213878a6531d3e7eb966b0a4) )
ROM_END

ROM_START( hypsptsp )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "861f03.k18", 0x08000, 0x08000, CRC(8c61aebd) SHA1(de720acfe07fd70fe467f9c73122e0fbeab2b8c8) )
	ROM_LOAD( "861f02.k16", 0x10000, 0x10000, CRC(d2460c28) SHA1(936220aa3983ffa2330843f683347768772561af) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "861d01.d9", 0x00000, 0x08000, CRC(0ff1dec0) SHA1(749dc98f8740beee1383f85effc9336081315f4b) )

	ROM_REGION(  0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "861a08.a", 0x000000, 0x10000, CRC(77a00dd6) SHA1(e3667839f8ae3699236da3e312c20d571db38670) )
	ROM_LOAD32_BYTE( "861a08.c", 0x000001, 0x10000, CRC(b422edfc) SHA1(b3842c8dc60975cc71812df098f29b4571b18120) )
	ROM_LOAD32_BYTE( "861a09.a", 0x000002, 0x10000, CRC(df8917b6) SHA1(3614b78c2100f135ea0701409ce279a423decb23) )
	ROM_LOAD32_BYTE( "861a09.c", 0x000003, 0x10000, CRC(f577b88f) SHA1(7d5d88e1492ed361dc7b2135595393b89b9cb5b1) )
	ROM_LOAD32_BYTE( "861a08.b", 0x040000, 0x10000, CRC(28a8304f) SHA1(6b4037eff6d209fec29d05f1071ed3bf9c2bd098) )
	ROM_LOAD32_BYTE( "861a08.d", 0x040001, 0x10000, CRC(e01a3802) SHA1(3fb5fe512c2497160a66e9de0cd45c38dfe46410) )
	ROM_LOAD32_BYTE( "861a09.b", 0x040002, 0x10000, CRC(4917158d) SHA1(b53da3f29c9aeb59933dc3a8214cc1314e21000b) )
	ROM_LOAD32_BYTE( "861a09.d", 0x040003, 0x10000, CRC(2bb3282c) SHA1(6ca54948a02c91543b7e595641b0edc2564f83ff) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_BYTE( "861a05.a", 0x000000, 0x10000, CRC(cedc19d0) SHA1(6eb2a292d574dee06e214e61c0e08fa233ac68e8) )
	ROM_LOAD32_BYTE( "861a05.e", 0x000001, 0x10000, CRC(725af3fc) SHA1(98ac364db4b2c5682a299f4d2a288ebc8a303b1f) )
	ROM_LOAD32_BYTE( "861a06.a", 0x000002, 0x10000, CRC(85e2e30e) SHA1(11010727db8c71650c5b9df5340f9bc412435d11) )
	ROM_LOAD32_BYTE( "861a06.e", 0x000003, 0x10000, CRC(6f96651c) SHA1(c740a814a3e203348b269a70256e01fe2a914118) )
	ROM_LOAD32_BYTE( "861a05.b", 0x040000, 0x10000, CRC(db2a8808) SHA1(dad6b127761889aac198014139cc524a4cea32e7) )
	ROM_LOAD32_BYTE( "861a05.f", 0x040001, 0x10000, CRC(32d830ca) SHA1(a3f10720151f538cf1bec5953a4212bc96ba42fe) )
	ROM_LOAD32_BYTE( "861a06.b", 0x040002, 0x10000, CRC(ce17eaf0) SHA1(cc121c5742428e2613b7da2d8357f15e897161ca) )
	ROM_LOAD32_BYTE( "861a06.f", 0x040003, 0x10000, CRC(88310bf3) SHA1(77bac66489e7fc2ddd714fc684e79d70b089ee84) )
	ROM_LOAD32_BYTE( "861a05.c", 0x080000, 0x10000, CRC(cf03c449) SHA1(234714212dd7288a5128d36c96cca5b62e86d37d) )
	ROM_LOAD32_BYTE( "861a05.g", 0x080001, 0x10000, CRC(fd51c4ea) SHA1(fc8923819fa7f3d02b4d159aea45cb5d1a80f1b0) )
	ROM_LOAD32_BYTE( "861a06.c", 0x080002, 0x10000, CRC(a568b34e) SHA1(8b69a0ac90f32cea31f8c7fcd985ad58fb6c009e) )
	ROM_LOAD32_BYTE( "861a06.g", 0x080003, 0x10000, CRC(4a55beb3) SHA1(35088bf7f6acd2bc95f673a2816b35238d611308) )
	ROM_LOAD32_BYTE( "861a05.d", 0x0c0000, 0x10000, CRC(97d78c77) SHA1(2c123fd08cb9626cf309e7320fe2eb99e4b483fb) )
	ROM_LOAD32_BYTE( "861a05.h", 0x0c0001, 0x10000, CRC(60d0c8a5) SHA1(c7d3531eb65abd51ae4e6f55244d674353d23d36) )
	ROM_LOAD32_BYTE( "861a06.d", 0x0c0002, 0x10000, CRC(bc70ab39) SHA1(a6fa0502ceb6862e7b1e4815326e268fd6511881) )
	ROM_LOAD32_BYTE( "861a06.h", 0x0c0003, 0x10000, CRC(d906b79b) SHA1(905814ce708d80fd4d1a398f60faa0bc680fccaf) )

	ROM_REGION( 0x040000, "k051316", 0 )    /* zoom/rotate */
	ROM_LOAD( "861a04.a", 0x000000, 0x10000, CRC(092a8b15) SHA1(d98a81bfa4bba73805f0236f8a80da130fcb378d) )
	ROM_LOAD( "861a04.b", 0x010000, 0x10000, CRC(75744b56) SHA1(5133d8f6622796ed6b9e6a0d0f1df28f00331fc7) )
	ROM_LOAD( "861a04.c", 0x020000, 0x10000, CRC(a00021c5) SHA1(f73f88af33387d73b4262e8652507e699926fabe) )
	ROM_LOAD( "861a04.d", 0x030000, 0x10000, CRC(d208304c) SHA1(77dd31163c8431416ab0593f084719c914222912) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "861.g3",   0x0000, 0x0100, CRC(429785db) SHA1(d27e8e180f19d2b160f18c79520a77182a62218c) )    /* priority encoder (not used) */

	ROM_REGION( 0x20000, "upd1", 0 ) /* samples for UPD7759 #0 */
	ROM_LOAD( "861a07.a", 0x000000, 0x10000, CRC(5d035d69) SHA1(9df63e004a4f52768331dfb3c3889301ac174ea1) )
	ROM_LOAD( "861a07.b", 0x010000, 0x10000, CRC(6337dd91) SHA1(74ba58f1664abd1491598c1a9467f470304fa430) )

	ROM_REGION( 0x20000, "upd2", 0 ) /* samples for UPD7759 #1 */
	ROM_LOAD( "861a07.c", 0x000000, 0x10000, CRC(5067a38b) SHA1(b5a8f7122356dd72a97e71b480835ba500116aaf) )
	ROM_LOAD( "861a07.d", 0x010000, 0x10000, CRC(86731451) SHA1(c1410f6c7a23aa0c213878a6531d3e7eb966b0a4) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, 88games,  0,       _88games, 88games, _88games_state, empty_init, ROT0, "Konami", "'88 Games", MACHINE_SUPPORTS_SAVE )
GAME( 1988, konami88, 88games, _88games, 88games, _88games_state, empty_init, ROT0, "Konami", "Konami '88", MACHINE_SUPPORTS_SAVE )
GAME( 1988, hypsptsp, 88games, _88games, 88games, _88games_state, empty_init, ROT0, "Konami", "Hyper Sports Special (Japan)", MACHINE_SUPPORTS_SAVE )
