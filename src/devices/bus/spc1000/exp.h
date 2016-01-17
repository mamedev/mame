// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SCP1000_SLOT_H
#define __SCP1000_SLOT_H

// ======================> device_spc1000_card_interface

class device_spc1000_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_spc1000_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_spc1000_card_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write) {}
};


// ======================> spc1000_exp_device

class spc1000_exp_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	spc1000_exp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~spc1000_exp_device();

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

protected:

	device_spc1000_card_interface*       m_card;
};



// device type definition
extern const device_type SPC1000_EXP_SLOT;

#endif
