// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __LPC2103__
#define __LPC2103__

#include "emu.h"
#include "arm7.h"
#include "arm7core.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class lpc210x_device : public arm7_cpu_device
{
public:
	lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers

	// todo, use an appropriate flash type instead
	UINT8 m_flash[0x8000];


	DECLARE_READ32_MEMBER(arm_E01FC088_r);
	DECLARE_READ32_MEMBER(flash_r);
	DECLARE_WRITE32_MEMBER(flash_w);


protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	address_space_config m_program_config;

};


// device type definition
extern const device_type LPC2103;


#endif /// __LPC2103__
