// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    DIMM serial presence detect (SPD) readback device
    by R. Belmont

    Each DIMM contains a small EEPROM with information about the capacity and
    timings of the module.  The EEPROM speaks a version of I2C called SMBus.

    This does not attempt to be a generalized I2C/SMBus solution.
*/


#include "emu.h"
#include "dimm_spd.h"

#define LOG_DATAOUT (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DIMM_SPD, dimm_spd_device, "dimm_spd", "DIMM Serial Presence Detect")

constexpr int STATE_IDLE = 0;
constexpr int STATE_GET_ADDRESS = 1;
constexpr int STATE_GET_SUBADDRESS = 2;
constexpr int STATE_READ_DATA = 3;
constexpr int STATE_WAIT_ACK = 4;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dimm_spd_device - constructor
//-------------------------------------------------

dimm_spd_device::dimm_spd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DIMM_SPD, tag, owner, clock),
	write_sda(*this)
{
	m_data_offset = 0;
	m_sda = m_scl = 1;
	m_state = m_state_next = STATE_IDLE;
	m_last_address = 0;
	m_just_acked = false;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dimm_spd_device::device_start()
{
	std::fill(std::begin(m_data), std::end(m_data), 0);

	m_data[0] = 128;    // # of bytes in EEPROM
	m_data[1] = 8;      // log2 of EEPROM size
	m_data[2] = 4;      // 4 is SDRAM, 7 is DDR SDRAM, 8 is DDR2, 11 is DDR3, 12 is DDR4
	m_data[3] = 12;     // # of rows
	m_data[4] = 8;      // # of columns
	m_data[5] = 1;      // # of banks (12/8/1 = 32 MiB)
	m_data[6] = 64;     // data bus width low byte
	m_data[7] = 0;      // data bus width high byte
	m_data[11] = 0;     // non-ECC (1=parity, 2=ECC)
	m_data[62] = 0x12;  // SPD version 1.2

	m_latch = m_bit = 0;
	m_state = STATE_IDLE;
	m_sda = m_scl = 1;
	m_last_address = 0;
	m_data_offset = 0;

	save_item(NAME(m_latch));
	save_item(NAME(m_bit));
	save_item(NAME(m_state));
	save_item(NAME(m_state_next));
	save_item(NAME(m_data_offset));
	save_item(NAME(m_just_acked));

	write_sda(1);
}

void dimm_spd_device::set_dimm_size(dimm_size_t size)
{
	m_size = size;

	switch (size)
	{
		case SIZE_4_MIB:
			m_data[3] = 12; // # of rows
			m_data[4] = 5;  // # of columns
			m_data[5] = 1;  // # of banks
			break;

		case SIZE_8_MIB:
			m_data[3] = 12;
			m_data[4] = 6;
			m_data[5] = 1;
			break;

		case SIZE_16_MIB:
			m_data[3] = 12;
			m_data[4] = 7;
			m_data[5] = 1;
			break;

		case SIZE_32_MIB:
			m_data[3] = 12;
			m_data[4] = 8;
			m_data[5] = 1;
			break;

		case SIZE_64_MIB:
			m_data[3] = 12;
			m_data[4] = 9;
			m_data[5] = 1;
			break;

		case SIZE_128_MIB:
			m_data[3] = 12;
			m_data[4] = 10;
			m_data[5] = 1;
			break;

		case SIZE_256_MIB:
			m_data[3] = 12;
			m_data[4] = 10;
			m_data[5] = 2;
			break;

		case SIZE_SLOT_EMPTY:
			break;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dimm_spd_device::device_reset()
{
}

void dimm_spd_device::sda_write(int state)
{
	if (m_size == SIZE_SLOT_EMPTY)
	{
		return;
	}

	if (m_sda != state)
	{
		m_sda = state & 1;

		if (m_scl)
		{
			if (m_sda)
			{
				LOG("%s: stop\n", tag());
				m_state = STATE_IDLE;
				m_last_address = 0;
				m_just_acked = false;
				m_data_offset = 0;
			}
			else
			{
				LOG("%s: start\n", tag());
				m_state = STATE_GET_ADDRESS;
				m_bit = 0;
				m_latch = 0;
				m_just_acked = false;
			}
		}
	}
}

void dimm_spd_device::scl_write(int state)
{
	if (m_size == SIZE_SLOT_EMPTY)
	{
		return;
	}

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
								LOG("%s: Got address %02x (ours is %02x r/w %d)\n", tag(), m_latch >> 1, m_address, m_latch & 1);
								// check if reading
								if (m_latch & 1)
								{
									if ((m_latch >> 1) == m_address)
									{
										LOG("%s: address matches, ACKing\n", tag());
										write_sda(0);
										m_bit = 0;
										m_latch = 0;
										m_state_next = STATE_READ_DATA;
										m_state = STATE_WAIT_ACK;
									}
									else
									{
										LOG("%s: address doesn't match, ignoring\n", tag());
										m_state = STATE_IDLE;
										write_sda(1);
									}
								}
								else
								{
									LOGMASKED(LOG_DATAOUT, "%s: write: getting subaddress\n", tag());
									m_last_address = m_latch >> 1;
									write_sda(0);
									m_bit = 0;
									m_latch = 0;
									m_state_next = STATE_GET_SUBADDRESS;
									m_state = STATE_WAIT_ACK;
								}
							}
							else if (m_state == STATE_GET_SUBADDRESS)
							{
								LOGMASKED(LOG_DATAOUT, "%s: subaddress is %02x\n", tag(), m_latch);
								m_data_offset = m_latch;
								write_sda(0);
								m_bit = 0;
								m_latch = 0;
								m_state_next = STATE_IDLE;  // is this correct?
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
							m_latch = m_data[m_data_offset++];
							m_data_offset &= 0xff;
							LOGMASKED(LOG_DATAOUT, "%s: outputting byte %02x\n", tag(), m_latch);
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
							LOGMASKED(LOG_DATAOUT, "%s: master NACK\n", tag());
							m_state = STATE_IDLE;
							write_sda(1);
						}
						else
						{
							LOGMASKED(LOG_DATAOUT, "%s: master ACK\n", tag());
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
