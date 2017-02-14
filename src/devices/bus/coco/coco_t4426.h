// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef MAME_BUS_COCO_T4426_H
#define MAME_BUS_COCO_T4426_H

#pragma once

#include "cococart.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_t4426_device

class coco_t4426_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_t4426_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
		coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;
		virtual ioport_constructor device_input_ports() const override;

		virtual uint8_t* get_cart_base() override;
		DECLARE_WRITE8_MEMBER( pia_A_w );

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		// internal state
		device_image_interface *m_cart;
		cococart_slot_device *m_owner;
		uint8_t m_select;

		optional_ioport m_autostart;

		virtual DECLARE_READ8_MEMBER(read) override;
		virtual DECLARE_WRITE8_MEMBER(write) override;
private:
		// internal state
		required_device<acia6850_device> m_uart;
		required_device<pia6821_device> m_pia;
};


// device type definition
extern const device_type COCO_T4426;

#endif  /* MAME_BUS_COCO_T4426_H */
