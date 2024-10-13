// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atapihle.h

    ATAPI High Level Emulation

***************************************************************************/

#ifndef MAME_BUS_ATA_ATAPIHLE_H
#define MAME_BUS_ATA_ATAPIHLE_H

#pragma once

#include "machine/atahle.h"
#include "machine/t10spc.h"

class atapi_hle_device : public ata_hle_device_base, public virtual t10spc
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

	void set_is_ready(bool state);

protected:
	atapi_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int sector_length() override { return ATAPI_BUFFER_LENGTH; }
	virtual void process_buffer() override;
	virtual void fill_buffer() override;
	virtual bool is_ready() override { return m_is_ready; }
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
	bool m_is_ready;

	static constexpr int ATAPI_BUFFER_LENGTH = 0xf800;
};

#endif // MAME_BUS_ATA_ATAPIHLE_H
