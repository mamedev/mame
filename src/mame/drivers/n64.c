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
#include "imagedev/cartslot.h"
#include "includes/n64.h"


static ADDRESS_MAP_START( n64_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM	AM_BASE(&rdram)				// RDRAM
	AM_RANGE(0x03f00000, 0x03f00027) AM_READWRITE(n64_rdram_reg_r, n64_rdram_reg_w)
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_SHARE("dmem")					// RSP DMEM
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_SHARE("imem")					// RSP IMEM
	AM_RANGE(0x04040000, 0x040fffff) AM_DEVREADWRITE("rsp", n64_sp_reg_r, n64_sp_reg_w)	// RSP
	AM_RANGE(0x04100000, 0x041fffff) AM_DEVREADWRITE("rsp", n64_dp_reg_r, n64_dp_reg_w)	// RDP
	AM_RANGE(0x04300000, 0x043fffff) AM_READWRITE(n64_mi_reg_r, n64_mi_reg_w)	// MIPS Interface
	AM_RANGE(0x04400000, 0x044fffff) AM_READWRITE(n64_vi_reg_r, n64_vi_reg_w)	// Video Interface
	AM_RANGE(0x04500000, 0x045fffff) AM_READWRITE(n64_ai_reg_r, n64_ai_reg_w)	// Audio Interface
	AM_RANGE(0x04600000, 0x046fffff) AM_READWRITE(n64_pi_reg_r, n64_pi_reg_w)	// Peripheral Interface
	AM_RANGE(0x04700000, 0x047fffff) AM_READWRITE(n64_ri_reg_r, n64_ri_reg_w)	// RDRAM Interface
	AM_RANGE(0x04800000, 0x048fffff) AM_READWRITE(n64_si_reg_r, n64_si_reg_w)	// Serial Interface
	//AM_RANGE(0x08000000, 0x08007fff) AM_RAM                                   // Cartridge SRAM
	AM_RANGE(0x10000000, 0x13ffffff) AM_ROM AM_REGION("user2", 0)	// Cartridge
	AM_RANGE(0x1fc00000, 0x1fc007bf) AM_ROM AM_REGION("user1", 0)	// PIF ROM
	AM_RANGE(0x1fc007c0, 0x1fc007ff) AM_READWRITE(n64_pif_ram_r, n64_pif_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rsp_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000fff) AM_RAM AM_SHARE("dmem")
	AM_RANGE(0x00001000, 0x00001fff) AM_RAM AM_SHARE("imem")
	AM_RANGE(0x04000000, 0x04000fff) AM_RAM AM_BASE(&rsp_dmem) AM_SHARE("dmem")
	AM_RANGE(0x04001000, 0x04001fff) AM_RAM AM_BASE(&rsp_imem) AM_SHARE("imem")
ADDRESS_MAP_END

static INPUT_PORTS_START( n64 )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("P1 Button A")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("P1 Button B")
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

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("P2 Button A")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("P2 Button B")
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

INPUT_PORTS_END

/* ?? */
static const mips3_config config =
{
	16384,				/* code cache size */
	8192,				/* data cache size */
	62500000			/* system clock */
};

static INTERRUPT_GEN( n64_vblank )
{
	signal_rcp_interrupt(device->machine(), VI_INTERRUPT);
}

static DEVICE_IMAGE_LOAD(n64_cart)
{
	int i, length;
	UINT8 *cart = image.device().machine().region("user2")->base();

	if (image.software_entry() == NULL)
	{
		length = image.fread( cart, 0x4000000);
	}
	else
	{
		length = image.get_software_region_length("rom");
		memcpy(cart, image.get_software_region("rom"), length);
	}

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

	logerror("cart length = %d\n", length);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( n64, _n64_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4300BE, 93750000)
	MCFG_CPU_CONFIG(config)
	MCFG_CPU_PROGRAM_MAP(n64_map)
	MCFG_CPU_VBLANK_INT("screen", n64_vblank)

	MCFG_CPU_ADD("rsp", RSP, 62500000)
	MCFG_CPU_CONFIG(n64_rsp_config)
	MCFG_CPU_PROGRAM_MAP(rsp_map)

	MCFG_MACHINE_START( n64 )
	MCFG_MACHINE_RESET( n64 )
	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 525)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_UPDATE(n64)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(n64)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("v64,z64,rom,n64,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("n64_cart")
	MCFG_CARTSLOT_LOAD(n64_cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","n64")
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

CONS(1996, n64, 	0,		0,		n64,	n64,	0,	"Nintendo", "Nintendo 64", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
