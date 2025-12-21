// license:BSD-3-Clause
// copyright-holders: R. Belmont, Ted Green
/***************************************************************************

    Incredible Technologies "Eagle" hardware

    by Ted Green & R. Belmont

    Known games on this hardware and their security chip IDs:
        * ITVP-1     (c) 1998     Virtual Pool
        * E2-LED0    (c) 2000     Golden Tee Fore!
        * E2-BBH0    (c) 2000     Big Buck Hunter
        * G42-US-U   (c) 2001     Golden Tee Fore! 2002
        * BB15-US    (c) 2002     Big Buck Hunter: Shooter's Challenge (AKA Big Buck Hunter v1.5)
        * BBH2-US    (c) 2002     Big Buck Hunter II: Sportsman's Paradise
        * CK1-US     (c) 2002     Carnival King
        * G43-US-U   (c) 2002     Golden Tee Fore! 2003
        * G44-US-U   (c) 2003     Golden Tee Fore! 2004
        * G45-US-U   (c) 2004     Golden Tee Fore! 2005
        * CW-US-U    (c) 2005     Big Buck Hunter: Call of the Wild
        * G4C-US-U   (c) 2006     Golden Tee Complete

    Valid regions: US = USA, CAN = Canada, ENG = England, EUR = Euro, SWD = Sweden, AUS = Australia, NZ  = New Zealand, SA  = South Africa

    Note:
        Golden Tee Fore! 2004 & Golden Tee Fore! 2005 had updates called "Extra" that installed 2 additional courses.
        There are "8-meg" versions of Big Buck Hunter: Call of the Wild to upgrade Eagle PCBs with only 8 megs of main
         memory. This was common for the earlier Big Buck Hunter series. The security chip is labeled CW-US-8

    Hardware overview:
        * NEC VR4310 CPU (similar to the N64's VR4300)
        * NEC VR4373 "Nile 3" system controller / PCI bridge
        * 3DFX Voodoo Banshee video
        * Creative/Ensoniq AudioPCI ES1373 audio
        * Atmel 90S2313 AVR-based microcontroller for protection
        * STM48T02 NVRAM
        * AMD AM85C30 Enhanced Serial Communications Controller
        * Conexant CX88168 modem
    Eagle 1 Notes:
        * Cypress CY82C693 Peripheral Controller
        * 3DFX Voodoo 1 video

    TODO:
        * Big buck hunter sportmans paradise and shooters challenge only work with secondary targeting (reduced threshold)

  Notes:
    Sound volume may be muted, it can be adjusted through the service menu or with volume up/down buttons (+/-)

    The PCB for Virtual Pool is considered "Eagle 1" while the boards
    that were production runs for later games are considered Eagle 2.
    IE: GT Fore! & BBH, both security chips are "E2-" as are various
    preprogrammed PALs: E2-CARD1 & E2-RES3

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
#include "iteagle_fpga.h"
#include "machine/pci-ide.h"
#include "screen.h"


namespace {

//*************************************
// Main iteagle driver
//*************************************
#define PCI_ID_NILE     "pci:00.0"
#define PCI_ID_PERIPH   "pci:06.0"
#define PCI_ID_IDE      "pci:06.1"
// Secondary IDE Control "pci:06.2"
#define PCI_ID_SOUND    "pci:07.0"
#define PCI_ID_FPGA     "pci:08.0"
#define PCI_ID_VIDEO    "pci:09.0"
#define PCI_ID_EEPROM   "pci:0a.0"

class iteagle_state : public driver_device
{
public:
	iteagle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fpga(*this, PCI_ID_FPGA)
		, m_eeprom(*this, PCI_ID_EEPROM)
	{}

	required_device<mips3_device> m_maincpu;
	required_device<iteagle_fpga_device> m_fpga;
	required_device<iteagle_eeprom_device> m_eeprom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void gtfore05(machine_config &config);
	void gtfore02(machine_config &config);
	void gtfore03(machine_config &config);
	void gtfore01(machine_config &config);
	void gtfore06(machine_config &config);
	void gtfore04(machine_config &config);
	void iteagle(machine_config &config);
	void bbh(machine_config &config);
	void bbhsc(machine_config &config);
	void bbhcotw(machine_config &config);
	void virtpool(machine_config &config);
	void carnking(machine_config &config);
	void bbh2sp(machine_config &config);
};

void iteagle_state::machine_start()
{
	// Setting MIPS3DRC_STRICT_VERIFY seems to eliminate the hangs in the bbh series
	m_maincpu->mips3drc_set_options(MIPS3DRC_STRICT_VERIFY);
}

void iteagle_state::machine_reset()
{
}

void iteagle_state::iteagle(machine_config &config)
{
	/* basic machine hardware */
	VR4310LE(config, m_maincpu, 133'333'333);
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(8192);
	m_maincpu->set_system_clock(66'666'666);

	PCI_ROOT(config, "pci", 0);

	vrc4373_device &vrc4373(VRC4373(config, PCI_ID_NILE, 0, m_maincpu));
	vrc4373.set_ram_size(0x00800000);
	vrc4373.set_simm0_size(0x02000000);

	ITEAGLE_PERIPH(config, PCI_ID_PERIPH, 0);
	IDE_PCI(config, PCI_ID_IDE, 0, 0x1080C693, 0x00, 0x0)
		.irq_handler().set_inputline(m_maincpu, MIPS3_IRQ2);

	ITEAGLE_FPGA(config, m_fpga, 0, "screen", m_maincpu, MIPS3_IRQ1, MIPS3_IRQ4);
	m_fpga->in_callback<iteagle_fpga_device::IO_SW5>().set_ioport("SW5");
	m_fpga->in_callback<iteagle_fpga_device::IO_IN1>().set_ioport("IN1");
	m_fpga->in_callback<iteagle_fpga_device::IO_SYSTEM>().set_ioport("SYSTEM");
	m_fpga->trackx_callback().set_ioport("TRACKX1");
	m_fpga->tracky_callback().set_ioport("TRACKY1");
	m_fpga->gunx_callback().set_ioport("GUNX1");
	m_fpga->guny_callback().set_ioport("GUNY1");

	es1373_device &pci_sound(ES1373(config, PCI_ID_SOUND, 0));
	pci_sound.add_route(0, PCI_ID_SOUND":speaker", 1.0, 0).add_route(1, PCI_ID_SOUND":speaker", 1.0, 1);
	pci_sound.irq_handler().set_inputline(m_maincpu, MIPS3_IRQ3);

	voodoo_3_pci_device &voodoo(VOODOO_3_PCI(config, PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(16);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status
	subdevice<generic_voodoo_device>(PCI_ID_VIDEO":voodoo")->vblank_callback().set(m_fpga, FUNC(iteagle_fpga_device::vblank_update));

	ITEAGLE_EEPROM(config, m_eeprom, 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(512, 384);
	screen.set_visarea(0, 512 - 1, 0, 384 - 1);
	screen.set_screen_update(PCI_ID_VIDEO, FUNC(voodoo_pci_device::screen_update));
}

void iteagle_state::gtfore01(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000401, 0x0b0b0b);
	m_eeprom->set_info(0x0401, 0x7);
}

void iteagle_state::gtfore02(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000402, 0x020201);
	m_eeprom->set_info(0x0402, 0x7);
}

void iteagle_state::gtfore03(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000403, 0x0a0b0a);
	m_eeprom->set_info(0x0403, 0x7);
}

void iteagle_state::gtfore04(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000404, 0x0a020b);
	m_eeprom->set_info(0x0404, 0x7);
}

void iteagle_state::gtfore05(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000405, 0x0b0a0c);
	m_eeprom->set_info(0x0405, 0x7);
}

void iteagle_state::gtfore06(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000406, 0x0c0b0d);
	m_eeprom->set_info(0x0406, 0x9);
}

void iteagle_state::carnking(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x01000a01, 0x0e0a0a);
	m_eeprom->set_info(0x0a01, 0x9);
}

void iteagle_state::bbh(machine_config &config)
{
	iteagle(config);
	// 0xXX01XXXX = tournament board
	m_fpga->set_init_info(0x02010600, 0x0b0a0a);
	m_eeprom->set_info(0x0000, 0x7);
}

void iteagle_state::bbhsc(machine_config &config)
{
	iteagle(config);
	// 0xXX01XXXX = tournament board
	m_fpga->set_init_info(0x02010600, 0x0c0a0a);
	m_eeprom->set_info(0x0000, 0x7);
}

void iteagle_state::bbh2sp(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x02010602, 0x0d0a0a);
	m_eeprom->set_info(0x0000, 0x7);
}

void iteagle_state::bbhcotw(machine_config &config)
{
	iteagle(config);
	m_fpga->set_init_info(0x02010603, 0x080704);
	m_eeprom->set_info(0x0603, 0x9);
}

void iteagle_state::virtpool(machine_config &config)
{
	iteagle(config);

	voodoo_1_pci_device &voodoo(VOODOO_1_PCI(config.replace(), PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(4);
	voodoo.set_tmumem(4, 4);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status

	m_fpga->set_init_info(0x01000202, 0x080808);
	m_eeprom->set_info(0x0202, 0x7);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/
static INPUT_PORTS_START( iteagle )

	PORT_START("SW5")
	PORT_DIPNAME( 0x3, 0x1, "Resolution" )
	PORT_DIPSETTING(0x1, "Medium" )
	PORT_DIPSETTING(0x0, "Low" )
	PORT_DIPSETTING(0x2, "Low_Alt" )
	PORT_DIPNAME(0x4, 0x0, "SW5-3")
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPNAME(0x8, 0x0, "SW5-4")
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPSETTING(0x8, DEF_STR(Off))

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Right" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Fly By" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Backspin" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Service" )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0010, 0x00, "Operator Mode" )
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPSETTING(0x10, DEF_STR(Yes))
	PORT_DIPNAME( 0x0020, 0x00, "SW51-2" )
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x00, "SW51-3" )
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPNAME(0x80, 0x00, "SW51-4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xC000, 0xC000, "Voltage" )
	PORT_DIPSETTING(0xC000, "OK" )
	PORT_DIPSETTING(0x8000, "Low" )
	PORT_DIPSETTING(0x4000, "High" )
	PORT_DIPSETTING(0x0000, "Not Detected" )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("GUNX1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X )
	PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_Y )
	PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

INPUT_PORTS_END

static INPUT_PORTS_START( virtpool )
	PORT_INCLUDE( iteagle )

	PORT_MODIFY("SW5")
	PORT_DIPNAME( 0x3, 0x1, "Resolution" )  // Setting to low resolution will hang the game
	PORT_DIPSETTING(0x1, "Medium" )
	PORT_DIPSETTING(0x0, "Low (Hangs)" )
	PORT_DIPSETTING(0x3, "VGA (Buggy)" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "English" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Aim" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Slop" )
	PORT_DIPNAME(0x0010, 0x00, "SW51-1")  // Turning this on freezes virtpool at boot
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPNAME(0x80, 0x00, "Operator Mode")
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPSETTING(0x80, DEF_STR(Yes))

INPUT_PORTS_END

static INPUT_PORTS_START( bbh )
	PORT_INCLUDE( iteagle )

	// bbhsc is low resolution only, bbhsc version 1.60.01 will set low resolution automatically
	PORT_MODIFY("SW5")
	PORT_DIPNAME(0x3, 0x0, "Resolution")
	PORT_DIPSETTING(0x1, "Medium" )
	PORT_DIPSETTING(0x0, "Low" )
	PORT_DIPSETTING(0x2, "Low_Alt" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Trigger" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Pump" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	// bbhsc and bbh2sp only work with seconday targeting
	PORT_DIPNAME(0x0020, 0x20, "Targeting")
	PORT_DIPSETTING(0x00, "Regular")
	PORT_DIPSETTING(0x20, "Secondary")

	PORT_MODIFY("GUNX1")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X )
	PORT_SENSITIVITY(50) PORT_KEYDELTA(24)
	PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_MODIFY("GUNY1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_Y )
	PORT_SENSITIVITY(50) PORT_KEYDELTA(12)
	PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

INPUT_PORTS_END

static INPUT_PORTS_START( bbh2 )
	PORT_INCLUDE( bbh )

	// Default bbh2sp and bbhcotw to medium resolution
	PORT_MODIFY("SW5")
	PORT_DIPNAME(0x3, 0x1, "Resolution")
	PORT_DIPSETTING(0x1, "Medium" )
	PORT_DIPSETTING(0x0, "Low" )
	PORT_DIPSETTING(0x2, "Low_Alt" )

	PORT_MODIFY("GUNX1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X )
	PORT_SENSITIVITY(50) PORT_KEYDELTA(12)
	PORT_CROSSHAIR(X, 1.0, 0.0, 0)
INPUT_PORTS_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/
#define EAGLE_BIOS \
	ROM_REGION( 0x100000, PCI_ID_NILE":rom", 0 ) /* MIPS code */ \
	ROM_SYSTEM_BIOS(  0, "209", "bootrom 2.09" ) \
	ROMX_LOAD( "eagle209.u15", 0x000000, 0x100000, CRC(e0fc1a16) SHA1(c9524f7ee6b95bd484a3b75bcbe2243cb273f84c), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS(  1, "208", "bootrom 2.08" ) \
	ROMX_LOAD( "eagle208.u15", 0x000000, 0x100000, CRC(772f2864) SHA1(085063a4e34f29ebe3814823cd2c6323a050da36), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS(  2, "204", "bootrom 2.04" ) \
	ROMX_LOAD( "eagle204.u15", 0x000000, 0x100000, CRC(f02e5523) SHA1(b979cf72a6992f1ecad9695a08c8d51e315ab537), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS(  3, "201", "bootrom 2.01" ) \
	ROMX_LOAD( "eagle201.u15", 0x000000, 0x100000, CRC(e180442b) SHA1(4f50821fed5bcd786d989520aa2559d6c416fb1f), ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS(  4, "107", "bootrom 1.07" ) \
	ROMX_LOAD( "eagle107.u15", 0x000000, 0x100000, CRC(97a01fc9) SHA1(a421dbf4d097b2f50cc005d3cd0d63e562e03df8), ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS(  5, "106a", "bootrom 1.06a" ) \
	ROMX_LOAD( "eagle106a.u15", 0x000000, 0x100000, CRC(9c79b7ad) SHA1(ccf1c86e79d65bee30f399e0fa33a7839570d93b), ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS(  6, "106", "bootrom 1.06" ) \
	ROMX_LOAD( "eagle106.u15", 0x000000, 0x100000, CRC(56bc193d) SHA1(e531d208ef27f777d0784414885f390d1be654b9), ROM_BIOS(6) ) \
	ROM_SYSTEM_BIOS(  7, "105", "bootrom 1.05" ) \
	ROMX_LOAD( "eagle105.u15", 0x000000, 0x100000, CRC(3870dbe0) SHA1(09be2d86c7259cd81d945c757044b167a76f30db), ROM_BIOS(7) ) \
	ROM_SYSTEM_BIOS(  8, "103", "bootrom 1.03" ) \
	ROMX_LOAD( "eagle103.u15", 0x000000, 0x100000, CRC(c35f4cf2) SHA1(45301c18c7f8f78754c8ad60ea4d2da5a7dc55fb), ROM_BIOS(8) ) \
	ROM_SYSTEM_BIOS(  9, "102", "bootrom 1.02" ) \
	ROMX_LOAD( "eagle102.u15", 0x000000, 0x100000, CRC(1fd39e73) SHA1(d1ac758f94defc5c55c62594b3999a406dd9ef1f), ROM_BIOS(9) ) \
	ROM_SYSTEM_BIOS( 10, "101", "bootrom 1.01" ) \
	ROMX_LOAD( "eagle101.u15", 0x000000, 0x100000, CRC(2600bc2b) SHA1(c4b89e69c51e4a3bb1874407c4d30b6caed4f396), ROM_BIOS(10) ) \
	ROM_REGION( 0x30000, "fpga", 0 ) \
	ROM_LOAD( "17s20lpc_sb4.u26", 0x000000, 0x008000, CRC(62c4af8a) SHA1(6eca277b9c66a401990599e98fdca64a9e38cc9a) ) \
	ROM_LOAD( "17s20lpc_sb5.u26", 0x008000, 0x008000, CRC(c88b9d42) SHA1(b912d0fc50ecdc6a198c626f6e1644e8405fac6e) ) \
	ROM_LOAD( "17s50a_red1.u26", 0x010000, 0x020000, CRC(f5cf3187) SHA1(83b4a14de9959e5a776d97d424945d43501bda7f) ) \
	ROM_REGION( 0x2000, "pals", 0 ) \
	ROM_LOAD( "e2-card1.u22.jed", 0x000000, 0x000bd1, CRC(9d1e1ace) SHA1(287d6a30e9f32137ef4eba54f0effa092c97a6eb) ) \
	ROM_LOAD( "e2-res3.u117.jed", 0x001000, 0x000bd1, CRC(4f1ff45a) SHA1(213cbdd6cd37ad9b5bfc9545084892a68d29f5ff) )


ROM_START( iteagle )
	EAGLE_BIOS

	//DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	//DISK_REGION( PCI_ID_IDE":ide2:1:cdrom" ) // program CD-ROM
ROM_END

ROM_START( virtpool ) /* On earlier Eagle 1 PCB, possibly a prototype version - later boards are known as Eagle 2 */
	ROM_REGION( 0x100000, PCI_ID_NILE":rom", 0 ) /* MIPS code */
	ROM_SYSTEM_BIOS( 0, "pool", "Virtual Pool bootrom" )
	ROMX_LOAD( "eagle1_bootrom_v1p01", 0x000000, 0x080000, CRC(6c8c1593) SHA1(707d5633388f8dd4e9252f4d8d6f27c98c2cb35a), ROM_BIOS(0) )

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "itvp-1.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "virtualpool", 0, SHA1(be8f890c33701ca17fab8112ee6cd7b5e435d8cf) ) /* HD hand labeled 3-1-99 V.P. */
ROM_END

ROM_START( carnking ) /* REQUIRES a "RED" board, will NOT work with earlier green boards */
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "ck1-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "carnival_king_v_1.00.11", 0, SHA1(c819af66d36df173ab17bf42f4045c7cca3203d8) ) /* Labeled Carnival King V 1.00.11 */
ROM_END

ROM_START( gtfore01 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "e2-led0.u53", 0x0000, 0x0880, CRC(6ec86dc6) SHA1(01665ad6d92d2b8e917e33ca705fab9258766513) )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_v1.00.25", 0, SHA1(6dc445b982aee3bab93ade5c4f5d148471939ecc) ) /* Build 19:19:59, Sep 11 2000 */
ROM_END

ROM_START( gtfore02 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g42-us-u.u53", 0x0000, 0x0880, CRC(06e0b452) SHA1(f6b865799cb94941e0e77453b9d556d5988b0194) )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2002_v2.01.06", 0, SHA1(d1363bc17337c91684148b76fa1e73ac9dd80d8f) ) /* Build 11:27:20, Nov  5 2001 */
ROM_END

ROM_START( gtfore03 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g43-us-u.u53", 0x0000, 0x0880, CRC(51c6f726) SHA1(9930337315128f89f7202893fb123ee3f0d33649) )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2003_v3.00.10", 0, SHA1(d789ef86837a5012beb224c487537dd563d93886) ) /* Build 09:36:45, Nov  7 2002 */
ROM_END

ROM_START( gtfore03a )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g43-us-u.u53", 0x0000, 0x0880, CRC(51c6f726) SHA1(9930337315128f89f7202893fb123ee3f0d33649) )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2003_v3.00.09", 0, SHA1(3c9cf82c3ad87b0d6b5a21089795abd8a08f8dd2) ) /* Build 09:36:45, Oct 17 2002 */
ROM_END

ROM_START( gtfore04 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g44-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2004_v4.00.08", 0, SHA1(739a52d6ce13bb6ac7a543ee0e8086fb66be19b9) ) /* Build 14:15:44, Aug 27 2003 - Has been upgraded to Extra */
ROM_END

ROM_START( gtfore04a )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g44-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2004_v4.00.00", 0, SHA1(fe7525de89d67e0e3d10c48572fd04382543c19f) ) /* Build 16:40:59, Feb 28 2003 */
ROM_END

ROM_START( gtfore05 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2005_v5.01.06", 0, SHA1(fa465263218d8e39102ec81d116c11447ef07e19) ) /* Build 10:55:49, Oct 27 2005 - Has been upgraded to Extra */
ROM_END

ROM_START( gtfore05a )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2005_v5.01.02", 0, SHA1(6e20d60fb7e9ab6bf0086267fa5b4329d8a9f468) ) /* Build 15:02:32, Feb 27 2004 - Has been upgraded to Extra */
ROM_END

ROM_START( gtfore05b )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2005_v5.01.00", 0, SHA1(d8de569d8cf97b5aaada10ce896eb3c75f1b37f1) ) /* Build 12:30:35, Feb 16 2004 - Has been upgraded to Extra */
ROM_END

ROM_START( gtfore05c )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g45-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_2005_v5.00.00", 0, SHA1(4236f57e639cae2e5a3eaa97fb24f5ff80557e84) ) /* Build 23:15:38, Jan 31 2004 - Has been upgraded to Extra */
ROM_END

ROM_START( gtfore06 )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "g4c-us-u.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "golf_fore_complete_v6.00.01", 0, SHA1(e902b91bd739daee0b95b10e5cf33700dd63a76b) ) /* Build 09:51:13, Jan 20 2006 */
ROM_END

ROM_START( bbh )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "e2-bbh0.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE("bbh_v1.00.14", 0, SHA1(dd56f758c3e421005e06ac24c21d12f0f29b0f44)) /* Build 10:59:51, Feb 25 2003 */
ROM_END

ROM_START( bbhsc )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "bb15-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE("bbhsc_v1.60.01", 0, SHA1(8554fdd7193ee27c0fe8ca921aa8db9c0378b313)) /* Build 09:50:13, May 29 2002 */
ROM_END

ROM_START( bbhsca )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "bb15-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "bbhsc_v1.50.07_cf", 0, SHA1(21dcf1f7e5ab901ac64e6afb099c35e273b3bf1f) ) /* Build 16:35:34, Feb 26 2002 - 4gb Compact Flash conversion */
ROM_END

ROM_START( bbh2sp )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "bbh2-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "bbh2sp_v2.02.11", 0, SHA1(63e41cca534f4774bfba4b4dda9620fe805029b4) ) /* Build 10:52:30, March 26, 2004 */
ROM_END

ROM_START( bbh2spa )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "bbh2-us.u53", 0x0000, 0x0880, NO_DUMP ) /* Build 18:07:45, Sept 15, 2003 */

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "bbh2sp_v2.02.09", 0, SHA1(fac3963b6da35a8c8b00f6826bc10e9c7230b1d6) ) /* Build 18:07:45, Sept 15, 2003 */
ROM_END

ROM_START( bbh2spb )
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "bbh2-us.u53", 0x0000, 0x0880, NO_DUMP )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "bbh2sp_v2.02.08", 0, SHA1(13b9b4ea0465f55dd1c7bc6e2f962c3c9b9566bd) ) /* Build 09:09:03, July 23, 2003 */
ROM_END

ROM_START( bbhcotw ) /* This version is meant for 8meg GREEN board PCBs */
	EAGLE_BIOS

	ROM_REGION( 0x0880, "atmel", 0 ) /* Atmel 90S2313 AVR internal CPU code */
	ROM_LOAD( "cw-us-8.u53", 0x0000, 0x0880, CRC(c5234b58) SHA1(fb47b2233147a3f633f01edebef9994c358bd162) )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "bbhcotw_v3.02.05_cf", 0, SHA1(b1fcaab3a5aa51821673a914333c8868d36f77ae) ) /* Build 21:00:39, Sep 10 2006 - 4gb Compact Flash conversion  */
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, iteagle,   0,        iteagle,  iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Eagle BIOS", MACHINE_IS_BIOS_ROOT )
GAME( 1998, virtpool,  iteagle,  virtpool, virtpool, iteagle_state, empty_init, ROT0, "Incredible Technologies", "Virtual Pool", MACHINE_SUPPORTS_SAVE )
GAME( 2002, carnking,  iteagle,  carnking, bbh2,     iteagle_state, empty_init, ROT0, "Incredible Technologies", "Carnival King (v1.00.11)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, gtfore01,  iteagle,  gtfore01, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! (v1.00.25)", MACHINE_SUPPORTS_SAVE )
GAME( 2001, gtfore02,  iteagle,  gtfore02, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2002 (v2.01.06)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, gtfore03,  iteagle,  gtfore03, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2003 (v3.00.10)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, gtfore03a, gtfore03, gtfore03, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2003 (v3.00.09)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, gtfore04,  iteagle,  gtfore04, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2004 Extra (v4.00.08)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, gtfore04a, gtfore04, gtfore04, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2004 (v4.00.00)", MACHINE_SUPPORTS_SAVE )
GAME( 2004, gtfore05,  iteagle,  gtfore05, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005 Extra (v5.01.06)", MACHINE_SUPPORTS_SAVE )
GAME( 2004, gtfore05a, gtfore05, gtfore05, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005 Extra (v5.01.02)", MACHINE_SUPPORTS_SAVE )
GAME( 2004, gtfore05b, gtfore05, gtfore05, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005 Extra (v5.01.00)", MACHINE_SUPPORTS_SAVE )
GAME( 2004, gtfore05c, gtfore05, gtfore05, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005 Extra (v5.00.00)", MACHINE_SUPPORTS_SAVE )
GAME( 2005, gtfore06,  iteagle,  gtfore06, iteagle,  iteagle_state, empty_init, ROT0, "Incredible Technologies", "Golden Tee Fore! 2006 Complete (v6.00.01)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, bbh,       iteagle,  bbh,      bbh,      iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter (v1.00.14)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, bbhsc,     iteagle,  bbhsc,    bbh,      iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter - Shooter's Challenge (v1.60.01)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, bbhsca,    bbhsc,    bbhsc,    bbh,      iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter - Shooter's Challenge (v1.50.07)", MACHINE_SUPPORTS_SAVE )
GAME( 2004, bbh2sp,    iteagle,  bbh2sp,   bbh2,     iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter II - Sportsman's Paradise (v2.02.11)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, bbh2spa,   bbh2sp,   bbh2sp,   bbh2,     iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter II - Sportsman's Paradise (v2.02.09)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, bbh2spb,   bbh2sp,   bbh2sp,   bbh2,     iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter II - Sportsman's Paradise (v2.02.08)", MACHINE_SUPPORTS_SAVE )
GAME( 2006, bbhcotw,   iteagle,  bbhcotw,  bbh2,     iteagle_state, empty_init, ROT0, "Incredible Technologies", "Big Buck Hunter Call of the Wild (v3.02.5)", MACHINE_SUPPORTS_SAVE )
