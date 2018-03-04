// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    Siemens SDA5708 8 character 7x5 dot matrix LED display

**********************************************************************

  Main source of information about the SDA5708 came from San Bergmans:
  http://www.sbprojects.com/knowledge/footprints/sda5708/index.php

**********************************************************************/
#include "emu.h"
#include "sda5708.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h
#define LOG_SETUP   (1U <<  1)
#define LOG_CMD     (1U <<  2)
#define LOG_DATA    (1U <<  3)
#define LOG_CNTR    (1U <<  4)

//#define VERBOSE  (LOG_DATA|LOG_SETUP|LOG_CMD|LOG_CNTR|LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

//#define LOG(...)      LOGMASKED(LOG_GENERAL, __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGCMD(...)   LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGDATA(...)  LOGMASKED(LOG_DATA,    __VA_ARGS__)
#define LOGCNTR(...)  LOGMASKED(LOG_CNTR,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
//const device_type SDA5708 = &device_creator<sda5708_device>;
DEFINE_DEVICE_TYPE(SDA5708,         sda5708_device,   "sda5708",         "SDA5708")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sda5708_device - constructor
//-------------------------------------------------

sda5708_device::sda5708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SDA5708, tag, owner, clock)
	, m_dots(*this, "Dot_%u%u%u", 1U, 1U, 1U)
	, m_serial(0)
	, m_load(0)
	, m_reset(0)
	, m_data(0)
	, m_sdclk(0)
	, m_cdp(0)
	, m_digit(0)
	, m_bright(0)
	, m_clear(0)
	, m_ip(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sda5708_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	// look up outputs
	m_dots.resolve();

	// register for state saving
	save_item(NAME(m_serial));
	save_item(NAME(m_load));
	save_item(NAME(m_reset));
	save_item(NAME(m_data));
	save_item(NAME(m_sdclk));
	save_item(NAME(m_cdp));
	save_item(NAME(m_digit));
	save_item(NAME(m_bright));
	save_item(NAME(m_clear));
	save_item(NAME(m_ip));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sda5708_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
	m_ip = 0;
	m_clear = 0;
	m_bright = 0;
	m_digit = 0;
	memset(m_dispmem, 0x00, sizeof(m_dispmem));
	update_display();
}

//-------------------------------------------------
//  load_w - load signal
//
// The Load pin is an active low input used to enable data transfer into the display.
// When Load is low, data is clocked into the 8 bit serial data register. When Load goes
// high, the contents of the 8 bit serial data register are evaluated by the display controller.
// While Load remains high the Data and SDCLK pins may be used to control other serial devices
// on the same bus.
//-------------------------------------------------

void sda5708_device::update_display()
{
	LOG("%s\n", FUNCNAME);
	for (int d = 0; d < 8; d++)
	{
		for (int y = 0; y < 7; y++)
		{
			LOGDATA("- SDA5708 data: ");
			for (int x = 0; x < 5; x++)
			{
				bool const dot = !BIT(m_dispmem[(d * 7) + y], 4 - x);
				m_dots[d][y][x] = dot ? 7 : m_bright;
				LOGDATA("%c", dot ? '-' : 'x');
			}
			LOGDATA("\n");
		}
		LOGDATA("\n");
	}
}

WRITE_LINE_MEMBER( sda5708_device::load_w )
{
	LOG("%s - line %s\n", FUNCNAME, state == ASSERT_LINE ? "asserted" : "cleared");
	if (m_load != state && m_reset == CLEAR_LINE && state == CLEAR_LINE)
	{
		switch (m_serial & SDA5708_REG_MASK)
		{
		case SDA5708_CNTR_COMMAND:
			LOGCMD("- Control command: %02x\n", m_serial);
			m_bright = m_serial & 0x07;
			m_clear  = m_serial & 0x20;
			m_ip     = m_serial & 0x80;
			LOGCNTR("- SDA5708 control command - Clear:%d IP:%d Bright:%d\n", m_clear, m_ip, m_bright);
			break;
		case SDA5708_CLEAR_COMMAND:
			LOGCMD("- Display cleared\n");
			memset(m_dispmem, 0x00, sizeof(m_dispmem));
			update_display();
			m_bright = m_serial & 0x07;
			m_clear  = m_serial & 0x20;
			m_ip     = m_serial & 0x80;
			LOGCNTR("- SDA5708 clear command - Clear:%d IP:%d Bright:%d\n", m_clear, m_ip, m_bright);
			break;
		case SDA5708_ADDR_COMMAND:
			LOGCMD("- Address command: %02x\n", m_serial & 0x1f);
			m_digit = m_serial & 7;
			m_cdp = 0;
			break;
		case SDA5708_DATA_COMMAND:
			LOGCMD("- Data command: %02x\n", m_serial & 0x1f);
			LOGDATA("- SDA5708 data: %c%c%c%c%c\n",
					(m_serial & 0x10) ? (m_clear == 0 ? 'o' : 'x') : ' ',
					(m_serial & 0x08) ? (m_clear == 0 ? 'o' : 'x')  : ' ',
					(m_serial & 0x04) ? (m_clear == 0 ? 'o' : 'x')  : ' ',
					(m_serial & 0x02) ? (m_clear == 0 ? 'o' : 'x')  : ' ',
					(m_serial & 0x01) ? (m_clear == 0 ? 'o' : 'x')  : ' ' );
			m_dispmem[m_digit * 7 + m_cdp] = m_serial;
			if (m_clear != 0)
			{
				m_dots[m_digit][m_cdp][0] = BIT(m_serial, 4) ? m_bright : 7;
				m_dots[m_digit][m_cdp][1] = BIT(m_serial, 3) ? m_bright : 7;
				m_dots[m_digit][m_cdp][2] = BIT(m_serial, 2) ? m_bright : 7;
				m_dots[m_digit][m_cdp][3] = BIT(m_serial, 1) ? m_bright : 7;
				m_dots[m_digit][m_cdp][4] = BIT(m_serial, 0) ? m_bright : 7;
			}
			m_cdp = (m_cdp + 1) % 7;
			break;
		default:
			LOGCMD("- Unknown command:%02x\n", m_serial);
			break;
		}
	}
		m_load = state;
}

//-------------------------------------------------
//  data_w - data line
//
// The Data pin holds the bits to be clocked into the serial data register whenever the SDCLK
// line goes high. The least significant bit D0 is loaded first.
//-------------------------------------------------

WRITE_LINE_MEMBER( sda5708_device::data_w )
{
	LOG("%s - line %s\n", FUNCNAME, state == ASSERT_LINE ? "asserted" : "cleared");
	m_data = state;
}


//-------------------------------------------------
//  sdclk_w - serial data clock
//
// SDCLK is the serial clock line. Data is accepted by the display's serial data register when the SDCLK line
// goes high. The Load pin must be low for the serial data register to accept any data. The minimum clock period is
// 200ns. Setup time, the time between a stable Data line and a rising SDCLK signal, should be a minimum of 50ns.
//-------------------------------------------------

WRITE_LINE_MEMBER( sda5708_device::sdclk_w )
{
	LOG("%s - line %s\n", FUNCNAME, state == ASSERT_LINE ? "asserted" : "cleared");

	if ( m_sdclk != state && m_reset == CLEAR_LINE && m_load == ASSERT_LINE && state == ASSERT_LINE)
	{
		m_serial = (((m_serial >> 1) & 0x7f) | (m_data == ASSERT_LINE ? 0x80 : 0));
	}
	m_sdclk = state;
}


//-------------------------------------------------
//  reset_w - chip reset
//
// When the Reset pin is held low, the display will be reset. The multiplex counter, the address register,
// the control word and the display bit patterns are all cleared. This means that the display will be
// blank and the display is set to 100% brightness and maximum peak current.
// During normal operation the Reset pin is only made low for a short period when power is applied to
// the circuit and is left at high level from then on.
//-------------------------------------------------

WRITE_LINE_MEMBER( sda5708_device::reset_w )
{
	LOG("%s - line %s\n", FUNCNAME, state == ASSERT_LINE ? "asserted" : "cleared");
	m_reset = state;
	if (state == ASSERT_LINE)
		device_reset();
}
