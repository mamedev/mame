// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_MCS51_I80C52_H
#define MAME_CPU_MCS51_I80C52_H

#include "i8052.h"

class i80c52_device : public i8052_device
{
public:
	// construction/destruction
	i80c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	u8 m_saddr, m_saden;

	i80c52_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void sfr_map(address_map &map) override ATTR_COLD;

	u8 iph_r();
	void iph_w(u8 data);
	u8 saddr_r();
	void saddr_w(u8 data);
	u8 saden_r();
	void saden_w(u8 data);
};

class i80c32_device : public i80c52_device
{
public:
	// construction/destruction
	i80c32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i87c52_device : public i80c52_device
{
public:
	// construction/destruction
	i87c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i87c51fa_device : public i80c52_device
{
public:
	// construction/destruction
	i87c51fa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	i87c51fa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i80c51gb_device : public i87c51fa_device
{
public:
	// construction/destruction
	i80c51gb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class at89c52_device : public i80c52_device
{
public:
	// construction/destruction
	at89c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class at89s52_device : public i80c52_device
{
public:
	// construction/destruction
	at89s52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class ds80c320_device : public i80c52_device
{
public:
	// construction/destruction
	ds80c320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(I80C32, i80c32_device)
DECLARE_DEVICE_TYPE(I80C52, i80c52_device)
DECLARE_DEVICE_TYPE(I87C52, i87c52_device)
DECLARE_DEVICE_TYPE(I87C51FA, i87c51fa_device)
DECLARE_DEVICE_TYPE(I80C51GB, i80c51gb_device)
DECLARE_DEVICE_TYPE(AT89C52, at89c52_device)
DECLARE_DEVICE_TYPE(AT89S52, at89s52_device)
DECLARE_DEVICE_TYPE(DS80C320, ds80c320_device)

#endif
