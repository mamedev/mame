// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_MACHINE_PC9801_CD_H
#define MAME_MACHINE_PC9801_CD_H

#include "machine/atapicdr.h"

class pc9801_cd_device : public atapi_cdrom_device
{
public:
	pc9801_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void fill_buffer() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_CD, pc9801_cd_device)

#endif // MAME_MACHINE_PC9801_CD_H
