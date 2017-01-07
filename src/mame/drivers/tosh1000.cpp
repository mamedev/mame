// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Toshiba T1000 portable

    80C88 CPU @ 5 MHz [OKI MSM80C88A-10GS-K (56 pin PQFP)]
    512KB RAM + 16KB video RAM
    32KB BIOS ROM [Toshiba TC54256AD]
    256KB MS-DOS 2.11 ROM [Toshiba TC534000]
    SuperIO chip (Toshiba T7885) = 82C84 + 82C88 + 82C59 + upd765 + 82C53 + 82C37 + 82C55
    Real Time Clock chip: TC8521
    Keyboard controller: 80C50
    RS232C controller: 8250

    NB: dumps of ROM-DOS and CGA chargen are missing.

***************************************************************************/


#include "emu.h"

#include "machine/genpc.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "cpu/i86/i86.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "softlist.h"


#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
	if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-10s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)



class tosh1000_state : public driver_device
{
public:
	tosh1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "romdos")
		{ }

	DECLARE_MACHINE_RESET(tosh1000);
	DECLARE_DRIVER_INIT(tosh1000);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<address_map_bank_device> m_bankdev;
};


DRIVER_INIT_MEMBER( tosh1000_state, tosh1000 )
{
}

MACHINE_RESET_MEMBER( tosh1000_state, tosh1000 )
{
}


static ADDRESS_MAP_START( tosh1000_map, AS_PROGRAM, 8, tosh1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xa0000, 0xaffff) AM_NOP // AM_DEVREADWRITE("romdos", address_map_bank_device, read8, write8)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tosh1000_io, AS_IO, 8, tosh1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", ibm5160_mb_device, map)
	AM_RANGE(0x00c8, 0x00c8) AM_NOP // ROM-DOS page select [p. B-15]
	AM_RANGE(0x00e0, 0x00ef) AM_NOP // ???
	AM_RANGE(0x02c0, 0x02cf) AM_DEVREADWRITE("rtc", rp5c01_device, read, write)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( cfg_fdc_35 )
	MCFG_DEVICE_MODIFY("fdc:0")
	MCFG_SLOT_DEFAULT_OPTION("35dd")
	MCFG_SLOT_FIXED(true)

	MCFG_DEVICE_REMOVE("fdc:1")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tosh1000, tosh1000_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(tosh1000_map)
	MCFG_CPU_IO_MAP(tosh1000_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_MACHINE_RESET_OVERRIDE(tosh1000_state, tosh1000)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")

	MCFG_DEVICE_ADD("rtc", RP5C01, XTAL_32_768kHz)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("fdc_xt", cfg_fdc_35)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

//  MCFG_SOFTWARE_LIST_ADD("flop_list","tosh1000")

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END


ROM_START( tosh1000 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "v410", "V4.10")
	ROMX_LOAD( "tosh1000bios410.bin", 0x8000, 0x8000, CRC(a854939f) SHA1(0ff532f295a40716f53949a2fd64d02bf76d575a), ROM_BIOS(1))

	/* XXX IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END


/*     YEAR     ROM NAME    PARENT      COMPAT  MACHINE     INPUT                       INIT        COMPANY     FULLNAME */
COMP ( 1987,    tosh1000,   ibm5150,    0,      tosh1000,   0,        tosh1000_state,   tosh1000,   "Toshiba",  "Toshiba T1000", MACHINE_IS_SKELETON)
