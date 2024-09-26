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
    For paging via port 0x78A, a 16K RAM card with RAM at 0x0000 and 0x3fff and the banking logic (see above) is added to the standard 48K memory card.
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
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "video/tms9927.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "debugger.h"


namespace {

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
	void alphatp1(machine_config &config);
	void alphatp2(machine_config &config);
	void alphatp2u(machine_config &config);
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int kbd_matrix_r();
	void kbd_matrix_w(u8 data);
	u8 kbd_port2_r();
	void kbd_port2_w(u8 data);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);
	u8 fdc_stat_r();
	void fdc_cmd_w(u8 data);

	void fdcirq_w(int state);
	void fdcdrq_w(int state);
	void fdchld_w(int state);
	void beep_w(u8 data);
	void bank_w(u8 data);

	void alphatp2_io(address_map &map) ATTR_COLD;
	void alphatp2_map(address_map &map) ATTR_COLD;
	void alphatp2_mem(address_map &map) ATTR_COLD;

	required_device<address_map_bank_device> m_bankdev;
	required_device<i8041a_device> m_kbdmcu;
	required_device<crt5027_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<beep_device> m_beep;
	required_ioport_array<16> m_keycols;

	u8 m_kbdclk = 0, m_kbdread = 0, m_kbdport2 = 0;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_gfx;
	required_shared_ptr<u8> m_ram;
	floppy_image_device *m_curfloppy = nullptr;
	bool m_fdc_irq = false, m_fdc_drq = false, m_fdc_hld = false;
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
		m_pic(*this, "pic8259"),
		m_keycols(*this, "COL.%u", 0),
		m_scncfg(*this, "SCREEN"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram(*this, "vram"),
		m_gfx(*this, "gfx"),
		m_ram(*this, "ram")
	{ }

	void alphatp3(machine_config &config);
	void alphatp30(machine_config &config);
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
private:

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int kbd_matrix_r();
	void kbd_matrix_w(u8 data);
	u8 kbd_port2_r();
	void kbd_port2_w(u8 data);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);
	u8 fdc_stat_r();
	void fdc_cmd_w(u8 data);

	void fdcirq_w(int state);
	void fdcdrq_w(int state);
	void fdchld_w(int state);
	void beep_w(u8 data);
	void bank_w(u8 data);
	u8 start88_r(offs_t offset);
	u8 comm85_r(offs_t offset);
	void comm85_w(u8 data);
	u8 comm88_r(offs_t offset);
	void comm88_w(u8 data);
	u8 gfxext_r(offs_t offset);
	void gfxext_w(offs_t offset, u8 data);
	void gfxext1_w(u8 data);
	void gfxext2_w(u8 data);
	void gfxext3_w(offs_t offset, u8 data);

	u8* vramext_addr_xlate(offs_t offset);

	void alphatp30_8088_io(address_map &map) ATTR_COLD;
	void alphatp30_8088_map(address_map &map) ATTR_COLD;
	void alphatp3_io(address_map &map) ATTR_COLD;
	void alphatp3_map(address_map &map) ATTR_COLD;
	void alphatp3_mem(address_map &map) ATTR_COLD;

	required_device<address_map_bank_device> m_bankdev;
	required_device<i8041a_device> m_kbdmcu;
	required_device<crt5037_device> m_crtc;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	optional_device<cpu_device> m_i8088;
	required_device<beep_device> m_beep;
	optional_device<pic8259_device> m_pic;
	required_ioport_array<16> m_keycols;
	required_ioport m_scncfg;

	u8 m_kbdclk, m_kbdread, m_kbdport2;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_gfx;
	required_shared_ptr<u8> m_ram;
	floppy_image_device *m_curfloppy;
	bool m_fdc_irq, m_fdc_drq, m_fdc_hld;
	u8 m_85_data, m_88_data;
	bool m_88_da, m_85_da, m_88_started;
	u8 m_gfxext1, m_gfxext2, m_vramlatch;
	u16 m_gfxext3;
	u8 m_vramext[371*80];
	u8 m_vramchr[256*12];  // these are one 32K ram region but with complex mapping
};

//**************************************************************************
//  ADDRESS MAPS - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

void alphatp_12_state::alphatp2_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void alphatp_12_state::alphatp2_map(address_map &map)
{
	map(0x00000, 0x0ffff).bankrw("ram_0000");
	map(0x00000, 0x017ff).rom().region("boot", 0);  // P2  0x0000 , 0x17ff -hw 6kB, P3 only 4 kB
	map(0x01800, 0x01c00).ram(); // boot rom variables
	map(0x03000, 0x03bff).writeonly().share("vram"); // test  2017 hw, MOS directly writes to display RAM
	map(0x03FF0, 0x03fff).w(m_crtc, FUNC(crt5027_device::write)); //test hw, mem-mapped registers, cursor position can be determined through this range

	map(0x10000, 0x1ffff).ram().share("ram");
}

void alphatp_12_state::alphatp2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x04, 0x05).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x11).rw(m_kbdmcu, FUNC(i8041a_device::upi41_master_r), FUNC(i8041a_device::upi41_master_w));
	map(0x12, 0x12).w(FUNC(alphatp_12_state::beep_w));
	map(0x50, 0x53).rw(FUNC(alphatp_12_state::fdc_r), FUNC(alphatp_12_state::fdc_w));
	map(0x54, 0x54).rw(FUNC(alphatp_12_state::fdc_stat_r), FUNC(alphatp_12_state::fdc_cmd_w));
	map(0x78, 0x78).w(FUNC(alphatp_12_state::bank_w));
}


void alphatp_12_state::bank_w(u8 data)
{
	m_bankdev->set_bank(BIT(data, 6));
}


//**************************************************************************
//  ADDRESS MAPS - Alphatronic P3, P4, P30 and P40
//**************************************************************************

void alphatp_34_state::alphatp3_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void alphatp_34_state::alphatp3_map(address_map &map)
{
	map(0x00000, 0x0ffff).bankrw("ram_0000");
	map(0x00000, 0x017ff).rom().region("boot", 0);  // P2  0x0000 , 0x17ff -hw 6kB, P3 only 4 kB
	map(0x01800, 0x01c00).ram(); // boot rom variables
	map(0x03000, 0x03bff).writeonly().share("vram"); // test  2017 hw, MOS directly writes to display RAM
	map(0x03FF0, 0x03fff).w(m_crtc, FUNC(crt5037_device::write)); //test hw, mem-mapped registers, cursor position can be determined through this range

	map(0x10000, 0x1ffff).ram().share("ram");
}

void alphatp_34_state::alphatp3_io(address_map &map)
{
	map.unmap_value_high();
	//map(0x00, 0x00).r(FUNC(alphatp_34_state::)); // unknown
	map(0x04, 0x05).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x08, 0x09).rw(FUNC(alphatp_34_state::comm88_r), FUNC(alphatp_34_state::comm88_w));
	map(0x10, 0x11).rw(m_kbdmcu, FUNC(i8041a_device::upi41_master_r), FUNC(i8041a_device::upi41_master_w));
	map(0x12, 0x12).w(FUNC(alphatp_34_state::beep_w));
	map(0x40, 0x41).r(FUNC(alphatp_34_state::start88_r));
	//map(0x42, 0x42).w(FUNC(alphatp_34_state::)); // unknown
	map(0x50, 0x53).rw(FUNC(alphatp_34_state::fdc_r), FUNC(alphatp_34_state::fdc_w));
	map(0x54, 0x54).rw(FUNC(alphatp_34_state::fdc_stat_r), FUNC(alphatp_34_state::fdc_cmd_w));
	map(0x78, 0x78).w(FUNC(alphatp_34_state::bank_w));
}

void alphatp_34_state::alphatp30_8088_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xe0000, 0xeffff).rw(FUNC(alphatp_34_state::gfxext_r), FUNC(alphatp_34_state::gfxext_w));
	map(0xfe000, 0xfffff).rom().region("16bit", 0);
}

void alphatp_34_state::alphatp30_8088_io(address_map &map)
{
	//map(0x008a, 0x008a).r(FUNC(alphatp_34_state::)); // unknown
	map(0xf800, 0xf800).w(FUNC(alphatp_34_state::gfxext1_w));
	map(0xf900, 0xf900).w(FUNC(alphatp_34_state::gfxext2_w));
	map(0xfa00, 0xfa01).w(FUNC(alphatp_34_state::gfxext3_w));
	//map(0xfb00, 0xfb0f).w(FUNC(alphatp_34_state::)); // unknown possibly gfx ext
	map(0xffe0, 0xffe1).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xffe4, 0xffe7).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xffe9, 0xffea).rw(FUNC(alphatp_34_state::comm85_r), FUNC(alphatp_34_state::comm85_w));
}

u8 alphatp_34_state::start88_r(offs_t offset)
{
	if(!offset)
	{
		if(m_i8088 && !m_88_started)
		{
			m_i8088->resume(SUSPEND_REASON_DISABLE);
			m_88_started = true;
		}
		m_i8088->set_input_line(INPUT_LINE_TEST, ASSERT_LINE);
	}
	else
		m_i8088->set_input_line(INPUT_LINE_TEST, CLEAR_LINE);
	return 0;
}

void alphatp_34_state::bank_w(u8 data)
{
	m_bankdev->set_bank(BIT(data, 6));
}

u8 alphatp_34_state::comm88_r(offs_t offset)
{
	if(!offset)
		return (m_85_da ? 0 : 1) | (m_88_da ? 0 : 0x80);
	if(m_i8088)
		m_i8088->set_input_line(INPUT_LINE_TEST, ASSERT_LINE);
	m_85_da = false;
	return m_85_data;
}

void alphatp_34_state::comm88_w(u8 data)
{
	m_88_data = data;
	if(m_pic)
		m_pic->ir2_w(ASSERT_LINE);
	m_88_da = true;
}

u8 alphatp_34_state::comm85_r(offs_t offset)
{
	if(!offset)
		return m_88_da ? 0 : 1;
	m_pic->ir2_w(CLEAR_LINE);
	m_88_da = false;
	return m_88_data;
}

void alphatp_34_state::comm85_w(u8 data)
{
	m_85_data = data;
	m_85_da = true;
	m_i8088->set_input_line(INPUT_LINE_TEST, CLEAR_LINE);
}

void alphatp_34_state::gfxext1_w(u8 data)
{
	m_gfxext1 = data;
}

void alphatp_34_state::gfxext2_w(u8 data)
{
	m_gfxext2 = data;
}

void alphatp_34_state::gfxext3_w(offs_t offset, u8 data)
{
	u16 mask = 0xff << (offset ? 0 : 8);
	m_gfxext3 = (m_gfxext3 & mask) | (data << (offset * 8));
}

u8* alphatp_34_state::vramext_addr_xlate(offs_t offset)
{
	offset = offset >> 3;
	int bank = offset >> 7;
	int offs = offset & 0x7f;
	if(offs < 80)
		return &m_vramext[(((((m_gfxext2 & 0xf8) << 2) + bank) * 80) + offs) % (371*80)];
	else
		return &m_vramchr[(((((m_gfxext2 & 0x8) << 2) ^ bank) * 48) + (offs - 80)) % (256*12)];
}

u8 alphatp_34_state::gfxext_r(offs_t offset)
{
	switch(m_gfxext1)
	{
		case 0x33:
			m_vramlatch = *vramext_addr_xlate(offset);
			return 0;
	}
	logerror("gfxext read offset %x %x\n", offset, m_gfxext1);
	return 0;
}

void alphatp_34_state::gfxext_w(offs_t offset, u8 data)
{
	switch(m_gfxext1)
	{
		case 5:
		{
			if(m_gfxext3 == 0xe0f)
				data = ~data;
			u8 mask = 1 << (offset & 7);
			u8 *addr = vramext_addr_xlate(offset);
			*addr &= ~mask;
			*addr |= data & mask;
			break;
		}
		case 6:
			*vramext_addr_xlate(offset) ^= 1 << (offset & 7);
			break;
		case 0x33:
			*vramext_addr_xlate(offset) = m_vramlatch;
			break;
		default:
			logerror("gfxext write offset %x %x %x\n", offset, data, m_gfxext1);
	}
	if((offset & 0x3ff) > 0x280)
		m_gfxdecode->gfx(1)->mark_dirty(((uintptr_t)vramext_addr_xlate(offset) - (uintptr_t)m_vramchr) / 12);
}

//**************************************************************************
//  INPUTS -  Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

int alphatp_12_state::kbd_matrix_r()
{
	return m_kbdread;
}

void alphatp_12_state::kbd_matrix_w(u8 data)
{
	if ((data & 0x80) && (!m_kbdclk))
	{
		const ioport_value tmp_read = m_keycols[(data >> 3) & 0xf]->read() & (1 << (data & 0x7));
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;

}

// bit 2 is UPI-41 host IRQ to Z80
void alphatp_12_state::kbd_port2_w(u8 data)
{
	m_kbdport2 = data;

}

u8 alphatp_12_state::kbd_port2_r()
{
	return m_kbdport2;
}

//**************************************************************************
//  INPUTS - Alphatronic P3, P4, P30 and P40
//**************************************************************************

int alphatp_34_state::kbd_matrix_r()
{
	return m_kbdread;
}

void alphatp_34_state::kbd_matrix_w(u8 data)
{
	if (data & 0x80)
	{
		data--; // FIXME: the p30 kbc program doesn't clock the keyboard but gets the wrong value from t0
		const ioport_value tmp_read = m_keycols[(data >> 3) & 0xf]->read() & (1 << (data & 0x7));
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;

}

// bit 2 is UPI-41 host IRQ to Z80
void alphatp_34_state::kbd_port2_w(u8 data)
{
	m_kbdport2 = data;

}

u8 alphatp_34_state::kbd_port2_r()
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
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Y)        PORT_CHAR('z')      PORT_CHAR('Z')
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
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Z)        PORT_CHAR('y')      PORT_CHAR('Y')
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
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ö Ö")       PORT_CODE(KEYCODE_COLON)    PORT_CHAR(0x00f6)   PORT_CHAR(0x00d6)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _")         PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_CTRL")      PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  // 44h ->84h clear ?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER Pad")   PORT_CODE(KEYCODE_ENTER_PAD)PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)        PORT_CHAR('9')      PORT_CHAR(')')

PORT_START("COL.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ü Ü")       PORT_CODE(KEYCODE_OPENBRACE)PORT_CHAR(0x00fc)   PORT_CHAR(0x00dc)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ä Ä")       PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR(0x00e4)   PORT_CHAR(0x00c4)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_SHIFT")     PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 Pad")       PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)        PORT_CHAR('0')      PORT_CHAR('=')

PORT_START("COL.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ *")         PORT_CODE(KEYCODE_CLOSEBRACE)PORT_CHAR('+')     PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("# ^")         PORT_CODE(KEYCODE_BACKSLASH)PORT_CHAR('#')      PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[][]/ESC")    PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))           // Esc test this work ?!
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL Pad")     PORT_CODE(KEYCODE_DEL_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ß ?")       PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(0x00df)   PORT_CHAR('?')      // ß and ?

PORT_START("COL.11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')                         // TAB key
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 Pad")       PORT_CODE(KEYCODE_1_PAD)    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Pad")       PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"´ `")       PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR(0x00b4,'\'') PORT_CHAR(0x0060)

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
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"® /Ctrl")   PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_LCONTROL)         // scan 6Bh -> C2h funct.
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
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Y)        PORT_CHAR('z')      PORT_CHAR('Z')
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
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Z)        PORT_CHAR('y')      PORT_CHAR('Y')
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
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ö Ö")       PORT_CODE(KEYCODE_COLON)    PORT_CHAR(0x00f6)   PORT_CHAR(0x00d6)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _")         PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_CTRL")      PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  // 44h ->84h clear ?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER Pad")   PORT_CODE(KEYCODE_ENTER_PAD)PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)        PORT_CHAR('9')      PORT_CHAR(')')

PORT_START("COL.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ü Ü")       PORT_CODE(KEYCODE_OPENBRACE)PORT_CHAR(0x00fc)   PORT_CHAR(0x00dc)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ä Ä")       PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR(0x00e4)   PORT_CHAR(0x00c4)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R_SHIFT")     PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 Pad")       PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)        PORT_CHAR('0')      PORT_CHAR('=')

PORT_START("COL.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ *")         PORT_CODE(KEYCODE_CLOSEBRACE)PORT_CHAR('+')     PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("# ^")         PORT_CODE(KEYCODE_BACKSLASH)PORT_CHAR('#')      PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[][]/ESC")    PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))           // Esc test this work ?!
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL Pad")     PORT_CODE(KEYCODE_DEL_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"ß ?")       PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(0x00df)   PORT_CHAR('?')      // ß and ?

PORT_START("COL.11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')                         // TAB key
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 Pad")       PORT_CODE(KEYCODE_1_PAD)    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Pad")       PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"´ `")       PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR(0x00b4,'\'') PORT_CHAR(0x0060)

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
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"® /Ctrl")   PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_LCONTROL)         // scan 6Bh -> C2h funct.
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

PORT_START("SCREEN")
	PORT_CONFNAME(0x01, 0x00, "Screen")
	PORT_CONFSETTING(0x00, "Main")
	PORT_CONFSETTING(0x01, "Extension")

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

static const gfx_layout extcharlayout =
{
	8, 12,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*12
};

//**************************************************************************
//  VIDEO - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

u32 alphatp_12_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pen = m_palette->pens();
	int const start = m_crtc->upscroll_offset();
	rectangle cursor;
	m_crtc->cursor_bounds(cursor);
	for (int y = 0; y < 24; y++)
	{
		int const vramy = (start + y) % 24;
		for (int x = 0; x < 80; x++)
		{
			u8 code = m_vram[(vramy * 128) + x];   // helwie44 must be 128d is 080h physical display-ram step line
			// draw 12 lines of the character
			bool const cursoren = cursor.contains(x * 8, y * 12);
			for (int line = 0; line < 12; line++)
			{
				u8 data = m_gfx[((code & 0x7f) * 16) + line];
				if (cursoren)
					data ^= 0xff;
				bitmap.pix(y * 12 + line, x * 8 + 0) = pen[BIT(data, 0) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 1) = pen[BIT(data, 1) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 2) = pen[BIT(data, 2) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 3) = pen[BIT(data, 3) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 4) = pen[BIT(data, 4) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 5) = pen[BIT(data, 5) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 6) = pen[BIT(data, 6) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 7) = pen[BIT(data, 7) ^ BIT(code, 7)];
			}
		}
	}

	return 0;
}

//**************************************************************************
//  VIDEO - Alphatronic P3, P4, P30 and P40
//**************************************************************************

static GFXDECODE_START( gfx_alphatp3 )
	GFXDECODE_ENTRY("gfx", 0, charlayout, 0, 1)
GFXDECODE_END

u32 alphatp_34_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pen = m_palette->pens();
	int const start = m_crtc->upscroll_offset();
	rectangle cursor;
	m_crtc->cursor_bounds(cursor);
	bool const scrext = m_scncfg->read() ? true : false;
	for (int y = 0; y < 24; y++)
	{
		int const vramy = (start + y) % 24;
		for (int x = 0; x < 80; x++)
		{
			u8 code = m_vram[(vramy * 128) + x];   // helwie44 must be 128d is 080h physical display-ram step line
			// draw 12 lines of the character
			bool const cursoren = cursor.contains(x * 8, y * 12);
			for (int line = 0; line < 12; line++)
			{
				u8 data = 0;
				if (scrext)
				{
					offs_t offset = (((vramy * 12) + line) * 80) + x;
					if(offset < (371 * 80))
						data = m_vramext[offset];
					code = 0;
				}
				else
				{
					data = m_gfx[((code & 0x7f) * 16) + line];
					if (cursoren)
						data ^= 0xff;
				}
				bitmap.pix(y * 12 + line, x * 8 + 0) = pen[BIT(data, 0) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 1) = pen[BIT(data, 1) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 2) = pen[BIT(data, 2) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 3) = pen[BIT(data, 3) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 4) = pen[BIT(data, 4) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 5) = pen[BIT(data, 5) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 6) = pen[BIT(data, 6) ^ BIT(code, 7)];
				bitmap.pix(y * 12 + line, x * 8 + 7) = pen[BIT(data, 7) ^ BIT(code, 7)];
			}
		}
	}

	return 0;
}

//**************************************************************************
//  SOUND - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

void alphatp_12_state::beep_w(u8 data)
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  SOUND - Alphatronic P3, P4, P30 and P40
//**************************************************************************

void alphatp_34_state::beep_w(u8 data)
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  FLOPPY - Alphatronic P1, P2, P2S, P2U and Hell 2069
//**************************************************************************

void alphatp_12_state::fdcirq_w(int state)
{
	m_fdc_irq = state;
}

void alphatp_12_state::fdcdrq_w(int state)
{
	m_fdc_drq = state;
}

void alphatp_12_state::fdchld_w(int state)
{
	m_fdc_hld = state;
}

u8 alphatp_12_state::fdc_stat_r()
{
	u8 res = 0;
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

u8 alphatp_12_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void alphatp_12_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset, data ^ 0xff);
}


void alphatp_12_state::fdc_cmd_w(u8 data)
{
	floppy_image_device *floppy = nullptr;

	//logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

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

void alphatp_34_state::fdcirq_w(int state)
{
	m_fdc_irq = state;
}

void alphatp_34_state::fdcdrq_w(int state)
{
	m_fdc_drq = state;
}

void alphatp_34_state::fdchld_w(int state)
{
	m_fdc_hld = state;
}

u8 alphatp_34_state::fdc_stat_r()
{
	u8 res = 0;
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

u8 alphatp_34_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void alphatp_34_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset, data ^ 0xff);
}


void alphatp_34_state::fdc_cmd_w(u8 data)
{
	floppy_image_device *floppy = nullptr;

	//logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

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

static void alphatp2_floppies(device_slot_interface &device) // two BASF 2471 drives
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

static void alphatp2su_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void alphatp3_floppies(device_slot_interface &device) // P3:  two BASF 6106 drives
{
	device.option_add("525qd", FLOPPY_525_QD);       // P30: two Shugart SA465-3AA drives
}

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

void alphatp_12_state::alphatp2(machine_config &config)
{
	i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &alphatp_12_state::alphatp2_mem);
	maincpu.set_addrmap(AS_IO, &alphatp_12_state::alphatp2_io);

	config.set_perfect_quantum("maincpu");

	I8041A(config, m_kbdmcu, 12.8544_MHz_XTAL / 2);
	m_kbdmcu->t0_in_cb().set(FUNC(alphatp_12_state::kbd_matrix_r));
	m_kbdmcu->p1_out_cb().set(FUNC(alphatp_12_state::kbd_matrix_w));
	m_kbdmcu->p2_in_cb().set(FUNC(alphatp_12_state::kbd_port2_r));
	m_kbdmcu->p2_out_cb().set(FUNC(alphatp_12_state::kbd_port2_w));

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&alphatp_12_state::alphatp2_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(12.8544_MHz_XTAL, 824, 0, 640, 312, 0, 288);
	screen.set_screen_update(FUNC(alphatp_12_state::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CRT5027(config, m_crtc, 12.8544_MHz_XTAL / 8);
	m_crtc->set_char_width(8);
	m_crtc->hsyn_callback().set_inputline("maincpu", I8085_RST55_LINE);
	m_crtc->vsyn_callback().set_inputline("maincpu", I8085_RST65_LINE).exor(1);
	m_crtc->set_screen("screen");

	GFXDECODE(config, "gfxdecode", m_palette, gfx_alphatp3);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1060).add_route(ALL_OUTPUTS, "mono", 1.00);

	I8251(config, "uart", 0);
	// 4.9152_MHz_XTAL serial clock

	FD1791(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(alphatp_12_state::fdcirq_w));
	m_fdc->drq_wr_callback().set(FUNC(alphatp_12_state::fdcdrq_w));
	m_fdc->hld_wr_callback().set(FUNC(alphatp_12_state::fdchld_w));
	FLOPPY_CONNECTOR(config, "fdc:0", alphatp2_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", alphatp2_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
}

void alphatp_12_state::alphatp2u(machine_config &config)
{
	alphatp2(config);
	config.device_remove("fdc:0");
	config.device_remove("fdc:1");
	FLOPPY_CONNECTOR(config, "fdc:0", alphatp2su_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", alphatp2su_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
}


//**************************************************************************
//  MACHINE - Alphatronic P3, P4, P30 and P40
//**************************************************************************

void alphatp_34_state::machine_start()
{
	// setup banking
	membank("ram_0000")->set_base(m_ram.target());
	save_item(NAME(m_vramext));

	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
	if(m_i8088)
		m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, extcharlayout, &m_vramchr[0], 0, 1, 0));
}

void alphatp_34_state::machine_reset()
{
	m_kbdread = 1;
	m_kbdclk = 1;   m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = nullptr;
	m_88_da = m_85_da = m_88_started = false;
}
void alphatp_34_state::alphatp3(machine_config &config)
{
	i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &alphatp_34_state::alphatp3_mem);
	maincpu.set_addrmap(AS_IO, &alphatp_34_state::alphatp3_io);

	config.set_perfect_quantum("maincpu");

	I8041A(config, m_kbdmcu, 12.8544_MHz_XTAL / 2);
	m_kbdmcu->t0_in_cb().set(FUNC(alphatp_34_state::kbd_matrix_r));
	m_kbdmcu->p1_out_cb().set(FUNC(alphatp_34_state::kbd_matrix_w));
	m_kbdmcu->p2_in_cb().set(FUNC(alphatp_34_state::kbd_port2_r));
	m_kbdmcu->p2_out_cb().set(FUNC(alphatp_34_state::kbd_port2_w));

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&alphatp_34_state::alphatp3_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(12.8544_MHz_XTAL, 824, 0, 640, 312, 0, 288);
	screen.set_screen_update(FUNC(alphatp_34_state::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CRT5037(config, m_crtc, 12.8544_MHz_XTAL / 8);
	m_crtc->set_char_width(8);
	m_crtc->vsyn_callback().set_inputline("maincpu", I8085_RST65_LINE).exor(1);
	m_crtc->set_screen("screen");

	GFXDECODE(config, "gfxdecode", m_palette, gfx_alphatp3);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1060).add_route(ALL_OUTPUTS, "mono", 1.00);

	I8251(config, "uart", 0);
	// 4.9152_MHz_XTAL serial clock

	FD1791(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(alphatp_34_state::fdcirq_w));
	m_fdc->drq_wr_callback().set(FUNC(alphatp_34_state::fdcdrq_w));
	m_fdc->hld_wr_callback().set(FUNC(alphatp_34_state::fdchld_w));
	FLOPPY_CONNECTOR(config, "fdc:0", alphatp3_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", alphatp3_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
}

void alphatp_34_state::alphatp30(machine_config &config)
{
	alphatp3(config);
	I8088(config, m_i8088, 6000000); // unknown clock
	m_i8088->set_addrmap(AS_PROGRAM, &alphatp_34_state::alphatp30_8088_map);
	m_i8088->set_addrmap(AS_IO, &alphatp_34_state::alphatp30_8088_io);
	m_i8088->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));
	m_i8088->set_disable();

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_i8088, 0);
	m_pic->in_sp_callback().set_constant(0);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(100000);  // 15Mhz osc with unknown divisor
	pit.set_clk<1>(100000);
	pit.set_clk<2>(100000);
	pit.out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
}

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

	ROMX_LOAD("caap_96_00_5a.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(0) ) // earlier P2, three 16K RAM boards
	ROMX_LOAD("caap_05_02_12.bin", 0x0800, 0x0800, CRC(14f19693) SHA1(7ecb66818a3e352fede1857a18cd12bf742603a9), ROM_BIOS(0) )
	ROMX_LOAD("caap_94_01_50.bin", 0x1000, 0x0800, CRC(fda8d4a4) SHA1(fa91e6e8504e7f84cf69d86f72826ad5405fd82d), ROM_BIOS(0) )

	ROMX_LOAD("caap_06_00_17.bin", 0x0000, 0x0800, CRC(cb137796) SHA1(876bd0762faffc7b74093922d8fbf1c72ec70060), ROM_BIOS(1) ) // later P2, one 48K RAM board
	ROMX_LOAD("caap_05_01_12.bin", 0x0800, 0x0800, CRC(98c5ec7a) SHA1(b170e9a73b0b64d4111fa1170af6e333793da479), ROM_BIOS(1) )
	ROMX_LOAD("caap_04_01_03.bin", 0x1000, 0x0800, CRC(f304c3aa) SHA1(92213e77e4e6de12a4eaee25a9c1ec0ab54e93d4), ROM_BIOS(1) )

	ROMX_LOAD("caap_p2_es_1.bin", 0x0000, 0x0800, CRC(91b58eb3) SHA1(a4467cf9a14366198ee5f676b9471734e820522d), ROM_BIOS(2) )   // P2 Spain
	ROMX_LOAD("caap_p2_es_2.bin", 0x0800, 0x0800, CRC(f4dfac82) SHA1(266d1fdaef875515d9c68ae3e67ec88831bb55cb), ROM_BIOS(2) )
	ROMX_LOAD("caap_p2_es_3.bin", 0x1000, 0x0800, CRC(6f6918ba) SHA1(8dc9f5e337df8abf42e5b55f5f1a1a9d61c42d78), ROM_BIOS(2) )

	ROM_REGION(0x400, "kbdmcu", 0)                                                                                          // same across all dumped P2 and P3 machines so far
	ROM_LOAD("p2_keyboard_ip8041a_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51))   // needs to be checked with P2 sks and Spain

	ROM_REGION(0x800, "gfx", 0)
	ROMX_LOAD("cajp_97_00_5a.bin", 0x000, 0x800, CRC(aa5eab85) SHA1(2718924f5520e7e9aad635786847e78e3096b285), ROM_BIOS(0)) // came with caap94-96
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(1)) // came with caap04-06
	ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(2)) // sks KISS chargen not dumped yet
	// FIXME: no BIOS option for selecting this ROMX_LOAD("cajp_01_01_28.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(3)) // spanish P2 chargen not dumped yet
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
	ROM_SYSTEM_BIOS(0, "gx347", "gx347") // earlier P3, separate 48K and 16K RAM boards
	ROM_SYSTEM_BIOS(1, "lb352", "lb352") // later P3, one 64K RAM board

	ROM_LOAD("caap36_02_19.bin", 0x0000, 0x1000, CRC(23df6666) SHA1(5ea04cd299dec9951425eb91ecceb4818c4c6378) ) // identical between earlier and later P3
																												// BIOS names taken from CPU card labels
	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("p3_keyboard_8278.bin",  0x000, 0x400, CRC(5db00d85) SHA1(0dc8e274a5aece261ef60494901601c0d8b1eb51) )

	ROM_REGION(0x800, "gfx", 0)
	ROMX_LOAD("cajp08_00_15.bin", 0x000, 0x800, CRC(d6248209) SHA1(21689703de7183ecffb410eb8a6d516efe27da9d), ROM_BIOS(0) )
	ROMX_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8), ROM_BIOS(1) )
ROM_END

// Alphatronic P30
ROM_START( alphatp30 ) // P30 add-on card with 8088 needs to be emulated to boot DOS
	ROM_REGION(0x1800, "boot", 0)
	ROM_LOAD("hasl17.07.84.bin", 0x0000, 0x1000, CRC(6a91701b) SHA1(8a4f925d0fabab37852a54d04e06deb2aeaa349c))  // ...wait for INT6.5 or INT5.5 with RIM to write char in hsync or hsync GAP-time !!

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("caju_01_01_01.bin",  0x000, 0x400, CRC(e9b4359f) SHA1(835f4a580b4c108ef2f239039b765324adc7f078))

	ROM_REGION(0x800, "gfx", 0)
	ROM_LOAD("cajp08_01_15.bin", 0x000, 0x800, CRC(4ed11dac) SHA1(9db9b8e0edf471faaddbb5521d6223121146bab8))

	ROM_REGION(0x4000, "16bit", 0)
	ROM_LOAD("caxp_02_02_13.bin", 0x00000, 0x2000, CRC(e6bf6dd5) SHA1(dc87210bbcd96f3c1370565174a45199e3c1bc70)) // P30 ROM from 8088 card
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT    COMPAT   MACHINE    INPUT     CLASS             INIT        COMPANY          FULLNAME           FLAGS
COMP( 198?, alphatp1,  alphatp2, 0,       alphatp2,  alphatp2, alphatp_12_state, empty_init, "Triumph-Adler", "alphatronic P1",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, alphatp2,  0,        0,       alphatp2,  alphatp2, alphatp_12_state, empty_init, "Triumph-Adler", "alphatronic P2",  MACHINE_NOT_WORKING )
COMP( 198?, alphatp2u, alphatp2, 0,       alphatp2u, alphatp3, alphatp_12_state, empty_init, "Triumph-Adler", "alphatronic P2U", MACHINE_NOT_WORKING )
COMP( 1982, alphatp3,  0,        0,       alphatp3,  alphatp3, alphatp_34_state, empty_init, "Triumph-Adler", "alphatronic P3",  MACHINE_NOT_WORKING )
COMP( 198?, alphatp30, alphatp3, 0,       alphatp30, alphatp3, alphatp_34_state, empty_init, "Triumph-Adler", "alphatronic P30", MACHINE_NOT_WORKING )
