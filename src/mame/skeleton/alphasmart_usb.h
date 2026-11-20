// license:BSD-3-Clause
// copyright-holders:Kirk
// PDIUSBD11 packet engine and I2C slave. No host USB library is required.
#ifndef MAME_SKELETON_ALPHASMART_USB_H
#define MAME_SKELETON_ALPHASMART_USB_H

#include <array>
#include <algorithm>
#include <cstdint>
#include <functional>

class alphasmart_usb_chip
{
public:
	struct endpoint
	{
		std::array<uint8_t, 10> data{};
		uint8_t status = 0;
		bool full = false, stalled = false, setup = false, acknowledged = true, toggle = false;
	};
	std::array<endpoint, 14> ep{};
	uint8_t address = 0, enable = 0, mode = 0x8d, clock = 11;
	bool vbus = false;
	uint16_t interrupts = 0;
	uint16_t frame = 0;
	std::function<void(char, uint8_t)> trace;
	bool connected() const { return vbus && (mode & 0x10); }
	int irq_r() const { return interrupts == 0; }
	int sda_r() const { return m_sda && m_slave_sda; }
	void reset(bool bus = false)
	{
		ep = {}; address = enable = 0;
		frame = 0;
		if (bus) mode |= 1;
		else
		{
			mode = 0x8d; clock = 11;
			m_state = idle; m_scl = m_sda = m_slave_sda = true;
			m_address = true; m_bits = m_shift = 0;
		}
		interrupts = bus ? 0x4000 : 0;
		m_selected = m_command = m_pointer = m_phase = 0;
	}
	void command(uint8_t value)
	{
		if (trace) trace('C', value);
		m_command = value; m_phase = 0;
		if (value < ep.size()) { m_selected = value; m_pointer = 0; }
		auto &e = ep[m_selected];
		if (value == 0xf1) e.acknowledged = true;
		if (value == 0xf2 && e.acknowledged) { e.full = false; e.setup = false; }
		if (value == 0xfa && e.acknowledged) e.full = true;
	}
	void write(uint8_t value)
	{
		if (trace) trace('W', value);
		if (m_command == 0xd1) address = value;
		else if (m_command == 0xd8) enable = value;
		else if (m_command == 0xf3) { if (!m_phase++) mode = value; else clock = value; }
		else if (m_command == 0xf0) { if (m_pointer < 10) ep[m_selected].data[m_pointer++] = value; }
		else if (m_command >= 0x40 && m_command < 0x40 + ep.size())
		{
			auto &e = ep[m_command - 0x40];
			e.stalled = value & 1;
			if (!e.stalled) { e.full = e.toggle = false; }
		}
	}
	uint8_t read()
	{
		if (m_command < ep.size()) return ep[m_selected].full;
		if (m_command == 0xf0) return m_pointer < 10 ? ep[m_selected].data[m_pointer++] : 0;
		if (m_command == 0xf5) return frame >> (8 * (m_phase++ & 1));
		if (m_command == 0xf4)
		{
			uint8_t const value = interrupts >> (8 * (m_phase++ & 1));
			if (m_phase == 2) interrupts &= ~0x4000;
			return value;
		}
		if (m_command >= 0x40 && m_command < 0x40 + ep.size())
		{
			unsigned const index = m_command - 0x40;
			uint8_t const status = ep[index].status;
			ep[index].status = 0; interrupts &= ~(1U << index);
			return status;
		}
		if (m_command >= 0x80 && m_command < 0x80 + ep.size())
		{
			auto const &e = ep[m_command - 0x80];
			return (e.full ? 0x20 : 0) | (e.toggle ? 0x10 : 0) | (e.stalled ? 8 : 0) | (e.setup ? 4 : 0);
		}
		return 0;
	}
	// Endpoint indices 4/5 have the opposite direction to 6/7 and 8/9.
	static unsigned index(unsigned number, bool in) { return number == 0 ? 2 + in : number == 1 ? 5 - in : 2 + number * 2 + in; }
	// Packet status: ACK=0, NAK=1, STALL=2, disconnected/invalid=3.
	unsigned packet(unsigned number, bool in, bool setup, uint8_t *data, unsigned &length)
	{
		if (!connected() || number > 3 || length > 8 || !(address & 0x80)) return 3;
		if (number && !(enable & 2)) return 1;
		unsigned const i = index(number, in);
		auto &e = ep[i];
		if (setup)
		{
			if (number || in || length != 8) return 3;
			for (unsigned j : {2U, 3U}) { ep[j].full = ep[j].stalled = false; ep[j].acknowledged = false; ep[j].toggle = true; }
		}
		else if (e.stalled) return 2;
		if (in)
		{
			if (!e.full) return 1;
			length = std::min<unsigned>(length, std::min<unsigned>(8, e.data[1]));
			std::copy_n(e.data.begin() + 2, length, data);
			e.full = false;
		}
		else
		{
			if (e.full) return 1;
			e.data[0] = 0; e.data[1] = length;
			std::copy_n(data, length, e.data.begin() + 2);
			e.full = true; e.setup = setup;
		}
		e.status = 1 | (setup ? 0x20 : 0) | (e.toggle && !setup ? 0x40 : 0) | (e.status ? 0x80 : 0);
		if (!setup) e.toggle = !e.toggle;
		interrupts |= 1U << i;
		return 0;
	}
	// Open-drain bus levels, including GPIO direction changes that release a pin.
	void lines(bool scl, bool sda)
	{
		if (scl && m_scl && sda != m_sda)
		{
			m_state = sda ? idle : receive; m_address = true; m_bits = m_shift = 0; m_slave_sda = true;
		}
		else if (scl != m_scl)
		{
			if (scl) rising(sda); else falling();
		}
		m_scl = scl; m_sda = sda;
	}

private:
	enum { idle, receive, ack, transmit, master_ack } m_state = idle;
	bool m_scl = true, m_sda = true, m_slave_sda = true, m_address = true, m_reading = false, m_command_address = false, m_match = false;
	unsigned m_bits = 0;
	uint8_t m_shift = 0, m_selected = 0, m_command = 0, m_pointer = 0, m_phase = 0;
	void rising(bool sda)
	{
		if (m_state == receive)
		{
			m_shift = (m_shift << 1) | sda;
			if (++m_bits == 8)
			{
				if (m_address)
				{
					m_match = m_shift == 0x34 || m_shift == 0x35 || m_shift == 0x36;
					m_reading = m_shift & 1; m_command_address = m_shift == 0x36; m_address = false;
				}
				else if (m_match) { if (m_command_address) command(m_shift); else write(m_shift); }
				m_state = ack; m_bits = 0;
			}
		}
		else if (m_state == ack) m_bits = 1;
		else if (m_state == transmit) ++m_bits;
		else if (m_state == master_ack) { m_match = !sda; m_bits = 1; }
	}
	void falling()
	{
		if (m_state == ack)
		{
			if (!m_bits) m_slave_sda = !m_match;
			else
			{
				m_bits = m_shift = 0;
				m_state = !m_match ? idle : m_reading ? transmit : receive;
				m_slave_sda = true;
				if (m_state == transmit) { m_shift = read(); m_slave_sda = bool(m_shift & 0x80); }
			}
		}
		else if (m_state == transmit)
		{
			if (m_bits == 8) { m_state = master_ack; m_bits = 0; m_slave_sda = true; }
			else m_slave_sda = bool(m_shift & (0x80 >> m_bits));
		}
		else if (m_state == master_ack && m_bits)
		{
			m_bits = 0; m_state = m_match ? transmit : idle;
			if (m_match) { m_shift = read(); m_slave_sda = bool(m_shift & 0x80); }
		}
	}
};

#endif
