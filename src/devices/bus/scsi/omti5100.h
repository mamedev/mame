// license:BSD-3-Clause
// copyright-holders:smf
#ifndef OMTI5100_H_
#define OMTI5100_H_

#include "emu.h"
#include "scsi.h"
#include "scsihd.h"
#include "imagedev/harddriv.h"

class omti5100_device : public scsihd_device
{
public:
	omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	//virtual const rom_entry *device_rom_region() const;

	virtual void ExecCommand() override;
	virtual void ReadData( UINT8 *data, int dataLength ) override;
	virtual void WriteData( UINT8 *data, int dataLength ) override;
	void device_start() override;

private:
	required_device<harddisk_image_device> m_image0;
	required_device<harddisk_image_device> m_image1;
	hard_disk_info m_param[2];
};

extern const device_type OMTI5100;

#endif /* OMTI5100_H_ */
