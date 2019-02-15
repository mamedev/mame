// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

// Unknown CPU type

/*

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
TMP47C241MG = TCLS-47 series 4-bit CPU with 2048x8 internal ROM

*/

#include "emu.h"
#include "screen.h"

class gl8008cx_state : public driver_device
{
public:
	gl8008cx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void gl8008cx(machine_config &config);

private:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t gl8008cx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( gl8008cx )
INPUT_PORTS_END

void gl8008cx_state::gl8008cx(machine_config &config)
{
	/* basic machine hardware */
	// UNKNOWN(config, "maincpu", unknown); // CPU type is unknown, epoxy blob

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(gl8008cx_state::screen_update));
}

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


COMP( 1999, gl8008cx, 0, 0, gl8008cx, gl8008cx, gl8008cx_state, empty_init, "Video Technology", "Genius Leader 8008 CX (Germany)", MACHINE_IS_SKELETON)
COMP( 1999, bs9009cx, 0, 0, gl8008cx, gl8008cx, gl8008cx_state, empty_init, "Video Technology", "BrainStation 9009 CXL (Germany)", MACHINE_IS_SKELETON)
