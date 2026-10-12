// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_PBI_SLOT_H
#define MAME_BUS_A800_PBI_SLOT_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_pbi_slot_device

class atari_pbi_interface;
class atari_pbi_card_device;

class atari_pbi_slot_device : public device_t,
                              public device_single_card_slot_interface<atari_pbi_interface>,
							  public device_memory_interface
{
//	friend class atari_pbi_interface;

public:
	// construction/destruction
	template <typename T>
	atari_pbi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: atari_pbi_slot_device(mconfig, tag, owner)
	{
		set_options(std::forward<T>(slot_options), default_option, false);
	}

	atari_pbi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	u8 read(offs_t offset);
	u8 read_d8xx(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void write_d8xx(offs_t offset, uint8_t data);

	u8 pdvi_r(offs_t offset);
	void pdvs_w(offs_t offset, u8 data);

	auto mpd_handler() { return m_mpd_cb.bind(); }

	address_space &memspace() const { return *m_space; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	atari_pbi_interface *m_card;

private:
	address_space_config m_space_config;
	address_space *m_space;

	devcb_write_line m_mpd_cb;
};


// ======================> atari_pbi_interface

class atari_pbi_interface : public device_interface
{
public:
	virtual ~atari_pbi_interface();

	virtual u8 pdvi_slot_r() { return 0xff; }
	virtual int pdvs_slot_w(u8 data) { return 0; };

protected:
	atari_pbi_interface(const machine_config &mconfig, device_t &device);

	atari_pbi_slot_device *const m_slot;
};

class atari_pbi_card_device : public device_t,
                              public atari_pbi_interface
{
public:
	// construction/destruction
	atari_pbi_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// device type declaration
DECLARE_DEVICE_TYPE(ATARI_PBI_SLOT, atari_pbi_slot_device)



#endif // MAME_BUS_A800_PBI_SLOT_H

