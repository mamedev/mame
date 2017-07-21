// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atapicdr.h

    ATAPI CDROM

***************************************************************************/

#ifndef MAME_MACHINE_ATAPICDR_H
#define MAME_MACHINE_ATAPICDR_H

#pragma once

#include "atapihle.h"
#include "t10mmc.h"

class atapi_cdrom_device : public atapi_hle_device, public t10mmc
{
public:
	atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t *identify_device_buffer() { return m_identify_buffer; }

protected:
	atapi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void perform_diagnostic() override;
	virtual void identify_packet_device() override;
	virtual void process_buffer() override;
	virtual void ExecCommand() override;
	bool m_media_change;
};

class atapi_fixed_cdrom_device : public atapi_cdrom_device
{
public:
	atapi_fixed_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(ATAPI_CDROM,       atapi_cdrom_device)
DECLARE_DEVICE_TYPE(ATAPI_FIXED_CDROM, atapi_fixed_cdrom_device)

#endif // MAME_MACHINE_ATAPICDR_H
