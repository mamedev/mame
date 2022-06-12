// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Acorn Battery Management Unit

    The BMU is actually a 4-bit Hitachi HD404304F MCU, marked as
    BMU 0290032.

    TODO:
    - configurable status
    - callback when battery low

*********************************************************************/

#include "emu.h"
#include "acorn_bmu.h"

#define LOG_DATA (1 << 1)
#define LOG_LINE (1 << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ACORN_BMU, acorn_bmu_device, "acorn_bmu", "Acorn Battery Management Unit")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_bmu_device - constructor
//-------------------------------------------------

acorn_bmu_device::acorn_bmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_BMU, tag, owner, clock)
	, m_slave_address(BMU_SLAVE_ADDRESS)
	, m_scl(0)
	, m_sdaw(0)
	, m_sdar(1)
	, m_state(STATE_IDLE)
	, m_bits(0)
	, m_shift(0)
	, m_devsel(0)
	, m_register(0)
	, m_estimate(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_bmu_device::device_start()
{
	save_item(NAME(m_scl));
	save_item(NAME(m_sdaw));
	save_item(NAME(m_sdar));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
	save_item(NAME(m_devsel));
	save_item(NAME(m_register));
	save_item(NAME(m_slave_address));
	save_item(NAME(m_estimate));
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER(acorn_bmu_device::scl_w)
{
	if (m_scl != state)
	{
		m_scl = state;
		LOGMASKED(LOG_LINE, "set_scl_line %d\n", m_scl);

		switch (m_state)
		{
		case STATE_DEVSEL:
		case STATE_REGISTER:
		case STATE_DATAIN:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					m_shift = ((m_shift << 1) | m_sdaw) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if (m_scl)
				{
					m_bits++;
				}
				else
				{
					if (m_bits == 8)
					{
						switch (m_state)
						{
						case STATE_DEVSEL:
							m_devsel = m_shift;

							if ((m_devsel & 0xfe) != m_slave_address)
							{
								LOGMASKED(LOG_DATA, "devsel %02x: not this device\n", m_devsel);
								m_state = STATE_IDLE;
							}
							else if ((m_devsel & 1) == 0)
							{
								LOGMASKED(LOG_DATA, "devsel %02x: write\n", m_devsel);
								m_state = STATE_REGISTER;
							}
							else
							{
								LOGMASKED(LOG_DATA, "devsel %02x: read\n", m_devsel);
								m_state = STATE_READSELACK;
							}
							break;

						case STATE_REGISTER:
							m_register = m_shift;

							LOGMASKED(LOG_DATA, "register %02x\n", m_register);

							m_state = STATE_DATAIN;
							break;

						case STATE_DATAIN:
							LOGMASKED(LOG_DATA, "register[ %02x ] <- %02x\n", m_register, m_shift);

							switch (m_register)
							{
							case BMU_CHARGE_ESTIMATE:
								m_estimate = m_shift;
								break;
							}

							m_register++;
							break;
						}

						if( m_state != STATE_IDLE )
						{
							m_sdar = 0 ;
						}
					}
					else
					{
						m_bits = 0;
						m_sdar = 1;
					}
				}
			}
			break;

		case STATE_READSELACK:
			m_bits = 0;
			m_state = STATE_DATAOUT;
			break;

		case STATE_DATAOUT:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					m_bits++;
				}
				else
				{
					if (m_bits == 0)
					{
						switch (m_register)
						{
						case BMU_VERSION:
							m_shift = 0x03;
							break;

						case BMU_STATUS:
							m_shift = STATUS_CHARGER_PRESENT | STATUS_BATTERY_PRESENT | STATUS_CHARGE_STATE_KNOWN | STATUS_LID_OPEN;
							break;

						case BMU_CAPACITY_USED:
							m_shift = 0x40;
							break;

						case BMU_CHARGE_ESTIMATE:
							m_shift = m_estimate;
							break;

						case BMU_COMMAND:
							m_shift = 0;
							break;
						}

						LOGMASKED(LOG_DATA, "register[ %02x ] -> %02x\n", m_register, m_shift);
						m_register++;
					}

					m_sdar = (m_shift >> 7) & 1;

					m_shift = (m_shift << 1) & 0xff;
				}
			}
			else
			{
				if (m_scl)
				{
					if (m_sdaw)
					{
						LOGMASKED(LOG_DATA, "nack\n");
						m_state = STATE_IDLE;
					}

					m_bits = 0;
				}
				else
				{
					m_sdar = 1;
				}
			}
			break;
		}
	}
}

WRITE_LINE_MEMBER(acorn_bmu_device::sda_w)
{
	state &= 1;
	if (m_sdaw != state)
	{
		LOGMASKED(LOG_LINE, "set sda %d\n", state);
		m_sdaw = state;

		if (m_scl)
		{
			if (m_sdaw)
			{
				LOGMASKED(LOG_DATA, "stop\n");
				m_state = STATE_IDLE;
			}
			else
			{
				LOGMASKED(LOG_DATA, "start\n");
				m_state = STATE_DEVSEL;
				m_bits = 0;
			}

			m_sdar = 1;
		}
	}
}

READ_LINE_MEMBER(acorn_bmu_device::sda_r)
{
	int res = m_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}
