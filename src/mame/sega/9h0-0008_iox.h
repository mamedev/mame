// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    9h0-0008_iox.h

    Sega Toys 9H0-0008 I/O expansion slot

*******************************************************************************/

#ifndef MAME_SEGA_9H0_0008_IOX_H
#define MAME_SEGA_9H0_0008_IOX_H

#pragma once

class device_sega_9h0_0008_iox_slot_interface;

class sega_9h0_0008_iox_slot_device : public device_t, public device_single_card_slot_interface<device_sega_9h0_0008_iox_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	sega_9h0_0008_iox_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: sega_9h0_0008_iox_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	sega_9h0_0008_iox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~sega_9h0_0008_iox_slot_device();

	virtual u16 data() const noexcept { return 0; }

protected:
	sega_9h0_0008_iox_slot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_sega_9h0_0008_iox_slot_interface *m_device;
};


// class representing interface-specific live I/O expansion peripheral
class device_sega_9h0_0008_iox_slot_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_sega_9h0_0008_iox_slot_interface();

	virtual u32 read(bool is_enabled) { return 0; }

protected:
	device_sega_9h0_0008_iox_slot_interface(const machine_config &mconfig, device_t &device);

	sega_9h0_0008_iox_slot_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_9H0_0008_IOX_SLOT, sega_9h0_0008_iox_slot_device)

#endif // MAME_SEGA_9H0_0008_IOX_H
