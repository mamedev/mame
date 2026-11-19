// license:BSD-3-Clause
// copyright-holders:Paul Priest, R. Belmont
// Konami 056734 (ESC) security coprocessor

#ifndef MAME_CPU_K056734_K056734_H
#define MAME_CPU_K056734_K056734_H

#pragma once

#include "k056734d.h"

#include <array>

class k056734_device : public cpu_device
{
public:
	enum
	{
		K056734_PC = 1,
		K056734_FLAGS,
		K056734_R1,
		K056734_S0 = K056734_R1 + 31
	};

	k056734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// per-chip secret: s10[15:0] and the configuration nibble s11[27:24]
	k056734_device &set_key(u16 key, u8 config) { m_key = key; m_config = config; return *this; }
	// instruction fetch descrambler: canonical lane k is lane lanes[k] of (stored word ^ xor_mask)
	k056734_device &set_fetch_scramble(u32 xor_mask, const std::array<u8, 16> &lanes) { m_fetch_xor = xor_mask; m_fetch_lanes = lanes; return *this; }
	template <typename T> k056734_device &set_host_space(T &&tag, int spacenum) { m_host.set_tag(std::forward<T>(tag), spacenum); return *this; }

	auto irq_cb() { return m_irq_cb.bind(); }

	// host mailbox, written as longwords; the chip clears it by writing zero here itself
	void mailbox_w(u32 data);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum : u8
	{
		F_Z = 0x01, // zero
		F_N = 0x02, // negative
		F_C = 0x04, // carry
		F_V = 0x08  // signed overflow
	};

	// special registers, numbered as operands (s0 = 32)
	enum : unsigned
	{
		S_SP = 33,
		S_PC = 34,
		S_HOST = 38,
		S_MAILBOX = 39,
		S_MULDIV = 40,
		S_KEY = 42,
		S_BOOT = 43
	};

	void local_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(mailbox_sync);

	u32 get_reg(unsigned r);
	void set_reg(unsigned r, u32 data);
	void push(u32 data);
	u32 pop();
	u32 alu(unsigned op, u32 a, u32 b, unsigned size);
	u32 host_read(u32 addr, unsigned size);
	void host_write(u32 addr, u32 data, unsigned size);
	void boot();
	void execute_one(u32 op);

	address_space_config const m_local_config;
	required_address_space m_host;
	memory_access<13, 2, -2, ENDIANNESS_BIG>::specific m_local;
	devcb_write_line m_irq_cb;

	u16 m_key;
	u8 m_config;
	u32 m_fetch_xor;
	std::array<u8, 16> m_fetch_lanes;
	k056734_disassembler::descrambler m_fetch;

	int m_icount;
	u32 m_pc;
	u32 m_reg[64];
	u8 m_flags;
	bool m_boot;
};

DECLARE_DEVICE_TYPE(K056734, k056734_device)

#endif // MAME_CPU_K056734_K056734_H
