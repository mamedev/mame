// license:BSD-3-Clause
// copyright-holders:Hydreigon223

/*************************************************************

Texas Instruments TSB12LV01A "IEEE 1394" Link Layer Controller

**************************************************************/

#ifndef MAME_MACHINE_TSB12LV01A_H
#define MAME_MACHINE_TSB12LV01A_H

#pragma once

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tsb12lv01a_device : public device_t
{
public:
	tsb12lv01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint32_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

//device type definition
DECLARE_DEVICE_TYPE(TSB12LV01A, tsb12lv01a_device)

#endif // MAME_MACHINE_TSB12LV01A_H
