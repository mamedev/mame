// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    x87 coprocessor sockets for CPUs without an on-chip FPU

***************************************************************************/

#ifndef MAME_CPU_I386_X87SLOT_H
#define MAME_CPU_I386_X87SLOT_H

#pragma once


class device_x87_interface : public device_interface
{
protected:
	device_x87_interface(const machine_config &mconfig, device_t &device);
};


class x87_slot_device : public device_t, public device_single_card_slot_interface<device_x87_interface>
{
public:
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_fitted(bool fitted) { set_default_option(fitted ? m_option : nullptr); }

protected:
	x87_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const char *option, device_type card);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_cpu;
	const char *const m_option;
};


class i387_slot_device : public x87_slot_device
{
public:
	template <typename T>
	i387_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, bool fitted = false)
		: i387_slot_device(mconfig, tag, owner, uint32_t(0))
	{
		set_cpu(std::forward<T>(cpu));
		set_fitted(fitted);
	}
	i387_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class i387sx_slot_device : public x87_slot_device
{
public:
	template <typename T>
	i387sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, bool fitted = false)
		: i387sx_slot_device(mconfig, tag, owner, uint32_t(0))
	{
		set_cpu(std::forward<T>(cpu));
		set_fitted(fitted);
	}
	i387sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class i487sx_slot_device : public x87_slot_device
{
public:
	template <typename T>
	i487sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, bool fitted = false)
		: i487sx_slot_device(mconfig, tag, owner, uint32_t(0))
	{
		set_cpu(std::forward<T>(cpu));
		set_fitted(fitted);
	}
	i487sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


DECLARE_DEVICE_TYPE(I387_SLOT,   i387_slot_device)
DECLARE_DEVICE_TYPE(I387SX_SLOT, i387sx_slot_device)
DECLARE_DEVICE_TYPE(I487SX_SLOT, i487sx_slot_device)

#endif // MAME_CPU_I386_X87SLOT_H
