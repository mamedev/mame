// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef MAME_BUS_COCO_COCO_PAK_H
#define MAME_BUS_COCO_COCO_PAK_H

#pragma once

#include "cococart.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_pak_device

class coco_pak_device :
		public device_t,
		public device_cococart_interface
{
public:
	// construction/destruction
	coco_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual u8 *get_cart_base() override;
	virtual u32 get_cart_size() override;
	virtual memory_region *get_cart_memregion() override;

protected:
	coco_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual u8 cts_read(offs_t offset) override;

	// internal state
	device_image_interface *m_cart;
	required_memory_region m_eprom;
	optional_ioport m_autostart;
};


// ======================> coco_pak_banked_device

class coco_pak_banked_device : public coco_pak_device
{
public:
	// construction/destruction
	coco_pak_banked_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	coco_pak_banked_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual u8 *get_cart_base() override;
	virtual u32 get_cart_size() override;
	virtual u8 cts_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;

private:
	u8 m_pos;
};

// ======================> coco_pak_rmulticart_device
// Ramoth Dragon/CoCo Multicart.

class coco_pak_rmulticart_device : public coco_pak_device
{
public:
	// construction/destruction
	coco_pak_rmulticart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	coco_pak_rmulticart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual u8 *get_cart_base() override;
	virtual u32 get_cart_size() override;
	virtual u8 cts_read(offs_t offset) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;

	void SetCART();
private:
	u8 Page;		// Page register, at $FF50
	u8 CTRL;		// Control register, at $FF52
	u16 PageSize;	// Page size in bytes
	u8 PageMask;	// Page mask
};


// device type definitions
DECLARE_DEVICE_TYPE(COCO_PAK, coco_pak_device)
DECLARE_DEVICE_TYPE(COCO_PAK_BANKED, coco_pak_banked_device)
DECLARE_DEVICE_TYPE(COCO_PAK_RMULTICART, coco_pak_rmulticart_device)

#endif // MAME_BUS_COCO_COCO_PAK_H
