// license:BSD-3-Clause
// copyright-holders:Foxhack, Angelo Salese
/*
    Nintendo Gateway 64

    LodgeNet was a pay-per-view system that allowed television, movie and video
	game rentals on hotels starting in 1993. They entered an agreement with
	Nintendo to allow the rental of their games on airplanes and hotels for a
	moderate fee.

	The LodgeNet 64 system was exclusively used in hotels, and consisted of a
	set-top box with a customized N64 system inside. You could choose one of
	several games via a TV-driven menu, which would then be downloaded to your
	room's system, and you would be charged a fee for one hour of playtime.

	The games were different from their retail counterparts; Games had to have
	their multiplayer modes, accessories and game save support removed, plus have
	additional support for the system's "halt" functions, so they would be allowed
	on the system. The game controllers were custom, with additional buttons for
	use with their rental system and a reset button, and did not allow the use of
	Rumble Paks or Memory Paks. While the hardware allowed temporary save files,
	these would be deleted from the system with a special utility before a new game
	was loaded. These files were stored in encrypted format in the LodgeNet
	servers, and were decrypted after being downloaded to the system. An extra file
	with additional info about the game, its encryption code, and EEPROM settings
	was also present.

	While the development info released mentions a requirement for PAL versions of
	the games, none are currently known to exist.

	38 Nintendo 64 titles were made available on the service, and 31 are currently
	dumped. The following games have not been found:

	- Extreme-G
	- Forsaken 64
	- Iggy's Reckin' Balls
	- Milo's Astro Lanes
	- Namco Museum 64
	- San Francisco Rush: Extreme Racing
	- Turok 2: Seeds of Evil

	These games will function, but the actual LodgeNet system is not emulated,
	so the extra controller buttons and other system functions are not
	implemented.

	The existing roms are decrypted, and should be considered bad dumps.

    TODO:
    - Set-top box part;
    - Extra joypad buttons;
    - Doesn't really have cartslot but rather uploads games thru a DOS/V Windows 95/98 program
      that works with encrypted files.
      \- Notice that the available dump is "patched to work on emulators",
         it may normally expect a specific ISA/PCI board setup plus a setup/install CD-ROM;
    - Host HALT override
      \- maps thru the "cartslot" space, adds extra state signals to the joypad
         (up + down reads halts, left + right unhalts)

*/

#include "emu.h"
#include "n64.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class n64_lodgenet_state : public n64_console_state
{
public:
	n64_lodgenet_state(const machine_config& mconfig, device_type type, const char* tag) :
		n64_console_state(mconfig, type, tag)
	{ }

    void n64_lodgenet_map(address_map &map);
    void n64_lodgenet(machine_config &config);
};

void n64_lodgenet_state::n64_lodgenet_map(address_map &map)
{
    n64_map(map);
    // map(0x0ff70000, 0x0ff70003) Host HALT control
}


static INPUT_PORTS_START( n64_lodgenet )
	PORT_START("input")
	PORT_CONFNAME(0x0003, 0x0001, "Controller Port 0 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Joypad")
//	PORT_CONFSETTING(0x02, "Mouse")
    PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
//	PORT_CONFNAME(0x000C, 0x0000, "Controller Port 1 Device")
//	PORT_CONFSETTING(0x00, "None")
//	PORT_CONFSETTING(0x04, "Joypad")
//	PORT_CONFSETTING(0x08, "Mouse")
//	PORT_CONFNAME(0x0030, 0x0000, "Controller Port 2 Device")
//	PORT_CONFSETTING(0x00, "None")
//	PORT_CONFSETTING(0x10, "Joypad")
//	PORT_CONFSETTING(0x20, "Mouse")
//	PORT_CONFNAME(0x00C0, 0x0000, "Controller Port 3 Device")
//	PORT_CONFSETTING(0x00, "None")
//	PORT_CONFSETTING(0x40, "Joypad")
//	PORT_CONFSETTING(0x80, "Mouse")

    PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )

//	PORT_CONFNAME(0x0100, 0x0000, "Disk Drive")
//	PORT_CONFSETTING(0x0000, "Retail")
//	PORT_CONFSETTING(0x0100, "Development")
//

    PORT_CONFNAME(0xC000, 0x8000, "EEPROM Size")
    PORT_CONFSETTING(0x0000, "None")
    PORT_CONFSETTING(0x8000, "4KB")
    PORT_CONFSETTING(0xC000, "16KB")

	//Player 1
    // TODO: extra buttons
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("P1 Button A / Left Click")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("P1 Button B / Right Click")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("P1 Button Z")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START )          PORT_PLAYER(1) PORT_NAME("P1 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("P1 Joypad \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("P1 Joypad \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("P1 Joypad \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Joypad \xE2\x86\x92") /* Right */
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("P1 Button L")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(1) PORT_NAME("P1 Button C \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(1) PORT_NAME("P1 Button C \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(1) PORT_NAME("P1 Button C \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )        PORT_PLAYER(1) PORT_NAME("P1 Button C \xE2\x86\x92") /* Right */

	PORT_START("P1_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("P1_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("P1_MOUSE_X")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_MOUSE_Y")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	//Player 2
	PORT_START("P2")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_ANALOG_X")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_ANALOG_Y")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MOUSE_X")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_MOUSE_Y")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	//Player 3
	PORT_START("P3")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3_ANALOG_X")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3_ANALOG_Y")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3_MOUSE_X")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3_MOUSE_Y")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	//Player 4
	PORT_START("P4")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4_ANALOG_X")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4_ANALOG_Y")
    PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4_MOUSE_X")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4_MOUSE_Y")
    PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
    // no reset button available
//	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER )        PORT_NAME("Warm Reset") PORT_CODE(KEYCODE_3)

INPUT_PORTS_END

void n64_lodgenet_state::n64_lodgenet(machine_config &config)
{
    n64_console_state::n64(config);
   	m_vr4300->set_addrmap(AS_PROGRAM, &n64_lodgenet_state::n64_lodgenet_map);
	m_vr4300->set_force_no_drc(false);
	m_rsp->set_force_no_drc(false);

	SOFTWARE_LIST(config.replace(), "cart_list").set_original("n64_lodgenet");
}

ROM_START( n64_lodgenet )
    ROM_REGION( 0x20000, "gateway64", ROMREGION_ERASEFF )
    // unknown CPU/controller, likely multiple ROMs for set-top box & game data receiver.
    ROM_LOAD( "gateway64.bios", 0, 0x20000, NO_DUMP )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for R4300 */

	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifdata.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

	ROM_REGION32_BE( 0x4000000, "user2", ROMREGION_ERASEFF)

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

CONS(1997?, n64_lodgenet,   n64,   0, n64_lodgenet,   n64_lodgenet, n64_lodgenet_state, empty_init, "Nintendo / LodgeNet", "Nintendo Gateway 64",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
