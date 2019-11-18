// license:GPL-2.0+
// copyright-holders:Dirk Best
/******************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)

    TODO: Strobed modes
    TODO: Check the ODRx register for where to get pin values, _cb() vs m_out_x

*******************************************************************************/

#include "emu.h"
#include "ins8154.h"

#define LOG_BITS   (1U <<  1)
//#define VERBOSE (LOG_BITS) // (LOG_GENERAL|LOG_BITS)
#include "logmacro.h"

#define LOGBITS(...)   LOGMASKED(LOG_BITS,   __VA_ARGS__)

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Mode Definition Register */
enum
{
	MDR_BASIC                 = 0x00,
	MDR_STROBED_INPUT         = 0x20,
	MDR_STROBED_OUTPUT        = 0x60,
	MDR_STROBED_OUTPUT_3STATE = 0xe0
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(INS8154, ins8154_device, "ins8154", "INS8154 RAM I/O")

//-------------------------------------------------
//  ins8154_device - constructor
//-------------------------------------------------

ins8154_device::ins8154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INS8154, tag, owner, clock)
	, m_in_a_cb(*this)
	, m_out_a_cb(*this)
	, m_in_b_cb(*this)
	, m_out_b_cb(*this)
	, m_out_irq_cb(*this)
	, m_in_a(0), m_in_b(0), m_out_a(0), m_out_b(0), m_mdr(0), m_odra(0), m_odrb(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ins8154_device::device_start()
{
	/* resolve callbacks */
	m_in_a_cb.resolve();
	m_out_a_cb.resolve_safe();
	m_in_b_cb.resolve();
	m_out_b_cb.resolve_safe();
	m_out_irq_cb.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_in_a));
	save_item(NAME(m_in_b));
	save_item(NAME(m_out_a));
	save_item(NAME(m_out_b));
	save_item(NAME(m_mdr));
	save_item(NAME(m_odra));
	save_item(NAME(m_odrb));
	save_item(NAME(m_ram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ins8154_device::device_reset()
{
	m_in_a = 0;
	m_in_b = 0;
	m_out_a = 0;
	m_out_b = 0;
	m_mdr = 0;
	m_odra = 0;
	m_odrb = 0;
}


uint8_t ins8154_device::read_io(offs_t offset)
{
	uint8_t val = 0xff;

	if (offset > 0x24)
	{
		LOG("%s: INS8154 Read from invalid offset %02x!\n", machine().describe_context(), offset);
		return 0xff;
	}

	switch (offset)
	{
	case 0x20:
		if (!m_in_a_cb.isnull())
			val = m_in_a_cb(0);
		m_in_a = val;
		break;

	case 0x21:
		if (!m_in_b_cb.isnull())
			val = m_in_b_cb(0);
		m_in_b = val;
		break;

	default:
		val = 0;
		if (offset < 0x08) // Read a bit in Port A
		{
			if (!m_in_a_cb.isnull())
			{
				//val = (m_in_a_cb(0) << (8 - offset)) & 0x80;
				val = (m_in_a_cb(0) & ~m_odra & (1 << (offset & 0x07))) ? 0x80 : 0x00;
			}
			LOGBITS("%s: INS8154 Port A read bit %02x: %02x\n", machine().describe_context(), offset & 0x07, val);
		}
		else  // Read a bit in Port B
		{
			if (!m_in_b_cb.isnull())
			{
				val = (m_in_b_cb(0) & ~m_odrb & (1 << (offset & 0x07))) ? 0x80 : 0x00;
			}
			LOGBITS("%s: INS8154 Port B read bit %02x: %02x\n", machine().describe_context(), offset & 0x07, val);
		}
		break;
	}

	return val;
}

uint8_t ins8154_device::read_ram(offs_t offset)
{
	return m_ram[offset & 0x7f];
}

void ins8154_device::porta_w(uint8_t data)
{
	m_out_a = data;

	/* Test if any pins are set as outputs */
	if (m_odra)
		m_out_a_cb(offs_t(0), (data & m_odra) | (m_odra ^ 0xff));
}

void ins8154_device::portb_w(uint8_t data)
{
	LOG("%s: INS8154 Write PortB %02x with odrb: %02x\n", machine().describe_context(), data, m_odrb);
	m_out_b = data;

	/* Test if any pins are set as outputs */
	if (m_odrb)
		m_out_b_cb(offs_t(0), (data & m_odrb) | (m_odrb ^ 0xff));
}

void ins8154_device::write_io(offs_t offset, uint8_t data)
{
	if (offset > 0x24)
	{
		LOG("%s: INS8154 Write %02x to invalid offset %02x!\n", machine().describe_context(), data, offset);
		return;
	}

	switch (offset)
	{
	case 0x20:
		porta_w(data);
		break;

	case 0x21:
		portb_w(data);
		break;

	case 0x22:
		LOG("%s: INS8154 ODRA set to %02x\n", machine().describe_context(), data);
		m_odra = data;
		break;

	case 0x23:
		LOG("%s: INS8154 ODRB set to %02x\n", machine().describe_context(), data);
		m_odrb = data;
		break;

	case 0x24:
		LOG("%s: INS8154 MDR set to %02x\n", machine().describe_context(), data);
		m_mdr = data;
		break;

	default:
		if (offset & 0x10)
		{
			/* Set bit */
			if (offset < 0x08)
			{
				LOGBITS("%s: INS8154 Port A set bit %02x\n", machine().describe_context(), offset & 0x07);
				porta_w(m_out_a |= (1 << (offset & 0x07)));
			}
			else
			{
				LOGBITS("%s: INS8154 Port B set bit %02x\n", machine().describe_context(), offset & 0x07);
				portb_w(m_out_b |= (1 << (offset & 0x07)));
			}
		}
		else
		{
			/* Clear bit */
			if (offset < 0x08)
			{
				LOGBITS("%s: INS8154 Port A clear bit %02x\n", machine().describe_context(), offset & 0x07);
				porta_w(m_out_a & ~(1 << (offset & 0x07)));
			}
			else
			{
				LOGBITS("%s: INS8154 Port B clear bit %02x\n", machine().describe_context(), offset & 0x07);
				portb_w(m_out_b & ~(1 << (offset & 0x07)));
			}
		}
		break;
	}
}

void ins8154_device::write_ram(offs_t offset, uint8_t data)
{
	m_ram[offset & 0x7f] = data;
}
