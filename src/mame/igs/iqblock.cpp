// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Ernesto Corvi

/***************************************************************************

IQ Block   (c) 1992 IGS

Driver by Nicola Salmoria and Ernesto Corvi

TODO:
- Who generates IRQ and NMI? How many should there be per frame?

- Coin 2 doesn't work? DIP switch setting?

- Protection:
  I can see it reading things like the R register here and there, so it might
  be cycle-dependant or something.

  'Crash 1' checks I was able to see:
  PC = $52FA
  PC = $507F

  'Crash 2' checks I was able to see:
  PC = $54E6

Stephh's notes :

  - Coin 2 as well as buttons 2 to 4 for each player are only read in "test mode".
    Same issue for Dip Switches 0-7 and 1-2 to 1-6.
    Some other games on the same hardware might use them.
  - Dip Switch 0 is stored at 0xf0ac and Dip Switch 1 is stored at 0xf0ad.
    However they are both read back at the same time with "ld   hl,($F0AC)" instructions.
  - Dip Switches 0-0 and 0-1 are read via code at 0x9470.
    This routine is called when you made a "line" after the routine that checks the score
    for awarding extra help and/or changing background.
    Data is coming from 4 possible tables (depending on them) which seem to be 0x84 bytes wide.
    Table 0 offset is 0xeaf7.
    IMO, this has something to do with difficulty but there is no confirmation about that !
  - Dip Switch 1-0 is read only once after the P.O.S.T. via code at 0xa200.
    It changes (or not) the contents of 0xf0db.w which can get these 2 possible values
    at start : 0x47a3 (when OFF) or 0x428e (when ON) which seem to be tables.
    If you set a WP to 0xf0db, you'll notice that it's called more often in the "demo mode"
    when the Dip Switch is ON, so, as it implies writes to outport 0x50b0, I think it has
    something to do with "Demo Sounds".
    I can't tell however if setting the Dip Switch to OFF means "Demo Sounds" OFF or ON !

Grndtour:
 - Title should flash 3X slowly. In MAME it flashes too fast, or strangely??


***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class iqblock_state : public driver_device
{
public:
	iqblock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram")
	{ }

	void iqblock(machine_config &config);

	void init_grndtour();
	void init_iqblock();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	uint8_t m_videoenable = 0;
	uint8_t m_video_type = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void iqblock_prot_w(uint8_t data);
	void grndtour_prot_w(uint8_t data);
	void irqack_w(uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgscroll_w(offs_t offset, uint8_t data);
	void port_c_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(irq);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void program_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(iqblock_state::get_bg_tile_info)
{
	int const code = m_bgvideoram[tile_index] + (m_bgvideoram[tile_index + 0x800] << 8);
	tileinfo.set(0,
			code &(m_video_type ? 0x1fff : 0x3fff),
			m_video_type? (2 * (code >> 13) + 1) : (4 * (code >> 14) + 3),
			0);
}

TILE_GET_INFO_MEMBER(iqblock_state::get_fg_tile_info)
{
	int const code = m_fgvideoram[tile_index];
	tileinfo.set(1,
			code & 0x7f,
			(code & 0x80) ? 3 : 0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void iqblock_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(iqblock_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(iqblock_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(64);

	save_item(NAME(m_videoenable));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void iqblock_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void iqblock_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void iqblock_state::fgscroll_w(offs_t offset, uint8_t data)
{
	m_fg_tilemap->set_scrolly(offset, data);
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t iqblock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_videoenable) return 0;
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void iqblock_state::iqblock_prot_w(uint8_t data)
{
	m_rambase[0xe26] = data;
	m_rambase[0xe27] = data;
	m_rambase[0xe1c] = data;
}

void iqblock_state::grndtour_prot_w(uint8_t data)
{
	m_rambase[0xe39] = data;
	m_rambase[0xe3a] = data;
	m_rambase[0xe2f] = data;
}


TIMER_DEVICE_CALLBACK_MEMBER(iqblock_state::irq)
{
	int const scanline = param;

	if((scanline % 16) != 0)
		return;

	if((scanline % 32) == 16)
		m_maincpu->set_input_line(0, HOLD_LINE);
	else if ((scanline % 32) == 0)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void iqblock_state::irqack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void iqblock_state::port_c_w(uint8_t data)
{
	// bit 4 unknown; it is pulsed at the end of every NMI

	// bit 5 seems to be 0 during screen redraw
	m_videoenable = data & 0x20;

	// bit 6 is coin counter
	machine().bookkeeping().coin_counter_w(0, data & 0x40);

	// bit 7 could be a second coin counter, but coin 2 doesn't seem to work...
}

void iqblock_state::program_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram().share(m_rambase);
}


void iqblock_state::port_map(address_map &map)
{
	map(0x2000, 0x23ff).w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2bff).w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x5080, 0x5083).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5090, 0x5090).portr("SW0");
	map(0x50a0, 0x50a0).portr("SW1");
	map(0x50b0, 0x50b1).w("ymsnd", FUNC(ym2413_device::write)); // UM3567_data_port_0_w
	map(0x50c0, 0x50c0).w(FUNC(iqblock_state::irqack_w));
	map(0x6000, 0x603f).w(FUNC(iqblock_state::fgscroll_w));
	map(0x6800, 0x69ff).w(FUNC(iqblock_state::fgvideoram_w)).share(m_fgvideoram); // initialized up to 6fff... bug or larger tilemap?
	map(0x7000, 0x7fff).ram().w(FUNC(iqblock_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x8000, 0xffff).rom().region("bgmaps", 0);
}

static INPUT_PORTS_START( iqblock )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )                // "test mode" only

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )                  // "test mode" only
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // "test mode" only

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )                // "test mode" only
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )                // "test mode" only
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL  // "test mode" only
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL  // "test mode" only
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW0")
	PORT_DIPNAME( 0x03, 0x03, "Unknown SW 0-0&1" )  // Difficulty ? Read notes above
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Helps" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Demo Sounds?" )  // To be confirmed ! Read notes above
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END



static INPUT_PORTS_START( grndtour )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x00, "Attract Music" ) // NOT Demo Sounds (SFX are always played?)
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Gal Images" )
	PORT_DIPSETTING(    0x00, "No" )
	PORT_DIPSETTING(    0x80, "Yes" )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Allow P2 to Join / Always Split Screen" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// in Level test mode the following select the start level, do they have any effect during normal gameplay?
	PORT_DIPNAME( 0x02, 0x02, "Level Test +1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Level Test +2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Level Test +4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Level Test +8" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Level Test +16" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Test Mode" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "Background Test" )
	PORT_DIPSETTING(    0x40, "Input Test" )
	PORT_DIPSETTING(    0x00, "Level Test / Debug Mode (use 5 above switches)" )
INPUT_PORTS_END

static const gfx_layout tilelayout1 =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ 8, 0, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0, RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tilelayout2 =
{
	8,32,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{   0, 1, 2, 3, 4, 5, 6, 7 },
	{   0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	32*16
};

#if 0
static const gfx_layout tilelayout3 =
{
	8,32,
	RGN_FRAC(1,3),
	6,
	{ 8, 0, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0, RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{   0, 1, 2, 3, 4, 5, 6, 7 },
	{   0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 },
	32*16
};
#endif

static GFXDECODE_START( gfx_iqblock )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout1, 0, 16 )    // only odd color codes are used
	GFXDECODE_ENTRY( "fgtiles", 0, tilelayout2, 0,  4 )    // only color codes 0 and 3 used
GFXDECODE_END



void iqblock_state::iqblock(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &iqblock_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &iqblock_state::port_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(iqblock_state::irq), "screen", 0, 1);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set_ioport("P1");
	ppi.in_pb_callback().set_ioport("P2");
	ppi.in_pc_callback().set_ioport("EXTRA");
	ppi.out_pc_callback().set(FUNC(iqblock_state::port_c_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(iqblock_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_iqblock);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

/*
IQ-Block
IGS 1993

PCB 0036-5
                              SW1     SW2

  12MHz         W2466             8255

          IGS002   U24.5
                   U25.4   AMT001  6116-45
                                   6116-45
  Z80              U26.3
                   U27.2
  U7.V5            U28.1
  U8.6
  W2466
                              UMC UM3567
*/

ROM_START( iqblock )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u7.v5",        0x0000, 0x10000, CRC(811f306e) SHA1(d0aef80f1624002d05721276358f26a3ef69a3f6) )

	ROM_REGION( 0x8000, "bgmaps", 0 )
	ROM_LOAD( "u8.6",         0x0000, 0x8000, CRC(2651bc27) SHA1(53e1d6ffd78c8a612863b29b0f8734e740d563c7) )    // background maps, read by the CPU

	ROM_REGION( 0x60000, "bgtiles", 0 )
	ROM_LOAD( "u28.1",        0x00000, 0x20000, CRC(ec4b64b4) SHA1(000e9df0c0b5fcde5ead218dfcdc156bc4be909d) )
	ROM_LOAD( "u27.2",        0x20000, 0x20000, CRC(74aa3de3) SHA1(16757c24765d22026793a0c53d3f24c106951a18) )
	ROM_LOAD( "u26.3",        0x40000, 0x20000, CRC(2896331b) SHA1(51eba9f9f653a11cb96c461ab495d943d34cedc6) )

	ROM_REGION( 0x8000, "fgtiles", 0 )
	ROM_LOAD( "u25.4",        0x0000, 0x4000, CRC(8fc222af) SHA1(ac1fb5e6caec391a76e3af51e133aecc65cd5aed) )
	ROM_LOAD( "u24.5",        0x4000, 0x4000, CRC(61050e1e) SHA1(1f7185b2a5a2e237120276c95344744b146b4bf6) )
ROM_END


/*
Grand Tour
IGS, 1993

This game probably runs on the same board as IGS's IQ Block.
Two of the PALs are labelled GRAND3 and GRAND4. However, there may be other
games that run on this same PCB also, since three of the PALs are
labelled AF1, AF2 and AF3, meaning the main/first game to run on this
hardware was called A-something F-something.


PCB Layout
----------

IGS PCB N0. 0036-5
----------------------------------------------
|                                 6264       |
|  UM3567                        GRAND6      |
|    3.579545MHz   GRAND1                    |
|                                GRAND7      |
|                  GRAND2                    |
|J                                           |
|A                 GRAND3                    |
|M                                    Z80    |
|M                 GRAND4                    |
|A                                    PAL    |
|    6116          GRAND5                    |
|    6116  IGS001          IGS002     PAL    |
|                  6264                      |
|                                     PAL    |
|  8255      PAL                             |
|                            PAL      12MHz  |
| DSW2(8) DSW1(8)                            |
----------------------------------------------

Notes:
      Z80 clock: 5.997MHz
          VSync: 60Hz
          HSync: 15.21kHz
          UM3567 compatible with YM2413
*/

ROM_START( grndtour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "grand7.u7",        0x0000, 0x10000, CRC(95cac31e) SHA1(47bbcce6981ea3d38e0aa49ccd3762a4529f3c96) )

	ROM_REGION( 0x8000, "bgmaps", 0 )
	ROM_LOAD( "grand6.u8",         0x0000, 0x8000, CRC(4c634b86) SHA1(c36df147187bc526f2348bc2f4d4c4e35bb45f38) )   // background maps, read by the CPU

	ROM_REGION( 0xc0000, "bgtiles", 0 )
	ROM_LOAD( "grand1.u28",        0x00000, 0x40000, CRC(de85c664) SHA1(3a4b0cac88a0fea1c80541fe49c799e3550bedee) )
	ROM_LOAD( "grand2.u27",        0x40000, 0x40000, CRC(8456204e) SHA1(b604d501f360670f57b937ad96af64c1c2038ef7) )
	ROM_LOAD( "grand3.u26",        0x80000, 0x40000, CRC(77632917) SHA1(d91eadec2e0fb3082299362d18814b8ec4c5e068) )

	ROM_REGION( 0x8000, "fgtiles", 0 )
	ROM_LOAD( "grand4.u25",        0x0000, 0x4000, CRC(48d09746) SHA1(64669f572b9a98b078ee1ea0b614c117e5dfbec9) )
	ROM_LOAD( "grand5.u24",        0x4000, 0x4000, CRC(f896efb2) SHA1(8dc8546e363b4ff80983e3b8e2a19ebb7ff30c7b) )
ROM_END

void iqblock_state::init_iqblock()
{
	uint8_t *rom = memregion("maincpu")->base();
	// decrypt the program ROM
	for (int i = 0; i < 0xf000; i++)
	{
		if ((i & 0x0282) != 0x0282) rom[i] ^= 0x01;
		if ((i & 0x0940) == 0x0940) rom[i] ^= 0x02;
		if ((i & 0x0090) == 0x0010) rom[i] ^= 0x20;
	}

	m_maincpu->space(AS_PROGRAM).install_write_handler(0xfe26, 0xfe26, write8smo_delegate(*this, FUNC(iqblock_state::iqblock_prot_w)));
	m_video_type = 1;
}

void iqblock_state::init_grndtour()
{
	uint8_t *rom = memregion("maincpu")->base();
	// decrypt the program ROM
	for (int i = 0; i < 0xf000; i++)
	{
		if ((i & 0x0282) != 0x0282) rom[i] ^= 0x01;
		if ((i & 0x0940) == 0x0940) rom[i] ^= 0x02;
		if ((i & 0x0060) == 0x0040) rom[i] ^= 0x20;
	}

	m_maincpu->space(AS_PROGRAM).install_write_handler(0xfe39, 0xfe39, write8smo_delegate(*this, FUNC(iqblock_state::grndtour_prot_w)));
	m_video_type = 0;
}

} // anonymous namespace


GAME( 1993, iqblock,  0, iqblock, iqblock,  iqblock_state, init_iqblock,  ROT0, "IGS", "IQ-Block (V100U)",   MACHINE_SUPPORTS_SAVE )
GAME( 1993, grndtour, 0, iqblock, grndtour, iqblock_state, init_grndtour, ROT0, "IGS", "Grand Tour (V100U)", MACHINE_SUPPORTS_SAVE )
