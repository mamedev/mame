// license:BSD-3-Clause
// copyright-holders:

/*
PCB 07-03-20
Title screen shows IGS copyright but PCB isn't marked as such
The 2 unknown chips have a Xin Yun sticker. Underneath they are scratched.

144-pin chip (possibly CPU with internal ROM?)
24 MHz XTAL (near 144-pin chip)
2x IS61LV12816L-10T RAM
HM6264ALFP-12T RAM
240-pin chip (GFX related?)
20 MHz XTAL (near 240-pin chip)
U6295
bank of 8 switches
battery
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class xinyun_state : public driver_device
{
public:
	xinyun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void xinyun(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t xinyun_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void xinyun_state::video_start()
{
}


void xinyun_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( hongmayi )
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

	PORT_START("IN1")
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

	PORT_START("IN2")
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

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_xinyun )
GFXDECODE_END


void xinyun_state::xinyun(machine_config &config)
{
	ARM7(config, m_maincpu, 24_MHz_XTAL); // TODO: wrong. Unidentified CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &xinyun_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(xinyun_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_xinyun);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: pin 7 and clock not verified
}


// 红蚂蚁 (Hóng Mǎyǐ)
ROM_START( hongmxy )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x800000, "gfx", 0 ) // maybe?
	ROM_LOAD( "v217cn.u16", 0x000000, 0x400000, CRC(7bfeeaf1) SHA1(24b9fb7c5dd6d93cd8a84105a640f9333ae6202b) )
	ROM_LOAD( "rom.u15",    0x400000, 0x400000, CRC(be2a9054) SHA1(f0da3d6b79156e0a8a94eb21218783eca04fb389) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "rom.u14", 0x000000, 0x200000, CRC(6e885c0d) SHA1(0e7d62a5ebc80bbdc76c0c1d2942b30c29bb8b59) )
ROM_END

} // anonymous namespace


GAME( 200?, hongmxy, 0, xinyun, hongmayi, xinyun_state, empty_init, ROT0, "Xin Yun", "Hong Mayi (Xin Yun)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
