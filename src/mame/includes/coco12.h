// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#pragma once

#ifndef __COCO12__
#define __COCO12__


#include "includes/coco.h"
#include "machine/6883sam.h"
#include "video/mc6847.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SAM_TAG         "sam"
#define VDG_TAG         "vdg"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coco12_state : public coco_state
{
public:
	coco12_state(const machine_config &mconfig, device_type type, const char *tag)
	: coco_state(mconfig, type, tag),
		m_sam(*this, SAM_TAG),
		m_vdg(*this, VDG_TAG)
	{
	}


	required_device<sam6883_device> m_sam;
	required_device<mc6847_base_device> m_vdg;

	uint8_t sam_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void horizontal_sync(int state);
	void field_sync(int state);
protected:
	virtual void device_start() override;
	virtual void update_cart_base(uint8_t *cart_base) override;

	/* PIA1 */
	virtual void pia1_pb_changed(uint8_t data) override;

private:

	void configure_sam(void);
};


#endif /* __COCO12__ */
