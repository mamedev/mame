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
         it normally expect a specific ISA/PCI SCSI board plus a setup/install CD-ROM;
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

class n64_gateway_state : public n64_state
{
public:
	n64_gateway_state(const machine_config& mconfig, device_type type, const char* tag) :
		n64_state(mconfig, type, tag)
	{ }

	void n64_lodgenet_map(address_map &map) ATTR_COLD;
	void n64_lodgenet(machine_config &config);

private:
	void n64_map(address_map &map) ATTR_COLD;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void rsp_imem_map(address_map &map) ATTR_COLD;
	void rsp_dmem_map(address_map &map) ATTR_COLD;
};

void n64_gateway_state::n64_lodgenet_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram().share("rdram");                   // RDRAM
	map(0x03f00000, 0x03f00027).rw("rcp", FUNC(n64_periphs::rdram_reg_r), FUNC(n64_periphs::rdram_reg_w));
	map(0x04000000, 0x04000fff).ram().share("rsp_dmem");                    // RSP DMEM
	map(0x04001000, 0x04001fff).ram().share("rsp_imem");                    // RSP IMEM
	map(0x04040000, 0x040fffff).rw("rcp", FUNC(n64_periphs::sp_reg_r), FUNC(n64_periphs::sp_reg_w));  // RSP
	map(0x04100000, 0x041fffff).rw("rcp", FUNC(n64_periphs::dp_reg_r), FUNC(n64_periphs::dp_reg_w));  // RDP
	map(0x04300000, 0x043fffff).rw("rcp", FUNC(n64_periphs::mi_reg_r), FUNC(n64_periphs::mi_reg_w));    // MIPS Interface
	map(0x04400000, 0x044fffff).rw("rcp", FUNC(n64_periphs::vi_reg_r), FUNC(n64_periphs::vi_reg_w));    // Video Interface
	map(0x04500000, 0x045fffff).rw("rcp", FUNC(n64_periphs::ai_reg_r), FUNC(n64_periphs::ai_reg_w));    // Audio Interface
	map(0x04600000, 0x046fffff).rw("rcp", FUNC(n64_periphs::pi_reg_r), FUNC(n64_periphs::pi_reg_w));    // Peripheral Interface
	map(0x04700000, 0x047fffff).rw("rcp", FUNC(n64_periphs::ri_reg_r), FUNC(n64_periphs::ri_reg_w));    // RDRAM Interface
	map(0x04800000, 0x048fffff).rw("rcp", FUNC(n64_periphs::si_reg_r), FUNC(n64_periphs::si_reg_w));    // Serial Interface
	map(0x05000508, 0x0500050b).lr32(NAME([] () { return 0xffffffff; }));
	map(0x08000000, 0x0801ffff).ram().share("sram");                                        // Cartridge SRAM
	map(0x10000000, 0x13ffffff).rom().region("user2", 0);                                   // Cartridge
	map(0x1fc00000, 0x1fc007bf).rom().region("user1", 0);                                   // PIF ROM
	map(0x1fc007c0, 0x1fc007ff).rw("rcp", FUNC(n64_periphs::pif_ram_r), FUNC(n64_periphs::pif_ram_w));

	// map(0x0ff70000, 0x0ff70003) Host HALT control
}

void n64_gateway_state::rsp_imem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_imem");
}

void n64_gateway_state::rsp_dmem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_dmem");
}



static INPUT_PORTS_START( n64_lodgenet )
	PORT_START("input")
	PORT_CONFNAME(0x0003, 0x0001, "Controller Port 0 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Joypad")
//  PORT_CONFSETTING(0x02, "Mouse")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
//  PORT_CONFNAME(0x000C, 0x0000, "Controller Port 1 Device")
//  PORT_CONFSETTING(0x00, "None")
//  PORT_CONFSETTING(0x04, "Joypad")
//  PORT_CONFSETTING(0x08, "Mouse")
//  PORT_CONFNAME(0x0030, 0x0000, "Controller Port 2 Device")
//  PORT_CONFSETTING(0x00, "None")
//  PORT_CONFSETTING(0x10, "Joypad")
//  PORT_CONFSETTING(0x20, "Mouse")
//  PORT_CONFNAME(0x00C0, 0x0000, "Controller Port 3 Device")
//  PORT_CONFSETTING(0x00, "None")
//  PORT_CONFSETTING(0x40, "Joypad")
//  PORT_CONFSETTING(0x80, "Mouse")

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )

//  PORT_CONFNAME(0x0100, 0x0000, "Disk Drive")
//  PORT_CONFSETTING(0x0000, "Retail")
//  PORT_CONFSETTING(0x0100, "Development")
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
//  PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER )        PORT_NAME("Warm Reset") PORT_CODE(KEYCODE_3)

INPUT_PORTS_END

// TODO: c&p from n64.cpp, shouldn't have this at all
DEVICE_IMAGE_LOAD_MEMBER(n64_gateway_state::cart_load)
{
	int i, length;
	uint8_t *cart = memregion("user2")->base();

	if (!image.loaded_through_softlist())
	{
		length = image.fread(cart, 0x4000000);
	}
	else
	{
		length = image.get_software_region_length("rom");
		memcpy(cart, image.get_software_region("rom"), length);
	}
	m_rcp_periphs->cart_length = length;

	if (cart[0] == 0x37 && cart[1] == 0x80)
	{
		for (i = 0; i < length; i += 4)
		{
			uint8_t b1 = cart[i + 0];
			uint8_t b2 = cart[i + 1];
			uint8_t b3 = cart[i + 2];
			uint8_t b4 = cart[i + 3];
			cart[i + 0] = b3;
			cart[i + 1] = b4;
			cart[i + 2] = b1;
			cart[i + 3] = b2;
		}
	}
	else
	{
		for (i = 0; i < length; i += 4)
		{
			uint8_t b1 = cart[i + 0];
			uint8_t b2 = cart[i + 1];
			uint8_t b3 = cart[i + 2];
			uint8_t b4 = cart[i + 3];
			cart[i + 0] = b4;
			cart[i + 1] = b3;
			cart[i + 2] = b2;
			cart[i + 3] = b1;
		}
	}

	m_rcp_periphs->m_nvram_image = &image.device();

	logerror("cart length = %d\n", length);

	device_image_interface *battery_image = dynamic_cast<device_image_interface *>(m_rcp_periphs->m_nvram_image);
	if (battery_image)
	{
		//printf("Loading\n");
		uint8_t data[0x30800];
		battery_image->battery_load(data, 0x30800, 0x00);
		if (m_sram != nullptr)
		{
			memcpy(m_sram, data, 0x20000);
		}
		memcpy(m_rcp_periphs->m_save_data.eeprom, data + 0x20000, 0x800);
		//memcpy(m_rcp_periphs->m_save_data.mempak[0], data + 0x20800, 0x8000);
		//memcpy(m_rcp_periphs->m_save_data.mempak[1], data + 0x28800, 0x8000);
	}

	//if (m_rcp_periphs->m_save_data.mempak[0][0] == 0) // Init if new
	//{
		//memset(m_rcp_periphs->m_save_data.eeprom, 0, 0x800);
		//mempak_format(m_rcp_periphs->m_save_data.mempak[0]);
		//mempak_format(m_rcp_periphs->m_save_data.mempak[1]);
	//}

	return std::make_pair(std::error_condition(), std::string());
}

void n64_gateway_state::n64_lodgenet(machine_config &config)
{
	VR4300BE(config, m_vr4300, 93750000);
	m_vr4300->set_force_no_drc(false);
	//m_vr4300->set_icache_size(16384);
	//m_vr4300->set_dcache_size(8192);
	//m_vr4300->set_system_clock(62500000);
	m_vr4300->set_addrmap(AS_PROGRAM, &n64_gateway_state::n64_lodgenet_map);
//  m_vr4300->set_vblank_int("screen", FUNC(n64_console_state::n64_reset_poll));

	RSP(config, m_rsp, 62500000);
	m_rsp->set_force_no_drc(false);
	m_rsp->dp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_r));
	m_rsp->dp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_w));
	m_rsp->sp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_r));
	m_rsp->sp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_w));
	m_rsp->status_set().set(m_rcp_periphs, FUNC(n64_periphs::sp_set_status));
	m_rsp->set_addrmap(AS_PROGRAM, &n64_gateway_state::rsp_imem_map);
	m_rsp->set_addrmap(AS_DATA, &n64_gateway_state::rsp_dmem_map);

	config.set_maximum_quantum(attotime::from_hz(500000));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DACRATE_NTSC*2,3093,0,3093,525,0,525);
	m_screen->set_screen_update(FUNC(n64_state::screen_update));
	m_screen->screen_vblank().set(FUNC(n64_state::screen_vblank));

	PALETTE(config, "palette").set_entries(0x1000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, "dac2").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	DMADAC(config, "dac1").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	N64PERIPH(config, m_rcp_periphs, 0);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "n64_cart", "v64,z64,rom,n64,bin"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(n64_gateway_state::cart_load));


	SOFTWARE_LIST(config, "cart_list").set_original("n64_lodgenet");
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

CONS(1997?, n64_lodgenet,   n64,   0, n64_lodgenet,   n64_lodgenet, n64_gateway_state, empty_init, "Nintendo / LodgeNet", "Nintendo Gateway 64",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
