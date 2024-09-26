// license:BSD-3-Clause
// copyright-holders:Angelo Salese
// thanks-to: Takeda Toshiya
/**************************************************************************************************

FP-200 (c) 1982 Casio

TODO:
- cassette i/f, glued together by discrete;
- serial i/f;
- FDC (requires test program that Service manual mentions);
- ROM/RAM slot interface;
- mini plotter printer (FP-1011PL)
- graphic printer (FP-1012PR)

Notes:
- on start-up there's a "memory illegal" warning. Issue a "RESET" command to initialize it.

**************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/nvram.h"
#include "machine/rp5c01.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

static constexpr XTAL MAIN_CLOCK = 6.144_MHz_XTAL;

class fp200_state : public driver_device
{
public:
	fp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_nvram(*this, "nvram")
		, m_ioview(*this, "ioview")
		, m_key(*this, "KEY%X", 0U)
		, m_gfxrom(*this, "chargen")
	{ }

	void fp200(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(keyb_irq);

private:
	// devices
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	required_device<nvram_device> m_nvram;
	memory_view m_ioview;
	required_ioport_array<16> m_key;
	required_memory_region m_gfxrom;

	//uint8_t *m_chargen = nullptr;
	uint8_t m_keyb_matrix = 0;

	std::unique_ptr<uint8_t[]> m_lcd_vram[2];
	u16 m_lcd_yoffset[2]{};
	u16 m_lcd_address = 0;
	u8 m_lcd_status = 0;
	bool m_lcd_text_mode = false;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <unsigned N> u8 lcd_data_r(offs_t offset);
	template <unsigned N> void lcd_data_w(offs_t offset, u8 data);

	void lcd_map(address_map &map) ATTR_COLD;

	uint8_t keyb_r(offs_t offset);
	void keyb_w(offs_t offset, uint8_t data);

	void sod_w(int state);
	int sid_r();

	void palette_init(palette_device &palette) const;
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
};

void fp200_state::video_start()
{
	m_lcd_vram[0] = make_unique_clear<uint8_t[]>(0x400);
	m_lcd_vram[1] = make_unique_clear<uint8_t[]>(0x400);

	save_pointer(NAME(m_lcd_vram[0]), 0x400);
	save_pointer(NAME(m_lcd_vram[1]), 0x400);
}

uint32_t fp200_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for(int y = cliprect.min_x; y <= cliprect.max_y; y ++)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x ++)
		{
			const u8 which = x < 80;
			const u8 x_tile = x >> 3;
			const u8 y_tile = y >> 3;
			const u16 base_addr = ((x_tile % 10 + y_tile * 0x80) + m_lcd_yoffset[which]) & 0x3ff;
			const u8 xi = x & 7;
			const u8 yi = y & 7;

			const u8 vram = m_lcd_vram[which][(base_addr + yi * 0x10) & 0x3ff];
			uint8_t const pix = BIT(vram, xi);
			bitmap.pix(y, x) = pix;
		}
	}

	return 0;
}

template <unsigned N> u8 fp200_state::lcd_data_r(offs_t offset)
{
	return m_lcd_vram[N][m_lcd_address & 0x3ff];
}

template <unsigned N> void fp200_state::lcd_data_w(offs_t offset, u8 data)
{
	switch (m_lcd_status)
	{
		// select mode
		case 0xb:
			if (BIT(data, 6))
			{
				if (BIT(data, 4))
					m_lcd_yoffset[N] = m_lcd_address;
				else
					m_lcd_text_mode = bool(BIT(data, 5));
			}

			if (data & 0x8f)
				logerror("Warning: LCD write with unknown mode %02x\n", data);

			break;

		// data write
		case 0x1:
			if (m_lcd_text_mode)
			{
				// The handling doesn't make a whole lot of sense for being practically usable ...
				const u8 tile_address = bitswap<8>(data, 3, 2, 1, 0, 7, 6, 5, 4);
				for (int yi = 0; yi < 8; yi ++)
				{
					u8 tile = 0;
					for (int xi = 0; xi < 8; xi ++)
						tile |= BIT(m_gfxrom->base()[tile_address * 8 + xi], 7 - yi) << xi;
					m_lcd_vram[N][(m_lcd_address + 0x10 * yi) & 0x3ff] = tile;
				}
			}
			else
				m_lcd_vram[N][m_lcd_address & 0x3ff] = data;

			break;
		default:
			logerror("Warning: LCD write with unknown status type %02x\n", m_lcd_status);
			break;
	}
}

/*
 * video section is 2x MSM6216-01GS-1K + 1x MSM6215-01GS-K glued together by the gate array
 *
 * [1] DDDD DDDD vram data/mode select (right half)
 * [2] DDDD DDDD vram data/mode select (left half)
 * [8] SSSS ---- Status code (1=vram type/0xb=attr type)
 *     ---- --YY upper part of Y address
 * [9] YYYY XXXX lower part of Y address / X address
 */
void fp200_state::lcd_map(address_map &map)
{
	map(0x1, 0x1).rw(FUNC(fp200_state::lcd_data_r<1>), FUNC(fp200_state::lcd_data_w<1>));
	map(0x2, 0x2).rw(FUNC(fp200_state::lcd_data_r<0>), FUNC(fp200_state::lcd_data_w<0>));
	map(0x8, 0x8).lrw8(
		NAME([this] (offs_t offset) {
			u8 res =  (m_lcd_status & 0xf) << 4 | (m_lcd_address & 0x300) >> 4;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lcd_status = (data & 0xf0) >> 4;
			if (m_lcd_status == 0xb)
			{
				m_lcd_address &= 0xff;
				m_lcd_address |= (data & 0x3) << 8;
			}
		})
	);
	map(0x9, 0x9).lrw8(
		NAME([this] (offs_t offset) {
			u8 res =  m_lcd_address & 0xff;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lcd_address &= 0x300;
			m_lcd_address |= data;
		})
	);
}

uint8_t fp200_state::keyb_r(offs_t offset)
{
	const uint8_t res = m_key[m_keyb_matrix & 0xf]->read();
	return res;
}

void fp200_state::keyb_w(offs_t offset, uint8_t data)
{
	m_keyb_matrix = data & 0xf;
}

void fp200_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // basic & cetl
	// TODO: 1 waitstate penalty for accessing any RAM
	map(0x8000, 0x9fff).ram().share("nvram"); // internal RAM
//  0xa000, 0xffff exp RAM (FP-201RAM)
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xffff).ram(); // or exp ROM (FP-206ROM)
}

void fp200_state::main_io(address_map &map)
{
	map(0x00, 0xff).view(m_ioview);
	m_ioview[0](0x10, 0x1f).rw("rtc", FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
//  m_ioview[0](0x20, 0x2f) AUTO-POWER OFF
//  m_ioview[0](0x40, 0x4f) FDC Device ID Code (5 for "FP-1021FD")
//  m_ioview[0](0x80, 0xff) FDD (unknown type)
	m_ioview[1](0x00, 0x0f).m(*this, FUNC(fp200_state::lcd_map));
//  m_ioview[1](0x10, 0x10) I/O control (w/o), D1 selects CMT or RS-232C
//  m_ioview[1](0x11, 0x11) I/O control (w/o), uPD65010G gate array control
	// TODO: writes are undocumented, just before reads PC=35C (strobe for update?)
	m_ioview[1](0x20, 0x20).r(FUNC(fp200_state::keyb_r)).nopw();
	m_ioview[1](0x21, 0x21).w(FUNC(fp200_state::keyb_w));
//  m_ioview[1](0x40, 0x4f) CMT & RS-232C control
//  m_ioview[1](0x80, 0x8f) [Centronics] printer
}

INPUT_CHANGED_MEMBER(fp200_state::keyb_irq)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, (newval) ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( fp200 )
	PORT_START("KEY0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 / '") PORT_CODE(KEYCODE_7) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 / (") PORT_CODE(KEYCODE_8) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 / )") PORT_CODE(KEYCODE_9) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; / +") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY4")
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) //PORT_TOGGLE PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS / DEL") PORT_CODE(KEYCODE_INSERT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME / CLS") PORT_CODE(KEYCODE_HOME) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF0 / PF5") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 / !") PORT_CODE(KEYCODE_1) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP / CONT") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\ / |") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1 / PF6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 / \"") PORT_CODE(KEYCODE_2) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("^ / ~") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2 / PF7") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 / #") PORT_CODE(KEYCODE_3) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@ / '") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- / =") PORT_CODE(KEYCODE_MINUS) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3 / PF8") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 / $") PORT_CODE(KEYCODE_4) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": / *") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4 / PF9") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 / %") PORT_CODE(KEYCODE_5) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("] / }") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 / &") PORT_CODE(KEYCODE_6) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, fp200_state,keyb_irq, 0)

	PORT_START("KEYA")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("KEYB")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYC")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYD")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYF")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYMOD")
	PORT_BIT( 0x01f, IP_ACTIVE_LOW, IPT_UNUSED )
	// positional switch on keyboard
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Basic / CETL Mode") PORT_TOGGLE
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 1) },
	8*8
};

static GFXDECODE_START( gfx_fp200 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout,     0, 1 )
GFXDECODE_END


void fp200_state::machine_start()
{
}

void fp200_state::machine_reset()
{
}


void fp200_state::palette_init(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa0, 0xa8, 0xa0);
	palette.set_pen_color(1, 0x30, 0x38, 0x10);
}

void fp200_state::sod_w(int state)
{
	m_ioview.select(state);
}

int fp200_state::sid_r()
{
	return (ioport("KEYMOD")->read() >> m_keyb_matrix) & 1;
}

void fp200_state::fp200(machine_config &config)
{
	I8085A(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp200_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &fp200_state::main_io);
	m_maincpu->in_sid_func().set(FUNC(fp200_state::sid_r));
	m_maincpu->out_sod_func().set(FUNC(fp200_state::sod_w));

	RP5C01(config, "rtc", XTAL(32'768));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(fp200_state::screen_update));
	screen.set_size(20*8, 8*8);
	screen.set_visarea(0*8, 20*8-1, 0*8, 8*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_fp200);

	PALETTE(config, "palette", FUNC(fp200_state::palette_init), 2);

	// No sound HW
}


/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( fp200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fp200rom.bin", 0x0000, 0x8000, CRC(dba6e41b) SHA1(c694fa19172eb56585a9503997655bcf9d369c34) )

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD( "chr.bin", 0x0000, 0x800, CRC(2e6501a5) SHA1(6186e25feabe6db851ee7d61dad11e182a6d3a4a) )
ROM_END

} // anonymous namespace


COMP( 1982, fp200, 0, 0, fp200, fp200, fp200_state, empty_init, "Casio", "FP-200 (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
