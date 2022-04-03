// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

	intrf.h

	CRC / Disto Mini Expansion Bus management

*********************************************************************/

#ifndef MAME_BUS_COCO_DISTOMEB_H
#define MAME_BUS_COCO_DISTOMEB_H

#pragma once

/***************************************************************************
	TYPE DEFINITIONS
***************************************************************************/

// ======================> distomeb_slot_device
class device_distomeb_interface;

class distomeb_slot_device final
	: public device_t
	, public device_single_card_slot_interface<device_distomeb_interface>
{
public:
	// construction/destruction
	template <typename T>
	distomeb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: distomeb_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	distomeb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto cart_callback() { return m_cart_callback.bind(); }

	// device-level overrides
	virtual void device_start() override;

	// reading and writing to $FF50-$FF57
	u8 meb_read(offs_t offset);
	void meb_write(offs_t offset, u8 data);

	// manipulation of CART signal
	void set_cart_line(int value);
	int get_cart_line() const;

private:
	int m_cart_line;
public:
	devcb_write_line m_cart_callback;
private:
	device_distomeb_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(DISTOMEB_SLOT, distomeb_slot_device)


// ======================> device_distomeb_host_interface

// this is implemented by the Disto root devices
class device_distomeb_host_interface
{
public:
};


// ======================> device_distomeb_interface

class device_distomeb_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_distomeb_interface();
	virtual void interface_pre_start() override;

	virtual u8 meb_read(offs_t offset);
	virtual void meb_write(offs_t offset, u8 data);

protected:
	virtual void interface_config_complete() override;

	device_distomeb_interface(const machine_config &mconfig, device_t &device);

	// accessors for containers
	distomeb_slot_device &owning_slot()		{ assert(m_owning_slot); return *m_owning_slot; }
	device_distomeb_host_interface &host()	{ assert(m_host); return *m_host; }

	// setting cart values
	void set_cart_value(int value);
	void set_cart_value(bool value) { set_cart_value(value ? 1 : 0); }

private:
	distomeb_slot_device *m_owning_slot;
	device_distomeb_host_interface *m_host;
};

void disto_meb_add_basic_devices(device_slot_interface &device);

#endif // MAME_BUS_COCO_DISTOMEB_H
