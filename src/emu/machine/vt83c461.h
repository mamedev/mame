// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    vt83c461.h

    VIA VT83C461 (IDE Hard Drive controller).

***************************************************************************/

#pragma once

#ifndef __VT83C461_H__
#define __VT83C461_H__

#include "idectrl.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_VT83C461_ADD(_tag, _slot_intf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, VT83C461, 0) \
	MCFG_DEVICE_MODIFY(_tag ":0") \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _master, _fixed) \
	MCFG_DEVICE_MODIFY(_tag ":1") \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _slave, _fixed) \
	MCFG_DEVICE_MODIFY(_tag)

#define IDE_CONFIG_REGISTERS                0x10

class vt83c461_device : public ide_controller_32_device
{
public:
	vt83c461_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER(read_config);
	DECLARE_WRITE32_MEMBER(write_config);

protected:
	virtual void device_start();

private:
	UINT8           m_config_unknown;
	UINT8           m_config_register[IDE_CONFIG_REGISTERS];
	UINT8           m_config_register_num;
};

extern const device_type VT83C461;

#endif
