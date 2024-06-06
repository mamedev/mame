// license:BSD-3-Clause
// copyright-holders:

/*

Skeleton driver for Sony Librie e-Books.

  Librie EBR-1000 (PCB EBX-5003):
    __________________________________________________________________________
   |                    ________                                             |
   |                    M5M5V416C ________                                    \__
   |                    | RAM   | M5M5V416C                                     |
   |                    |_______| | RAM   |                                     |
   |                              |_______|                                     |
   |                   ___________                                              |
   |                   |         |                                              |
   |                   APOLLO 1.18                 SONY                        _|_
   |                   T6TW8XBG-001                B-LIBR01X           POWER->|___|
   |                   |_________|                 EBX-5003                     |
   |                           ______              1-860-998-14                _|__
   |                           |IC1611        ____                       USB->|___|
   |                           |FLASH|        |||||                             |
   |                           |_____|                                         _|___
   | ________    __________   __________                         AUDIO JACK ->|____|
   | | SONY  |   | IC1224  |  | IC1203  |                                       |
   | CXD3452GA   | FLASH   |  | FLASH   |                                       |
   | |_______|   |_________|  |_________|                                       |
   |                                                                            |
   |  ___      ____________       _________                                     |
   |  |=|    V54C3256164VBUT7     | MX-1   |                                    |
   |  |=|      |  RAM      |     MC9328MX1VN20           _______                |
   |  |=|      |___________|      DragonBall             |      |               |
   |  |=|                         |________|             |      |               |
   |  |=|                                                |IC1125|               |
   |  |=|                             ____________       |FLASH |               /
   |  |=|                           V54C3256164VBUT7     |______|              |
   |                 __________       |  RAM      |     _______                |
   |                 |||||||||||      |___________|     |      |   ________    |
   |__________________________________________________  |      |   |       |   |
                                                      | |IC1126|   |H8/3802|   |
                                                      | |FLASH |   |_______|   |
                                                      | |______|               |
                                                      |________________________|

   CXD3452GA is the MS I/F CONTROLLER, according to the PRS-500 Service Manual (also uses this chip).
*/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "cpu/h8/h83002.h"

#include "screen.h"
#include "speaker.h"


namespace {

class librie_state : public driver_device
{
public:
	librie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu")
		{ }

	void ebx5003(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<h83002_device> m_mcu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


uint32_t librie_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static INPUT_PORTS_START( ebx5003 )
INPUT_PORTS_END

void librie_state::ebx5003(machine_config &config)
{
	ARM920T(config, m_maincpu, 66'000'000); // ARM920T core, unknown clock

	screen_device&screen(SCREEN(config, "screen", SCREEN_TYPE_LCD)); // Not LCD, but e-Ink, 800×600 with 167 PPI. It only had 4 grey scales
	screen.set_refresh_hz(60);
	screen.set_size(600, 800);
	screen.set_visarea(0, 600 - 1, 0, 800 - 1);
	screen.set_screen_update(FUNC(librie_state::screen_update));

	// TODO: Apollo 1.18 T6TW8XBG-001 e-Ink screen controller

	SPEAKER(config, "speaker").front_center();

	H83002(config, m_mcu, 66'000'000); // Actually an Hitachi H8/3802 HD6473802FP. Unknown clock
}


ROM_START( librie ) // TODO: factory-reset
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "02nor0.ic1203", 0x000000, 0x200000, CRC(b5b11902) SHA1(9df606009d0e526a5b968c7487c11cd49db2d222) )
	ROM_LOAD( "02nor1.ic1204", 0x200000, 0x200000, CRC(228c2618) SHA1(06c3b82a59fff27a8cc5cbb74e41898817c86fa3) )

	ROM_REGION( 0x3180000, "user", 0 )
	ROM_LOAD( "uca690dc.ic1106", 0x0000000, 0x2100000, CRC(8489e4e5) SHA1(944aae4e3e546569b070ed9f843f218260325ed8) )
	ROM_LOAD( "upb514ff.ic1107", 0x2100000, 0x1080000, CRC(64f62a30) SHA1(0bdb03f439554dd6a66b19225712091e5634b746) )

	ROM_REGION( 0x80000, "extra", 0 )
	ROM_LOAD( "c_29lv400.ic1611", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x4000, "mcu", 0 ) // H8/3802 HD6473802FP
	ROM_LOAD( "v22.ic801", 0x0000, 0x4000, NO_DUMP ) // 16 kbytes ROM
ROM_END

} // anonymous namespace


SYST( 2004, librie, 0, 0, ebx5003, ebx5003, librie_state, empty_init, "Sony", "Librie EBR 1000", MACHINE_IS_SKELETON )
