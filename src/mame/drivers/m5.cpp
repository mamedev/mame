// license:BSD-3-Clause
// copyright-holders:Curt Coder, Ales Dlabac
/***************************************************************************

    Sord m.5

    http://www.retropc.net/mm/m5/
    http://www.museo8bits.es/wiki/index.php/Sord_M5 not working
    http://k5.web.klfree.net/content/view/10/11/ not working
    http://k5.web.klfree.net/images/stories/sord/m5heap.htm  not working
    http://k5.klfree.net/index.php?option=com_content&task=view&id=5&Itemid=3
    http://k5.klfree.net/index.php?option=com_content&task=view&id=10&Itemid=11
    http://k5.klfree.net/index.php?option=com_content&task=view&id=14&Itemid=3
    http://www.dlabi.cz/?s=sord
    https://www.facebook.com/groups/59667560188/
    http://www.oldcomp.cz/viewtopic.php?f=103&t=1164

****************************************************************************/

/***************************************************************************

TODO:

    - fd5 floppy
    - SI-5 serial interface (8251, ROM)
    - ramdisk for KRX Memory expansion
    - rewrite fd5 floppy as unpluggable device
    - move dipswitch declaration to softwarelist file?
    - 64krx: get windows ROM version with cpm & ramdisk support (Stuchlik S.E.I. version)

    - brno mod: make the dsk image writeable
    - brno mod: in console version lost data on RAMDISK after soft reset
    - brno mod: add support for lzr floppy disc format
    - brno mod: include basic-i



CHANGELOG:

10.02.2016
    - fixed bug: crash if rom card was only cart
    - fixed bug: when em-5 selected monitor rom wasn't paged in
    - brno mod: spin motor on upon restart
    - brno mod: windowed boot as default rom
    - brno mod: fixed bug: tape command in menu now works

05.02.2016
    - added BRNO modification - 1024kB Ramdisk + CP/M support
    - 32/64KB RAM expansions EM-5, 64KBI, 64KBF, 64KRX
    - since now own version of rom and slot handlers
    - 2 slots for carts


******************************************************************************


Controlling (paging) of homebrew 64KB RAM carts
================================================

Used ports:
EM-64, 64KBI:   OUT 6CH,00H - enables ROM
                OUT 6CH,01H - enables RAM
64KBF:          OUT 30H,00000xxxB   - enables RAM or ROM, see bellow
64KRD, 64KRX:   OUT 7FH,00000000B   - enables RAM
                OUT 7FH,11111111B   - enables ROM
                OUT 7FH,xxxxxxxxB   - enables RAM and ROM, see bellow

===========================================================================================================================

RAM/ROM modes of EM-64/64KBI cart
------------------------------------------
mode 0: 0x0000-0x6fff ROM 0x7000-0xffff RAM (it is possible to limit actual ROM size by DIP switch only to 32kb)
mode 1: 0x0000-0xffff RAM

===========================================================================================================================

RAM/ROM modes of 64KBF version 2C cart
------------------------------------------
Memory paging is done by using "OUT &30,mod".

MODE    READ                            WRITE
----------------------------------------------------------------------
 00 8 KB MON + 20 KB BF + 36 KB RAM     28 KB DIS + 36 KB RAM
 01 64 KB RAM                           64 KB RAM
 02 8 KB MON + 56 KB RAM                64 KB RAM
 03 64 KB RAM                           28 KB DIS + 36 KB RAM
 04 64 KB RAM                           16 KB DIS + 48 KB RAM
 05 8 KB MON + 20 KB BF + 36 KB RAM     64 KB RAM
 06 8 KB MON + 20 KB DIS + 36 KB RAM    64 KB RAM
 07 64 KB DIS                           64 KB DIS

Version LZR ( 2C )
================

+------------+
|////////////|  READ ONLY AREA
+------------+
|\\\\\\\\\\\\|  WRITE ONLY AREA
+------------+
|XXXXXXXXXXXX|  R&W AREA
+------------+
|            |  DISABLED R&W
+------------+

      0   0   0   1   1   2   2   2   3   3   4   4   4   5   5   6   6
kB    0   4   8   2   6   0   4   8   2   6   0   4   8   2   6   0   4
      +-------+-------------------+
ROM   |MONITOR|      BASIC-F      |
      +-------+-------+-------+---+---+-------+-------+-------+-------+
RAM   |       |       |       |       |       |       |       |       |
      +-------+-------+-------+-------+-------+-------+-------+-------+
CART  |       |       |       |       |       |       |       |       |
      +-------+-------+-------+-------+-------+-------+-------+-------+


Mode
    +-------+-------------------+
    |///////|///////////////////|
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M0  |       |       |       |   |XXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |       |                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M1  |XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |///////|                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M2  |\\\\\\\|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |       |                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M3  |///////|///////|///////|///|XXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |       |                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M4  |///////|///////|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |///////|///////////////////|
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M5  |\\\\\\\|\\\\\\\|\\\\\\\|\\\|XXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+-------+-------+-------+-------+-------+

    +-------+-------------------+
    |///////|                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M6  |\\\\\\\|\\\\\\\|\\\\\\\|\\\|XXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +-------+-------+-------+---+---+-------+-------+-------+-------+
            |///////|///////|///|
            +-------+-------+---+

    +-------+-------------------+
    |       |                   |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
M7  |       |       |       |       |       |       |       |       |
    +-------+-------+-------+-------+-------+-------+-------+-------+
    |XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|XXXXXXX|
    +---------------------------------------------------------------+

===========================================================================================================

Memory map of ROM and RAM in configuration SORD M5 + 64 KRX memory cart
-----------------------------------------------------------------------

         cart     inside Sord   inside Sord     cart        cart
FFFF +----------+ +----------+ +----------+ +----------+ +----------+
     |          |                           |          | |          |
     |          |                           | EPROM 16K| | EPROM 16K|
     |          |                           |        5 | |        7 |
C000 |   DRAM   |                           +----------+ +----------+
     |          |                           |          | |          |
     |          |                           | EPROM 16K| | EPROM 16K|
     |          |                           |        4 | |        6 |
7FFF +----------+ +----------+              +----------+ +----------+
                  |   SRAM   |
7000 +----------+ +----------+
     |          |
6000 |          |                           +----------+
     |          |                           |          |
5000 |          |                           | EPROM 8K |
     |          |                           |        3 |
4000 |          |              +----------+ +----------+
     |          |              |          |
3000 |   DRAM   |              | EPROM 8K |
     |          |              |        2 |
2000 |          |              +----------+
     |          |              |          |
1000 |          |              | EPROM 8K |
     |          |              |        1 |
0000 +----------+ +----------+ +----------+ +----------+ +----------+

1 - MONITOR ROM
2 - WINDOWS + BASIC-F 3rd part
3 - BASIC-I
4 - 2nd part of BASIC-F + 1st part of BASIC-F
5 - 1st part of BASIC-G + 2nd part of BASIC-G
6 - 1st part of MSX 1.C
7 - 2nd part of MSX 1.C

Note: position 3 could be replaced with SRAM 8KB with battery power backup!

Upon powering up either SRAM + 1,2,3,4,5 or SRAM + 1,2,3,6,7 are selected.
Switching between 4,5 and 6,7 is provided by hw switch, selecting ROM/RAM mode happens
using OUT (7FH),A, where each bit of A means 8KB memory chunk ( state: 0=RAM,
1=ROM, bit: 0=1, 1=2, 2=3, 3=always SRAM, 4=4, 5=5, 6=6, 7=7 ).


*/

/*
*************************************************************
*                       BRNO MOD                            *
*************************************************************
HW and SW was originally created by Pavel Brychta with help of Jiri Kubin and L. Novak
This driver mod was implemented by Ales Dlabac with great help of Pavel Brychta. Without him this would never happen
This mod exists in two versions. First one is "windows"(brno_rom12.rom) version and was created by Ladislav Novak.
Second version version is "pure text" and was created by Pavel Brychta and Jiri Kubin

Function:
Whole Sord's address area (0000-FFFF) is divided to 16 4kB banks. To this 16 banks
you can map any of possible 256 ramdisc blocks what allows user to have 1024kB large ramdisc.
Of course to be able to realise this is necessary page out all roms

As pagination port MMU(page select) is used.
For RAM write protection port CASEN is used. 0=access to ramdisk enabled, 0xff=ramdisk access disabled(data protection), &80=ROM2+48k RAM, &81=ROM2+4k RAM(this is not implemented)
For ROM page out port RAMEN is used. 0=rom enable; 0xff=rom+sord ram disabled (ramdisk visible)

SORD M5 RAM memory map in address area 7000H-7FFFH
7000H     7300H                   7800H                        7E00H     7FFFH
  +---------+-----------------------+----------------------------+---------+
  |    a.   |                       |            c.              |   d.    |

a. SORD system variables and stack
c. Area where the first sector of 1st track is loaded, simultaneously is reserved for Hook program
d. Reserved for memory tests and ramdisk mapping(pagination). After boot is used as buffer for cursor position,
   type of floppy and so on. Area consists of:

7FFFH .... bootloader version
7FFEH .... identification byte of floppy - is transferred from EPROM, it might be changed by SETUP
7FFDH .... number of last Ramdisk segment of RAM
7FFBH .... address of cursor in VRAM in 40 columns CRT. For 80 columns CRT both bytes are zero
7FF9H .... X,Y cursor actual position for 40 columns CRTs. In case of 80 columns CRT both bytes are zero
7203H .... Actual memory bank buffer

System floppy disk header on track 00 of 1st sector
         byte 0-1  ... system disk identification SY
         byte 2    ... # of physical sectors for BIOS or DOS plus # of segments for DIR
         byte 3-4  ... Start address for loading of BIOS or DOS
         byte 5    ... # of bytes for possible HOOK program
         byte 6-   ... HOOK program, or either BIOS or DOS

In case of HOOK, bytes 8 and 9 contains characters 'H' and 'O' for HOOK testing

Few other notes:
 Ramdisc warm boot is provided by pressing Ctrl+C


 Floppy formats as follows:

 A: Ramdisk 1024kB, 8 sectors,
 B: Floppy format "Heat Magnolia" SingleSide SingleDensity , 40 tracks, 9 sectors, 512  sec. length, 128 dirs, offset 3, 166kB
 C: Floppy format "Robotron aka PC1715", DS DD,              80 tracks, 5 sectors, 1024 sec. length, 128 dirs, offset 2, 780kB

**********************************************************************************************************************************/



#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/m5_dsk.h"
#include "formats/sord_cas.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h" //brno mod
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "bus/m5/slot.h"
#include "bus/m5/rom.h"
#include "softlist.h"
#include "includes/m5.h"




//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

WRITE_LINE_MEMBER( m5_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

//-------------------------------------------------
//  sts_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::sts_r )
{
	/*

	    bit     description

	    0       cassette input
	    1       busy
	    2
	    3
	    4
	    5
	    6
	    7       RESET key

	*/

	UINT8 data = 0;

	// cassette input
	data |= m_cassette->input() >= 0 ? 1 : 0;

	// centronics busy
	data |= m_centronics_busy << 1;

	// RESET key
	data |= m_reset->read();

	return data;
}


//-------------------------------------------------
//  com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::com_w )
{
	/*

	    bit     description

	    0       cassette output, centronics strobe
	    1       cassette remote
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	// cassette output
	m_cassette->output( BIT(data, 0) ? -1.0 : 1.0);

	// centronics strobe
	m_centronics->write_strobe(BIT(data, 0));

	// cassette remote
	m_cassette->change_state(BIT(data,1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}



//**************************************************************************
//  FD-5
//**************************************************************************

//-------------------------------------------------
//  fd5_data_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::fd5_data_r )
{
	m_ppi->pc6_w(0);

	return m_fd5_data;
}


//-------------------------------------------------
//  fd5_data_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_data_w )
{
	m_fd5_data = data;

	m_ppi->pc4_w(0);
}


//-------------------------------------------------
//  fd5_com_r -
//-------------------------------------------------

READ8_MEMBER( m5_state::fd5_com_r )
{
	/*

	    bit     description

	    0       ?
	    1       1?
	    2       IBFA?
	    3       OBFA?
	    4
	    5
	    6
	    7

	*/

	return m_obfa << 3 | m_ibfa << 2 | 0x02;
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_com_w )
{
	/*

	    bit     description

	    0       PPI PC2
	    1       PPI PC0
	    2       PPI PC1
	    3
	    4
	    5
	    6
	    7

	*/

	m_fd5_com = data;
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_ctrl_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	m_floppy0->mon_w(!BIT(data, 0));
}


//-------------------------------------------------
//  fd5_com_w -
//-------------------------------------------------

WRITE8_MEMBER( m5_state::fd5_tc_w )
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}

//**************************************************************************
//  64KBI support for oldest memory module
//**************************************************************************

READ8_MEMBER( m5_state::mem64KBI_r ) //in 0x6c
{
	return BIT(m_ram_mode, 0);
}

WRITE8_MEMBER( m5_state::mem64KBI_w ) //out 0x6c
{
	if (m_ram_type != MEM64KBI) return;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart_ram->tag()).append(M5SLOT_ROM_REGION_TAG).c_str());
	memory_region *ram_region=memregion(region_tag.assign(m_cart_ram->tag()).append(":ram").c_str());

	if (m_ram_mode == BIT(data, 0))
		return;

	m_ram_mode = BIT(data, 0);

	//if 32kb only mode don't map top ram
	if (m_ram_mode && (m_DIPS->read() & 4) != 4)
	{
		program.install_ram(0x0000, 0x6fff, ram_region->base());
	}
	else
	{
		program.install_rom(0x0000, 0x1fff, memregion(Z80_TAG)->base());
		program.unmap_write(0x0000, 0x1fff);

		//if AUTOSTART is on don't load any ROM cart
		if (m_cart && (m_DIPS->read() & 2) != 2)
		{
			program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom), (m5_cart_slot_device*)m_cart)); //m_cart pointer to rom cart
			program.unmap_write(0x2000, 0x3fff);
		}
		else
			program.unmap_readwrite(0x2000, 0x3fff);
	}

	logerror("64KBI: ROM %s", m_ram_mode == 0 ? "enabled\n" : "disabled\n");
}

//**************************************************************************
//  64KBF paging
//**************************************************************************

WRITE8_MEMBER( m5_state::mem64KBF_w ) //out 0x30
{
	if (m_ram_type != MEM64KBF) return;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart_ram->tag()).append(M5SLOT_ROM_REGION_TAG).c_str()); //ROM region of the cart
	memory_region *ram_region=memregion(region_tag.assign(m_cart_ram->tag()).append(":ram").c_str());   //RAM region of the cart
	memory_region *rom_region=memregion(region_tag.assign(m_cart->tag()).append(M5SLOT_ROM_REGION_TAG).c_str()); //region where clasic ROM cartridge resides

	if (m_ram_mode == data)
		return;

	m_ram_mode = data;

	switch(m_ram_mode)
	{
		case 0:
			program.unmap_write(0x0000, 0x6fff);
			membank("bank1r")->set_base(memregion(Z80_TAG)->base());
			membank("bank2r")->set_base(m_cart_rom->base());
			membank("bank3r")->set_base(m_cart_rom->base()+0x2000);
			membank("bank4r")->set_base(m_cart_rom->base()+0x4000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 1:
			program.install_write_bank(0x0000,0x1fff,"bank1w");
			program.install_write_bank(0x2000,0x3fff,"bank2w");
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			membank("bank1r")->set_base(ram_region->base()+0x0000);     membank("bank1w")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(ram_region->base()+0x2000);     membank("bank2w")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(ram_region->base()+0x4000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(ram_region->base()+0x6000);     membank("bank4w")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 2:
			program.install_write_bank(0x0000,0x1fff,"bank1w");
			program.install_write_bank(0x2000,0x3fff,"bank2w");
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			membank("bank1r")->set_base(memregion(Z80_TAG)->base());    membank("bank1w")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(ram_region->base()+0x2000);     membank("bank2w")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(ram_region->base()+0x4000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(ram_region->base()+0x6000);     membank("bank4w")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 3:
			program.unmap_write(0x0000, 0x6fff);
			membank("bank1r")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 4:
			program.unmap_write(0x0000, 0x3fff);
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			membank("bank1r")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(ram_region->base()+0x4000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(ram_region->base()+0x6000);     membank("bank4w")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 5:
			program.install_write_bank(0x0000,0x1fff,"bank1w");
			program.install_write_bank(0x2000,0x3fff,"bank2w");
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			membank("bank1r")->set_base(memregion(Z80_TAG)->base());    membank("bank1w")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(m_cart_rom->base());            membank("bank2w")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(m_cart_rom->base()+0x2000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(m_cart_rom->base()+0x4000);     membank("bank4w")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 6:
			program.install_write_bank(0x0000,0x1fff,"bank1w");
			program.install_write_bank(0x2000,0x3fff,"bank2w");
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			membank("bank1r")->set_base(memregion(Z80_TAG)->base());    membank("bank1w")->set_base(ram_region->base()+0x0000);
			membank("bank2r")->set_base(rom_region->base()+0x0000);     membank("bank2w")->set_base(ram_region->base()+0x2000);
			membank("bank3r")->set_base(rom_region->base()+0x2000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
			membank("bank4r")->set_base(rom_region->base()+0x4000);     membank("bank4w")->set_base(ram_region->base()+0x6000);
			membank("bank5r")->set_base(ram_region->base()+0x8000);     membank("bank5w")->set_base(ram_region->base()+0x8000);
			membank("bank6r")->set_base(ram_region->base()+0xc000);     membank("bank6w")->set_base(ram_region->base()+0xc000);
			break;
		case 7: //probably this won't work - it should redirect rw to another ram module
			program.install_write_bank(0x0000,0x1fff,"bank1w");
			program.install_write_bank(0x2000,0x3fff,"bank2w");
			program.install_write_bank(0x4000,0x5fff,"bank3w");
			program.install_write_bank(0x6000,0x6fff,"bank4w");
			program.install_readwrite_bank(0x7000,0x7fff,"sram");
			membank("bank1r")->set_base(rom_region->base()+0x0000);     membank("bank1w")->set_base(rom_region->base()+0x0000);
			membank("bank2r")->set_base(rom_region->base()+0x2000);     membank("bank2w")->set_base(rom_region->base()+0x2000);
			membank("bank3r")->set_base(rom_region->base()+0x4000);     membank("bank3w")->set_base(rom_region->base()+0x4000);
			membank("bank4r")->set_base(rom_region->base()+0x6000);     membank("bank4w")->set_base(rom_region->base()+0x6000);
			membank("sram")->set_base(rom_region->base()+0x7000);
			membank("bank5r")->set_base(rom_region->base()+0x8000);     membank("bank5w")->set_base(rom_region->base()+0x8000);
			membank("bank6r")->set_base(rom_region->base()+0xc000);     membank("bank6w")->set_base(rom_region->base()+0xc000);
			break;
	}

	logerror("64KBF RAM mode set to %d\n", m_ram_mode);
}

//**************************************************************************
//  64KRX paging
//**************************************************************************

WRITE8_MEMBER( m5_state::mem64KRX_w ) //out 0x7f
{
	if (m_ram_type != MEM64KRX) return;
	if (m_ram_mode == data) return;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart_ram->tag()).append(M5SLOT_ROM_REGION_TAG).c_str());
	memory_region *ram_region=memregion(region_tag.assign(m_cart_ram->tag()).append(":ram").c_str());

	m_ram_mode = data;

	BIT(m_ram_mode, 0) ? membank("bank1r")->set_base(memregion(Z80_TAG)->base())    :   membank("bank1r")->set_base(ram_region->base());
	BIT(m_ram_mode, 1) ? membank("bank2r")->set_base(m_cart_rom->base())            :   membank("bank2r")->set_base(ram_region->base()+0x2000);
	BIT(m_ram_mode, 2) ? membank("bank3r")->set_base(m_cart_rom->base()+0x2000)     :   membank("bank3r")->set_base(ram_region->base()+0x4000);

	if ((m_DIPS->read() & 0x01))
	{
		BIT(m_ram_mode, 4) ? membank("bank5r")->set_base(m_cart_rom->base()+0x6000) :   membank("bank5r")->set_base(ram_region->base()+0x8000);
		BIT(m_ram_mode, 5) ? membank("bank6r")->set_base(m_cart_rom->base()+0xa000) :   membank("bank6r")->set_base(ram_region->base()+0xc000);
	}
	else
	{
		BIT(m_ram_mode, 6) ? membank("bank5r")->set_base(m_cart_rom->base()+0xe000) :   membank("bank5r")->set_base(ram_region->base()+0x8000);
		BIT(m_ram_mode, 7) ? membank("bank6r")->set_base(m_cart_rom->base()+0x12000):   membank("bank6r")->set_base(ram_region->base()+0xc000);
	}

	//if KRX ROM is paged out page in cart ROM if any
	if (m_cart && BIT(m_ram_mode, 1) == 0 )
	{
		program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom),(m5_cart_slot_device*)m_cart));
		program.unmap_write(0x2000, 0x6fff);
	}

	logerror("64KRX RAM mode set to %02x\n", m_ram_mode);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( m5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m5_mem, AS_PROGRAM, 8, m5_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_READ_BANK("bank1r") AM_WRITE_BANK("bank1w") //monitor rom(bios)
	AM_RANGE(0x2000, 0x3fff) AM_READ_BANK("bank2r") AM_WRITE_BANK("bank2w")
	AM_RANGE(0x4000, 0x5fff) AM_READ_BANK("bank3r") AM_WRITE_BANK("bank3w")
	AM_RANGE(0x6000, 0x6fff) AM_READ_BANK("bank4r") AM_WRITE_BANK("bank4w")
	AM_RANGE(0x7000, 0x7fff) AM_RAM                                         //4kb internal RAM
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank5r") AM_WRITE_BANK("bank5w")
	AM_RANGE(0xc000, 0xffff) AM_READ_BANK("bank6r") AM_WRITE_BANK("bank6w")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( m5_io, AS_IO, 8, m5_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x08) AM_READ_PORT("Y0") AM_WRITE( mem64KBF_w) // 64KBF paging
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x08) AM_READ_PORT("Y1")
	AM_RANGE(0x32, 0x32) AM_MIRROR(0x08) AM_READ_PORT("Y2")
	AM_RANGE(0x33, 0x33) AM_MIRROR(0x08) AM_READ_PORT("Y3")
	AM_RANGE(0x34, 0x34) AM_MIRROR(0x08) AM_READ_PORT("Y4")
	AM_RANGE(0x35, 0x35) AM_MIRROR(0x08) AM_READ_PORT("Y5")
	AM_RANGE(0x36, 0x36) AM_MIRROR(0x08) AM_READ_PORT("Y6")
	AM_RANGE(0x37, 0x37) AM_MIRROR(0x08) AM_READ_PORT("JOY")
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x0f) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0x0f) AM_READWRITE(sts_r, com_w)
//  AM_RANGE(0x60, 0x63) SIO
	AM_RANGE(0x6c, 0x6c) AM_READWRITE(mem64KBI_r, mem64KBI_w) //EM-64/64KBI paging
	AM_RANGE(0x70, 0x73) /*AM_MIRROR(0x0c) don't know if necessary mirror this*/ AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0x7f, 0x7f) AM_WRITE( mem64KRX_w) //64KRD/64KRX paging
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fd5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fd5_mem, AS_PROGRAM, 8, m5_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fd5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( fd5_io, AS_IO, 8, m5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0x10, 0x10) AM_READWRITE(fd5_data_r, fd5_data_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(fd5_com_w)
	AM_RANGE(0x30, 0x30) AM_READ(fd5_com_r)
	AM_RANGE(0x40, 0x40) AM_WRITE(fd5_ctrl_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(fd5_tc_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( m5 )
//-------------------------------------------------

static INPUT_PORTS_START( m5 )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Func") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_  Triangle") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR('`') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)

	PORT_START("RESET")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) /* 1st line, 1st key from right! */

	PORT_START("DIPS")
	PORT_DIPNAME(0x01, 0x01, "KRX: BASIC[on]/MSX[off]") //switching between BASIC and MSX ROMs which share same address area
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x00, "KBI: AUTOSTART")  //pages out cart and starts loading from tape
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPNAME(0x04, 0x00, "KBI: 32kb only") //compatible with em-5
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
INPUT_PORTS_END

//-------------------------------------------------
//  TMS9928a_interface vdp_intf
//-------------------------------------------------

WRITE_LINE_MEMBER(m5_state::sordm5_video_interrupt_callback)
{
	if (state)
	{
		m_ctc->trg3(1);
		m_ctc->trg3(0);
	}
}


//-------------------------------------------------
//  I8255 Interface
//-------------------------------------------------

READ8_MEMBER( m5_state::ppi_pa_r )
{
	return m_fd5_data;
}

READ8_MEMBER(m5_state::ppi_pc_r )
{
	/*

	    bit     description

	    0       ?
	    1       ?
	    2       ?
	    3
	    4       STBA
	    5
	    6       ACKA
	    7

	*/

	return (
			/* FD5 bit 0-> M5 bit 2 */
			((m_fd5_com & 0x01)<<2) |
			/* FD5 bit 2-> M5 bit 1 */
			((m_fd5_com & 0x04)>>1) |
			/* FD5 bit 1-> M5 bit 0 */
			((m_fd5_com & 0x02)>>1)
	);
}

WRITE8_MEMBER( m5_state::ppi_pa_w )
{
	m_fd5_data = data;
}

WRITE8_MEMBER( m5_state::ppi_pb_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (data == 0xf0)
	{
		m_fd5cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_fd5cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

WRITE8_MEMBER( m5_state::ppi_pc_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3       INTRA
	    4
	    5       IBFA
	    6
	    7       OBFA

	*/

	m_intra = BIT(data, 3);
	m_ibfa = BIT(data, 5);
	m_obfa = BIT(data, 7);
}

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( m5_state::floppy_formats )
	FLOPPY_M5_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( m5_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(m5_cart)
	SLOT_INTERFACE_INTERNAL("std",  M5_ROM_STD)
	SLOT_INTERFACE_INTERNAL("ram",  M5_ROM_RAM)
SLOT_INTERFACE_END

//-------------------------------------------------
//  z80_daisy_config m5_daisy_chain
//-------------------------------------------------

static const z80_daisy_config m5_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ NULL }
};


//-------------------------------------------------
//  BRNO mod code below
//-------------------------------------------------


//-------------------------------------------------
//  ADDRESS_MAP( m5_mem_brno )
//-------------------------------------------------


static ADDRESS_MAP_START( m5_mem_brno, AS_PROGRAM, 8, brno_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_READWRITE_BANK("bank1")
	AM_RANGE(0x1000, 0x1fff) AM_READWRITE_BANK("bank2")
	AM_RANGE(0x2000, 0x2fff) AM_READWRITE_BANK("bank3")
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE_BANK("bank4")
	AM_RANGE(0x4000, 0x4fff) AM_READWRITE_BANK("bank5")
	AM_RANGE(0x5000, 0x5fff) AM_READWRITE_BANK("bank6")
	AM_RANGE(0x6000, 0x6fff) AM_READWRITE_BANK("bank7")
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE_BANK("bank8")
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE_BANK("bank9")
	AM_RANGE(0x9000, 0x9fff) AM_READWRITE_BANK("bank10")
	AM_RANGE(0xa000, 0xafff) AM_READWRITE_BANK("bank11")
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE_BANK("bank12")
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE_BANK("bank13")
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE_BANK("bank14")
	AM_RANGE(0xe000, 0xefff) AM_READWRITE_BANK("bank15")
	AM_RANGE(0xf000, 0xffff) AM_READWRITE_BANK("bank16")
ADDRESS_MAP_END

//-------------------------------------------------
//  ADDRESS_MAP( brno_io )
//-------------------------------------------------
static ADDRESS_MAP_START( brno_io, AS_IO, 8, brno_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0f) AM_DEVWRITE(SN76489AN_TAG, sn76489a_device, write)
	AM_RANGE(0x30, 0x30) AM_READ_PORT("Y0")
	AM_RANGE(0x31, 0x31) AM_READ_PORT("Y1")
	AM_RANGE(0x32, 0x32) AM_READ_PORT("Y2")
	AM_RANGE(0x33, 0x33) AM_READ_PORT("Y3")
	AM_RANGE(0x34, 0x34) AM_READ_PORT("Y4")
	AM_RANGE(0x35, 0x35) AM_READ_PORT("Y5")
	AM_RANGE(0x36, 0x36) AM_READ_PORT("Y6")
	AM_RANGE(0x37, 0x37) AM_READ_PORT("JOY")
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x0f) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0x0f) AM_READWRITE(sts_r, com_w)
//  AM_RANGE(0x60, 0x63)                                                                            //  SIO
	AM_RANGE(0x64, 0x67) AM_READWRITE(mmu_r, mmu_w)                                                 //  MMU - page select (ramdisk memory paging)
	AM_RANGE(0x68, 0x6b) AM_READWRITE(ramsel_r, ramsel_w)                                           //  CASEN 0=access to ramdisk enabled, 0xff=ramdisk access disabled(data protection), &80=ROM2+48k RAM, &81=ROM2+4k RAM
	AM_RANGE(0x6c, 0x6f) AM_READWRITE(romsel_r, romsel_w)                                           //  RAMEN 0=rom enable; 0xff=rom+sord ram disabled (ramdisk visible)
//  AM_RANGE(0x70, 0x73) AM_MIRROR(0x04) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)     //  PIO
	AM_RANGE(0x78, 0x7b) AM_DEVREADWRITE(WD2797_TAG, wd_fdc_t, read, write)                         //  WD2797 registers -> 78 - status/cmd, 79 - track #, 7a - sector #, 7b - data
	AM_RANGE(0x7c, 0x7c) AM_READWRITE(fd_r, fd_w)                                                   //  drive select
ADDRESS_MAP_END


READ8_MEMBER( brno_state::mmu_r )
{
	return 0;
}


WRITE8_MEMBER( brno_state::mmu_w )
{
	m_ramcpu = m_maincpu->state_int(Z80_B);
	m_rambank = ~data; //m_maincpu->state_int(Z80_A);
	m_rammap[m_ramcpu >> 4]=m_rambank;


	switch (m_ramcpu>>4)
	{
		case 0: membank("bank1")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 1: membank("bank2")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 2: membank("bank3")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 3: membank("bank4")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 4: membank("bank5")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 5: membank("bank6")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 6: membank("bank7")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 7: if (!m_romen) membank("bank8")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 8: membank("bank9")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 9: membank("bank10")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 10: membank("bank11")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 11: membank("bank12")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 12: membank("bank13")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 13: membank("bank14")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 14: membank("bank15")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
		case 15: membank("bank16")->set_base(memregion(RAMDISK)->base()+(m_rambank << 12));break;
	}

	//logerror("RAMdisk page change(CPURAM<=BANK): &%02X00<=%02X at address &%04X\n",m_ramcpu,m_rambank,m_maincpu->state_int(Z80_PC)-2);


}

READ8_MEMBER( brno_state::ramsel_r )
{
	return m_ramen;
}


WRITE8_MEMBER( brno_state::ramsel_w ) //out 6b
{
	//address_space &program = m_maincpu->space(AS_PROGRAM);

	if (!data)
		m_ramen=true;
	else
		m_ramen=false;

	logerror("CASEN change: out (&6b),%x\n",data);
}

READ8_MEMBER( brno_state::romsel_r )
{
	return m_romen;
}

WRITE8_MEMBER( brno_state::romsel_w ) //out 6c
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (!data)
	{
		program.install_rom(0x0000, 0x3fff, memregion(Z80_TAG)->base());
		program.unmap_write(0x0000, 0x3fff);
		m_romen=true;
	}

	else
	{
		program.install_readwrite_bank(0x0000, 0x0fff, "bank1");
		program.install_readwrite_bank(0x1000, 0x1fff, "bank2");
		program.install_readwrite_bank(0x2000, 0x2fff, "bank3");
		program.install_readwrite_bank(0x3000, 0x3fff, "bank4");
		program.install_readwrite_bank(0x4000, 0x4fff, "bank5");
		program.install_readwrite_bank(0x5000, 0x5fff, "bank6");
		program.install_readwrite_bank(0x6000, 0x6fff, "bank7");

		m_romen=false;
	}

	logerror("RAMEN change: out (&6c),%x\n",data);
}


//-------------------------------------------------
//  FD port 7c - Floppy select
//-------------------------------------------------

READ8_MEMBER( brno_state::fd_r )
{
	return 0;
}


WRITE8_MEMBER( brno_state::fd_w )
{
	floppy_image_device *floppy;
	m_floppy = NULL;
	int disk = 0;


	floppy = m_floppy0->get_device();
	if (floppy)
	{
		if(BIT(data,0))
		{
			m_floppy= floppy;
			disk=1;
		}
		else
		{
			floppy->mon_w(1);
		}
	}
	floppy = m_floppy1->get_device();
	if (floppy)
	{
		if(BIT(data,1))
		{
			m_floppy= floppy;
			disk=2;
		}
		else
		{
			floppy->mon_w(1);
		}
	}

	m_fdc->set_floppy(m_floppy);
	if (m_floppy)
	{
		m_floppy->set_rpm(300);
		m_floppy->mon_w(0);
		logerror("Select floppy %d\n", disk);
	}

}



FLOPPY_FORMATS_MEMBER( brno_state::floppy_formats )
	FLOPPY_DSK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( brno_floppies )
		SLOT_INTERFACE("35hd", FLOPPY_35_DD)
SLOT_INTERFACE_END


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( m5 )
//-------------------------------------------------
void m5_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_fd5_data));
	save_item(NAME(m_fd5_com));
	save_item(NAME(m_intra));
	save_item(NAME(m_ibfa));
	save_item(NAME(m_obfa));

}

void m5_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	std::string region_tag;

	//is ram/rom cart plugged in?
	if (m_cart1->exists())
	{
		if (m_cart1->get_type() > 0)
			m_cart_ram=m_cart1;
		else
			m_cart=m_cart1;
	}
	if (m_cart2->exists())
	{
		if (m_cart2->get_type() > 0)
			m_cart_ram=m_cart2;
		else
			m_cart=m_cart2;
	}
	// no cart inserted - there is nothing to do - not allowed in original Sord m5
	if (m_cart_ram == NULL && m_cart == NULL)
		{
			membank("bank1r")->set_base(memregion(Z80_TAG)->base());
			program.unmap_write(0x0000, 0x1fff);
		//  program.unmap_readwrite(0x2000, 0x6fff); //if you uncomment this line Sord starts cassette loading but it is not correct on real hw
			program.unmap_readwrite(0x8000, 0xffff);
			return;
		}

	//cart is ram module
	if (m_cart_ram)
	{
		m_ram_type=m_cart_ram->get_type();

		m_cart_rom = memregion(region_tag.assign(m_cart_ram->tag()).append(M5SLOT_ROM_REGION_TAG).c_str());
		memory_region *ram_region=memregion(region_tag.assign(m_cart_ram->tag()).append(":ram").c_str());

		switch (m_ram_type)
		{
			case EM_5:
				program.install_rom(0x0000, 0x1fff, memregion(Z80_TAG)->base());
				program.unmap_write(0x0000, 0x1fff);
				program.install_readwrite_handler(0x8000, 0xffff, read8_delegate(FUNC(m5_cart_slot_device::read_ram),(m5_cart_slot_device*)m_cart_ram), write8_delegate(FUNC(m5_cart_slot_device::write_ram),(m5_cart_slot_device*)m_cart_ram));
				if (m_cart)
				{
					program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom),(m5_cart_slot_device*)m_cart));
					program.unmap_write(0x2000, 0x6fff);
				}
				break;
			case MEM64KBI:
				program.install_rom(0x0000, 0x1fff, memregion(Z80_TAG)->base());
				program.unmap_write(0x0000, 0x1fff);
				program.install_ram(0x8000, 0xffff, ram_region->base()+0x8000);

				//if AUTOSTART is on then page out cart and start tape loading
				if (m_cart && ((m_DIPS->read() & 2) != 2))
				{
					program.install_read_handler(0x2000, 0x3fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom),(m5_cart_slot_device*)m_cart));
					program.unmap_write(0x2000, 0x3fff);
				}
				else
					program.unmap_readwrite(0x2000, 0x6fff); //monitor rom is testing this area for 0xFFs otherwise thinks there is some ROM cart plugged in

				break;
			case MEM64KBF:
				program.unmap_write(0x0000, 0x6fff);
				membank("bank1r")->set_base(memregion(Z80_TAG)->base());
				membank("bank2r")->set_base(m_cart_rom->base());
				membank("bank3r")->set_base(m_cart_rom->base()+0x2000);
				membank("bank4r")->set_base(m_cart_rom->base()+0x4000);
				membank("bank5r")->set_base(ram_region->base()+0x8000); membank("bank5w")->set_base(ram_region->base()+0x8000);
				membank("bank6r")->set_base(ram_region->base()+0xc000); membank("bank6w")->set_base(ram_region->base()+0xc000);
				break;
			case MEM64KRX:
				membank("bank1r")->set_base(memregion(Z80_TAG)->base());    membank("bank1w")->set_base(ram_region->base());
				membank("bank2r")->set_base(m_cart_rom->base());            membank("bank2w")->set_base(ram_region->base()+0x2000);
				membank("bank3r")->set_base(m_cart_rom->base()+0x2000);     membank("bank3w")->set_base(ram_region->base()+0x4000);
				membank("bank4r")->set_base(ram_region->base()+0x6000);     membank("bank4w")->set_base(ram_region->base()+0x6000);

				//page in BASIC or MSX
				if ((m_DIPS->read() & 0x01))
				{
					membank("bank5r")->set_base(m_cart_rom->base()+0x6000); membank("bank5w")->set_base(ram_region->base()+0x8000);
					membank("bank6r")->set_base(m_cart_rom->base()+0xa000); membank("bank6w")->set_base(ram_region->base()+0xc000);
				}
				else
				{
					membank("bank5r")->set_base(m_cart_rom->base()+0xe000);  membank("bank5w")->set_base(ram_region->base()+0x8000);
					membank("bank6r")->set_base(m_cart_rom->base()+0x12000); membank("bank6w")->set_base(ram_region->base()+0xc000);
				}
				break;
			default:
				program.unmap_readwrite(0x8000, 0xffff);
		}
		//I don't have idea what to do with savestates, please someone take care of it
		//m_cart_ram->save_ram();
	}
	else
		//ram cart wasn't found so if rom cart present install it
		if (m_cart)
		{
			program.install_rom(0x0000, 0x1fff, memregion(Z80_TAG)->base());
			program.unmap_write(0x0000, 0x1fff);
			program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom),(m5_cart_slot_device*)m_cart));
			program.unmap_write(0x2000, 0x6fff);
		}
	m_ram_mode=0;
}



void brno_state::machine_start()
{
}

void brno_state::machine_reset()
{
	/* enable ROM1+ROM2 */
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_rom(0x0000, 0x3fff, memregion(Z80_TAG)->base());
	program.unmap_write(0x0000, 0x3fff);


	//is ram/rom cart plugged in?
	if (m_cart1->exists())
	{
		if (m_cart1->get_type() > 0)
			m_cart_ram=m_cart1;
		else
			m_cart=m_cart1;
	}
	if (m_cart2->exists())
	{
		if (m_cart2->get_type() > 0)
			m_cart_ram=m_cart2;
		else
			m_cart=m_cart2;
	}

	if (m_cart)
		{
			program.install_read_handler(0x2000, 0x6fff, read8_delegate(FUNC(m5_cart_slot_device::read_rom),(m5_cart_slot_device*)m_cart));
			program.unmap_write(0x2000, 0x6fff);
		}

	m_romen=true;
	m_ramen=false;

	floppy_image_device *floppy = NULL;
	floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( m5 )
//-------------------------------------------------

static MACHINE_CONFIG_START( m5, m5_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(m5_mem)
	MCFG_CPU_IO_MAP(m5_io)
	MCFG_CPU_CONFIG(m5_daisy_chain)

	MCFG_CPU_ADD(Z80_FD5_TAG, Z80, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(fd5_mem)
	MCFG_CPU_IO_MAP(fd5_io)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// devices
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_14_31818MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	// CK0 = EXINT, CK1 = GND, CK2 = TCK, CK3 = VDP INT
	// ZC2 = EXCLK

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(m5_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(sordm5_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("m5_cass")

	MCFG_DEVICE_ADD(I8255A_TAG, I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(m5_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(m5_state, ppi_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(m5_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(m5_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(m5_state, ppi_pc_w))

	MCFG_UPD765A_ADD(UPD765_TAG, true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE(Z80_FD5_TAG, INPUT_LINE_IRQ0))
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", m5_floppies, "525dd", m5_state::floppy_formats)

	// cartridge
	MCFG_M5_CARTRIDGE_ADD("cartslot1", m5_cart, NULL)
	MCFG_M5_CARTRIDGE_ADD("cartslot2", m5_cart, NULL)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "m5_cart")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "m5_cass")
	//MCFG_SOFTWARE_LIST_ADD("flop_list", "m5_flop")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("36K,64K") //68K is not possible, 'cos internal ram always overlays any expansion memory in that area
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_DERIVED( ntsc, m5 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( ntsc, m5 )
	// video hardware
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(m5_state, sordm5_video_interrupt_callback))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_DERIVED( pal, m5 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pal, m5 )
	// video hardware
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(m5_state, sordm5_video_interrupt_callback))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END

//-------------------------------------------------
//  MACHINE_CONFIG( m5p_brno )
//-------------------------------------------------


static MACHINE_CONFIG_DERIVED_CLASS( brno, m5, brno_state )

	// basic machine hardware
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_PROGRAM_MAP(m5_mem_brno)
	MCFG_CPU_IO_MAP(brno_io)
//  MCFG_CPU_CONFIG(m5_daisy_chain)


	//remove devices used for fd5 floppy
	MCFG_DEVICE_REMOVE(Z80_FD5_TAG)
	MCFG_DEVICE_REMOVE(I8255A_TAG)
	MCFG_DEVICE_REMOVE(UPD765_TAG)

	// video hardware
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(m5_state, sordm5_video_interrupt_callback))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )


	// floppy
	MCFG_WD2797_ADD(WD2797_TAG, XTAL_1MHz)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG":0", brno_floppies, "35hd", brno_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG":1", brno_floppies, "35hd", brno_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	// only one floppy drive
	//MCFG_DEVICE_REMOVE(WD2797_TAG":1")

	//MCFG_SNAPSHOT_ADD("snapshot", brno_state, brno, "rmd", 0)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list","m5_flop")

MACHINE_CONFIG_END


//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( m5 )
//-------------------------------------------------

ROM_START( m5 )
	ROM_REGION( 0x7000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "sordjap.ic21", 0x0000, 0x2000, CRC(92cf9353) SHA1(b0a4b3658fde68cb1f344dfb095bac16a78e9b3e) )

	ROM_REGION( 0x4000, Z80_FD5_TAG, 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
ROM_END


//-------------------------------------------------
//  ROM( m5p )
//-------------------------------------------------

ROM_START( m5p )
	ROM_REGION( 0x7000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "sordint.ic21", 0x0000, 0x2000, CRC(78848d39) SHA1(ac042c4ae8272ad6abe09ae83492ef9a0026d0b2) )

	ROM_REGION( 0x4000, Z80_FD5_TAG, 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
ROM_END

//-------------------------------------------------
//  ROM( brno )
//-------------------------------------------------

ROM_START( m5p_brno )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "sordint.ic21", 0x0000, 0x2000, CRC(78848d39) SHA1(ac042c4ae8272ad6abe09ae83492ef9a0026d0b2)) // monitor rom
	ROM_LOAD( "brno_win.rom", 0x2000, 0x2000, CRC(f4cfb2ee) SHA1(23f41d2d9ac915545409dd0163f3dc298f04eea2)) //windows
	//ROM_LOAD( "brno_rom12.rom", 0x2000, 0x4000, CRC(cac52406) SHA1(91f6ba97e85a2b3a317689635d425ee97413bbe3)) //windows+BI
	//ROM_LOAD( "brno_boot.rom", 0x2000, 0xd80, CRC(60008729) SHA1(FB26E2AE9F74B0AE0D723B417A038A8EF3D72782))

	//Ramdisc area (maximum is 1024kB 256x 4kB banks)
	ROM_REGION(1024*1024,RAMDISK,0)
	ROM_FILL(0,1024*1024,0xff)
ROM_END

//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  ROM( ntsc )
//-------------------------------------------------

DRIVER_INIT_MEMBER(m5_state,ntsc)
{
}


//-------------------------------------------------
//  ROM( pal )
//-------------------------------------------------

DRIVER_INIT_MEMBER(m5_state,pal)
{
}

//-------------------------------------------------
//  ROM( BRNO )
//-------------------------------------------------

DRIVER_INIT_MEMBER(brno_state,brno)
{
//  logerror("Driver init entered\n" );
}


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY     FULLNAME            FLAGS
COMP( 1983, m5,     0,      0,      ntsc,   m5, m5_state,       ntsc,   "Sord",     "m.5 (Japan)",      0 )
COMP( 1983, m5p,    m5,     0,      pal,    m5, m5_state,       pal,    "Sord",     "m.5 (Europe)",     0 )
COMP( 1983, m5p_brno,   m5,     0,      brno,   m5, brno_state,     brno,   "Sord",     "m.5 (Europe) BRNO mod",    0 )
