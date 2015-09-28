// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Nintendo 64

    Driver by Ville Linde
    Reformatted to share hardware by R. Belmont
    Additional work by Ryan Holtz
    Porting from Mupen64 by Harmony
*/

#include "emu.h"
#include "cpu/rsp/rsp.h"
#include "cpu/mips/mips3.h"
#include "sound/dmadac.h"
#include "includes/n64.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "imagedev/snapquik.h"

class n64_mess_state : public n64_state
{
public:
	n64_mess_state(const machine_config &mconfig, device_type type, const char *tag)
		: n64_state(mconfig, type, tag)
		{ }

	DECLARE_READ32_MEMBER(dd_null_r);
	DECLARE_MACHINE_START(n64dd);
	INTERRUPT_GEN_MEMBER(n64_reset_poll);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(n64_cart);
	void mempak_format(UINT8* pak);
	int quickload(device_image_interface &image, const char *file_type, int quickload_size);
	DECLARE_QUICKLOAD_LOAD_MEMBER( n64dd );
};

READ32_MEMBER(n64_mess_state::dd_null_r)
{
	return 0xffffffff;
}

static ADDRESS_MAP_START( n64_map, AS_PROGRAM, 32, n64_mess_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_SHARE("rdram")                   // RDRAM
	AM_RANGE(0x03f00000, 0x03f00027) AM_DEVREADWRITE("rcp", n64_periphs, rdram_reg_r, rdram_reg_w)
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_SHARE("rsp_dmem")                    // RSP DMEM
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_SHARE("rsp_imem")                    // RSP IMEM
	AM_RANGE(0x04040000, 0x040fffff) AM_DEVREADWRITE("rcp", n64_periphs, sp_reg_r, sp_reg_w)  // RSP
	AM_RANGE(0x04100000, 0x041fffff) AM_DEVREADWRITE("rcp", n64_periphs, dp_reg_r, dp_reg_w)  // RDP
	AM_RANGE(0x04300000, 0x043fffff) AM_DEVREADWRITE("rcp", n64_periphs, mi_reg_r, mi_reg_w)    // MIPS Interface
	AM_RANGE(0x04400000, 0x044fffff) AM_DEVREADWRITE("rcp", n64_periphs, vi_reg_r, vi_reg_w)    // Video Interface
	AM_RANGE(0x04500000, 0x045fffff) AM_DEVREADWRITE("rcp", n64_periphs, ai_reg_r, ai_reg_w)    // Audio Interface
	AM_RANGE(0x04600000, 0x046fffff) AM_DEVREADWRITE("rcp", n64_periphs, pi_reg_r, pi_reg_w)    // Peripheral Interface
	AM_RANGE(0x04700000, 0x047fffff) AM_DEVREADWRITE("rcp", n64_periphs, ri_reg_r, ri_reg_w)    // RDRAM Interface
	AM_RANGE(0x04800000, 0x048fffff) AM_DEVREADWRITE("rcp", n64_periphs, si_reg_r, si_reg_w)    // Serial Interface
	AM_RANGE(0x05000508, 0x0500050b) AM_READ(dd_null_r);
	AM_RANGE(0x08000000, 0x0801ffff) AM_RAM AM_SHARE("sram")                                        // Cartridge SRAM
	AM_RANGE(0x10000000, 0x13ffffff) AM_ROM AM_REGION("user2", 0)                                   // Cartridge
	AM_RANGE(0x1fc00000, 0x1fc007bf) AM_ROM AM_REGION("user1", 0)                                   // PIF ROM
	AM_RANGE(0x1fc007c0, 0x1fc007ff) AM_DEVREADWRITE("rcp", n64_periphs, pif_ram_r, pif_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( n64dd_map, AS_PROGRAM, 32, n64_mess_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_SHARE("rdram")               // RDRAM
	AM_RANGE(0x03f00000, 0x03f00027) AM_DEVREADWRITE("rcp", n64_periphs, rdram_reg_r, rdram_reg_w)
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_SHARE("rsp_dmem")                    // RSP DMEM
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_SHARE("rsp_imem")                    // RSP IMEM
	AM_RANGE(0x04040000, 0x040fffff) AM_DEVREADWRITE("rcp", n64_periphs, sp_reg_r, sp_reg_w)  // RSP
	AM_RANGE(0x04100000, 0x041fffff) AM_DEVREADWRITE("rcp", n64_periphs, dp_reg_r, dp_reg_w)  // RDP
	AM_RANGE(0x04300000, 0x043fffff) AM_DEVREADWRITE("rcp", n64_periphs, mi_reg_r, mi_reg_w)    // MIPS Interface
	AM_RANGE(0x04400000, 0x044fffff) AM_DEVREADWRITE("rcp", n64_periphs, vi_reg_r, vi_reg_w)    // Video Interface
	AM_RANGE(0x04500000, 0x045fffff) AM_DEVREADWRITE("rcp", n64_periphs, ai_reg_r, ai_reg_w)    // Audio Interface
	AM_RANGE(0x04600000, 0x046fffff) AM_DEVREADWRITE("rcp", n64_periphs, pi_reg_r, pi_reg_w)    // Peripheral Interface
	AM_RANGE(0x04700000, 0x047fffff) AM_DEVREADWRITE("rcp", n64_periphs, ri_reg_r, ri_reg_w)    // RDRAM Interface
	AM_RANGE(0x04800000, 0x048fffff) AM_DEVREADWRITE("rcp", n64_periphs, si_reg_r, si_reg_w)    // Serial Interface
	AM_RANGE(0x05000000, 0x05ffffff) AM_DEVREADWRITE("rcp", n64_periphs, dd_reg_r, dd_reg_w) // 64DD Interface
	AM_RANGE(0x06000000, 0x063fffff) AM_ROM AM_REGION("ddipl", 0)                                   // 64DD IPL ROM
	AM_RANGE(0x08000000, 0x0801ffff) AM_RAM AM_SHARE("sram")                                        // Cartridge SRAM
	AM_RANGE(0x10000000, 0x13ffffff) AM_ROM AM_REGION("user2", 0)                                   // Cartridge
	AM_RANGE(0x1fc00000, 0x1fc007bf) AM_ROM AM_REGION("user1", 0)                                   // PIF ROM
	AM_RANGE(0x1fc007c0, 0x1fc007ff) AM_DEVREADWRITE("rcp", n64_periphs, pif_ram_r, pif_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rsp_map, AS_PROGRAM, 32, n64_mess_state )
	AM_RANGE(0x00000000, 0x00000fff) AM_RAM AM_SHARE("rsp_dmem")
	AM_RANGE(0x00001000, 0x00001fff) AM_RAM AM_SHARE("rsp_imem")
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_SHARE("rsp_dmem")
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_SHARE("rsp_imem")
ADDRESS_MAP_END

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

#if 0
/* ?? */
static const mips3_config config =
{
	16384,              /* code cache size */
	8192,               /* data cache size */
	62500000            /* system clock */
};
#endif

void n64_mess_state::mempak_format(UINT8* pak)
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
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0x00,0x71,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03
	};

	for (int i = 0; i < 0x8000; i += 2)
	{
		pak[i+0] = 0x00;
		pak[i+1] = 0x03;
	}
	memcpy(pak, pak_header, 272);
}

DEVICE_IMAGE_LOAD_MEMBER(n64_mess_state,n64_cart)
{
	int i, length;
	n64_periphs *periphs = machine().device<n64_periphs>("rcp");
	UINT8 *cart = memregion("user2")->base();

	if (image.software_entry() == NULL)
	{
		length = image.fread(cart, 0x4000000);
	}
	else
	{
		length = image.get_software_region_length("rom");
		memcpy(cart, image.get_software_region("rom"), length);
	}
	periphs->cart_length = length;

	if (cart[0] == 0x37 && cart[1] == 0x80)
	{
		for (i = 0; i < length; i += 4)
		{
			UINT8 b1 = cart[i + 0];
			UINT8 b2 = cart[i + 1];
			UINT8 b3 = cart[i + 2];
			UINT8 b4 = cart[i + 3];
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
			UINT8 b1 = cart[i + 0];
			UINT8 b2 = cart[i + 1];
			UINT8 b3 = cart[i + 2];
			UINT8 b4 = cart[i + 3];
			cart[i + 0] = b4;
			cart[i + 1] = b3;
			cart[i + 2] = b2;
			cart[i + 3] = b1;
		}
	}

	periphs->m_nvram_image = image;

	logerror("cart length = %d\n", length);

	device_image_interface *battery_image = dynamic_cast<device_image_interface *>(periphs->m_nvram_image);
	if(battery_image)
	{
		//printf("Loading\n");
		UINT8 data[0x30800];
		battery_image->battery_load(data, 0x30800, 0x00);
		//memcpy(n64_sram, data, 0x20000);
		memcpy(memshare("sram")->ptr(), data, 0x20000);
		memcpy(periphs->m_save_data.eeprom, data + 0x20000, 0x800);
		memcpy(periphs->m_save_data.mempak[0], data + 0x20800, 0x8000);
		memcpy(periphs->m_save_data.mempak[1], data + 0x28800, 0x8000);
	}

	if(periphs->m_save_data.mempak[0][0] == 0) // Init if new
	{
		memset(periphs->m_save_data.eeprom, 0, 0x800);
		mempak_format(periphs->m_save_data.mempak[0]);
		mempak_format(periphs->m_save_data.mempak[1]);
	}

	return IMAGE_INIT_PASS;
}

MACHINE_START_MEMBER(n64_mess_state,n64dd)
{
	machine_start();
	machine().device<n64_periphs>("rcp")->dd_present = true;
	UINT8 *ipl = memregion("ddipl")->base();

	for (int i = 0; i < 0x400000; i += 4)
	{
		UINT8 b1 = ipl[i + 0];
		UINT8 b2 = ipl[i + 1];
		UINT8 b3 = ipl[i + 2];
		UINT8 b4 = ipl[i + 3];
		ipl[i + 0] = b1;
		ipl[i + 1] = b2;
		ipl[i + 2] = b3;
		ipl[i + 3] = b4;
	}
}

QUICKLOAD_LOAD_MEMBER(n64_mess_state,n64dd)
{
	return quickload(image, file_type, quickload_size);
}

int n64_mess_state::quickload(device_image_interface &image, const char *file_type, int quickload_size)
{
	image.fseek(0, SEEK_SET);
	image.fread(memregion("disk")->base(), quickload_size);
	machine().device<n64_periphs>("rcp")->disk_present = true;
	return IMAGE_INIT_PASS;
}


INTERRUPT_GEN_MEMBER(n64_mess_state::n64_reset_poll)
{
	n64_periphs *periphs = machine().device<n64_periphs>("rcp");
	periphs->poll_reset_button((ioport("RESET")->read() & 1) ? true : false);
}

static MACHINE_CONFIG_START( n64, n64_mess_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4300BE, 93750000)
	MCFG_FORCE_NO_DRC()
	MCFG_CPU_CONFIG(config)
	MCFG_CPU_PROGRAM_MAP(n64_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", n64_mess_state, n64_reset_poll)

	MCFG_CPU_ADD("rsp", RSP, 62500000)
	MCFG_RSP_DP_REG_R_CB(DEVREAD32("rcp",n64_periphs, dp_reg_r))
	MCFG_RSP_DP_REG_W_CB(DEVWRITE32("rcp",n64_periphs, dp_reg_w))
	MCFG_RSP_SP_REG_R_CB(DEVREAD32("rcp",n64_periphs, sp_reg_r))
	MCFG_RSP_SP_REG_W_CB(DEVWRITE32("rcp",n64_periphs, sp_reg_w))
	MCFG_RSP_SP_SET_STATUS_CB(DEVWRITE32("rcp",n64_periphs, sp_set_status))
	MCFG_CPU_PROGRAM_MAP(rsp_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(1000000))
	//MCFG_QUANTUM_TIME(attotime::from_hz(1200))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	/* Video DACRATE is for quarter pixels, so the horizontal is also given in quarter pixels.  However, the horizontal and vertical timing and sizing is adjustable by register and will be reset when the registers are written. */
	MCFG_SCREEN_RAW_PARAMS(DACRATE_NTSC*2,3093,0,3093,525,0,525)
	//MCFG_SCREEN_REFRESH_RATE(60)
	//MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	//MCFG_SCREEN_SIZE(640, 525)
	//MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(n64_state, screen_update_n64)
	MCFG_SCREEN_VBLANK_DRIVER(n64_state, screen_eof_n64)

	MCFG_PALETTE_ADD("palette", 0x1000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_N64_PERIPHS_ADD("rcp");

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "n64_cart")
	MCFG_GENERIC_EXTENSIONS("v64,z64,rom,n64,bin")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(n64_mess_state, n64_cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "n64")

	MCFG_FORCE_NO_DRC()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( n64dd, n64 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(n64dd_map)

	MCFG_MACHINE_START_OVERRIDE(n64_mess_state, n64dd)

	MCFG_DEVICE_REMOVE("cartslot")
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "n64_cart")
	MCFG_GENERIC_EXTENSIONS("v64,z64,rom,n64,bin")
	MCFG_GENERIC_LOAD(n64_mess_state, n64_cart)

	MCFG_QUICKLOAD_ADD("quickload", n64_mess_state, n64dd, "bin,dsk", 0)
	MCFG_QUICKLOAD_INTERFACE("n64dd_disk")

	MCFG_SOFTWARE_LIST_ADD("dd_list", "n64dd")
MACHINE_CONFIG_END

ROM_START( n64 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )      /* dummy region for R4300 */

	ROM_REGION32_BE( 0x800, "user1", 0 )
	ROM_LOAD( "pifdata.bin", 0x0000, 0x0800, CRC(5ec82be9) SHA1(9174eadc0f0ea2654c95fd941406ab46b9dc9bdd) )

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

	ROM_REGION32_LE( 0x4400000, "disk", ROMREGION_ERASEFF)

	ROM_REGION16_BE( 0x80, "normpoint", 0 )
	ROM_LOAD( "normpnt.rom", 0x00, 0x80, CRC(e7f2a005) SHA1(c27b4a364a24daeee6e99fd286753fd6216362b4) )

	ROM_REGION16_BE( 0x80, "normslope", 0 )
	ROM_LOAD( "normslp.rom", 0x00, 0x80, CRC(4f2ae525) SHA1(eab43f8cc52c8551d9cff6fced18ef80eaba6f05) )
ROM_END

CONS(1996, n64,     0,      0,      n64,    n64, driver_device, 0,  "Nintendo", "Nintendo 64", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS(1996, n64dd,   n64,    0,      n64dd,  n64, driver_device, 0,  "Nintendo", "Nintendo 64DD", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
