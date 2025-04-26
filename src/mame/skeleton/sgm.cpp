// license:BSD-3-Clause
// copyright-holders:

/*
Wang Pai Dou Dizhu, S.G.M., 2004?
Hardware Info by Guru
---------------------

SGM-0509L-1
|--------------------------------------|
|  S2          JAMMA            TA7252 |
|                     6295     VOL     |
|1              ULN2803                |
|8                                     |
|W             |----|        FLASH.U21 |
|A             |CPU |   24MHz    6264  |
|Y  FLASH.U15  |    |                  |
|              |----|         708SEN   |
|   FLASH.U16                          |
|                                      |
|    BUTTON    22MHz                   |
|  A3.3       |----------|             |
|1 A1.8       |          |             |
|0    A5.0    |          | 62LV12816   |
|W EPCS1N.U39 | ALTERA   |             |
|A            | CYCLONE  |             |
|Y   BATT     |          | 62LV12816   |
|       JTAG  |----------|             |
|--------------------------------------|
Notes:
          CPU - Unknown QFP144 CPU, looks like Hyperstone or ARM? Clock input 24.000MHz
       ALTERA - Altera Cyclone (unknown model, surface scratched) QFP240 FPGA (Graphics Chip)
       EPCS1N - Altera EPCS1N-9530B 128kB Serial Configuration Device for FPGA
         JTAG - 10 pin JTAG Header
         BATT - CR2032 Coin Battery
       BUTTON - Push button, does nothing when pressed.
         A3.3 - AMS1117-3.3 3.3V Linear Regulator
         A1.8 - AMS1117-1.8 1.8V Linear Regulator
         A5.0 - AMS1117-5.0 5.0V Linear Regulator
           S2 - 8-Position DIP Switch
       708SEN - Sipex 708SEN System Reset IC
      ULN2803 - ULN2803 8-Channel Darlington Transistor Array
       TA7252 - Toshiba TA7252 5.9W Audio Power Amplifier
         6295 - Oki M6295 ADPCM Sample Player. Clock 1.100MHz [22/20]. Pin 7 HIGH.
                Note title screen music is ripped off from cjddz!
    FLASH.U21 - Macronix MX29F1615 1MB x16-bit DIP42 Flash ROM
FLASH.U15/U16 - Sharp LH28F320BJD-TTL80 2MB x16-bit DIP42 Flash ROM
         6264 - 8kB x8-bit SRAM (battery-backed by CR2032 Coin Cell)
    62LV12816 - ISSI IS62LV12816 128kB x16-bit SRAM

TODO:
- identify CPU arch. There is no obvious code in external ROMs. Encrypted or does it have
  internal ROM? With either of the larger ROMs removed from PCB it doesn't boot.
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sgm_state : public driver_device
{
public:
	sgm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void wpddz(machine_config &config) ATTR_COLD;


private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t sgm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void sgm_state::program_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
}


static INPUT_PORTS_START( wpddz )
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

	// no DIP switches on PCB
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
GFXDECODE_END


void sgm_state::wpddz(machine_config &config)
{
	// basic machine hardware
	ARM7(config, m_maincpu, 24_MHz_XTAL); // actual CPU arch unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &sgm_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sgm_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 22_MHz_XTAL / 20, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( wpddz )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_rom", 0x00000, 0x20000, NO_DUMP ) // TODO: verify this theory

	ROM_REGION( 0x20000, "cyclone_config", 0 )
	ROM_LOAD( "epcs1n.u39", 0x000000, 0x20000, CRC(874f9122) SHA1(f9f5f721065cdb91416f611da987f2edece1237c) )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "flash.u15", 0x000000, 0x400000, CRC(60a6bb59) SHA1(3f0e7e650643901ebbafd2ac195ad1a99cc645d9) )
	ROM_LOAD( "flash.u16", 0x400000, 0x400000, CRC(429b4938) SHA1(4497ed85f6cbfb03b420ea68427f6a24d092f6b1) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "flash.u21", 0x000000, 0x200000, CRC(e0813bdb) SHA1(41b487da6bfbfb231a0c7297d5a4955a5d4019ff) )
ROM_END

} // anonymous namespace


GAME( 2004?, wpddz, 0, wpddz, wpddz, sgm_state, empty_init, ROT0, "SGM", "Wang Pai Dou Dizhou", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
