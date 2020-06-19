// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_PCCARD_H
#define MAME_MACHINE_PCCARD_H

#pragma once


class device_pccard_interface : public device_interface
{
public:
	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0);
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0);
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual ~device_pccard_interface() {}

protected:
	device_pccard_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device)

class pccard_slot_device : public device_t, public device_single_card_slot_interface<device_pccard_interface>
{
public:
	template <typename T>
	pccard_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pccard_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ_LINE_MEMBER(read_line_inserted);
	uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0);
	void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override;

private:
	// internal state
	device_pccard_interface *m_pccard;
};

#endif // MAME_MACHINE_PCCARD_H
