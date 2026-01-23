// license:BSD-3-Clause
// copyright-holders:

/*
Subsino H8-based kiddie ride hardware

CS186P190 0608 PCB with CS186P409 0648 riser PCB

Main components of the main PCB:
SUBSINO SS9689 6433044A22F CPU
32.000 MHz XTAL
MX27C2000PC-70 program ROM (bad, reads all 0xff)
4x W24257AK-15 SRAM
2x Subsino SG001 S0550 JPAX3 customs (GFX?)
4x 27C801 or equivalent GFX ROMs
4x 27C401 or equivalent GFX ROMs
Subsino SS9802-6 0616 custom (I/O)
Subsino SS9804 0420 custom (sound)
MX29F1610MC-12 sound ROM (bad, won't read)
MX29F1610MC-10 sound ROM (bad, won't read)
S-1 (some kind of audio DAC, according to other driver)

bank of 8 switches (DS1)
push-button (DS1)

The riser board has a Subsino SG003 custom (RAMDAC, according to other driver)
and a good number of capacitors and resistors.
*/


#include "emu.h"

#include "subsino_io.h"

#include "cpu/h8/h83048.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class subsino_kr_h8_state : public driver_device
{
public:
	subsino_kr_h8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void modcart(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void subsino_kr_h8_state::video_start()
{
}

uint32_t subsino_kr_h8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	return 0;
}


void subsino_kr_h8_state::program_map(address_map &map)
{
	map(0x000000, 0x007fff).rom();
}


static INPUT_PORTS_START( modcart )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// TODO: verify
static GFXDECODE_START( gfx_modcart )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x8_raw, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 0, 16 )
GFXDECODE_END


void subsino_kr_h8_state::modcart(machine_config &config)
{
	H83044(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_kr_h8_state::program_map);

	SS9802(config, "io");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify once it works
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-16-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(subsino_kr_h8_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_modcart);
	PALETTE(config, "palette").set_entries(256);

	RAMDAC(config, "ramdac", 0, "palette");

	SPEAKER(config, "mono").front_center();

	// SS9804
}


ROM_START( modcart )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "ss9689_6433044a22f.u31",   0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) ) // wasn't dumped for this set, but part number matches
	ROM_LOAD( "modern_cart_std_v112.u35", 0x080000, 0x100000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "modern_cart_v100.u26", 0x000000, 0x80000, CRC(f2910fbf) SHA1(dd06fc160a09333a15f2d753ac0c4ef267a422ee) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u27", 0x000001, 0x80000, CRC(2aacf7e3) SHA1(6edf3d7e047aa02e6438dd33c5e44b94b29510dd) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u28", 0x000002, 0x80000, CRC(1d24fb35) SHA1(b063e9f2c84d6904bc7633d75e7dfcc881923fac) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u29", 0x000003, 0x80000, CRC(6fa9f574) SHA1(68afac743b72be30b63eda53cf917c8f2568381a) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "u22", 0x000000, 0x100000, CRC(af7cd652) SHA1(f0532fdb75296e6c03b52d0e35331a3b930ca490) )
	ROM_LOAD32_BYTE( "u23", 0x000001, 0x100000, CRC(c69f7f90) SHA1(a594bf299fd94cf3be61b9c7af163d9d9eb15da3) )
	ROM_LOAD32_BYTE( "u24", 0x000002, 0x100000, CRC(174934ec) SHA1(ca55199e5071ffc9282cea1c7fb6cb4b244ddaf9) BAD_DUMP ) // FIXED BITS (xxxxxx1x)
	ROM_LOAD32_BYTE( "u25", 0x000003, 0x100000, CRC(64560184) SHA1(85f4a5b328f19d3630159c82f8a8af0826622afc) BAD_DUMP ) // FIXED BITS (xxx1xxxx)

	ROM_REGION( 0x400000, "ss9804", ROMREGION_ERASE00 )
	ROM_LOAD( "u18", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "u19", 0x200000, 0x200000, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2002, modcart, 0, modcart, modcart, subsino_kr_h8_state, empty_init, ROT0, "Subsino", "Modern Cart", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
