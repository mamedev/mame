// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi

/***************************************************************************

Truco Clemente (c) 1991 Miky SRL

driver by Ernesto Corvi

Notes:
- Sloppy coin insertion, needs to stay high for 60 Hz wtf?
- Audio is almost there.
- I think this runs on a heavily modified PacMan type of board.

----------------------------------
Additional Notes (Roberto Fresca):
----------------------------------
Mainboard: Pacman bootleg jamma board.
Daughterboard: Custom made, plugged in the 2 roms and Z80 mainboard sockets.

  - 01 x Z80
  - 03 x 27c010
  - 02 x am27s19
  - 03 x GAL 16v8b      (All of them have the same contents... Maybe read protected.)
  - 01 x PAL CE 20v8h   (The fuse map is suspect too)
  - 01 x lm324n

  To not overload the driver, I put the rest of technical info in
  http://robbie.mameworld.info/trucocl.htm

- Added 2 "hidden" color proms (am27s19)
- One GAL is connected to the color proms inputs.
- The name of the company is "Miky SRL" instead of "Caloi Miky SRL".
  Caloi (Carlos Loiseau), is the Clemente's creator.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class trucocl_state : public driver_device
{
public:
	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu_rom(*this, "maincpu"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode") { }

	void trucocl(machine_config &config);

	void init_trucocl();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	required_region_ptr<uint8_t> m_maincpu_rom;

	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_bg_tilemap = nullptr;
	int32_t m_cur_dac_address = 0;
	uint16_t m_cur_dac_address_index = 0;
	uint8_t m_irq_mask = 0;
	emu_timer *m_dac_irq_timer = nullptr;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	void irq_enable_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void audio_dac_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);

	TIMER_CALLBACK_MEMBER(dac_irq);
};


void trucocl_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 32; i++)
		palette.set_pen_color(i, pal4bit(color_prom[i] >> 0), pal4bit(color_prom[i + 32] >> 0), pal4bit(color_prom[i + 32] >> 4));
}

void trucocl_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void trucocl_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(trucocl_state::get_bg_tile_info)
{
	int const gfxsel = m_colorram[tile_index] & 1;
	int const bank = ((m_colorram[tile_index] >> 2) & 0x07);
	int code = m_videoram[tile_index];
	int const colour = (m_colorram[tile_index] & 2) >> 1;

	code |= (bank & 1) << 10;
	code |= (bank & 2) << 8;
	code += (bank & 4) << 6;

	tileinfo.set(gfxsel, code, colour, 0);
}

void trucocl_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(trucocl_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t trucocl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// TODO: doesn't seem suited to neither irq nor nmi
void trucocl_state::irq_enable_w(uint8_t data)
{
	m_irq_mask = (data & 1) ^ 1;
}


TIMER_CALLBACK_MEMBER(trucocl_state::dac_irq)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void trucocl_state::audio_dac_w(uint8_t data)
{
	int dac_address = (data & 0xf0) << 8;
	int const sel = (((~data) >> 1) & 2) | (data & 1);

	if (m_cur_dac_address != dac_address)
	{
		m_cur_dac_address_index = 0;
		m_cur_dac_address = dac_address;
	}
	else
	{
		m_cur_dac_address_index++;
	}

	if (sel & 1)
		dac_address += 0x10000;

	if (sel & 2)
		dac_address += 0x10000;

	dac_address += 0x10000;

	m_dac->write(m_maincpu_rom[dac_address + m_cur_dac_address_index]);

	m_dac_irq_timer->adjust(attotime::from_hz(16'000));
}

void trucocl_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(trucocl_state::videoram_w)).share(m_videoram);
	map(0x4400, 0x47ff).ram().w(FUNC(trucocl_state::colorram_w)).share(m_colorram);
	map(0x4800, 0x4fff).ram();
	map(0x5000, 0x5000).w(FUNC(trucocl_state::irq_enable_w));
	map(0x5000, 0x503f).portr("IN0");
	map(0x5080, 0x5080).portr("DSW").w(FUNC(trucocl_state::audio_dac_w));
	map(0x50c0, 0x50c0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom();
}

void trucocl_state::main_io(address_map &map)
{
	map(0x0000, 0xffff).nopr(); // read then always discarded?
}

static INPUT_PORTS_START( trucocl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) //PORT_IMPULSE(60)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable BGM fanfare" ) // enables extra BGMs on attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	// TODO: more are tested ingame
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,        // 8*8 characters
	0x10000/32, // 2048 characters
	4,          // 4 bits per pixel
	{ 0, 1,2,3 },
	{ 0, 4, 0x8000*8+0,0x8000*8+4, 8*8+0, 8*8+4, 0x8000*8+8*8+0,0x8000*8+8*8+4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8        // every char takes 16 consecutive bytes
};



static GFXDECODE_START( gfx_trucocl )
	GFXDECODE_ENTRY( "tiles", 0,       tilelayout,      0, 2 )
	GFXDECODE_ENTRY( "tiles", 0x10000, tilelayout,      0, 2 )
GFXDECODE_END

void trucocl_state::machine_start()
{
	m_dac_irq_timer = timer_alloc(FUNC(trucocl_state::dac_irq), this);

	save_item(NAME(m_cur_dac_address));
	save_item(NAME(m_cur_dac_address_index));
	save_item(NAME(m_irq_mask));
}

void trucocl_state::machine_reset()
{
	m_cur_dac_address = -1;
	m_cur_dac_address_index = 0;
	m_dac_irq_timer->adjust(attotime::never);
}

INTERRUPT_GEN_MEMBER(trucocl_state::interrupt)
{
//  if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

void trucocl_state::trucocl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18'432'000 / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &trucocl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &trucocl_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(trucocl_state::interrupt));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(trucocl_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_trucocl);
	PALETTE(config, "palette", FUNC(trucocl_state::palette), 32);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( trucocl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "trucocl.01", 0x00000, 0x20000, CRC(c9511c37) SHA1(d6a0fa573c8d2faf1a94a2be26fcaafe631d0699) )
	ROM_LOAD( "trucocl.03", 0x20000, 0x20000, CRC(b37ce38c) SHA1(00bd506e9a03cb8ed65b0b599514db6b9b0ee5f3) ) // samples

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "trucocl.02", 0x0000, 0x20000, CRC(bda803e5) SHA1(e4fee42f23be4e0dc8926b6294e4b3e4a38ff185) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "27s19.u2",    0x0000, 0x0020, CRC(75aeff6a) SHA1(fecd117ec9bb8ac2834d422eb507ec78410aff0f) )
	ROM_LOAD( "27s19.u1",    0x0020, 0x0020, CRC(f952f823) SHA1(adc6a05827b1bc47d84827808c324d93ee0f32b9) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME      PARENT  MACHINE  INPUT    STATE          INIT          MONITOR
GAME( 1991, trucocl,  0,      trucocl, trucocl, trucocl_state, empty_init,   ROT0, "Miky SRL", "Truco Clemente", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
