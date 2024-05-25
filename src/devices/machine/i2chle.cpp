// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    I2C HLE mix-in
    by R. Belmont

    A mix-in to allow devices to speak I2C without reimplementing
    the protocol themselves.

    If the device returns data over I2C, override read_data.
    If the device is written data over I2C, override write_data.
    If you want the mix-in's logging to show your device name, override
    get_tag() with something that just returns your tag().

    This mix-in will auto-increment the address on repeated reads/writes,
    it's up to your class that consumes this mix-in
*/

#include "emu.h"
#include "i2chle.h"

#define LOG_DATAOUT (1U << 1)

#define VERBOSE (0)

#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

constexpr int STATE_IDLE = 0;
constexpr int STATE_GET_ADDRESS = 1;
constexpr int STATE_GET_SUBADDRESS = 2;
constexpr int STATE_READ_DATA = 3;
constexpr int STATE_WAIT_ACK = 4;
constexpr int STATE_GET_WRITE_DATA = 5;

i2c_hle_interface::i2c_hle_interface(const machine_config &mconfig, device_t &device, u16 address) :
	device_interface(device, "i2chle"),
	write_sda(*this),
	m_address(address)
{
	m_data_offset = 0;
	m_sda = m_scl = 1;
	m_state = m_state_next = STATE_IDLE;
	m_last_address = 0;
	m_just_acked = false;
}

i2c_hle_interface::~i2c_hle_interface()
{
}

void i2c_hle_interface::interface_post_start()
{
	m_latch = m_bit = 0;
	m_state = STATE_IDLE;
	m_sda = m_scl = 1;
	m_last_address = 0;
	m_data_offset = 0;

	device().save_item(NAME(m_latch));
	device().save_item(NAME(m_bit));
	device().save_item(NAME(m_state));
	device().save_item(NAME(m_state_next));
	device().save_item(NAME(m_data_offset));
	device().save_item(NAME(m_just_acked));

	write_sda(1);
}

void i2c_hle_interface::sda_write(int state)
{
	if (m_sda != state)
	{
		m_sda = state & 1;

		if (m_scl)
		{
			if (m_sda)
			{
				LOG("%s: stop\n", get_tag());
				m_state = STATE_IDLE;
				m_last_address = 0;
				m_just_acked = false;
				m_data_offset = 0;
			}
			else
			{
				LOG("%s: start\n", get_tag());
				m_state = STATE_GET_ADDRESS;
				m_bit = 0;
				m_latch = 0;
				m_just_acked = false;
			}
		}
	}
}

void i2c_hle_interface::scl_write(int state)
{
	if (m_scl != state)
	{
		m_scl = state & 1;

		switch (m_state)
		{
			case STATE_IDLE:
				// just ignore everything until a START
				break;

			case STATE_GET_ADDRESS:
			case STATE_GET_SUBADDRESS:
			case STATE_GET_WRITE_DATA:
				if (m_bit < 8)
				{
					if (m_scl)
					{
						m_latch <<= 1;
						m_latch |= m_sda;
						m_bit++;
					}
				}
				else
				{
					if (m_scl)
					{
						m_bit++;
					}
					else
					{
						if (m_bit == 8)
						{
							if (m_state == STATE_GET_ADDRESS)
							{
								LOG("%s: Got address %02x (ours is %02x r/w %d)\n", get_tag(), m_latch >> 1, m_address, m_latch & 1);
								// check if reading
								if (m_latch & 1)
								{
									if ((m_latch >> 1) == m_address)
									{
										LOG("%s: address matches, ACKing\n", get_tag());
										write_sda(0);
										m_bit = 0;
										m_latch = 0;
										m_state_next = STATE_READ_DATA;
										m_state = STATE_WAIT_ACK;
									}
									else
									{
										LOG("%s: address doesn't match, ignoring\n", get_tag());
										m_state = STATE_IDLE;
										write_sda(1);
									}
								}
								else
								{
									if ((m_latch >> 1) == m_address)
									{
										LOGMASKED(LOG_DATAOUT, "%s: write: getting subaddress\n", get_tag());
										m_last_address = m_latch >> 1;
										write_sda(0);
										m_bit = 0;
										m_latch = 0;
										m_state_next = STATE_GET_SUBADDRESS;
										m_state = STATE_WAIT_ACK;
									}
									else
									{
										LOG("%s: address doesn't match, ignoring\n", get_tag());
										m_state = STATE_IDLE;
										write_sda(1);
									}
								}
							}
							else if (m_state == STATE_GET_SUBADDRESS)
							{
								LOGMASKED(LOG_DATAOUT, "%s: subaddress is %02x\n", get_tag(), m_latch);
								m_data_offset = m_latch;
								write_sda(0);
								m_bit = 0;
								m_latch = 0;
								m_state_next = STATE_GET_WRITE_DATA;
								m_state = STATE_WAIT_ACK;
							}
							else if (m_state == STATE_GET_WRITE_DATA)
							{
								LOGMASKED(LOG_DATAOUT, "%s: got write data %02x for address %02x\n", get_tag(), m_latch, m_data_offset);
								write_data(m_data_offset, m_latch);
								m_data_offset++;
								write_sda(0);
								m_bit = 0;
								m_latch = 0;
								m_state_next = STATE_GET_WRITE_DATA;
								m_state = STATE_WAIT_ACK;
							}
						}
					}
				}
				break;

			case STATE_WAIT_ACK:
				if (!m_scl)
				{
					m_state = m_state_next;
					write_sda(1);
				}
				break;

			case STATE_READ_DATA:
				if (m_bit < 8)
				{
					if (!m_scl)
					{
						m_bit++;
						write_sda(1);
					}
					else
					{
						if (m_bit == 0)
						{
							m_latch = read_data(m_data_offset);
							m_data_offset++;
							m_data_offset &= 0xff;
							LOGMASKED(LOG_DATAOUT, "%s: outputting byte %02x\n", get_tag(), m_latch);
						}

						write_sda(BIT(m_latch, 7));
						m_latch <<= 1;
					}
				}
				else
				{
					if (m_scl)
					{
						// did the master ACK or NACK?
						if (m_sda)
						{
							LOGMASKED(LOG_DATAOUT, "%s: master NACK\n", get_tag());
							m_state = STATE_IDLE;
							write_sda(1);
						}
						else
						{
							LOGMASKED(LOG_DATAOUT, "%s: master ACK\n", get_tag());
							m_just_acked = true;
						}
					}
					else
					{
						write_sda(1);
						if (m_just_acked)
						{
							m_bit = 0;
							m_just_acked = false;
						}
					}
				}
				break;
		}
	}
}

u8 i2c_hle_interface::read_data(u16 offset)
{
	return 0xff;
}

void i2c_hle_interface::write_data(u16 offset, u8 data)
{
}

static const char i2chle_name[] = "i2chle";
const char *i2c_hle_interface::get_tag()
{
	return i2chle_name;
}
