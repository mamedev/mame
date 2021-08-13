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
	PORT_START("CNF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, session_time_device, reset_session_time, 0)
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
	pagecount = 0;

}

void session_time_device::device_reset_after_children()
{
}

void session_time_device::device_reset()
{
}


INPUT_CHANGED_MEMBER(session_time_device::reset_session_time)
{
	device_start();
	machine().popmessage("Reset Session Time = " + getprintername());
}

void session_time_device::write_snapshot_to_file(std::string directory, std::string name)
{
	printf("write snapshot\n");
	emu_file file(machine().options().snapshot_directory() + std::string("/") + directory,
		  OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	auto const filerr = file.open(name);

	if (filerr == osd_file::error::NONE)
	{
		static const rgb_t png_palette[] = { rgb_t::white(), rgb_t::black() };

		// save the paper into a png
		util::png_write_bitmap(file, nullptr, *m_bitmap, 2, png_palette);
	}
}

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

std::string session_time_device::fixcolons(std::string in)
{
	return fixchar(in, ':', '-');
}

std::string session_time_device::sessiontime()
{
	struct tm *info;
	char buffer[80];
	info = localtime( &m_session_time );
	strftime(buffer,120,"%Y-%m-%d__%H-%M-%S", info);
	return std::string(buffer);
}

std::string session_time_device::tagname()
{
   return fixcolons(std::string(getrootdev()->shortname()) + std::string(tag()));
}

std::string session_time_device::simplename()
{
	device_t * dev;
	dev = this;
	device_t * rootdev = getrootdev();
//  [[maybe_unused]]device_t * rootdev2 = & machine().root_device();
//  printf("rootdevs equal %x\n",rootdev == rootdev2);

	std::string s;
	int skipcount = 2;

	while (dev){
			if (skipcount-- <= 0)
				s = std::string( (dev == rootdev ? dev->shortname() : dev->basetag() ) ) +
						std::string( s.length() ? "-" : "") + s;
//              s=std::string(dev->shortname()) + std::string(" ") + s;
			dev=dev->owner();
	}
	return s;
}

device_t* session_time_device::getrootdev()
{
	device_t* dev;
	device_t* lastdev = NULL;

	dev = this;
	while (dev){
			lastdev = dev;
			dev=dev->owner();
	}
	return lastdev;
}



