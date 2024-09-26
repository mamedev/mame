// license: GPL-2.0+
// copyright-holders: Angelo Salese, Jonathan Edwards, Christopher Edwards, Robbbert
/**************************************************************************************************

Basic Master Level 3 (MB-689x) "Peach" (c) 1980 Hitachi
ベーシックマスターレベル3

References:
- http://s-sasaji.ddo.jp/bml3mk5/tech.htm
- https://www.leadedsolder.com/2023/05/09/hitachi-basic-master-level-iii-mark-ii-cleaning-pickup.html
- https://github.com/bml3mk5/EmuB-6892/blob/master/src/docs/spec.txt

TODO:
- keyboard break NMI (as per bmjr);
- Move keyboard timer logic as 6845 hsync callback, fix logic (would key repeat too fast);
- Cassette baud rate bump (can switch from 600 to 1200 bauds thru $ffd7);
- implement sound as a bus slot device;
- implement RAM expansion as bus slots (RAM3 at 0x8000-0xbfff, RAM4 at 0xc000-0xefff);
- bml3mk5: BANK REG $ffe8 (applies EMS for the RAM expansion?);
- Cassettes requires manual press of play button when issuing a LOAD (verify);

**************************************************************************************************/

#include "emu.h"
#include "bml3.h"

#include "bus/bml3/kanji.h"
#include "bus/bml3/mp1802.h"
#include "bus/bml3/mp1805.h"
#include "bus/bml3/rtc.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/clock.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


MC6845_UPDATE_ROW( bml3_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	// The MB-6890 has a 5-bit colour RAM region.  The meaning of the bits are:
	// 0: blue
	// 1: red
	// 2: green
	// 3: reverse/inverse video
	// 4: graphic (not character)

	u8 const interlace = (m_crtc_vreg[8] & 3) ? 1 : 0;
	bool const lowres = BIT(m_hres_reg, 6);
	u8 const bgcolor = m_hres_reg & 7;

	if (interlace)
	{
		ra >>= 1;
		if (y > 0x191) return;
	}

	for(u8 x=0; x<x_count; x++)
	{
		u16 mem;
		if (lowres)
			mem = (ma + x - 0x400) & 0x3fff;
		else
			mem = (ma + x + ra * x_count/40 * 0x400 -0x400) & 0x3fff;

		u8 const attr = m_aram[mem];
		u8 const rawbits = m_vram[mem];
		// HACK: tracer bullet, should be composed in an entirely different way
		// This will never be hit by regular l3 being a specific feature of mk5,
		// still expect further headaches for S1 support ...
		if (get_ig_mode(attr))
		{
			for(u8 xi = 0; xi < 8; xi++)
			{
				u8 pen = get_ig_dots(rawbits, ra, 7 - xi);
				bitmap.pix(y, x*8+xi) = palette[pen];
			}

			continue;
		}

		u8 const color = attr & 7;
		bool const reverse = BIT(attr, 3) ^ (x == cursor_x);
		bool const graphic = BIT(attr, 4);


		u8 dots[2] = { 0, 0 };
		if (graphic)
		{
			if (lowres)
			{
				// low-res graphics, each tile has 8 bits arranged as follows:
				// 4 0
				// 5 1
				// 6 2
				// 7 3
				dots[0] = dots[1] = (rawbits >> ra/2 & 0x11) * 0xf;
			}
			else
			{
				dots[0] = dots[1] = rawbits;
			}
		}
		else
		{
			// character mode
			int const tile = rawbits & 0x7f;
			int const tile_bank = BIT(rawbits, 7);
			if (interlace)
			{
				dots[0] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(ra<<1)];
				dots[1] = m_p_chargen[(tile_bank<<11)|(tile<<4)|(ra<<1)|tile_bank];
			}
			else
			{
				dots[0] = dots[1] = m_p_chargen[(tile<<4)|(ra<<1)|tile_bank];
			}
		}

		for(u8 hf=0;hf<=interlace;hf++)
		{
			for(u8 xi=0;xi<8;xi++)
			{
				u8 pen;
				if(reverse)
					pen = (dots[hf] >> (7-xi) & 1) ? bgcolor : color;
				else
					pen = (dots[hf] >> (7-xi) & 1) ? color : bgcolor;

				bitmap.pix(y, x*8+xi) = palette[pen];
				// when the mc6845 device gains full interlace&video support, replace the line above with the line below
				// bitmap.pix(y*(interlace+1)+hf, x*8+xi) = palette[pen];
			}
		}
	}
}


u8 bml3_state::mc6845_r(offs_t offset)
{
	if (offset)
		return m_crtc->register_r();
	else
		return m_crtc->status_r();
}

void bml3_state::mc6845_w(offs_t offset, u8 data)
{
	if(offset == 0)
	{
		m_crtc_index = data;
		m_crtc->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(data);
	}
}

u8 bml3_state::kb_sel_r()
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	u8 ret = m_keyb_scancode;
	if (!machine().side_effects_disabled())
		m_keyb_scancode &= 0x7f;
	return ret;
}

// KB SEL - Keyboard mode register, interrupt control, keyboard LEDs
void bml3_state::kb_sel_w(u8 data)
{
	m_keyb_katakana_led_on = BIT(data, 0);
	m_keyb_hiragana_led_on = BIT(data, 1);
	m_keyb_capslock_led_on = BIT(data, 2);
	m_keyb_counter_operation_disabled = BIT(data, 3);
	m_keyb_interrupt_enabled = BIT(data, 6);
	m_keyb_nmi_disabled = !BIT(data, 7);
}

void bml3_state::crtc_change_clock()
{
	const u8 width80 = BIT(m_hres_reg, 7);
	const u8 interlace = BIT(m_vres_reg, 3);
	// CRTC and MPU are synchronous by default
	int clock = (width80 ? C80_CLOCK : C40_CLOCK).value() << interlace;
	m_crtc->set_unscaled_clock(clock);
}

/*
 * MODE_SEL - Graphics mode select
 * cfr. see service manual p.43
 * x--- ---- "W" bit: 0 = 40 columns, 1 = 80 columns
 * -x-- ---- "HR" bit: 0 = high resolution, 1 = normal
 * --x- ---- "C" bit - ACIA mode: 0 = cassette, 1 = RS-232C
 * ---- -RGB Background colour
 */
void bml3_state::mode_sel_w(u8 data)
{
	m_hres_reg = data;

	crtc_change_clock();
}

// INTERLACE_SEL - Interlaced video mode
void bml3_state::interlace_sel_w(u8 data)
{
	m_vres_reg = data;

	crtc_change_clock();
}


u8 bml3_state::vram_r(offs_t offset)
{
	// Bit 7 masks reading back to the latch
	if (!BIT(m_attr_latch, 7) && !machine().side_effects_disabled())
		m_attr_latch = m_aram[offset];

	return m_vram[offset];
}

void bml3_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	// color ram is 5-bit
	// NOTE: will break hiwriter with this, "write enable" only on reads!?
	//if (!BIT(m_attr_latch, 7))
	m_aram[offset] = m_attr_latch & get_attr_mask();
}

u8 bml3_state::psg_latch_r()
{
	return 0x7f;
}

void bml3_state::psg_latch_w(u8 data)
{
	m_psg_latch = data;
}

u8 bml3_state::ym2203_r()
{
	u8 dev_offs = ((m_psg_latch & 3) != 3);

	return m_ym2203->read(dev_offs);
}

void bml3_state::ym2203_w(u8 data)
{
	u8 dev_offs = ((m_psg_latch & 3) != 3);

	m_ym2203->write(dev_offs, data);
}

/*
 * C-REG-SELECT register
 * Reads from a VRAM address copy the corresponding 'colour RAM' address to the
 * low-order 5 bits of this register as a side-effect
 */
u8 bml3_state::c_reg_sel_r()
{
	return m_attr_latch;
}

/*
 * C_REG_SEL - Attribute register (character/video mode and colours)
 * Writes to a VRAM address copy the low-order 5 bits of this register to the corresponding 'colour RAM' address as a side-effect
 * x--- ---- "MK" bit: 0 = enable write, 1 = prohibit write
 * ---x ---- "GC" bit: 0 = character, 1 = graphic
 * ---- x--- "RV" bit: 0 = normal, 1 - reverse
 * ---- -RGB Foreground colour
 *
 */
void bml3_state::c_reg_sel_w(u8 data)
{
	m_attr_latch = data;
}

u8 bml3_state::music_sel_r()
{
	return -1; // BEEP status read?
}

// MUSIC SEL - Music select: toggle audio output level when rising
void bml3_state::music_sel_w(u8 data)
{
	m_speaker->level_w(BIT(data, 7));
}

// REMOTE - Remote relay control for cassette - bit 7
void bml3_state::remote_w(u8 data)
{
	m_cassette->change_state(
		BIT(data, 7) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

// KBNMI - Keyboard "Break" key non-maskable interrupt
u8 bml3_state::kbnmi_r()
{
	return m_nmi; // bit 7 used to signal a BREAK key pressure
}

// TIME_MASK - Prohibit timer IRQ
void bml3_state::time_mask_w(u8 data)
{
	m_firq_mask = data & 0x80;
	if(m_firq_mask)
	{
		m_firq_status = 0; // clear pending firq
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	}
}

// TIMER - System timer enable
u8 bml3_state::timer_r()
{
	u8 res = m_firq_status << 7;
	if (!machine().side_effects_disabled())
	{
		m_firq_status = 0;
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	}
	return res;
}


void bml3_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x43ff).rw(FUNC(bml3_state::vram_r), FUNC(bml3_state::vram_w));
	map(0x4400, 0x7fff).ram();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).view(m_banka);
	m_banka[0](0xa000, 0xbfff).readonly().share("rama");
	map(0xa000, 0xbfff).writeonly().share("rama");
	map(0xc000, 0xdfff).view(m_bankc);
	m_bankc[0](0xc000, 0xdfff).readonly().share("ramc");
	map(0xc000, 0xdfff).writeonly().share("ramc");
	map(0xe000, 0xefff).view(m_banke);
	m_banke[0](0xe000, 0xefff).readonly().share("rame");
	map(0xe000, 0xefff).writeonly().share("rame");
	map(0xf000, 0xfeff).view(m_bankf);
	m_bankf[0](0xf000, 0xfeff).readonly().share("ramf");
	map(0xf000, 0xfeff).writeonly().share("ramf");
	map(0xfff0, 0xffff).view(m_bankg);
	m_bankg[0](0xfff0, 0xffff).readonly().share("ramg");
	map(0xfff0, 0xffff).writeonly().share("ramg");
	map(0x8000, 0xffff).view(m_rom_view);
	m_rom_view[0](0xa000, 0xfeff).rom().region("maincpu", 0xa000);
	m_rom_view[0](0xfff0, 0xffff).rom().region("maincpu", 0xfff0);
	map(0xff00, 0xffef).m(*this, FUNC(bml3_state::system_io));
}

// Relative to $ffxx block
void bml3_state::system_io(address_map &map)
{
	map(0x0040, 0x0046).noprw(); // RS-232C
	map(0x00c0, 0x00c3).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00c4, 0x00c5).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x00c6, 0x00c7).rw(FUNC(bml3_state::mc6845_r), FUNC(bml3_state::mc6845_w));
	map(0x00c8, 0x00c8).r(FUNC(bml3_state::kbnmi_r));
	map(0x00c9, 0x00c9).portr("DIPSW");
	map(0x00ca, 0x00ca).r(FUNC(bml3_state::timer_r));
//  map(0x00cb, 0x00cb) LPFLG - Light pen interrupt
	map(0x00d0, 0x00d0).w(FUNC(bml3_state::mode_sel_w));
//  map(0x00d1, 0x00d1) TRACE - Trace counter
	map(0x00d2, 0x00d2).w(FUNC(bml3_state::remote_w));
	map(0x00d3, 0x00d3).rw(FUNC(bml3_state::music_sel_r), FUNC(bml3_state::music_sel_w));
	map(0x00d4, 0x00d4).w(FUNC(bml3_state::time_mask_w));
	map(0x00d5, 0x00d5).noprw(); // L/P ENBL - Light pen operation enable
	map(0x00d6, 0x00d6).w(FUNC(bml3_state::interlace_sel_w));
//  map(0x00d7, 0x00d7) BANK SEL - baud select
	map(0x00d8, 0x00d8).rw(FUNC(bml3_state::c_reg_sel_r), FUNC(bml3_state::c_reg_sel_w));
	map(0x00e0, 0x00e0).rw(FUNC(bml3_state::kb_sel_r), FUNC(bml3_state::kb_sel_w));
//  map(0x00e8, 0x00e8) bank register
//  map(0x00e9, 0x00e9) IG mode register
//  map(0x00ea, 0x00ea) IG enable register

#if 0
	map(0xff00, 0xff00).rw(FUNC(bml3_state::ym2203_r), FUNC(bml3_state::ym2203_w));
	map(0xff02, 0xff02).rw(FUNC(bml3_state::psg_latch_r), FUNC(bml3_state::psg_latch_w)); // PSG address/data select
#endif
}

void bml3mk5_state::ig_ram_w(offs_t offset, u8 data)
{
	for (int i = 0; i < 3; i++)
	{
		if (BIT(m_igen, i))
			m_ig_ram[offset + 0x800 * i] = data;
	}
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
}

void bml3mk5_state::main_map(address_map &map)
{
	bml3_state::main_map(map);
	map(0xa000, 0xa7ff).view(m_ig_view);
	m_ig_view[0](0xa000, 0xa7ff).writeonly().w(FUNC(bml3mk5_state::ig_ram_w));
}

void bml3mk5_state::system_io(address_map &map)
{
	bml3_state::system_io(map);
	// IGMODREG
	map(0x00e9, 0x00e9).lw8(
		NAME([this] (u8 data) {
			if (BIT(data, 0))
				m_ig_view.select(0);
			else
				m_ig_view.disable();
		})
	);
	// IGENREG
	map(0x00ea, 0x00ea).lw8(
		NAME([this] (u8 data) {
			m_igen = data & 7;
		})
	);
}

INPUT_CHANGED_MEMBER(bml3_state::nmi_button)
{
	// TODO: supposed to actually raise an NMI, just like earlier Basic Master LV1/2
	m_nmi = newval ? 0x80 : 0;
}

static INPUT_PORTS_START( bml3 )

	// DIP switches (service manual p.88)
	// Note the NEWON command reboots with a soft override for the DIP switch
	PORT_START("DIPSW")
	PORT_DIPNAME( 0x01, 0x01, "BASIC/terminal mode") PORT_DIPLOCATION("SW:!1")
	PORT_DIPSETTING(0x00, "Terminal mode")
	PORT_DIPSETTING(0x01, "BASIC mode")
	PORT_DIPNAME( 0x02, 0x02, "Interlaced video") PORT_DIPLOCATION("SW:!2")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x02, DEF_STR( On ))
	// This is overridden by the 'Mode' toggle button
	PORT_DIPNAME( 0x04, 0x04, "40-/80-column") PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(0x00, "40 chars/line")
	PORT_DIPSETTING(0x04, "80 chars/line")
	PORT_DIPNAME( 0x08, 0x00, "Video resolution") PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(0x00, "High")
	PORT_DIPSETTING(0x08, "Low")
	PORT_DIPNAME( 0x10, 0x00, "Show PF key content") PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(0x10, DEF_STR ( On ))
	PORT_DIPNAME( 0x20, 0x00, "Terminal duplex") PORT_DIPLOCATION("SW:!6")
	PORT_DIPSETTING(0x00, "Full duplex")
	PORT_DIPSETTING(0x20, "Half duplex")
	PORT_DIPNAME( 0x40, 0x00, "Terminal bits") PORT_DIPLOCATION("SW:!7")
	PORT_DIPSETTING(0x00, "8 bits/char")
	PORT_DIPSETTING(0x40, "7 bits/char")
	PORT_DIPNAME( 0x80, 0x00, "Hiragana->Katakana") PORT_DIPLOCATION("SW:!8")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )

// TODO: also model the CS jumper block?

	// RAM expansion configurations (see service manual p.76)
/*
    PORT_START("RAM")
    PORT_DIPNAME( 0x0003, 0x0002, "RAM size" )
    PORT_DIPSETTING(   0x0000, "32 kiB (standard)" )
    PORT_DIPSETTING(   0x0001, "40 kiB (32 kiB + 8 kiB)" )
    PORT_DIPSETTING(   0x0002, "60 kiB (32 kiB + 28 kiB)" )
*/

	PORT_START("X0") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("? PAD")
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X2")  // ???
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Lock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Shift")  PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 PAD") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 PAD") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('^')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 PAD") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete PAD") //backspace
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(u8"¥") PORT_CODE(KEYCODE_TAB)

	PORT_START("X1") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 PAD") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". PAD")  PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) //or cls?
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 PAD") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 PAD") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 PAD") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- PAD") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("X2") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ PAD") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 PAD") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 PAD") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 PAD") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0xffe00000,IP_ACTIVE_HIGH,IPT_UNKNOWN)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHANGED_MEMBER(DEVICE_SELF, bml3_state, nmi_button, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( bml3mk5 )
	PORT_INCLUDE( bml3 )
	// No dipswitches on Mark 5
	PORT_MODIFY("DIPSW")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	// TODO: add MODE front panel button here, in place of dipswitch
	// On regular bml3 there's an extra button that xor content, that is directly routed here
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(bml3_state::keyboard_callback)
{
	bool trigger = false;

	if(!BIT(m_keyb_scancode, 7))
	{
		m_keyb_scancode = (m_keyb_scancode + 1) & 0x7F;
		if (m_keyb_counter_operation_disabled)
		{
			m_keyb_scancode &= 0x7;
		}

		if (m_keyb_scancode == 0x7F)
		{
			if (m_keyb_empty_scan == 1)
			{
				// full scan completed with no keypress
				trigger = true;
			}
			if (m_keyb_empty_scan > 0)
				m_keyb_empty_scan--;
		}
		else if (m_keyb_scancode < 32*3)
		{
			u8 port_i = m_keyb_scancode / 32;
			u8 i = m_keyb_scancode % 32;
			if(BIT(m_io_keyboard[port_i]->read(),i))
			{
				m_keyb_empty_scan = 2;
				trigger = true;
			}
		}
		if (trigger)
		{
			m_kbt = 0xfff;
			m_keyb_scancode |= 0x80;
			if (m_keyb_interrupt_enabled)
				m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
		}
	}
	else
	{
		if (m_kbt > 0)
			m_kbt--;
		if (m_kbt == 1)
			m_keyb_scancode &= 0x7f;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( bml3_state::kansas_r )
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cassette->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}


INTERRUPT_GEN_MEMBER(bml3_state::timer_firq)
{
	if(!m_firq_mask)
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_firq_status = 1;
	}
}

void bml3_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x4000);
	m_aram = make_unique_clear<u8[]>(0x4000);

	// TODO: this setup is goofy, and bound to fail if things changes downstream.
	// Should really overlay the ROM region, not the view itself.
	// Additionally each slot has separate ROM KIL and EXROM KIL signal controls ...
	m_bml3bus->map_exrom(m_rom_view[0]);
	m_bml3bus->map_io(m_maincpu->space(AS_PROGRAM));

	save_pointer(NAME(m_vram), 0x4000);
	save_pointer(NAME(m_aram), 0x4000);

	save_item(NAME(m_hres_reg));
	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_psg_latch));
	save_item(NAME(m_attr_latch));
	save_item(NAME(m_vres_reg));
	save_item(NAME(m_keyb_interrupt_enabled));
	save_item(NAME(m_keyb_nmi_disabled));
	save_item(NAME(m_keyb_counter_operation_disabled));
	save_item(NAME(m_keyb_empty_scan));
	save_item(NAME(m_keyb_scancode));
	save_item(NAME(m_keyb_capslock_led_on));
	save_item(NAME(m_keyb_hiragana_led_on));
	save_item(NAME(m_keyb_katakana_led_on));
	save_item(NAME(m_kbt));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_crtc_index));
	save_item(NAME(m_firq_mask));
	save_item(NAME(m_firq_status));
	save_item(NAME(m_nmi));
}

void bml3_state::machine_reset()
{
	/* defaults */
	m_rom_view.select(0);
	m_banka.disable();
	m_bankc.disable();
	m_banke.disable();
	m_bankf.disable();
	m_bankg.disable();

	m_firq_mask = -1; // disable firq
	m_psg_latch = 0;
	m_attr_latch = 0;
	m_vres_reg = 0;
	m_keyb_interrupt_enabled = 0;
	m_keyb_nmi_disabled = 0;
	m_keyb_counter_operation_disabled = 0;
	m_keyb_empty_scan = 0;
	m_keyb_scancode = 0;
	m_keyb_capslock_led_on = 0;
	m_keyb_hiragana_led_on = 0;
	m_keyb_katakana_led_on = 0;
	m_crtc_index = 0;
	m_firq_status = 0;
	m_cassbit = 0;
	m_cassold = 0;
	m_nmi = 0;
	m_kbt = 0;

	// NOTE: bml3 do not bother with CRTC on soft resets (which is physically tied to front panel).
	//m_hres_reg = 0;
	//m_vres_reg = 0;
	//crtc_change_clock();
}

void bml3mk5_state::machine_start()
{
	bml3_state::machine_start();
	m_ig_ram = make_unique_clear<u8[]>(0x800 * 3);
	m_gfxdecode->gfx(0)->set_source_and_total(m_ig_ram.get(), 0x100);

	save_pointer(NAME(m_ig_ram), 0x800 * 3);
}

void bml3mk5_state::machine_reset()
{
	bml3_state::machine_reset();
	m_ig_view.disable();
	m_igen = 0;
}

void bml3_state::piaA_w(uint8_t data)
{
	/* ROM banking:
	-0-- --0- 0xa000 - 0xbfff ROM R RAM W
	-1-- --0- 0xa000 - 0xbfff RAM R/W
	-x-- --1- 0xa000 - 0xbfff no change
	0--- -0-- 0xc000 - 0xdfff ROM R RAM W
	1--- -0-- 0xc000 - 0xdfff RAM R/W
	x--- -1-- 0xc000 - 0xdfff no change
	0--- 0--- 0xe000 - 0xefff ROM R RAM W
	1--- 0--- 0xe000 - 0xefff RAM R/W
	x--- 1--- 0xe000 - 0xefff no change
	---- ---x 0xf000 - 0xfeff (0) ROM R RAM W (1) RAM R/W
	---- --x- 0xfff0 - 0xffff (0) ROM R RAM W (1) RAM R/W
	*/
	logerror("Check banking PIA A -> %02x\n",data);

	if (!BIT(data, 1))
	{
		if (BIT(data, 6))
			m_banka.select(0);
		else
			m_banka.disable();
	}

	if (!BIT(data, 2))
	{
		if (BIT(data, 7))
			m_bankc.select(0);
		else
			m_bankc.disable();
	}

	if (!BIT(data, 3))
	{
		if (BIT(data, 7))
			m_banke.select(0);
		else
			m_banke.disable();
	}

	if (BIT(data, 0))
		m_bankf.select(0);
	else
		m_bankf.disable();

	if (BIT(data, 1))
		m_bankg.select(0);
	else
		m_bankg.disable();
}

void bml3_state::acia_rts_w(int state)
{
	logerror("%02x TAPE RTS\n",state);
}

void bml3_state::acia_irq_w(int state)
{
	logerror("%02x TAPE IRQ\n",state);
}

TIMER_DEVICE_CALLBACK_MEMBER( bml3_state::kansas_w )
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cassette->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cassette->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

// Debugging only
static const gfx_layout ig_charlayout =
{
	8, 8,
	0x100,
	3,
	{ 0x1000*8, 0x800*8, 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8*8
};

static GFXDECODE_START( gfx_bml3mk5 )
	GFXDECODE_ENTRY( nullptr, 0, ig_charlayout, 0, 1 )
GFXDECODE_END


static void bml3_cards(device_slot_interface &device)
{
	device.option_add("mp1802", BML3BUS_MP1802);
	device.option_add("mp1805", BML3BUS_MP1805);
	device.option_add("kanji",  BML3BUS_KANJI);
	device.option_add("rtc",    BML3BUS_RTC);
}

void bml3_state::bml3(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, CPU_EXT_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &bml3_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(bml3_state::timer_firq));
//  m_maincpu->set_periodic_int(FUNC(bml3_state::firq), attotime::fromhz(45));

	// fire once per scan of an individual key
	// According to the service manual (p.65), the keyboard timer is driven by the horizontal video sync clock.
	TIMER(config, "keyboard_timer").configure_periodic(FUNC(bml3_state::keyboard_callback), attotime::from_hz(H_CLOCK/2));
	TIMER(config, "kansas_w").configure_periodic(FUNC(bml3_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(bml3_state::kansas_r), attotime::from_hz(40000));

	pia6821_device &pia(PIA6821(config, "pia"));
	pia.writepa_handler().set(FUNC(bml3_state::piaA_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });
	m_acia->rts_handler().set(FUNC(bml3_state::acia_rts_w));
	m_acia->irq_handler().set(FUNC(bml3_state::acia_irq_w));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 9'600)); // 600 baud x 16(divider) = 9600
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("bml3_cass");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Service manual specifies "Raster return period" as 2.4 ms (p.64),
	// although the total vertical non-displaying time seems to be 4 ms.
	// NOTE: D80_CLOCK x 2 as per MAME interlace limitation
	screen.set_raw(D80_CLOCK * 2, 1024, 0, 640 - 1, 518, 0, 400 - 1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	/* Devices */
	// CRTC clock should be synchronous with the CPU clock.
	HD6845S(config, m_crtc, CPU_CLOCK); // HD46505SP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(bml3_state::crtc_update_row));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	BML3BUS(config, m_bml3bus, 0);
	m_bml3bus->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_bml3bus->irq_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_bml3bus->firq_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	/* Default to nothing, to stop machine hanging at start.
	   Can use MP-1805 disk (3" or 5.25" SS/SD), as our MB-6892 ROM dump includes the MP-1805 ROM.
	   Or use MP-1802 (5.25" DS/DD).
	   Note it isn't feasible to use both, as they each place boot ROM at F800.
	 */
	// TODO: find actual defaults for each variant (should be no option for at least base model)
	BML3BUS_SLOT(config, "sl1", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl2", m_bml3bus, bml3_cards, "rtc");
	BML3BUS_SLOT(config, "sl3", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl4", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl5", m_bml3bus, bml3_cards, nullptr);
	BML3BUS_SLOT(config, "sl6", m_bml3bus, bml3_cards, "kanji");

	SOFTWARE_LIST(config, "cass_list").set_original("bml3_cass");

#if 0
	// TODO: slot device for sound card
	// audio
	YM2203(config, m_ym2203, 2000000); //unknown clock / divider
	m_ym2203->set_flags(AY8910_LEGACY_OUTPUT);
	m_ym2203->add_route(0, "mono", 0.25);
	m_ym2203->add_route(1, "mono", 0.25);
	m_ym2203->add_route(2, "mono", 0.50);
	m_ym2203->add_route(3, "mono", 0.50);
#endif
}

void bml3mk2_state::bml3mk2(machine_config &config)
{
	bml3_state::bml3(config);
	// TODO: override bus defaults
}

void bml3mk5_state::bml3mk5(machine_config &config)
{
	bml3mk2_state::bml3mk2(config);
	// TODO: override bus defaults
	GFXDECODE(config, "gfxdecode", "palette", gfx_bml3mk5);
}

/* ROM definition */
// floppy-drive slot devices expect "maincpu" is sized 0x10000.
ROM_START( bml3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	// TODO: replace with MB-6890 ROMs (these are from an MB-6892)
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	// TODO: Replace checksums with a ROM dump without a disk controller patch (checksums here are inclusive of the MP1805 patch)
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x0000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted
ROM_END

ROM_START( bml3mk2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	// TODO: replace with MB-6891 ROMs (these are from an MB-6892)
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x0000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted
ROM_END

ROM_START( bml3mk5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "l3bas.rom", 0xa000, 0x6000, BAD_DUMP CRC(d81baa07) SHA1(a8fd6b29d8c505b756dbf5354341c48f9ac1d24d)) //original, 24k isn't a proper rom size!
	/* Handcrafted ROMs, rom labels and contents might not match */
	ROM_LOAD( "598 p16611.ic3", 0xa000, 0x2000, BAD_DUMP CRC(954b9bad) SHA1(047948fac6808717c60a1d0ac9205a5725362430))
	ROM_LOAD( "599 p16561.ic4", 0xc000, 0x2000, BAD_DUMP CRC(b27a48f5) SHA1(94cb616df4caa6415c5076f9acdf675acb7453e2))
	// TODO: Replace checksums with a ROM dump without a disk controller patch (checksums here are inclusive of the MP1805 patch)
	ROM_LOAD( "600 p16681.ic5", 0xe000, 0x2000, BAD_DUMP CRC(fe3988a5) SHA1(edc732f1cd421e0cf45ffcfc71c5589958ceaae7))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x0000, 0x1000, BAD_DUMP CRC(0b6f2f10) SHA1(dc411b447ca414e94843636d8b5f910c954581fb) ) // handcrafted
ROM_END

COMP( 1980, bml3,    0,     0,      bml3,    bml3,     bml3_state,    empty_init, "Hitachi", "Basic Master Level 3 (MB-6890)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1982, bml3mk2, bml3,  0,      bml3mk2, bml3,     bml3mk2_state, empty_init, "Hitachi", "Basic Master Level 3 Mark II (MB-6891)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1983, bml3mk5, bml3,  0,      bml3mk5, bml3mk5,  bml3mk5_state, empty_init, "Hitachi", "Basic Master Level 3 Mark 5 (MB-6892)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
