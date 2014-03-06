/***************************************************************************

    Incredible Technologies "Eagle" hardware

    skeleton by R. Belmont

    Known games on this hardware and their security chip IDs:
        * E2-LED0    (c) 2000     Golden Tee Fore!
        * E2-BBH0    (c) 2000     Big Buck Hunter
        * G42-US-U   (c) 2001     Golden Tee Fore! 2002
        * BB15-US    (c) 2002     Big Buck Hunter: Shooter's Challenge (AKA Big Buck Hunter v1.5)
        * BBH2-US    (c) 2002     Big Buck Hunter II: Sportsman's Paradise
        * CK1-US     (C) 2002     Carnival King
        * G43-US-U   (c) 2002     Golden Tee Fore! 2003
        * G44-US-U   (c) 2003     Golden Tee Fore! 2004
        * G45-US-U   (c) 2004     Golden Tee Fore! 2005
        * CW-US-U    (c) 2005     Big Buck Hunter: Call of the Wild
        * G4C-US-U   (c) 2006     Golden Tee Complete
        * ????????   (c) ????     Virtual Pool (not on IT's website master list but known to exist)

    Valid regions: US = USA, CAN = Canada, ENG = England, EUR = Euro, SWD = Sweden, AUS = Australia, NZ  = New Zealand, SA  = South Africa

    Note:
        Golden Tee Fore! 2004 & Golden Tee Fore! 2005 had updates called "Extra" that installed 2 additional courses.
         It's unknown if the Extra version required a different security chip or simply validation with IT's servers.
        There are "8-meg" versions of Big Buck Hunter: Call of the Wild to upgrade Eagle PCBs with only 8 megs of main
         memory. This was common for the ealier Big Buck Hunter series. The security chip is labeled CW-US-8

    Hardware overview:
        * NEC VR4310 CPU (similar to the N64's VR4300)
        * NEC VR4373 "Nile 3" system controller / PCI bridge
        * 3DFX Voodoo Banshee video
        * Creative/Ensoniq AudioPCI ES1373 audio
        * Atmel 90S2313 AVR-based microcontroller for protection
        * STM48T02 NVRAM
        * Conexant CX88168 modem

    TODO:
        * Everything (need new PCI subsystem to do this right)

***************************************************************************/

/*

Big Buck Hunter II
Incredible Technologies 2004

PCB Layout
----------

(main board sticker)
M/N:SC336
B/O:H1
Serial#:8434570
DOM:2003.05.02
IC:125A-0005
Multi-Tech Systems, Inc.
www.multitech.com

                                          |---------------------|
|-----------------------------------------|       JAMMA         |----|
|           PAL(E2-CARD1)                                            |-|
|                                                                    | |
|RJ45  4MHz  7.3728MHz                        DSW51(4)               | |VGA IN <--|
|                 AM85C30                                            |-|          |
|  ATMEL                     POWER_CONN                     |---|    |            |
| 90S2313                                  IDE40            |VGA-OUT---->---------|
| (BH2-AUS-U)     STM48T02                              |---|---|---||
|                                          IDE40        |14.31818MHz||
|RJ45        DSW5(4)                                    |48LC1M16A1 ||
|-------------|   48LC1M16A1         |-------|17S50APC  |48LC1M16A1 ||
|CONNEXANT    |   48LC1M16A1         |XILINX |(RED1.U26)|48LC1M16A1 ||
|SMART SCM/336|   48LC1M16A1         |SPARTAN|          |48LC1M16A1 ||
|CX88168-12   |   48LC1M16A1         |XC2550 |          |48LC1M16A1 ||
|             |   48LC1M16A1         |-------|          |48LC1M16A1 ||
|SMART DAA    |   48LC1M16A1                            |48LC1M16A1 ||
|20463-11     |   48LC1M16A1                            |48LC1M16A1 ||
|-------------|   48LC1M16A1                            ||--------| ||
|    |--------|  |-----------|              AT93C46.U61 ||BGA     | ||
|    |QFP120  |  |           |      27C080              ||WITH    | ||
|    |WITH    |  |NEC        |      (EAGLE U15 V2.08    ||HEATSINK| ||
|    |HEATSINK|  |UPD65907GC |      (C)2004 IT, INC)    ||--------| ||
|    |        |  |-012-NMU   |                          |-----------||
|    |        |  |VRC 4373   |                                       |
|    |--------|  |REV1.0     |                        CREATIVE       |
|PAL(E2-RE53)    |-----------|                        ES1373         |
|--------------------------------------------------------------------|

*/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/ataintf.h"
#include "video/voodoo.h"

class iteagle_state : public driver_device
{
public:
	iteagle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_voodoo(*this, "voodoo")
	{}

	required_device<cpu_device> m_maincpu;
	required_device<device_t>   m_voodoo;

	DECLARE_DRIVER_INIT(iteagle);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_WRITE_LINE_MEMBER(vblank_assert);
	UINT32 screen_update_iteagle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void machine_start();
};


/*************************************
 *
 *  Machine start
 *
 *************************************/

void iteagle_state::machine_start()
{
	/* set the fastest DRC options */
	mips3drc_set_options(m_maincpu, MIPS3DRC_FASTEST_OPTIONS);
}


/*************************************
 *
 *  Video refresh
 *
 *************************************/

UINT32 iteagle_state::screen_update_iteagle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return voodoo_update(m_voodoo, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

WRITE_LINE_MEMBER(iteagle_state::ide_interrupt)
{
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, iteagle_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM
	// Nile 3 northbridge/PCI controller at 0f000000
	AM_RANGE(0x1fc00000, 0x1fcfffff) AM_ROM AM_REGION("maincpu", 0) AM_SHARE("rombase")
ADDRESS_MAP_END

WRITE_LINE_MEMBER(iteagle_state::vblank_assert)
{
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( gtfore )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const voodoo_config iteagle_voodoo_intf =
{
	16, //              fbmem;
	0,//                tmumem0;
	0,//                tmumem1;
	"screen",//     screen;
	"maincpu",//            cputag;
	DEVCB_DRIVER_LINE_MEMBER(iteagle_state,vblank_assert),//    vblank;
	DEVCB_NULL//             stall;
};

static const mips3_config r4310_config =
{
	16384,              /* code cache size */
	16384               /* data cache size */
};

static MACHINE_CONFIG_START( gtfore, iteagle_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)
	MCFG_CPU_CONFIG(r4310_config)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", NULL, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(iteagle_state, ide_interrupt))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(iteagle_state, screen_update_iteagle)

	MCFG_3DFX_VOODOO_BANSHEE_ADD("voodoo", STD_VOODOO_BANSHEE_CLOCK, iteagle_voodoo_intf)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

#define EAGLE_BIOS \
	ROM_REGION( 0x100000, "maincpu", 0 ) /* MIPS code */ \
	ROM_SYSTEM_BIOS(  0, "209", "bootrom 2.09" ) \
	ROM_LOAD( "eagle209.u15", 0x000000, 0x100000, CRC(e0fc1a16) SHA1(c9524f7ee6b95bd484a3b75bcbe2243cb273f84c) ) \
	ROM_SYSTEM_BIOS(  1, "208", "bootrom 2.08" ) \
	ROM_LOAD( "eagle208.u15", 0x000000, 0x100000, CRC(772f2864) SHA1(085063a4e34f29ebe3814823cd2c6323a050da36) ) \
	ROM_SYSTEM_BIOS(  2, "204", "bootrom 2.04" ) \
	ROM_LOAD( "eagle204.u15", 0x000000, 0x100000, CRC(f02e5523) SHA1(b979cf72a6992f1ecad9695a08c8d51e315ab537) ) \
	ROM_SYSTEM_BIOS(  3, "201", "bootrom 2.01" ) \
	ROM_LOAD( "eagle201.u15", 0x000000, 0x100000, CRC(e180442b) SHA1(4f50821fed5bcd786d989520aa2559d6c416fb1f) ) \
	ROM_SYSTEM_BIOS(  4, "107", "bootrom 1.07" ) \
	ROM_LOAD( "eagle107.u15", 0x000000, 0x100000, CRC(97a01fc9) SHA1(a421dbf4d097b2f50cc005d3cd0d63e562e03df8) ) \
	ROM_SYSTEM_BIOS(  5, "106a", "bootrom 1.06a" ) \
	ROM_LOAD( "eagle106a.u15", 0x000000, 0x100000, CRC(9c79b7ad) SHA1(ccf1c86e79d65bee30f399e0fa33a7839570d93b) ) \
	ROM_SYSTEM_BIOS(  6, "106", "bootrom 1.06" ) \
	ROM_LOAD( "eagle106.u15", 0x000000, 0x100000, CRC(56bc193d) SHA1(e531d208ef27f777d0784414885f390d1be654b9) ) \
	ROM_SYSTEM_BIOS(  7, "105", "bootrom 1.05" ) \
	ROM_LOAD( "eagle105.u15", 0x000000, 0x100000, CRC(3870dbe0) SHA1(09be2d86c7259cd81d945c757044b167a76f30db) ) \
	ROM_SYSTEM_BIOS(  8, "103", "bootrom 1.03" ) \
	ROM_LOAD( "eagle103.u15", 0x000000, 0x100000, CRC(c35f4cf2) SHA1(45301c18c7f8f78754c8ad60ea4d2da5a7dc55fb) ) \
	ROM_SYSTEM_BIOS(  9, "102", "bootrom 1.02" ) \
	ROM_LOAD( "eagle102.u15", 0x000000, 0x100000, CRC(1fd39e73) SHA1(d1ac758f94defc5c55c62594b3999a406dd9ef1f) ) \
	ROM_SYSTEM_BIOS( 10, "101", "bootrom 1.01" ) \
	ROM_LOAD( "eagle101.u15", 0x000000, 0x100000, CRC(2600bc2b) SHA1(c4b89e69c51e4a3bb1874407c4d30b6caed4f396) ) \
	ROM_REGION( 0x30000, "fpga", 0 ) \
	ROM_LOAD( "17s20lpc_sb4.u26", 0x000000, 0x008000, CRC(62c4af8a) SHA1(6eca277b9c66a401990599e98fdca64a9e38cc9a) ) \
	ROM_LOAD( "17s20lpc_sb5.u26", 0x008000, 0x008000, CRC(c88b9d42) SHA1(b912d0fc50ecdc6a198c626f6e1644e8405fac6e) ) \
	ROM_LOAD( "17s50a_red1.u26", 0x010000, 0x020000, CRC(f5cf3187) SHA1(83b4a14de9959e5a776d97d424945d43501bda7f) ) \
	ROM_REGION( 0x2000, "pals", 0 ) \
	ROM_LOAD( "e2-card1.u22.jed", 0x000000, 0x000bd1, CRC(9d1e1ace) SHA1(287d6a30e9f32137ef4eba54f0effa092c97a6eb) ) \
	ROM_LOAD( "e2-res3.u117.jed", 0x001000, 0x000bd1, CRC(4f1ff45a) SHA1(213cbdd6cd37ad9b5bfc9545084892a68d29f5ff) )

/*
17s20lpc_sb4.u26 & 17s20lpc_sb5.u26 are alternate versions of configuration data for GREEN boards only.
17s50a_red1.u26 is configuration data for RED boards only.
*/

ROM_START( iteagle )
	EAGLE_BIOS

	DISK_REGION( "ata:0:hdd:image" )
ROM_END

ROM_START( gtfore02 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g42-us-u.u53", 0x0000, 0x0880, CRC(06e0b452) SHA1(f6b865799cb94941e0e77453b9d556d5988b0194) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "golf_fore_2002_v2.01.04_umv", 0, SHA1(e902b91bd739daee0b95b10e5cf33700dd63a76b) ) /* Labeled Golf Fore! V2.01.04 UMV */
ROM_END

ROM_START( gtfore02o )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g42-us-u.u53", 0x0000, 0x0880, CRC(06e0b452) SHA1(f6b865799cb94941e0e77453b9d556d5988b0194) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "golf_fore_2002_v2.00.00", 0, SHA1(d789ef86837a5012beb224c487537dd563d93886) ) /* Labeled Golf Fore! 2002 V2.00.00 */
ROM_END

ROM_START( carnking )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "ck1-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "carnival_king_v_1.00.11", 0, SHA1(c819af66d36df173ab17bf42f4045c7cca3203d8) ) /* Labeled Carnival King V 1.00.11 */
ROM_END

ROM_START( gtfore04 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g44-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "gt2004", 0, SHA1(739a52d6ce13bb6ac7a543ee0e8086fb66be19b9) )
ROM_END

ROM_START( gtfore05 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "gt2005", 0, SHA1(d8de569d8cf97b5aaada10ce896eb3c75f1b37f1) )
ROM_END

DRIVER_INIT_MEMBER(iteagle_state, iteagle)
{
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, iteagle,          0, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Eagle BIOS", GAME_IS_BIOS_ROOT )
GAME( 2001, gtfore02,   iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2002 (v2.01.04 UMV)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, gtfore02o, gtfore02, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2002 (v2.00.00)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, carnking,   iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Carnival King (v1.00.11)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2003, gtfore04,   iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2004", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2004, gtfore05,   iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005", GAME_NOT_WORKING | GAME_NO_SOUND )
