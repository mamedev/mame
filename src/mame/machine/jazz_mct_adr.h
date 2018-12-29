// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_JAZZ_MCT_ADR_H
#define MAME_MACHINE_JAZZ_MCT_ADR_H

#pragma once

class jazz_mct_adr_device : public device_t
{
public:
	jazz_mct_adr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(JAZZ_MCT_ADR, jazz_mct_adr_device)

#endif // MAME_MACHINE_JAZZ_MCT_ADR_H
