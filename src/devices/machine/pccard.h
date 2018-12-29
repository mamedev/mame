// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_PCCARD_H
#define MAME_MACHINE_PCCARD_H

#pragma once


class pccard_interface
{
public:
	virtual DECLARE_READ16_MEMBER(read_memory);
	virtual DECLARE_READ16_MEMBER(read_reg);
	virtual DECLARE_WRITE16_MEMBER(write_memory);
	virtual DECLARE_WRITE16_MEMBER(write_reg);

	virtual ~pccard_interface() {}
};

DECLARE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device)

class pccard_slot_device : public device_t, public device_slot_interface
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
	DECLARE_READ16_MEMBER(read_memory);
	DECLARE_READ16_MEMBER(read_reg);
	DECLARE_WRITE16_MEMBER(write_memory);
	DECLARE_WRITE16_MEMBER(write_reg);

protected:
	virtual void device_start() override;

private:
	// internal state
	pccard_interface *m_pccard;
};

#endif // MAME_MACHINE_PCCARD_H
