// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Hanimex Pencil 2 Memory Expansion Slot

***************************************************************************/

#ifndef MAME_BUS_PENCIL2_SLOT_H
#define MAME_BUS_PENCIL2_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_pencil2_memexp_interface;

class pencil2_memexp_slot_device : public device_t, public device_single_card_slot_interface<device_pencil2_memexp_interface>
{
public:
	// construction/destruction
	template <typename T>
	pencil2_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: pencil2_memexp_slot_device(mconfig, tag, owner)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pencil2_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callbacks
	auto romdis_handler() { return m_romdis_handler.bind(); }

	// called from cart device
	void romdis_w(int state) { m_romdis_handler(state); }

	// called from host
	u8 m0_r(offs_t offset);
	void m0_w(offs_t offset, u8 data);
	u8 m2_r(offs_t offset);
	void m2_w(offs_t offset, u8 data);
	u8 m4_r(offs_t offset);
	void m4_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_pencil2_memexp_interface *m_card;

	devcb_write_line m_romdis_handler;
};


class device_pencil2_memexp_interface : public device_interface
{
public:
	// reading and writing
	virtual u8 m0_r(offs_t offset) { return 0xff; }
	virtual void m0_w(offs_t offset, u8 data) { }
	virtual u8 m2_r(offs_t offset) { return 0xff; }
	virtual void m2_w(offs_t offset, u8 data) { }
	virtual u8 m4_r(offs_t offset) { return 0xff; }
	virtual void m4_w(offs_t offset, u8 data) { }

protected:
	// construction/destruction
	device_pencil2_memexp_interface(const machine_config &mconfig, device_t &device);

	pencil2_memexp_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PENCIL2_MEMEXP_SLOT, pencil2_memexp_slot_device)


// supported devices
void pencil2_memexp_devices(device_slot_interface &device);


#endif // MAME_BUS_PENCIL2_SLOT_H
