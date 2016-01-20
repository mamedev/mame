// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atapicdr.h

    ATAPI CDROM

***************************************************************************/

#pragma once

#ifndef __ATAPICDR_H__
#define __ATAPICDR_H__

#include "atapihle.h"
#include "t10mmc.h"

class atapi_cdrom_device : public atapi_hle_device,
	public t10mmc
{
public:
	atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	atapi_cdrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source);

	UINT16 *identify_device_buffer() { return m_identify_buffer; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void perform_diagnostic() override;
	virtual void identify_packet_device() override;
};

// device type definition
extern const device_type ATAPI_CDROM;

#endif
