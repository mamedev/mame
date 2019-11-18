// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton drivers

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL(8'000'000)

class xxx_state : public driver_device
{
public:
	xxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
	{
	}

	void xxx(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void xxx_palette(palette_device &palette) const;

	void xxx_io(address_map &map);
	void xxx_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

void xxx_state::video_start()
{
}

uint32_t xxx_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void xxx_state::xxx_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void xxx_state::xxx_io(address_map &map)
{
	map.global_mask(0xff);
}

static INPUT_PORTS_START( xxx )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_xxx )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void xxx_state::machine_start()
{
}

void xxx_state::machine_reset()
{
}


void xxx_state::xxx_palette(palette_device &palette) const
{
}

void xxx_state::xxx(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &xxx_state::xxx_map);
	m_maincpu->set_addrmap(AS_IO, &xxx_state::xxx_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(xxx_state::screen_update));
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_raw(MAIN_CLOCK/2, 442, 0, 320, 264, 0, 240);          /* generic NTSC video timing at 320x240 */
//  screen.set_raw(XTAL(12'000'000)/2, 384, 0, 256, 264, 16, 240);  /* generic NTSC video timing at 256x224 */
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xxx);
	PALETTE(config, "palette", FUNC(xxx_state::xxx_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
//  AY8910(config, "aysnd", MAIN_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.30);
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( xxx )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 )
ROM_END

// See src/emu/gamedrv.h for details
// For a game:
// GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,DRIVER_INIT,MONITOR,COMPANY,FULLNAME,FLAGS)

// For a console:
// CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,DRIVER_INIT,COMPANY,FULLNAME,FLAGS)

// For a computer:
// COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,DRIVER_INIT,COMPANY,FULLNAME,FLAGS)

// For a generic system:
// SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,DRIVER_INIT,COMPANY,FULLNAME,FLAGS)

GAME( 198?, xxx,  0,   xxx,  xxx, xxx_state, empty_init, ROT0, "<template_manufacturer>",      "<template_machinename>", MACHINE_IS_SKELETON )
