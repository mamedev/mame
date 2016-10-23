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
		isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		uint8_t finalchs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void finalchs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

		void io7ff8_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
		uint8_t io7ff8_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		uint8_t io6000_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void io6000_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

private:
		// internal state
		uint8_t m_FCH_latch_data;
};


// device type definition
extern const device_type ISA8_FINALCHS;

#endif  /* __ISA_FINALCHS_H__ */
