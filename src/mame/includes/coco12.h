// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.h

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_COCO12_H
#define MAME_INCLUDES_COCO12_H


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
		: coco_state(mconfig, type, tag)
		, m_sam(*this, SAM_TAG)
		, m_vdg(*this, VDG_TAG)
	{
	}

	DECLARE_READ8_MEMBER( sam_read );

	DECLARE_WRITE_LINE_MEMBER( horizontal_sync );
	DECLARE_WRITE_LINE_MEMBER( field_sync );

	void coco2bh(machine_config &config);
	void coco2h(machine_config &config);
	void cocoeh(machine_config &config);
	void cocoh(machine_config &config);
	void coco2b(machine_config &config);
	void cd6809(machine_config &config);
	void coco2(machine_config &config);
	void t4426(machine_config &config);
	void cp400(machine_config &config);
	void cocoe(machine_config &config);
	void coco(machine_config &config);
protected:
	virtual void device_start() override;
	virtual void update_cart_base(uint8_t *cart_base) override;

	// PIA1
	virtual void pia1_pb_changed(uint8_t data) override;

	sam6883_device &sam() { return *m_sam; }
	required_device<sam6883_device> m_sam;

private:
	void configure_sam(void);

protected:
	required_device<mc6847_base_device> m_vdg;
};

#endif // MAME_INCLUDES_COCO12_H
