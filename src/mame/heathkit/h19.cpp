// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Heathkit H19

    A smart terminal designed and manufactured by Heath Company.

    The keyboard consists of a 9x10 matrix connected to a MM5740AAC/N
    mask-programmed keyboard controller. The output of this passes
    through a rom.

    Input can also come from the serial port (a 8250).
    Either device will signal an interrupt to the CPU when a key
    is pressed/data is received.

    TODO:
    - speed up emulation
    - update SW401 baud rate options for Watz ROM
    - update SW401 & SW402 definitions for Super-19 ROM
    - update SW401 & SW402 definitions for ULTRA ROM
    - add option for ULTRA ROMs second page of screen RAM

****************************************************************************/

#include "tlb.h"

#define H19_CLOCK (XTAL(12'288'000) / 6)

class h19_state : public driver_device
{
public:
	h19_state(const machine_config &mconfig, device_type type, const char *tag)
			: driver_device(mconfig, type, tag)
			, m_tlb(*this, "tlb")
	{
	}

	void h19(machine_config &config);

private:

	required_device<heath_terminal_logic_board_device> m_tlb;

};

void h19_state::h19(machine_config &config)
{
	TLB(config, m_tlb, H19_CLOCK);

}

/* ROM definition */
ROM_START( h19 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	// Original
	ROM_LOAD( "2732_444-46_h19code.bin", 0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	ROM_REGION( 0x1000, "keyboard", 0 )
	// Original dump
	ROM_LOAD( "2716_444-37_h19keyb.bin", 0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( super19 )
	// Super H19 ROM
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_super19_h447.bin", 0x0000, 0x1000, CRC(6c51aaa6) SHA1(5e368b39fe2f1af44a905dc474663198ab630117))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	ROM_REGION( 0x1000, "keyboard", 0 )
	// Original dump
	ROM_LOAD( "2716_444-37_h19keyb.bin", 0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( watz19 )
	// Watzman ROM
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "watzman.bin", 0x0000, 0x1000, CRC(8168b6dc) SHA1(bfaebb9d766edbe545d24bc2b6630be4f3aa0ce9))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	// Watzman keyboard
	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "keybd.bin", 0x0000, 0x0800, CRC(58dc8217) SHA1(1b23705290bdf9fc6342065c6a528c04bff67b13))
ROM_END

ROM_START( ultra19 )
	// ULTRA ROM
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2532_h19_ultra_firmware.bin", 0x0000, 0x1000, CRC(8ad4cdb4) SHA1(d6e1fc37a1f52abfce5e9adb1819e0030bed1df3))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	// Watzman keyboard
	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "2716_h19_ultra_keyboard.bin", 0x0000, 0x0800, CRC(76130c92) SHA1(ca39c602af48505139d2750a084b5f8f0e662ff7))
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY      FULLNAME                         FLAGS
COMP( 1979, h19,     0,      0,      h19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19",                 MACHINE_SUPPORTS_SAVE )
//Super-19 ROM - ATG Systems, Inc - Adv in Sextant Issue 4, Winter 1983. With the magazine lead-time, likely released late 1982.
COMP( 1982, super19, h19,    0,      h19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Super-19 ROM", MACHINE_SUPPORTS_SAVE )
// Watzman ROM - HUG p/n 885-1121, announced in REMark Issue 33, Oct. 1982
COMP( 1982, watz19,  h19,    0,      h19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Watzman ROM",  MACHINE_SUPPORTS_SAVE )
// ULTRA ROM - Software Wizardry, Inc., (c) 1983 William G. Parrott, III
COMP( 1983, ultra19, h19,    0,      h19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ ULTRA ROM",    MACHINE_SUPPORTS_SAVE )

