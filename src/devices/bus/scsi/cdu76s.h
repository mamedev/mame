// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    cdu76s.h

    Sony CDU-76S

***************************************************************************/

#ifndef MAME_BUS_SCSI_CDU76S_H
#define MAME_BUS_SCSI_CDU76S_H

#pragma once

#include "scsicd.h"
#include "machine/t10mmc.h"

class sony_cdu76s_device : public scsicd_device
{
public:
	sony_cdu76s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ExecCommand() override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(CDU76S, sony_cdu76s_device)

#endif // MAME_BUS_SCSI_CDU76S_H
