// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Nintendo 64

    Driver by Ville Linde
    Reformatted to share hardware by R. Belmont
    Additional work by Ryan Holtz
    Porting from Mupen64 by Ryan Holtz
*/

#include "emu.h"
#include "n64.h"

#include "emupal.h"
#include "softlist.h"
#include "speaker.h"

class n64_console_state : public n64_state
{
public:
	n64_console_state(const machine_config &mconfig, device_type type, const char *tag)
		: n64_state(mconfig, type, tag)
		{ }

	void n64(machine_config &config);
	void n64dd(machine_config &config);

private:
	uint32_t dd_null_r();
	void n64_map(address_map &map) ATTR_COLD;
	void n64dd_map(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(n64dd);
	INTERRUPT_GEN_MEMBER(n64_reset_poll);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void mempak_format(uint8_t* pak);
	std::error_condition disk_load(device_image_interface &image);
	void disk_unload(device_image_interface &image);
	void rsp_imem_map(address_map &map) ATTR_COLD;
	void rsp_dmem_map(address_map &map) ATTR_COLD;
};

uint32_t n64_console_state::dd_null_r()
{
	return 0xffffffff;
}

void n64_console_state::n64_map(address_map &map)
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
	map(0x05000508, 0x0500050b).r(FUNC(n64_console_state::dd_null_r));
	map(0x08000000, 0x0801ffff).ram().share("sram");                                        // Cartridge SRAM
	map(0x10000000, 0x13ffffff).rom().region("user2", 0);                                   // Cartridge
	map(0x1fc00000, 0x1fc007bf).rom().region("user1", 0);                                   // PIF ROM
	map(0x1fc007c0, 0x1fc007ff).rw("rcp", FUNC(n64_periphs::pif_ram_r), FUNC(n64_periphs::pif_ram_w));
}

void n64_console_state::n64dd_map(address_map &map)
{
	n64_console_state::n64_map(map);
	// TODO: expansion bus
	map(0x05000000, 0x05ffffff).rw("rcp", FUNC(n64_periphs::dd_reg_r), FUNC(n64_periphs::dd_reg_w)); // 64DD Interface
	map(0x06000000, 0x063fffff).rom().region("ddipl", 0);                                   // 64DD IPL ROM
}

void n64_console_state::rsp_imem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_imem");
}

void n64_console_state::rsp_dmem_map(address_map &map)
{
	map(0x00000000, 0x00000fff).ram().share("rsp_dmem");
}

static INPUT_PORTS_START( n64 )
	PORT_START("input")
	PORT_CONFNAME(0x0003, 0x0001, "Controller Port 0 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Joypad")
	PORT_CONFSETTING(0x02, "Mouse")
	PORT_CONFNAME(0x000C, 0x0000, "Controller Port 1 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x04, "Joypad")
	PORT_CONFSETTING(0x08, "Mouse")
	PORT_CONFNAME(0x0030, 0x0000, "Controller Port 2 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x10, "Joypad")
	PORT_CONFSETTING(0x20, "Mouse")
	PORT_CONFNAME(0x00C0, 0x0000, "Controller Port 3 Device")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x40, "Joypad")
	PORT_CONFSETTING(0x80, "Mouse")

	PORT_CONFNAME(0x0100, 0x0000, "Disk Drive")
	PORT_CONFSETTING(0x0000, "Retail")
	PORT_CONFSETTING(0x0100, "Development")

	PORT_CONFNAME(0xC000, 0x8000, "EEPROM Size")
	PORT_CONFSETTING(0x0000, "None")
	PORT_CONFSETTING(0x8000, "4KB")
	PORT_CONFSETTING(0xC000, "16KB")

	//Player 1
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
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_X ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("P1_MOUSE_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_Y ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_REVERSE

	//Player 2
	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("P2 Button A / Left Click")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("P2 Button B / Right Click")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(2) PORT_NAME("P2 Button Z")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START )          PORT_PLAYER(2) PORT_NAME("P2 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_NAME("P2 Joypad \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_NAME("P2 Joypad \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_NAME("P2 Joypad \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 Joypad \xE2\x86\x92") /* Right */
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(2) PORT_NAME("P2 Button L")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(2) PORT_NAME("P2 Button C \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(2) PORT_NAME("P2 Button C \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(2) PORT_NAME("P2 Button C \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )        PORT_PLAYER(2) PORT_NAME("P2 Button C \xE2\x86\x92") /* Right */

	PORT_START("P2_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("P2_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("P2_MOUSE_X")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_X ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)

	PORT_START("P2_MOUSE_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_Y ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2) PORT_REVERSE

	//Player 3
	PORT_START("P3")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(3) PORT_NAME("P3 Button A / Left Click")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(3) PORT_NAME("P3 Button B / Right Click")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(3) PORT_NAME("P3 Button Z")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START )          PORT_PLAYER(3) PORT_NAME("P3 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_NAME("P3 Joypad \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_NAME("P3 Joypad \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_NAME("P3 Joypad \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_NAME("P3 Joypad \xE2\x86\x92") /* Right */
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(3) PORT_NAME("P3 Button L")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(3) PORT_NAME("P3 Button R")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(3) PORT_NAME("P3 Button C \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(3) PORT_NAME("P3 Button C \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(3) PORT_NAME("P3 Button C \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )        PORT_PLAYER(3) PORT_NAME("P3 Button C \xE2\x86\x92") /* Right */

	PORT_START("P3_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(3)

	PORT_START("P3_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(3) PORT_REVERSE

	PORT_START("P3_MOUSE_X")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_X ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(3)

	PORT_START("P3_MOUSE_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_Y ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(3) PORT_REVERSE

	//Player 4
	PORT_START("P4")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(4) PORT_NAME("P4 Button A / Left Click")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(4) PORT_NAME("P4 Button B / Right Click")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(4) PORT_NAME("P4 Button Z")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START )          PORT_PLAYER(4) PORT_NAME("P4 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(4) PORT_NAME("P4 Joypad \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4) PORT_NAME("P4 Joypad \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4) PORT_NAME("P4 Joypad \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_NAME("P4 Joypad \xE2\x86\x92") /* Right */
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(4) PORT_NAME("P4 Button L")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(4) PORT_NAME("P4 Button R")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(4) PORT_NAME("P4 Button C \xE2\x86\x91") /* Up */
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(4) PORT_NAME("P4 Button C \xE2\x86\x93") /* Down */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(4) PORT_NAME("P4 Button C \xE2\x86\x90") /* Left */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )        PORT_PLAYER(4) PORT_NAME("P4 Button C \xE2\x86\x92") /* Right */

	PORT_START("P4_ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(4)

	PORT_START("P4_ANALOG_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(4) PORT_REVERSE

	PORT_START("P4_MOUSE_X")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_X ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(4)

	PORT_START("P4_MOUSE_Y")
	PORT_BIT( 0xffff, 0x0000, IPT_MOUSE_Y ) PORT_MINMAX(0x0000,0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(4) PORT_REVERSE

	PORT_START("RESET")
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER )        PORT_NAME("Warm Reset") PORT_CODE(KEYCODE_3)

INPUT_PORTS_END

void n64_console_state::mempak_format(uint8_t* pak)
{
	unsigned char pak_header[] =
	{
		0x81,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
		0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b, 0x1c,0x1d,0x1e,0x1f,
		0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
		0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
	};

	unsigned char pak_inode_table[] =
	{
		0x01,0x71,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03
	};

	memset(pak, 0, 0x8000);
	memcpy(pak, pak_header, 256);
	memcpy(pak+256, pak_inode_table, 256); // Main
	memcpy(pak+512, pak_inode_table, 256); // Mirror
}

DEVICE_IMAGE_LOAD_MEMBER(n64_console_state::cart_load)
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
		memcpy(m_rcp_periphs->m_save_data.mempak[0], data + 0x20800, 0x8000);
		memcpy(m_rcp_periphs->m_save_data.mempak[1], data + 0x28800, 0x8000);
	}

	if (m_rcp_periphs->m_save_data.mempak[0][0] == 0) // Init if new
	{
		memset(m_rcp_periphs->m_save_data.eeprom, 0, 0x800);
		mempak_format(m_rcp_periphs->m_save_data.mempak[0]);
		mempak_format(m_rcp_periphs->m_save_data.mempak[1]);
	}

	return std::make_pair(std::error_condition(), std::string());
}

MACHINE_START_MEMBER(n64_console_state,n64dd)
{
	machine_start();
	m_rcp_periphs->dd_present = true;
}

std::error_condition n64_console_state::disk_load(device_image_interface &image)
{
	image.fseek(0, SEEK_SET);
	image.fread(memregion("disk")->base(), image.length());
	m_rcp_periphs->disk_present = true;
	return std::error_condition();
}

void n64_console_state::disk_unload(device_image_interface &image)
{
	m_rcp_periphs->disk_present = false;
}

INTERRUPT_GEN_MEMBER(n64_console_state::n64_reset_poll)
{
	m_rcp_periphs->poll_reset_button((ioport("RESET")->read() & 1) ? true : false);
}

void n64_console_state::n64(machine_config &config)
{
	/* basic machine hardware */
	VR4300BE(config, m_vr4300, 93750000);
	m_vr4300->set_force_no_drc(false);
	//m_vr4300->set_icache_size(16384);
	//m_vr4300->set_dcache_size(8192);
	//m_vr4300->set_system_clock(62500000);
	m_vr4300->set_addrmap(AS_PROGRAM, &n64_console_state::n64_map);
	m_vr4300->set_vblank_int("screen", FUNC(n64_console_state::n64_reset_poll));

	RSP(config, m_rsp, 62500000);
	m_rsp->set_force_no_drc(false);
	m_rsp->dp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_r));
	m_rsp->dp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::dp_reg_w));
	m_rsp->sp_reg_r().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_r));
	m_rsp->sp_reg_w().set(m_rcp_periphs, FUNC(n64_periphs::sp_reg_w));
	m_rsp->status_set().set(m_rcp_periphs, FUNC(n64_periphs::sp_set_status));
	m_rsp->set_addrmap(AS_PROGRAM, &n64_console_state::rsp_imem_map);
	m_rsp->set_addrmap(AS_DATA, &n64_console_state::rsp_dmem_map);

	config.set_maximum_quantum(attotime::from_hz(500000));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	/* Video DACRATE is for quarter pixels, so the horizontal is also given in quarter pixels.  However, the horizontal and vertical timing and sizing is adjustable by register and will be reset when the registers are written. */
	// TODO: with 480 vertical will generate invalid vblanks
	// cfr. amenairc -drc
	m_screen->set_raw(DACRATE_NTSC*2,3093,0,3093,525,0,525);
	//m_screen->set_raw(DACRATE_NTSC*2,3093,0,3093,525,0,480);
	m_screen->set_screen_update(FUNC(n64_state::screen_update));
	m_screen->screen_vblank().set(FUNC(n64_state::screen_vblank));

	PALETTE(config, "palette").set_entries(0x1000);

	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, "dac2").add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DMADAC(config, "dac1").add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	N64PERIPH(config, m_rcp_periphs, 0);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "n64_cart", "v64,z64,rom,n64,bin"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(n64_console_state::cart_load));

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("n64");
}

// TODO: different xtal for PAL

void n64_console_state::n64dd(machine_config &config)
{
	n64(config);
	m_vr4300->set_addrmap(AS_PROGRAM, &n64_console_state::n64dd_map);

	MCFG_MACHINE_START_OVERRIDE(n64_console_state, n64dd)

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config.replace(), "cartslot", generic_plain_slot, "n64_cart"));
	cartslot.set_extensions("v64,z64,rom,n64,bin");
	cartslot.set_device_load(FUNC(n64_console_state::cart_load));

	harddisk_image_device &hdd(HARDDISK(config, "n64disk"));
	hdd.set_device_load(FUNC(n64_console_state::disk_load));
	hdd.set_device_unload(FUNC(n64_console_state::disk_unload));
	hdd.set_interface("n64dd_disk");

	SOFTWARE_LIST(config, "dd_list").set_original("n64dd");
}

ROM_START( n64 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for R4300 */

	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifntsc.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

	ROM_REGION32_BE( 0x4000000, "user2", ROMREGION_ERASEFF)

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

ROM_START( n64pal )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for R4300 */

	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifpal.bin", 0x0000, 0x0800, CRC(2ae77e68) SHA1(46cae59d31f9298b93f3380879454fcef54ee6cc) )

	ROM_REGION32_BE( 0x4000000, "user2", ROMREGION_ERASEFF)

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END


ROM_START( n64dd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for R4300 */

	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifdata.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

	ROM_REGION32_BE( 0x4000000, "user2", ROMREGION_ERASEFF)

	ROM_REGION32_BE( 0x400000, "ddipl", ROMREGION_ERASEFF)
	ROM_LOAD( "64ddipl.bin", 0x000000, 0x400000, CRC(7f933ce2) SHA1(bf861922dcb78c316360e3e742f4f70ff63c9bc3) )

	ROM_REGION32_BE( 0x4400000, "disk", ROMREGION_ERASEFF)

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

CONS(1996, n64,    0,   0, n64,   n64, n64_console_state, empty_init, "Nintendo", "Nintendo 64 (NTSC)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS(1997, n64pal, n64, 0, n64,   n64, n64_console_state, empty_init, "Nintendo", "Nintendo 64 (PAL)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS(1999, n64dd,  n64, 0, n64dd, n64, n64_console_state, empty_init, "Nintendo", "Nintendo 64DD",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
