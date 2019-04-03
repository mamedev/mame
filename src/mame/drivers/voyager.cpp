// license:BSD-3-Clause
// copyright-holders:R. Belmont
/************************************************************************************

Star Trek Voyager (c) 2002 Team Play, Inc. / Game Refuge / Monaco Entertainment
Police Trainer 2 (c) 2003 Team Play, Inc. / Phantom Systems

skeleton driver by R. Belmont

All of these games run Linux.

Motherboard is FIC AZIIEA with AMD Duron processor of unknown speed
Chipset: VIA KT133a with VT8363A Northbridge and VT82C686B Southbridge
Video: Jaton 3DForce2MX-32, based on Nvidia GeForce 2MX chipset w/32 MB of VRAM
I/O: JAMMA adapter board connects to parallel port, VGA out, audio out.
    Labelled "MEGAJAMMA 101 REV A2" for the stand-up Voyager

HDD for stand-up Voyager is a Maxtor D740X-6L 20 GB model.

Upright Voyager runs at 15 kHz standard res, sit-down at 24 kHz medium res.

TODO: VIA KT133a chipset support, GeForce 2MX video support, lots of things ;-)

*************************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/idectrl.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/pcshare.h"
#include "video/pc_vga.h"

#include "bus/isa/trident.h"

#include "screen.h"
#include "speaker.h"


class voyager_state : public pcat_base_state
{
public:
	voyager_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	void voyager(machine_config &config);

	void init_voyager();

private:
	std::unique_ptr<uint32_t[]> m_bios_ram;
	uint8_t m_mtxc_config_reg[256];
	uint8_t m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER(bios_ram_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
	void voyager_io(address_map &map);
	void voyager_map(address_map &map);

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

uint8_t voyager_state::mtxc_config_r(int function, int reg)
{
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return m_mtxc_config_reg[reg];
}

void voyager_state::mtxc_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:MTXC: write %d, %02X, %02X\n", machine().describe_context().c_str(), function, reg, data);

	switch(reg)
	{
		//case 0x59:
		case 0x63:  // PAM0
		{
			//if (data & 0x10)     // enable RAM access to region 0xf0000 - 0xfffff
			if ((data & 0x50) | (data & 0xA0))
			{
				membank("bank1")->set_base(m_bios_ram.get());
			}
			else                // disable RAM access (reads go to BIOS ROM)
			{
				//Execution Hack to avoid crash when switch back from Shadow RAM to Bios ROM, since i386 emu haven't yet pipelined execution structure.
				//It happens when exit from BIOS SETUP.
				#if 0
				if ((m_mtxc_config_reg[0x63] & 0x50) | ( m_mtxc_config_reg[0x63] & 0xA0)) // Only DO if comes a change to disable ROM.
				{
					if (m_maincpu->pc()==0xff74e) m_maincpu->set_pc(0xff74d);
				}
				#endif

				membank("bank1")->set_base(memregion("bios")->base() + 0x10000);
				membank("bank1")->set_base(memregion("bios")->base());
			}
			break;
		}
	}

	m_mtxc_config_reg[reg] = data;
}

void voyager_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

uint32_t voyager_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
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

void voyager_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
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

uint8_t voyager_state::piix4_config_r(int function, int reg)
{
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return m_piix4_config_reg[function][reg];
}

void voyager_state::piix4_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine().describe_context().c_str(), function, reg, data);
	m_piix4_config_reg[function][reg] = data;
}

uint32_t voyager_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
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

void voyager_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
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

WRITE32_MEMBER(voyager_state::bios_ram_w)
{
	//if (m_mtxc_config_reg[0x59] & 0x20)       // write to RAM if this region is write-enabled
			if (m_mtxc_config_reg[0x63] & 0x50)
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

void voyager_state::voyager_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(trident_vga_device::mem_r), FUNC(trident_vga_device::mem_w)); // VGA VRAM
	map(0x000c0000, 0x000c7fff).ram().region("video_bios", 0);
	map(0x000c8000, 0x000cffff).noprw();
	//AM_RANGE(0x000d0000, 0x000d0003) AM_RAM  // XYLINX - Sincronus serial communication
	map(0x000d0008, 0x000d000b).nopw(); // ???
	map(0x000d0800, 0x000d0fff).rom().region("nvram", 0); //
//  AM_RANGE(0x000d0800, 0x000d0fff) AM_RAM  // GAME_CMOS

	//GRULL AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	//GRULL-AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	//GRULL AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE(bios_ram_w)
	map(0x000e0000, 0x000fffff).bankr("bank1");
	map(0x000e0000, 0x000fffff).w(FUNC(voyager_state::bios_ram_w));
	map(0x00100000, 0x03ffffff).ram();  // 64MB
	map(0x04000000, 0x28ffffff).noprw();
	//AM_RANGE(0x04000000, 0x040001ff) AM_RAM
	//AM_RANGE(0x08000000, 0x080001ff) AM_RAM
	//AM_RANGE(0x0c000000, 0x0c0001ff) AM_RAM
	//AM_RANGE(0x10000000, 0x100001ff) AM_RAM
	//AM_RANGE(0x14000000, 0x140001ff) AM_RAM
	//AM_RANGE(0x18000000, 0x180001ff) AM_RAM
	//AM_RANGE(0x20000000, 0x200001ff) AM_RAM
	//AM_RANGE(0x28000000, 0x280001ff) AM_RAM
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void voyager_state::voyager_io(address_map &map)
{
	pcat32_io_common(map);

	//AM_RANGE(0x00e8, 0x00eb) AM_NOP
	map(0x00e8, 0x00ef).noprw(); //AMI BIOS write to this ports as delays between I/O ports operations sending al value -> NEWIODELAY
	map(0x0170, 0x0177).noprw(); //To debug
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
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
	// AM_RANGE(0x0300, 0x03af) AM_NOP
	// AM_RANGE(0x03b0, 0x03df) AM_NOP
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));
	map(0x03f8, 0x03ff).noprw(); // To debug Serial Port COM1:
	map(0x0a78, 0x0a7b).nopw();//AM_WRITE(pnp_data_w)
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
	map(0x42e8, 0x43ef).noprw(); //To debug
	map(0x43c0, 0x43cf).ram().share("share1");
	map(0x46e8, 0x46ef).noprw(); //To debug
	map(0x4ae8, 0x4aef).noprw(); //To debug
	map(0x83c0, 0x83cf).ram().share("share1");
	map(0x92e8, 0x92ef).noprw(); //To debug

}

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)



#if 1
static INPUT_PORTS_START( voyager )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0010, "T",            KEYCODE_T           ) /* T                           14  94 */
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x0100, "O",            KEYCODE_O           ) /* O                           18  98 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0001, "B",            KEYCODE_B           ) /* B                           30  B0 */
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */
//  AT_KEYB_HELPER( 0x8000, "F5",           KEYCODE_F5          )

	PORT_START("pc_keyboard_4")
//  AT_KEYB_HELPER( 0x0004, "F8",           KEYCODE_F8          )

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",        KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                      KEYCODE_A           ) /* Delete                      6f  ef */

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
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
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

void voyager_state::machine_start()
{
}

void voyager_state::machine_reset()
{
	//membank("bank1")->set_base(memregion("bios")->base() + 0x10000);
	membank("bank1")->set_base(memregion("bios")->base());
}

MACHINE_CONFIG_START(voyager_state::voyager)
	PENTIUM3(config, m_maincpu, 133000000); // actually AMD Duron CPU of unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &voyager_state::voyager_map);
	m_maincpu->set_addrmap(AS_IO, &voyager_state::voyager_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, DEVICE_SELF, voyager_state, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, DEVICE_SELF, voyager_state, intel82371ab_pci_r, intel82371ab_pci_w)

	/* video hardware */
	pcvideo_trident_vga(config);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
MACHINE_CONFIG_END

void voyager_state::init_voyager()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x20000/4);

	intel82439tx_init();
}

// unknown version and cabinet style, but believed to be the deluxe sit-down.
ROM_START( voyager )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "stv.u23", 0x000000, 0x040000, CRC(0bed28b6) SHA1(8e7f17af65ca9d17c5c7ddedb2313507d0ea8181) )

	ROM_REGION( 0x8000, "video_bios", 0 )   // incorrect, need GeForce 2MX BIOS for 32MB card
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) BAD_DUMP SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "voyager", 0, SHA1(8b94f2420f6abb40148e4ba6eed8819d8e85dbde))
ROM_END

// upright version 1.002
ROM_START( voyagers )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "stv.u23", 0x000000, 0x040000, CRC(0bed28b6) SHA1(8e7f17af65ca9d17c5c7ddedb2313507d0ea8181) )

	ROM_REGION( 0x8000, "video_bios", 0 )   // incorrect, need GeForce 2MX BIOS for 32MB card
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) BAD_DUMP SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "voyagers", 0, SHA1(839527eee24272e5ad59b871975feadfdfc07a9a))
ROM_END

ROM_START( policet2 )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "pm29f002t.u22", 0x000000, 0x040000, CRC(eb32ace6) SHA1(1b1eeb07e20822c690d05959077c7ddcc22d1708) )

	ROM_REGION( 0x8000, "video_bios", 0 )   // incorrect, need GeForce 2MX BIOS for 32MB card
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) BAD_DUMP SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE_READONLY( "pt2", 0, SHA1(11d29548c685f12bc9bc1db7791957cd5e62db10))
ROM_END

GAME( 2002, voyager,  0,       voyager, voyager, voyager_state, init_voyager, ROT0, "Team Play/Game Refuge/Monaco Entertainment", "Star Trek: Voyager", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 2002, voyagers, voyager, voyager, voyager, voyager_state, init_voyager, ROT0, "Team Play/Game Refuge/Monaco Entertainment", "Star Trek: Voyager (stand-up version 1.002)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 2003, policet2, 0,       voyager, voyager, voyager_state, init_voyager, ROT0, "Team Play/Phantom Entertainment", "Police Trainer 2", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
