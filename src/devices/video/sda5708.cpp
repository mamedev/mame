// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    Siemens SDA5708 8 character 7x5 dot matrix LED display

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

//#define VERBOSE  (LOG_DATA|LOG_SETUP|LOG_CMD|LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

//#define LOG(...)      LOGMASKED(LOG_GENERAL, __VA_ARGS__) // Already defined in logmacro.h 
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGCMD(...)   LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGDATA(...)  LOGMASKED(LOG_DATA,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64d"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
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

sda5708_device::sda5708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SDA5708, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sda5708_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	// register for state saving
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sda5708_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
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

WRITE_LINE_MEMBER( sda5708_device::load_w )
{
	LOG("%s - line %s\n", FUNCNAME, state == ASSERT_LINE ? "asserted" : "cleared");
	if (m_load != state && m_reset == CLEAR_LINE && state == CLEAR_LINE)
	{
	  	switch (m_serial & SDA5708_REG_MASK)
		{
		case SDA5708_CNTR_COMMAND:
			LOGCMD("- Control command: %0x02\n", m_serial & 0x1f);
			break;
		case SDA5708_CLEAR_COMMAND:
			LOGCMD("- Display cleared\n");
			memset(m_dispmem, 0x00, sizeof(m_dispmem));
			break;
		case SDA5708_ADDR_COMMAND:
			LOGCMD("- Address command: %02x\n", m_serial & 0x1f);
			m_digit = m_serial & 7;
			m_cdp = 0;
			break;
		case SDA5708_DATA_COMMAND:
			LOGCMD("- Data command: %02x\n", m_serial & 0x1f);
			LOGDATA("- SDA5708 data: %c%c%c%c%c\n",
				(m_serial & 0x10) ? 'x' : ' ',
				(m_serial & 0x08) ? 'x' : ' ',
				(m_serial & 0x04) ? 'x' : ' ',
				(m_serial & 0x02) ? 'x' : ' ',
				(m_serial & 0x01) ? 'x' : ' ' );
			m_dispmem[m_digit * 7 + m_cdp] = m_serial;
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
}
