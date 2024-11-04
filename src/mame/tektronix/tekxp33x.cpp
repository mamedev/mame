// license:BSD-3-Clause
// copyright-holders:Justin Kerk
/***************************************************************************

    Tektronix TekXpress XP33x series X terminals

    Skeleton driver.

****************************************************************************/

/*

    TODO:

    - everything

    Technical info:
    https://www.linux-mips.org/wiki/Tektronix_TekXPress_XP338
    https://web.archive.org/web/20061013084616/http://tekxp-linux.hopto.org/pmwiki/pmwiki.php/Hardware/XP33x
    http://bio.gsi.de/DOCS/NCD/xp.html

    CPU: IDT 79R3052E (MIPS I R3000 embedded core. Big Endian. With MMU, without FPU.)
    Graphic Chipset: Texas Instruments (34020AGBL-40) TI 34010 (TIGA)
    Graphic RAMDAC: Brooktree BT458LPJ135 (Supported in XFree86)
    RAM: 4MB onboard, upgradeable to 52MB, 3 slots for 5V FPM NoParity SIMMs
    ROM: 256Kb, 2 x 27c1024 (64Kx16), with BootMonitor
    Peripherals:
        1x Ethernet Port (AUI and UTP) -- AMD AM79C98 Chipset
        2x Serial Ports -- Philips SCC2692 Dual UART chipset
        Keyboard and Mouse controller is a generic PC-Style i8742 with Phoenix BIOS.
        1x "ERGO" Port (Combined Monitor, Keyboard, Mouse)
    16K (2K*8) EEPROM is SEEQ NQ2816A-250, AT28C16 compatible. Contains Boot Monitor settings. The content is compressed.
    Micron MT56C0816 - ??? Flash

*/


#include "emu.h"
#include "cpu/mips/mips1.h"
#include "cpu/tms34010/tms34010.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define SCREEN_TAG "screen"

class tekxp330_state : public driver_device
{
public:
	tekxp330_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void tekxp330(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void cpu_map(address_map &map) ATTR_COLD;
	void tms_map(address_map &map) ATTR_COLD;
};

/* Memory Maps */

void tekxp330_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();
	map(0x1fc00000, 0x1fdfffff).rom().region("maincpu", 0);
}

void tekxp330_state::tms_map(address_map &map)
{
}

/* Input Ports */

static INPUT_PORTS_START( tekxp330 )
INPUT_PORTS_END

/* Video */

void tekxp330_state::video_start()
{
}

uint32_t tekxp330_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Machine Initialization */

void tekxp330_state::machine_start()
{
}

/* Machine Driver */

void tekxp330_state::tekxp330(machine_config &config)
{
	/* basic machine hardware */
	r3052e_device &maincpu(R3052E(config, "maincpu", XTAL(20'000'000))); /* IDT 79R3052E, clock unknown */
	maincpu.set_endianness(ENDIANNESS_BIG);
	maincpu.set_addrmap(AS_PROGRAM, &tekxp330_state::cpu_map);

	tms34010_device &tms(TMS34010(config, "tms", XTAL(40'000'000))); /* clock unknown */
	tms.set_addrmap(AS_PROGRAM, &tekxp330_state::tms_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(tekxp330_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);

	PALETTE(config, "palette").set_entries(64);
}

/* ROMs */

ROM_START( tekxp330 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	/* TekXpress XP300 Boot ROM V1.0  Thu Feb 6 14:50:16 PST 1992 */
	ROM_LOAD32_DWORD( "xp300.bin", 0x000000, 0x200000, CRC(9a324588) SHA1(a6e10275f8215f446be91128bab4c643693da653) )
ROM_END

} // anonymous namespace


/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME           FLAGS
COMP( 1992, tekxp330, 0,      0,      tekxp330, tekxp330, tekxp330_state, empty_init, "Tektronix", "TekXpress XP330", MACHINE_IS_SKELETON )
