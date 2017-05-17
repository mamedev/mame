// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SPC1000_EXP_H
#define MAME_BUS_SPC1000_EXP_H

#pragma once

// ======================> device_spc1000_card_interface

class device_spc1000_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_spc1000_card_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write) { }

protected:
	device_spc1000_card_interface(const machine_config &mconfig, device_t &device);
};


// ======================> spc1000_exp_device

class spc1000_exp_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	spc1000_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~spc1000_exp_device();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

	device_spc1000_card_interface* m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(SPC1000_EXP_SLOT, spc1000_exp_device)

#endif // MAME_BUS_SPC1000_EXP_H
