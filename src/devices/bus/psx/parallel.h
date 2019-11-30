// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_BUS_PSX_PARALLEL_H
#define MAME_BUS_PSX_PARALLEL_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psx_parallel_slot_device

class psx_parallel_interface;

class psx_parallel_slot_device : public device_t, public device_single_card_slot_interface<psx_parallel_interface>
{
public:
	// construction/destruction
	template <typename T>
	psx_parallel_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: psx_parallel_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	psx_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~psx_parallel_slot_device();

	DECLARE_READ16_MEMBER(exp_r);
	DECLARE_WRITE16_MEMBER(exp_w);

	bool hascard() const { return bool(m_card); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	psx_parallel_interface *m_card;
};


// ======================> psx_parallel_interface

class psx_parallel_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~psx_parallel_interface();

	// reading and writing
	virtual DECLARE_READ16_MEMBER(exp_r) { return 0xff; }
	virtual DECLARE_WRITE16_MEMBER(exp_w) { }

protected:
	psx_parallel_interface(const machine_config &mconfig, device_t &device);
};


// device type definition
DECLARE_DEVICE_TYPE(PSX_PARALLEL_SLOT, psx_parallel_slot_device)

void psx_parallel_devices(device_slot_interface &device);


#endif // MAME_BUS_PSX_PARALLEL_H
