// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
#ifndef MAME_BUS_NSCSI_OMTI5100_H
#define MAME_BUS_NSCSI_OMTI5100_H

#pragma once

#include "imagedev/harddriv.h"
#include "machine/nscsi_hle.h"

class nscsi_omti5100_device : public nscsi_full_device
{
public:
	nscsi_omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int id, int pos, uint8_t data) override;

private:
	enum : uint8_t
	{
		SC_CHECK_TRACK_FORMAT     = 0x05,
		SC_FORMAT_TRACK           = 0x06,
		SC_ASSIGN_DISK_PARAMETERS = 0xc2,
		SC_READ_DATA_BUFFER       = 0xec
	};

	// SASI error codes
	enum : uint8_t
	{
		ERR_NO_SEEK_COMPLETE = 0x02,
		ERR_NOT_READY        = 0x04,
		ERR_INVALID_COMMAND  = 0x20
	};

	enum : int
	{
		BUF_DISK   = 2,
		BUF_PARAM  = 3,
		BUF_BUFFER = 4
	};

	bool lun_ready(int lun) const;
	bool address_valid(uint32_t lba) const;
	void sasi_error(uint8_t code, bool valid = false, uint32_t lba = 0);
	void fill_blocks(uint32_t lba, uint32_t count);

	required_device_array<harddisk_image_device, 2> m_image;

	uint16_t m_heads[2];
	uint16_t m_sectors[2];
	uint8_t m_param[10];
	uint8_t m_lun;
	uint32_t m_lba;
	uint32_t m_cur_lba;
	uint8_t m_block[1024];
};

DECLARE_DEVICE_TYPE(NSCSI_OMTI5100, nscsi_omti5100_device)

#endif // MAME_BUS_NSCSI_OMTI5100_H
