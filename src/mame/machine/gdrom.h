// license:BSD-3-Clause
// copyright-holders:smf, Angelo Salese
/***************************************************************************

 gdrom.h

***************************************************************************/

#ifndef _GDROM_H_
#define _GDROM_H_

#include "machine/atapicdr.h"

class gdrom_device : public atapi_cdrom_device
{
public:
	gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Sega GD-ROM handler

	virtual void ExecCommand() override;
	virtual void WriteData( UINT8 *data, int dataLength ) override;
	virtual void ReadData( UINT8 *data, int dataLength ) override;

protected:
	virtual void process_buffer() override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 GDROM_Cmd11_Reply[32];
	UINT32 read_type;   // for command 0x30 only
	UINT32 data_select; // for command 0x30 only
	UINT32 transferOffset;
};

// device type definition
extern const device_type GDROM;

#endif
