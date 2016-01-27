// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#pragma once

#ifndef __ISA_FINALCHS_H__
#define __ISA_FINALCHS_H__

#include "emu.h"
#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_finalchs_device

class isa8_finalchs_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		DECLARE_READ8_MEMBER(finalchs_r);
		DECLARE_WRITE8_MEMBER(finalchs_w);

		DECLARE_WRITE8_MEMBER( io7ff8_write );
		DECLARE_READ8_MEMBER( io7ff8_read );
		DECLARE_READ8_MEMBER( io6000_read );
		DECLARE_WRITE8_MEMBER( io6000_write );

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

private:
		// internal state
		UINT8 m_FCH_latch_data;
};


// device type definition
extern const device_type ISA8_FINALCHS;

#endif  /* __ISA_FINALCHS_H__ */
