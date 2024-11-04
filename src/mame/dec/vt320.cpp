// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT320

        30/06/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ram.h"
#include "emupal.h"
#include "screen.h"


namespace {

class vt320_state : public driver_device
{
public:
	vt320_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
	{
	}

	void vt320(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_vt320(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	void vt320_io(address_map &map) ATTR_COLD;
	void vt320_mem(address_map &map) ATTR_COLD;
};

/*

Partlist :

Siemens SAB8031A-16-P
ROMless version of the 8051 microcontroller, running at 16 MHz.
Motorola MC2681P
Dual Universal Asynchronous Receiver/Transmitter (DUART), 40-pin package.
Toshiba TC53512AP
ROM, 512K bits = 64K bytes. 28-pin package.
Toshiba TC5565APL-12, 2 off
Static RAM, 64K bit = 8K byte.
ST TDA1170N
Vertical deflection system IC.
UC 80343Q
20 pins. Unknown.
UC 80068Q
20 pins. Unknown.
Motorola SN74LS157NQST
16 pins. Quad 2-to-1 multiplexer.
Microchip ER5911
8 pins. Serial EEPROM. 1K bits = 128 bytes.
Texas Inst. 749X 75146
8 pins. Unknown.
Signetics? 74LS373N
8-bit D-type latch. This has eight inputs and eight outputs.
*/
void vt320_state::vt320_mem(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void vt320_state::vt320_io(address_map &map)
{
}

/* Input ports */
static INPUT_PORTS_START( vt320 )
INPUT_PORTS_END

void vt320_state::machine_reset()
{
	memset(m_ram->pointer(),0,16*1024);
}

void vt320_state::video_start()
{
}

uint32_t vt320_state::screen_update_vt320(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void vt320_state::vt320(machine_config &config)
{
	/* basic machine hardware */
	I8051(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vt320_state::vt320_mem);
	m_maincpu->set_addrmap(AS_IO, &vt320_state::vt320_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(vt320_state::screen_update_vt320));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K");
}

/* ROM definition */
ROM_START( vt320 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt320" )
	//DOL: http://web.archive.org/web/20060905115711/http://cmcnabb.cc.vt.edu/dec94mds/vt320dol.txt
	ROM_SYSTEM_BIOS( 0, "vt320v11", "VT320 V1.1" )
	// 23-054E7 below can also appear (same contents?) as 23-048E7 which is a mask rom
	ROMX_LOAD( "23-054e7.e9", 0x0000, 0x10000, CRC(be98f9a4) SHA1(b8044d42ffaadb734fbd047fbca9c8aadeb0bf6c), ROM_BIOS(0) ) // EPROM
	ROM_SYSTEM_BIOS( 1, "vt320", "VT320 V1.2" )
	ROMX_LOAD( "23-104e7.e9", 0x0000, 0x10000, CRC(5f419b5a) SHA1(dbc429b32d6baefd8a56862717d6e7fea1fb0c1c), ROM_BIOS(1) )
ROM_END

ROM_START( vt330 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt330" )
	ROM_SYSTEM_BIOS( 0, "vt330", "VT330" )
	ROMX_LOAD( "23-236e6", 0x0000, 0x8000, CRC(38379339) SHA1(394e8511581abc796c8c612149eff280146b0ac8), ROM_BIOS(0) ) // 27256 EPROM
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR   NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY                          FULLNAME   FLAGS */
COMP( 1987,  vt320,  0,      0,      vt320,   vt320, vt320_state,  empty_init, "Digital Equipment Corporation", "VT320",   MACHINE_IS_SKELETON )
COMP( 1987,  vt330,  0,      0,      vt320,   vt320, vt320_state,  empty_init, "Digital Equipment Corporation", "VT330",   MACHINE_IS_SKELETON )
//COMP( 1989?, vt340,  0,      0,      vt320,   vt320, vt320_state,  empty_init, "Digital Equipment Corporation", "VT340",   MACHINE_IS_SKELETON )
//COMP( 1990?, vt340p, 0,      0,      vt320,   vt320, vt320_state,  empty_init, "Digital Equipment Corporation", "VT340+",  MACHINE_IS_SKELETON )
