// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-100

    preliminary driver by Angelo Salese
    Thanks to Carl for the i8259 tip;

    TODO:
    - floppy support (no images available right now);
    - i8259 works in edge triggering mode, kludged to work somehow.

    Notes:
    - First two POST checks are for the irqs, first one checks the timer irq:
    F8209: B8 FB 00                  mov     ax,0FBh
    F820C: E6 02                     out     2h,al
    F820E: B9 00 00                  mov     cx,0h
    F8211: FB                        sti
    F8212: 0A E4                     or      ah,ah <- irq fires here
    F8214: E1 FC                     loopz   0F8212h
    F8216: FA                        cli
    F8217: 0A E9                     or      ch,cl
    F8219: 74 15                     je      0F8230h
    - Second one is for the vblank irq timing:
        F8238: 8B D3                     mov     dx,bx
        F823A: 8B D9                     mov     bx,cx
        F823C: CF                        iret
    F824D: E4 02                     in      al,2h
    F824F: 8A E0                     mov     ah,al
    F8251: B0 EF                     mov     al,0EFh
    F8253: E6 02                     out     2h,al
    F8255: BB 00 00                  mov     bx,0h
    F8258: BA 00 00                  mov     dx,0h
    F825B: B9 20 4E                  mov     cx,4E20h
    F825E: FB                        sti
    F825F: E2 FE                     loop    0F825Fh ;calculates the vblank here
    F8261: FA                        cli
    F8262: 8A C4                     mov     al,ah
    F8264: E6 02                     out     2h,al
    F8266: 2B D3                     sub     dx,bx
    F8268: 81 FA 58 1B               cmp     dx,1B58h
    F826C: 78 06                     js      0F8274h ;error if DX is smaller than 0x1b58
    F826E: 81 FA 40 1F               cmp     dx,1F40h
    F8272: 78 0A                     js      0F827Eh ;error if DX is greater than 0x1f40
    F8274: B1 05                     mov     cl,5h
    F8276: E8 CB 03                  call    0F8644h
    F8279: E8 79 FF                  call    0F81F5h
    F827C: EB FE                     jmp     0F827Ch
    F827E: B0 FF                     mov     al,0FFh
    fwiw with current timings, we get DX=0x1f09, enough for passing the test;

****************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm58321.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/d88_dsk.h"
#include "formats/mfi_dsk.h"


namespace {

class pc100_state : public driver_device
{
public:
	pc100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "upd765"),
		m_pic(*this, "pic8259"),
		m_palette(*this, "palette"),
		m_kanji_rom(*this, "kanji"),
		m_vram(*this, "vram"),
		m_keys(*this, "ROW.%u", 0U),
		m_rtc_portc(0)
	{
	}
	void pc100(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device<msm58321_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<pic8259_device> m_pic;
	required_device<palette_device> m_palette;
	required_region_ptr<uint16_t> m_kanji_rom;
	required_region_ptr<uint16_t> m_vram;
	required_ioport_array<6> m_keys;

	uint16_t pc100_vram_r(offs_t offset);
	void pc100_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pc100_kanji_r();
	void pc100_kanji_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t pc100_key_r(offs_t offset);
	void pc100_output_w(offs_t offset, uint8_t data);
	void pc100_tc_w(uint8_t data);
	uint8_t pc100_shift_r();
	void pc100_shift_w(uint8_t data);
	uint8_t pc100_vs_vreg_r(offs_t offset);
	void pc100_vs_vreg_w(offs_t offset, uint8_t data);
	void pc100_crtc_addr_w(uint8_t data);
	void pc100_crtc_data_w(uint8_t data);
	void lower_mask_w(uint8_t data);
	void upper_mask_w(uint8_t data);
	void crtc_bank_w(uint8_t data);
	void rtc_porta_w(uint8_t data);
	uint8_t rtc_portc_r();
	void rtc_portc_w(uint8_t data);
	void irqnmi_w(int state);
	void drqnmi_w(int state);
	uint16_t m_kanji_addr = 0;
	uint8_t m_timer_mode = 0;

	uint8_t m_bank_r = 0, m_bank_w = 0, m_key = 0;
	bool m_nmi_mask = false, m_irq_state = false, m_drq_state = false;

	struct{
		uint8_t shift = 0;
		uint16_t mask = 0;
		uint16_t cmd = 0;
		uint16_t vstart = 0;
		uint8_t addr = 0;
		uint8_t reg[8]{};
	}m_crtc;
	uint32_t screen_update_pc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pc100_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_600hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_100hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_50hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_10hz_irq);


	void rtc_portc_0_w(int state) { m_rtc_portc = (m_rtc_portc & ~(1 << 0)) | ((state & 1) << 0); }
	void rtc_portc_1_w(int state) { m_rtc_portc = (m_rtc_portc & ~(1 << 1)) | ((state & 1) << 1); }
	void rtc_portc_2_w(int state) { m_rtc_portc = (m_rtc_portc & ~(1 << 2)) | ((state & 1) << 2); }
	void rtc_portc_3_w(int state) { m_rtc_portc = (m_rtc_portc & ~(1 << 3)) | ((state & 1) << 3); }
	uint8_t m_rtc_portc;
	void pc100_io(address_map &map) ATTR_COLD;
	void pc100_map(address_map &map) ATTR_COLD;
};

void pc100_state::video_start()
{
}

uint32_t pc100_state::screen_update_pc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = (m_crtc.vstart + 0x20) * 0x40;

	for(int y=0;y<512;y++)
	{
		count &= 0xffff;

		for(int x=0;x<1024/16;x++)
		{
			for(int xi=0;xi<16;xi++)
			{
				if(m_crtc.cmd != 0xffff)
				{
					int pen[4];
					for(int pen_i=0;pen_i<4;pen_i++)
						pen[pen_i] = (m_vram[count+pen_i*0x10000] >> xi) & 1;

					int dot = 0;
					for(int pen_i=0;pen_i<4;pen_i++)
						dot |= pen[pen_i]<<pen_i;

					if(y < 512 && x*16+xi < 768) /* TODO: safety check */
						bitmap.pix(y, x*16+xi) = m_palette->pen(dot);
				}
				else
				{
					int dot = (m_vram[count] >> xi) & 1;

					if(y < 512 && x*16+xi < 768) /* TODO: safety check */
						bitmap.pix(y, x*16+xi) = m_palette->pen(dot ? 15 : 0);
				}
			}

			count++;
		}
	}

	return 0;
}

void pc100_state::irqnmi_w(int state)
{
	if(!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ((state == ASSERT_LINE) || m_drq_state) ? ASSERT_LINE : CLEAR_LINE);
	m_irq_state = state == ASSERT_LINE;
}

void pc100_state::drqnmi_w(int state)
{
	if(!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ((state == ASSERT_LINE) || m_irq_state) ? ASSERT_LINE : CLEAR_LINE);
	m_drq_state = state == ASSERT_LINE;
}

uint16_t pc100_state::pc100_vram_r(offs_t offset)
{
	return m_vram[offset+m_bank_r*0x10000];
}

void pc100_state::pc100_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t old_vram;
	for(int i=0;i<4;i++)
	{
		if((m_bank_w >> i) & 1)
		{
			old_vram = m_vram[offset+i*0x10000];
			COMBINE_DATA(&m_vram[offset+i*0x10000]);
			m_vram[offset+i*0x10000] = ((m_vram[offset+i*0x10000]|(m_vram[offset+i*0x10000]<<16)) >> m_crtc.shift);
			if(ACCESSING_BITS_0_15)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ~m_crtc.mask) | (old_vram & m_crtc.mask);
			else if(ACCESSING_BITS_8_15)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ((~m_crtc.mask) & 0xff00)) | (old_vram & (m_crtc.mask|0xff));
			else if(ACCESSING_BITS_0_7)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ((~m_crtc.mask) & 0xff)) | (old_vram & (m_crtc.mask|0xff00));

		}
	}
}

void pc100_state::pc100_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xbffff).ram(); // work ram
	map(0xc0000, 0xdffff).rw(FUNC(pc100_state::pc100_vram_r), FUNC(pc100_state::pc100_vram_w)); // vram, blitter based!
	map(0xf8000, 0xfffff).rom().region("ipl", 0);
}

uint16_t pc100_state::pc100_kanji_r()
{
	return m_kanji_rom[m_kanji_addr];
}


void pc100_state::pc100_kanji_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_kanji_addr);
}

uint8_t pc100_state::pc100_key_r(offs_t offset)
{
	if(offset)
		return ioport("DSW")->read(); // bit 5: horizontal/vertical monitor dsw
	else
	{
		m_pic->ir3_w(0);
		return m_key;
	}

	return 0;
}

void pc100_state::pc100_output_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		m_timer_mode = (data & 0x18) >> 3;
		m_beeper->set_state(((data & 0x40) >> 6) ^ 1);
		//printf("%02x\n",data & 0xc0);
	}
}

void pc100_state::pc100_tc_w(uint8_t data)
{
	m_fdc->tc_w(data & 0x40);
}

uint8_t pc100_state::pc100_shift_r()
{
	return m_crtc.shift;
}

void pc100_state::pc100_shift_w(uint8_t data)
{
	m_crtc.shift = data & 0xf;
}

uint8_t pc100_state::pc100_vs_vreg_r(offs_t offset)
{
	if(offset)
		return m_crtc.vstart >> 8;

	return m_crtc.vstart & 0xff;
}

void pc100_state::pc100_vs_vreg_w(offs_t offset, uint8_t data)
{
	if(offset)
		m_crtc.vstart = (m_crtc.vstart & 0xff) | (data << 8);
	else
		m_crtc.vstart = (m_crtc.vstart & 0xff00) | (data & 0xff);
}

void pc100_state::pc100_crtc_addr_w(uint8_t data)
{
	m_crtc.addr = data & 7;
}

void pc100_state::pc100_crtc_data_w(uint8_t data)
{
	m_crtc.reg[m_crtc.addr] = data;
	//printf("%02x %02x\n",m_crtc.addr,data);
}


/* everything is 8-bit bus wide */
void pc100_state::pc100_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff); // i8259
//  map(0x04, 0x07) i8237?
	map(0x08, 0x0b).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff); // upd765
	map(0x10, 0x17).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // i8255 #1
	map(0x18, 0x1f).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // i8255 #2
	map(0x20, 0x23).r(FUNC(pc100_state::pc100_key_r)).umask16(0x00ff); //i/o, keyboard, mouse
	map(0x22, 0x22).w(FUNC(pc100_state::pc100_output_w)); //i/o, keyboard, mouse
	map(0x24, 0x24).w(FUNC(pc100_state::pc100_tc_w)); //i/o, keyboard, mouse
	map(0x28, 0x2b).rw("uart8251", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x30, 0x30).rw(FUNC(pc100_state::pc100_shift_r), FUNC(pc100_state::pc100_shift_w)); // crtc shift
	map(0x38, 0x38).w(FUNC(pc100_state::pc100_crtc_addr_w)); //crtc address reg
	map(0x3a, 0x3a).w(FUNC(pc100_state::pc100_crtc_data_w)); //crtc data reg
	map(0x3c, 0x3f).rw(FUNC(pc100_state::pc100_vs_vreg_r), FUNC(pc100_state::pc100_vs_vreg_w)).umask16(0x00ff); //crtc vertical start position
	map(0x40, 0x5f).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x60, 0x61).lrw16(
			NAME([this] () { return m_crtc.cmd; }),
			NAME([this] (u16 d) { m_crtc.cmd = d; }));
	map(0x80, 0x81).rw(FUNC(pc100_state::pc100_kanji_r), FUNC(pc100_state::pc100_kanji_w));
	map(0x82, 0x83).nopw(); //kanji-related?
	map(0x84, 0x87).nopw(); //kanji "strobe" signal 0/1
}

INPUT_CHANGED_MEMBER(pc100_state::key_stroke)
{
	if(newval != oldval)
	{
		m_key = uint8_t(param & 0xff);
		if(!((newval ^ oldval) & newval))
			m_key |= 0x80;
		m_pic->ir3_w(1);
	}
}

/* Input ports */
static INPUT_PORTS_START( pc100 )
	PORT_START("ROW.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x00)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x01)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x02)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x03)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x04)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x05)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x06)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x07)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x08)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x09)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x0f)
	PORT_START("ROW.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x10)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x11)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x12)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x13)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x14)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x15)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x16)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x17)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x18)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x19)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x1f)
	PORT_START("ROW.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x20)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x21)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x22)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x23)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x24)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x25)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x26)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x27)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x28)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x29)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x2f)
	PORT_START("ROW.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x30)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x31)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x32)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x33)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x34)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x35)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x36)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x37)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x38)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x39)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x3f)
	PORT_START("ROW.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x40)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x41)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x42)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x43)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x44)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x45)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x46)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x47)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x48)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x49)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x4f)
	PORT_START("ROW.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x50)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x51)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x52)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x53)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x54)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x55)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x56)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x57)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x58)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x59)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, pc100_state, key_stroke, 0x5f)
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW" )
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
	PORT_DIPNAME( 0x20, 0x20, "Monitor" )
	PORT_DIPSETTING(    0x20, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8 },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_pc100 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanji_layout, 8, 1 )
GFXDECODE_END

/* TODO: untested */
void pc100_state::rtc_porta_w(uint8_t data)
{
/*
    ---- -x-- chip select
    ---- --x- read
    ---- ---x write
*/

	if(data != 0) // work around 8255 behavior that resets the whole chip on mode write
		m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(!(data & 0x20));
	m_rtc->write_w((data >> 0) & 1);
	m_rtc->read_w((data >> 1) & 1);
	m_rtc->cs1_w((data >> 2) & 1);
}

void pc100_state::rtc_portc_w(uint8_t data)
{
	m_rtc->d0_w((data >> 0) & 1);
	m_rtc->d1_w((data >> 1) & 1);
	m_rtc->d2_w((data >> 2) & 1);
	m_rtc->d3_w((data >> 3) & 1);
}

uint8_t pc100_state::rtc_portc_r()
{
	return m_rtc_portc;
}

void pc100_state::lower_mask_w(uint8_t data)
{
	m_crtc.mask = (m_crtc.mask & 0xff00) | data;
}

void pc100_state::upper_mask_w(uint8_t data)
{
	m_crtc.mask = (m_crtc.mask & 0xff) | (data << 8);
}

void pc100_state::crtc_bank_w(uint8_t data)
{
	if(data & 0x80)
	{
		m_nmi_mask = false;
		m_maincpu->set_input_line(INPUT_LINE_NMI, (m_irq_state || m_drq_state) ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		m_nmi_mask = true;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	m_bank_w = data & 0xf;
	m_bank_r = (data & 0x30) >> 4;
}

void pc100_state::machine_start()
{
	m_nmi_mask = true;
	m_irq_state = false;
	m_drq_state = false;
}

void pc100_state::machine_reset()
{
	m_beeper->set_state(0);
	m_key = 0xf0;
}

INTERRUPT_GEN_MEMBER(pc100_state::pc100_vblank_irq)
{
	m_pic->ir4_w(0);
	m_pic->ir4_w(1);
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_600hz_irq)
{
	if(m_timer_mode == 0)
	{
		m_pic->ir2_w(0);
		m_pic->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_100hz_irq)
{
	if(m_timer_mode == 1)
	{
		m_pic->ir2_w(0);
		m_pic->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_50hz_irq)
{
	if(m_timer_mode == 2)
	{
		m_pic->ir2_w(0);
		m_pic->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_10hz_irq)
{
	if(m_timer_mode == 3)
	{
		m_pic->ir2_w(0);
		m_pic->ir2_w(1);
	}
}

static void pc100_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

#define MASTER_CLOCK 6988800

void pc100_state::pc100(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc100_state::pc100_map);
	m_maincpu->set_addrmap(AS_IO, &pc100_state::pc100_io);
	m_maincpu->set_vblank_int("screen", FUNC(pc100_state::pc100_vblank_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	TIMER(config, "600hz").configure_periodic(FUNC(pc100_state::pc100_600hz_irq), attotime::from_hz(MASTER_CLOCK/600));
	TIMER(config, "100hz").configure_periodic(FUNC(pc100_state::pc100_100hz_irq), attotime::from_hz(MASTER_CLOCK/100));
	TIMER(config, "50hz").configure_periodic(FUNC(pc100_state::pc100_50hz_irq), attotime::from_hz(MASTER_CLOCK/50));
	TIMER(config, "10hz").configure_periodic(FUNC(pc100_state::pc100_10hz_irq), attotime::from_hz(MASTER_CLOCK/10));

	i8255_device &ppi1(I8255(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(pc100_state::rtc_porta_w));
	ppi1.in_pc_callback().set(FUNC(pc100_state::rtc_portc_r));
	ppi1.out_pc_callback().set(FUNC(pc100_state::rtc_portc_w));

	i8255_device &ppi2(I8255(config, "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(pc100_state::lower_mask_w));
	ppi2.out_pb_callback().set(FUNC(pc100_state::upper_mask_w));
	ppi2.out_pc_callback().set(FUNC(pc100_state::crtc_bank_w));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic->in_sp_callback().set_constant(0); // ???

	i8251_device &i8251(I8251(config, "uart8251", 0));
	//i8251.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//i8251.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	//i8251.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	i8251.rxrdy_handler().set(m_pic, FUNC(pic8259_device::ir1_w));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(pc100_state::irqnmi_w));
	m_fdc->drq_wr_callback().set(FUNC(pc100_state::drqnmi_w));

	MSM58321(config, m_rtc, XTAL(32'768));
	m_rtc->d0_handler().set(FUNC(pc100_state::rtc_portc_0_w));
	m_rtc->d1_handler().set(FUNC(pc100_state::rtc_portc_1_w));
	m_rtc->d2_handler().set(FUNC(pc100_state::rtc_portc_2_w));
	m_rtc->d3_handler().set(FUNC(pc100_state::rtc_portc_3_w));

	FLOPPY_CONNECTOR(config, "upd765:0", pc100_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pc100_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	/* TODO: Unknown Pixel Clock and CRTC is dynamic */
	screen.set_raw(MASTER_CLOCK*4, 1024, 0, 768, 264*2, 0, 512);
	screen.set_screen_update(FUNC(pc100_state::screen_update_pc100));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pc100);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_333, 16);

	SPEAKER(config, "mono").front_center();

	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( pc100 )
	ROM_REGION16_LE( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x8000, CRC(fd54a80e) SHA1(605a1b598e623ba2908a14a82454b9d32ea3c331))

	ROM_REGION16_LE( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, BAD_DUMP CRC(29298591) SHA1(d10174553ceea556fc53fc4e685d939524a4f64b))

	ROM_REGION16_LE( 0x20000*4, "vram", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 198?, pc100, 0,      0,      pc100,   pc100, pc100_state, empty_init, "NEC",   "PC-100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
