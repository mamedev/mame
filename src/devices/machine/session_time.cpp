// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    session_time.cpp

    Implementation of Session Time

**********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"
#include "session_time.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SESSION_TIME, session_time_device, "session_time", "Session Time Device")


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

INPUT_PORTS_START(session_time)
	PORT_START("SESSION_CNF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset Session Time") PORT_CODE(KEYCODE_5_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, session_time_device, reset_session_time, 0)
INPUT_PORTS_END


ioport_constructor session_time_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(session_time);
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void session_time_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

session_time_device::session_time_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock)
{
}

session_time_device::session_time_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		session_time_device(mconfig, SESSION_TIME, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void session_time_device::device_start()
{
	time(&m_session_time);  // initialize session time
	initprintername();
	page_num = 1;  // reset page number
}

void session_time_device::device_reset_after_children()
{
}

void session_time_device::device_reset()
{
}

//-------------------------------------------------
//  Input Changed Member to reset session time
//-------------------------------------------------

INPUT_CHANGED_MEMBER(session_time_device::reset_session_time)
{
	device_start();
	machine().popmessage("Reset Session Time = " + getprintername());
}

//-------------------------------------------------
//  fixchar : replace specific character in string
//-------------------------------------------------

std::string session_time_device::fixchar(std::string in, char from, char to)
{
	std::string final;
	for(std::string::const_iterator it = in.begin(); it != in.end(); ++it)
	{
		if((*it) != from)
		{
			final += *it;
		}
		else final += to;
	}
	return final;
}

//-------------------------------------------------
//  fixcolons : replace colons with dashes
//-------------------------------------------------

std::string session_time_device::fixcolons(std::string in)
{
	return fixchar(in, ':', '-');
}

//-------------------------------------------------
//  sessiontime : return session_time as a string
//-------------------------------------------------

std::string session_time_device::sessiontime()
{
	struct tm *info;
	char buffer[80];
	info = localtime( &m_session_time );
	strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", info);
	return std::string(buffer);
}

//------------------------------------------------------------------
//  buildname : return tag name with dashes, skipping last x devices
//------------------------------------------------------------------

std::string session_time_device::build_name_skip_last(int skipcount = 0)
{
	device_t * dev;
	dev = this;
	device_t * rootdev = getrootdev();

	std::string s;
	while (dev)
	{
		if (skipcount-- <= 0)  // we won't skip this device
			s = std::string( (dev == rootdev ? dev->shortname() : dev->basetag() ) ) +
					std::string( s.length() ? "-" : "") + s;
		dev=dev->owner();
	}
	return s;
}

//-------------------------------------------------
//  getrootdev : return root device
//-------------------------------------------------

device_t* session_time_device::getrootdev()
{
	device_t* dev;
	device_t* lastdev = NULL;

	dev = this;
	while (dev)
	{
			lastdev = dev;
			dev=dev->owner();
	}
	return lastdev;
}



