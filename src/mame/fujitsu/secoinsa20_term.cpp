// license:BSD-3-Clause
// copyright-holders:Eduardo Cruz
/***************************************************************************

    Secoinsa Serie 20 terminal (HLE)

    Stands in for the Secoinsa IS 9502 terminal, whose firmware and
    documentation haven't been found. It follows the cursor, erase and
    attribute sequences sent by the S.O. Multi-Usuario CLASE1 (ESC G column
    row) and CLASE2 (ANSI) terminal drivers.

    TODO:
    - original character generator
    - double size lines (ESC # n is accepted but ignored)
    - blink rate
    - Dump the keyboard ROM and implement as LLE

Keyboard layout:

⌈CHK ⌉⌈CYCL⌉  ⌈PF1‾⌉⌈PF2‾⌉⌈PF3‾⌉⌈PF4‾⌉⌈PF5‾⌉⌈PF6‾⌉⌈PF7‾⌉⌈PF8‾⌉⌈PF9‾⌉⌈PF10⌉⌈PF11⌉⌈PF12⌉⌈PF13⌉⌈PF14⌉⌈PF15⌉⌈PF16⌉
⌊____⌋⌊E___⌋  ⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋⌊____⌋
----------------------------------------------------------------------------------------------------------
⌈CAN‾⌉⌈CMD‾⌉⌈MOD‾⌉ ⌈‾1‾⌉⌈‾2‾⌉⌈‾3‾⌉⌈‾4‾⌉⌈‾5‾⌉⌈‾6‾⌉⌈‾7‾⌉⌈‾8‾⌉⌈‾9‾⌉⌈‾0‾⌉⌈‾'‾⌉⌈‾[‾⌉⌈‾]‾⌉   ⌈BOR‾‾⌉⌈BOR‾‾⌉⌈PAUSA⌉
⌊____⌋⌊____⌋⌊____⌋ ⌊___⌋⌊_=_⌋⌊_#_⌋⌊_$_⌋⌊_%_⌋⌊_&_⌋⌊_-_⌋⌊_(_⌋⌊_)_⌋⌊ _ ⌋⌊_?_⌋⌊_!_⌋⌊_|_⌋   ⌊LINEA⌋⌊PANT_⌋⌊_____⌋
⌈BOR‾⌉⌈SIG‾⌉⌈GRAB⌉ ⌈ESC⌉⌈‾Q‾⌉⌈‾W‾⌉⌈‾E‾⌉⌈‾R‾⌉⌈‾T‾⌉⌈‾Y‾⌉⌈‾U‾⌉⌈‾I‾⌉⌈‾O‾⌉⌈‾P‾⌉⌈‾@‾⌉⌈‾⇥‾⌉   ⌈‾‾7‾‾⌉⌈‾‾8‾‾⌉⌈‾‾9‾‾⌉
⌊REG_⌋⌊REG_⌋⌊REG_⌋ ⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋   ⌊_____⌋⌊_____⌋⌊_____⌋
⌈FIN‾⌉⌈BOR‾⌉⌈DUPL⌉ ⌈LOC⌉⌈ A ⌉⌈ S ⌉⌈ D ⌉⌈ F ⌉⌈ G ⌉⌈ H ⌉⌈ J ⌉⌈ K ⌉⌈ L ⌉⌈ Ñ ⌉⌈ * ⌉⌈RC ⌉   ⌈‾‾4‾‾⌉⌈‾‾5‾‾⌉⌈‾‾6‾‾⌉
⌊____⌋⌊CAR_⌋⌊____⌋ ⌊K__⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋   ⌊_____⌋⌊_____⌋⌊_____⌋
⌈PAG‾⌉⌈‾↑‾‾⌉⌈‾↓‾‾⌉ ⌈SHI⌉⌈‾Z‾⌉⌈‾X‾⌉⌈‾C‾⌉⌈‾V‾⌉⌈‾B‾⌉⌈‾N‾⌉⌈‾M‾⌉⌈‾+‾⌉⌈‾<‾⌉⌈‾?‾⌉⌈‾SHIFT‾‾⌉   ⌈‾‾1‾‾⌉⌈‾‾2‾‾⌉⌈‾‾3‾‾⌉
⌊____⌋⌊____⌋⌊____⌋ ⌊FT_⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊___⌋⌊________⌋   ⌊_____⌋⌊_____⌋⌊_____⌋
⌈‾‾‾‾⌉⌈‾‾‾‾⌉⌈‾→‾‾⌉      ⌈‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾SPACE BAR‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾⌉                 ⌈‾‾,‾‾⌉⌈‾‾0‾‾⌉⌈‾‾.‾‾⌉
⌊____⌋⌊____⌋⌊____⌋      ⌊____________________________________________⌋                 ⌊_____⌋⌊_____⌋⌊_____⌋

***************************************************************************/

#include "emu.h"
#include "secoinsa20_term.h"

#include "machine/terminal.h"

#include "screen.h"

#include <algorithm>
#include <iterator>


namespace {

class secoinsa20_terminal_device : public generic_terminal_device, public device_serial_interface, public device_rs232_port_interface
{
public:
	secoinsa20_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void input_txd(int state) override { device_serial_interface::rx_w(state); }
	virtual void input_rts(int state) override;

	void update_serial(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// generic_terminal_device implementation
	virtual void term_write(u8 data) override;
	virtual void send_key(u8 code) override;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	enum : u8
	{
		UNDERLINE = 0x01,
		BLINK = 0x02,
		REVERSE = 0x04
	};

	enum : u8
	{
		SEQ_NONE,
		SEQ_ESC,
		SEQ_COLUMN,
		SEQ_ROW,
		SEQ_CSI,
		SEQ_SIZE,
		SEQ_SKIP
	};

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void control_sequence(u8 command);
	void erase(unsigned start, unsigned end);
	void line_feed();
	void next_key();

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	u8 m_attributes[TERMINAL_WIDTH * TERMINAL_HEIGHT];
	u8 m_attribute;
	u8 m_sequence;
	u8 m_column;
	u16 m_parameters[8];
	u8 m_parameter;
	u8 m_keys[64];
	u8 m_key_head;
	u8 m_key_count;
	bool m_rts;
};

secoinsa20_terminal_device::secoinsa20_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: generic_terminal_device(mconfig, SECOINSA20_TERMINAL, tag, owner, clock, TERMINAL_WIDTH, TERMINAL_HEIGHT)
	, device_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_rxbaud(*this, "RS232_RXBAUD")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
	, m_attributes()
	, m_attribute(0)
	, m_sequence(SEQ_NONE)
	, m_column(0)
	, m_parameters()
	, m_parameter(0)
	, m_keys()
	, m_key_head(0)
	, m_key_count(0)
	, m_rts(true)
{
}

INPUT_PORTS_START(secoinsa20_terminal)
	PORT_INCLUDE(generic_terminal)

	PORT_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_19200, "TX Baud", secoinsa20_terminal_device, update_serial)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_19200, "RX Baud", secoinsa20_terminal_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_7, "Data Bits", secoinsa20_terminal_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", secoinsa20_terminal_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", secoinsa20_terminal_device, update_serial)
INPUT_PORTS_END

ioport_constructor secoinsa20_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(secoinsa20_terminal);
}

void secoinsa20_terminal_device::device_add_mconfig(machine_config &config)
{
	generic_terminal_device::device_add_mconfig(config);

	subdevice<screen_device>(TERMINAL_SCREEN_TAG)->set_screen_update(FUNC(secoinsa20_terminal_device::screen_update));
}

void secoinsa20_terminal_device::device_start()
{
	generic_terminal_device::device_start();

	save_item(NAME(m_attributes));
	save_item(NAME(m_attribute));
	save_item(NAME(m_sequence));
	save_item(NAME(m_column));
	save_item(NAME(m_parameters));
	save_item(NAME(m_parameter));
	save_item(NAME(m_keys));
	save_item(NAME(m_key_head));
	save_item(NAME(m_key_count));
	save_item(NAME(m_rts));
}

void secoinsa20_terminal_device::device_reset()
{
	generic_terminal_device::device_reset();

	std::fill(std::begin(m_attributes), std::end(m_attributes), 0);
	m_attribute = 0;
	m_sequence = SEQ_NONE;

	update_serial(0);
}

void secoinsa20_terminal_device::update_serial(int state)
{
	set_data_frame(1, convert_databits(m_rs232_databits->read()), convert_parity(m_rs232_parity->read()), convert_stopbits(m_rs232_stopbits->read()));
	set_tra_rate(convert_baud(m_rs232_txbaud->read()));
	set_rcv_rate(convert_baud(m_rs232_rxbaud->read()));
	receive_register_reset();
	transmit_register_reset();
	m_key_head = 0;
	m_key_count = 0;

	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

u32 secoinsa20_terminal_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// the base renderer draws 8x10 character cells
	generic_terminal_device::update(screen, bitmap, cliprect);

	u32 color;
	switch (m_io_term_conf->read() & 0x030)
	{
	case 0x010:
		color = rgb_t(0xf7, 0xaa, 0x00);
		break;
	case 0x020:
		color = rgb_t::white();
		break;
	default:
		color = rgb_t(0x00, 0xff, 0x00);
		break;
	}
	bool const blink = BIT(screen.frame_number(), 4);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u8 const *const attributes = &m_attributes[(y / 10) * m_width];
		u32 *const pixels = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u8 const attribute = attributes[x / 8];
			if (attribute)
			{
				bool on = pixels[x] != 0;
				if ((attribute & UNDERLINE) && ((y % 10) == 8))
					on = true;
				if ((attribute & BLINK) && blink)
					on = false;
				if (attribute & REVERSE)
					on = !on;
				pixels[x] = on ? color : 0;
			}
		}
	}

	return 0;
}

void secoinsa20_terminal_device::term_write(u8 data)
{
	if ((data == 0x1b) && (m_sequence != SEQ_COLUMN) && (m_sequence != SEQ_ROW))
	{
		m_sequence = SEQ_ESC;
		return;
	}

	switch (m_sequence)
	{
	case SEQ_ESC:
		if (data == 'G')
		{
			m_sequence = SEQ_COLUMN;
		}
		else if (data == '[')
		{
			std::fill(std::begin(m_parameters), std::end(m_parameters), 0);
			m_parameter = 0;
			m_sequence = SEQ_CSI;
		}
		else if (data == '#')
		{
			m_sequence = SEQ_SIZE;
		}
		else
		{
			m_sequence = SEQ_NONE;
		}
		return;

	case SEQ_COLUMN:
		m_column = data - 0x20;
		m_sequence = SEQ_ROW;
		return;

	case SEQ_ROW:
		m_x_pos = std::min<unsigned>(m_column, m_width - 1);
		m_y_pos = std::min<unsigned>(u8(data - 0x20), m_height - 1);
		m_sequence = SEQ_NONE;
		return;

	case SEQ_CSI:
		if ((data >= '0') && (data <= '9'))
		{
			m_parameters[m_parameter] = std::min<unsigned>((m_parameters[m_parameter] * 10) + (data - '0'), 999);
		}
		else if (data == ';')
		{
			if ((m_parameter + 1U) < std::size(m_parameters))
				m_parameter++;
			else
				m_sequence = SEQ_SKIP;
		}
		else if ((data >= 0x40) && (data <= 0x7e))
		{
			control_sequence(data);
			m_sequence = SEQ_NONE;
		}
		else
		{
			m_sequence = SEQ_SKIP;
		}
		return;

	case SEQ_SIZE:
		m_sequence = SEQ_NONE;
		return;

	case SEQ_SKIP:
		if ((data >= 0x40) && (data <= 0x7e))
			m_sequence = SEQ_NONE;
		return;
	}

	unsigned const cursor = (m_y_pos * m_width) + m_x_pos;
	if ((data >= 0x20) && (data != 0x7f))
	{
		m_buffer[cursor] = data;
		m_attributes[cursor] = m_attribute;
		if (++m_x_pos >= m_width)
		{
			m_x_pos = 0;
			line_feed();
		}
		return;
	}

	switch (data)
	{
	case 0x07: // bell
		generic_terminal_device::term_write(data);
		break;

	case 0x08: // backspace
		if (m_x_pos)
			m_x_pos--;
		break;

	case 0x09: // horizontal tab
		m_x_pos = std::min<unsigned>((m_x_pos & 0xf8) + 8, m_width - 1);
		break;

	case 0x0a: // line feed
		line_feed();
		if (m_io_term_conf->read() & 0x040)
			m_x_pos = 0;
		break;

	case 0x0c: // form feed
		erase(0, m_width * m_height);
		m_x_pos = 0;
		m_y_pos = 0;
		break;

	case 0x0d: // carriage return
		m_x_pos = 0;
		if (m_io_term_conf->read() & 0x080)
			line_feed();
		break;

	case 0x0e: // erase to end of screen
		erase(cursor, m_width * m_height);
		break;

	case 0x0f: // erase to end of line
		erase(cursor, (m_y_pos + 1) * m_width);
		break;

	case 0x18: // cursor right
		if (m_x_pos < (m_width - 1))
			m_x_pos++;
		break;

	case 0x1a: // cursor up
		if (m_y_pos)
			m_y_pos--;
		break;

	case 0x1d: // cursor home
		m_x_pos = 0;
		m_y_pos = 0;
		break;
	}
}

void secoinsa20_terminal_device::control_sequence(u8 command)
{
	unsigned const n = m_parameters[0];
	unsigned const count = n ? n : 1;
	unsigned const cursor = (m_y_pos * m_width) + m_x_pos;

	switch (command)
	{
	case 'A': // cursor up
		m_y_pos -= std::min<unsigned>(count, m_y_pos);
		break;

	case 'B': // cursor down
		m_y_pos = std::min<unsigned>(m_y_pos + count, m_height - 1);
		break;

	case 'C': // cursor right
		m_x_pos = std::min<unsigned>(m_x_pos + count, m_width - 1);
		break;

	case 'D': // cursor left
		m_x_pos -= std::min<unsigned>(count, m_x_pos);
		break;

	case 'H': // cursor position
	case 'f':
		m_y_pos = std::min<unsigned>(n ? (n - 1) : 0, m_height - 1);
		m_x_pos = std::min<unsigned>(m_parameters[1] ? (m_parameters[1] - 1) : 0, m_width - 1);
		break;

	case 'J': // erase in screen
		if (n == 0)
			erase(cursor, m_width * m_height);
		else if (n == 1)
			erase(0, cursor + 1);
		else if (n == 2)
			erase(0, m_width * m_height);
		break;

	case 'K': // erase in line
		if (n == 0)
			erase(cursor, (m_y_pos + 1) * m_width);
		else if (n == 1)
			erase(m_y_pos * m_width, cursor + 1);
		else if (n == 2)
			erase(m_y_pos * m_width, (m_y_pos + 1) * m_width);
		break;

	case 'm': // attributes
		for (unsigned i = 0; i <= m_parameter; i++)
		{
			switch (m_parameters[i])
			{
			case 0: m_attribute = 0; break;
			case 4: m_attribute |= UNDERLINE; break;
			case 5: m_attribute |= BLINK; break;
			case 7: m_attribute |= REVERSE; break;
			case 24: m_attribute &= ~UNDERLINE; break;
			case 25: m_attribute &= ~BLINK; break;
			case 27: m_attribute &= ~REVERSE; break;
			}
		}
		break;
	}
}

void secoinsa20_terminal_device::erase(unsigned start, unsigned end)
{
	std::fill(m_buffer.get() + start, m_buffer.get() + end, 0x20);
	std::fill(m_attributes + start, m_attributes + end, 0);
}

void secoinsa20_terminal_device::line_feed()
{
	if (m_y_pos < (m_height - 1))
	{
		m_y_pos++;
	}
	else
	{
		unsigned const size = m_width * m_height;
		std::copy(m_buffer.get() + m_width, m_buffer.get() + size, m_buffer.get());
		std::copy(m_attributes + m_width, m_attributes + size, m_attributes);
		erase(size - m_width, size);
	}
}

void secoinsa20_terminal_device::send_key(u8 code)
{
	if (m_key_count == std::size(m_keys))
	{
		logerror("Keyboard buffer full, key %02x lost\n", code);
		return;
	}

	m_keys[(m_key_head + m_key_count) % std::size(m_keys)] = code;
	m_key_count++;
	next_key();
}

void secoinsa20_terminal_device::next_key()
{
	if (!m_rts && m_key_count && is_transmit_register_empty())
	{
		transmit_register_setup(m_keys[m_key_head]);
		m_key_head = (m_key_head + 1) % std::size(m_keys);
		m_key_count--;
	}
}

void secoinsa20_terminal_device::input_rts(int state)
{
	m_rts = bool(state);
	next_key();
}

void secoinsa20_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void secoinsa20_terminal_device::tra_complete()
{
	next_key();
}

void secoinsa20_terminal_device::rcv_complete()
{
	receive_register_extract();
	term_write(get_received_char());
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SECOINSA20_TERMINAL, device_rs232_port_interface, secoinsa20_terminal_device, "secoinsa20_term", "Secoinsa Serie 20 terminal (HLE)")
