// license:BSD-3-Clause
// copyright-holders:smf, Angelo Salese
/***************************************************************************

 gdrom.h

***************************************************************************/

#ifndef MAME_BUS_ATA_GDROM_H
#define MAME_BUS_ATA_GDROM_H

#pragma once

#include "atapicdr.h"

class gdrom_device : public atapi_cdrom_device
{
public:
	gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Sega GD-ROM handler

	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;
	void cdda_end_mark_cb(int state);

protected:
	virtual void process_buffer() override;
	virtual void signature() override;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
//  virtual bool set_features() override;
	// TODO: how GDROM determines ready flag?
	// cfr. dc.xml [GDROM READY] for a list of SW that wants this on.
	virtual bool is_ready() override { return true; }

private:
	uint8_t GDROM_Cmd11_Reply[32]{};
	uint32_t read_type = 0;   // for command 0x30 only
	uint32_t data_select = 0; // for command 0x30 only
	uint32_t transferOffset = 0;

	struct cd_status
	{
		u8 repeat_count = 0;
		u8 repeat_current = 0;
		u32 cdda_fad = 0;
		u32 cdda_blocks = 0;
	};

	cd_status m_cd_status;
};

DECLARE_DEVICE_TYPE(ATAPI_GDROM, gdrom_device)

#endif // MAME_BUS_ATA_GDROM_H
