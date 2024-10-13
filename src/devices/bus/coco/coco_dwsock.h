// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef MAME_BUS_COCO_COCO_DWSOCKH_H
#define MAME_BUS_COCO_COCO_DWSOCKH_H

#include "osdfile.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define DRIVEWIRE_PORT_TAG          "drivewire_port"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> beckerport_device

class beckerport_device : public device_t
{
public:
	beckerport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~beckerport_device();

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start(void) override;
	virtual void device_stop(void) override;
	virtual void device_config_complete(void) override;

	void    update_port(void);

	// driver update handlers
	DECLARE_INPUT_CHANGED_MEMBER(drivewire_port_changed);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// types
	enum dwsock_ports {
		DWS_STATUS,
		DWS_DATA
	};

private:
	/* IP hostname */
	const char *            m_hostname;

	/* IP port */
	required_ioport         m_dwconfigport;
	int                     m_dwtcpport;

	osd_file::ptr m_pSocket;

	unsigned int m_rx_pending;
	unsigned int m_head;
	char m_buf[0x80];
};

// device type definition
DECLARE_DEVICE_TYPE(COCO_DWSOCK, beckerport_device)

// device iterator
typedef device_type_enumerator<beckerport_device> beckerport_device_enumerator;

#endif // MAME_BUS_COCO_COCO_DWSOCKH_H
