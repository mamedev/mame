// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Korg M1 (c) 1988

    skeleton driver

    Note: driver isn't yet hooked up to mess.lst / mess.mak, until a ROM
    dump is found.

***************************************************************************/


#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/cxd1095.h"
//#include "sound/ay8910.h"
#include "emupal.h"

#define MAIN_CLOCK XTAL(16'000'000) // Unknown clock

class korgm1_state : public driver_device
{
public:
	korgm1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void korgm1(machine_config &config);

	void korgm1_io(address_map &map);
	void korgm1_map(address_map &map);
protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void korgm1_state::video_start()
{
}

uint32_t korgm1_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void korgm1_state::korgm1_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram(); // 64 KB
//  map(0x50000, 0x57fff).ram(); // memory card 32 KB
	map(0xe0000, 0xfffff).rom().region("ipl", 0);
}

void korgm1_state::korgm1_io(address_map &map)
{
//  map(0x0000, 0x00ff); internal peripheral (?)
//  map(0x0100, 0x01ff); VDF 1 (MB87404)
//  map(0x0200, 0x02ff); VDF 2 (MB87404)
//  map(0x0500, 0x0503); MDE (MB87405)
//  map(0x0600, 0x0601); OPZ 1 (8-bit port)
//  map(0x0700, 0x0701); OPZ 2 (8-bit port)
//  map(0x0800, 0x0801); SCAN (8-bit port) (keyboard)
//  map(0x0900, 0x0900); ADC (M58990P, compatible with ADC0808; Joystick, "value" and After Touch routes here)
//  map(0x0a00, 0x0a07); PIO (CXD1095Q; LCD, LED and SW routes here; also controls ADC channel select and TG/VDF/MDE reset line)
//  map(0x0b00, 0x0b01); LCDC (8-bit port)
//  map(0x1000, 0x11ff); TG (MB87402)
//  map(0x2000, 0x23ff); SCSI
//  map(0x3000, 0x33ff); FDC
//  TG 2?
}

static INPUT_PORTS_START( korgm1 )
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

#if 0
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
#endif

static GFXDECODE_START( gfx_korgm1 )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void korgm1_state::machine_start()
{
}

void korgm1_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(korgm1_state, korgm1)
{
}

void korgm1_state::korgm1(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, MAIN_CLOCK); // V50 actually
	m_maincpu->set_addrmap(AS_PROGRAM, &korgm1_state::korgm1_map);
	m_maincpu->set_addrmap(AS_IO, &korgm1_state::korgm1_io);

	CXD1095(config, "pio");

	/* video hardware */
	/* TODO: LCD actually */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(korgm1_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);

	GFXDECODE(config, "gfxdecode", "palette", gfx_korgm1);

	PALETTE(config, "palette").set_entries(8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
//  AY8910(config, "aysnd", MAIN_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.30);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( korgm1 )
	ROM_REGION( 0x20000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "bios.rom", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x200000, "pcm", ROMREGION_ERASE00 )
	ROM_LOAD( "pcm.rom", 0x00000, 0x200000, NO_DUMP )

//  ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 )
ROM_END

GAME( 1988, korgm1,  0,   korgm1,  korgm1,  empty_init, ROT0, "Korg",      "M1", MACHINE_IS_SKELETON )
