// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef _DWSOCK_H_
#define _DWSOCK_H_

#include "emu.h"
#include "osdcore.h"

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
	beckerport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~beckerport_device();

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	virtual void device_start(void) override;
	virtual void device_stop(void) override;
	virtual void device_config_complete(void) override;

	void    update_port(void);

	// driver update handlers
	void drivewire_port_changed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
extern const device_type COCO_DWSOCK;

// device iterator
typedef device_type_iterator<&device_creator<beckerport_device>, beckerport_device> beckerport_device_iterator;

#endif /* _DWSOCK_H_ */
