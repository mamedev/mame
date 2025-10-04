// license:BSD-3-Clause
// copyright-holders:

/*
Amcoe HA1 PCB

PCB MODEL HB 1.2-HAX4 (for Burning Rubber)

main components:
RMI AU1250-400MGD SoC
2x Zentel A3R12E4JFF-G8E DDR2-800MHz 32Mx16 DRAM
Lattice XP2
bank of 8 switches

other components aren't readable
*/


#include "emu.h"

#include "cpu/mips/mips3.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class amcoe_ha1_state : public driver_device
{
public:
	amcoe_ha1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void amcoe_ha1(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t amcoe_ha1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void amcoe_ha1_state::video_start()
{
}


void amcoe_ha1_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( burningr )
INPUT_PORTS_END


static GFXDECODE_START( gfx_amcoe_ha1 )
	// TODO
GFXDECODE_END


void amcoe_ha1_state::amcoe_ha1(machine_config &config)
{
	R4600BE(config, m_maincpu, 24_MHz_XTAL); // wrong, RMI AU1250 (no CPU core available)
	m_maincpu->set_addrmap(AS_PROGRAM, &amcoe_ha1_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(amcoe_ha1_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_amcoe_ha1);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();
	// TODO: identify
}


ROM_START( burningr )
	ROM_REGION( 0x10000000, "maincpu", 0 )
	ROM_LOAD( "k9f2g08u0a.bin", 0x00000000, 0x10000000, BAD_DUMP CRC(cbc1f099) SHA1(318ccf2d7fb1f1cb0762945a603979475ccd3bd9) )

	ROM_REGION( 0x100000, "unsorted", 0 )
	ROM_LOAD16_BYTE( "mx29lv040.u21", 0x00000, 0x80000, BAD_DUMP CRC(e23e54b2) SHA1(4672f32784a0c8e07151fecfe3f97f73b5a916f0) ) // very suspect dump, odd bytes are always 0
	ROM_LOAD16_BYTE( "mx29lv040.u22", 0x00001, 0x80000, BAD_DUMP CRC(8dfbee69) SHA1(4a899e81aafe68d76086c1d1269df046a44b6a33) ) // very suspect dump, odd bytes are always 0
ROM_END

} // anonymous namespace


GAME( 2010, burningr, 0, amcoe_ha1, burningr, amcoe_ha1_state, empty_init, ROT0, "Amcoe", "Burning Rubber (Amcoe)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
