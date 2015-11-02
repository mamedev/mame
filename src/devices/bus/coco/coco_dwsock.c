// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include <stdio.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>

#include "emu.h"
#include "osdcore.h"
#include "includes/coco.h"

#include "coco_dwsock.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COCO_DWSOCK = &device_creator<beckerport_device>;

//-------------------------------------------------
//  INPUT_PORTS( coco_drivewire )
//-------------------------------------------------

INPUT_PORTS_START( coco_drivewire )
	PORT_START(DRIVEWIRE_PORT_TAG)
	PORT_CONFNAME( 0xffff, 65504, "Drivewire Server TCP Port")
		PORT_CHANGED_MEMBER(DEVICE_SELF, beckerport_device, drivewire_port_changed, NULL )
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

beckerport_device::beckerport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COCO_DWSOCK, "Virtual Becker Port", tag, owner, clock, "coco_dwsock", __FILE__),
	m_dwconfigport(*this, DRIVEWIRE_PORT_TAG)
{
	m_pSocket = NULL;
	m_head = 0;
	m_rx_pending = 0;
}

beckerport_device::~beckerport_device()
{
	if (m_pSocket != NULL)
		device_stop();
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void beckerport_device::device_start(void)
{
	char chAddress[64];

	/* format address string for opening the port */
	snprintf(chAddress, sizeof(chAddress), "socket.%s:%d", m_hostname, m_dwtcpport);

	osd_printf_verbose("Connecting to Drivewire server on %s:%d... ", m_hostname, m_dwtcpport);

	UINT64 filesize; // unused
	file_error filerr = osd_open(chAddress, 0, &m_pSocket, &filesize);
	if (filerr != FILERR_NONE)
	{
		osd_printf_verbose("Error: osd_open returned error %i!\n", (int) filerr);
		return;
	}

	osd_printf_verbose("Connected!\n");
}

/*-------------------------------------------------
    device_stop
-------------------------------------------------*/

void beckerport_device::device_stop(void)
{
	if (m_pSocket != NULL)
	{
		printf("Closing connection to Drivewire server\n");
		osd_close(m_pSocket);
		m_pSocket = NULL;
	}
}

/*-------------------------------------------------
    device_config_complete
-------------------------------------------------*/

void beckerport_device::device_config_complete(void)
{
	m_hostname = "127.0.0.1";
	m_dwtcpport = 65504;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(beckerport_device::read)
{
	unsigned char data = 0x5a;

	if (m_pSocket == NULL)
		return data;

	switch (offset)
	{
		case DWS_STATUS:
			if (!m_rx_pending)
			{
				/* Try to read from dws */
				file_error filerr = osd_read(m_pSocket, m_buf, 0, sizeof(m_buf), &m_rx_pending);
				if (filerr != FILERR_NONE && filerr != FILERR_FAILURE)  // FILERR_FAILURE means no data available, so don't throw error message
					fprintf(stderr, "coco_dwsock.c: beckerport_device::read() socket read operation failed with file_error %i\n", filerr);
				else
					m_head = 0;
			}
			//printf("beckerport_device: status read. %i bytes remaining.\n", m_rx_pending);
			data = (m_rx_pending > 0) ? 2 : 0;
			break;
		case DWS_DATA:
			if (!m_rx_pending) {
				fprintf(stderr, "coco_dwsock.c: beckerport_device::read() buffer underrun\n");
				break;
			}
			data = m_buf[m_head++];
			m_rx_pending--;
			//printf("beckerport_device: data read 1 byte (0x%02x).  %i bytes remaining.\n", data&0xff, m_rx_pending);
			break;
		default:
			fprintf(stderr, "%s: read from bad offset %d\n", __FILE__, offset);
	}

	return (int)data;
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(beckerport_device::write)
{
	char d = (char)data;
	file_error filerr;

	if (m_pSocket == NULL)
		return;

	switch (offset)
	{
		case DWS_STATUS:
			//printf("beckerport_write: error: write (0x%02x) to status register\n", d);
			break;
		case DWS_DATA:
			filerr = osd_write(m_pSocket, &d, 0, 1, NULL);
			if (filerr != FILERR_NONE)
				fprintf(stderr, "coco_dwsock.c: beckerport_device::write() socket write operation failed with file_error %i\n", filerr);
			//printf("beckerport_write: data write one byte (0x%02x)\n", d & 0xff);
			break;
		default:
			fprintf(stderr, "%s: write to bad offset %d\n", __FILE__, offset);
	}
}

/*-------------------------------------------------
    update_port
-------------------------------------------------*/

void beckerport_device::update_port(void)
{
	device_stop();
	m_dwtcpport = read_safe(m_dwconfigport, 65504);
	device_start();
}
