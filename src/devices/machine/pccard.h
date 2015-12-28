// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __PCCARD_H__
#define __PCCARD_H__

#include "emu.h"

class pccard_interface
{
public:
	virtual DECLARE_READ16_MEMBER(read_memory);
	virtual DECLARE_READ16_MEMBER(read_reg);
	virtual DECLARE_WRITE16_MEMBER(write_memory);
	virtual DECLARE_WRITE16_MEMBER(write_reg);

	virtual ~pccard_interface() {}
};

extern const device_type PCCARD_SLOT;

class pccard_slot_device : public device_t,
	public device_slot_interface
{
public:
	pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ_LINE_MEMBER(read_line_inserted);
	DECLARE_READ16_MEMBER(read_memory);
	DECLARE_READ16_MEMBER(read_reg);
	DECLARE_WRITE16_MEMBER(write_memory);
	DECLARE_WRITE16_MEMBER(write_reg);

protected:
	virtual void device_start() override;

private:
	// internal state
	pccard_interface *m_pccard;
};

#endif
