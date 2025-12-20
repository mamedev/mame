// license:BSD-3-Clause
// copyright-holders:Curt Coder, Ales Dlabac
/***************************************************************************

    Sord m.5

	https://web.archive.org/web/20241214083230/http://m5.arigato.cz/en_index.html
    http://www.dlabi.cz/?s=sord
    https://www.facebook.com/groups/59667560188/
    http://www.oldcomp.cz/viewtopic.php?f=103&t=1164
	https://dlabi.cz/data/IMG_8234.jpg //brno_mod ramdisk
	http://blog.livedoor.jp/hardyboy/tag/SORDM5
	https://web.archive.org/web/20180817030621/http://www.museo8bits.es/wiki/index.php/Sord_M5
	#fd-5
	https://web.archive.org/web/20250516192317if_/http://m5.arigato.cz/cs_fd5.html
	https://dlabi.cz/data/imgs/schemata/PCB-schema-FD-5-A2-Zeravsky.png
	https://dlabi.cz/data/imgs/schemata/PCB-schema-FD-5-A4half.png

****************************************************************************/

/***************************************************************************

TODO:

    - replace fd5 rom hack with proper emulation 
    - SI-5 serial interface (8251, ROM)
    - ramdisk for KRX Memory expansion
    - rewrite fd5 floppy as unpluggable device
    - 64krx: get windows ROM version with cpm & ramdisk support (Stuchlik S.E.I. version)

    - brno mod: add support for lzr floppy disc format
	- brno mod: add support of 16kb carts




CHANGELOG:

30.11.2025
	- replaced named banks with arrays
	- cleaned up redundant memory regions
	- added FD-5 floppies
	- fixed all roms in software list to proper checksums and sizes

	- brno mod: fixed in console version lost data on RAMDISK after soft reset

07.11.2025
	- fd5 floppy emulation works but only with rom hack
	- updated Sord m5 www links
	- added fd5 utility disk to software list - not original dump, made from program listings
	- added support of optional sram in Basic-F and Basic-G cartridges. Works only if shortname(softlist) is used
	- marked 32/64KB RAM expansions EM-5, 64KBI, 64KBF, 64KRX as supported in sofwarelist
	- reenabled and refactored memory banking

	- brno mod: switched to rom including basic-i
	- brno mod: reenabled and refactored memory banking 

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
kB  0   4   8   2   6   0   4   8   2   6   0   4   8   2   6   0   4
    +-------+-------------------+
ROM |MONITOR|      BASIC-F      |
    +-------+-------+-------+---+---+-------+-------+-------+-------+
RAM |       |       |       |       |       |       |       |       |
    +-------+-------+-------+-------+-------+-------+-------+-------+
CART|       |       |       |       |       |       |       |       |
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
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/wd_fdc.h" //brno mod
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "bus/m5/rom.h"
#include "bus/m5/slot.h"

#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/m5_dsk.h"
#include "formats/sord_cas.h"

#define FD5_DEBUG 1
#define BRNO_DEBUG 1
#define VERBOSE 0

#include "logmacro.h"

#define FDC_MOTOR_TIMEOUT 3

namespace {

class m5_state : public driver_device
{
public:
	m5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_fd5cpu(*this, "z80fd5")
		, m_ppi(*this, "ppi")
		, m_fdc(*this, "upd765")
		, m_floppy0(*this, "upd765:0:3ssdd")
		, m_cassette(*this, "cassette")
		, m_cart1(*this, "cartslot1")
		, m_cart2(*this, "cartslot2")
		, m_centronics(*this, "centronics")
		, m_ram(*this, RAM_TAG)
		, m_bankr(*this, "bankr%u", 0)
		, m_bankw(*this, "bankw%u", 0)
		, m_rom_view(*this, "rom")
		, m_reset(*this, "RESET")
		, m_DIPS(*this, "DIPS")
		, m_motor_timer(nullptr)
	{ }

	void m5(machine_config &config);
	void pal(machine_config &config);
	void ntsc(machine_config &config);

protected:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	optional_device<cpu_device> m_fd5cpu;
	optional_device<i8255_device> m_ppi;
	optional_device<upd765a_device> m_fdc;
	optional_device<floppy_image_device> m_floppy0;
	required_device<cassette_image_device> m_cassette;
	optional_device<m5_cart_slot_device> m_cart1;
	optional_device<m5_cart_slot_device> m_cart2;
	required_device<centronics_device> m_centronics;
	optional_device<ram_device> m_ram;
	optional_memory_bank_array<6> m_bankr;
	optional_memory_bank_array<6> m_bankw;
	memory_view m_rom_view;
	required_ioport m_reset;
	optional_ioport m_DIPS;

	m5_cart_slot_device *m_ramcart = nullptr;
	m5_cart_slot_device *m_romcart = nullptr;

	bool m_centronics_busy = false;

	u8 sts_r();
	void com_w(u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(FDC_MOTOR_TIMEOUT_cb);
	emu_timer *m_motor_timer = nullptr;

private:
	u8 ppi_pa_r();
	void ppi_pa_w(u8 data);
	void ppi_pb_w(u8 data);
	u8 ppi_pc_r();
	//void ppi_pc_w(u8 data);

	u8 fd5_data_r();
	void fd5_data_w(u8 data);
	u8 fd5_com_r();
	void fd5_com_w(u8 data);
	void fd5_ctrl_w(u8 data);
	void fd5_tc_w(u8 data);
	 
	static void floppy_formats(format_registration &fr);

	void write_centronics_busy(int state);

	// memory
	u8 mem64KBI_r();
	void mem64KBI_w(offs_t offset, u8 data);
	void mem64KBF_w(u8 data);
	void mem64KRX_w(offs_t offset, u8 data);

	void fd5_io(address_map &map) ATTR_COLD;
	void fd5_mem(address_map &map) ATTR_COLD;
	void m5_io(address_map &map) ATTR_COLD;
	void m5_mem(address_map &map) ATTR_COLD;
	u8 cart_window_r(offs_t offset);
	void cart_window_w(offs_t offset, u8 data);

	u8 m_ram_mode = 0;
	u8 m_ram_type = 0;
	//memory_region *m_cart_rom = nullptr;
	std::unique_ptr<u8[]> m_ignore_writes;

	// floppy state for fd5
	u8 m_fd5_data = 0;
	u8 m_fd5_com = 0;
	//int m_intr = 0;
	//bool m_ibf = 0;
	//bool m_obf = 0;
};


class brno_state : public m5_state
{
public:
	brno_state(const machine_config &mconfig, device_type type, const char *tag)
		: m5_state(mconfig, type, tag)
		, m_rom12_view(*this, "rom12")
		, m_wdfdc(*this, "wd")
		, m_floppy0(*this, "wd:0")
		, m_floppy1(*this, "wd:1")
	{ }

	void brno(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 ramdisk_r(offs_t offset);
	void ramdisk_w(offs_t offset, u8 data);
	void mmu_w(offs_t offset, u8 data);
	void romsel_w(u8 data);
	void ramen_w(u8 data);
	void fd_w(u8 data);
	u8 cartrom_r(offs_t offset);
	//void cartrom_w(offs_t offset, u8 data);

	static void floppy_formats(format_registration &fr);

	//DECLARE_SNAPSHOT_LOAD_MEMBER(brno);

	void brno_io(address_map &map) ATTR_COLD;
	void m5_mem_brno(address_map &map) ATTR_COLD;

	memory_view m_rom12_view;
	required_device<wd2797_device> m_wdfdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy = nullptr;

	u8 m_rammap[16]{}; // memory map
};


void m5_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

//-------------------------------------------------
//  sts_r -
//-------------------------------------------------

u8 m5_state::sts_r()
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

	u8 data = 0;

	// cassette input
	data |= m_cassette->input() >= 0 ? 1 : 0;

	// centronics busy
	if (m_centronics_busy)
		data |= 2;

	// RESET key
	data |= m_reset->read();

	return data;
}


//-------------------------------------------------
//  com_w -
//-------------------------------------------------

void m5_state::com_w(u8 data)
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
//  MEMORY BANKING
//**************************************************************************

//**************************************************************************
//  64KBI support for oldest memory module
//**************************************************************************

u8 m5_state::mem64KBI_r() //in 0x6c
{
	return BIT(m_ram_mode, 0);
}

void m5_state::mem64KBI_w(offs_t offset, u8 data) //out 0x6c
{
	if (m_ram_type != MEM64KBI) return;

	if (m_ram_mode == BIT(data, 0))
		return;

	m_ram_mode = BIT(data, 0);

	//if 32kb only mode don't map top ram
	if (m_ram_mode && (m_DIPS->read() & 4) != 4)
		m_rom_view.select(2); // 64kb RAM
	else

		m_rom_view.select(1); // 32kb RAM, ROMs are preconfigured in machine_start

	logerror("64KBI: ROM %s", m_ram_mode == 0 ? "enabled\n" : "disabled\n");
}

//**************************************************************************
//  64KBF banking
//**************************************************************************

void m5_state::mem64KBF_w(u8 data) //out 0x30
{
	if (m_ram_type != MEM64KBF) return;

	if (m_ram_mode == data)
		return;

	m_ram_mode = data;

	switch(m_ram_mode)
	{
		case 0:
			for (int i = 0; i < 4; i++)
			{
				m_bankr[i]->set_entry(1); m_bankw[i]->set_entry(1);
			}
			m_bankr[4]->set_entry(0); m_bankw[4]->set_entry(0);
			m_bankr[5]->set_entry(0); m_bankw[5]->set_entry(0);
			break;
		case 1:
			for (int i = 0; i < 6; i++)
			{
				m_bankr[i]->set_entry(0); m_bankw[i]->set_entry(0);
			}
			break;
		case 2:
			m_bankr[0]->set_entry(1); m_bankw[0]->set_entry(0);
			for (int i = 1; i < 4; i++)
			{
				m_bankr[i]->set_entry(0); m_bankw[i]->set_entry(0);
			}
			break;
		case 3:
			for (int i = 0; i < 4; i++)
			{
				m_bankr[i]->set_entry(0); m_bankw[i]->set_entry(1);
			}
			m_bankr[4]->set_entry(0); m_bankw[4]->set_entry(0);
			m_bankr[5]->set_entry(0); m_bankw[5]->set_entry(0);
			break;
		case 4:
			m_bankr[0]->set_entry(0); m_bankw[0]->set_entry(1);
			m_bankr[1]->set_entry(0); m_bankw[1]->set_entry(1);
			for (int i = 2; i < 4; i++)
				m_bankr[i]->set_entry(0), m_bankw[i]->set_entry(0);
			break;
		case 5:
			for	(int i = 0; i < 4; i++)
			{
				m_bankr[i]->set_entry(1); m_bankw[i]->set_entry(0);
			}
			m_bankr[4]->set_entry(0); m_bankw[4]->set_entry(0);
			m_bankr[5]->set_entry(0); m_bankw[5]->set_entry(0);
			break;
		case 6: //replace Basic-F with pluged other rom cart
			m_bankr[0]->set_entry(1); m_bankw[0]->set_entry(0);
			m_bankr[1]->set_entry(2); m_bankw[1]->set_entry(0);
			m_bankr[2]->set_entry(2); m_bankw[2]->set_entry(0);
			m_bankr[3]->set_entry(2); m_bankw[3]->set_entry(0);
			m_bankr[4]->set_entry(0); m_bankw[4]->set_entry(0);
			m_bankr[5]->set_entry(0); m_bankw[5]->set_entry(0);
			break;
		case 7: //reserve whole address space for other hardware
			m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x0000, 0xffff);
			break;
	}

	logerror("64KBF RAM mode set to %d\n", m_ram_mode);
}

//**************************************************************************
//  64KRX banking
//**************************************************************************

void m5_state::mem64KRX_w(offs_t offset, u8 data) //out 0x7f
{
	if (m_ram_type != MEM64KRX) return;
	if (m_ram_mode == data) return;
	m_ram_mode = data;

	// it seem no matter RAM/ROM selected writes allways go to ram
	m_bankr[0]->set_entry(BIT(m_ram_mode, 0));
	m_bankr[1]->set_entry(BIT(m_ram_mode, 1));
	m_bankr[2]->set_entry(BIT(m_ram_mode, 2));

	if ((m_DIPS->read() & 0x01))
	{
		m_bankr[4]->set_entry(BIT(m_ram_mode, 4));
		m_bankr[5]->set_entry(BIT(m_ram_mode, 5));
	}
	else
	{
		m_bankr[4]->set_entry(BIT(m_ram_mode, 6) ? 2 : 0);
		m_bankr[5]->set_entry(BIT(m_ram_mode, 7) ? 2 : 0);
	}

	logerror("64KRX RAM mode set to %02x\n", m_ram_mode);
}

u8 m5_state::cart_window_r(offs_t offset)
{	
	return m_ramcart->read_ram(offset);
}

void m5_state::cart_window_w(offs_t offset, u8 data)
{
    if (m_ramcart)
        m_ramcart->write_ram(offset, data);
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( m5_mem )
//-------------------------------------------------

void m5_state::m5_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).view(m_rom_view);
	map(0x7000, 0x7fff).ram().share("ram");

	// for 64KBF, 64KRX memory modules
	m_rom_view[0](0x0000, 0x1fff).bankr(m_bankr[0]).bankw(m_bankw[0]);
	m_rom_view[0](0x2000, 0x3fff).bankr(m_bankr[1]).bankw(m_bankw[1]);
	m_rom_view[0](0x4000, 0x5fff).bankr(m_bankr[2]).bankw(m_bankw[2]);
	m_rom_view[0](0x6000, 0x6fff).bankr(m_bankr[3]).bankw(m_bankw[3]);
	m_rom_view[0](0x8000, 0xbfff).bankr(m_bankr[4]).bankw(m_bankw[4]);
	m_rom_view[0](0xc000, 0xffff).bankr(m_bankr[5]).bankw(m_bankw[5]);

	// 64KBI memory module
	m_rom_view[1](0x0000, 0xffff).rw(FUNC(m5_state::cart_window_r),
		FUNC(m5_state::cart_window_w));
	m_rom_view[1](0x0000, 0x1fff).rom().region("monitor", 0x0000).unmapw(); // monitor rom
	m_rom_view[1](0x2000, 0x3fff).bankr(m_bankr[1]); // cart rom
	m_rom_view[1](0x4000, 0x5fff).bankr(m_bankr[2]); // cart rom
	m_rom_view[1](0x6000, 0x6fff).bankr(m_bankr[3]); // cart rom
	m_rom_view[1](0x7000, 0x7fff).ram().share("ram");

	// for RAM only mode
	m_rom_view[2](0x0000, 0xffff).rw(FUNC(m5_state::cart_window_r),
		FUNC(m5_state::cart_window_w));
	m_rom_view[2](0x7000, 0x7fff).ram().share("ram");
}


//-------------------------------------------------
//  ADDRESS_MAP( m5_io )
//-------------------------------------------------

void m5_state::m5_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x0c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x11).mirror(0x0e).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0x20, 0x20).mirror(0x0f).w("sgc", FUNC(sn76489a_device::write));
	map(0x30, 0x30).mirror(0x00).portr("Y0").w(FUNC(m5_state::mem64KBF_w)); // 64KBF paging
	map(0x31, 0x31).mirror(0x00).portr("Y1");
	map(0x32, 0x32).mirror(0x00).portr("Y2");
	map(0x33, 0x33).mirror(0x00).portr("Y3");
	map(0x34, 0x34).mirror(0x00).portr("Y4");
	map(0x35, 0x35).mirror(0x00).portr("Y5");
	map(0x36, 0x36).mirror(0x00).portr("Y6");
	map(0x37, 0x37).mirror(0x00).portr("JOY");
	map(0x40, 0x40).mirror(0x0f).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x50, 0x50).mirror(0x0f).rw(FUNC(m5_state::sts_r), FUNC(m5_state::com_w));
//  map(0x60, 0x63) SIO
	map(0x6c, 0x6c).rw(FUNC(m5_state::mem64KBI_r), FUNC(m5_state::mem64KBI_w)); //EM-64/64KBI paging
	map(0x70, 0x73).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)); //PIO
	map(0x7f, 0x7f).w(FUNC(m5_state::mem64KRX_w)); //64KRD/64KRX paging
	map(0xa8, 0xab).mirror(0x00).noprw(); //some ported MSX games write here
}


//-------------------------------------------------
//  ADDRESS_MAP( fd5_mem )
//-------------------------------------------------

void m5_state::fd5_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xffff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( fd5_io )
//-------------------------------------------------

void m5_state::fd5_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).m(m_fdc, FUNC(upd765a_device::map));
	map(0x10, 0x10).rw(FUNC(m5_state::fd5_data_r), FUNC(m5_state::fd5_data_w));
	map(0x20, 0x20).w(FUNC(m5_state::fd5_com_w));
	map(0x30, 0x30).r(FUNC(m5_state::fd5_com_r));
	map(0x40, 0x40).w(FUNC(m5_state::fd5_ctrl_w));
	map(0x50, 0x50).w(FUNC(m5_state::fd5_tc_w));
}



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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR('\\') PORT_CHAR('|') // backslash ok for m5p, shows as ¥ on m5.

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
//  I8255 Interface
//-------------------------------------------------

// 0x70
u8 m5_state::ppi_pa_r()
{
		return m_fd5_data;
}

void m5_state::ppi_pa_w(u8 data)
{
	m_fd5_data = data;
}

// 0x71
void m5_state::ppi_pb_w(u8 data)
{
	/*

	    bit     description

		0 	AD0 \
		1	AD1	 > only zeroes will work here, as the FD5 expects
	    2	AD2 /
		3	PIO/Peripherial SWITCH 1 = DEFAULT, 0 = INTELIGENT Interface
	    4
	    5
	    6	!NMI
	    7   !RTY

	*/

	if (!BIT(data, 6))
	{
		if (FD5_DEBUG) LOG("%04X: Out (71h), %02X. Resetting FD5 CPU\n", m_maincpu->pc() - 2, data);
	}

	m_fd5cpu->set_input_line(INPUT_LINE_NMI, !BIT(data,6));
	
	
	

	
}

// 0x72
u8 m5_state::ppi_pc_r()
{
	/*

	    bit     description

	    0       CMD/STATUS
	    1       DTO
	    2       !RFD
	    3		?
	    4       STB
	    5		IBF
	    6       !ACK
	    7		!OBF

	*/

	u8 out = (
		/* FD5 bit 0-> M5 bit 2 */
		((m_fd5_com & 0x01) << 2) |
		/* FD5 bit 1-> M5 bit 0 */
		((m_fd5_com & 0x02) >> 1) |
		/* FD5 bit 2-> M5 bit 1 */
		((m_fd5_com & 0x04) >> 1) |
		/* FD5 bit 3-> M5 bit 4 */
		((m_fd5_com & 0x08) << 1) |
		/* FD5 bit 4-> M5 bit 6 */
		((m_fd5_com & 0x10) << 2)
		);
	return out;
}


//**************************************************************************
//  FD-5
//**************************************************************************

//-------------------------------------------------
//  fd5_data_r - 0x10
//-------------------------------------------------

u8 m5_state::fd5_data_r()
{
	m_ppi->pc6_w(0); //ACK
	m_ppi->pc6_w(1); 
	return m_fd5_data;
}


//-------------------------------------------------
//  fd5_data_w - 0x10
//-------------------------------------------------

void m5_state::fd5_data_w(u8 data)
{
	m_fd5_data = data;
	m_ppi->pc4_w(0); //STB
	m_ppi->pc4_w(1);
	
}


//-------------------------------------------------
//  fd5_com_r - 0x30
//-------------------------------------------------

u8 m5_state::fd5_com_r()
{
	/*

		bit     description

		0       DEVICE NUMBER, needs to be 0 and this is only if PB0-PB3=0
		1       /RTY Sequence reset, needs to be 1
		2       IBF
		3       !OBF
		4
		5
		6
		7

	*/

	uint8_t pc = m_ppi->read(2);     // Read Port C (index 2)
	bool obf = BIT(pc, 7);
	bool ibf = BIT(pc, 5);
	uint8_t pb = m_ppi->pb_r();
	bool DEV = BIT(pb, 0) | BIT(pb, 1) | BIT(pb, 2) | BIT(pb, 3);
	uint8_t RTY = BIT(pb, 7) << 1;

	uint8_t out = obf << 3 | ibf << 2 | RTY | DEV; // device number 0 and RTY is harcoded here
	//LOG("IN A,(30) -> %02X\n", out);
	return out;
}


//-------------------------------------------------
//  fd5_com_w - 0x20
//-------------------------------------------------

void m5_state::fd5_com_w(u8 data)
{
	/*

		bit     description

		0       PPI PC2/RFD
		1       PPI PC0/CMD
		2       PPI PC1/DTO
		3		PPI PC4/STB
		4		PPI PC5/ACK
		5
		6
		7

	*/
	//LOG("%04x: Out (20h),%02x\n", m_fd5cpu->pc() - 2, data);

	m_fd5_com = data;
}


//-------------------------------------------------
//  fd5_ctrl_w - 0x40
//-------------------------------------------------

void m5_state::fd5_ctrl_w(u8 data)
{
	//data aren't used at all. Writing to this port just prolongs time when motor is running

	bool state = m_floppy0->mon_r();

	//spin up motor only if it isn't spinning already
	if (state) {
		if (FD5_DEBUG) LOG("FD5 Motor ON\n");
		m_floppy0->mon_w(0); //motor on
	}
	//add more time before switching off
	m_motor_timer->adjust(attotime::from_seconds(FDC_MOTOR_TIMEOUT));
}



TIMER_CALLBACK_MEMBER(m5_state::FDC_MOTOR_TIMEOUT_cb)
{
	if (FD5_DEBUG) LOG("FD5 Motor OFF\n");
	m_floppy0->mon_w(1); //timeout -> turn motor off
}


//-------------------------------------------------
//  fd5_com_w - 0x50
//-------------------------------------------------

void m5_state::fd5_tc_w(u8 data)
{
	if (FD5_DEBUG) LOG("TC Pulse\n");
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

void m5_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	//fr.add(FLOPPY_M5_FORMAT);
	fr.add(FLOPPY_FD5_FORMAT);
}

static void fd5_floppies(device_slot_interface &device)
{
	//device.option_add("525dd", FLOPPY_525_DD);
	//device.option_add("3dsdd", FLOPPY_3_DSDD);
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}

static void m5_cart(device_slot_interface &device)
{
	device.option_add_internal("std",  M5_ROM_STD);
	device.option_add_internal("ram",  M5_ROM_RAM);
}

//-------------------------------------------------
//  z80_daisy_config m5_daisy_chain
//-------------------------------------------------

static const z80_daisy_config m5_daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


//-------------------------------------------------
//  BRNO mod code below
//-------------------------------------------------


//-------------------------------------------------
//  ADDRESS_MAP( m5_mem_brno )
//-------------------------------------------------


void brno_state::m5_mem_brno(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).view(m_rom_view);
	m_rom_view[0](0x0000, 0xffff).rw(FUNC(brno_state::ramdisk_r),			  // where isn't ROM map RAMdisk
		FUNC(brno_state::ramdisk_w));

	// boot config
	m_rom_view[0](0x0000, 0x1fff).rom().region("maincpu", 0x0000).unmapw();	  // monitor ROM
	m_rom_view[0](0x2000, 0x3fff).view(m_rom12_view);						  // ROM1,ROM2 or cart, selected by m_rom12_view
	m_rom_view[0](0x7000, 0x7fff).ram().share("internal");					  // internal RAM
	m_rom12_view[0](0x2000, 0x3fff).rom().region("maincpu", 0x2000).unmapw(); // ROM1
	m_rom12_view[1](0x2000, 0x3fff).rom().region("maincpu", 0x4000).unmapw(); // ROM2 - Basic-I
	m_rom12_view[2](0x2000, 0x3fff).r(FUNC(brno_state::cartrom_r)).unmapw();  // cartridge ROM

	// Ramdisk only
	m_rom_view[1](0x0000, 0xffff).rw(FUNC(brno_state::ramdisk_r),
		FUNC(brno_state::ramdisk_w));

	
}

//-------------------------------------------------
//  ADDRESS_MAP( brno_io )
//-------------------------------------------------
void brno_state::brno_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0xff0c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x11).mirror(0xff0e).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0x20, 0x20).mirror(0xff0f).w("sgc", FUNC(sn76489a_device::write));
	map(0x30, 0x30).mirror(0xff00).portr("Y0");
	map(0x31, 0x31).mirror(0xff00).portr("Y1");
	map(0x32, 0x32).mirror(0xff00).portr("Y2");
	map(0x33, 0x33).mirror(0xff00).portr("Y3");
	map(0x34, 0x34).mirror(0xff00).portr("Y4");
	map(0x35, 0x35).mirror(0xff00).portr("Y5");
	map(0x36, 0x36).mirror(0xff00).portr("Y6");
	map(0x37, 0x37).mirror(0xff00).portr("JOY");
	map(0x40, 0x40).mirror(0xff0f).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x50, 0x50).mirror(0xff0f).rw(FUNC(brno_state::sts_r), FUNC(brno_state::com_w));
	map(0x60, 0x61).mirror(0xff02).rw("sio", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x64, 0x64).select(0xf000).mirror(0x0f03).w(FUNC(brno_state::mmu_w));         //  MMU - page select (ramdisk memory paging), uses also A12-A15
	map(0x68, 0x68).mirror(0xff03).w(FUNC(brno_state::romsel_w));                     //  CASEN
	map(0x6c, 0x6c).mirror(0xff03).w(FUNC(brno_state::ramen_w));                      //  RAMEN
	map(0x70, 0x73).mirror(0xff00).rw("pio", FUNC(i8255_device::read), FUNC(i8255_device::write)); //  PIO
	map(0x78, 0x7b).mirror(0xff00).rw(m_wdfdc, FUNC(wd_fdc_device_base::read), FUNC(wd_fdc_device_base::write));   //  WD2797 registers -> 78 - status/cmd, 79 - track #, 7a -sector #, 7b - data
	map(0x7c, 0x7c).mirror(0xff00).w(FUNC(brno_state::fd_w));                         //  drive select
	map(0xa8, 0xab).mirror(0xff00).noprw(); //some ported MSX games write here
}


u8 brno_state::ramdisk_r(offs_t offset)
{
	offset = (offset & 0x0fff) | u32(m_rammap[offset >> 12]) << 12;
	if (offset < m_ram->size())
		return m_ram->read(offset);
	else
		return 0xff;
}

void brno_state::ramdisk_w(offs_t offset, u8 data)
{
	offset = (offset & 0x0fff) | u32(m_rammap[offset >> 12]) << 12;
	if (offset < m_ram->size())
		m_ram->write(offset, data);
}


void brno_state::mmu_w(offs_t offset, u8 data)
{
	u8 ramcpu = offset >> 12;
	u8 rambank = ~data;
	m_rammap[ramcpu] = rambank;

	if (BRNO_DEBUG) logerror("RAMdisk page change(CPURAM<=BANK): &%X000<=%02X at %s\n", ramcpu, rambank, machine().describe_context());
}

void brno_state::romsel_w(u8 data) //out 6b
{
	// 0= ROM1 + 64kb RAM, &80=ROM2+48k RAM, &81=ROM2+4k RAM
	if (m_romcart != nullptr)
		return;
	if (BIT(data, 7))
		m_rom12_view.select(1);
	else if (data == 0)
		m_rom12_view.select(0);

	if (BRNO_DEBUG) logerror("CASEN change: out (&6b),%x\n",data);
}

void brno_state::ramen_w(u8 data) //out 6c
{
	// 0=rom+sord ram enabled, rest is ramdisk; 1=rom+sord ram disabled (only ramdisk visible)
	m_rom_view.select((data & 1) ? 1 : 0);
	if (BRNO_DEBUG) logerror("RAMEN change: out (&6c),%x\n",data);
}

u8 brno_state::cartrom_r(offs_t offset) { return m_romcart ? m_romcart->read_rom(offset) : 0xff; }

//-------------------------------------------------
//  FD port 7c - Floppy select
//-------------------------------------------------

void brno_state::fd_w(u8 data)
{
	floppy_image_device *floppy;
	m_floppy = nullptr;
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

	m_wdfdc->set_floppy(m_floppy);
	if (m_floppy)
	{
		m_floppy->set_rpm(300);
		m_floppy->mon_w(0);
		logerror("Select floppy %d\n", disk);
	}
}



static void brno_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_DD);
}


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
	save_item(NAME(m_centronics_busy));
	m_motor_timer = timer_alloc(FUNC(m5_state::FDC_MOTOR_TIMEOUT_cb), this); //SED9420 trigger-in emulate
}

void m5_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint32_t romcart_rom_size = 0;
	uint32_t romcart_ram_size = 0;
	//uint32_t ramcart_ram_size = 0;
	uint8_t* romcart_ram_base = nullptr;
	uint8_t* romcart_rom_base = nullptr;
	uint8_t* ramcart_ram_base = nullptr;
	uint8_t* ramcart_rom_base = nullptr;

	m_ignore_writes = std::make_unique<u8[]>(0x2000);
	std::memset(m_ignore_writes.get(), 0xFF, 0x2000);
	u8 entry = 1;

	//is ram/rom cart plugged in?
	if (m_cart1->exists())
	{
		if (m_cart1->get_type() > 0)
			m_ramcart = m_cart1;
		else
			m_romcart = m_cart1;
	}

	if (m_cart2->exists())
	{
		if (m_cart2->get_type() > 0)
			m_ramcart = m_cart2;
		else
			m_romcart = m_cart2;
	}

	// no cart inserted - there is nothing to do - Sord m5 won't power up
	// but some owners bypassed this by shorting some lines on the motherboard and Sord power up and starts cassette loading
	if (m_ramcart == nullptr && m_romcart == nullptr)
	{
		program.unmap_write(0x0000, 0x1fff);
		program.unmap_readwrite(0x2000, 0x6fff); //if you uncomment this line Sord starts cassette loading but it is not correct on vanilla Sord m5. see above comment
		program.unmap_readwrite(0x8000, 0xffff);
		return;
	}

	if (m_romcart) {
		romcart_rom_size = m_romcart->get_rom_size();
		romcart_rom_base = m_romcart->get_rom_base();
	}

	//cart is ram module
	if (m_ramcart)
	{
		m_ram_type=m_ramcart->get_type();
		//ramcart_ram_size = m_ramcart->get_ram_size();
		ramcart_ram_base = m_ramcart->get_ram_base();
		ramcart_rom_base = m_ramcart->get_rom_base();

		switch (m_ram_type)
		{
			case EM_5:
			{
				program.install_rom(0x0000, 0x1fff, memregion("monitor")->base());
				program.unmap_write(0x0000, 0x1fff);
				program.install_ram(0x8000, 0xffff, ramcart_ram_base);
				if (m_romcart)
				{
					program.install_read_handler(0x2000, 0x2000 + romcart_rom_size - 1, read8sm_delegate(*m_romcart, FUNC(m5_cart_slot_device::read_rom)));
					program.unmap_write(0x2000, 0x2000 + romcart_rom_size - 1);
				}
				else
					program.unmap_readwrite(0x2000, 0x6fff);
				break;
			}
			case MEM64KBI:
			{
				m_rom_view.select(1);
				//if AUTOSTART is on then page out cart and start tape loading
				bool autostart = (m_DIPS->read() & 2) == 2;
				bool ram32k_only = (m_DIPS->read() & 4) == 4;
				if (m_romcart)
				{	//if not autostart map needed banks to rom cart, rest to ram
					for (int i = 1; i < 4; ++i) {
						if ((i - 1) * 0x2000 < romcart_rom_size - 1 && !autostart)
						{
							m_bankr[i]->set_base(romcart_rom_base + (i - 1) * 0x2000);
						}
						else if (ram32k_only) m_bankr[i]->set_base(m_ignore_writes.get());
						 else m_bankr[i]->set_base(ramcart_ram_base + (i - 1) * 0x2000);
					}
				}

				break;
			}
			case MEM64KBF:
			{
				if (m_romcart)
				{
					//replace 64Kbf Basic-F with rom cart, works only with mode 6
					for (int i = 1; i < 4; i++)
						m_bankr[i]->configure_entry(2, romcart_rom_base + (i - 1) * 0x2000);

				}
				// ram entries
				for (int i = 0; i < 5; i++) {
					m_bankr[i]->configure_entry(0, ramcart_ram_base + i * 0x2000);
					m_bankw[i]->configure_entry(0, ramcart_ram_base + i * 0x2000);
				}
				m_bankr[5]->configure_entry(0, ramcart_ram_base + 0xc000); m_bankw[5]->configure_entry(0, ramcart_ram_base + 0xc000);

				// rom entries
				m_bankr[0]->configure_entry(1, memregion("monitor")->base());	m_bankw[0]->configure_entry(1, m_ignore_writes.get());
				m_bankr[1]->configure_entry(1, ramcart_rom_base);				m_bankw[1]->configure_entry(1, m_ignore_writes.get());
				m_bankr[2]->configure_entry(1, ramcart_rom_base + 0x2000);		m_bankw[2]->configure_entry(1, m_ignore_writes.get());
				m_bankr[3]->configure_entry(1, ramcart_rom_base + 0x4000);		m_bankw[3]->configure_entry(1, m_ignore_writes.get());
	
				m_rom_view.select(0);

				for (int i = 0; i < 4; i++)
				{
					m_bankr[i]->set_entry(1); m_bankw[i]->set_entry(1);
				}
				m_bankr[4]->set_entry(0); m_bankw[4]->set_entry(0);
				m_bankr[5]->set_entry(0); m_bankw[5]->set_entry(0);

				break;
			}
			case MEM64KRX:
			{
				// ram entries
				for (int i = 0; i < 5; i++) {
					m_bankr[i]->configure_entry(0, ramcart_ram_base + i * 0x2000);
					m_bankw[i]->configure_entry(0, ramcart_ram_base + i * 0x2000);
				}
				m_bankr[5]->configure_entry(0, ramcart_ram_base + 0xc000); m_bankw[5]->configure_entry(0, ramcart_ram_base + 0xc000);

				// rom entries
				m_bankr[0]->configure_entry(1, memregion("monitor")->base()); m_bankw[0]->configure_entry(1, m_ignore_writes.get());
				for (int i = 1; i < 5; i++) {
					m_bankr[i]->configure_entry(1, ramcart_rom_base + (i - 1) * 0x2000);
					m_bankw[i]->configure_entry(1, m_ignore_writes.get());
				}
				m_bankr[5]->configure_entry(1, ramcart_rom_base + 0xa000); m_bankw[5]->configure_entry(1, m_ignore_writes.get());

				// MSX ROM
				m_bankr[4]->configure_entry(2, ramcart_rom_base + 0xe000);
				m_bankr[5]->configure_entry(2, ramcart_rom_base + 0x12000);

				//select BASICs or MSX
				if ((m_DIPS->read() & 0x01))
				{
					//BASIC-G and BASIC-F
					entry = 1;
				}
				else
				{
					//MSX
					entry = 2;
				}

				m_rom_view.select(0);

				// it seem no matter RAM/ROM selected writes allways go to ram
				m_bankr[0]->set_entry(1);		m_bankw[0]->set_entry(0);
				m_bankr[1]->set_entry(1);		m_bankw[1]->set_entry(0);
				m_bankr[2]->set_entry(1);		m_bankw[2]->set_entry(0);
				m_bankr[3]->set_entry(0);		m_bankw[3]->set_entry(0);
				m_bankr[4]->set_entry(entry);	m_bankw[4]->set_entry(0);
				m_bankr[5]->set_entry(entry);	m_bankw[5]->set_entry(0);
				break;
			}
			default:
				program.unmap_readwrite(0x8000, 0xffff);
		}
	}
	else
		//ram cart wasn't found so if rom cart present install it
		if (m_romcart)
		{
			//BASIC-G and BASIC-F carts may have builtin ram so map it here;
			romcart_ram_base = m_romcart->get_ram_base();
			romcart_ram_size = m_romcart->get_ram_size();

				if (romcart_ram_base) {
					if (romcart_ram_size > 0) {
						uint32_t map_start = 0x8000;
						uint32_t map_end = 0x8000 + romcart_ram_size - 1;
						if (map_end > 0xffff) map_end = 0xffff;
						program.install_ram(map_start, map_end, romcart_ram_base);
					}
					else program.unmap_readwrite(0x8000, 0xffff);
				}
				else program.unmap_readwrite(0x8000, 0xffff);
			//}

			program.install_rom(0x0000, 0x1fff, memregion("monitor")->base());
			program.unmap_write(0x0000, 0x1fff);
			program.install_read_handler(0x2000, 0x6fff, read8sm_delegate(*m_romcart, FUNC(m5_cart_slot_device::read_rom)));
			program.unmap_write(0x2000, 0x6fff);
		}
	m_ram_mode = 0;
}



void brno_state::machine_start()
{
	std::fill(std::begin(m_rammap), std::end(m_rammap), 0);

	save_item(NAME(m_rammap));
	save_item(NAME(m_centronics_busy));
}

void brno_state::machine_reset()
{

	//is ram/rom cart plugged in?
	if (m_cart1->exists())
	{
		if (m_cart1->get_type() > 0)
			m_ramcart=m_cart1;
		else
			m_romcart=m_cart1;
	}
	if (m_cart2->exists())
	{
		if (m_cart2->get_type() > 0)
			m_ramcart=m_cart2;
		else
			m_romcart=m_cart2;
	}

	if (m_romcart) m_rom12_view.select(2); //use cartridge
	else m_rom12_view.select(0); //ROM1 as default

	m_rom_view.select(0); // enable ROMS + internal RAM


	floppy_image_device *floppy = nullptr;
	floppy = m_floppy0->get_device();
	m_wdfdc->set_floppy(floppy);
	floppy->mon_w(0);
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( m5 )
//-------------------------------------------------

void m5_state::m5(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10.738635_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &m5_state::m5_mem);
	m_maincpu->set_addrmap(AS_IO, &m5_state::m5_io);
	m_maincpu->set_daisy_config(m5_daisy_chain);

	Z80(config, m_fd5cpu, 16_MHz_XTAL / 4);
	m_fd5cpu->set_addrmap(AS_PROGRAM, &m5_state::fd5_mem);
	m_fd5cpu->set_addrmap(AS_IO, &m5_state::fd5_io);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, "sgc", 10.738635_MHz_XTAL / 3).add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	Z80CTC(config, m_ctc, 10.738635_MHz_XTAL / 3);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<2>(10.738635_MHz_XTAL / 24);
	// CK0 = EXINT, CK1 = GND, CK2 = TCK, CK3 = VDP INT
	// ZC2 = EXCLK

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(m5_state::write_centronics_busy));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(sordm5_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("m5_cass");

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(m5_state::ppi_pa_r));
	m_ppi->out_pa_callback().set(FUNC(m5_state::ppi_pa_w));
	m_ppi->out_pb_callback().set(FUNC(m5_state::ppi_pb_w));
	m_ppi->in_pc_callback().set(FUNC(m5_state::ppi_pc_r));


	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true); // clocked by SED9420C
	m_fdc->intrq_wr_callback().set_inputline(m_fd5cpu, INPUT_LINE_IRQ0); //.invert();

	//FLOPPY_CONNECTOR(config, "upd765:0", m5_floppies, "525dd", m5_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:0", fd5_floppies, "3ssdd", m5_state::floppy_formats).enable_sound(true);

	// cartridge
	M5_CART_SLOT(config, m_cart1, m5_cart, nullptr);
	M5_CART_SLOT(config, m_cart2, m5_cart, nullptr);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("m5_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("m5_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("m5_flop");

	// internal ram
	//68K is not possible, 'cos internal ram always overlays any expansion memory in that area
	//RAM(config, m_ram).set_default_size("4K").set_extra_options("36K,64K");
}


//-------------------------------------------------
//  machine_config( ntsc )
//-------------------------------------------------

void m5_state::ntsc(machine_config &config)
{
	m5(config);
	// video hardware
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(m_ctc, FUNC(z80ctc_device::trg3)).invert();
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}


//-------------------------------------------------
//  machine_config( pal )
//-------------------------------------------------

void m5_state::pal(machine_config &config)
{
	m5(config);
	// video hardware
	tms9929a_device &vdp(TMS9929A(config, "tms9928a", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(m_ctc, FUNC(z80ctc_device::trg3)).invert();
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

//-------------------------------------------------
//  machine_config( m5p_brno )
//-------------------------------------------------


void brno_state::brno(machine_config &config)
{
	m5(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &brno_state::m5_mem_brno);
	m_maincpu->set_addrmap(AS_IO, &brno_state::brno_io);


	//remove devices used for fd5 floppy
	config.device_remove("z80fd5");
	config.device_remove("ppi");
	config.device_remove("upd765");

	// video hardware
	tms9929a_device &vdp(TMS9929A(config, "tms9928a", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(m_ctc, FUNC(z80ctc_device::trg3)).invert();
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// RAM disk (maximum is 1024kB 256x 4kB banks)
	RAM(config, m_ram).set_default_size("1M").set_extra_options("256K,768K,1M");

	i8251_device &sio(I8251(config, "sio", 10.738635_MHz_XTAL / 3));
	sio.txd_handler().set("serial", FUNC(rs232_port_device::write_txd));
	sio.rts_handler().set("serial", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.rxd_handler().set("sio", FUNC(i8251_device::write_rxd));
	serial.cts_handler().set("sio", FUNC(i8251_device::write_cts));

	m_ctc->zc_callback<2>().set("sio", FUNC(i8251_device::write_txc));
	m_ctc->zc_callback<2>().append("sio", FUNC(i8251_device::write_rxc));

	I8255(config, "pio");

	// floppy
	WD2797(config, m_wdfdc, 4_MHz_XTAL / 4);
	FLOPPY_CONNECTOR(config, m_floppy0, brno_floppies, "35hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, brno_floppies, "35hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	//SNAPSHOT(config, "snapshot", "rmd", 0).set_load_callback(brno_state::snapshot_cb));

}


//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( m5 )
//-------------------------------------------------

ROM_START( m5 )
	ROM_REGION( 0x2000, "monitor", ROMREGION_ERASEFF )
	ROM_LOAD( "sordjap.ic21", 0x0000, 0x2000, CRC(92cf9353) SHA1(b0a4b3658fde68cb1f344dfb095bac16a78e9b3e) )

	ROM_REGION( 0x4000, "z80fd5", 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
ROM_END


//-------------------------------------------------
//  ROM( m5p )
//-------------------------------------------------

ROM_START( m5p )
	ROM_REGION( 0x2000, "monitor", ROMREGION_ERASEFF )
	ROM_LOAD( "sordint.ic21", 0x0000, 0x2000, CRC(78848d39) SHA1(ac042c4ae8272ad6abe09ae83492ef9a0026d0b2) )

	ROM_REGION( 0x4000, "z80fd5", 0 )
	ROM_LOAD( "sordfd5.rom", 0x0000, 0x4000, CRC(7263bbc5) SHA1(b729500d3d2b2e807d384d44b76ea5ad23996f4a))
	//rom patch which fixes case where IRQ fireups sooner than polling rutine expects. don't know real reason why timings is so different
	//ROM_FILL(0x2038, 3, 0) short but not safe fix

	// RST 18 rutine
	ROM_FILL(0x18, 1, 0xf3)		//di added this to avoid premature IRQ
	ROM_FILL(0x19, 1, 0xed)		// ld de,($c3df)
	ROM_FILL(0x1a, 1, 0x5b)
	ROM_FILL(0x1b, 1, 0xdf)
	ROM_FILL(0x1c, 1, 0xc3)
	ROM_FILL(0x1d, 1, 0xc9)		//ret
	//patch to jump to rst 18
	ROM_FILL(0x1fef, 1, 0xdf)	// replace ld de,($c3df) by rst 18h where above subrutine is located
	ROM_FILL(0x1ff0, 3, 0)		// replace rest of ld instruction by nops

ROM_END

//-------------------------------------------------
//  ROM( brno )
//-------------------------------------------------

ROM_START( m5p_brno )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "sordint.ic21", 0x0000, 0x2000, CRC(78848d39) SHA1(ac042c4ae8272ad6abe09ae83492ef9a0026d0b2)) // monitor rom
	//ROM_LOAD( "brno_win.rom", 0x2000, 0x2000, CRC(f4cfb2ee) SHA1(23f41d2d9ac915545409dd0163f3dc298f04eea2)) //windows version
	ROM_LOAD( "brno_rom12.rom", 0x2000, 0x4000, CRC(cac52406) SHA1(91f6ba97e85a2b3a317689635d425ee97413bbe3)) //windows+BI version
	//ROM_LOAD("brno_boot.rom", 0x2000, 0xd80, CRC(60008729) SHA1(fb26e2ae9f74b0ae0d723b417a038a8ef3d72782)) //console version
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME                 FLAGS
COMP( 1983, m5,       0,      0,      ntsc,    m5,    m5_state,   empty_init, "Sord",  "m.5 (Japan)",           0 )
COMP( 1983, m5p,      m5,     0,      pal,     m5,    m5_state,   empty_init, "Sord",  "m.5 (Europe)",          0 )
COMP( 1983, m5p_brno, m5,     0,      brno,    m5,    brno_state, empty_init, "Sord",  "m.5 (Europe) BRNO mod", 0 )
