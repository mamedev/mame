// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_CPU_M68000_M68KCOMMON_H
#define MAME_CPU_M68000_M68KCOMMON_H

#pragma once

/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 *
 * If set_interrupt_mixer(false) has been called, the 3 interrupt lines
 * are modeled instead, as numbers 0-2.
 */
constexpr int M68K_IRQ_NONE = 0;
constexpr int M68K_IRQ_1    = 1;
constexpr int M68K_IRQ_2    = 2;
constexpr int M68K_IRQ_3    = 3;
constexpr int M68K_IRQ_4    = 4;
constexpr int M68K_IRQ_5    = 5;
constexpr int M68K_IRQ_6    = 6;
constexpr int M68K_IRQ_7    = 7;

constexpr int M68K_IRQ_IPL0 = 0;
constexpr int M68K_IRQ_IPL1 = 1;
constexpr int M68K_IRQ_IPL2 = 2;

enum
{
	// NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP
	M68K_PC = STATE_GENPC, M68K_SP = 1, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_IR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7,
	M68K_FP0, M68K_FP1, M68K_FP2, M68K_FP3, M68K_FP4, M68K_FP5, M68K_FP6, M68K_FP7,
	M68K_FPSR, M68K_FPCR, M68K_CRP_LIMIT, M68K_CRP_APTR, M68K_SRP_LIMIT, M68K_SRP_APTR,
	M68K_MMU_TC, M68K_TT0, M68K_TT1, M68K_MMU_SR, M68K_ITT0, M68K_ITT1,
	M68K_DTT0, M68K_DTT1, M68K_URP_APTR
};

class m68000_base_device : public cpu_device
{
public:
	enum {
		AS_CPU_SPACE = 4,
		AS_USER_PROGRAM = 5,
		AS_USER_OPCODES = 6,
	};

	static constexpr u8 autovector(int level) { return 0x18 + level; }
	void autovectors_map(address_map &map) ATTR_COLD;

	void set_cpu_space(int space_id) { m_cpu_space_id = space_id; }
	void set_interrupt_mixer(bool enable) { m_interrupt_mixer = enable; }
	auto reset_cb() { return m_reset_cb.bind(); }

	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return m_interrupt_mixer ? inputnum == M68K_IRQ_7 : false; }

	virtual bool supervisor_mode() const noexcept = 0;
	virtual u16 get_fc() const noexcept = 0;

protected:
	bool   m_interrupt_mixer; // Indicates whether to put a virtual 8->3 priority mixer on the input lines
	int    m_cpu_space_id;    // CPU space address space id
	devcb_write_line m_reset_cb;

	m68000_base_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock) :
		cpu_device(mconfig, type, tag, owner, clock),
		m_interrupt_mixer(true),
		m_cpu_space_id(AS_CPU_SPACE),
		m_reset_cb(*this)
	{ }
};

#endif
