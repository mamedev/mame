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
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; }
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) {}
};


// ======================> spc1000_exp_device

class spc1000_exp_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	spc1000_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~spc1000_exp_device();

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:

	device_spc1000_card_interface*       m_card;
};



// device type definition
extern const device_type SPC1000_EXP_SLOT;

#endif
