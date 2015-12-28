// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __ASTROCADE_EXP_H
#define __ASTROCADE_EXP_H

// ======================> device_astrocade_card_interface

class device_astrocade_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_astrocade_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_astrocade_card_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write) {}
};


// ======================> astrocade_exp_device

class astrocade_exp_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	astrocade_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~astrocade_exp_device();

	// device-level overrides
	virtual void device_start() override;

	bool get_card_mounted() { return m_card_mounted; }

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

protected:
	bool m_card_mounted;
	device_astrocade_card_interface* m_card;
};



// device type definition
extern const device_type ASTROCADE_EXP_SLOT;


#define MCFG_ASTROCADE_EXPANSION_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ASTROCADE_EXP_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#endif
