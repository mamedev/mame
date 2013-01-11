/******************************************************************************
 Synertek Systems Corp. SYM-1

 Early driver by PeT mess@utanet.at May 2000
 Rewritten by Dirk Best October 2007

******************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/speaker.h"
#include "includes/sym1.h"

/* Peripheral chips */
#include "machine/6532riot.h"
#include "machine/6522via.h"
#include "machine/74145.h"

/* Layout */
#include "sym1.lh"

#include "machine/ram.h"


/* pointers to memory locations */



/******************************************************************************
 Memory Maps
******************************************************************************/


static ADDRESS_MAP_START( sym1_map, AS_PROGRAM, 8, sym1_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM                              /* U12/U13 RAM */
	AM_RANGE(0x0400, 0x07ff) AM_RAMBANK("bank2") AM_SHARE("ram_1k")
	AM_RANGE(0x0800, 0x0bff) AM_RAMBANK("bank3") AM_SHARE("ram_2k")
	AM_RANGE(0x0c00, 0x0fff) AM_RAMBANK("bank4") AM_SHARE("ram_3k")
	AM_RANGE(0x8000, 0x8fff) AM_ROM AM_SHARE("monitor")       /* U20 Monitor ROM */
	AM_RANGE(0xa000, 0xa00f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)      /* U25 VIA #1 */
	AM_RANGE(0xa400, 0xa40f) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)  /* U27 RIOT */
	AM_RANGE(0xa600, 0xa67f) AM_RAMBANK("bank5") AM_SHARE("riot_ram")  /* U27 RIOT RAM */
	AM_RANGE(0xa800, 0xa80f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)      /* U28 VIA #2 */
	AM_RANGE(0xac00, 0xac0f) AM_DEVREADWRITE("via6522_2", via6522_device, read, write)      /* U29 VIA #3 */
	AM_RANGE(0xb000, 0xefff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
 Input Ports
******************************************************************************/


static INPUT_PORTS_START( sym1 )
	PORT_START("ROW-0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0     USR 0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4     USR 4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8     JUMP")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C     CALC")  PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CR    S DBL") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO    LD P")  PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LD 2  LD 1")  PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW-1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1     USR 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5     USR 5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9     VER")   PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D     DEP")   PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-     +")     PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG   SAV P") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAV 2 SAV 1") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW-2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2     USR 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6     USR 6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A     ASCII") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E     EXEC")  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x92     \xE2\x86\x90") PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM   WP")    PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW-3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3     USR 3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7     USR 7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B     B MOV") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F     FILL")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")       PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")           /* IN4 */
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEBUG OFF") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEBUG ON")  PORT_CODE(KEYCODE_F7)

	PORT_START("WP")
	PORT_DIPNAME(0x01, 0x01, "6532 RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "1K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "2K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "3K RAM WP")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
INPUT_PORTS_END



/******************************************************************************
 Machine Drivers
******************************************************************************/


static MACHINE_CONFIG_START( sym1, sym1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, SYM1_CLOCK)  /* 1 MHz */
	MCFG_CPU_PROGRAM_MAP(sym1_map)

	MCFG_DEFAULT_LAYOUT(layout_sym1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* devices */
	MCFG_RIOT6532_ADD("riot", SYM1_CLOCK, sym1_r6532_interface)
	MCFG_TTL74145_ADD("ttl74145", sym1_ttl74145_intf)
	MCFG_VIA6522_ADD("via6522_0", 0, sym1_via0)
	MCFG_VIA6522_ADD("via6522_1", 0, sym1_via1)
	MCFG_VIA6522_ADD("via6522_2", 0, sym1_via2)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("1K,2K,3K")
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/


ROM_START( sym1 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ver10",  "Version 1.0")
	ROMX_LOAD("symon1_0.bin", 0x8000, 0x1000, CRC(97928583) SHA1(6ac52c54adb7a086d51bc7f6d55dd30ab3a0a331),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ver11",  "Version 1.1")
	ROMX_LOAD("symon1_1.bin", 0x8000, 0x1000, CRC(7a4b1e12) SHA1(cebdf815105592658cfb7af262f2101d2aeab786),ROM_BIOS(2))
	ROM_LOAD("rae_b000", 0xb000, 0x1000, CRC(F6429326) SHA1(6f2f10649b54f54217bb35c8c453b5d05434bd86) )
	ROM_LOAD("bas_c000", 0xc000, 0x1000, CRC(C168FE70) SHA1(7447a5e229140cbbde4cf90886966a5d93aa24e1) )
	ROM_LOAD("bas_d000", 0xd000, 0x1000, CRC(8375A978) SHA1(240301bf8bb8ddb99b65a585f17895e1ad872631) )
	ROM_LOAD("rae_e000", 0xe000, 0x1000, CRC(2255444B) SHA1(c7dd812962c2e2edd2faa7055e9cce4e769c0388) )
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/
/*    YEAR  NAME  PARENT COMPAT MACHINE INPUT INIT  COMPANY                   FULLNAME          FLAGS */
COMP( 1978, sym1, 0,     0,     sym1,   sym1, sym1_state, sym1, "Synertek Systems Corp.", "SYM-1/SY-VIM-1", 0 )
