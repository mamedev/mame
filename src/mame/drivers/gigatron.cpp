// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

   Skeleton driver for Gigatron TTL Microcomputer
   Driver by Sterophonick

***************************************************************************/


#include "emu.h"
#include "cpu/gigatron/gigatron.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK 6250000

class gigatron_state : public driver_device
{
public:
	gigatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
	{
	}

	void gigatron(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gigatron_palette(palette_device &palette) const;

	void gigatron_io(address_map &map);
	void gigatron_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

void gigatron_state::video_start()
{
}

uint32_t gigatron_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void gigatron_state::gigatron_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void gigatron_state::gigatron_io(address_map &map)
{
	map.global_mask(0xff);
}

static INPUT_PORTS_START( gigatron )
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

static GFXDECODE_START( gfx_gigatron )
GFXDECODE_END


void gigatron_state::machine_start()
{
}

void gigatron_state::machine_reset()
{
}


void gigatron_state::gigatron_palette(palette_device &palette) const
{
}

void gigatron_state::gigatron(machine_config &config)
{
	/* basic machine hardware */
	GTRON(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gigatron_state::gigatron_map);
	m_maincpu->set_addrmap(AS_IO, &gigatron_state::gigatron_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(gigatron_state::screen_update));
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_raw(MAIN_CLOCK/2, 442, 0, 160, 264, 0, 120);          /* generic NTSC video timing at 320x240 */
//  screen.set_raw(XTAL(12'000'000)/2, 384, 0, 256, 264, 16, 240);  /* generic NTSC video timing at 256x224 */
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_gigatron);
	PALETTE(config, "palette", FUNC(gigatron_state::gigatron_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( gigatron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gigatron.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e) )  // 0x2000 of data repeated along the dump
ROM_END


COMP(2018, gigatron,  0,   gigatron,  gigatron, gigatron_state, empty_init, ROT0, "Marcel van Kervinck",      "Gigatron TTL Microcomputer", MACHINE_IS_SKELETON )
