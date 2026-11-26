// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Michael Wessel
/***************************************************************************

Kosmos CP1 "Computer-Praxis" (1983), an experimental computer kit

Hardware notes:
- M5L8049 @ 6 MHz (or an 8039 with the program in a 2716 EPROM)
- 8155: 256 bytes RAM = program cells 000-127 (opcode, operand), port A = display
  segments, port B = Port 2 (the LEDs on the I/O module), port C = digit select and
  keyboard rows
- 6-digit 7-segment display, membrane keyboard with 20+10 keys (the number block is
  wired in parallel with the digit keys)
- 8049 P1 = Port 1 (the switches on the I/O module); the CP2 cassette interface uses
  P1.6 (write) and P1.7 (data)
- 8049 P2: keyboard columns (low nibble, read with IN A,P2 in the mask ROM, MOVD A,P4
  in the 2716; there is no 8243), 8155 /CE, CP3 /CE, 8155 RESET, 8155 IO/M
- CP3 memory expansion: a second 8155 for cells 128-255, Port 3 (inputs, port B),
  Port 4 (port A) and Port 5 (port C); the firmware detects it at power-on
- CP5: not emulated

The manual's random number programs wire Port 2 back to Port 1 (Bild 65: lines 1-8 to
3, 4, 2, 1, 7, 8, 6, 5). This is a configuration setting; the switches must be up.

Quickload (.cp1): one program cell per line, "NNN OO.DDD" (decimal, a mnemonic may
stand before the opcode), or "OO.DDD", a mnemonic with its operand or "DATA n" for the
next cell; "@NNN" or ".ORG NNN" sets the next cell, '#' and ';' start comments.
Load a program after the display shows "P  000".

****************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/i8155.h"
#include "video/pwm.h"

#include "speaker.h"

#include "corestr.h"

#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cp1.lh"


namespace {

class cp1_state : public driver_device
{
public:
	cp1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8155(*this, "i8155"),
		m_i8155_cp3(*this, "i8155_cp3"),
		m_cassette(*this, "cassette"),
		m_display(*this, "display"),
		m_io_lines(*this, "LINE%u", 0U),
		m_io_port1(*this, "PORT1"),
		m_io_config(*this, "CONFIG"),
		m_port2_led(*this, "p2_led%u", 0U)
	{ }

	void cp1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cp1_io(address_map &map) ATTR_COLD;

	uint8_t port1_r();
	uint8_t port2_r();
	void port1_w(uint8_t data);
	void port2_w(uint8_t data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	uint8_t i8155_read(offs_t offset);
	void i8155_write(offs_t offset, uint8_t data);
	void i8155_porta_w(uint8_t data);
	void i8155_portb_w(uint8_t data);
	void i8155_portc_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<i8155_device> m_i8155;
	required_device<i8155_device> m_i8155_cp3;
	required_device<cassette_image_device> m_cassette;
	required_device<pwm_display_device> m_display;
	required_ioport_array<5> m_io_lines;
	required_ioport m_io_port1;
	required_ioport m_io_config;
	output_finder<8> m_port2_led;

	uint8_t m_port2 = 0;
	uint8_t m_matrix = 0;
	uint8_t m_port2_out = 0xff;
};

uint8_t cp1_state::port1_r()
{
	const uint8_t config = m_io_config->read();
	uint8_t data = m_io_port1->read();

	// Port 2 wired back to Port 1 (Bild 65: lines 1-8 to 3, 4, 2, 1, 7, 8, 6, 5)
	if (config & 0x04)
		data &= bitswap<8>(m_port2_out, 5, 4, 6, 7, 1, 0, 2, 3);

	// CP2 cassette data on P1.7
	if (config & 0x01)
		data = (data & 0x7f) | ((m_cassette->input() > 0.03) ? 0x80 : 0x00);

	return data;
}

void cp1_state::port1_w(uint8_t data)
{
	if (m_io_config->read() & 0x01)
		m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

uint8_t cp1_state::port2_r()
{
	// x--- ----   I8155 IO/M
	// -x-- ----   I8155 RESET
	// --x- ----   expansion port CE
	// ---x ----   I8155 CE
	// ---- xxxx   keyboard input

	uint8_t data = 0;

	for (int i = 0; i < 5; i++)
		if (BIT(m_matrix, i))
			data |= m_io_lines[i]->read();

	return (data & 0x0f) | (m_port2 & 0xf0);
}

void cp1_state::port2_w(uint8_t data)
{
	if (data & 0x40)
	{
		m_i8155->reset();

		if (m_io_config->read() & 0x02)
			m_i8155_cp3->reset();
	}

	m_port2 = data;
}

uint8_t cp1_state::i8155_read(offs_t offset)
{
	uint8_t data = 0;

	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(BIT(m_port2, 7), offset);
		data |= m_i8155->data_r();
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(BIT(m_port2, 7), offset);
		data |= m_i8155_cp3->data_r();
	}

	return data;
}

void cp1_state::i8155_write(offs_t offset, uint8_t data)
{
	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(BIT(m_port2, 7), offset);
		m_i8155->data_w(data);
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(BIT(m_port2, 7), offset);
		m_i8155_cp3->data_w(data);
	}
}

void cp1_state::i8155_porta_w(uint8_t data)
{
	m_display->write_mx(data | 0x80); // PA7 is not connected
}

void cp1_state::i8155_portb_w(uint8_t data)
{
	// Port 2: the LEDs on the I/O module
	m_port2_out = data;
	for (int i = 0; i < 8; i++)
		m_port2_led[i] = BIT(data, i);
}

void cp1_state::i8155_portc_w(uint8_t data)
{
	// --xx xxxx   keyboard matrix, 7seg select
	m_matrix = ~data & 0x3f;
	m_display->write_my(m_matrix);
}


void cp1_state::cp1_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0xff).rw(FUNC(cp1_state::i8155_read), FUNC(cp1_state::i8155_write));
}

/* Input ports */
INPUT_PORTS_START( cp1 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)   PORT_NAME("CAS [Cass. speichern]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CLR [Irrtum]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C)   PORT_CODE(KEYCODE_P) PORT_NAME(u8"PC [Programmzähler]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)   PORT_NAME("ACC [Akku]")
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)   PORT_NAME("CAL [Cass. lesen]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)   PORT_NAME("STEP [Schritt]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B)   PORT_CODE(KEYCODE_H) PORT_NAME("STP [Stopp]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)   PORT_NAME("RUN [Lauf]")
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)   PORT_NAME("8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9)   PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)   PORT_NAME("OUT [Auslesen]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)   PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP [Eingeben]")
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)   PORT_NAME("4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)   PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)   PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)   PORT_NAME("7")
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0)   PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)   PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)   PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)   PORT_NAME("3")

	PORT_START("PORT1") // switches on the I/O module, up = 1
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Port 1 Switch 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Port 1 Switch 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Port 1 Switch 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Port 1 Switch 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Port 1 Switch 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Port 1 Switch 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Port 1 Switch 7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Port 1 Switch 8")

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "CP2 Cassette Interface (on Port 1)" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x02, 0x00, "CP3 RAM Expansion" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x02, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x04, 0x00, "Port 2 Wired to Port 1 (Random Numbers)" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x04, DEF_STR( Yes ) )
INPUT_PORTS_END

void cp1_state::machine_start()
{
	save_item(NAME(m_port2));
	save_item(NAME(m_matrix));
	save_item(NAME(m_port2_out));
}

void cp1_state::machine_reset()
{
	m_cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}

QUICKLOAD_LOAD_MEMBER(cp1_state::quickload_cb)
{
	if (image.length() > 0x10000)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	std::string text(image.length(), '\0');
	if (image.fread(text.data(), text.size()) != text.size())
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	static constexpr char const *const mnemonics[] =
	{
		"HLT", "ANZ", "VZG", "AKO", "LDA", "ABS", "ADD", "SUB", "SPU", "VGL", "SPB", "VGR",
		"VKL", "NEG", "UND", "P1E", "P1A", "P2A", "LIA", "AIS", "SIU", "P3E", "P4A", "P5A"
	};

	// a decimal number of min to max digits, or -1
	auto number = [] (std::string_view s, size_t min, size_t max) -> int
	{
		if (s.size() < min || s.size() > max)
			return -1;
		int value = 0;
		for (char c : s)
		{
			if (!isdigit(uint8_t(c)))
				return -1;
			value = value * 10 + (c - '0');
		}
		return value;
	};

	// "OO.DDD" (a comma may stand for the dot): opcode * 1000 + operand, or -1
	auto cell = [&number] (std::string_view s) -> int
	{
		const size_t dot = s.find_first_of(".,");
		if (dot == std::string_view::npos)
			return -1;
		const int op = number(s.substr(0, dot), 1, 2), data = number(s.substr(dot + 1), 1, 3);
		return (op < 0 || data < 0) ? -1 : (op * 1000 + data);
	};

	std::vector<std::pair<int, int> > cells; // cell, opcode * 1000 + operand
	int addr = 0;
	uint32_t line_number = 0;

	for (size_t pos = 0; pos < text.size(); )
	{
		size_t end = text.find('\n', pos);
		if (end == std::string::npos)
			end = text.size();
		std::string line = text.substr(pos, end - pos);
		pos = end + 1;
		line_number++;

		const size_t comment = line.find_first_of("#;");
		if (comment != std::string::npos)
			line.erase(comment);

		std::vector<std::string> words;
		for (size_t i = 0; i < line.size(); )
		{
			while (i < line.size() && isspace(uint8_t(line[i])))
				i++;
			const size_t start = i;
			while (i < line.size() && !isspace(uint8_t(line[i])))
				i++;
			if (i > start)
				words.emplace_back(line.substr(start, i - start));
		}
		if (words.empty())
			continue;

		int word = -1;
		const std::string first = strmakeupper(words[0]);

		if (first[0] == '@' || first == ".ORG")
		{
			std::string n = (first[0] == '@') ? first.substr(1) : std::string();
			if (words.size() == 2)
				n += words[1];
			const int a = (words.size() <= 2) ? number(n, 1, 3) : -1;
			if (a < 0 || a > 255)
				return std::make_pair(image_error::INVALIDIMAGE, util::string_format("Line %u: bad cell number", line_number));
			addr = a;
			continue;
		}
		else if (number(words[0], 3, 3) >= 0 && (words.size() == 2 || (words.size() == 3 && isalpha(uint8_t(words[1][0])))))
		{
			// "NNN [mnemonic] OO.DDD" or "NNN OODDD"
			addr = number(words[0], 3, 3);
			word = cell(words.back());
			if (word < 0 && words.size() == 2 && number(words[1], 5, 5) >= 0)
				word = number(std::string_view(words[1]).substr(0, 2), 2, 2) * 1000 + number(std::string_view(words[1]).substr(2), 3, 3);
		}
		else if (words.size() == 1 && cell(words[0]) >= 0)
		{
			word = cell(words[0]);
		}
		else if ((first == "DATA" || first == ".DB") && words.size() == 2)
		{
			word = number(words[1], 1, 3);
		}
		else if (words.size() <= 2)
		{
			// mnemonic with an optional operand
			const int data = (words.size() == 2) ? number(words[1], 1, 3) : 0;
			for (int i = 0; i < std::size(mnemonics); i++)
				if (first == mnemonics[i] && data >= 0)
					word = (i + 1) * 1000 + data;
		}

		if (word < 0)
			return std::make_pair(image_error::INVALIDIMAGE, util::string_format("Line %u: cannot read \"%s\"", line_number, line));
		if (addr > 255)
			return std::make_pair(image_error::INVALIDIMAGE, util::string_format("Line %u: past cell 255", line_number));
		if (word / 1000 > 24 || word % 1000 > 255)
			return std::make_pair(image_error::INVALIDIMAGE, util::string_format("Line %u: opcode or operand out of range", line_number));
		if (addr > 127 && !(m_io_config->read() & 0x02))
			return std::make_pair(image_error::UNSUPPORTED, util::string_format("Line %u: cell %03d needs the CP3 RAM expansion (Machine Configuration)", line_number, addr));

		cells.emplace_back(addr++, word);
	}

	if (cells.empty())
		return std::make_pair(image_error::INVALIDIMAGE, std::string("No program cells found"));

	for (const auto &[a, w] : cells)
	{
		i8155_device &chip = (a < 128) ? *m_i8155 : *m_i8155_cp3;
		chip.memory_w((a & 0x7f) * 2, w / 1000);
		chip.memory_w((a & 0x7f) * 2 + 1, w % 1000);
	}

	return std::make_pair(std::error_condition(), std::string());
}

void cp1_state::cp1(machine_config &config)
{
	/* basic machine hardware */
	i8049_device &maincpu(I8049(config, m_maincpu, 6_MHz_XTAL));
	maincpu.set_addrmap(AS_IO, &cp1_state::cp1_io);
	maincpu.p1_in_cb().set(FUNC(cp1_state::port1_r));
	maincpu.p1_out_cb().set(FUNC(cp1_state::port1_w));
	maincpu.p2_in_cb().set(FUNC(cp1_state::port2_r));
	maincpu.p2_out_cb().set(FUNC(cp1_state::port2_w));

	i8155_device &i8155(I8155(config, "i8155"));
	i8155.out_pa_callback().set(FUNC(cp1_state::i8155_porta_w));
	i8155.out_pb_callback().set(FUNC(cp1_state::i8155_portb_w));
	i8155.out_pc_callback().set(FUNC(cp1_state::i8155_portc_w));

	i8155_device &i8155_cp3(I8155(config, "i8155_cp3"));
	i8155_cp3.in_pb_callback().set_constant(0xff); // Port 3, nothing connected

	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0x7f);
	m_display->set_segmask(0x08, 0xff); // 3rd digit DP is always on
	config.set_default_layout(layout_cp1);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "cp1,obj", attotime::from_seconds(1)).set_load_callback(FUNC(cp1_state::quickload_cb));
}

/* ROM definition */
/*
  KOSMOS B
  <Mitsubishi Logo> M5L8049-136P-6
  JAPAN 83F301
*/

ROM_START( cp1 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "b", "b" )
	ROMX_LOAD( "cp1-kosmos-b.rom", 0x0000, 0x0800, CRC(fea8a2b2) SHA1(c987b79a7b90fcbd58b66a69e95913f2655a1f0d), ROM_BIOS(0))
	// This is from 2716 eprom that was on board with I8039 instead of I8049
	ROM_SYSTEM_BIOS( 1, "2716", "2716" )
	ROMX_LOAD( "cp1-2716.bin",     0x0000, 0x0800, CRC(3a2caf0e) SHA1(ff4befcf82a664950186d3af1843fdef70d2209f), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY   FULLNAME                 FLAGS
COMP( 1983, cp1,  0,      0,      cp1,     cp1,   cp1_state, empty_init, "Kosmos", "CP1 / Computer-Praxis", MACHINE_SUPPORTS_SAVE )
