// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_SCSI_OMTI5100_H
#define MAME_BUS_SCSI_OMTI5100_H

#include "scsi.h"
#include "scsihd.h"
#include "imagedev/harddriv.h"

class omti5100_device : public scsihd_device
{
public:
	omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ExecCommand() override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;

protected:
	void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<harddisk_image_device> m_image0;
	required_device<harddisk_image_device> m_image1;
	hard_disk_file::info m_param[2];
};

DECLARE_DEVICE_TYPE(OMTI5100, omti5100_device)

#endif // MAME_BUS_SCSI_OMTI5100_H
