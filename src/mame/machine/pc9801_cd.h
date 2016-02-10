// license:BSD-3-Clause
// copyright-holders:smf

#ifndef __PC9801_CD_H__
#define __PC9801_CD_H__

#include "machine/atapicdr.h"

class pc9801_cd_device : public atapi_cdrom_device
{
public:
	pc9801_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void fill_buffer() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
};

// device type definition
extern const device_type PC9801_CD;

#endif
