// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_NEC_PC98_CD_H
#define MAME_NEC_PC98_CD_H

#include "bus/ata/atapicdr.h"

class pc98_cd_device : public atapi_cdrom_device
{
public:
	pc98_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void fill_buffer() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
};

// device type definition
DECLARE_DEVICE_TYPE(PC98_CD, pc98_cd_device)

#endif // MAME_NEC_PC98_CD_H
