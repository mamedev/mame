// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

**********************************************************************/

#pragma once

#ifndef __CBM2_GRAPHIC__
#define __CBM2_GRAPHIC__

#include "emu.h"
#include "exp.h"
#include "video/ef9365.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_hrg_t

class cbm2_hrg_t : public device_t,
					public device_cbm2_expansion_card_interface
{
public:
	// construction/destruction
	cbm2_hrg_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_cbm2_expansion_card_interface overrides
	virtual UINT8 cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3) override;
	virtual void cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3) override;

private:
	required_device<ef9365_device> m_gdc;
	required_memory_region m_bank3;
};


// ======================> cbm2_hrg_a_t

class cbm2_hrg_a_t :  public cbm2_hrg_t
{
public:
	// construction/destruction
	cbm2_hrg_a_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> cbm2_hrg_b_t

class cbm2_hrg_b_t :  public cbm2_hrg_t
{
public:
	// construction/destruction
	cbm2_hrg_b_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// device type definition
extern const device_type CBM2_HRG_A;
extern const device_type CBM2_HRG_B;



#endif
