// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
#ifndef MAME_BUS_NSCSI_ACB4070_H
#define MAME_BUS_NSCSI_ACB4070_H

#pragma once

#include "bus/nscsi/hd.h"

class nscsi_acb4070_device : public nscsi_harddisk_device
{
public:
	nscsi_acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int id, int pos, uint8_t data) override;

private:
	enum : uint8_t
	{
		SC_WRITE_DATA_BUFFER = 0x13,
		SC_READ_DATA_BUFFER  = 0x14
	};

	enum : int
	{
		BUF_DATA = 3,
		BUF_MODE = 4
	};

	uint8_t m_buffer[0x400];
	uint8_t m_mode[24];
};

DECLARE_DEVICE_TYPE(NSCSI_ACB4070, nscsi_acb4070_device)

#endif // MAME_BUS_NSCSI_ACB4070_H
