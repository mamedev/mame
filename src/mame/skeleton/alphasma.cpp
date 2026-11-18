// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        AlphaSmart Pro

        driver by Sandro Ronco

    TODO:
    - ADB and PS/2
    - charset ROM is wrong
    - asma2k reads from nonexistent internal register at 0x0001
      (probably a bug, since the same code exists in both BIOSes)

****************************************************************************/

#include "emu.h"
#include "fileio.h"
#include "emuopts.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class alphasmart_state : public driver_device
{
public:
	alphasmart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "ks0066_%u", 0U)
		, m_nvram(*this, "nvram")
		, m_nvram_base(*this, "nvram", 0x20000, ENDIANNESS_BIG)
		, m_rambank(*this, "rambank")
		, m_keyboard(*this, "COL.%u", 0)
		, m_battery_status(*this, "BATTERY")
	{
	}

	void alphasmart(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);

protected:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device_array<ks0066_device, 2> m_lcdc;
	required_device<nvram_device> m_nvram;
	memory_share_creator<uint8_t> m_nvram_base;
	required_memory_bank m_rambank;
	required_ioport_array<16> m_keyboard;
	required_ioport m_battery_status;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void alphasmart_palette(palette_device &palette) const;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t kb_r();
	void kb_matrixl_w(uint8_t data);
	void kb_matrixh_w(uint8_t data);
	uint8_t port_a_r();
	virtual void port_a_w(uint8_t data);
	uint8_t port_d_r();
	void port_d_w(uint8_t data);
	void update_lcdc(bool lcdc0, bool lcdc1);

	void alphasmart_mem(address_map &map) ATTR_COLD;

	uint8_t           m_matrix[2];
	uint8_t           m_port_a;
	uint8_t           m_port_d;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;
};

enum class as2k_pc_key : uint8_t
{
	none,
	space, tab, enter, backspace, caps_lock,
	left_shift, right_shift, left_ctrl, right_ctrl, left_alt, right_alt,
	a, b, c, d, e, f, g, h, i, j, k, l, m,
	n, o, p, q, r, s, t, u, v, w, x, y, z,
	num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,
	grave, minus, equals, lbrace, rbrace, backslash, semicolon, quote, comma, period, slash,
	kp_0, kp_1, kp_2, kp_3, kp_4, kp_5, kp_6, kp_7, kp_8, kp_9
};

class asma2k_state : public alphasmart_state
{
public:
	asma2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: alphasmart_state(mconfig, type, tag)
		, m_io_view(*this, "io")
		, m_dictbank(*this, "dictbank")
		, m_pc_connected(*this, "PC_CONNECTED")
	{
	}

	void asma2k(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(pc_connected_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void lcd_ctrl_w(uint8_t data);
	uint8_t asma2k_port_a_r();
	void asma2k_port_d_w(uint8_t data);
	virtual void port_a_w(uint8_t data) override;
	void pc_clock_rising(bool data);
	void pc_set2_byte(uint8_t data);
	void pc_key_event(as2k_pc_key key, bool pressed);
	void pc_keyboard_reset();
	void send_sink_begin();
	void send_sink_codepoint(char32_t codepoint);
	void send_sink_end();

	void asma2k_mem(address_map &map) ATTR_COLD;

	memory_view m_io_view;
	required_memory_bank m_dictbank;
	required_ioport m_pc_connected;

	uint8_t m_lcd_ctrl;
	uint16_t m_pc_frame = 0;
	uint8_t m_pc_frame_bits = 0;
	bool m_pc_set2_break = false;
	bool m_pc_set2_extended = false;
	uint8_t m_pc_set2_e1_remaining = 0;
	bool m_pc_lshift = false;
	bool m_pc_rshift = false;
	bool m_pc_lctrl = false;
	bool m_pc_rctrl = false;
	bool m_pc_lalt = false;
	bool m_pc_ralt = false;
	bool m_pc_caps_lock = false;
	// A host-side Windows ANSI keyboard profile; firmware only emits Set-2.
	// Numeric composition is tracked across make/break events, not ROM PCs.
	bool m_pc_alt_numeric_valid = false;
	uint8_t m_pc_alt_numeric_count = 0;
	char m_pc_alt_numeric_digits[4]{};
	bool m_send_sink_active = false;
	std::unique_ptr<emu_file> m_send_sink;
};

INPUT_CHANGED_MEMBER(alphasmart_state::kb_irq)
{
	// IRQ on every key transition
	m_maincpu->set_input_line(MC68HC11_IRQ_LINE, ASSERT_LINE);
}

uint8_t alphasmart_state::kb_r()
{
	uint16_t matrix = (m_matrix[1]<<8) | m_matrix[0];
	uint8_t data = 0xff;

	for(int i=0; i<16; i++)
		if (!(matrix & (1<<i)))
			data &= m_keyboard[i]->read();

	return data;
}

void alphasmart_state::kb_matrixl_w(uint8_t data)
{
	m_matrix[0] = data;
	m_maincpu->set_input_line(MC68HC11_IRQ_LINE, CLEAR_LINE);
}

void alphasmart_state::kb_matrixh_w(uint8_t data)
{
	m_matrix[1] = data;
}

uint8_t alphasmart_state::port_a_r()
{
	return (m_port_a & 0xfd) | (m_battery_status->read() << 1);
}

void alphasmart_state::update_lcdc(bool lcdc0, bool lcdc1)
{
	if (m_matrix[1] & 0x04)
	{
		uint8_t lcdc_data = 0;

		if (lcdc0)
			lcdc_data |= m_lcdc[0]->read(BIT(m_matrix[1], 1));

		if (lcdc1)
			lcdc_data |= m_lcdc[1]->read(BIT(m_matrix[1], 1));

		m_port_d = (m_port_d & 0xc3) | (lcdc_data>>2);
	}
	else
	{
		uint8_t lcdc_data = (m_port_d<<2) & 0xf0;

		if (lcdc0)
			m_lcdc[0]->write(BIT(m_matrix[1], 1), lcdc_data);

		if (lcdc1)
			m_lcdc[1]->write(BIT(m_matrix[1], 1), lcdc_data);
	}
}

void alphasmart_state::port_a_w(uint8_t data)
{
	uint8_t changed = (m_port_a ^ data) & data;
	update_lcdc(changed & 0x80, changed & 0x20);
	m_rambank->set_entry(((data>>3) & 0x01) | ((data>>4) & 0x02));
	m_port_a = data;
}

uint8_t alphasmart_state::port_d_r()
{
	return m_port_d;
}

void alphasmart_state::port_d_w(uint8_t data)
{
	m_port_d = data;
}


void alphasmart_state::alphasmart_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("rambank");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0x8000, 0x8000).rw(FUNC(alphasmart_state::kb_r), FUNC(alphasmart_state::kb_matrixh_w));
	map(0xc000, 0xc000).w(FUNC(alphasmart_state::kb_matrixl_w));
}

void asma2k_state::lcd_ctrl_w(uint8_t data)
{
	uint8_t changed = (m_lcd_ctrl ^ data) & data;
	update_lcdc(changed & 0x01, changed & 0x02);
	m_dictbank->set_entry((m_port_a & 0x30) >> 3 | (data & 0x80) >> 7);
	m_lcd_ctrl = data;
}

uint8_t asma2k_state::asma2k_port_a_r()
{
	uint8_t data = (m_port_a & 0xfd) | (m_battery_status->read() << 1);
	if (BIT(m_pc_connected->read(), 0))
		data |= 0x05; // PC attached: PA2 asserted, PA0 idle high
	return data;
}

struct as2k_set2_key_map
{
	uint8_t code;
	bool extended;
	as2k_pc_key key;
};

static constexpr as2k_set2_key_map s_set2_keys[] =
{
	{ 0x0d, false, as2k_pc_key::tab },        { 0x0e, false, as2k_pc_key::grave },
	{ 0x11, false, as2k_pc_key::left_alt },   { 0x11, true,  as2k_pc_key::right_alt },
	{ 0x12, false, as2k_pc_key::left_shift }, { 0x14, false, as2k_pc_key::left_ctrl },
	{ 0x14, true,  as2k_pc_key::right_ctrl }, { 0x15, false, as2k_pc_key::q },
	{ 0x16, false, as2k_pc_key::num_1 },      { 0x1a, false, as2k_pc_key::z },
	{ 0x1b, false, as2k_pc_key::s },          { 0x1c, false, as2k_pc_key::a },
	{ 0x1d, false, as2k_pc_key::w },          { 0x1e, false, as2k_pc_key::num_2 },
	{ 0x21, false, as2k_pc_key::c },          { 0x22, false, as2k_pc_key::x },
	{ 0x23, false, as2k_pc_key::d },          { 0x24, false, as2k_pc_key::e },
	{ 0x25, false, as2k_pc_key::num_4 },      { 0x26, false, as2k_pc_key::num_3 },
	{ 0x29, false, as2k_pc_key::space },      { 0x2a, false, as2k_pc_key::v },
	{ 0x2b, false, as2k_pc_key::f },          { 0x2c, false, as2k_pc_key::t },
	{ 0x2d, false, as2k_pc_key::r },          { 0x2e, false, as2k_pc_key::num_5 },
	{ 0x31, false, as2k_pc_key::n },          { 0x32, false, as2k_pc_key::b },
	{ 0x33, false, as2k_pc_key::h },          { 0x34, false, as2k_pc_key::g },
	{ 0x35, false, as2k_pc_key::y },          { 0x36, false, as2k_pc_key::num_6 },
	{ 0x3a, false, as2k_pc_key::m },          { 0x3b, false, as2k_pc_key::j },
	{ 0x3c, false, as2k_pc_key::u },          { 0x3d, false, as2k_pc_key::num_7 },
	{ 0x3e, false, as2k_pc_key::num_8 },      { 0x41, false, as2k_pc_key::comma },
	{ 0x42, false, as2k_pc_key::k },          { 0x43, false, as2k_pc_key::i },
	{ 0x44, false, as2k_pc_key::o },          { 0x45, false, as2k_pc_key::num_0 },
	{ 0x46, false, as2k_pc_key::num_9 },      { 0x49, false, as2k_pc_key::period },
	{ 0x4a, false, as2k_pc_key::slash },      { 0x4b, false, as2k_pc_key::l },
	{ 0x4c, false, as2k_pc_key::semicolon },  { 0x4d, false, as2k_pc_key::p },
	{ 0x4e, false, as2k_pc_key::minus },      { 0x52, false, as2k_pc_key::quote },
	{ 0x54, false, as2k_pc_key::lbrace },     { 0x55, false, as2k_pc_key::equals },
	{ 0x58, false, as2k_pc_key::caps_lock },  { 0x59, false, as2k_pc_key::right_shift },
	{ 0x5a, false, as2k_pc_key::enter },      { 0x5b, false, as2k_pc_key::rbrace },
	{ 0x5d, false, as2k_pc_key::backslash },  { 0x66, false, as2k_pc_key::backspace },
	{ 0x69, false, as2k_pc_key::kp_1 },       { 0x6b, false, as2k_pc_key::kp_4 },
	{ 0x6c, false, as2k_pc_key::kp_7 },       { 0x70, false, as2k_pc_key::kp_0 },
	{ 0x72, false, as2k_pc_key::kp_2 },       { 0x73, false, as2k_pc_key::kp_5 },
	{ 0x74, false, as2k_pc_key::kp_6 },       { 0x75, false, as2k_pc_key::kp_8 },
	{ 0x7a, false, as2k_pc_key::kp_3 },       { 0x7d, false, as2k_pc_key::kp_9 }
};

// The host text layout is indexed by consecutive as2k_pc_key values from
// a through slash.  Each row is [unshifted, shifted]; Caps Lock applies
// only to the contiguous a..z range, not to punctuation or number keys.
// Alt-numeric composition is handled separately from this ordinary layout.
static constexpr char32_t s_pc_us_text[][2] =
{
	{ U'a', U'A' }, // a
	{ U'b', U'B' }, // b
	{ U'c', U'C' }, // c
	{ U'd', U'D' }, // d
	{ U'e', U'E' }, // e
	{ U'f', U'F' }, // f
	{ U'g', U'G' }, // g
	{ U'h', U'H' }, // h
	{ U'i', U'I' }, // i
	{ U'j', U'J' }, // j
	{ U'k', U'K' }, // k
	{ U'l', U'L' }, // l
	{ U'm', U'M' }, // m
	{ U'n', U'N' }, // n
	{ U'o', U'O' }, // o
	{ U'p', U'P' }, // p
	{ U'q', U'Q' }, // q
	{ U'r', U'R' }, // r
	{ U's', U'S' }, // s
	{ U't', U'T' }, // t
	{ U'u', U'U' }, // u
	{ U'v', U'V' }, // v
	{ U'w', U'W' }, // w
	{ U'x', U'X' }, // x
	{ U'y', U'Y' }, // y
	{ U'z', U'Z' }, // z
	{ U'0', U')' }, // num_0
	{ U'1', U'!' }, // num_1
	{ U'2', U'@' }, // num_2
	{ U'3', U'#' }, // num_3
	{ U'4', U'$' }, // num_4
	{ U'5', U'%' }, // num_5
	{ U'6', U'^' }, // num_6
	{ U'7', U'&' }, // num_7
	{ U'8', U'*' }, // num_8
	{ U'9', U'(' }, // num_9
	{ U'`', U'~' }, // grave
	{ U'-', U'_' }, // minus
	{ U'=', U'+' }, // equals
	{ U'[', U'{' }, // lbrace
	{ U']', U'}' }, // rbrace
	{ U'\\', U'|' }, // backslash
	{ U';', U':' }, // semicolon
	{ U'\'', U'"' }, // quote
	{ U',', U'<' }, // comma
	{ U'.', U'>' }, // period
	{ U'/', U'?' }, // slash
};

static_assert(std::size(s_pc_us_text) == unsigned(as2k_pc_key::slash) - unsigned(as2k_pc_key::a) + 1);

INPUT_CHANGED_MEMBER(asma2k_state::pc_connected_changed)
{
	// PC Connected defines the host session.  Send is intentionally not part
	// of the sink contract: a real PC only sees keyboard traffic on the wire.
	m_pc_frame = 0;
	m_pc_frame_bits = 0;
	pc_keyboard_reset();
	send_sink_end();
}

void asma2k_state::pc_keyboard_reset()
{
	m_pc_set2_break = false;
	m_pc_set2_extended = false;
	m_pc_set2_e1_remaining = 0;
	m_pc_lshift = false;
	m_pc_rshift = false;
	m_pc_lctrl = false;
	m_pc_rctrl = false;
	m_pc_lalt = false;
	m_pc_ralt = false;
	m_pc_caps_lock = false;
	m_pc_alt_numeric_valid = false;
	m_pc_alt_numeric_count = 0;
}

void asma2k_state::pc_clock_rising(bool data)
{
	if (!m_pc_frame_bits)
	{
		if (data)
			return;
		m_pc_frame = 0;
	}

	if (data)
		m_pc_frame |= uint16_t(1) << m_pc_frame_bits;
	++m_pc_frame_bits;

	if (m_pc_frame_bits != 11)
		return;

	uint8_t const value = (m_pc_frame >> 1) & 0xff;
	bool odd_parity = BIT(m_pc_frame, 9);
	for (unsigned bit = 0; bit < 8; ++bit)
		odd_parity ^= BIT(value, bit);

	bool const valid = !BIT(m_pc_frame, 0) && BIT(m_pc_frame, 10) && odd_parity;
	bool const resync_start = !BIT(m_pc_frame, 10);
	m_pc_frame = 0;
	m_pc_frame_bits = 0;

	if (valid)
	{
		if (!m_send_sink_active)
		{
			if (!BIT(m_pc_connected->read(), 0))
				return;
			send_sink_begin();
			if (!m_send_sink_active)
				return;
		}

		pc_set2_byte(value);
	}
	else
	{
		logerror("AS2K_PC_FRAME_ERROR value=%02X\n", value);
		if (resync_start)
			m_pc_frame_bits = 1;
	}
}

void asma2k_state::asma2k_port_d_w(uint8_t data)
{
	// The wired PC keyboard interface uses PD0 as clock and inverted PD1 as data.
	if (BIT(m_pc_connected->read(), 0) && !BIT(m_port_d, 0) && BIT(data, 0))
		pc_clock_rising(!BIT(data, 1));

	alphasmart_state::port_d_w(data);
}

void asma2k_state::send_sink_begin()
{
	if (m_send_sink)
	{
		m_send_sink->close();
		m_send_sink.reset();
	}

	std::string rom_directory;
	path_iterator rom_paths(machine().options().media_path());
	if (!rom_paths.next(rom_directory) || rom_directory.empty())
	{
		logerror("AS2K_TX TEXT_SINK_OPEN_FAILED reason=no_rom_directory\n");
		m_send_sink_active = false;
		return;
	}

	m_send_sink = std::make_unique<emu_file>(
		rom_directory,
		OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	std::error_condition const err = m_send_sink->open("send.txt");
	if (err)
	{
		logerror("AS2K_TX TEXT_SINK_OPEN_FAILED path=%s%s%s error=%s\n",
			rom_directory,
			PATH_SEPARATOR,
			"send.txt",
			err.message());
		m_send_sink.reset();
		m_send_sink_active = false;
	}
	else
	{
		logerror("AS2K_TX TEXT_SINK_OPEN path=%s\n", m_send_sink->fullpath());
		m_send_sink_active = true;
	}

}

void asma2k_state::pc_set2_byte(uint8_t data)
{
	if (m_pc_set2_e1_remaining)
	{
		--m_pc_set2_e1_remaining;
		return;
	}

	if (data == 0xe1)
	{
		// Pause/Break is the canonical eight-byte E1 sequence in Set 2.  It
		// carries no text, so consume the sequence without fabricating a key.
		m_pc_set2_e1_remaining = 7;
		m_pc_set2_break = false;
		m_pc_set2_extended = false;
		return;
	}

	if (data == 0xe0)
	{
		m_pc_set2_extended = true;
		return;
	}

	if (data == 0xf0)
	{
		m_pc_set2_break = true;
		return;
	}

	as2k_pc_key key = as2k_pc_key::none;
	for (auto const &entry : s_set2_keys)
	{
		if ((entry.code == data) && (entry.extended == m_pc_set2_extended))
		{
			key = entry.key;
			break;
		}
	}

	if (key != as2k_pc_key::none)
		pc_key_event(key, !m_pc_set2_break);
	else
		logerror("AS2K_PC_UNKNOWN_SET2 extended=%u break=%u code=%02X\n",
			m_pc_set2_extended ? 1U : 0U, m_pc_set2_break ? 1U : 0U, data);

	m_pc_set2_break = false;
	m_pc_set2_extended = false;
}

// Windows ANSI Alt+0xxx numeric codes use Windows-1252 for the
// 0x80-0x9f range. Undefined CP1252 byte positions have value zero.
// This is a selected host profile, NOT a claim about all receiving PCs.
static constexpr char32_t s_pc_windows_1252_extended[32] =
{
	U'€', 0, U'‚', U'ƒ', U'„', U'…', U'†', U'‡',
	U'ˆ', U'‰', U'Š', U'‹', U'Œ', 0, U'Ž', 0,
	0, U'‘', U'’', U'“', U'”', U'•', U'–', U'—',
	U'˜', U'™', U'š', U'›', U'œ', 0, U'ž', U'Ÿ'
};

void asma2k_state::pc_key_event(as2k_pc_key key, bool pressed)
{
	bool const was_alt = m_pc_lalt || m_pc_ralt;
	if (key == as2k_pc_key::left_alt || key == as2k_pc_key::right_alt)
	{
		if (key == as2k_pc_key::left_alt)
			m_pc_lalt = pressed;
		else
			m_pc_ralt = pressed;

		bool const now_alt = m_pc_lalt || m_pc_ralt;
		if (!was_alt && now_alt)
		{
			m_pc_alt_numeric_count = 0;
			m_pc_alt_numeric_valid = !(m_pc_lctrl || m_pc_rctrl || m_pc_lshift || m_pc_rshift);
		}
		else if (was_alt && !now_alt)
		{
			// Host Windows ANSI profile: Alt+0ddd is one composition.
			// Releasing Alt commits a complete, bounded keypad sequence only.
			if (m_pc_alt_numeric_valid && m_pc_alt_numeric_count == 4 && m_pc_alt_numeric_digits[0] == '0')
			{
				unsigned code = 0;
				for (char digit : m_pc_alt_numeric_digits)
					code = code * 10 + unsigned(digit - '0');

				char32_t character = 0;
				if (code >= 0x20 && code < 0x7f)
					character = char32_t(code);
				else if (code >= 0x80 && code < 0xa0)
					character = s_pc_windows_1252_extended[code - 0x80];
				else if (code >= 0xa0 && code <= 0xff)
					character = char32_t(code);

				if (character)
					send_sink_codepoint(character);
				else
					logerror("AS2K_PC_ALT_NUMERIC_UNMAPPED code=%u\n", code);
			}
			else if (m_pc_alt_numeric_count)
				logerror("AS2K_PC_ALT_NUMERIC_CANCEL count=%u\n", m_pc_alt_numeric_count);

			m_pc_alt_numeric_count = 0;
			m_pc_alt_numeric_valid = false;
		}
		return;
	}

	switch (key)
	{
	case as2k_pc_key::left_shift:  m_pc_lshift = pressed; break;
	case as2k_pc_key::right_shift: m_pc_rshift = pressed; break;
	case as2k_pc_key::left_ctrl:   m_pc_lctrl = pressed; break;
	case as2k_pc_key::right_ctrl:  m_pc_rctrl = pressed; break;
	case as2k_pc_key::caps_lock:
		if (pressed)
			m_pc_caps_lock = !m_pc_caps_lock;
		break;
	default:
		break;
	}

	// Any non-keypad press during Alt cancels the numeric transaction,
	// even if the pressed key is not normally a text key.
	if (pressed && was_alt)
	{
		char digit = 0;
		switch (key)
		{
		case as2k_pc_key::kp_0: digit = '0'; break;
		case as2k_pc_key::kp_1: digit = '1'; break;
		case as2k_pc_key::kp_2: digit = '2'; break;
		case as2k_pc_key::kp_3: digit = '3'; break;
		case as2k_pc_key::kp_4: digit = '4'; break;
		case as2k_pc_key::kp_5: digit = '5'; break;
		case as2k_pc_key::kp_6: digit = '6'; break;
		case as2k_pc_key::kp_7: digit = '7'; break;
		case as2k_pc_key::kp_8: digit = '8'; break;
		case as2k_pc_key::kp_9: digit = '9'; break;
		default: break;
		}
		if (digit && m_pc_alt_numeric_valid && m_pc_alt_numeric_count < 4)
			m_pc_alt_numeric_digits[m_pc_alt_numeric_count++] = digit;
		else
			m_pc_alt_numeric_valid = false;
		return;
	}

	if (!pressed)
		return;

	// Unrecognized combinations must not leak an ordinary printable key.
	if (m_pc_lctrl || m_pc_rctrl || was_alt)
	{
		logerror("AS2K_PC_COMPOSE_PENDING key=%u ctrl=%u alt=%u\n",
			unsigned(key), (m_pc_lctrl || m_pc_rctrl) ? 1U : 0U, was_alt ? 1U : 0U);
		return;
	}

	if (key == as2k_pc_key::left_shift || key == as2k_pc_key::right_shift ||
		key == as2k_pc_key::left_ctrl || key == as2k_pc_key::right_ctrl ||
		key == as2k_pc_key::caps_lock)
		return;

	if (key == as2k_pc_key::space)
	{
		send_sink_codepoint(U' ');
		return;
	}
	if (key == as2k_pc_key::tab)
	{
		send_sink_codepoint(U'\t');
		return;
	}
	if (key == as2k_pc_key::enter)
	{
		send_sink_codepoint(U'\n');
		return;
	}
	if (key == as2k_pc_key::backspace)
	{
		logerror("AS2K_PC_NON_TEXT backspace\n");
		return;
	}

	if (key >= as2k_pc_key::a && key <= as2k_pc_key::slash)
	{
		bool const shifted = m_pc_lshift || m_pc_rshift;
		bool const letter = key <= as2k_pc_key::z;
		bool const use_shifted = letter ? (shifted != m_pc_caps_lock) : shifted;
		send_sink_codepoint(s_pc_us_text[unsigned(key) - unsigned(as2k_pc_key::a)][use_shifted ? 1 : 0]);
		return;
	}

	logerror("AS2K_PC_NON_TEXT key=%u\n", unsigned(key));
}

void asma2k_state::send_sink_codepoint(char32_t codepoint)
{
	if (!m_send_sink_active)
		return;

	char utf8[4];
	size_t length = 0;
	if (codepoint <= 0x7f)
	{
		utf8[0] = char(codepoint);
		length = 1;
	}
	else if (codepoint <= 0x7ff)
	{
		utf8[0] = char(0xc0 | (codepoint >> 6));
		utf8[1] = char(0x80 | (codepoint & 0x3f));
		length = 2;
	}
	else if (codepoint <= 0xffff)
	{
		utf8[0] = char(0xe0 | (codepoint >> 12));
		utf8[1] = char(0x80 | ((codepoint >> 6) & 0x3f));
		utf8[2] = char(0x80 | (codepoint & 0x3f));
		length = 3;
	}
	else if (codepoint <= 0x10ffff)
	{
		utf8[0] = char(0xf0 | (codepoint >> 18));
		utf8[1] = char(0x80 | ((codepoint >> 12) & 0x3f));
		utf8[2] = char(0x80 | ((codepoint >> 6) & 0x3f));
		utf8[3] = char(0x80 | (codepoint & 0x3f));
		length = 4;
	}

	if (length)
	{
		m_send_sink->write(utf8, length);
		m_send_sink->flush();
	}
}

void asma2k_state::send_sink_end()
{
	if (m_send_sink)
	{
		m_send_sink->flush();
		m_send_sink->close();
		m_send_sink.reset();
	}
	m_send_sink_active = false;
}

void asma2k_state::port_a_w(uint8_t data)
{
	m_io_view.select(BIT(data, 6));

	m_rambank->set_entry(((data>>4) & 0x03));
	m_dictbank->set_entry((data & 0x30) >> 3 | (m_lcd_ctrl & 0x80) >> 7);
	m_port_a = data;
}


void asma2k_state::asma2k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).view(m_io_view);
	m_io_view[0](0x2000, 0x2000).rw(FUNC(asma2k_state::kb_r), FUNC(asma2k_state::kb_matrixh_w));
	m_io_view[0](0x4000, 0x4000).w(FUNC(asma2k_state::lcd_ctrl_w));
	m_io_view[0](0x4000, 0x7fff).bankr("dictbank");
	m_io_view[1](0x0000, 0x7fff).bankrw("rambank");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0x9000, 0x9000).w(FUNC(asma2k_state::kb_matrixl_w));
}

/* Input ports */
static INPUT_PORTS_START( alphasmart )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)    PORT_CHAR(UCHAR_MAMEKEY(F8))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)    PORT_CHAR(UCHAR_MAMEKEY(F7))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('^')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('&')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')    PORT_CHAR('\"') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)  PORT_CHAR(UCHAR_MAMEKEY(END))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Left Command") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Right Command") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)//
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)  PORT_NAME("Clear File") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_NAME("Send") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)   PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Right Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')    PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)   PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')    PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)    PORT_CHAR('k')  PORT_CHAR('K')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)    PORT_CHAR('8')  PORT_CHAR('*')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)    PORT_CHAR('i')  PORT_CHAR('I')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)   PORT_NAME("Pause") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)  PORT_NAME("ScrLk") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_NAME("Return") PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_CHAR(UCHAR_MAMEKEY(F2))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)   PORT_CHAR(UCHAR_MAMEKEY(F4))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_CHAR(UCHAR_MAMEKEY(F3))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d')  PORT_CHAR('D')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)    PORT_CHAR('3')  PORT_CHAR('#')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e')  PORT_CHAR('E')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)    PORT_CHAR('c')  PORT_CHAR('C')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)   PORT_CHAR(UCHAR_MAMEKEY(F1))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('s')  PORT_CHAR('S')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)    PORT_CHAR('2')  PORT_CHAR('@')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)    PORT_CHAR('w')  PORT_CHAR('W')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)    PORT_CHAR('x')  PORT_CHAR('X')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)  PORT_CHAR(UCHAR_MAMEKEY(ESC))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)  PORT_CHAR('\t')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)    PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)    PORT_CHAR('1')  PORT_CHAR('!')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)    PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)

	PORT_START("BATTERY")
	PORT_CONFNAME(0x01, 0x01, "Battery status")
	PORT_CONFSETTING (0x00, DEF_STR(Low))
	PORT_CONFSETTING (0x01, DEF_STR(Normal))
INPUT_PORTS_END

static INPUT_PORTS_START( asma2k )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)   PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("F6 (File 6)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('*')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Right Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(RALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)   PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("F7 (File 7)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)   PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("F8 (File 8)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')    PORT_CHAR('\"') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Command") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)  PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_NAME("Clear File") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(ESC))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)  PORT_CHAR(UCHAR_MAMEKEY(END))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_NAME("Send") PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)  PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_NAME("Find") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Delete") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)   PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("F5 (File 5)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)   PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_NAME("Print") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)  PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_NAME("Spell Check") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_NAME("Return") PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("F3 (File 3)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)   PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("F4 (File 4)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d')  PORT_CHAR('D')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e')  PORT_CHAR('E')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("F2 (File 2)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)    PORT_CHAR('3')  PORT_CHAR('#')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)    PORT_CHAR('c')  PORT_CHAR('C')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('s')  PORT_CHAR('S')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)    PORT_CHAR('w')  PORT_CHAR('W')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)   PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("F1 (File 1)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)    PORT_CHAR('2')  PORT_CHAR('@')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)    PORT_CHAR('x')  PORT_CHAR('X')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)  PORT_CHAR('\t')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)    PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)    PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)    PORT_CHAR('1')  PORT_CHAR('!')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)    PORT_CHAR('t')  PORT_CHAR('T')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)    PORT_CHAR('g')  PORT_CHAR('G')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)    PORT_CHAR('f')  PORT_CHAR('F')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)    PORT_CHAR('r')  PORT_CHAR('R')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)    PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)    PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)    PORT_CHAR('v')  PORT_CHAR('V')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)    PORT_CHAR('b')  PORT_CHAR('B')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)    PORT_CHAR('y')  PORT_CHAR('Y')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)    PORT_CHAR('h')  PORT_CHAR('H')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)    PORT_CHAR('j')  PORT_CHAR('J')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)    PORT_CHAR('u')  PORT_CHAR('U')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)    PORT_CHAR('6')  PORT_CHAR('^')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)    PORT_CHAR('7')  PORT_CHAR('&')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)    PORT_CHAR('m')  PORT_CHAR('M')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)    PORT_CHAR('n')  PORT_CHAR('N')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart_state::kb_irq), 0)

	// Emulator-only host attachment control.  Pause/Break is not part of the
	// AlphaSmart 2000 keyboard matrix, so it cannot be mistaken for an AS2K key.
	PORT_START("PC_CONNECTED")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PC Connected (Pause/Break)") PORT_CODE(KEYCODE_PAUSE) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(asma2k_state::pc_connected_changed), 0)

	PORT_START("BATTERY")
	PORT_CONFNAME(0x01, 0x01, "Battery status")
	PORT_CONFSETTING (0x00, DEF_STR(Low))
	PORT_CONFSETTING (0x01, DEF_STR(Normal))
INPUT_PORTS_END

void alphasmart_state::alphasmart_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint32_t alphasmart_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc[0]->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 0, cliprect);
	m_lcdc[1]->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 18,cliprect);
	return 0;
}

void alphasmart_state::machine_start()
{
	m_rambank->configure_entries(0, 4, &m_nvram_base[0], 0x8000);

	m_tmp_bitmap = std::make_unique<bitmap_ind16>(6 * 40, 9 * 4);
}

void asma2k_state::machine_start()
{
	alphasmart_state::machine_start();

	m_dictbank->configure_entries(0, 8, memregion("spellcheck")->base(), 0x4000);
	m_dictbank->set_entry(0);
}

void alphasmart_state::machine_reset()
{
	m_rambank->set_entry(0);
	m_matrix[0] = m_matrix[1] = 0;
	m_port_a = 0;
	m_port_d = 0;
}

void alphasmart_state::alphasmart(machine_config &config)
{
	/* basic machine hardware */
	MC68HC11D0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &alphasmart_state::alphasmart_mem);
	m_maincpu->in_pa_callback().set(FUNC(alphasmart_state::port_a_r));
	m_maincpu->out_pa_callback().set(FUNC(alphasmart_state::port_a_w));
	m_maincpu->in_pd_callback().set(FUNC(alphasmart_state::port_d_r));
	m_maincpu->out_pd_callback().set(FUNC(alphasmart_state::port_d_w));

	for (auto &lcdc : m_lcdc)
	{
		KS0066(config, lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
		lcdc->set_default_bios_tag("f05");
		lcdc->set_lcd_size(2, 40);
	}

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(alphasmart_state::screen_update));
	screen.set_size(6*40, 9*4);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(alphasmart_state::alphasmart_palette), 2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void asma2k_state::asma2k(machine_config &config)
{
	alphasmart(config);
	m_maincpu->in_pa_callback().set(FUNC(asma2k_state::asma2k_port_a_r));
	m_maincpu->out_pd_callback().set(FUNC(asma2k_state::asma2k_port_d_w));
	m_maincpu->set_addrmap(AS_PROGRAM, &asma2k_state::asma2k_mem);
}

// MCU: MC68HC11D0P
// NVRAM: KM681000ALP-7L (or TC551001BPL-85L) + CR2032 battery
// XTAL: 8.000MHz
// LCD: 2x KS0066F05 + 8x HD44100H
ROM_START( asmapro )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "alphasmartpro212.rom",  0x0000, 0x8000, CRC(896ddf1c) SHA1(c3c6a421c9ced92db97431d04b4a3f09a39de716) )   // Checksum 8D24 on label
ROM_END

// MCU: MC68HC11D0FN
// NVRAM: NEC D431000ACW-70LL + battery
// XTAL: SB8.000
// LCD: 2x KS0066F05 + 4x KS0063B
// Optional IrDA sub board
ROM_START( asma2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/*
	    These dumps 33,253 bytes each, probably contain 32768 bytes of rom,
	    plus the remaining area is pal data for the mapper/io pal, all of
	    which is integrated onto one plcc44 chip called a zpsd211r.
	*/
	ROM_SYSTEM_BIOS( 0, "v314", "v3.14" )
	ROMX_LOAD( "alphasmart__2000__v3.1.4__h4.zpsd211r.plcc44.bin",  0x0000, 0x81e5, CRC(49487f6d) SHA1(e0b777dc68c671c31ba808e214fb9d2573b9a853), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v308", "v3.08" )
	ROMX_LOAD( "alphasmart__2000__v3.0.8.zpsd211r.plcc44.bin",  0x0000, 0x81e5, CRC(0b3b1a0c) SHA1(97878819188a1ec40052fbce9d5a5059728d5aec), ROM_BIOS(1) )

	ROM_REGION( 0x20000, "spellcheck", 0 )
	ROM_LOAD( "dictrom__v1.stm_m27c1001-1501.plcc32.bin", 0x00000, 0x20000, CRC(a143949c) SHA1(033094bb850c614008b4ecc2eefbcb01b8a2bcda) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                           FULLNAME           FLAGS
COMP( 1995, asmapro, 0,      0,      alphasmart, alphasmart, alphasmart_state, empty_init, "Intelligent Peripheral Devices", "AlphaSmart Pro" , MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, asma2k,  0,      0,      asma2k,     asma2k,     asma2k_state,     empty_init, "Intelligent Peripheral Devices", "AlphaSmart 2000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
