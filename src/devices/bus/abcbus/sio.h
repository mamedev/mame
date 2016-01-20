// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ABC_SIO__
#define __ABC_SIO__

#include "emu.h"
#include "abcbus.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_sio_device

class abc_sio_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data) override;
	virtual UINT8 abcbus_xmemfl(offs_t offset) override;

private:
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_sio;
	required_memory_region m_rom;
};


// device type definition
extern const device_type ABC_SIO;



#endif
