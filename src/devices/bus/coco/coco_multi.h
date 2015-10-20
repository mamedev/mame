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
	coco_multipak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual UINT8* get_cart_base();

	DECLARE_WRITE_LINE_MEMBER(multi_cart_w);
	DECLARE_WRITE_LINE_MEMBER(multi_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(multi_halt_w);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	// device references
	cococart_slot_device *m_owner;
	cococart_slot_device *m_slots[4];

	// internal state
	UINT8 m_select;

	// methods
	DECLARE_WRITE8_MEMBER(ff7f_write);
	cococart_slot_device *active_scs_slot(void);
	cococart_slot_device *active_cts_slot(void);
	void set_select(UINT8 new_select);
};


// device type definition
extern const device_type COCO_MULTIPAK;

#endif  /* __COCO_MULTI_H__ */
