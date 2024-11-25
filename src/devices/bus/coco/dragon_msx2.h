// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_DRAGON_MSX2_H
#define MAME_BUS_COCO_DRAGON_MSX2_H

#pragma once

#include "cococart.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "video/v9938.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dragon_msx2_device

class dragon_msx2_device :
	public device_t,
	public device_cococart_interface
{
public:
	// construction/destruction
	dragon_msx2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;

private:
	required_device<ym2413_device> m_ym2413;
	required_device<v9958_device> m_v9958;
	required_device<ym2149_device> m_ym2149;
	required_ioport m_config;

	void video_select_w(u8 data);
};


// device type definition
DECLARE_DEVICE_TYPE(DRAGON_MSX2, dragon_msx2_device)

#endif // MAME_BUS_COCO_DRAGON_MSX2_H
