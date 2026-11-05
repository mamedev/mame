// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Nordic Semiconductor nRF2401 2.4 GHz transceiver. The serial configuration
    and ShockBurst packet interfaces connect two local radios, including
    channel/address selection, receive buffers and radio settling times.
    The link delivers error-free packets; RF propagation is not simulated.

    TODO:
    - Direct mode and communication with more than one other transceiver.

***************************************************************************/

#include "emu.h"
#include "nrf2401.h"

#include <algorithm>

#define LOG_CONFIG (1U << 1)
#define LOG_PACKET (1U << 2)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NRF2401, nrf2401_device, "nrf2401", "Nordic nRF2401 Transceiver")

nrf2401_device::nrf2401_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NRF2401, tag, owner, clock)
	, m_peer(*this, finder_base::DUMMY_TAG)
	, m_dr_cb(*this)
{
}

void nrf2401_device::device_start()
{
	m_tx_timer = timer_alloc(FUNC(nrf2401_device::transmit_tick), this);
	for (unsigned channel = 0; channel < 2; channel++)
		m_rx_timer[channel] = timer_alloc(FUNC(nrf2401_device::receive_tick), this);

	save_item(NAME(m_config));
	save_item(NAME(m_config_shift));
	save_item(NAME(m_config_bits));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_packet));
	save_item(NAME(m_packet_bits));
	save_item(NAME(m_packet_config));
	save_item(NAME(m_tx_busy));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_position));
	save_item(NAME(m_rx_pending));
	save_item(NAME(m_dr));
	save_item(NAME(m_pwr_up));
	save_item(NAME(m_cs));
	save_item(NAME(m_ce));
	save_item(NAME(m_clock));
	save_item(NAME(m_data));
	save_item(NAME(m_power_ready));
	save_item(NAME(m_rx_ready));
}

void nrf2401_device::device_reset()
{
	std::copy(std::begin(RESET_CONFIG), std::end(RESET_CONFIG), m_config.begin());
	m_config_shift.fill(0);
	m_config_bits = 0;
	m_tx_buffer.fill(0);
	m_tx_bits = 0;
	m_packet.fill(0);
	m_packet_bits = 0;
	std::fill(std::begin(m_packet_config), std::end(m_packet_config), 0);
	m_tx_busy = false;
	m_tx_timer->adjust(attotime::never);
	if (m_peer)
		m_peer->cancel_reception();
	for (unsigned channel = 0; channel < 2; channel++)
	{
		std::fill(std::begin(m_rx_buffer[channel]), std::end(m_rx_buffer[channel]), 0);
		m_rx_bits[channel] = 0;
		m_rx_position[channel] = 0;
		m_rx_pending[channel] = false;
		m_rx_timer[channel]->adjust(attotime::never);
		m_rx_ready[channel] = attotime::never;
		m_dr[channel] = false;
		m_clock[channel] = false;
	}
	m_pwr_up = false;
	m_cs = false;
	m_ce = false;
	m_data = false;
	m_power_ready = attotime::never;
	m_dr_cb[0](0);
	m_dr_cb[1](0);
}

void nrf2401_device::pwr_up_w(int state)
{
	if (m_pwr_up == bool(state))
		return;

	m_pwr_up = bool(state);
	if (m_pwr_up)
		m_power_ready = machine().time() + attotime::from_msec(3);
	else
	{
		m_power_ready = attotime::never;
		m_tx_timer->adjust(attotime::never);
		if (m_tx_busy && m_peer)
			m_peer->cancel_reception();
		m_tx_busy = false;
		m_tx_bits = 0;
	}
	restart_receiver();
}

void nrf2401_device::cs_w(int state)
{
	if (m_cs == bool(state))
		return;

	m_cs = bool(state);
	if (m_cs)
	{
		m_config_shift.fill(0);
		m_config_bits = 0;
	}
	else if (m_pwr_up && m_config_bits && (m_config_bits <= CONFIG_BYTES * 8))
	{
		// Only the low bits written in this transaction replace the configuration.
		unsigned const bytes = m_config_bits / 8;
		unsigned const remainder = m_config_bits & 7;
		std::copy_n(m_config_shift.begin(), bytes, m_config.begin());
		if (remainder)
		{
			u8 const mask = (1U << remainder) - 1;
			m_config[bytes] = (m_config[bytes] & ~mask) | (m_config_shift[bytes] & mask);
		}
		LOGMASKED(LOG_CONFIG, "Config %u bits: channel %u %s, mode %02x, address/CRC %02x, payload %u/%u bits\n",
			m_config_bits, m_config[0] >> 1, BIT(m_config[0], 0) ? "RX" : "TX", m_config[1], m_config[2], m_config[13], m_config[14]);
	}
	restart_receiver();
}

void nrf2401_device::ce_w(int state)
{
	if (m_ce == bool(state))
		return;

	m_ce = bool(state);
	if (m_pwr_up && !m_cs && BIT(m_config[1], 6) && !BIT(m_config[0], 0))
	{
		if (m_ce)
		{
			m_tx_buffer.fill(0);
			m_tx_bits = 0;
		}
		else
			begin_transmit();
	}
	restart_receiver();
}

void nrf2401_device::clock_w(int state)
{
	bool const rising = !m_clock[0] && state;
	if (rising && m_pwr_up)
	{
		if (m_cs)
		{
			if (m_config_bits < CONFIG_BYTES * 8)
			{
				for (unsigned byte = CONFIG_BYTES - 1; byte > 0; byte--)
					m_config_shift[byte] = (m_config_shift[byte] << 1) | (m_config_shift[byte - 1] >> 7);
				m_config_shift[0] = (m_config_shift[0] << 1) | m_data;
			}
			if (m_config_bits <= CONFIG_BYTES * 8)
				m_config_bits++;
		}
		else if (m_ce && BIT(m_config[1], 6) && !BIT(m_config[0], 0) && !m_tx_busy)
		{
			if (m_tx_bits < PACKET_BITS)
				set_packet_bit(m_tx_buffer.data(), m_tx_bits, m_data);
			if (m_tx_bits <= PACKET_BITS)
				m_tx_bits++;
		}
	}
	clock_rx(0, bool(state));
}

void nrf2401_device::clock2_w(int state)
{
	clock_rx(1, bool(state));
}

int nrf2401_device::data_r() const
{
	return m_dr[0] ? packet_bit(m_rx_buffer[0], m_rx_position[0]) : 0;
}

int nrf2401_device::data2_r() const
{
	return m_dr[1] ? packet_bit(m_rx_buffer[1], m_rx_position[1]) : 0;
}

void nrf2401_device::set_packet_bit(u8 *data, unsigned bit, int state)
{
	u8 const mask = 0x80 >> (bit & 7);
	data[bit / 8] = (data[bit / 8] & ~mask) | (state ? mask : 0);
}

void nrf2401_device::set_dr(unsigned channel, bool state)
{
	if (m_dr[channel] != state)
	{
		m_dr[channel] = state;
		m_dr_cb[channel](state);
	}
}

void nrf2401_device::restart_receiver()
{
	cancel_reception();
	for (unsigned channel = 0; channel < 2; channel++)
		m_rx_ready[channel] = receiving() ? std::max(m_power_ready, machine().time() + attotime::from_usec(202)) : attotime::never;
}

void nrf2401_device::cancel_reception()
{
	for (unsigned channel = 0; channel < 2; channel++)
	{
		m_rx_pending[channel] = false;
		m_rx_timer[channel]->adjust(attotime::never);
	}
}

void nrf2401_device::clock_rx(unsigned channel, bool state)
{
	// DATA is valid before the first rising edge and advances on falling edges.
	if (m_clock[channel] && !state && m_pwr_up && !m_cs && m_dr[channel])
	{
		if (++m_rx_position[channel] == m_rx_bits[channel])
		{
			set_dr(channel, false);
			m_rx_ready[channel] = receiving() ? machine().time() + attotime::from_usec(202) : attotime::never;
		}
	}
	m_clock[channel] = state;
}

void nrf2401_device::begin_transmit()
{
	if (m_tx_busy || !m_tx_bits || (m_tx_bits + crc_bits(m_config[2]) > PACKET_BITS) || (machine().time() < m_power_ready))
		return;

	m_packet = m_tx_buffer;
	m_packet_bits = m_tx_bits;
	std::copy_n(m_config.begin(), 3, std::begin(m_packet_config));
	m_tx_busy = true;
	m_tx_timer->adjust(attotime::from_usec(195), 0);
	LOGMASKED(LOG_PACKET, "Transmit %u bits on channel %u\n", m_packet_bits, m_packet_config[0] >> 1);
}

TIMER_CALLBACK_MEMBER(nrf2401_device::transmit_tick)
{
	if (!param)
	{
		// The preamble adds eight bits, and the modulator needs one final bit time.
		attotime const duration = attotime::from_ticks(m_packet_bits + crc_bits(m_packet_config[2]) + 9,
			BIT(m_packet_config[1], 5) ? 1'000'000 : 250'000);
		if (m_peer)
			m_peer->receive_packet(m_packet.data(), m_packet_bits, m_packet_config, duration);
		m_tx_timer->adjust(duration, 1);
	}
	else
	{
		m_tx_busy = false;
		restart_receiver();
	}
}

void nrf2401_device::receive_packet(const u8 *data, unsigned bits, const u8 *config, const attotime &duration)
{
	if (!receiving() || ((m_config[1] ^ config[1]) & 0x20) || (crc_bits(m_config[2]) != crc_bits(config[2])))
		return;

	unsigned const address_bits = m_config[2] >> 2;
	if ((address_bits < 8) || (address_bits > 40))
		return;

	for (unsigned channel = 0; channel < (BIT(m_config[1], 7) ? 2U : 1U); channel++)
	{
		unsigned const payload_bits = m_config[13 + channel];
		if ((m_config[0] >> 1) + channel * 8 != (config[0] >> 1)
			|| m_dr[channel] || m_rx_pending[channel] || (machine().time() < m_rx_ready[channel])
			|| !payload_bits || (bits != address_bits + payload_bits))
			continue;

		bool match = true;
		for (unsigned bit = 0; bit < address_bits; bit++)
		{
			unsigned const address_bit = address_bits - 1 - bit;
			if (packet_bit(data, bit) != BIT(m_config[3 + channel * 5 + address_bit / 8], address_bit & 7))
			{
				match = false;
				break;
			}
		}
		if (!match)
			continue;

		for (unsigned bit = 0; bit < payload_bits; bit++)
			set_packet_bit(m_rx_buffer[channel], bit, packet_bit(data, address_bits + bit));
		m_rx_bits[channel] = payload_bits;
		m_rx_position[channel] = 0;
		m_rx_pending[channel] = true;
		m_rx_timer[channel]->adjust(duration, channel);
	}
}

TIMER_CALLBACK_MEMBER(nrf2401_device::receive_tick)
{
	m_rx_pending[param] = false;
	set_dr(param, true);
	LOGMASKED(LOG_PACKET, "Receive %u payload bits on channel %u\n", m_rx_bits[param], param + 1);
}
