// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_M6805_M68HC05_H
#define MAME_CPU_M6805_M68HC05_H

#pragma once

#include "m6805.h"

#include <array>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(M68HC05C4,   m68hc05c4_device)
DECLARE_DEVICE_TYPE(M68HC05C8,   m68hc05c8_device)
DECLARE_DEVICE_TYPE(M68HC705C4A, m68hc705c4a_device)
DECLARE_DEVICE_TYPE(M68HC705C8A, m68hc705c8a_device)
DECLARE_DEVICE_TYPE(M68HC705J1A, m68hc705j1a_device)
DECLARE_DEVICE_TYPE(M68HC05L9,   m68hc05l9_device)
DECLARE_DEVICE_TYPE(M68HC05L11,  m68hc05l11_device)


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> m68hc05_device

class m68hc05_device : public m6805_base_device
{
public:
	//  configuration helpers
	auto porta_r() { return m_port_cb_r[0].bind(); }
	auto portb_r() { return m_port_cb_r[1].bind(); }
	auto portc_r() { return m_port_cb_r[2].bind(); }
	auto portd_r() { return m_port_cb_r[3].bind(); }
	auto porta_w() { return m_port_cb_w[0].bind(); }
	auto portb_w() { return m_port_cb_w[1].bind(); }
	auto portc_w() { return m_port_cb_w[2].bind(); }
	auto portd_w() { return m_port_cb_w[3].bind(); }
	auto tcmp() { return m_tcmp_cb.bind(); }

protected:
	// state index constants
	enum
	{
		M68HC05_A = M6805_A,
		M68HC05_PC = M6805_PC,
		M68HC05_S = M6805_S,
		M68HC05_X = M6805_X,
		M68HC05_CC = M6805_CC,
		M68HC05_IRQ_STATE = M6805_IRQ_STATE,

		M68HC05_IRQLATCH = 0x10,

		M68HC05_LATCHA,
		M68HC05_LATCHB,
		M68HC05_LATCHC,
		M68HC05_LATCHD,
		M68HC05_DDRA,
		M68HC05_DDRB,
		M68HC05_DDRC,
		M68HC05_DDRD,

		M68HC05_TCR,
		M68HC05_TSR,
		M68HC05_ICR,
		M68HC05_OCR,
		M68HC05_PS,
		M68HC05_TR,

		M68HC05_COPRST,
		M68HC05_COPCR,
		M68HC05_PCOP,
		M68HC05_NCOPE,
		M68HC05_NCOP,

		M68HC705C8A_OPTION
	};

	enum { PORT_COUNT = 4 };

	m68hc05_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			u32 addr_width,
			u16 vector_mask,
			address_map_constructor internal_map);

	void set_port_bits(std::array<u8, PORT_COUNT> const &bits);
	void set_port_interrupt(std::array<u8, PORT_COUNT> const &interrupt);
	u8 port_read(offs_t offset);
	void port_latch_w(offs_t offset, u8 data);
	u8 port_ddr_r(offs_t offset);
	void port_ddr_w(offs_t offset, u8 data);

	u8 tcr_r();
	void tcr_w(u8 data);
	u8 tsr_r();
	u8 icr_r(offs_t offset);
	u8 ocr_r(offs_t offset);
	void ocr_w(offs_t offset, u8 data);
	u8 timer_r(offs_t offset);

	void set_ncope(bool state) { m_ncope = state ? 1 : 0; }
	void coprst_w(u8 data);
	u8 copcr_r();
	void copcr_w(u8 data);
	void copr_w(u8 data);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void interrupt() override;
	virtual bool test_il() override;
	virtual void burn_cycles(unsigned count) override;

	void add_port_state(std::array<bool, PORT_COUNT> const &ddr);
	void add_timer_state();
	void add_pcop_state();
	void add_ncop_state();

private:
	u8 port_value(unsigned offset) const;
	void update_port_irq();

	bool    tcr_icie() const    { return BIT(m_tcr, 7); }
	bool    tcr_ocie() const    { return BIT(m_tcr, 6); }
	bool    tcr_toie() const    { return BIT(m_tcr, 5); }
	bool    tcr_iedg() const    { return BIT(m_tcr, 1); }
	bool    tcr_olvl() const    { return BIT(m_tcr, 0); }

	bool    tsr_icf() const     { return BIT(m_tsr, 7); }
	bool    tsr_ocf() const     { return BIT(m_tsr, 6); }
	bool    tsr_tof() const     { return BIT(m_tsr, 5); }

	bool    copcr_copf() const  { return BIT(m_copcr, 4); }
	bool    copcr_cme() const   { return BIT(m_copcr, 3); }
	bool    copcr_pcope() const { return BIT(m_copcr, 2); }
	u8      copcr_cm() const    { return m_copcr & 0x03; }

	// digital I/O
	devcb_read8::array<PORT_COUNT> m_port_cb_r;
	devcb_write8::array<PORT_COUNT> m_port_cb_w;
	u8                  m_port_bits[PORT_COUNT];
	u8                  m_port_interrupt[PORT_COUNT];
	u8                  m_port_input[PORT_COUNT];
	u8                  m_port_latch[PORT_COUNT];
	u8                  m_port_ddr[PORT_COUNT];
	bool                m_port_irq_state, m_irq_line_state;
	u8                  m_irq_latch;

	// timer/counter
	devcb_write_line    m_tcmp_cb;
	bool                m_tcap_state;
	u8                  m_tcr;
	u8                  m_tsr, m_tsr_seen;
	u8                  m_prescaler;
	u16                 m_counter, m_icr, m_ocr;
	bool                m_inhibit_cap, m_inhibit_cmp;
	u8                  m_trl_buf[2];
	bool                m_trl_latched[2];

	// COP watchdogs
	u32                 m_pcop_cnt, m_ncop_cnt;
	u8                  m_coprst, m_copcr;
	u8                  m_ncope;
};


// ======================> m68hc705_device

class m68hc705_device : public m68hc05_device
{
protected:
	m68hc705_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			u32 addr_width,
			address_map_constructor internal_map);
};


// ======================> m68hc05c4_device

class m68hc05c4_device : public m68hc05_device
{
public:
	m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c4_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05c8_device

class m68hc05c8_device : public m68hc05_device
{
public:
	m68hc05c8_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c8_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc705c4a_device

class m68hc705c4a_device : public m68hc705_device
{
public:
	m68hc705c4a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c4a_map(address_map &map) ATTR_COLD;

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	required_region_ptr<u8> m_rom;

	u8 m_option;
};


// ======================> m68hc705c8a_device

class m68hc705c8a_device : public m68hc705_device
{
public:
	m68hc705c8a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c8a_map(address_map &map) ATTR_COLD;

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	required_region_ptr<u8> m_rom;

	u8 ram0_r(offs_t offset);
	void ram0_w(offs_t offset, u8 data);
	u8 ram1_r(offs_t offset);
	void ram1_w(offs_t offset, u8 data);

	u8 m_ram[0x80];
	u8 m_option;
};


// ======================> m68hc705j1a_device

class m68hc705j1a_device : public m68hc705_device
{
public:
	m68hc705j1a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void j1a_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05l9_device

class m68hc05l9_device : public m68hc05_device
{
public:
	m68hc05l9_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void l9_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05l11_device

class m68hc05l11_device : public m68hc05_device
{
public:
	m68hc05l11_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void l11_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


/****************************************************************************
 * 68HC05 section
 ****************************************************************************/

#define M68HC05_IRQ_LINE            (M6805_IRQ_LINE + 0)
#define M68HC05_TCAP_LINE           (M6805_IRQ_LINE + 1)

#endif // MAME_CPU_M6805_M68HC05_H
