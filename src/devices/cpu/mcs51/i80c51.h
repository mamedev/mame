// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_MCS51_I80C51_H
#define MAME_CPU_MCS51_I80C51_H

#include "i8051.h"

// cmos variants
DECLARE_DEVICE_TYPE(I80C31, i80c31_device)
DECLARE_DEVICE_TYPE(I80C51, i80c51_device)
DECLARE_DEVICE_TYPE(I87C51, i87c51_device)
DECLARE_DEVICE_TYPE(P80C552, p80c552_device)
DECLARE_DEVICE_TYPE(P87C552, p87c552_device)
DECLARE_DEVICE_TYPE(P80C562, p80c562_device)

// 4k internal eprom and 128 internal ram and 2 analog comparators
DECLARE_DEVICE_TYPE(AT89C4051, at89c4051_device)

class i80c51_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i80c51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	u8 m_slave_address, m_slave_mask;

	i80c51_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width);

	virtual void sfr_map(address_map &map) override ATTR_COLD;
	virtual bool manage_idle_on_interrupt(u8 ints) override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void device_start() override ATTR_COLD;

	void slave_address_w(u8 data);
	u8 slave_address_r();
	void slave_mask_w(u8 data);
	u8 slave_mask_r();
};

class i80c31_device : public i80c51_device
{
public:
	// construction/destruction
	i80c31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i87c51_device : public i80c51_device
{
public:
	// construction/destruction
	i87c51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class at89c4051_device : public i80c51_device
{
public:
	// construction/destruction
	at89c4051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class p80c562_device : public i80c51_device
{
public:
	// construction/destruction
	p80c562_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	p80c562_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class p80c552_device : public p80c562_device
{
public:
	// construction/destruction
	p80c552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class p87c552_device : public p80c562_device
{
public:
	// construction/destruction
	p87c552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


#endif
