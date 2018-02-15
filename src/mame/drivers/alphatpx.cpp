// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Triumph-Adler Alphatronic Px series
    ===================================

    The Px series was designed by SKS (Steinmetz-Krischke Systemtechnik), like the ITT3030 and the SKS Nano,
    the boards are closely related.

    Keyboard and floppy stuff was copypasted from ITT3030 and adapted to the best of knowledge.

    P1, P2 and P2S: no paging
    Lower 16K for P1, P2 and P2 S:

    0x0000 - 0x17ff ROM monitor (MOS)
    0x1800 - 0x1bff 1K RAM
    0x1c00 - 0x1fff reserved
    0x2000 - 0x2fff reserved, belonging to the video card's memory space (video ROM?)
    0x3000 - 0x3fff actual video memory

    P1
    ==
    Upper 32K:
    0x4000 - 0x400a reserved
    0x4010 - 0xc000 32K RAM
    1x 160K, single sided, 40 tracks, 16 sectors/track, 256 bytes/sector floppy disk drive

    P2, P2S
    =======
    Upper 48K:
    0x4000 - 0x400a reserved
    0x4010 - 0xfff 48K RAM
    P2: 2x 160K, single sided, 40 tracks, 16 sectors/track, 256 bytes/sector floppy disk drives
    There is a non-standard CP/M for the P2 and P2S with the program origin at 0x4300 instead of 0x0100

    P2U
    ===
    For paging via port 0x78A, a 16K RAM card with RAM at 0x0000 and 0x3fff and the banking logic (see above) is added to the the standard 48K memory card.
    P2S, P2U: 2x 320K, double sided, 40 tracks, 16 sectors/track, 256 bytes/sector floppy disk drives

    P3, P4
    ======
    0x0000 - 0x0fff ROM monitor (MOS)
    0x1000 - 0x17ff free
    0x1800 - 0x1bff monitor stack (RAM)
    0x1c00 - 0x2fff free
    0x3000 - 0x3fff video memory
    0x4000 - 0xffff RAM
    P3: 2x785K, double sided, 80 tracks, 5 sectors/track, 1024 bytes/sector floppy disk drives
    P4: 1x785K, double sided, 80 tracks, 5 sectors/track, 1024 bytes/sector floppy disk drive, 1x harddisk 360 tracks, 4 heads, 17 sectors/track, 512 bytes/sector

    P2U and P3 support regular CP/M use with a full 64K RAM complement.
    Still, the video RAM is at 0x3000 and 0x3ffff even for these machines, and from what I've read they also use the routine present in the ROM monitor, the MOS.
    That means, that in order to update the video RAM and probably other I/O the lower 16K (page 0) are constantly paged in and paged out.
    This is accomplished by writing 2FH to port 0x78 in order to switch in the ROM (and assorted, see below) area and by writing 63H to port 0x78 in order to swap the full 64K RAM back in.

    P30 and P40
    ===========
    Those were P3 and P4's with an additional 8088 card, a 128K RAM card (some with an extra 32K graphics extension) to support MS-DOS.
	
	
	comments, testing, modification: rfka01, helwie44

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/pic8259.h"
#include "video/tms9927.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"
#include "debugger.h"


//**************************************************************************
//  TYPE DEFINITIONS - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

class alphatp_12_state : public driver_device
{
public:
	alphatp_12_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bankdev(*this, "bankdev"),
		m_kbdmcu(*this, "kbdmcu"),
		m_crtc(*this, "crtc"),
		m_fdc (*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_beep(*this, "beeper"),
		m_keycols(*this, "COL.%u", 0),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_gfx(*this, "gfx"),
		m_ram(*this, "ram")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ_LINE_MEMBER(kbd_matrix_r);
	DECLARE_WRITE8_MEMBER(kbd_matrix_w);
	DECLARE_READ8_MEMBER(kbd_port2_r);
	DECLARE_WRITE8_MEMBER(kbd_port2_w);

	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(fdc_stat_r);
	DECLARE_WRITE8_MEMBER(fdc_cmd_w);

	DECLARE_WRITE_LINE_MEMBER(fdcirq_w);
	DECLARE_WRITE_LINE_MEMBER(fdcdrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdchld_w);
	DECLARE_WRITE8_MEMBER(beep_w);
	DECLARE_WRITE8_MEMBER(bank_w);

	void alphatp1(machine_config &config);
	void alphatp2(machine_config &config);
	void alphatp2u(machine_config &config);

	void alphatp2_io(address_map &map);
	void alphatp2_map(address_map &map);
	void alphatp2_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<address_map_bank_device> m_bankdev;
	required_device<i8041_device> m_kbdmcu;
	required_device<crt5027_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<beep_device> m_beep;
	required_ioport_array<16> m_keycols;

private:
	uint8_t m_kbdclk, m_kbdread, m_kbdport2;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_gfx;
	required_shared_ptr<u8> m_ram;
	floppy_image_device *m_curfloppy;
	bool m_fdc_irq, m_fdc_drq, m_fdc_hld;
};

//**************************************************************************
//  TYPE DEFINITIONS - Alphatronic P3, P4, P30 and P4
//**************************************************************************

class alphatp_34_state : public driver_device
{
public:
	alphatp_34_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bankdev(*this, "bankdev"),
		m_kbdmcu(*this, "kbdmcu"),
		m_crtc(*this, "crtc"),
		m_fdc (*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_i8088(*this, "i8088"),
		m_beep(*this, "beeper"),
		m_keycols(*this, "COL.%u", 0),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_gfx(*this, "gfx"),
		m_ram(*this, "ram")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ_LINE_MEMBER(kbd_matrix_r);
	DECLARE_WRITE8_MEMBER(kbd_matrix_w);
	DECLARE_READ8_MEMBER(kbd_port2_r);
	DECLARE_WRITE8_MEMBER(kbd_port2_w);

	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(fdc_stat_r);
	DECLARE_WRITE8_MEMBER(fdc_cmd_w);

	DECLARE_WRITE_LINE_MEMBER(fdcirq_w);
	DECLARE_WRITE_LINE_MEMBER(fdcdrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdchld_w);
	DECLARE_WRITE8_MEMBER(beep_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(start88_r);

	void alphatp3(machine_config &config);
	void alphatp30(machine_config &config);

	void alphatp30_8088_io(address_map &map);
	void alphatp30_8088_map(address_map &map);
	void alphatp3_io(address_map &map);
	void alphatp3_map(address_map &map);
	void alphatp3_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<address_map_bank_device> m_bankdev;
	required_device<i8041_device> m_kbdmcu;
	required_device<crt5037_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	optional_device<cpu_device> m_i8088;
	required_device<beep_device> m_beep;
	required_ioport_array<16> m_keycols;

private:
	uint8_t m_kbdclk, m_kbdread, m_kbdport2;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_gfx;
	required_shared_ptr<u8> m_ram;
	floppy_image_device *m_curfloppy;
	bool m_fdc_irq, m_fdc_drq, m_fdc_hld;
};

//**************************************************************************
//  ADDRESS MAPS - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

ADDRESS_MAP_START(alphatp_12_state::alphatp2_mem)
	AM_RANGE(0x0000, 0xffff) AM_DEVICE("bankdev", address_map_bank_device, amap8)
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_12_state::alphatp2_map)
	AM_RANGE(0x00000, 0x0ffff) AM_RAMBANK("ram_0000")
	AM_RANGE(0x00000, 0x017ff) AM_ROM AM_REGION("boot", 0)  // P2  0x0000 , 0x17ff -hw 6kB, P3 only 4 kB
	AM_RANGE(0x01800, 0x01c00) AM_RAM // boot rom variables
	AM_RANGE(0x03000, 0x03bff) AM_WRITEONLY AM_SHARE("vram") // test  2017 hw, MOS directly writes to display RAM
	AM_RANGE(0x03FF0, 0x03fff) AM_DEVWRITE("crtc", crt5027_device, write) //test hw, mem-mapped registers, cursor position can be determined through this range

	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_12_state::alphatp2_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("kbdmcu", i8041_device, upi41_master_r, upi41_master_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(beep_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x54, 0x54) AM_READWRITE(fdc_stat_r, fdc_cmd_w)
	AM_RANGE(0x78, 0x78) AM_WRITE(bank_w)
ADDRESS_MAP_END


WRITE8_MEMBER(alphatp_12_state::bank_w)
{
	m_bankdev->set_bank(BIT(data, 6));
}


//**************************************************************************
//  ADDRESS MAPS - Alphatronic P3, P4, P30 and P40
//**************************************************************************

ADDRESS_MAP_START(alphatp_34_state::alphatp3_mem)
	AM_RANGE(0x0000, 0xffff) AM_DEVICE("bankdev", address_map_bank_device, amap8)
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_34_state::alphatp3_map)
	AM_RANGE(0x00000, 0x0ffff) AM_RAMBANK("ram_0000")
	AM_RANGE(0x00000, 0x017ff) AM_ROM AM_REGION("boot", 0)  // P2  0x0000 , 0x17ff -hw 6kB, P3 only 4 kB
	AM_RANGE(0x01800, 0x01c00) AM_RAM // boot rom variables
	AM_RANGE(0x03000, 0x03bff) AM_WRITEONLY AM_SHARE("vram") // test  2017 hw, MOS directly writes to display RAM
	AM_RANGE(0x03FF0, 0x03fff) AM_DEVWRITE("crtc", crt5037_device, write) //test hw, mem-mapped registers, cursor position can be determined through this range

	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_34_state::alphatp3_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("kbdmcu", i8041_device, upi41_master_r, upi41_master_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(beep_w)
	AM_RANGE(0x40, 0x40) AM_READ(start88_r)
	AM_RANGE(0x50, 0x53) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x54, 0x54) AM_READWRITE(fdc_stat_r, fdc_cmd_w)
	AM_RANGE(0x78, 0x78) AM_WRITE(bank_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_34_state::alphatp30_8088_map)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("16bit", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(alphatp_34_state::alphatp30_8088_io)
	AM_RANGE(0xffe0, 0xffe1) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
ADDRESS_MAP_END

READ8_MEMBER(alphatp_34_state::start88_r)
{
	if(m_i8088)
		m_i8088->resume(SUSPEND_REASON_DISABLE);
	return 0;
}

WRITE8_MEMBER(alphatp_34_state::bank_w)
{
	m_bankdev->set_bank(BIT(data, 6));
}

//**************************************************************************
//  INPUTS -  Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

READ_LINE_MEMBER(alphatp_12_state::kbd_matrix_r)
{
	return m_kbdread;
}

WRITE8_MEMBER(alphatp_12_state::kbd_matrix_w)
{
	if ((data & 0x80) && (!m_kbdclk))
	{
		const ioport_value tmp_read = m_keycols[(data >> 3) & 0xf]->read() & (1 << (data & 0x7));
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;

}

// bit 2 is UPI-41 host IRQ to Z80
WRITE8_MEMBER(alphatp_12_state::kbd_port2_w)
{
	m_kbdport2 = data;

}

READ8_MEMBER(alphatp_12_state::kbd_port2_r)
{
	return m_kbdport2;
}

//**************************************************************************
//  INPUTS - Alphatronic P3, P4, P30 and P40
//**************************************************************************

READ_LINE_MEMBER(alphatp_34_state::kbd_matrix_r)
{
	return m_kbdread;
}

WRITE8_MEMBER(alphatp_34_state::kbd_matrix_w)
{
	if ((data & 0x80) && (!m_kbdclk))
	{
		const ioport_value tmp_read = m_keycols[(data >> 3) & 0xf]->read() & (1 << (data & 0x7));
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;

}

// bit 2 is UPI-41 host IRQ to Z80
WRITE8_MEMBER(alphatp_34_state::kbd_port2_w)
{
	m_kbdport2 = data;

}

READ8_MEMBER(alphatp_34_state::kbd_port2_r)
{
	return m_kbdport2;
}


//**************************************************************************
//  KEYBOARD - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

/*
P2 keyboard:

[   ][ ! ][ " ][ § ][ $ ][ % ][ & ][ / ][ ( ][ ) ][ = ][ ? ][ ` ][ F ][ F ][ F ][ F ][ F ][ F ]
[_SM][_1_][_2_][_3_][_4_][_5_][_6_][_7_][_8_][_9_][_0_][_ß_][_´_][_1_][_2_][_3_][_4_][_5_][_6_]
[     ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][ * ][   ]   [   ][   ][   ][   ]
[_(C)_][_Q_][_W_][_E_][_R_][_T_][_Z_][_U_][_I_][_O_][_P_][_Ü_][_+_][_TB]   [_7_][_8_][_9_][_/_]
[      ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][ ^ ][   | ][   ][   ][   ][   ]
[__CL__][_A_][_S_][_D_][_F_][_G_][_H_][_J_][_K_][_L_][_Ö_][_Ä_][_#_][_<=| ][_4_][_5_][_6_][_x_]
[    ][ > ][   ][   ][   ][   ][   ][   ][   ][ ; ][ : ][ _ ][    ][[] ]   [   ][   ][   ][   ]
[_SH_][ < ][_Y_][_X_][_C_][_V_][_B_][_N_][_M_][_,_][_._][_-_][_SH_][ []]   [_1_][_2_][_3_][_-_]
[   ][   ][   ][                                      ][   ][   ][   ][   ][        ][   ][   ]
[(R)][_UP][LFT][_______________SPACE__________________][RGH][DWN][PS1][ C ][____0___][_._][_+_]

The SH(ift), C(aps)L(ock) and Space keys are unmarked, to the right of + / * is unmarked as well.
SM is "Schreibmaschinen-Modus", typewriter mode
The direction keys and Pos1 are marked with arrows
<- moves the cursor left one position and deletes the character
-> moves the cursor right one position
(C) has a red LED and is the RESET key
The C key is called "Korrektur" in the manual. You have to press it if the keyboard locks up, e.g. when characters have been
entered too quickly.
[]
 [] is two overlapping squares, the manual calls it "Kontroll-Taste", i.e. CTRL
TB is the TAB key and is unmarked
*/

// translation table at offset 0xdc0 in the main rom
static INPUT_PORTS_START( alphatp2 )

PORT_START("COL.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)        PORT_CHAR('w')      PORT_CHAR('W')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)        PORT_CHAR('s')      PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)        PORT_CHAR('x')      PORT_CHAR('X')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT")        PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(8)                    // left arrow works as backspace
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)        PORT_CHAR('q')      PORT_CHAR('Q')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1)        PORT_CHAR('1')      PORT_CHAR('!')

PORT_START("COL.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)        PORT_CHAR('e')      PORT_CHAR('E')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)        PORT_CHAR('d')      PORT_CHAR('D')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)        PORT_CHAR('c')      PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE")       PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5")          PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5)) // SCAN:=0Dh ->8Ah-funct F5 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2)        PORT_CHAR('2')      PORT_CHAR('"')

PORT_START("COL.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)        PORT_CHAR('r')      PORT_CHAR('R')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)        PORT_CHAR('f')      PORT_CHAR('F')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)        PORT_CHAR('v')      PORT_CHAR('V')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT")       PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 0x82
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6")          PORT_CODE(KEYCODE_F6)       PORT_CHAR(UCHAR_MAMEKEY(F6)) // scan:=15h 8Ch-> F6 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3)        PORT_CHAR('3')      PORT_CHAR(0x00a7)

PORT_START("COL.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)        PORT_CHAR('t')      PORT_CHAR('T')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)        PORT_CHAR('g')      PORT_CHAR('G')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)        PORT_CHAR('b')      PORT_CHAR('B')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN")        PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // 0x8b
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)        PORT_CHAR('a')      PORT_CHAR('A')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4)        PORT_CHAR('4')      PORT_CHAR('$')

PORT_START("COL.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)        PORT_CHAR('z')      PORT_CHAR('Z')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)        PORT_CHAR('h')      PORT_CHAR('H')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)        PORT_CHAR('n')      PORT_CHAR('N')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS")        PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  // scan:=25h ->0xc0 ?capslock ?
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5)        PORT_CHAR('5')      PORT_CHAR('%')

PORT_START("COL.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)        PORT_CHAR('u')      PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)        PORT_CHAR('j')      PORT_CHAR('J')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)        PORT_CHAR('m')      PORT_CHAR('M')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP")          PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))        // 0x89
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)        PORT_CHAR('y')      PORT_CHAR('Y')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6)        PORT_CHAR('6')      PORT_CHAR('&')

PORT_START("COL.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)        PORT_CHAR('i')      PORT_CHAR('I')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)        PORT_CHAR('k')      PORT_CHAR('k')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", ;")         PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',')      PORT_CHAR(';')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<>")          PORT_CODE(KEYCODE_BACKSLASH2)PORT_CHAR('<')     PORT_CHAR('>')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7)        PORT_CHAR('7')      PORT_CHAR('/')

PORT_START("COL.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)        PORT_CHAR('o')      PORT_CHAR('O')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)        PORT_CHAR('l')      PORT_CHAR('L')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". :")         PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.')      PORT_CHAR(':')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pos 1")       PORT_CODE(KEYCODE_HOME)     PORT_CHAR(UCHAR_MAMEKEY(HOME))      // 0x8f
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L_SHIFT")     PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))    // 3Dh ->C1h-function P3 key left
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8)        PORT_CHAR('8')      PORT_CHAR('(')

PORT_START("COL.8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)        PORT_CHAR('p')      PORT_CHAR('P')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ö Ö")         PORT_CODE(KEYCODE_COLON)    PORT_CHAR(0x00f6)   PORT_CHAR(0x00d6)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _")         PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_CTRL")      PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  // 44h ->84h clear ?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER Pad")   PORT_CODE(KEYCODE_ENTER_PAD)PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)        PORT_CHAR('9')      PORT_CHAR(')')

PORT_START("COL.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ü Ü")         PORT_CODE(KEYCODE_OPENBRACE)PORT_CHAR(0x00fc)   PORT_CHAR(0x00dc)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ä Ä")         PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR(0x00e4)   PORT_CHAR(0x00c4)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_SHIFT")     PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 Pad")       PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)        PORT_CHAR('0')      PORT_CHAR('=')

PORT_START("COL.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("+ *")        PORT_CODE(KEYCODE_CLOSEBRACE)PORT_CHAR('+')     PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("# ^")         PORT_CODE(KEYCODE_BACKSLASH)PORT_CHAR('#')      PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[][]/ESC")    PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))           // Esc test this work ?!
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL Pad")     PORT_CODE(KEYCODE_DEL_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ß ?")         PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(0x00df)   PORT_CHAR('?')      // ß and ?

PORT_START("COL.11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')							// TAB key
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 Pad")       PORT_CODE(KEYCODE_1_PAD)    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Pad")       PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("´ `")         PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR(0x00b4)   PORT_CHAR(0x0060)

PORT_START("COL.12")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 Pad")       PORT_CODE(KEYCODE_7_PAD)    PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 Pad")       PORT_CODE(KEYCODE_4_PAD)    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 Pad")       PORT_CODE(KEYCODE_2_PAD)    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")          PORT_CODE(KEYCODE_F4)       PORT_CHAR(UCHAR_MAMEKEY(F4))             // scan 68h -> 88h func. F4

PORT_START("COL.13")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ Pad")       PORT_CODE(KEYCODE_SLASH_PAD)PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*")           PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // test ?
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\xc2\xae /Ctrl")PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_LCONTROL)         // scan 6Bh -> C2h funct.
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)                  // 0xc2 ??
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")          PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3)) // scan:=68h 88h-> F3 ok

PORT_START("COL.14")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 Pad")       PORT_CODE(KEYCODE_9_PAD)    PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 Pad")       PORT_CODE(KEYCODE_6_PAD)    PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- Pad")       PORT_CODE(KEYCODE_MINUS_PAD)PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")          PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))    // 70h -> 87h func.F2 ok

PORT_START("COL.15")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 Pad")       PORT_CODE(KEYCODE_8_PAD)    PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 Pad")       PORT_CODE(KEYCODE_5_PAD)    PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 Pad")       PORT_CODE(KEYCODE_3_PAD)    PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")          PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1))    // 7Dh -> 85H func. F1 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SM")          PORT_CODE(KEYCODE_NUMLOCK)  PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))// SM (typewriter) mode key */
INPUT_PORTS_END

//**************************************************************************
//  KEYBOARD - Alphatronic P3, P4, P30 and P40
//**************************************************************************

/*
P3 keyboard:

 [RST]
 [___]

      [ ! ][ " ][ § ][ $ ][ % ][ & ][ / ][ ( ][ ) ][ = ][ ? ][ ` ][ SM  ]       [(1)][(2)]  [(1)][(2)][(3)][(4)]
      [_1_][_2_][_3_][_4_][_5_][_6_][_7_][_8_][_9_][_0_][_ß_][_´_][_____]       [___][___]  [___][___][___][___]
 [     ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][ * ][[] ][ | ]  [        ]  [   ][   ][   ][ / ]
 [_==>_][_Q_][_W_][_E_][_R_][_T_][_Z_][_U_][_I_][_O_][_P_][_Ü_][_+_][_[]][ | ]  [__POS1__]  [_7_][_8_][_9_][_-_]
  [     ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   ][   <=| ]  [        ]  [   ][   ][   ][ x ]
  [_CL__][_A_][_S_][_D_][_F_][_G_][_G_][_H_][_J_][_K_][_L_][_Ö_][_Ä_][_______]  [___UP___]  [_4_][_5_][_6_][_+_]
[     ][ > ][   ][   ][   ][   ][   ][   ][   ][ ; ][ : ][ _ ][     ]           [   ][   ]  [   ][   ][   ][  |]
[__SH_][_<_][_Y_][_X_][_C_][_V_][_B_][_N_][_M_][_,_][_._][_-_][__SH_]           [_L_][ R ]  [_1_][_2_][_3_][  |]
     [   ][                                            ][   ]                   [        ]  [        ][   ][<=|]
     [(R)][___________________SPACE____________________][ C ]                   [__DOWN__]  [____0___][ . ][___]

RST is a hard reset, the key has a red status LED
The SH(ift), C(aps)L(ock) and Space keys are unmarked
(1)-(6) are function keys with the number in a circle
(R) R is in a circle, pressing (R) repeats the previous key as long as you press it under CP/M
[]
 [] is two overlapping squares
SM is an abbreviation of the German "Schreibmaschinen-Modus", typewriter mode. It has a green status LED.
Up, Down, Left, Right and Pos1 are marked with arrows
The C key is called "Korrektur" in the manual. You have to press it if the keyboard locks up, e.g. when characters have been
entered too quickly.


P30 keyboard differences:
(R) is now marked CTRL for use under DOS on the P30
[]
 [] is now marked ESC
C can be used as ALT(ernate) in DOS


*/

// translation table at offset 0xdc0 in the main rom
static INPUT_PORTS_START( alphatp3 )

PORT_START("COL.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)        PORT_CHAR('w')      PORT_CHAR('W')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)        PORT_CHAR('s')      PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)        PORT_CHAR('x')      PORT_CHAR('X')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT")        PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(8)                    // left arrow works as backspace
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)        PORT_CHAR('q')      PORT_CHAR('Q')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1)        PORT_CHAR('1')      PORT_CHAR('!')

PORT_START("COL.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)        PORT_CHAR('e')      PORT_CHAR('E')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)        PORT_CHAR('d')      PORT_CHAR('D')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)        PORT_CHAR('c')      PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE")       PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5")          PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5)) // SCAN:=0Dh ->8Ah-funct F5 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2)        PORT_CHAR('2')      PORT_CHAR('"')

PORT_START("COL.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)        PORT_CHAR('r')      PORT_CHAR('R')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)        PORT_CHAR('f')      PORT_CHAR('F')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)        PORT_CHAR('v')      PORT_CHAR('V')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT")       PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 0x82
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6")          PORT_CODE(KEYCODE_F6)       PORT_CHAR(UCHAR_MAMEKEY(F6)) // scan:=15h 8Ch-> F6 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3)        PORT_CHAR('3')      PORT_CHAR(0x00a7)

PORT_START("COL.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)        PORT_CHAR('t')      PORT_CHAR('T')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)        PORT_CHAR('g')      PORT_CHAR('G')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)        PORT_CHAR('b')      PORT_CHAR('B')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN")        PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // 0x8b
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)        PORT_CHAR('a')      PORT_CHAR('A')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4)        PORT_CHAR('4')      PORT_CHAR('$')

PORT_START("COL.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)        PORT_CHAR('z')      PORT_CHAR('Z')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)        PORT_CHAR('h')      PORT_CHAR('H')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)        PORT_CHAR('n')      PORT_CHAR('N')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS")        PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  // scan:=25h ->0xc0 ?capslock ?
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5)        PORT_CHAR('5')      PORT_CHAR('%')

PORT_START("COL.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)        PORT_CHAR('u')      PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)        PORT_CHAR('j')      PORT_CHAR('J')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)        PORT_CHAR('m')      PORT_CHAR('M')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP")          PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))        // 0x89
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)        PORT_CHAR('y')      PORT_CHAR('Y')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6)        PORT_CHAR('6')      PORT_CHAR('&')

PORT_START("COL.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)        PORT_CHAR('i')      PORT_CHAR('I')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)        PORT_CHAR('k')      PORT_CHAR('k')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", ;")         PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',')      PORT_CHAR(';')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<>")          PORT_CODE(KEYCODE_BACKSLASH2)PORT_CHAR('<')     PORT_CHAR('>')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7)        PORT_CHAR('7')      PORT_CHAR('/')

PORT_START("COL.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)        PORT_CHAR('o')      PORT_CHAR('O')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)        PORT_CHAR('l')      PORT_CHAR('L')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". :")         PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.')      PORT_CHAR(':')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pos 1")       PORT_CODE(KEYCODE_HOME)     PORT_CHAR(UCHAR_MAMEKEY(HOME))      // 0x8f
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L_SHIFT")     PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))    // 3Dh ->C1h-function P3 key left
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8)        PORT_CHAR('8')      PORT_CHAR('(')

PORT_START("COL.8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)        PORT_CHAR('p')      PORT_CHAR('P')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ö Ö")         PORT_CODE(KEYCODE_COLON)    PORT_CHAR(0x00f6)   PORT_CHAR(0x00d6)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _")         PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_CTRL")      PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  // 44h ->84h clear ?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER Pad")   PORT_CODE(KEYCODE_ENTER_PAD)PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)        PORT_CHAR('9')      PORT_CHAR(')')

PORT_START("COL.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ü Ü")         PORT_CODE(KEYCODE_OPENBRACE)PORT_CHAR(0x00fc)   PORT_CHAR(0x00dc)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ä Ä")         PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR(0x00e4)   PORT_CHAR(0x00c4)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_SHIFT")     PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 Pad")       PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)        PORT_CHAR('0')      PORT_CHAR('=')

PORT_START("COL.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("+ *")        PORT_CODE(KEYCODE_CLOSEBRACE)PORT_CHAR('+')     PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("# ^")         PORT_CODE(KEYCODE_BACKSLASH)PORT_CHAR('#')      PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[][]/ESC")    PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))           // Esc test this work ?!
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL Pad")     PORT_CODE(KEYCODE_DEL_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ß ?")         PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(0x00df)   PORT_CHAR('?')      // ß and ?

PORT_START("COL.11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')							// TAB key
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 Pad")       PORT_CODE(KEYCODE_1_PAD)    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Pad")       PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("´ `")         PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR(0x00b4)   PORT_CHAR(0x0060)

PORT_START("COL.12")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 Pad")       PORT_CODE(KEYCODE_7_PAD)    PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 Pad")       PORT_CODE(KEYCODE_4_PAD)    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 Pad")       PORT_CODE(KEYCODE_2_PAD)    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")          PORT_CODE(KEYCODE_F4)       PORT_CHAR(UCHAR_MAMEKEY(F4))             // scan 68h -> 88h func. F4

PORT_START("COL.13")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ Pad")       PORT_CODE(KEYCODE_SLASH_PAD)PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*")           PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // test ?
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\xc2\xae /Ctrl")PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_LCONTROL)         // scan 6Bh -> C2h funct.
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)                  // 0xc2 ??
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")          PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3)) // scan:=68h 88h-> F3 ok

PORT_START("COL.14")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 Pad")       PORT_CODE(KEYCODE_9_PAD)    PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 Pad")       PORT_CODE(KEYCODE_6_PAD)    PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- Pad")       PORT_CODE(KEYCODE_MINUS_PAD)PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")          PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))    // 70h -> 87h func.F2 ok

PORT_START("COL.15")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 Pad")       PORT_CODE(KEYCODE_8_PAD)    PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 Pad")       PORT_CODE(KEYCODE_5_PAD)    PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 Pad")       PORT_CODE(KEYCODE_3_PAD)    PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")          PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1))    // 7Dh -> 85H func. F1 ok
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SM")          PORT_CODE(KEYCODE_NUMLOCK)  PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))// SM (typewriter) mode key */
INPUT_PORTS_END

//**************************************************************************
//  VIDEO - Alphatronic Px
//**************************************************************************

static const gfx_layout charlayout =
{
	8, 12,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

//**************************************************************************
//  VIDEO - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

uint32_t alphatp_12_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	int start = m_crtc->upscroll_offset();
	rectangle cursor;
	m_crtc->cursor_bounds(cursor);
	for (int y = 0; y < 24; y++)
	{
		int vramy = (start + y) % 24;
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[(vramy * 128) + x];   // helwie44 must be 128d is 080h physical display-ram step line
			// draw 12 lines of the character
			bool cursoren = cursor.contains(x * 8, vramy * 12);
			for (int line = 0; line < 12; line++)
			{
				uint8_t data = m_gfx[((code & 0x7f) * 16) + line];
				if (cursoren)
					data ^= 0xff;
				bitmap.pix32(y * 12 + line, x * 8 + 0) = pen[BIT(data, 0) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 1) = pen[BIT(data, 1) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 2) = pen[BIT(data, 2) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 3) = pen[BIT(data, 3) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 4) = pen[BIT(data, 4) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 5) = pen[BIT(data, 5) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 6) = pen[BIT(data, 6) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 7) = pen[BIT(data, 7) ^ BIT(code, 7)];
			}
		}
	}

	return 0;
}

//**************************************************************************
//  VIDEO - Alphatronic P3, P4, P30 and P40
//**************************************************************************

static GFXDECODE_START( alphatp3 )
	GFXDECODE_ENTRY("gfx", 0, charlayout, 0, 1)
GFXDECODE_END

uint32_t alphatp_34_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	int start = m_crtc->upscroll_offset();
	rectangle cursor;
	m_crtc->cursor_bounds(cursor);
	for (int y = 0; y < 24; y++)
	{
		int vramy = (start + y) % 24;
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[(vramy * 128) + x];   // helwie44 must be 128d is 080h physical display-ram step line
			// draw 12 lines of the character
			bool cursoren = cursor.contains(x * 8, y * 12);
			for (int line = 0; line < 12; line++)
			{
				uint8_t data = m_gfx[((code & 0x7f) * 16) + line];
				if (cursoren)
					data ^= 0xff;
				bitmap.pix32(y * 12 + line, x * 8 + 0) = pen[BIT(data, 0) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 1) = pen[BIT(data, 1) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 2) = pen[BIT(data, 2) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 3) = pen[BIT(data, 3) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 4) = pen[BIT(data, 4) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 5) = pen[BIT(data, 5) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 6) = pen[BIT(data, 6) ^ BIT(code, 7)];
				bitmap.pix32(y * 12 + line, x * 8 + 7) = pen[BIT(data, 7) ^ BIT(code, 7)];
			}
		}
	}

	return 0;
}

//**************************************************************************
//  SOUND - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

WRITE8_MEMBER( alphatp_12_state::beep_w )
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  SOUND - Alphatronic P3, P4, P30 and P40
//**************************************************************************

WRITE8_MEMBER( alphatp_34_state::beep_w )
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  FLOPPY - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

WRITE_LINE_MEMBER(alphatp_12_state::fdcirq_w)
{
	m_fdc_irq = state;
}

WRITE_LINE_MEMBER(alphatp_12_state::fdcdrq_w)
{
	m_fdc_drq = state;
}

WRITE_LINE_MEMBER(alphatp_12_state::fdchld_w)
{
	m_fdc_hld = state;
}

READ8_MEMBER(alphatp_12_state::fdc_stat_r)
{
	uint8_t res = 0;
	floppy_image_device *floppy1,*floppy2;
	floppy1 = floppy2 = nullptr;

	floppy1 = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
	floppy2 = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;

	res = m_fdc_drq ? 0x80 : 0x00;
	res |= m_fdc_irq ? 0x40 : 0x00;
	res |= m_fdc_hld ? 0x00 : 0x20;

	if (floppy2) res |= !floppy2->ready_r() ? 0x08 : 0;
	if (floppy1) res |= !floppy1->ready_r() ? 0x04 : 0;
	if (m_curfloppy) res |= m_curfloppy->wpt_r() ? 0x02 : 0;

	return res;
}

READ8_MEMBER(alphatp_12_state::fdc_r)
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER(alphatp_12_state::fdc_w)
{
	m_fdc->gen_w(offset, data ^ 0xff);
}


WRITE8_MEMBER(alphatp_12_state::fdc_cmd_w)
{
	floppy_image_device *floppy = nullptr;

	logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

	// select drive
	if (!(data & 0x80))
	{
		floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
	}
	else if (!(data & 0x40))
	{
		floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
	}

	// selecting a new drive?
	if (floppy != m_curfloppy)
	{
		m_fdc->set_floppy(floppy);
		m_curfloppy = floppy;
	}

	if (floppy != nullptr)
	{
		// side select
		floppy->ss_w((data & 4) ? 0 : 1);

		// motor control (active low)
		floppy->mon_w((data & 0x10) ? 1 : 0);
	}
}

//**************************************************************************
//  FLOPPY - Alphatronic P3, P4, P30 and P40
//**************************************************************************

WRITE_LINE_MEMBER(alphatp_34_state::fdcirq_w)
{
	m_fdc_irq = state;
}

WRITE_LINE_MEMBER(alphatp_34_state::fdcdrq_w)
{
	m_fdc_drq = state;
}

WRITE_LINE_MEMBER(alphatp_34_state::fdchld_w)
{
	m_fdc_hld = state;
}

READ8_MEMBER(alphatp_34_state::fdc_stat_r)
{
	uint8_t res = 0;
	floppy_image_device *floppy1 = m_floppy[0]->get_device();
	floppy_image_device *floppy2 = m_floppy[1]->get_device();

	res = m_fdc_drq ? 0x80 : 0x00;
	res |= m_fdc_irq ? 0x40 : 0x00;
	res |= m_fdc_hld ? 0x00 : 0x20;
	if (floppy2) res |= floppy2->ready_r() ? 0x08 : 0;
	if (floppy1) res |= floppy1->ready_r() ? 0x04 : 0;
	if (m_curfloppy) res |= m_curfloppy->wpt_r() ? 0x02 : 0;

	return res;
}

READ8_MEMBER(alphatp_34_state::fdc_r)
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER(alphatp_34_state::fdc_w)
{
	m_fdc->gen_w(offset, data ^ 0xff);
}


WRITE8_MEMBER(alphatp_34_state::fdc_cmd_w)
{
	floppy_image_device *floppy = nullptr;

	logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

	// select drive
	if (!(data & 0x80))
		floppy = m_floppy[0]->get_device();
	else if (!(data & 0x40))
		floppy = m_floppy[1]->get_device();

	// selecting a new drive?
	if (floppy != m_curfloppy)
	{
		m_fdc->set_floppy(floppy);
		m_curfloppy = floppy;
	}

	if (floppy != nullptr)
	{
		// side select
		floppy->ss_w((data & 4) ? 0 : 1);

		// motor control (active low)
		floppy->mon_w((data & 0x10) ? 1 : 0);
	}
}

//**************************************************************************
//  FLOPPY - Drive definitions
//**************************************************************************

static SLOT_INTERFACE_START( alphatp2_floppies ) // two BASF 2471 drives
	SLOT_INTERFACE("525ssdd", FLOPPY_525_SSDD)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( alphatp2su_floppies )
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( alphatp3_floppies ) // P3:  two BASF 6106 drives
	SLOT_INTERFACE("525qd", FLOPPY_525_QD)       // P30: two Shugart SA465-3AA drives
SLOT_INTERFACE_END

//**************************************************************************
//  MACHINE - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

void alphatp_12_state::machine_start()
{
	// setup banking
	membank("ram_0000")->set_base(m_ram.target());


	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
}

void alphatp_12_state::machine_reset()
{
	m_kbdread = 1;
	m_kbdclk = 1;   m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = nullptr;
}

MACHINE_CONFIG_START(alphatp_12_state::alphatp2)
	MCFG_CPU_ADD("maincpu", I8085A, XTAL(6'000'000))
	MCFG_CPU_PROGRAM_MAP(alphatp2_mem)
	MCFG_CPU_IO_MAP(alphatp2_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("kbdmcu", I8041, XTAL(12'854'400)/2)
	MCFG_MCS48_PORT_T0_IN_CB(READLINE(alphatp_12_state, kbd_matrix_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(alphatp_12_state, kbd_matrix_w))
	MCFG_MCS48_PORT_P2_IN_CB(READ8(alphatp_12_state, kbd_port2_r))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(alphatp_12_state, kbd_port2_w))

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(alphatp2_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS(XTAL(12'854'400), 824, 0, 640, 312, 0, 288)
	MCFG_SCREEN_UPDATE_DRIVER(alphatp_12_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("crtc", CRT5027, XTAL(12'854'400))
	MCFG_TMS9927_CHAR_WIDTH(8)
	MCFG_TMS9927_HSYN_CALLBACK(INPUTLINE("maincpu", I8085_RST55_LINE))
	MCFG_TMS9927_VSYN_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE)) MCFG_DEVCB_XOR(1)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", alphatp3)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 1060 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_DEVICE_ADD("uart", I8251, 0)
	// XTAL(4'915'200) serial clock

	MCFG_FD1791_ADD("fdc", XTAL(4'000'000) / 4)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(alphatp_12_state, fdcirq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(alphatp_12_state, fdcdrq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(alphatp_12_state, fdchld_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(alphatp_12_state::alphatp2u)
	alphatp2(config);
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_DEVICE_REMOVE("fdc:1")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp2su_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp2su_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END


//**************************************************************************
//  MACHINE - Alphatronic P3, P4, P30 and P40
//**************************************************************************

void alphatp_34_state::machine_start()
{
	// setup banking
	membank("ram_0000")->set_base(m_ram.target());


	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
}

void alphatp_34_state::machine_reset()
{
	m_kbdread = 1;
	m_kbdclk = 1;   m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = nullptr;
}
MACHINE_CONFIG_START(alphatp_34_state::alphatp3)
	MCFG_CPU_ADD("maincpu", I8085A, XTAL(6'000'000))
	MCFG_CPU_PROGRAM_MAP(alphatp3_mem)
	MCFG_CPU_IO_MAP(alphatp3_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("kbdmcu", I8041, XTAL(12'854'400)/2)
	MCFG_MCS48_PORT_T0_IN_CB(READLINE(alphatp_34_state, kbd_matrix_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(alphatp_34_state, kbd_matrix_w))
	MCFG_MCS48_PORT_P2_IN_CB(READ8(alphatp_34_state, kbd_port2_r))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(alphatp_34_state, kbd_port2_w))

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(alphatp3_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS(XTAL(12'854'400), 824, 0, 640, 312, 0, 288)
	MCFG_SCREEN_UPDATE_DRIVER(alphatp_34_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("crtc", CRT5037, XTAL(12'854'400))
	MCFG_TMS9927_CHAR_WIDTH(8)

	MCFG_TMS9927_VSYN_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE)) MCFG_DEVCB_XOR(1)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", alphatp3)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 1060 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_DEVICE_ADD("uart", I8251, 0)
	// XTAL(4'915'200) serial clock

	MCFG_FD1791_ADD("fdc", XTAL(4'000'000) / 4)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(alphatp_34_state, fdcirq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(alphatp_34_state, fdcdrq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(alphatp_34_state, fdchld_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(alphatp_34_state::alphatp30)
	alphatp3(config);
	MCFG_CPU_ADD("i8088", I8088, 6000000) // unknown clock
	MCFG_CPU_PROGRAM_MAP(alphatp30_8088_map)
	MCFG_CPU_IO_MAP(alphatp30_8088_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("pic8259", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("i8088", 0))
	MCFG_PIC8259_IN_SP_CB(GND)
MACHINE_CONFIG_END

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// Alphatronic P1
ROM_START( alphatp1 )
	ROM_REGION(0x1800, "boot", 0)
	ROM_LOAD("p1mos1.bin", 0x0000, 0x0800, CRC(9317a694) SHA1(3b51a6b72d2ccae2459ddb2e16fbd21b19dfa2b8) )
	ROM_LOAD("p1mos2.bin", 0x0800, 0x0800, CRC(f38113a3) SHA1(078405ad202e26b7bac7132b06682fb01270af63) )
	ROM_LOAD("p1mos3.bin", 0x1000, 0x0800, CRC(fb5ae050) SHA1(ba55553764326dfda3fbd35237761c3fb6fde18a) )

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))   // P1 keyboard driver is contained in a MF-1702AR on the keyboard
																															// needs to be dumped
	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("p1chargen.bin", 0x000, 0x800, CRC(51ea8a7e) SHA1(c514df7ab3761490af4a16c9106d08540f0d7352))
ROM_END

// Alphatronic P2
ROM_START( alphatp2 ) // P2 ROM space 0x1800
	ROM_REGION(0x1800, "boot", 0)
	ROM_SYSTEM_BIOS(0, "caap94-96", "caap94-96")
	ROM_SYSTEM_BIOS(1, "caap04-06", "caap04-06")
	ROM_SYSTEM_BIOS(2, "p2_es", "p2_es")

	ROMX_LOAD("caap_96_00_5a.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(1) ) // earlier P2, three 16K RAM boards
	ROMX_LOAD("caap_05_02_12.bin", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9), ROM_BIOS(1) )
	ROMX_LOAD("caap_94_01_50.bin", 0x1000, 0x0800, CRC(fda8d4a4) SHA1(fa91e6e8504e7f84cf69d86f72826ad5405fd82d), ROM_BIOS(1) )

	ROMX_LOAD("caap_06_00_17.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(2) ) // later P2, one 48K RAM board
	ROMX_LOAD("caap_05_01_12.bin", 0x0800, 0x0800, CRC(98c5ec7a) SHA1(b170e9a73b0b64d4111fa1170af6e333793da479), ROM_BIOS(2) )
	ROMX_LOAD("caap_04_01_03.bin", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4), ROM_BIOS(2) )

	ROMX_LOAD("caap_p2_es_1.bin", 0x0000, 0x0800, CRC(91b58eb3) SHA1(a4467cf9a14366198ee5f676b9471734e820522d), ROM_BIOS(3) )   // P2 Spain
	ROMX_LOAD("caap_p2_es_2.bin", 0x0800, 0x0800, CRC(f4dfac82) SHA1(266d1fdaef875515d9c68ae3e67ec88831bb55cb), ROM_BIOS(3) )
	ROMX_LOAD("caap_p2_es_3.bin", 0x1000, 0x0800, CRC(6f6918ba) SHA1(8dc9f5e337df8abf42e5b55f5f1a1a9d61c42d78), ROM_BIOS(3) )

	ROM_REGION(0x400, "kbdmcu", 0)                                                                                          // same across all dumped P2 and P3 machines so far
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))   // needs to be checked with P2 sks and Spain

	ROM_REGION(0x800, "gfx", 0)
	ROMX_LOAD("cajp_97_00_5a.bin", 0x000, 0x800, CRC(aa5eab85) SHA1(2718924f5520e7e9aad635786847e78e3096b285), ROM_BIOS(1)) // came with caap94-96
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(2)) // came with caap04-06
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(3)) // sks KISS chargen not dumped yet
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(4)) // spanish P2 chargen not dumped yet
ROM_END

// Alphatronic P2U
ROM_START( alphatp2u )
	ROM_REGION(0x1800, "boot", 0)
	ROM_LOAD("mos3-p2_sks_1", 0x0000, 0x0800, CRC(c98d2982) SHA1(11e98ae441b7a9c8dfd22795f8208667959f1d1c) )
	ROM_LOAD("mos3-p2_sks_2", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9) )
	ROM_LOAD("mos3-p2_sks_3", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4) )

	ROM_REGION(0x400, "kbdmcu", 0)                                                                                          // same across all dumped P2 and P3 machines so far
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))   // needs to be checked for P2U

	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d))   // P2U chargen needs to be dumped
ROM_END

// Alphatronic P3
ROM_START( alphatp3 )
	ROM_REGION(0x1800, "boot", 0) // P3 ROM space 0x1000
	ROM_SYSTEM_BIOS(0, "gx347", "gx347") // earlier P3, seperate 48K and 16K RAM boards
	ROM_SYSTEM_BIOS(1, "lb352", "lb352") // later P3, one 64K RAM board

	ROM_LOAD("caap36_02_19.bin", 0x0000, 0x1000, CRC(23df6666) SHA1(5ea04cd299dec9951425eb91ecceb4818c4c6378) ) // identical between earlier and later P3
																												// BIOS names taken from CPU card labels
	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("p3_keyboard_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51) )

	ROM_REGION(0x800, "gfx", 0)
	ROMX_LOAD("cajp08_00_15.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(1) )
	ROMX_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8), ROM_BIOS(2) )
ROM_END

// Alphatronic P30
ROM_START( alphatp30 ) // P30 add-on card with 8088 needs to be emulated to boot DOS
	ROM_REGION(0x1800, "boot", 0)
	ROM_LOAD("hasl17.07.84.bin", 0x0000, 0x1000, CRC(6A91701B) SHA1(8A4F925D0FABAB37852A54D04E06DEB2AEAA349C))  // ...wait for INT6.5 or INT5.5 with RIM to write char in hsync or hsync GAP-time !!

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("caju_01_01_01.bin",  0x000, 0x400, CRC(e9b4359f) SHA1(835f4a580b4c108ef2f239039b765324adc7f078))

	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8))

	ROM_REGION(0x4000, "16bit", 0)
	ROM_LOAD("caxp_02_02_13.bin", 0x00000, 0x2000, CRC(e6bf6dd5) SHA1(dc87210bbcd96f3c1370565174a45199e3c1bc70)) // P30 ROM from 8088 card
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT   COMPAT   MACHINE   INPUT       CLASS           INIT  COMPANY          FULLNAME  FLAGS
COMP( 198?, alphatp1,  alphatp2, 0,     alphatp2, alphatp2, alphatp_12_state, 0,    "Triumph-Adler", "alphatronic P1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, alphatp2,  0,        0,     alphatp2, alphatp2, alphatp_12_state, 0,    "Triumph-Adler", "alphatronic P2", MACHINE_NOT_WORKING )
COMP( 198?, alphatp2u, alphatp2, 0,     alphatp2u,alphatp3, alphatp_12_state, 0,    "Triumph-Adler", "alphatronic P2U", MACHINE_NOT_WORKING )
COMP( 1982, alphatp3,  0,        0,     alphatp3, alphatp3, alphatp_34_state, 0,    "Triumph-Adler", "alphatronic P3", MACHINE_NOT_WORKING )
COMP( 198?, alphatp30, alphatp3, 0,     alphatp30, alphatp3, alphatp_34_state, 0,    "Triumph-Adler", "alphatronic P30",MACHINE_NOT_WORKING )
