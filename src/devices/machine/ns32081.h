// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NS32081_H
#define MAME_MACHINE_NS32081_H

#pragma once

#include "cpu/ns32000/common.h"

class ns32081_device_base
	: public device_t
	, public ns32000_fpu_interface
{
protected:
	ns32081_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ns32000_slave_interface implementation
	virtual void state_add(device_state_interface &parent, int &index) override;

	// slave interface handlers
	template <typename T> T read();
	template <typename T> void write(T data);

	// execution helpers
	bool decode(u8 const idbyte, u16 const opword);
	void execute();
	u16 status(int *icount);
	void complete(s32 param);

	// register helpers
	virtual void reg_get(unsigned const op_size, u64 &op_value, unsigned const reg) const = 0;
	virtual void reg_set(unsigned const reg, unsigned const op_size, u64 const op_value) = 0;

private:
	emu_timer *m_complete;

	u32 m_fsr; // floating-point status register

	// operating state
	u32 m_state;
	u8 m_idbyte;
	u16 m_opword;
	struct operand
	{
		u32 expected;
		u32 issued;
		u64 value;
	}
	m_op[3];
	u16 m_status;
	u32 m_tcy;
};

class ns32081_device
	: public ns32081_device_base
	, public ns32000_slow_slave_interface
{
public:
	ns32081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// ns32000_slave_interface implementation
	virtual void state_add(device_state_interface &parent, int &index) override;

	// ns32000_slow_slave_interface implementation
	virtual u16 slow_status(int *icount = nullptr) override;
	virtual u16 slow_read() override;
	virtual void slow_write(u16 data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// register helpers
	virtual void reg_get(unsigned const op_size, u64 &op_value, unsigned const reg) const override;
	virtual void reg_set(unsigned const reg, unsigned const op_size, u64 const op_value) override;

private:
	// registers
	u32 m_f[8];
};

class ns32381_device
	: public ns32081_device_base
	, public ns32000_slow_slave_interface
	, public ns32000_fast_slave_interface
{
public:
	ns32381_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// ns32000_slave_interface implementation
	virtual void state_add(device_state_interface &parent, int &index) override;

	// ns32000_slow_slave_interface implementation
	virtual u16 slow_status(int *icount = nullptr) override;
	virtual u16 slow_read() override;
	virtual void slow_write(u16 data) override;

	// ns32000_fast_slave_interface implementation
	virtual u32 fast_status(int *icount = nullptr) override;
	virtual u32 fast_read() override;
	virtual void fast_write(u32 data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// register helpers
	virtual void reg_get(unsigned const op_size, u64 &op_value, unsigned const reg) const override;
	virtual void reg_set(unsigned const reg, unsigned const op_size, u64 const op_value) override;

private:
	// registers
	u64 m_l[8];
};

DECLARE_DEVICE_TYPE(NS32081, ns32081_device)
DECLARE_DEVICE_TYPE(NS32381, ns32381_device)

#endif // MAME_MACHINE_NS32081_H
