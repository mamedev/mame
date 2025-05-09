// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    9h0-0008_card_reader.cpp

    Sega Toys 9H0-0008 barcode card reader

*******************************************************************************/

#include "emu.h"

#include "9h0-0008_card_reader.h"
#include "9h0-0008_iox.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(BEENA_CARD_READER, sega_beena_card_reader_device, "sega_beena_card_reader", "Sega Beena Card Reader")
DEFINE_DEVICE_TYPE(BEENA_CARD_READER_2061, sega_beena_card_reader_2061_device, "sega_beena_card_reader_2061", "Sega Beena Card Reader RD2061")
DEFINE_DEVICE_TYPE(TVOCHKEN_CARD_READER, sega_tvochken_card_reader_device, "sega_tvochken_card_reader", "Sega TV Ocha-ken Card Reader")


static INPUT_PORTS_START( sega_beena_card_reader )
	PORT_START("IO")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("Scan Card") PORT_WRITE_LINE_MEMBER(FUNC(sega_beena_card_reader_device::scan_card))
INPUT_PORTS_END

ioport_constructor sega_beena_card_reader_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sega_beena_card_reader );
}


sega_beena_card_reader_device::sega_beena_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega_beena_card_reader_device(mconfig, BEENA_CARD_READER, tag, owner, clock, true, 12, 32)
{
}

sega_beena_card_reader_device::sega_beena_card_reader_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock,
		bool is_sync_cycle,
		u8 num_bits,
		u8 num_hold)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sega_9h0_0008_iox_slot_interface(mconfig, *this)
	, m_io(*this, "IO")
	, m_is_sync_cycle(is_sync_cycle)
	, m_num_bits(num_bits)
	, m_num_hold(num_hold)
{
}

void sega_beena_card_reader_device::device_start()
{
	save_item(NAME(m_card_data));
	save_item(NAME(m_card_data_i));
	save_item(NAME(m_card_hold_i));
	save_item(NAME(m_card_state));
	save_item(NAME(m_card_status));

	save_item(NAME(m_is_sync_cycle));
	save_item(NAME(m_num_bits));
	save_item(NAME(m_num_hold));
}

void sega_beena_card_reader_device::device_reset()
{
	m_card_data = 0;
	m_card_data_i = 0;
	m_card_hold_i = 0;
	m_card_state = IDLE;
	m_card_status = 0;
}

u32 sega_beena_card_reader_device::read(bool is_enabled)
{
	if (machine().side_effects_disabled() || !is_enabled) {
		return 0x18;
	}

	if (m_card_state == START_WRITE_DATA) {
		LOG("start write card: hold %d\n", m_card_hold_i);
		m_card_hold_i--;
		if (m_card_hold_i == 0) {
			m_card_hold_i = m_num_hold;
			m_card_data_i = 0;
			m_card_status = 0;
			m_card_state = WRITE_DATA;
		}

		return 0x18 | (1 << 5);
	}

	if (m_card_state == WRITE_DATA) {
		u8 data_bit = (m_card_data >> (m_num_bits - 1 - m_card_data_i)) & 1;
		LOG("write card: bit %d -> %d (sync %d)\n", m_card_data_i, data_bit, m_card_status);

		m_card_hold_i--;
		if (m_card_hold_i == 0) {
			m_card_hold_i = m_num_hold;
			m_card_status ^= 1;

			// If barcodes stripes have spacing between them, each read bit takes 2 sync bits to be parsed.
			if (m_is_sync_cycle) {
				if (m_card_status == 0) {
					m_card_data_i++;
				}
			} else {
				m_card_data_i++;
			}

			if (m_card_data_i == m_num_bits) {
				m_card_data_i = 0;
				m_card_status = 0;
				m_card_state = IDLE;
			}
		}

		return 0x18 | (m_card_status << 6) | (data_bit << 5);
	}

	return 0x18;
}

void sega_beena_card_reader_device::scan_card(int state)
{
	if (state && (m_card_state == IDLE)) {
		m_card_data = m_port->data();
		m_card_hold_i = 10;
		m_card_state = START_WRITE_DATA;
		LOG("scanning card: %04x\n", m_card_data);
	}
}


sega_beena_card_reader_2061_device::sega_beena_card_reader_2061_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega_beena_card_reader_device(mconfig, BEENA_CARD_READER_2061, tag, owner, clock, true, 12, 5)
{
}


sega_tvochken_card_reader_device::sega_tvochken_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega_beena_card_reader_device(mconfig, TVOCHKEN_CARD_READER, tag, owner, clock, false, 16, 32)
{
}
