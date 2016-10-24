// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_multi.h

    Multi-Pak interface emulation

***************************************************************************/

#pragma once

#ifndef __COCO_MULTI_H__
#define __COCO_MULTI_H__

#include "emu.h"
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

	void multi_cart_w(int state);
	void multi_nmi_w(int state);
	void multi_halt_w(int state);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	// device references
	cococart_slot_device *m_owner;
	cococart_slot_device *m_slots[4];

	// internal state
	uint8_t m_select;

	// methods
	void ff7f_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	cococart_slot_device *active_scs_slot(void);
	cococart_slot_device *active_cts_slot(void);
	void set_select(uint8_t new_select);
};


// device type definition
extern const device_type COCO_MULTIPAK;

#endif  /* __COCO_MULTI_H__ */
