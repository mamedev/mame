/***************************************************************************

    drivers/genpc.c

    Driver file for geenric PC machines

***************************************************************************/


#include "emu.h"

#include "includes/genpc.h"

#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"

#include "video/pc_cga.h"
#include "video/isa_ega.h"
#include "video/isa_mda.h"
#include "video/isa_svga_tseng.h"
#include "video/isa_svga_s3.h"

#include "machine/ram.h"
#include "machine/isa.h"

#include "machine/isa_adlib.h"
#include "machine/isa_com.h"
#include "machine/isa_fdc.h"
#include "machine/isa_finalchs.h"
#include "machine/isa_gblaster.h"
#include "machine/isa_hdc.h"
#include "machine/isa_sblaster.h"
#include "machine/isa_mpu401.h"
#include "machine/3c503.h"
#include "machine/ne1000.h"
#include "machine/isa_ibm_mfc.h"
#include "machine/pc_lpt.h"

#include "machine/pc_keyboards.h"

class genpc_state : public driver_device
{
public:
	genpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};

static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc16_map, AS_PROGRAM, 16, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(pc8_io, AS_IO, 8, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc16_io, AS_IO, 16, genpc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END


static INPUT_PORTS_START( pcgen )
INPUT_PORTS_END

static INPUT_PORTS_START( pccga )
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static const unsigned i86_address_mask = 0x000fffff;

static DEVICE_INPUT_DEFAULTS_START(cga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(vga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x00)
DEVICE_INPUT_DEFAULTS_END

static SLOT_INTERFACE_START(pc_isa8_cards)
	SLOT_INTERFACE("mda", ISA8_MDA)
	SLOT_INTERFACE("ega", ISA8_EGA)
	SLOT_INTERFACE("svga_et4k", ISA8_SVGA_ET4K)
	SLOT_INTERFACE("com", ISA8_COM)
	SLOT_INTERFACE("fdc", ISA8_FDC)
	SLOT_INTERFACE("finalchs", ISA8_FINALCHS)
	SLOT_INTERFACE("hdc", ISA8_HDC)
	SLOT_INTERFACE("adlib", ISA8_ADLIB)
	SLOT_INTERFACE("hercules", ISA8_HERCULES)
	SLOT_INTERFACE("gblaster", ISA8_GAME_BLASTER)
	SLOT_INTERFACE("sblaster1_0", ISA8_SOUND_BLASTER_1_0)
	SLOT_INTERFACE("sblaster1_5", ISA8_SOUND_BLASTER_1_5)
	SLOT_INTERFACE("mpu401", ISA8_MPU401)
	SLOT_INTERFACE("ne1000", NE1000)
	SLOT_INTERFACE("3c503", EL2_3C503)
	SLOT_INTERFACE("lpt", ISA8_LPT)
	SLOT_INTERFACE("ibm_mfc", ISA8_IBM_MFC)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( pcmda, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")

	/* video hardware */
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "mda", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "adlib", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pcherc, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "hercules", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "adlib", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, NULL, false)

	/* video hardware */
	MCFG_PALETTE_LENGTH( 256 )

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pccga, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_CONFIG(i86_address_mask)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(cga)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "com", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "hdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "sblaster1_0", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, NULL, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, NULL, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pcega, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_CONFIG(i86_address_mask)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(vga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "com", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "hdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "sblaster1_0", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, "ega", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, NULL, NULL, false)

	/* video hardware */
	MCFG_PALETTE_LENGTH( 256 )

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( xtvga, genpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc16_map)
	MCFG_CPU_IO_MAP(pc16_io)
	MCFG_CPU_CONFIG(i86_address_mask)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(vga)

	MCFG_ISA8_SLOT_ADD("mb:isa","isa1", pc_isa8_cards, "com", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa2", pc_isa8_cards, "fdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa3", pc_isa8_cards, "hdc", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa4", pc_isa8_cards, "sblaster1_0", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa5", pc_isa8_cards, "svga_et4k", NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa","isa6", pc_isa8_cards, NULL, NULL, false)

	/* video hardware */
	MCFG_PALETTE_LENGTH( 256 )

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


ROM_START( pcmda )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f))
ROM_END


ROM_START( pcherc )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f))
ROM_END

ROM_START( pcega )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("xtbios.bin",  0xfe000, 0x02000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987))
ROM_END

ROM_START( pc )
	ROM_REGION(0x100000,"maincpu", 0)
//  ROM_LOAD("xthdd.rom",  0xc8000, 0x02000, CRC(a96317da))
	ROM_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( xtvga )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME */
COMP(  1987,	pc,         ibm5150,	0,		pccga,		pccga, driver_device,		0,  		"<generic>",  "PC (CGA)" , GAME_NO_SOUND)
COMP(  1987,	pcega,      ibm5150,	0,		pcega,		pccga, driver_device,		0,			"<generic>",  "PC (EGA)" , GAME_NO_SOUND)
COMP ( 1987,	pcmda,      ibm5150,	0,		pcmda,      pcgen, driver_device,		0,  		"<generic>",  "PC (MDA)" , GAME_NO_SOUND)
COMP ( 1987,	pcherc,     ibm5150,	0,		pcherc,     pcgen, driver_device,      0,		"<generic>",  "PC (Hercules)" , GAME_NO_SOUND)
COMP ( 1987,	xtvga,      ibm5150,	0,		xtvga,      pcgen, driver_device,		0,			"<generic>",  "PC (VGA)" , GAME_NOT_WORKING | GAME_NO_SOUND)
