// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    cr589.h

    Matsushita CR589

***************************************************************************/

#ifndef MAME_BUS_ATA_CR589_H
#define MAME_BUS_ATA_CR589_H

#pragma once

#include "atapicdr.h"

class matsushita_cr589_device : public atapi_cdrom_device,
	public device_nvram_interface
{
public:
	matsushita_cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;
	// ksys573 changes discs without telling the cdrom_image_device
	virtual void process_buffer() override { atapi_hle_device::process_buffer(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	int download;
	uint8_t buffer[ 65536 ];
	int bufferOffset;
};

// device type definition
DECLARE_DEVICE_TYPE(CR589, matsushita_cr589_device)

#endif // MAME_BUS_ATA_CR589_H
