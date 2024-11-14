// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Multi 8 (c) 1983 Mitsubishi

    preliminary driver by Angelo Salese

    TODO:
    - dunno how to trigger the text color mode in BASIC, I just modify
      $f0b1 to 1 for now
    - bitmap B/W mode is untested
    - keyboard
    - check cassette with real software

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/beep.h"
#include "sound/ymopn.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class multi8_state : public driver_device
{
public:
	multi8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_ppi(*this, "ppi")
		, m_crtc(*this, "crtc")
		, m_uart(*this, "uart")
		, m_cass(*this, "cassette")
		, m_beeper(*this, "beeper")
		, m_palette(*this, "palette")
		, m_aysnd(*this, "aysnd")
	{ }

	void multi8(machine_config &config);

private:
	uint8_t key_input_r();
	uint8_t key_status_r();
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t pal_r(offs_t offset);
	void pal_w(offs_t offset, uint8_t data);
	uint8_t kanji_r(offs_t offset);
	void kanji_w(offs_t offset, uint8_t data);
	uint8_t porta_r();
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);
	void ym2203_porta_w(uint8_t data);
	uint8_t ay8912_0_r();
	uint8_t ay8912_1_r();
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void kansas_w(int state);
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t *m_p_vram = nullptr;
	uint8_t *m_p_wram = nullptr;
	uint8_t *m_p_kanji = nullptr;
	uint8_t m_mcu_init = 0;
	uint8_t m_keyb_press = 0;
	uint8_t m_keyb_press_flag = 0;
	uint8_t m_shift_press_flag = 0;
	uint8_t m_display_reg = 0;
	uint8_t m_vram_bank = 0;
	uint8_t m_pen_clut[8]{};
	uint8_t m_bw_mode = 0;
	uint16_t m_knj_addr = 0;
	u8 m_cass_data[4]{};
	bool m_cassbit = 0, m_cassold = 0;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<i8255_device> m_ppi;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beeper;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_aysnd;
};

void multi8_state::video_start()
{
	m_keyb_press = m_keyb_press_flag = m_shift_press_flag = m_display_reg = 0;

	for (m_bw_mode = 0; m_bw_mode < 8; m_bw_mode++)
		m_pen_clut[m_bw_mode]=0;

	m_vram_bank = 8;
	m_bw_mode = 0;
}

MC6845_UPDATE_ROW( multi8_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u16 mem = y*80;
	u32 *p = &bitmap.pix(y);

	for(u16 x = 0; x < x_count; x++)
	{
		for(u8 i = 0; i < 8; i++)
		{
			u8 pen_b = BIT(m_p_vram[mem | 0x0000], 7-i);
			u8 pen_r = BIT(m_p_vram[mem | 0x4000], 7-i);
			u8 pen_g = BIT(m_p_vram[mem | 0x8000], 7-i);
			u8 color = (pen_b) | (pen_r << 1) | (pen_g << 2);

			if (m_bw_mode)
			{
				pen_b = BIT(m_display_reg, 0) ? pen_b : 0;
				pen_r = BIT(m_display_reg, 1) ? pen_r : 0;
				pen_g = BIT(m_display_reg, 2) ? pen_g : 0;

				color = (pen_b | pen_r | pen_g) ? 7 : 0;
			}

			*p++ = palette[color];
		}
		mem++;
	}

	const u8 x_width = BIT(m_display_reg, 6) ? 80 : 40;
	const u8 x_step = BIT(m_display_reg, 6) ? 1 : 2;
	mem = 0xc000 + ma;
	p = &bitmap.pix(y);

	for(u16 x = 0; x < x_width; x++)
	{
		const u8 chr = m_p_vram[mem];
		const u8 attr = m_p_vram[mem | 0x800];
		const u8 color = (BIT(m_display_reg, 7)) ? 7 : (attr & 0x07);

		u8 gfx = BIT(attr, 5);

		if (cursor_x >= 0)
			gfx ^= (x == (cursor_x / x_step));

		if (gfx)
			gfx = 0xff;

		if (ra < 8)
			gfx ^= m_p_chargen[(chr << 3) | ra];

		for(u8 i = 0; i < 8; i++)
		{
			u8 pen = BIT(gfx, 7-i) ? color : 0;

			if (x_step == 1)
			{
				if (pen)
					*p = palette[color];
				p++;
			}
			else
			{
				if (pen)
					*p = palette[color];
				p++;
				if (pen)
					*p = palette[color];
				p++;
			}
		}
		mem += x_step;
	}
}

uint8_t multi8_state::key_input_r()
{
	if (m_mcu_init == 0)
	{
		m_mcu_init++;
		return 3;
	}

	m_keyb_press_flag &= 0xfe;

	return m_keyb_press;
}

uint8_t multi8_state::key_status_r()
{
	if (m_mcu_init == 0)
		return 1;
	else
	if (m_mcu_init == 1)
	{
		m_mcu_init++;
		return 1;
	}
	else
	if (m_mcu_init == 2)
	{
		m_mcu_init++;
		return 0;
	}

	return m_keyb_press_flag | (m_shift_press_flag << 7);
}

uint8_t multi8_state::vram_r(offs_t offset)
{
	uint8_t res;

	if (!BIT(m_vram_bank, 4)) //select plain work ram
		return m_p_wram[offset];

	res = 0xff;

	if (!BIT(m_vram_bank, 0))
		res &= m_p_vram[offset | 0x0000];

	if (!BIT(m_vram_bank, 1))
		res &= m_p_vram[offset | 0x4000];

	if (!BIT(m_vram_bank, 2))
		res &= m_p_vram[offset | 0x8000];

	if (!BIT(m_vram_bank, 3))
		res &= m_p_vram[offset | 0xc000];

	return res;
}

void multi8_state::vram_w(offs_t offset, uint8_t data)
{
	if (!BIT(m_vram_bank, 4)) //select plain work ram
	{
		m_p_wram[offset] = data;
		return;
	}

	if (!BIT(m_vram_bank, 0))
		m_p_vram[offset | 0x0000] = data;

	if (!BIT(m_vram_bank, 1))
		m_p_vram[offset | 0x4000] = data;

	if (!BIT(m_vram_bank, 2))
		m_p_vram[offset | 0x8000] = data;

	if (!BIT(m_vram_bank, 3))
		m_p_vram[offset | 0xc000] = data;
}

uint8_t multi8_state::pal_r(offs_t offset)
{
	return m_pen_clut[offset];
}

void multi8_state::pal_w(offs_t offset, uint8_t data)
{
	m_pen_clut[offset] = data;

	uint8_t i;
	for(i=0;i<8;i++)
	{
		if (m_pen_clut[i])
		{
			m_bw_mode = 0;
			return;
		}
		m_bw_mode = 1;
	}
}

uint8_t multi8_state::ay8912_0_r(){ return m_aysnd->data_r(); }
uint8_t multi8_state::ay8912_1_r(){ return m_aysnd->data_r(); }

uint8_t multi8_state::kanji_r(offs_t offset)
{
	return m_p_kanji[(m_knj_addr << 1) | (offset & 1)];
}

void multi8_state::kanji_w(offs_t offset, uint8_t data)
{
	m_knj_addr = (offset == 0) ? (m_knj_addr & 0xff00) | (data & 0xff) : (m_knj_addr & 0x00ff) | (data << 8);
}

void multi8_state::kansas_w(int state)
{
	// incoming @19200Hz
	u8 twobit = m_cass_data[3] & 3;

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassbit;

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 2) ? -1.0 : +1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 3) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}

	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( multi8_state::kansas_r )
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}


void multi8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).rw(FUNC(multi8_state::vram_r), FUNC(multi8_state::vram_w));
	map(0xc000, 0xffff).ram();
}

void multi8_state::io_map(address_map &map)
{
//  map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(multi8_state::key_input_r)); //keyboard
	map(0x01, 0x01).r(FUNC(multi8_state::key_status_r)); //keyboard
	map(0x18, 0x19).w(m_aysnd, FUNC(ay8910_device::address_data_w));
	map(0x18, 0x18).r(FUNC(multi8_state::ay8912_0_r));
	map(0x1a, 0x1a).r(FUNC(multi8_state::ay8912_1_r));
	map(0x1c, 0x1c).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x1d, 0x1d).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x20, 0x21).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write)); //cmt
	map(0x24, 0x27).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)); //pit
	map(0x28, 0x2b).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2c, 0x2d).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write)); //i8259
	map(0x30, 0x37).rw(FUNC(multi8_state::pal_r), FUNC(multi8_state::pal_w));
	map(0x40, 0x41).rw(FUNC(multi8_state::kanji_r), FUNC(multi8_state::kanji_w)); //kanji regs
//  map(0x70, 0x74) //upd765a fdc
//  map(0x78, 0x78) //memory banking
}

/* Input ports */
static INPUT_PORTS_START( multi8 )
	PORT_START("VBLANK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x01 soh
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x02 stx
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x03 etx
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x04 etx
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x05 eot
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x06 enq
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x07 ack
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0b lf
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0c vt
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0e cr
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0f so

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x10 si
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x11 dle
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x12 dc1
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x13 dc2
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x15 dc4
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x17 syn
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x18 etb
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1a em
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(27)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1c ???
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1d fs
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1e gs
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1f us

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x21 !
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x22 "
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x23 #
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x24 $
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x25 %
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x26 &
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x27 '
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED) //0x28 (
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED) //0x29 )
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2a *
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2c ,
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2e .
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2f /

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(multi8_state::keyboard_callback)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i,scancode;
	uint8_t keymod = ioport("key_modifiers")->read() & 0x1f;
	scancode = 0;

	m_shift_press_flag = ((keymod & 0x02) >> 1);

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((ioport(portnames[port_i])->read()>>i) & 1)
			{
				//key_flag = 1;
				if(!m_shift_press_flag)  // shift not pressed
				{
					if(scancode >= 0x41 && scancode < 0x5b)
						scancode += 0x20;  // lowercase
				}
				else
				{
					if(scancode >= 0x31 && scancode < 0x3a)
						scancode -= 0x10;
					if(scancode == 0x30)
						scancode = 0x3d;

					if(scancode == 0x3b)
						scancode = 0x2c;

					if(scancode == 0x3a)
						scancode = 0x2e;
					if(scancode == 0x5b)
						scancode = 0x2b;
					if(scancode == 0x3c)
						scancode = 0x3e;
				}
				m_keyb_press = scancode;
				m_keyb_press_flag = 1;
				return;
			}
			scancode++;
		}
	}
}

static const gfx_layout multi8_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout multi8_kanjilayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_multi8 )
	GFXDECODE_ENTRY( "chargen", 0x0000, multi8_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x0000, multi8_kanjilayout, 0, 1 )
GFXDECODE_END


uint8_t multi8_state::porta_r()
{
	int vsync = (ioport("VBLANK")->read() & 0x1) << 5;
	/*
	-x-- ---- kanji rom is present (0) yes
	--x- ---- vsync
	---- --x- fdc rom is present (0) yes
	*/

	return 0x9f | vsync | 0x00;
}


void multi8_state::portb_w(uint8_t data)
{
	/*
	    x--- ---- color mode
	    -x-- ---- screen width (80 / 40)
	    ---- x--- memory bank status
	    ---- -xxx page screen graphics in B/W mode
	*/

	m_display_reg = data;
}

void multi8_state::portc_w(uint8_t data)
{
//  printf("Port C w = %02x\n",data);
	m_vram_bank = data & 0x1f;

	if (data & 0x20 && data != 0xff)
		printf("Work RAM bank selected!\n");
//      fatalerror("Work RAM bank selected\n");
}


void multi8_state::ym2203_porta_w(uint8_t data)
{
	m_beeper->set_state(BIT(data, 3));
}

void multi8_state::machine_start()
{
	m_p_vram = memregion("vram")->base();
	m_p_wram = memregion("wram")->base();
	m_p_kanji = memregion("kanji")->base();
}

void multi8_state::machine_reset()
{
	m_beeper->set_state(0);
	m_mcu_init = 0;
	m_uart->write_cts(0);
}

void multi8_state::multi8(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &multi8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &multi8_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_multi8);

	/* Audio */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_aysnd, 1500000); //unknown clock / divider
	m_aysnd->port_a_write_callback().set(FUNC(multi8_state::ym2203_porta_w));
	m_aysnd->add_route(ALL_OUTPUTS, "mono", 0.50);
	BEEP(config, m_beeper, 1200).add_route(ALL_OUTPUTS,"mono",0.50); // guesswork

	/* devices */
	TIMER(config, "keyboard_timer").configure_periodic(FUNC(multi8_state::keyboard_callback), attotime::from_hz(240/32));

	MC6845(config, m_crtc, XTAL(3'579'545)/2);    /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(multi8_state::crtc_update_row));

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(multi8_state::porta_r));
	m_ppi->out_pb_callback().set(FUNC(multi8_state::portb_w));
	m_ppi->out_pc_callback().set(FUNC(multi8_state::portc_w));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 19200));
	uart_clock.signal_handler().set(FUNC(multi8_state::kansas_w));
	TIMER(config, "kansas_r").configure_periodic(FUNC(multi8_state::kansas_r), attotime::from_hz(40000));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	I8251(config, m_uart, 0); // for cassette
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });

	PIT8253(config, "pit", 0);
	PIC8259(config, "pic", 0);

	//UPD765A(config, "fdc", false, true);
	//FLOPPY_CONNECTOR(config, "fdc:0", multi8_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);
}

/* ROM definition */
ROM_START( multi8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(2131a3a8) SHA1(0f5a565ecfbf79237badbf9011dcb374abe74a57))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom",  0x0000, 0x0800, BAD_DUMP CRC(08f9ed0e) SHA1(57480510fb30af1372df5a44b23066ca61c6f0d9))

	ROM_REGION( 0x20000, "kanji", 0 )
	ROM_LOAD( "kanji.rom",  0x0000, 0x20000, BAD_DUMP CRC(c3cb3ff9) SHA1(7173cc5053a281d73cfecfacd31442e21ee7474a))

	ROM_REGION( 0x0100, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "kbd.rom",  0x0000, 0x0100, NO_DUMP )

	ROM_REGION( 0x1000, "fdc_bios", 0 )
	ROM_LOAD( "disk.rom",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "wram", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY       FULLNAME                FLAGS
COMP( 1983, multi8, 0,      0,      multi8,  multi8, multi8_state, empty_init, "Mitsubishi", "Multi 8 (Mitsubishi)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
