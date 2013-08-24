/***************************************************************************

    atapihle.h

    ATAPI High Level Emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __ATAPIHLE_H__
#define __ATAPIHLE_H__

#include "atahle.h"
#include "scsihle.h"

class atapi_hle_device : public ata_hle_device
{
public:
	atapi_hle_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source);

	enum
	{
		ATAPI_INTERRUPT_REASON_CD = 0x01, // 1 = command, 0 = data
		ATAPI_INTERRUPT_REASON_IO = 0x02, // 1 = to host, 0 = to device
		ATAPI_INTERRUPT_REASON_REL = 0x04, // 1 = bus release
		ATAPI_INTERRUPT_REASON_TAG = 0xf8 // command tag
	};

protected:
	virtual int sector_length() { return ATAPI_BUFFER_LENGTH; }
	virtual void process_buffer();
	virtual void fill_buffer();
	virtual bool is_ready() { return false	; }
	virtual void signature();
	virtual void process_command();
	virtual void finished_command();

	virtual void identify_packet_device() = 0;

private:
	int m_packet;
	int m_data_size;
	required_device<scsihle_device> m_scsidev_device;

	static const int ATAPI_BUFFER_LENGTH = 0x800;
};

#endif
