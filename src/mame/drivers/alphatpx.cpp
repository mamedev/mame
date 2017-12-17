// license: GPL-2.0+
// copyright-holders: Dirk Best
// test / modify oct-2017 begin: rfka01 + helwie44
/***************************************************************************

    Triumph-Adler Alphatronic Px series
	===================================
	
	The Px series was designed by SKS, like the ITT3030 and the SKS Nano,
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
	Those were P3 and P4's with an additional 8088 card (some with an extra graphics extension) to support MS-DOS.

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "video/tms9927.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class alphatpx_state : public driver_device
{
public:
	alphatpx_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_bankdev(*this, "bankdev"),
	m_kbdmcu(*this, "kbdmcu"),
	m_crtc(*this, "crtc"),
	m_fdc (*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_beep(*this, "beeper"),
	m_keycols(*this, "COL.%u", 0),
	m_palette(*this, "palette"),
	m_vram(*this, "vram"),
	m_gfx(*this, "gfx"),
	m_ram(*this, "ram")
	
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);	
public:
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

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<address_map_bank_device> m_bankdev;
	required_device<i8041_device> m_kbdmcu;
	required_device<crt5037_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
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
	floppy_connector *m_con1, *m_con2;
	
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( alphatp3_mem, AS_PROGRAM, 8, alphatpx_state )
	AM_RANGE(0x0000, 0xffff) AM_DEVICE("bankdev", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( alphatp3_map, AS_PROGRAM, 8, alphatpx_state )
	AM_RANGE(0x00000, 0x017ff) AM_ROM AM_REGION("boot", 0)  // P2  0x0000 , 0x17ff -hw 6kB, P3 only 4 kB
	AM_RANGE(0x01800, 0x01c00) AM_RAM // boot rom variables
	AM_RANGE(0x03000, 0x03bff) AM_WRITEONLY AM_SHARE("vram") // test  2017 hw, MOS directly writes to display RAM
	AM_RANGE(0x03FF0, 0x03fff) AM_DEVWRITE("crtc", crt5037_device, write) //test hw, mem-mapped registers, cursor position can be determined through this range
	AM_RANGE(0x00000, 0x0ffff) AM_RAMBANK("ram_0000")

	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( alphatp3_io, AS_IO, 8, alphatpx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("kbdmcu", i8041_device, upi41_master_r, upi41_master_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(beep_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x54, 0x54) AM_READWRITE(fdc_stat_r, fdc_cmd_w)
	AM_RANGE(0x78, 0x78) AM_WRITE(bank_w)
ADDRESS_MAP_END

WRITE8_MEMBER(alphatpx_state::bank_w)
{
	m_bankdev->set_bank(BIT(data, 6));
}

//**************************************************************************
//  INPUTS
//**************************************************************************

READ_LINE_MEMBER(alphatpx_state::kbd_matrix_r)
{
	return m_kbdread;
}

WRITE8_MEMBER(alphatpx_state::kbd_matrix_w)
{
	int rd_masks[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };
	int tmp_read;

	if ((data & 0x80) && (!m_kbdclk))
	{
		tmp_read = m_keycols[(data >> 3) & 0xf]->read() & rd_masks[data & 0x7];
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;

}

// bit 2 is UPI-41 host IRQ to Z80
WRITE8_MEMBER(alphatpx_state::kbd_port2_w)
{
	m_kbdport2 = data;

}

READ8_MEMBER(alphatpx_state::kbd_port2_r)
{
	return m_kbdport2;
}

// translation table at offset 0xdc0 in the main rom
static INPUT_PORTS_START( alphatp3 )
	PORT_START("COL.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	
	PORT_START("COL.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x8a
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	
	PORT_START("COL.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x82
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x8c
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('@')

	PORT_START("COL.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x8b
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("COL.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') 
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xc0
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	
	PORT_START("COL.5")  
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x89
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("COL.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('k')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_START("COL.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x8f
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("COL.8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x84
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("COL.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')

	PORT_START("COL.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('#') PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x7f
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('~') PORT_CHAR('?')

	PORT_START("COL.11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('`')

	PORT_START("COL.12")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x93

	PORT_START("COL.13")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xc2
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x92

	PORT_START("COL.14")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x91

	PORT_START("COL.15")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x90
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xff

INPUT_PORTS_END


//**************************************************************************
//  VIDEO
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

static GFXDECODE_START( alphatp3 )
	GFXDECODE_ENTRY("gfx", 0, charlayout, 0, 1)
GFXDECODE_END

uint32_t alphatpx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	int start = m_crtc->upscroll_offset();
	for (int y = 0; y < 24; y++)
	{
		int vramy = (start + y) % 24;
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[(vramy * 128) + x];   // helwie44 must be 128d is 080h physical display-ram step line
			// draw 12 lines of the character
			for (int line = 0; line < 12; line++)
			{
				uint8_t data = m_gfx[((code & 0x7f) * 16) + line];
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
//  SOUND
//**************************************************************************

WRITE8_MEMBER( alphatpx_state::beep_w )
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  SOUND
//**************************************************************************

WRITE_LINE_MEMBER(alphatpx_state::fdcirq_w)
{
	m_fdc_irq = state;
}

#include "debugger.h"

WRITE_LINE_MEMBER(alphatpx_state::fdcdrq_w)
{
	m_fdc_drq = state;
}

WRITE_LINE_MEMBER(alphatpx_state::fdchld_w)
{
	m_fdc_hld = state;
}

/*
    7 Data Request (DRQ - inverted 1791-Signal)
    6 Interrupt Request (INTRQ - 1791-Signal)
    5 Head Load (HLD - inverted 1791-Signal)
    4 Ready 3 (Drive 3 ready)
    3 Ready 2 (Drive 2 ready)
    2 Ready l (Drive 1 ready)
    1 Write protect (the disk in the selected drive is write protected)
    0 HLT (Halt signal during head load and track change)
*/
READ8_MEMBER(alphatpx_state::fdc_stat_r)
{
	uint8_t res = 0;
	floppy_image_device *floppy1 = m_con1 ? m_con1->get_device() : nullptr;
	floppy_image_device *floppy2 = m_con2 ? m_con2->get_device() : nullptr;

	res = m_fdc_drq ? 0x80 : 0x00;
	res |= m_fdc_irq ? 0x40 : 0x00;
	res |= m_fdc_hld ? 0x00 : 0x20;
	if (floppy2) res |= floppy2->ready_r() ? 0x08 : 0;
	if (floppy1) res |= floppy1->ready_r() ? 0x04 : 0;
	if (m_curfloppy) res |= m_curfloppy->wpt_r() ? 0x02 : 0;

	return res;
}

/* As far as we can tell, the mess of ttl de-inverts the bus */
READ8_MEMBER(alphatpx_state::fdc_r)
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER(alphatpx_state::fdc_w)
{
	m_fdc->gen_w(offset, data ^ 0xff);
}

/*
    7 SEL1 - Select drive 1
    6 SEL2 - Select drive 2
    5 SEL3 - Select drive 3
    4 MOTOR - Motor on
    3 DOOR - Drive door lock drives 1 + 2 (not possible with all drives)
    2 SIDESEL - Select disk side
    1 KOMP - write comp on/off
    0 RG J - Change separator stage to read
*/
WRITE8_MEMBER(alphatpx_state::fdc_cmd_w)
{
	floppy_image_device *floppy = nullptr;

	logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

	// select drive
	if (!(data & 0x80))
	{
		floppy = m_con1 ? m_con1->get_device() : nullptr;
	}
	else if (!(data & 0x40))
	{
		floppy = m_con2 ? m_con2->get_device() : nullptr;
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

static SLOT_INTERFACE_START( alphatp2_floppies ) // two BASF 6106 drives
	SLOT_INTERFACE("525ssdd", FLOPPY_525_SSDD)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( alphatp2su_floppies ) 
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( alphatp3_floppies ) // two BASF 2471 drives
	SLOT_INTERFACE("525qd", FLOPPY_525_QD)
SLOT_INTERFACE_END

//**************************************************************************
//  MACHINE
//**************************************************************************

void alphatpx_state::machine_start()
{
	// setup banking
	membank("ram_0000")->set_base(m_ram.target());


	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
}

void alphatpx_state::machine_reset()
{
	m_kbdread = 1;
	m_kbdclk = 1;	m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = nullptr;

	// look up floppies in advance
	m_con1 = machine().device<floppy_connector>("fdc:0");
	m_con2 = machine().device<floppy_connector>("fdc:1");
}

//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static MACHINE_CONFIG_START( alphatp3 )
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(alphatp3_mem)
	MCFG_CPU_IO_MAP(alphatp3_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("kbdmcu", I8041, XTAL_12_8544MHz/2)
	MCFG_MCS48_PORT_T0_IN_CB(READLINE(alphatpx_state, kbd_matrix_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(alphatpx_state, kbd_matrix_w))
	MCFG_MCS48_PORT_P2_IN_CB(READ8(alphatpx_state, kbd_port2_r))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(alphatpx_state, kbd_port2_w))

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(alphatp3_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS(XTAL_12_8544MHz, 824, 0, 640, 312, 0, 288)
	MCFG_SCREEN_UPDATE_DRIVER(alphatpx_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("crtc", CRT5037, XTAL_12_8544MHz)
	MCFG_TMS9927_CHAR_WIDTH(8)

	MCFG_TMS9927_VSYN_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE)) MCFG_DEVCB_XOR(1)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", alphatp3)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 1060 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_DEVICE_ADD("uart", I8251, 0)
	// XTAL_4_9152MHz serial clock

	MCFG_FD1791_ADD("fdc", XTAL_4MHz / 4)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(alphatpx_state, fdcirq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(alphatpx_state, fdcdrq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(alphatpx_state, fdchld_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp3_floppies, "525qd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED (alphatp2, alphatp3)
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_DEVICE_REMOVE("fdc:1")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED (alphatp2u, alphatp3)
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_DEVICE_REMOVE("fdc:1")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", alphatp2su_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", alphatp2su_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// Alphatronic P2
ROM_START( alphatp2 ) // P2 ROM space 0x1800
	ROM_SYSTEM_BIOS(0, "caap94-96", "caap94-96")
	ROM_SYSTEM_BIOS(1, "caap04-06", "caap04-06")	
	ROM_SYSTEM_BIOS(2, "p2_es", "p2_es")
	ROM_SYSTEM_BIOS(3, "p2_sks", "p2_sks")
	
	ROM_REGION(0x1800, "boot", 0)
	ROMX_LOAD("caap_96_00_5a.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(1) ) // earlier P2, three 16K RAM boards
	ROMX_LOAD("caap_05_02_12.bin", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9), ROM_BIOS(1) )
	ROMX_LOAD("caap_94_01_50.bin", 0x1000, 0x0800, CRC(fda8d4a4) SHA1(fa91e6e8504e7f84cf69d86f72826ad5405fd82d), ROM_BIOS(1) )

	ROMX_LOAD("caap_06_00_17.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(2) ) // later P2, one 48K RAM board
	ROMX_LOAD("caap_05_01_12.bin", 0x0800, 0x0800, CRC(98c5ec7a) SHA1(b170e9a73b0b64d4111fa1170af6e333793da479), ROM_BIOS(2) )
	ROMX_LOAD("caap_04_01_03.bin", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4), ROM_BIOS(2) )

	ROMX_LOAD("caap_p2_es_1.bin", 0x0000, 0x0800, CRC(91b58eb3) SHA1(a4467cf9a14366198ee5f676b9471734e820522d), ROM_BIOS(3) )	// P2 Spain
	ROMX_LOAD("caap_p2_es_2.bin", 0x0800, 0x0800, CRC(f4dfac82) SHA1(266d1fdaef875515d9c68ae3e67ec88831bb55cb), ROM_BIOS(3) )
	ROMX_LOAD("caap_p2_es_3.bin", 0x1000, 0x0800, CRC(6f6918ba) SHA1(8dc9f5e337df8abf42e5b55f5f1a1a9d61c42d78), ROM_BIOS(3) )
	
	ROMX_LOAD("mos3-p2_sks_1.bin", 0x0000, 0x0800, CRC(c98d2982) SHA1(11e98ae441b7a9c8dfd22795f8208667959f1d1c), ROM_BIOS(4) ) // P2 sks
	ROMX_LOAD("mos3-p2_sks_2.bin", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9), ROM_BIOS(4) )
	ROMX_LOAD("mos3-p2_sks_3.bin", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4), ROM_BIOS(4) )

	ROM_REGION(0x400, "kbdmcu", 0)																							// same across all dumped P2 and P3 machines so far
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))	// needs to be checked with P2 sks and Spain

	ROM_REGION(0x800, "gfx", 0)
	ROMX_LOAD("cajp_97_00_5a.bin", 0x000, 0x800, CRC(aa5eab85) SHA1(2718924f5520e7e9aad635786847e78e3096b285), ROM_BIOS(1)) // came with caap94-96
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(2)) // came with caap04-06
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(3)) // sks KISS chargen not dumped yet
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(4)) // spanish P2 chargen not dumped yet
ROM_END

// Alphatronic P2U
ROM_START (alphatp2u)
	ROM_REGION(0x1800, "boot", 0)
	ROM_LOAD("prom2p00.bin", 0x0000, 0x0800, CRC(c98d2982) SHA1(11e98ae441b7a9c8dfd22795f8208667959f1d1c) )
	ROM_LOAD("prom2p01.bin", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9) )
	ROM_LOAD("prom2p02.bin", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4) )

	ROM_REGION(0x400, "kbdmcu", 0)																							// same across all dumped P2 and P3 machines so far
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))	// needs to be checked for P2U
	
	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d))	// P2U chargen needs to be dumped
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
	ROM_LOAD("hasl17.07.84.bin", 0x0000, 0x1000, CRC(6A91701B) SHA1(8A4F925D0FABAB37852A54D04E06DEB2AEAA349C))	// netmercer Eprom P30 ...wait for INT6.5 or INT5.5 with RIM to write char in hsync or hsync GAP-time !!

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("p3_keyboard_8278.bin",  0x000, 0x400, CRC(5DB00D85) SHA1(0DC8E274A5AECE261EF60494901601C0D8B1EB51))  // same across all dumped P2 and P3 machines so far

	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8)) // borrowed from P3 lb352

// ROM_LOAD("p30_8088.bin", 0x000, 0x4000, CRC(e6bf6dd5) SHA1(dc87210bbcd96f3c1370565174a45199e3c1bc70)) // P30 ROM from 8088 card
ROM_END	


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT   COMPAT   MACHINE   INPUT       CLASS           INIT  COMPANY          FULLNAME  FLAGS
COMP( 198?, alphatp2,  alphatp3, 0,     alphatp2, alphatp3, alphatpx_state, 0,    "Triumph-Adler", "alphatronic P2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, alphatp2u, alphatp3, 0,     alphatp2u,alphatp3, alphatpx_state, 0,    "Triumph-Adler", "alphatronic P2U", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1982, alphatp3,  0,        0,     alphatp3, alphatp3, alphatpx_state, 0,    "Triumph-Adler", "alphatronic P3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, alphatp30, alphatp3, 0,		alphatp3, alphatp3, alphatpx_state, 0,	"Triumph-Adler", "alphatronic P30",MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
