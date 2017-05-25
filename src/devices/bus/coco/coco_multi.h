// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_multi.h

    Multi-Pak interface emulation

***************************************************************************/

#ifndef MAME_BUS_COCO_COCO_MULTI_H
#define MAME_BUS_COCO_COCO_MULTI_H

#pragma once

#include "cococart.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_multipak_device

class coco_multipak_device :
	public device_t,
	public device_cococart_interface
{
public:
	// construction/destruction
	coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual uint8_t* get_cart_base() override;

	// these are only public so they can be in a MACHINE_CONFIG_START
	// declaration; don't think about them as publically accessable
	DECLARE_WRITE_LINE_MEMBER(multi_slot1_cart_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot1_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot1_halt_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot2_cart_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot2_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot2_halt_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot3_cart_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot3_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot3_halt_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot4_cart_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot4_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(multi_slot4_halt_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual void set_sound_enable(bool sound_enable) override;

private:
	// device references
	required_device_array<cococart_slot_device, 4> m_slots;

	// internal state
	uint8_t m_select;

	// internal accessors
	cococart_slot_device &owning_slot();
	int active_scs_slot_number() const;
	int active_cts_slot_number() const;
	cococart_slot_device &slot(int slot_number);
	cococart_slot_device &active_scs_slot();
	cococart_slot_device &active_cts_slot();

	// methods
	void set_select(uint8_t new_select);
	DECLARE_WRITE8_MEMBER(ff7f_write);
	void update_line(int slot_number, cococart_slot_device::line line);
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_MULTIPAK, coco_multipak_device)

#endif  // MAME_BUS_COCO_COCO_MULTI_H
