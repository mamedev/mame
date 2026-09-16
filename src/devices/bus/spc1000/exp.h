// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SPC1000_EXP_H
#define MAME_BUS_SPC1000_EXP_H

#pragma once

// ======================> device_spc1000_card_interface

class device_spc1000_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_spc1000_card_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }

protected:
	device_spc1000_card_interface(const machine_config &mconfig, device_t &device);
};


// ======================> spc1000_exp_device

class spc1000_exp_device : public device_t, public device_single_card_slot_interface<device_spc1000_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	spc1000_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts)
		: spc1000_exp_device(mconfig, tag, owner, (uint32_t)0)
	{
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	spc1000_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~spc1000_exp_device();

	// reading and writing
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_spc1000_card_interface* m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(SPC1000_EXP_SLOT, spc1000_exp_device)

#endif // MAME_BUS_SPC1000_EXP_H
