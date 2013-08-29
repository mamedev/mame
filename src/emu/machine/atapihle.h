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

	enum packet_command_length_t
	{
		PACKET_COMMAND_LENGTH_12,
		PACKET_COMMAND_LENGTH_16
	};

	enum packet_command_response_t
	{
		PACKET_COMMAND_RESPONSE_DRQ_3MS,
		PACKET_COMMAND_RESPONSE_INTRQ,
		PACKET_COMMAND_RESPONSE_DRQ_50US
	};

protected:
	virtual int sector_length() { return ATAPI_BUFFER_LENGTH; }
	virtual void process_buffer();
	virtual void fill_buffer();
	virtual bool is_ready() { return false; }
	virtual void signature();
	virtual void process_command();
	virtual void finished_command();

	UINT16 m_identify_buffer[256];
	virtual void identify_packet_device() = 0;

	packet_command_length_t packet_command_length();
	packet_command_response_t packet_command_response();

private:
	int m_packet;
	int m_data_size;
	required_device<scsihle_device> m_scsidev_device;

	static const int ATAPI_BUFFER_LENGTH = 0xf800;
};

#endif
