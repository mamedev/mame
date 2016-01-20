// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __RF5C296_H__
#define __RF5C296_H__

#include "pccard.h"

#define MCFG_RF5C296_SLOT(name) \
	rf5c296_device::set_pccard_name(*device, name);

class rf5c296_device : public device_t
{
public:
	rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void set_pccard_name(device_t &device, const char *name) { downcast<rf5c296_device &>(device).m_pccard_name = name; }

	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_READ16_MEMBER(mem_r);
	DECLARE_WRITE16_MEMBER(mem_w);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void reg_w(ATTR_UNUSED UINT8 reg, UINT8 data);
	UINT8 reg_r(ATTR_UNUSED UINT8 reg);

	unsigned char m_rf5c296_reg;
	pccard_slot_device *m_pccard;
	const char *m_pccard_name;
};

extern const device_type RF5C296;

#endif
