// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADDS 4000/260

    ASCII/ANSI terminal

    Hardware:
    - P-80C32-16
    - KM622560LP-7L (32k RAM)
    - CY6225LL-70 (32k RAM)
    - LSI Victor 006-9802760 REV B
    - KM622560LP-7L x2 (32k RAM x2)
    - 16 MHz XTAL, 44.976 MHz XTAL

    TODO:
    - RS232 ports
    - More work on the ASIC

    Notes:
    - Press CTRL-SCRLOCK to enter setup mode
    - Other models in this line: 4000/260C, 4000/260LF, 4000/260LFC
    - Sold under the ADDS brand, but ADDS was part of SunRiver Data Systems
      by then, which became Boundless Technologies.

***************************************************************************/

#include "emu.h"

#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/timer.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_KEYBOARD (1U << 1)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class _4000_260_state : public driver_device
{
public:
	_4000_260_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_beeper(*this, "beeper"),
		m_keyboard(*this, "keyboard"),
		m_rombank(*this, "rombank"),
		m_rombase(*this, "rombase"),
		m_ramview(*this, "ramview")
	{ }

	void _4000_260(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum : uint8_t
	{
		INT0_UNK1 = 0x80,
		INT0_UNK2 = 0x40,
		INT0_UNK3 = 0x20,
		INT0_UNK4 = 0x10,
	};

	enum : uint8_t
	{
		INT1_ROW = 0x80,
		INT1_VBLANK = 0x40,
		INT1_KEYBOARD = 0x20,
		INT1_UNK4 = 0x10,
	};

	enum : uint8_t
	{
		STATUS_VBLANK = 0x20,
		STATUS_KEYBOARD_TRANSMIT = 0x04,
		STATUS_KEYBOARD_UNK2 = 0x02, // checked in the keyboard read function, ack related?
		STATUS_KEYBOARD_UNK1 = 0x01, // checked in the keyboard send function before reading 0x13
	};

	void mem_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	void keyboard_clock_w(int state);
	void keyboard_data_w(int state);
	TIMER_CALLBACK_MEMBER(keyboard_clock_release);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void vblank_w(int state);

	// asic registers
	void line_unk0_w(uint8_t data);
	void line_unk1_w(uint8_t data);
	void line_unk2_w(uint8_t data);
	void line_addr_msb_w(uint8_t data);
	void line_addr_lsb_w(uint8_t data);
	uint8_t cursor_addr_msb_r();
	uint8_t cursor_addr_lsb_r();
	void cursor_addr_msb_w(uint8_t data);
	void cursor_addr_lsb_w(uint8_t data);
	void unk_07_w(uint8_t data);
	uint8_t status_r();
	void status_w(uint8_t data);
	void unk_09_w(uint8_t data);
	void ram_char_w(uint8_t data);
	void ram_attr_w(uint8_t data);
	void ram_write_addr_msb_w(uint8_t data);
	void ram_write_addr_lsb_w(uint8_t data);
	uint8_t keyboard_r();
	void keyboard_w(uint8_t data);
	uint8_t int1_status_r();
	void ram_read_addr_msb_w(uint8_t data);
	void ram_read_addr_lsb_w(uint8_t data);
	// uint8_t unk_16_r();
	uint8_t ram_char_r();
	uint8_t ram_attr_r();
	void chargen_addr_w(uint8_t data);
	void unk_1c_w(uint8_t data);
	void unk_1f_w(uint8_t data);
	void unk_addr_msb_w(uint8_t data);
	void unk_addr_lsb_w(uint8_t data);
	uint8_t int0_status_r();
	void unk_23_w(uint8_t data);
	void unk_24_w(uint8_t data);
	void unk_25_w(uint8_t data);
	uint8_t unk_26_r();
	void rombank_w(uint8_t data);
	void unk_2b_w(uint8_t data);
	void unk_2c_w(uint8_t data);
	void unk_2d_w(uint8_t data);
	void unk_2e_w(uint8_t data);
	void unk_2f_w(uint8_t data);

	uint8_t chargen_r(offs_t offset);
	void chargen_w(offs_t offset, uint8_t data);

	uint8_t cpu_p3_r();
	void cpu_p3_w(uint8_t data);

	required_device<i80c32_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<beep_device> m_beeper;
	required_device<pc_kbdc_device> m_keyboard;
	required_memory_bank m_rombank;
	memory_view m_rombase;
	memory_view m_ramview;
	memory_passthrough_handler m_asic_tap;

	bool m_keyboard_clock = false;
	bool m_keyboard_data = false;
	uint8_t m_keyboard_shifter = 0;
	uint8_t m_keyboard_bits = 0;
	uint8_t m_keyboard_data_latch_in = 0;
	uint8_t m_keyboard_data_latch_out = 0;
	uint8_t m_keyboard_parity = 0;
	emu_timer *m_keyboard_timer = nullptr;
	bool m_keyboard_waiting = false;

	uint8_t m_int0_status = 0;
	uint8_t m_int1_status = 0;

	uint8_t m_char_latch = 0;
	uint8_t m_attr_latch = 0;
	uint16_t m_write_addr = 0;
	uint16_t m_read_addr = 0;
	uint16_t m_chargen_addr = 0;

	struct lineinfo
	{
		uint8_t unk0;
		uint8_t unk1;
		uint8_t unk2;
		uint16_t addr;
	};

	lineinfo m_lines[50];
	uint8_t m_line_index = 0;

	uint16_t m_cursor_addr = 0;
	uint8_t m_unk_07 = 0;
	uint8_t m_status = 0;
	uint8_t m_unk_09 = 0;
	uint8_t m_unk_1c = 0;
	uint8_t m_unk_1f = 0;
	uint16_t m_unk_addr = 0;
	uint8_t m_unk_23 = 0;
	uint8_t m_unk_24 = 0;
	uint8_t m_unk_25 = 0;
	uint8_t m_unk_2b = 0;
	uint8_t m_unk_2c = 0;
	uint8_t m_unk_2d = 0;
	uint8_t m_unk_2e = 0;
	uint8_t m_unk_2f = 0;

	uint8_t m_asic_regs[0x100];

	std::unique_ptr<uint8_t[]> m_chargen;
	std::unique_ptr<uint8_t[]> m_charram;
	std::unique_ptr<uint8_t[]> m_attrram;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void _4000_260_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_rombase);
	m_rombase[0](0x0000, 0x7fff).rom().region("maincpu", 0x00000);
	m_rombase[0](0x8000, 0xffff).bankr(m_rombank);
	m_rombase[1](0x0000, 0x7fff).rom().region("maincpu", 0x20000);
	m_rombase[1](0x8000, 0xffff).bankr(m_rombank);
}

void _4000_260_state::ext_map(address_map &map)
{
	map(0x0000, 0x00ff).view(m_ramview);
	m_ramview[0](0x0000, 0x0000).w(FUNC(_4000_260_state::line_unk0_w)); // attributes?
	m_ramview[0](0x0001, 0x0001).w(FUNC(_4000_260_state::line_unk1_w));
	m_ramview[0](0x0002, 0x0002).w(FUNC(_4000_260_state::line_unk2_w));
	m_ramview[0](0x0003, 0x0003).w(FUNC(_4000_260_state::line_addr_msb_w));
	m_ramview[0](0x0004, 0x0004).w(FUNC(_4000_260_state::line_addr_lsb_w));
	m_ramview[0](0x0005, 0x0005).rw(FUNC(_4000_260_state::cursor_addr_msb_r), FUNC(_4000_260_state::cursor_addr_msb_w));
	m_ramview[0](0x0006, 0x0006).rw(FUNC(_4000_260_state::cursor_addr_lsb_r), FUNC(_4000_260_state::cursor_addr_lsb_w));
	m_ramview[0](0x0007, 0x0007).w(FUNC(_4000_260_state::unk_07_w));
	m_ramview[0](0x0008, 0x0008).rw(FUNC(_4000_260_state::status_r), FUNC(_4000_260_state::status_w));
	m_ramview[0](0x0009, 0x0009).w(FUNC(_4000_260_state::unk_09_w));
	m_ramview[0](0x000a, 0x000a).w(FUNC(_4000_260_state::ram_char_w));
	m_ramview[0](0x000b, 0x000b).w(FUNC(_4000_260_state::ram_attr_w));
	m_ramview[0](0x000d, 0x000d).w(FUNC(_4000_260_state::ram_write_addr_msb_w));
	m_ramview[0](0x000e, 0x000e).w(FUNC(_4000_260_state::ram_write_addr_lsb_w));
	m_ramview[0](0x0010, 0x0010).rw(FUNC(_4000_260_state::keyboard_r), FUNC(_4000_260_state::keyboard_w));
	m_ramview[0](0x0013, 0x0013).r(FUNC(_4000_260_state::int1_status_r));
	m_ramview[0](0x0014, 0x0014).w(FUNC(_4000_260_state::ram_read_addr_msb_w));
	m_ramview[0](0x0015, 0x0015).w(FUNC(_4000_260_state::ram_read_addr_lsb_w));
	m_ramview[0](0x0016, 0x0016).portr("16"); // r(FUNC(_4000_260_state::unk_16_r));
	m_ramview[0](0x0017, 0x0017).r(FUNC(_4000_260_state::ram_char_r));
	m_ramview[0](0x0018, 0x0018).r(FUNC(_4000_260_state::ram_attr_r));
	m_ramview[0](0x001a, 0x001a).w(FUNC(_4000_260_state::chargen_addr_w));
	m_ramview[0](0x001c, 0x001c).w(FUNC(_4000_260_state::unk_1c_w));
	m_ramview[0](0x001f, 0x001f).w(FUNC(_4000_260_state::unk_1f_w));
	m_ramview[0](0x0020, 0x0020).w(FUNC(_4000_260_state::unk_addr_msb_w));
	m_ramview[0](0x0021, 0x0021).w(FUNC(_4000_260_state::unk_addr_lsb_w));
	m_ramview[0](0x0022, 0x0022).r(FUNC(_4000_260_state::int0_status_r));
	m_ramview[0](0x0023, 0x0023).w(FUNC(_4000_260_state::unk_23_w));
	m_ramview[0](0x0024, 0x0024).w(FUNC(_4000_260_state::unk_24_w));
	m_ramview[0](0x0025, 0x0025).w(FUNC(_4000_260_state::unk_25_w));
	m_ramview[0](0x0026, 0x0026).r(FUNC(_4000_260_state::unk_26_r));
	m_ramview[0](0x0026, 0x0026).w(FUNC(_4000_260_state::rombank_w));
	m_ramview[0](0x002b, 0x002b).w(FUNC(_4000_260_state::unk_2b_w));
	m_ramview[0](0x002c, 0x002c).w(FUNC(_4000_260_state::unk_2c_w));
	m_ramview[0](0x002d, 0x002d).w(FUNC(_4000_260_state::unk_2d_w));
	m_ramview[0](0x002e, 0x002e).w(FUNC(_4000_260_state::unk_2e_w));
	m_ramview[0](0x002f, 0x002f).w(FUNC(_4000_260_state::unk_2f_w));
	m_ramview[1](0x0000, 0x00ff).rw(FUNC(_4000_260_state::chargen_r), FUNC(_4000_260_state::chargen_w));
	map(0x8000, 0xffff).ram();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( _4000_260 )
	PORT_START("16")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "16:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "16:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "16:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "16:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "16:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "16:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "16:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "16:8")
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD INTERFACE
//**************************************************************************

void _4000_260_state::keyboard_clock_w(int state)
{
	if (m_keyboard_clock && state == 0 && !m_keyboard_waiting)
	{
		// send to keyboard
		if (m_status & STATUS_KEYBOARD_TRANSMIT)
		{
			if (m_keyboard_bits < 8)
			{
				int bit = BIT(m_keyboard_shifter, 0);

				m_keyboard_parity ^= bit;

				m_keyboard->data_write_from_mb(bit);
				m_keyboard_shifter >>= 1;
				m_keyboard_bits++;
			}
			else if (m_keyboard_bits == 8)
			{
				m_keyboard->data_write_from_mb(m_keyboard_parity ^ 1);
				m_keyboard_bits++;
			}
			else if (m_keyboard_bits == 9)
			{
				m_keyboard->data_write_from_mb(1);
				m_keyboard_bits++;
			}
			else
			{
				// transfer done, keyboard data line should be low now
				m_status &= ~STATUS_KEYBOARD_TRANSMIT;
				m_keyboard_bits = 0;
				m_keyboard_parity = 0;
			}
		}
		else
		{
			// receive from keyboard
			if (m_keyboard_bits == 0)
			{
				// start bit
				if (m_keyboard_data)
					logerror("keyboard framing error\n");

				m_keyboard_bits++;
			}
			else if (m_keyboard_bits >= 1 && m_keyboard_bits <= 8)
			{
				// data bits
				m_keyboard_shifter >>= 1;

				if (m_keyboard_data)
					m_keyboard_shifter |= 0x80;

				m_keyboard_bits++;
			}
			else if (m_keyboard_bits == 9)
			{
				// parity bit
				m_keyboard_bits++;
			}
			else if (m_keyboard_bits == 10)
			{
				// stop bit
				if (!m_keyboard_data)
					logerror("keyboard framing error\n");

				// transfer done, move to latch and signal interrupt
				m_keyboard_bits = 0;
				m_keyboard_data_latch_in = m_keyboard_shifter;
				m_int1_status |= INT1_KEYBOARD;
				m_maincpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);

				// tell keyboard to stop sending data, will be raised once the data has been read
				m_keyboard->clock_write_from_mb(0);
			}
		}
	}

	m_keyboard_clock = bool(state);
}

void _4000_260_state::keyboard_data_w(int state)
{
	m_keyboard_data = bool(state);
}

TIMER_CALLBACK_MEMBER( _4000_260_state::keyboard_clock_release )
{
	m_keyboard_waiting = false;

	m_keyboard->data_write_from_mb(0);
	m_keyboard->clock_write_from_mb(1);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t _4000_260_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *const pen = m_palette->pens();

	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int y = 0; y < 26; y++)
	{
		uint8_t line_attr = 0;

		// m_lines[y].unk0 bit 0 reverse video?
		// m_lines[y].unk0 bit 1 conceal line?

		if (BIT(m_lines[y].unk0, 1))
			continue;

		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_charram[m_lines[y].addr + x];
			uint8_t attr = m_attrram[m_lines[y].addr + x];

			uint16_t chargen_addr = 0x1000 | (code << 4);

			if (BIT(code, 7) == 1)
			{
				line_attr = code;
				continue;
			}

			// second half of chargen (likely only one of those bits)
			if (BIT(attr, 7) == 1 && BIT(attr, 6) == 1)
				chargen_addr |= 0x800;

			// highlight
			pen_t fg = (BIT(line_attr, 0) == 1) ? pen[2] : pen[1];
			pen_t bg = pen[0];

			// reverse
			if (BIT(attr | line_attr, 3) == 1)
				std::swap(fg, bg);

			// draw 15 lines
			for (int i = 0; i < 15; i++)
			{
				uint8_t data = m_chargen[chargen_addr + i];

				// draw cursor (TODO: cursor shape, cursor on/off)
				if (m_cursor_addr == (m_lines[y].addr + x))
					data = 0xff;

				// first pixel (extended if bit 7 or bit 6 is set)
				bitmap.pix(y * 15 + i, x * 9 + 0) = BIT(attr, 7) && BIT(data, 7) ? fg : bg;

				// 8 pixels of the character
				for (int p = 0; p < 8; p++)
					bitmap.pix(y * 15 + i, x * 9 + p + 1) = BIT(data, 7 - p) ? fg : bg;

				// last pixel (extended if bit 7 or bit 6 is set)
				bitmap.pix(y * 15 + i, x * 9 + 9) = BIT(attr, 7) && BIT(data, 0) ? fg : bg;
			}
		}
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(_4000_260_state::scanline)
{
	const int scanline = param;

	// TODO: support other sizes
	if ((scanline % 15) == 0)
	{
		m_int1_status |= INT1_ROW;
		m_maincpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	}
}

void _4000_260_state::vblank_w(int state)
{
	if (state)
	{
		m_int1_status |= INT1_VBLANK;
		m_status |= STATUS_VBLANK;
		m_maincpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	}
	else
	{
		m_status &= ~STATUS_VBLANK;
		m_line_index = 0;
	}
}

static const gfx_layout char_layout_8x16 =
{
	8, 16,
	512,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8*16
};

static GFXDECODE_START( chars )
	GFXDECODE_RAM(nullptr, 0, char_layout_8x16, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void _4000_260_state::line_unk0_w(uint8_t data)
{
	m_lines[m_line_index].unk0 = data;
}

void _4000_260_state::line_unk1_w(uint8_t data)
{
	m_lines[m_line_index].unk1 = data;
}

void _4000_260_state::line_unk2_w(uint8_t data)
{
	m_lines[m_line_index].unk2 = data;
}

void _4000_260_state::line_addr_msb_w(uint8_t data)
{
	m_lines[m_line_index].addr &= 0x00ff;
	m_lines[m_line_index].addr |= data << 8;
}

void _4000_260_state::line_addr_lsb_w(uint8_t data)
{
	m_lines[m_line_index].addr &= 0xff00;
	m_lines[m_line_index].addr |= data << 0;

	if (m_line_index < 49)
		m_line_index++;
	else
		logerror("line info overflow\n");
}

uint8_t _4000_260_state::cursor_addr_msb_r()
{
	return m_cursor_addr >> 8;
}

uint8_t _4000_260_state::cursor_addr_lsb_r()
{
	return m_cursor_addr >> 0;
}

void _4000_260_state::cursor_addr_msb_w(uint8_t data)
{
	m_cursor_addr &= 0x00ff;
	m_cursor_addr |= data << 8;
}

void _4000_260_state::cursor_addr_lsb_w(uint8_t data)
{
	m_cursor_addr &= 0xff00;
	m_cursor_addr |= data << 0;
}

void _4000_260_state::unk_07_w(uint8_t data)
{
	// logerror("unk_07_w: %02x\n", data);
	m_unk_07 = data;
}

uint8_t _4000_260_state::status_r()
{
	// 76------  unknown, code waits for them to be 1 to write to the ram registers
	// --5-----  vblank
	// ---4----  unknown
	// ----3---  unknown
	// -----2--  keyboard transmit buffer full
	// ------1-  keyboard, ack related?
	// -------0  keyboard, checked before reading 0x13

	return m_status;
}

void _4000_260_state::status_w(uint8_t data)
{
	// often written
	// logerror("status_w: %02x\n", data);
}

void _4000_260_state::unk_09_w(uint8_t data)
{
	// 7-------  beeper
	// -65-----  ram selection? (10 -> char ram, 01 -> attribute ram)
	// ---43210  unknown

	// logerror("unk_09_w: %02x\n", data);

	m_beeper->set_state(BIT(data, 7));

	m_unk_09 = data;
}

void _4000_260_state::ram_char_w(uint8_t data)
{
	m_char_latch = data;
}

void _4000_260_state::ram_attr_w(uint8_t data)
{
	m_attr_latch = data;
}

void _4000_260_state::ram_write_addr_msb_w(uint8_t data)
{
	m_write_addr &= 0x00ff;
	m_write_addr |= data << 8;
}

void _4000_260_state::ram_write_addr_lsb_w(uint8_t data)
{
	m_write_addr &= 0xff00;
	m_write_addr |= data << 0;

	// on a write here, update ram
	if (m_write_addr < 0x8000)
	{
		m_charram[m_write_addr] = m_char_latch;
		m_attrram[m_write_addr] = m_attr_latch;
	}
}

uint8_t _4000_260_state::keyboard_r()
{
	LOGMASKED(LOG_KEYBOARD, "keyboard_r: %02x\n", m_keyboard_data_latch_in);

	// the keyboard can send more data now
	m_keyboard->clock_write_from_mb(1);

	return m_keyboard_data_latch_in;
}

void _4000_260_state::keyboard_w(uint8_t data)
{
	LOGMASKED(LOG_KEYBOARD, "keyboard_w: %02x\n", data);

	m_status |= STATUS_KEYBOARD_TRANSMIT;
	m_keyboard_data_latch_out = data;

	m_keyboard_shifter = m_keyboard_data_latch_out;

	// lower clock line and set a timer to raise it again
	m_keyboard_waiting = true;
	m_keyboard->clock_write_from_mb(0);
	m_keyboard_timer->adjust(attotime::from_usec(60));
}

uint8_t _4000_260_state::int1_status_r()
{
	// 7-------  row interrupt
	// -6------  vblank interrupt
	// --5-----  keyboard data received
	// ---4----  unknown
	// ----3210  unused?

	uint8_t data = m_int1_status;

	m_maincpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	m_int1_status = 0;

	return data;
}

void _4000_260_state::ram_read_addr_msb_w(uint8_t data)
{
	m_read_addr &= 0x00ff;
	m_read_addr |= data << 8;
}

void _4000_260_state::ram_read_addr_lsb_w(uint8_t data)
{
	m_read_addr &= 0xff00;
	m_read_addr |= data << 0;
}

#if 0
uint8_t _4000_260_state::unk_16_r()
{
	logerror("unk_16_r\n");
	return 0x80;
}
#endif

uint8_t _4000_260_state::ram_char_r()
{
	return m_read_addr < 0x8000 ? m_charram[m_read_addr] : 0xff;
}

uint8_t _4000_260_state::ram_attr_r()
{
	return m_read_addr < 0x8000 ? m_attrram[m_read_addr] : 0xff;
}

void _4000_260_state::chargen_addr_w(uint8_t data)
{
	// 7-------  chargen write enable?
	// -6543210  chargen address high bits

	m_chargen_addr = (data & 0x7f) << 8;

	if (BIT(data, 7))
		m_ramview.select(1); // switch to ram when bit 7 is set?
}

void _4000_260_state::unk_1c_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_1c_w: %02x\n", data);
	m_unk_1c = data;
}

void _4000_260_state::unk_1f_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_1f_w: %02x\n", data);
	m_unk_1f = data;
}

void _4000_260_state::unk_addr_msb_w(uint8_t data)
{
	m_unk_addr &= 0x00ff;
	m_unk_addr |= data << 8;
}

void _4000_260_state::unk_addr_lsb_w(uint8_t data)
{
	m_unk_addr &= 0xff00;
	m_unk_addr |= data << 0;
}

uint8_t _4000_260_state::int0_status_r()
{
	// 7-------  unknown
	// -6------  unknown
	// --5-----  unknown
	// ---4----  unknown
	// ----3210  unused?

	uint8_t data = m_int0_status;

	m_maincpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	m_int0_status = 0;

	return data;
}

void _4000_260_state::unk_23_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_23_w: %02x\n", data);
	m_unk_23 = data;
}

void _4000_260_state::unk_24_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_24_w: %02x\n", data);
	m_unk_24 = data;
}

void _4000_260_state::unk_25_w(uint8_t data)
{
	// written on each frame
	// video mode related?
	// 25 = 4e for 71hz, 46 for 60hz, 42 for 82hz, 4a for 100hz
	// logerror("unk_25_w: %02x\n", data);
	m_unk_25 = data;
}

uint8_t _4000_260_state::unk_26_r()
{
	// read each frame
	// logerror("unk_26_r\n");
	return 0x00;
}

void _4000_260_state::rombank_w(uint8_t data)
{
	// 7-------  rom address line a17
	// 765-----  rom bank select
	// ---4----  always 1?
	// ----3210  always 0?

	if ((data & 0x1f) != 0x10)
		logerror("rombank_w: %02x\n", data);

	m_rombase.select(BIT(data, 7));
	m_rombank->set_entry(data >> 5);
}

void _4000_260_state::unk_2b_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_2b_w: %02x\n", data);
	m_unk_2b = data;
}

void _4000_260_state::unk_2c_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_2c_w: %02x\n", data);
	m_unk_2c = data;
}

void _4000_260_state::unk_2d_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_2d_w: %02x\n", data);
	m_unk_2d = data;
}

void _4000_260_state::unk_2e_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_2e_w: %02x\n", data);
	m_unk_2e = data;
}

void _4000_260_state::unk_2f_w(uint8_t data)
{
	// written on each frame
	// logerror("unk_2f_w: %02x\n", data);
	m_unk_2f = data;
}

uint8_t _4000_260_state::chargen_r(offs_t offset)
{
	offs_t addr = m_chargen_addr | offset;
	uint8_t data = m_chargen[addr];

	// after a read, switch back to the asic?
	m_ramview.select(0);

	return data;
}

void _4000_260_state::chargen_w(offs_t offset, uint8_t data)
{
	offs_t addr = m_chargen_addr | offset;
	m_chargen[addr] = data;

	m_gfxdecode->gfx(0)->mark_dirty(addr / 16);
}

uint8_t _4000_260_state::cpu_p3_r()
{
	return 0xff;
}

void _4000_260_state::cpu_p3_w(uint8_t data)
{
	// logerror("cpu_p3_w: %02x\n", data);
}

void _4000_260_state::machine_start()
{
	// first bank fixed, others banked
	m_rombank->configure_entries(0, 7, memregion("maincpu")->base() + 0x8000, 0x8000);

	// allocate space for ram
	m_chargen = make_unique_clear<uint8_t[]>(0x8000); // 32k for the character generator
	m_charram = make_unique_clear<uint8_t[]>(0x8000); // 32k for the characters
	m_attrram = make_unique_clear<uint8_t[]>(0x8000); // 32k for the attributes

	m_gfxdecode->gfx(0)->set_source(&m_chargen[0]);

	m_keyboard_timer = timer_alloc(FUNC(_4000_260_state::keyboard_clock_release), this);

	// capture asic state for debugging
	m_asic_tap = m_maincpu->space(AS_DATA).install_write_tap
	(
		0x00, 0xff,
		"asic_reg_w",
		[this] (offs_t offset, uint8_t &data, uint8_t mem_mask)
		{
			if (m_ramview.entry() == 0)
				m_asic_regs[offset] = data;
		},
		&m_asic_tap
	);

	// register for save states
	save_item(STRUCT_MEMBER(m_lines, unk0));
	save_item(STRUCT_MEMBER(m_lines, unk1));
	save_item(STRUCT_MEMBER(m_lines, unk2));
	save_item(STRUCT_MEMBER(m_lines, addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_unk_07));
	save_item(NAME(m_status));
	save_item(NAME(m_unk_1c));
	save_item(NAME(m_unk_1f));
	save_item(NAME(m_unk_addr));
	save_item(NAME(m_unk_23));
	save_item(NAME(m_unk_24));
	save_item(NAME(m_unk_25));
	save_item(NAME(m_unk_2b));
	save_item(NAME(m_unk_2c));
	save_item(NAME(m_unk_2d));
	save_item(NAME(m_unk_2e));
	save_item(NAME(m_asic_regs));
	save_item(NAME(m_chargen_addr));
	save_pointer(NAME(m_chargen), 0x8000);
	save_pointer(NAME(m_charram), 0x8000);
	save_pointer(NAME(m_attrram), 0x8000);
}

void _4000_260_state::machine_reset()
{
	m_rombase.select(0);
	m_rombank->set_entry(0);
	m_ramview.select(0);

	m_status = 0xc0;

	memset(m_asic_regs, 0, sizeof(m_asic_regs));
	memset(m_lines, 0, sizeof(m_lines));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void _4000_260_state::_4000_260(machine_config &config)
{
	I80C32(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4000_260_state::mem_map);
	m_maincpu->set_addrmap(AS_DATA, &_4000_260_state::ext_map);
	m_maincpu->port_in_cb<1>().set_constant(0xff); // prevent logspam
	m_maincpu->port_out_cb<1>().set_nop(); // prevent logspam
	m_maincpu->port_out_cb<2>().set_nop(); // prevent logspam
	m_maincpu->port_in_cb<3>().set(FUNC(_4000_260_state::cpu_p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(_4000_260_state::cpu_p3_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(44'976'000, 1056, 0, 800, 600, 0, 390); // no measurements, values guessed
	m_screen->set_screen_update(FUNC(_4000_260_state::screen_update));
	m_screen->screen_vblank().set(FUNC(_4000_260_state::vblank_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(_4000_260_state::scanline), "screen", 0, 1);

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1000).add_route(ALL_OUTPUTS, "mono", 1.0); // unknown frequency

	PC_KBDC(config, m_keyboard, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_keyboard->out_clock_cb().set(FUNC(_4000_260_state::keyboard_clock_w));
	m_keyboard->out_data_cb().set(FUNC(_4000_260_state::keyboard_data_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( 4000_260 )
	ROM_REGION(0x40000, "maincpu", 0)
	// 598-0010669 3.16 SunRiver Data Systems 1995
	ROM_LOAD("4000_260.bin", 0x00000, 0x40000, CRC(b957cd1d) SHA1(1b1185174ba95dca004169e4e1b51b05c8991c43))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME    FLAGS
COMP( 1995, 4000_260, 0,      0,      _4000_260, _4000_260, _4000_260_state, empty_init, "ADDS",  "4000/260", MACHINE_NOT_WORKING )
