// license:BSD-3-Clause
// copyright-holders:

/*
SYVG (Shyh-Yuan VideoGames / Shyh-Yuan Electronics - spelt Shih Yuan on some stickers)

Z80 or Z180 based MCU (164 pin, correct arch / model currently unknown);
CPLD (84 pin, model currently unknown);
UT6264CPCL-70LL SRAM;
UT61256JC-12 SRAM;
2x SB61H1024AS-12 SRAM;
U6295
21.0000 MHz XTAL

PCB has no evident marking. One sticker: 15.75KHz (CGA)

*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class syvg_state : public driver_device
{
public:
	syvg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void syvg(machine_config &config);

	void init_luckyvl();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t syvg_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void syvg_state::video_start()
{
}


void syvg_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}


static INPUT_PORTS_START( luckyvl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no DSWs on PCB
INPUT_PORTS_END


static GFXDECODE_START( gfx_syvg )
	// TODO
GFXDECODE_END


void syvg_state::syvg(machine_config &config)
{
	Z80(config, m_maincpu, 21_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &syvg_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(syvg_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_syvg);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 21_MHz_XTAL / 20, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: divider and pin 7 not verified
}


ROM_START( luckyvl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "lucky-v-line_202.u19", 0x00000, 0x20000, CRC(fe5aa9a6) SHA1(a64a5c2a74109595c5e37a6f12b7556a3a167d04) ) // encrypted

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD16_BYTE( "lucky-v-line.u5", 0x00000, 0x80000, CRC(06916190) SHA1(af172259058eb39df8e559e848236e312fe70af2) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "lucky-v-line.u6", 0x00001, 0x80000, CRC(48cf0795) SHA1(baabf1cb0b658bddf8f5dfbf7752dc7f41dcbc93) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "lucky-v-line_sound.u17", 0x00000, 0x40000, CRC(dd548bf9) SHA1(7aa0a772b98dbfee27a4f437f5723075b44d3cb7) )
ROM_END


void syvg_state::init_luckyvl()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		// TODO
		rom[i] = rom[i];
	}
}

} // anonymous namespace


GAME( 2003, luckyvl, 0, syvg, luckyvl, syvg_state, init_luckyvl, ROT0, "Shyh-Yuan Electronics", "Lucky V Line", MACHINE_IS_SKELETON )
