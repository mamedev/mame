// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_JCBSPCH_H
#define MAME_BUS_COCO_DRAGON_JCBSPCH_H

#pragma once

#include "cococart.h"
#include "machine/6821pia.h"
#include "sound/sp0256.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_jcbspch_device

class dragon_jcbspch_device :
		public device_t,
		public device_cococart_interface
{
public:
	// construction/destruction
	dragon_jcbspch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t* get_cart_base() override;
	virtual memory_region* get_cart_memregion() override;

	virtual DECLARE_READ8_MEMBER(cts_read) override;
	virtual DECLARE_READ8_MEMBER(scs_read) override;
	virtual DECLARE_WRITE8_MEMBER(scs_write) override;

private:
	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_w);

	required_memory_region m_eprom;
	required_device<pia6821_device> m_pia;
	required_device<sp0256_device> m_nsp;
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_JCBSPCH, dragon_jcbspch_device)

#endif // MAME_BUS_COCO_DRAGON_JCBSPCH_H
