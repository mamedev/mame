// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#pragma once

#ifndef __COCO_232_H__
#define __COCO_232_H__

#include "emu.h"
#include "cococart.h"
#include "machine/mos6551.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_232_device

class coco_232_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
		virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
private:
		// internal state
		required_device<mos6551_device> m_uart;
};


// device type definition
extern const device_type COCO_232;

#endif  /* __COCO_232_H__ */
