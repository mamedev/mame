// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Grull Osgo, Peter Ferrie
/************************************************************************************

California Chase (c) 1999 The Game Room
Eggs Playing Chicken (c) 2002 The Game Room
Host Invaders (c) 1998 The Game Room

driver by Angelo Salese & Grull Osgo

Hardware is known as "AUSCOM System 1" hardware.

Other games known to be on this hardware (if ever finished):
- Agro (c) The Game Room
- Tiger Odds (c) The Game Room
- one other unnamed/unfinished game

Notes:
- calchase: type boot to load game

TODO:
- get Win 98 to boot (most of Windows 98 copy is damaged inside current HDD dump);
- Various graphics bugs (title screen uses ROZ?);
- fix 129 Hz refresh rate bug;
- inputs (is there a service mode?)

I/O Memo (http://bochs.sourceforge.net/techspec/PORTS.LST):
46E8    ----    8514/A and compatible video cards (e.g. ATI Graphics Ultra)
46E8    w   ROM page select
83C0-83CF ----  Compaq QVision - Line Draw Engine
83C4      ----  Compaq Qvision EISA - Virtual Controller Select
83C6-83C9 ----  Compaq Qvision EISA - DAC color registers

43c4 is a 83c4 mirror?

=====================================================================================

California Chase
The Game Room, 1999

This is a rip off of Chase HQ running on PC hardware
and standard 15kHz arcade monitor

Main board is a cheap Taiwanese one made by SOYO.
Model SY-5EAS
Major main board chips are....
Cyrix 686MX-PR200 CPU
1M BIOS (Winbond 29C011)
64M RAM (2x 32M 72 pin SIMMs)
ETEQ EQ82C6617'97 MB817A0AG12 (QFP100, x2)
UT6164C32Q-6 (QFP100, x2)
ETEQ 82C6619'97 MB13J15001 (QFP208)
ETEQ 82C6618'97 MB14B10971 (QFP208)
UM61256
PLL52C61
SMC FD37C669QF (QFP100)
3V coin battery
3x PCI slots
4x 16-bit ISA slots
4x 72 pin RAM slots
connectors for COM1, COM2, LPT1, IDE0, IDE1, floppy etc
uses standard AT PSU

Video card is Trident TGUI9680 with 512k on-board VRAM
Card is branded "Union UTD73" - these are all over eBay, for instance
RAM is AS4C256K16EO-50JC (x2)
Trident BIOS V5.5 (DIP28). Actual size unknown, dumped as 64k, 128k, 256k and 512k (can only be one of these sizes)
6.5536MHz xtal

Custom JAMMA I/O board plugs into one ISA slot
Major components are...
XILINX XC3042
Dallas DS1220Y NVRAM (dumped)
MACH210 (protected)
16MHz OSC
VGA connector (tied to VGA card with a cable)
2x 4-pin connectors for controls
AD7547
ULN2803
LM324 (op amp)
TDA1552 (power amp)
ST TS272 (dual op amp)
2 volume pots
power input connector (from AT PSU)
JAMMA edge connector
ISA edge connector

HDD is WD Caviar 2170. C/H/S = 1010/6/55. Capacity = 170.6MB
The HDD is DOS-readable and in fact the OS is just Windows 98 DOS and can
be easily copied. Tested with another HDD.... formatted with DOS, copied
all files across to new HDD, boots up fine.


Host Invaders is the same motherboard and video card as above, but instead of an HDD,
there is a CD-ROM drive.

************************************************************************************/
/*
Grull Osgo - Improvements

-Changes about BIOS memory management so ROM Shadow now works properly.
 The changes are:
    Rom Memory Map remmapped to 128K size map(0xfffe0000, 0xffffffff).rom().region("bios", 0);

-Changes in mtxc write handler and bios_ram write handler. Now The internal register access are
compatible with chipset VIA.
 (this motherboard has VIA Apollo VXPro chipset. It is not compatible with Intel i430).
 With this changes now BIOS Shadow ram works fine, BIOS can relocate and decompress the full code
 necesary to run the Extended Bios, POST and Boot). No more BIOS Checksum error.

- Suppressed all video related items wich will be replaced with VGA driver.

- Temporarily added a VGA driver that is working based on original IBM VGA BIOS.(From MESS)
  (This VGA driver doesn't work yet with TRIDENT VGA BIOS as I can see).

- Added the flag READONLY to the calchase imagen rom load function, to avoid
  "DIFF CHD ERROR".

- Minor changes and NOPS into address maps for debugging purposes.

- Now Bios is looking for the disk (BIOS Auto detection). It seems all works fine but must be there
something wrong in the disk geometry reported by calchase.chd (20,255,63) since BIOS does not accept
 255 heads as parameter. Perhaps a bad dump?

 TODO: A lot of work to do yet!!!

- update by peter ferrie:
- corrected memory map to 64kb blocks
- corrected access to PAM register
 */


#include "emu.h"
#include "bus/isa/trident.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "machine/pcshare.h"
#include "machine/ds128x.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pc_vga.h"
#include "screen.h"
#include "speaker.h"


class calchase_state : public pcat_base_state
{
public:
	calchase_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	std::unique_ptr<uint32_t[]> m_bios_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext_ram;
	uint8_t m_mtxc_config_reg[256];
	uint8_t m_piix4_config_reg[4][256];

	uint32_t m_idle_skip_ram;
	DECLARE_WRITE32_MEMBER(bios_ext_ram_w);
	DECLARE_WRITE32_MEMBER(bios_ram_w);
	DECLARE_READ16_MEMBER(calchase_iocard1_r);
	DECLARE_READ16_MEMBER(calchase_iocard2_r);
	DECLARE_READ16_MEMBER(calchase_iocard3_r);
	DECLARE_READ16_MEMBER(calchase_iocard4_r);
	DECLARE_READ16_MEMBER(calchase_iocard5_r);
	DECLARE_READ32_MEMBER(calchase_idle_skip_r);
	DECLARE_WRITE32_MEMBER(calchase_idle_skip_w);
	void init_calchase();
	void init_hostinv();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
	void calchase(machine_config &config);
	void hostinv(machine_config &config);
	void calchase_io(address_map &map);
	void calchase_map(address_map &map);

	uint8_t mtxc_config_r(int function, int reg);
	void mtxc_config_w(int function, int reg, uint8_t data);
	uint32_t intel82439tx_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint8_t piix4_config_r(int function, int reg);
	void piix4_config_w(int function, int reg, uint8_t data);
	uint32_t intel82371ab_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};

// Intel 82439TX System Controller (MTXC)
// TODO: change with a VIA82C585VPX (North Bridge - APOLLO Chipset)

uint8_t calchase_state::mtxc_config_r(int function, int reg)
{
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return m_mtxc_config_reg[reg];
}

void calchase_state::mtxc_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:MTXC: write %d, %02X, %02X\n", machine().describe_context().c_str(), function, reg, data);

	/*
	memory banking with North Bridge:
	0x63 (PAM)  xx-- ---- BIOS extension 0xe0000 - 0xeffff
	            --xx ---- BIOS area 0xf0000-0xfffff
	            ---- xx-- ISA add-on BIOS 0xc0000 - 0xcffff
	            ---- --xx ISA add-on BIOS 0xd0000 - 0xdffff

	10 -> 1 = Write Enable, 0 = Read Enable
	*/

	if (reg == 0x63)
	{
		if (data & 0x20)        // enable RAM access to region 0xf0000 - 0xfffff
			membank("bios_bank")->set_base(m_bios_ram.get());
		else                    // disable RAM access (reads go to BIOS ROM)
			membank("bios_bank")->set_base(memregion("bios")->base() + 0x10000);
		if (data & 0x80)        // enable RAM access to region 0xe0000 - 0xeffff
			membank("bios_ext")->set_base(m_bios_ext_ram.get());
		else
			membank("bios_ext")->set_base(memregion("bios")->base() + 0);
	}

	m_mtxc_config_reg[reg] = data;
}

void calchase_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

uint32_t calchase_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;

	if(reg == 0)
		return 0x05851106; // VT82C585VPX, VIA

	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(function, reg + 0) << 0;
	}
	return r;
}

void calchase_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)
//TODO: change with VIA82C586B (South Bridge - APOLLO Chipset)

uint8_t calchase_state::piix4_config_r(int function, int reg)
{
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return m_piix4_config_reg[function][reg];
}

void calchase_state::piix4_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine().describe_context().c_str(), function, reg, data);
	m_piix4_config_reg[function][reg] = data;
}

uint32_t calchase_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;

	if(reg == 0)
		return 0x30401106; // VT82C586B, VIA

	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(function, reg + 0) << 0;
	}
	return r;
}

void calchase_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

WRITE32_MEMBER(calchase_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x63] & 0x10)       // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

WRITE32_MEMBER(calchase_state::bios_ext_ram_w)
{
	if (m_mtxc_config_reg[0x63] & 0x40)       // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext_ram.get() + offset);
	}
}

READ16_MEMBER(calchase_state::calchase_iocard1_r)
{
	return ioport("IOCARD1")->read();
}

READ16_MEMBER(calchase_state::calchase_iocard2_r)
{
	return ioport("IOCARD2")->read();
}

READ16_MEMBER(calchase_state::calchase_iocard3_r)
{
	return ioport("IOCARD3")->read();
}

/* These two controls wheel pot or whatever this game uses ... */
READ16_MEMBER(calchase_state::calchase_iocard4_r)
{
	return ioport("IOCARD4")->read();
}

READ16_MEMBER(calchase_state::calchase_iocard5_r)
{
	return ioport("IOCARD5")->read();
}


void calchase_state::calchase_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(trident_vga_device::mem_r), FUNC(trident_vga_device::mem_w)); // VGA VRAM
	map(0x000c0000, 0x000c7fff).ram().region("video_bios", 0);
	map(0x000c8000, 0x000cffff).noprw();
	//map(0x000d0000, 0x000d0003).ram();  // XYLINX - Sincronus serial communication
	map(0x000d0004, 0x000d0005).r(FUNC(calchase_state::calchase_iocard1_r));
	map(0x000d000c, 0x000d000d).r(FUNC(calchase_state::calchase_iocard2_r));
	map(0x000d0032, 0x000d0033).r(FUNC(calchase_state::calchase_iocard3_r));
	map(0x000d0030, 0x000d0031).r(FUNC(calchase_state::calchase_iocard4_r));
	map(0x000d0034, 0x000d0035).r(FUNC(calchase_state::calchase_iocard5_r));
	map(0x000d0008, 0x000d000b).nopw(); // ???
	map(0x000d0024, 0x000d0025).w("ldac", FUNC(dac_word_interface::data_w));
	map(0x000d0028, 0x000d0029).w("rdac", FUNC(dac_word_interface::data_w));
	map(0x000d0800, 0x000d0fff).rom().region("nvram", 0); //
//  map(0x000d0800, 0x000d0fff).ram();  // GAME_CMOS

	map(0x000e0000, 0x000effff).bankr("bios_ext").w(FUNC(calchase_state::bios_ext_ram_w));
	map(0x000f0000, 0x000fffff).bankr("bios_bank").w(FUNC(calchase_state::bios_ram_w));
	map(0x00100000, 0x03ffffff).ram();  // 64MB
	map(0x04000000, 0x28ffffff).noprw();
	//map(0x04000000, 0x040001ff).ram();
	//map(0x08000000, 0x080001ff).ram();
	//map(0x0c000000, 0x0c0001ff).ram();
	//map(0x10000000, 0x100001ff).ram();
	//map(0x14000000, 0x140001ff).ram();
	//map(0x18000000, 0x180001ff).ram();
	//map(0x20000000, 0x200001ff).ram();
	//map(0x28000000, 0x280001ff).ram();
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void calchase_state::calchase_io(address_map &map)
{
	pcat32_io_common(map);
	//map(0x00e8, 0x00eb).noprw();
	map(0x00e8, 0x00ef).noprw(); //AMI BIOS write to this ports as delays between I/O ports operations sending al value -> NEWIODELAY
	map(0x0170, 0x0177).noprw(); //To debug
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x0200, 0x021f).noprw(); //To debug
	map(0x0260, 0x026f).noprw(); //To debug
	map(0x0278, 0x027b).nopw();//AM_WRITE(pnp_config_w)
	map(0x0280, 0x0287).noprw(); //To debug
	map(0x02a0, 0x02a7).noprw(); //To debug
	map(0x02c0, 0x02c7).noprw(); //To debug
	map(0x02e0, 0x02ef).noprw(); //To debug
	map(0x02f8, 0x02ff).noprw(); //To debug
	map(0x0320, 0x038f).noprw(); //To debug
	map(0x03a0, 0x03a7).noprw(); //To debug
	map(0x03b0, 0x03bf).rw("vga", FUNC(trident_vga_device::port_03b0_r), FUNC(trident_vga_device::port_03b0_w));
	map(0x03c0, 0x03cf).rw("vga", FUNC(trident_vga_device::port_03c0_r), FUNC(trident_vga_device::port_03c0_w));
	map(0x03d0, 0x03df).rw("vga", FUNC(trident_vga_device::port_03d0_r), FUNC(trident_vga_device::port_03d0_w));
	map(0x03e0, 0x03ef).noprw(); //To debug
	map(0x0378, 0x037f).noprw(); //To debug
	// map(0x0300, 0x03af).noprw();
	// map(0x03b0, 0x03df).noprw();
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x03f8, 0x03ff).noprw(); // To debug Serial Port COM1:
	map(0x0a78, 0x0a7b).nopw();//AM_WRITE(pnp_data_w)
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
	map(0x42e8, 0x43ef).noprw(); //To debug
	map(0x43c4, 0x43cb).rw("vga", FUNC(trident_vga_device::port_43c6_r), FUNC(trident_vga_device::port_43c6_w));  // Trident Memory and Video Clock register
	map(0x46e8, 0x46ef).noprw(); //To debug
	map(0x4ae8, 0x4aef).noprw(); //To debug
	map(0x83c4, 0x83cb).rw("vga", FUNC(trident_vga_device::port_83c6_r), FUNC(trident_vga_device::port_83c6_w));  // Trident LUTDAC
	map(0x92e8, 0x92ef).noprw(); //To debug
}

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)



#if 1
static INPUT_PORTS_START( calchase )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
//  AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */
	// 0x0004, KEYCODE_0
	// 0x0008, KEYCODE_1
	// 0x0010, KEYCODE_2
	// 0x0020, KEYCODE_3
	// 0x0040, KEYCODE_4
	// 0x0080, KEYCODE_5
	// 0x0100, KEYCODE_6
	// 0x0200, KEYCODE_7
	// 0x0400, KEYCODE_8
	// 0x0800, KEYCODE_9
	// 0x1000, KEYCODE_MINUS
	// 0x2000, KEYCODE_EQUAL
	// 0x4000, KEYCODE_BACKSPACE
	// 0x8000, KEYCODE_TAB

	PORT_START("pc_keyboard_1")
	// 0x0001, KEYCODE_Q
	// 0x0002, KEYCODE_W
	AT_KEYB_HELPER( 0x0004, "E",            KEYCODE_E           )
	AT_KEYB_HELPER( 0x0008, "R",            KEYCODE_R           )
	AT_KEYB_HELPER( 0x0010, "T",            KEYCODE_T           ) /* T                           14  94 */
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	// 0x0040, KEYCODE_U
	AT_KEYB_HELPER( 0x0080, "I",            KEYCODE_I           )
	AT_KEYB_HELPER( 0x0100, "O",            KEYCODE_O           ) /* O                           18  98 */
	// 0x0200, KEYCODE_P
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */
	AT_KEYB_HELPER( 0x4000, "A",            KEYCODE_A          )
	AT_KEYB_HELPER( 0x8000, "S",            KEYCODE_S           )

	PORT_START("pc_keyboard_2")
	AT_KEYB_HELPER( 0x0001, "D",            KEYCODE_D           )
	AT_KEYB_HELPER( 0x0002, "F",            KEYCODE_F           )


	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0001, "B",            KEYCODE_B           ) /* B                           30  B0 */
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0200, "SPACE",        KEYCODE_SPACE       ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_F1          ) /* F1                          3B  BB */
//  AT_KEYB_HELPER( 0x8000, "F5",           KEYCODE_F5          )

	PORT_START("pc_keyboard_4")
//  AT_KEYB_HELPER( 0x0004, "F8",           KEYCODE_F8          )

	PORT_START("pc_keyboard_5")


	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",    KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                  KEYCODE_DEL           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")

	PORT_START("IOCARD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0008, 0x0008, "1" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("IOCARD2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) // guess
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset SW")
	PORT_DIPNAME( 0x0004, 0x0004, "2" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Turbo")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // returns back to MS-DOS (likely to be unmapped and actually used as a lame protection check)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("IOCARD3")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xdfff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOCARD4")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") //eggsplc
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_START("IOCARD5")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END
#endif

void calchase_state::machine_start()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x10000/4);
	m_bios_ext_ram = std::make_unique<uint32_t[]>(0x10000/4);
}

void calchase_state::machine_reset()
{
	//membank("bank1")->set_base(memregion("bios")->base() + 0x10000);
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x10000);
	membank("bios_ext")->set_base(memregion("bios")->base() + 0);
}

void calchase_state::calchase(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133000000); // Cyrix 686MX-PR200 CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &calchase_state::calchase_map);
	m_maincpu->set_addrmap(AS_IO, &calchase_state::calchase_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device_read (0, FUNC(calchase_state::intel82439tx_pci_r), this);
	pcibus.set_device_write(0, FUNC(calchase_state::intel82439tx_pci_w), this);
	pcibus.set_device_read (7, FUNC(calchase_state::intel82371ab_pci_r), this);
	pcibus.set_device_write(7, FUNC(calchase_state::intel82371ab_pci_w), this);

	/* video hardware */
	pcvideo_trident_vga(config);

	ds12885_device &rtc(DS12885(config.replace(), "rtc"));
	rtc.irq().set("pic8259_2", FUNC(pic8259_device::ir0_w));
	rtc.set_century_index(0x32);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_12BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25); // unknown DAC
	DAC_12BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "ldac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "rdac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);
}

void calchase_state::hostinv(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133000000); // Cyrix 686MX-PR200 CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &calchase_state::calchase_map);
	m_maincpu->set_addrmap(AS_IO, &calchase_state::calchase_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "cdrom", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device_read (0, FUNC(calchase_state::intel82439tx_pci_r), this);
	pcibus.set_device_write(0, FUNC(calchase_state::intel82439tx_pci_w), this);
	pcibus.set_device_read (7, FUNC(calchase_state::intel82371ab_pci_r), this);
	pcibus.set_device_write(7, FUNC(calchase_state::intel82371ab_pci_w), this);

	/* video hardware */
	pcvideo_trident_vga(config);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_12BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25); // unknown DAC
	DAC_12BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "ldac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "rdac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);
}


READ32_MEMBER(calchase_state::calchase_idle_skip_r)
{
	if(m_maincpu->pc()==0x1406f48)
		m_maincpu->spin_until_interrupt();

	return m_idle_skip_ram;
}

WRITE32_MEMBER(calchase_state::calchase_idle_skip_w)
{
	COMBINE_DATA(&m_idle_skip_ram);
}

void calchase_state::init_calchase()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x20000/4);

	intel82439tx_init();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x3f0b160, 0x3f0b163, read32_delegate(FUNC(calchase_state::calchase_idle_skip_r),this), write32_delegate(FUNC(calchase_state::calchase_idle_skip_w),this));
}

void calchase_state::init_hostinv()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x20000/4);

	intel82439tx_init();
}

ROM_START( calchase )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "mb_bios.bin", 0x00000, 0x20000, CRC(dea7a51b) SHA1(e2028c00bfa6d12959fc88866baca8b06a1eab68) )

	ROM_REGION( 0x8000, "video_bios", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "ds1220y_nv.bin", 0x000, 0x800, CRC(7912c070) SHA1(b4c55c7ca76bcd8dad1c4b50297233349ae02ed3) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "calchase", 0,BAD_DUMP SHA1(6ae51a9b3f31cf4166322328a98c0235b0874eb3) )
ROM_END

ROM_START( hostinv )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x8000, "video_bios", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_hostinv.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "ide:0:cdrom:image" )
	DISK_IMAGE_READONLY( "hostinv", 0, SHA1(3cb86c62e80be98a717172b717f7276a0e5f6830) )
ROM_END

ROM_START( eggsplc )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "hostinv_bios.bin", 0x000000, 0x020000, CRC(5111e4b8) SHA1(20ab93150b61fd068f269368450734bba5dcb284) )

	ROM_REGION( 0x8000, "video_bios", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASEFF )
	ROM_LOAD( "ds1220y_eggsplc.bin", 0x000, 0x800, NO_DUMP )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "eggsplc", 0, SHA1(fa38dd6b0d25cde644f68cf639768f137c607eb5) )
ROM_END

GAME( 1998, hostinv,  0, hostinv,  calchase, calchase_state, init_hostinv,  ROT0, "The Game Room", "Host Invaders",        MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, calchase, 0, calchase, calchase, calchase_state, init_calchase, ROT0, "The Game Room", "California Chase",     MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 2002, eggsplc,  0, calchase, calchase, calchase_state, init_hostinv,  ROT0, "The Game Room", "Eggs Playing Chicken", 0 )
