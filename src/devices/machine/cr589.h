// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    cr589.h

    Matsushita CR589

***************************************************************************/

#pragma once

#ifndef __CR589_H__
#define __CR589_H__

#include "atapicdr.h"
#include "t10mmc.h"

class matsushita_cr589_device : public atapi_cdrom_device,
	public device_nvram_interface
{
public:
	matsushita_cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand() override;
	virtual void WriteData( UINT8 *data, int dataLength ) override;
	virtual void ReadData( UINT8 *data, int dataLength ) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	int download;
	UINT8 buffer[ 65536 ];
	int bufferOffset;
};

// device type definition
extern const device_type CR589;

#endif
