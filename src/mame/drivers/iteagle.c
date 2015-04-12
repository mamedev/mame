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
#include "machine/pci.h"
#include "machine/vrc4373.h"
#include "video/voodoo_pci.h"
#include "sound/es1373.h"
#include "machine/iteagle_fpga.h"


//*************************************
// Main iteagle driver
//*************************************
class iteagle_state : public driver_device
{
public:
	iteagle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{}

	required_device<mips3_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();
};

void iteagle_state::machine_start()
{
	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);

	/* configure fast RAM regions for DRC */
	//m_maincpu->mips3drc_add_fastram(0x00000000, 16*1024*1024-1, FALSE, m_rambase);
	//m_maincpu->mips3drc_add_fastram(0x1fc00000, 0x1fc7ffff, TRUE, m_rombase);
}

void iteagle_state::machine_reset()
{
}

static MACHINE_CONFIG_START( gtfore, iteagle_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_VRC4373_ADD(                 ":pci:00.0", ":maincpu")
	MCFG_ITEAGLE_FPGA_ADD(            ":pci:06.0")
	MCFG_ITEAGLE_IDE_ADD(             ":pci:06.1")
	MCFG_ITEAGLE_IDE_IRQ_ADD(         ":maincpu", MIPS3_IRQ2)
	MCFG_ES1373_ADD(                  ":pci:07.0")
	MCFG_SOUND_ROUTE(0, ":pci:07.0:lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, ":pci:07.0:rspeaker", 1.0)
	MCFG_ES1373_IRQ_ADD(              ":maincpu", MIPS3_IRQ3)
	MCFG_VOODOO_ADD(                  ":pci:09.0", ":maincpu")
	MCFG_ITEAGLE_EEPROM_ADD(          ":pci:0a.0")


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_UPDATE_DEVICE(":pci:09.0", voodoo_pci_device, screen_update)


MACHINE_CONFIG_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/
static INPUT_PORTS_START( iteagle )

	PORT_START("SW5")
	PORT_DIPNAME( 0xf, 0x1, "Resolution" )
	PORT_DIPSETTING(0x1, "Medium" )
	PORT_DIPSETTING(0x0, "Low" )
	PORT_DIPSETTING(0x2, "Low_Alt" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Right" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Fly By" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Backspin" )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x00f0, 0x00, "SW51" )
	PORT_DIPSETTING(0x00, "Normal" )
	PORT_DIPSETTING(0x10, "Operator Mode" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x3000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0xC000, 0xC000, "Voltage" )
	PORT_DIPSETTING(0xC000, "OK" )
	PORT_DIPSETTING(0x8000, "Low" )
	PORT_DIPSETTING(0x4000, "High" )
	PORT_DIPSETTING(0x0000, "Not Detected" )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("VERSION")
	PORT_DIPNAME( 0x0F00, 0x0000, "GAME" )
	PORT_DIPNAME( 0x00F0, 0x0000, "MAJOR" )
	PORT_DIPNAME( 0x000F, 0x0000, "MINOR" )

INPUT_PORTS_END

static INPUT_PORTS_START( gtfore05 )
	PORT_INCLUDE(iteagle)

	PORT_MODIFY("VERSION")
	PORT_DIPNAME( 0x0F00, 0x0400, "GAME" )
	PORT_DIPNAME( 0x00F0, 0x0050, "MAJOR" )
	PORT_DIPNAME( 0x000F, 0x0001, "MINOR" )

INPUT_PORTS_END

static INPUT_PORTS_START( gtfore04 )
	PORT_INCLUDE(iteagle)

	PORT_MODIFY("VERSION")
	PORT_DIPNAME( 0x0F00, 0x0400, "GAME" )
	PORT_DIPNAME( 0x00F0, 0x0040, "MAJOR" )
	PORT_DIPNAME( 0x000F, 0x0000, "MINOR" )

INPUT_PORTS_END

static INPUT_PORTS_START( gtfore02 )
	PORT_INCLUDE(iteagle)

	PORT_MODIFY("VERSION")
	PORT_DIPNAME( 0x0F00, 0x0400, "GAME" )
	PORT_DIPNAME( 0x00F0, 0x0030, "MAJOR" )
	PORT_DIPNAME( 0x000F, 0x0000, "MINOR" )

INPUT_PORTS_END

static INPUT_PORTS_START( gtfore06 )
	PORT_INCLUDE(iteagle)

	PORT_MODIFY("VERSION")
	PORT_DIPNAME( 0x0F00, 0x0400, "GAME" )
	PORT_DIPNAME( 0x00F0, 0x0060, "MAJOR" )
	PORT_DIPNAME( 0x000F, 0x0000, "MINOR" )

INPUT_PORTS_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/
#define EAGLE_BIOS \
	ROM_REGION( 0x100000, ":pci:00.0", 0 ) /* MIPS code */ \
	ROM_SYSTEM_BIOS(  0, "209", "bootrom 2.09" ) \
	ROMX_LOAD( "eagle209.u15", 0x000000, 0x100000, CRC(e0fc1a16) SHA1(c9524f7ee6b95bd484a3b75bcbe2243cb273f84c), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS(  1, "208", "bootrom 2.08" ) \
	ROMX_LOAD( "eagle208.u15", 0x000000, 0x100000, CRC(772f2864) SHA1(085063a4e34f29ebe3814823cd2c6323a050da36), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS(  2, "204", "bootrom 2.04" ) \
	ROMX_LOAD( "eagle204.u15", 0x000000, 0x100000, CRC(f02e5523) SHA1(b979cf72a6992f1ecad9695a08c8d51e315ab537), ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS(  3, "201", "bootrom 2.01" ) \
	ROMX_LOAD( "eagle201.u15", 0x000000, 0x100000, CRC(e180442b) SHA1(4f50821fed5bcd786d989520aa2559d6c416fb1f), ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS(  4, "107", "bootrom 1.07" ) \
	ROMX_LOAD( "eagle107.u15", 0x000000, 0x100000, CRC(97a01fc9) SHA1(a421dbf4d097b2f50cc005d3cd0d63e562e03df8), ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS(  5, "106a", "bootrom 1.06a" ) \
	ROMX_LOAD( "eagle106a.u15", 0x000000, 0x100000, CRC(9c79b7ad) SHA1(ccf1c86e79d65bee30f399e0fa33a7839570d93b), ROM_BIOS(6) ) \
	ROM_SYSTEM_BIOS(  6, "106", "bootrom 1.06" ) \
	ROMX_LOAD( "eagle106.u15", 0x000000, 0x100000, CRC(56bc193d) SHA1(e531d208ef27f777d0784414885f390d1be654b9), ROM_BIOS(7) ) \
	ROM_SYSTEM_BIOS(  7, "105", "bootrom 1.05" ) \
	ROMX_LOAD( "eagle105.u15", 0x000000, 0x100000, CRC(3870dbe0) SHA1(09be2d86c7259cd81d945c757044b167a76f30db), ROM_BIOS(8) ) \
	ROM_SYSTEM_BIOS(  8, "103", "bootrom 1.03" ) \
	ROMX_LOAD( "eagle103.u15", 0x000000, 0x100000, CRC(c35f4cf2) SHA1(45301c18c7f8f78754c8ad60ea4d2da5a7dc55fb), ROM_BIOS(9) ) \
	ROM_SYSTEM_BIOS(  9, "102", "bootrom 1.02" ) \
	ROMX_LOAD( "eagle102.u15", 0x000000, 0x100000, CRC(1fd39e73) SHA1(d1ac758f94defc5c55c62594b3999a406dd9ef1f), ROM_BIOS(10) ) \
	ROM_SYSTEM_BIOS( 10, "101", "bootrom 1.01" ) \
	ROMX_LOAD( "eagle101.u15", 0x000000, 0x100000, CRC(2600bc2b) SHA1(c4b89e69c51e4a3bb1874407c4d30b6caed4f396), ROM_BIOS(11) ) \
	ROM_REGION( 0x30000, "fpga", 0 ) \
	ROM_LOAD( "17s20lpc_sb4.u26", 0x000000, 0x008000, CRC(62c4af8a) SHA1(6eca277b9c66a401990599e98fdca64a9e38cc9a) ) \
	ROM_LOAD( "17s20lpc_sb5.u26", 0x008000, 0x008000, CRC(c88b9d42) SHA1(b912d0fc50ecdc6a198c626f6e1644e8405fac6e) ) \
	ROM_LOAD( "17s50a_red1.u26", 0x010000, 0x020000, CRC(f5cf3187) SHA1(83b4a14de9959e5a776d97d424945d43501bda7f) ) \
	ROM_REGION( 0x2000, "pals", 0 ) \
	ROM_LOAD( "e2-card1.u22.jed", 0x000000, 0x000bd1, CRC(9d1e1ace) SHA1(287d6a30e9f32137ef4eba54f0effa092c97a6eb) ) \
	ROM_LOAD( "e2-res3.u117.jed", 0x001000, 0x000bd1, CRC(4f1ff45a) SHA1(213cbdd6cd37ad9b5bfc9545084892a68d29f5ff) )


ROM_START( iteagle )
	EAGLE_BIOS

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
ROM_END
ROM_START( gtfore06 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g42-us-u.u53", 0x0000, 0x0880, CRC(06e0b452) SHA1(f6b865799cb94941e0e77453b9d556d5988b0194) )

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
	DISK_IMAGE( "golf_fore_2002_v2.01.04_umv", 0, SHA1(e902b91bd739daee0b95b10e5cf33700dd63a76b) ) /* Labeled Golf Fore! V2.01.04 UMV */
	//DISK_REGION( "ide:1:cdrom" ) // program CD-ROM

ROM_END

ROM_START( gtfore02 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g42-us-u.u53", 0x0000, 0x0880, CRC(06e0b452) SHA1(f6b865799cb94941e0e77453b9d556d5988b0194) )

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
	DISK_IMAGE( "golf_fore_2002_v2.00.00", 0, SHA1(d789ef86837a5012beb224c487537dd563d93886) ) /* Labeled Golf Fore! 2002 V2.00.00 */
ROM_END

ROM_START( carnking )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "ck1-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
	DISK_IMAGE( "carnival_king_v_1.00.11", 0, SHA1(c819af66d36df173ab17bf42f4045c7cca3203d8) ) /* Labeled Carnival King V 1.00.11 */
ROM_END

ROM_START( gtfore04 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g44-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
	DISK_IMAGE( "gt2004", 0, SHA1(739a52d6ce13bb6ac7a543ee0e8086fb66be19b9) )
ROM_END

ROM_START( gtfore05 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( ":pci:06.1:ide2:0:hdd:image" )
	DISK_IMAGE( "gt2005", 0, SHA1(d8de569d8cf97b5aaada10ce896eb3c75f1b37f1) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, iteagle,          0, gtfore, iteagle,   driver_device, 0, ROT0, "Incredible Technologies", "Eagle BIOS", GAME_IS_BIOS_ROOT )
GAME( 2001, gtfore02,   iteagle, gtfore, gtfore02,  driver_device, 0, ROT0, "Incredible Technologies", "Golden Tee Fore! 2002 (v2.00.00)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, carnking,   iteagle, gtfore, iteagle,   driver_device, 0, ROT0, "Incredible Technologies", "Carnival King (v1.00.11)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2003, gtfore04,   iteagle, gtfore, gtfore04,  driver_device, 0, ROT0, "Incredible Technologies", "Golden Tee Fore! 2004", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2004, gtfore05,   iteagle, gtfore, gtfore05,  driver_device, 0, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2005, gtfore06,   iteagle, gtfore, gtfore06,  driver_device, 0, ROT0, "Incredible Technologies", "Golden Tee Fore! 2006 Complete", GAME_NOT_WORKING | GAME_NO_SOUND )
