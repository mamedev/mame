// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

P112 Single Board Computer

2010-08-30 Skeleton driver

The P112 is a stand-alone 8-bit CPU board. Typically running CP/M (tm) or a
similar operating system, it provides a Z80182 (Z-80 upgrade) CPU with up to
1MB of memory, serial, parallel and diskette IO, and realtime clock, in a
3.5-inch drive form factor. Powered solely from 5V, it draws 150mA (nominal:
not including disk drives) with a 16MHz CPU clock. Clock speeds up to 24.576MHz
are possible.

http://members.iinet.net.au/~daveb/p112/p112.html

Some of the parts:
 32kHz crystal          1       Y2              (RTC crystal)
 16MHz crystal          1       Y1              Parallel resonant (CPU clock)
 24MHz crystal          1       Y3              Parallel resonant (IO chip clock)
 28F256A                1       U4              (Intel Flash ROM, programmed)
 74HCT00                1       U5
 74ACT02                1       U8
 74ACT139               1       U11
 62256                  2       U2      U3      (Static RAM, 32kB)
 DS1202                 1       U6              (Dallas RTC chip)
 FDC37C665IR            1       U9              (SMC Super-IO chip)
 LT1133                 2       U7      U10     (Linear Technology RS232 Tx/Rx)
 NMF0512S               1       U12             (Newport Components Flash ROM power)
 TL7705ACP              1       U15             (Texas Insts. Reset genr.)
 Z8018216               1       U1              (Zilog CPU chip)

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "emupal.h"
#include "screen.h"


namespace {

class p112_state : public driver_device
{
public:
	p112_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void p112(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update_p112(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};


void p112_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).rom();
	map(0x08000, 0xfffff).ram();
}

void p112_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

/* Input ports */
static INPUT_PORTS_START( p112 )
INPUT_PORTS_END


void p112_state::machine_reset()
{
}

void p112_state::machine_start()
{
}

uint32_t p112_state::screen_update_p112(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void p112_state::p112(machine_config &config)
{
	/* basic machine hardware */
	Z80182(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &p112_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &p112_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(240, 320);
	screen.set_visarea(0, 240-1, 0, 320-1);
	screen.set_screen_update(FUNC(p112_state::screen_update_p112));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( p112 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "960513", "ver 13-05-1996" )
	ROMX_LOAD( "960513.bin",  0x00000, 0x8000, CRC(077c1c40) SHA1(c1e6b4b0de50bba90f0d4667f9344815bb578b9b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "970308", "ver 08-03-1997" )
	ROMX_LOAD( "970308.bin",  0x00000, 0x8000, CRC(15e61f0d) SHA1(66ba1af7da0450b69650086ab20230390ba23e17), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "4b1", "ver 4b1" )
	ROMX_LOAD( "romv4b1.bin", 0x00000, 0x8000, CRC(15d79beb) SHA1(f971f75a717e3f6d59b257eb3af369d4d2e0f301), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "4b2", "ver 4b2" )
	ROMX_LOAD( "romv4b2.bin", 0x00000, 0x8000, CRC(9b9a8a5e) SHA1(c40151ee654008b9f803d6a05d692a5a3619a726), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "4b3", "ver 4b3" )
	ROMX_LOAD( "romv4b3.bin", 0x00000, 0x8000, CRC(734031ea) SHA1(2e5e5ac6bd17d15cab24a36bc3da42ca49cbde92), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "4b4", "ver 4b4" )
	ROMX_LOAD( "romv4b4.bin", 0x00000, 0x8000, CRC(cd419c40) SHA1(6002130d874387c9ec23b4363fe9f0ca78208878), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "5", "ver 5" )
	ROMX_LOAD( "051103.bin",  0x00000, 0x8000, CRC(6c47ec13) SHA1(24f5bf1524425186fe10e1d29d05f6efbd3366d9), ROM_BIOS(6))
	ROM_SYSTEM_BIOS( 7, "5b1", "ver 5b1" )
	ROMX_LOAD( "romv5b1.bin", 0x00000, 0x8000, CRC(047296f7) SHA1(380f8e4237525636c605b7e37d989ace8437beb4), ROM_BIOS(7))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY        FULLNAME  FLAGS */
COMP( 1996, p112, 0,      0,      p112,    p112,  p112_state, empty_init, "Dave Brooks", "P112",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
