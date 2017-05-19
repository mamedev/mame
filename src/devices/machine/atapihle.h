// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atapihle.h

    ATAPI High Level Emulation

***************************************************************************/

#ifndef MAME_MACHINE_ATAPIHLE_H
#define MAME_MACHINE_ATAPIHLE_H

#pragma once

#include "atahle.h"
#include "t10spc.h"

class atapi_hle_device : public ata_hle_device, public virtual t10spc
{
public:
	enum atapi_features_flag_t
	{
		ATAPI_FEATURES_FLAG_DMA = 0x01,
		ATAPI_FEATURES_FLAG_OVL = 0x02
	};

	enum atapi_interrupt_reason_t
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
	atapi_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual int sector_length() override { return ATAPI_BUFFER_LENGTH; }
	virtual void process_buffer() override;
	virtual void fill_buffer() override;
	virtual bool is_ready() override { return false; }
	virtual void signature() override;
	virtual void process_command() override;
	virtual void finished_command() override;

	virtual void identify_packet_device() = 0;

	packet_command_length_t packet_command_length();
	packet_command_response_t packet_command_response();

private:
	void wait_buffer();

	int m_packet;
	int m_data_size;

	static constexpr int ATAPI_BUFFER_LENGTH = 0xf800;
};

#endif // MAME_MACHINE_ATAPIHLE_H
