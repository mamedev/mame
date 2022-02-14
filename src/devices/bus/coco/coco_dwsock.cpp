// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include "emu.h"
#include "coco_dwsock.h"

#include <cstdio>
#include <cstdlib>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_DWSOCK, beckerport_device, "coco_dwsock", "Virtual Becker Port")

//-------------------------------------------------
//  INPUT_PORTS( coco_drivewire )
//-------------------------------------------------

INPUT_PORTS_START( coco_drivewire )
	PORT_START(DRIVEWIRE_PORT_TAG)
	PORT_CONFNAME( 0xffff, 65504, "Drivewire Server TCP Port") PORT_CHANGED_MEMBER(DEVICE_SELF, beckerport_device, drivewire_port_changed, 0)
	PORT_CONFSETTING(      65500, "65500" )
	PORT_CONFSETTING(      65501, "65501" )
	PORT_CONFSETTING(      65502, "65502" )
	PORT_CONFSETTING(      65503, "65503" )
	PORT_CONFSETTING(      65504, "65504" )
	PORT_CONFSETTING(      65505, "65505" )
	PORT_CONFSETTING(      65506, "65506" )
	PORT_CONFSETTING(      65507, "65507" )
	PORT_CONFSETTING(      65508, "65508" )
	PORT_CONFSETTING(      65509, "65509" )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor beckerport_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_drivewire );
}

//-------------------------------------------------
//  drivewire_port_changed
//-------------------------------------------------
INPUT_CHANGED_MEMBER(beckerport_device::drivewire_port_changed)
{
	this->update_port();
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  beckerport_device - constructor / destructor
//-------------------------------------------------

beckerport_device::beckerport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_DWSOCK, tag, owner, clock)
	, m_hostname(nullptr), m_dwconfigport(*this, DRIVEWIRE_PORT_TAG), m_dwtcpport(0)
{
	m_head = 0;
	m_rx_pending = 0;
}

beckerport_device::~beckerport_device()
{
	if (m_pSocket)
		beckerport_device::device_stop();
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void beckerport_device::device_start()
{
	osd_printf_verbose("%s: Connecting to Drivewire server on %s:%d... ", tag(), m_hostname, m_dwtcpport);

	u64 filesize; // unused
	/* format address string for opening the port */
	std::error_condition filerr = osd_file::open(util::string_format("socket.%s:%d", m_hostname, m_dwtcpport), 0, m_pSocket, filesize);
	if (filerr)
	{
		osd_printf_verbose("Error: osd_open returned error %s:%d %s!\n", filerr.category().name(), filerr.value(), filerr.message());
		return;
	}

	osd_printf_verbose("Connected!\n");

	// save state support
	save_item(NAME(m_rx_pending));
	save_item(NAME(m_head));
	save_item(NAME(m_buf));

}

/*-------------------------------------------------
    device_stop
-------------------------------------------------*/

void beckerport_device::device_stop()
{
	if (m_pSocket)
	{
		printf("%s: Closing connection to Drivewire server\n", tag());
		m_pSocket.reset();
	}
}

/*-------------------------------------------------
    device_config_complete
-------------------------------------------------*/

void beckerport_device::device_config_complete()
{
	m_hostname = "127.0.0.1";
	m_dwtcpport = 65504;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

u8 beckerport_device::read(offs_t offset)
{
	unsigned char data = 0x5a;

	if (!m_pSocket)
		return data;

	switch (offset)
	{
		case DWS_STATUS:
			if (!m_rx_pending)
			{
				// Try to read from dws
				std::error_condition filerr = m_pSocket->read(m_buf, 0, sizeof(m_buf), m_rx_pending);
				if (filerr && (std::errc::operation_would_block != filerr))
					osd_printf_error("%s: coco_dwsock.c: beckerport_device::read() socket read operation failed with error %s:%d %s\n", tag(), filerr.category().name(), filerr.value(), filerr.message());
				else
					m_head = 0;
			}
			//logerror("beckerport_device: status read. %i bytes remaining.\n", m_rx_pending);
			data = (m_rx_pending > 0) ? 2 : 0;
			break;
		case DWS_DATA:
			if (!m_rx_pending)
			{
				osd_printf_error("%s: coco_dwsock.c: beckerport_device::read() buffer underrun\n", tag());
				break;
			}
			data = m_buf[m_head++];
			m_rx_pending--;
			//logerror("beckerport_device: data read 1 byte (0x%02x).  %i bytes remaining.\n", data&0xff, m_rx_pending);
			break;
		default:
			fprintf(stderr, "%s: read from bad offset %d\n", __FILE__, offset);
	}

	return data;
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void beckerport_device::write(offs_t offset, u8 data)
{
	char d = char(data);
	std::error_condition filerr;
	u32 written;

	if (!m_pSocket)
		return;

	switch (offset)
	{
		case DWS_STATUS:
			//logerror("beckerport_write: error: write (0x%02x) to status register\n", d);
			break;
		case DWS_DATA:
			filerr = m_pSocket->write(&d, 0, 1, written);
			if (filerr)
				osd_printf_error("%s: coco_dwsock.c: beckerport_device::write() socket write operation failed with error %s:%d %s\n", tag(), filerr.category().name(), filerr.value(), filerr.message());
			//logerror("beckerport_write: data write one byte (0x%02x)\n", d & 0xff);
			break;
		default:
			fprintf(stderr, "%s: write to bad offset %d\n", __FILE__, offset);
	}
}

/*-------------------------------------------------
    update_port
-------------------------------------------------*/

void beckerport_device::update_port()
{
	device_stop();
	m_dwtcpport = m_dwconfigport->read();
	device_start();
}
