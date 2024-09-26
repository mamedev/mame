// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ASTROCADE_EXP_H
#define MAME_BUS_ASTROCADE_EXP_H

// ======================> device_astrocade_exp_interface

class device_astrocade_exp_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_astrocade_exp_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual uint8_t read_io(offs_t offset) { return 0xff; }
	virtual void write_io(offs_t offset, uint8_t data) { }

protected:
	device_astrocade_exp_interface(const machine_config &mconfig, device_t &device);
};


// ======================> astrocade_exp_device

class astrocade_exp_device : public device_t, public device_single_card_slot_interface<device_astrocade_exp_interface>
{
public:
	// construction/destruction
	template <typename T>
	astrocade_exp_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: astrocade_exp_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	astrocade_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~astrocade_exp_device();

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	bool get_card_mounted() { return m_card_mounted; }

	// reading and writing
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read_io(offs_t offset);
	void write_io(offs_t offset, uint8_t data);

protected:
	bool m_card_mounted;
	device_astrocade_exp_interface* m_card;
};

// device type definition
DECLARE_DEVICE_TYPE(ASTROCADE_EXP_SLOT, astrocade_exp_device)

#endif // MAME_BUS_ASTROCADE_EXP_H
