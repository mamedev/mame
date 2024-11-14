// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

// gl6600cx uses a NSC1028 system-on-a-chip designed by National Semiconductor specifically for VTech
// http://web.archive.org/web/19991127134657/http://www.national.com/news/item/0,1735,425,00.html

/*

Other known undumped international versions:
- Genius Pro 2000 (French version of Genius Tabletop Black Magic CX)

Leader 8008 CX (German version)

+---+-----------+-----+-----------------------+-----+-----+-----+
|   |SERIAL PORT|     |PARALLEL PORT (PRINTER)|     |MOUSE|     |
|   +-----------+     +-----------------------+     +-----+     |
|                                                               |
|                                                               |
|                                                               |
|                                                               |
|   +----+                                                      |
|   | A0 |                                                      |
|   +----+                                                      |
|                                                               |
|                                                               |
|                                        +--------+             |
|                                        |        |             |
|                              CPU       | VTECH  |   +------+  |
|                                        |LHMV5GNS|   |      |  |
|                                        |        |   |GM76U8|  |
|                                        |1999    |   |128CLF|  |
|                                        |27-6393-|   |W85   |  |
|       +-----------+                    |11      |   |      |  |
|       |27-6296-0-0|                    |        |   |      |  |
|       |47C241M NH7|                    |        |   +------+  |
|       +-----------+                    +--------+             |
|                                                               |
|                                                               |
|                                                               |
|                                                               |
+---------------------------------------------------------------+

CPU = epoxy blob
GM76U8128CLFW85 = LGS / Hynix 131,072 WORDS x 8 BIT CMOS SRAM
TMP47C241MG = TLCS-47 series 4-bit CPU with 2048x8 internal ROM

*/

#include "emu.h"
#include "cpu/cr16b/cr16b.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class glcx_state : public driver_device
{
public:
	glcx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void glcx(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cr16b_device> m_maincpu;
};

uint32_t glcx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void glcx_state::mem_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( glcx )
INPUT_PORTS_END

void glcx_state::glcx(machine_config &config)
{
	/* basic machine hardware */
	CR16B(config, m_maincpu, 10000000); // FIXME: determine exact type and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &glcx_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(glcx_state::screen_update));

	SOFTWARE_LIST(config, "cart_list").set_original("glcx");
}

ROM_START( gl6600cx )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "54-06400-00.u1", 0x000000, 0x200000, CRC(b05cd075) SHA1(b1d9eb02ca56350eb9e89518db89c0a2a845ebd8))
ROM_END

ROM_START( gl8008cx )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-6393-11.u1", 0x0000, 0x200000, CRC(fd49db46) SHA1(fc55bb31f42068f9d6cc8e2c2f419c3c4edb4fe6) )

	ROM_REGION(0x800, "subcpu", 0)
	ROM_LOAD( "27-6296-0-0.u3", 0x000, 0x800, NO_DUMP )
ROM_END

ROM_START( bs9009cx )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-6603-01.u1", 0x0000, 0x200000, CRC(2c299f65) SHA1(44b37007a7c4087d7c2bd8c24907402bfe445ba4) )

	ROM_REGION(0x800, "subcpu", 0)
	ROM_LOAD( "mcu.u5", 0x000, 0x800, NO_DUMP )
ROM_END

ROM_START( gtbmcx )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-6455-00.u5", 0x0000, 0x200000, CRC(166f11b7) SHA1(5d57573f3c97cfd75a07c333833f920ebc417867) )

	// Cartridge "EUROPA" contains no ROM
ROM_END

} // anonymous namespace


COMP( 1999, gl6600cx, 0, 0, glcx, glcx, glcx_state, empty_init, "VTech", "Genius Leader 6600 CX (Germany)", MACHINE_IS_SKELETON )
COMP( 1999, gl8008cx, 0, 0, glcx, glcx, glcx_state, empty_init, "VTech", "Genius Leader 8008 CX (Germany)", MACHINE_IS_SKELETON)
COMP( 1999, bs9009cx, 0, 0, glcx, glcx, glcx_state, empty_init, "VTech", "BrainStation 9009 CXL (Germany)", MACHINE_IS_SKELETON)
COMP( 2000, gtbmcx,   0, 0, glcx, glcx, glcx_state, empty_init, "VTech", "Genius Tabletop Black Magic CX (Germany)", MACHINE_IS_SKELETON)
