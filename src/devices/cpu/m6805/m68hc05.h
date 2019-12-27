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
	auto uart_tx() { return m_uart_tx_cb.bind(); }
	void uart_rx(u8 data);

	auto sck_out() { return m_sck_out_cb.bind(); }
	auto sda_out() { return m_sda_out_cb.bind(); }
	DECLARE_WRITE_LINE_MEMBER(sck_in);
	DECLARE_WRITE_LINE_MEMBER(sda_in);
	DECLARE_WRITE_LINE_MEMBER(ss_in);

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
		M68HC05_NCOP
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
	DECLARE_READ8_MEMBER(port_read);
	DECLARE_WRITE8_MEMBER(port_latch_w);
	DECLARE_READ8_MEMBER(port_ddr_r);
	DECLARE_WRITE8_MEMBER(port_ddr_w);

	DECLARE_READ8_MEMBER(baud_r);
	DECLARE_WRITE8_MEMBER(baud_w);
	DECLARE_READ8_MEMBER(sccr1_r);
	DECLARE_WRITE8_MEMBER(sccr1_w);
	DECLARE_READ8_MEMBER(sccr2_r);
	DECLARE_WRITE8_MEMBER(sccr2_w);
	DECLARE_READ8_MEMBER(scsr_r);
	DECLARE_READ8_MEMBER(rdr_r);
	DECLARE_WRITE8_MEMBER(tdr_w);

	DECLARE_READ8_MEMBER(spcr_r);
	DECLARE_WRITE8_MEMBER(spcr_w);
	DECLARE_READ8_MEMBER(spsr_r);
	DECLARE_WRITE8_MEMBER(spsr_w);
	DECLARE_READ8_MEMBER(spdr_r);
	DECLARE_WRITE8_MEMBER(spdr_w);

	DECLARE_READ8_MEMBER(tcr_r);
	DECLARE_WRITE8_MEMBER(tcr_w);
	DECLARE_READ8_MEMBER(tsr_r);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_READ8_MEMBER(ocr_r);
	DECLARE_WRITE8_MEMBER(ocr_w);
	DECLARE_READ8_MEMBER(timer_r);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void interrupt() override;
	virtual bool test_il() override;
	virtual void burn_cycles(unsigned count) override;

	void add_port_state(std::array<bool, PORT_COUNT> const &ddr);
	void add_timer_state();

	virtual void run_cop(unsigned count);

	void check_spi_interrupts();

private:
	u8 port_value(unsigned offset) const;
	void update_port_irq();

	bool	sccr1_r8() const	{ return BIT(m_sccr1, 7); }
	bool	sccr1_t8() const	{ return BIT(m_sccr1, 6); }
	bool	sccr1_m() const		{ return BIT(m_sccr1, 4); }
	bool	sccr1_wake() const	{ return BIT(m_sccr1, 3); }

	bool	sccr2_tie() const	{ return BIT(m_sccr2, 7); }
	bool	sccr2_tcie() const	{ return BIT(m_sccr2, 6); }
	bool	sccr2_rie() const	{ return BIT(m_sccr2, 5); }
	bool	sccr2_ilie() const	{ return BIT(m_sccr2, 4); }
	bool	sccr2_te() const	{ return BIT(m_sccr2, 3); }
	bool	sccr2_re() const	{ return BIT(m_sccr2, 2); }
	bool	sccr2_rwu() const	{ return BIT(m_sccr2, 1); }
	bool	sccr2_sbk() const	{ return BIT(m_sccr2, 0); }

	bool	scsr_tdre() const	{ return BIT(m_scsr, 7); }
	bool	scsr_tc() const		{ return BIT(m_scsr, 6); }
	bool	scsr_rdrf() const	{ return BIT(m_scsr, 5); }
	bool	scsr_idle() const	{ return BIT(m_scsr, 4); }
	bool	scsr_or() const		{ return BIT(m_scsr, 3); }
	bool	scsr_nf() const		{ return BIT(m_scsr, 2); }
	bool	scsr_fe() const		{ return BIT(m_scsr, 1); }

	u8		baud_scp() const	{ return (m_baud >> 4) & 0x03; }
	u8		baud_scr() const	{ return m_baud & 0x07; }
	u8		baud_scp_count() const;
	u8		baud_scr_count() const { return 1U << baud_scr(); }

	bool	spcr_spie() const	{ return BIT(m_spcr, 7); }
	bool	spcr_spe() const	{ return BIT(m_spcr, 6); }
	bool	spcr_mstr() const	{ return BIT(m_spcr, 4); }
	bool	spcr_cpol() const	{ return BIT(m_spcr, 3); }
	bool	spcr_cpha() const	{ return BIT(m_spcr, 2); }
	u8		spcr_spr() const	{ return m_spcr & 0x03; }
	u8		spcr_spr_divider() const;

	bool	spsr_spif() const	{ return BIT(m_spsr, 7); }
	bool	spsr_modf() const	{ return BIT(m_spsr, 4); }

	bool    tcr_icie() const    { return BIT(m_tcr, 7); }
	bool    tcr_ocie() const    { return BIT(m_tcr, 6); }
	bool    tcr_toie() const    { return BIT(m_tcr, 5); }
	bool    tcr_iedg() const    { return BIT(m_tcr, 1); }
	bool    tcr_olvl() const    { return BIT(m_tcr, 0); }

	bool    tsr_icf() const     { return BIT(m_tsr, 7); }
	bool    tsr_ocf() const     { return BIT(m_tsr, 6); }
	bool    tsr_tof() const     { return BIT(m_tsr, 5); }

	// digital I/O
	devcb_read8         m_port_cb_r[PORT_COUNT];
	devcb_write8        m_port_cb_w[PORT_COUNT];
	u8                  m_port_bits[PORT_COUNT];
	u8                  m_port_interrupt[PORT_COUNT];
	u8                  m_port_input[PORT_COUNT];
	u8                  m_port_latch[PORT_COUNT];
	u8                  m_port_ddr[PORT_COUNT];
	bool                m_port_irq_state, m_irq_line_state;
	u8                  m_irq_latch;

	// UART
	u8                  m_baud;
	u8					m_sccr1;
	u8					m_sccr2;
	u8					m_scsr;
	u8                  m_rdr;
	u8                  m_tdr;
	bool				m_rdr_pending;
	bool				m_tdr_pending;
	u32					m_uart_tx_clocks;
	u32					m_uart_rx_clocks;
	devcb_write8		m_uart_tx_cb;

	// SPI
	u8					m_spcr;
	u8					m_spsr;
	u8					m_spdr;
	u8					m_sprr;
	u8					m_spi_rx_cnt;
	u8					m_spi_tx_cnt;
	u32					m_spi_tx_clocks;
	u32					m_spi_run_clocks;
	u8					m_sck;
	u8					m_sda;
	u8					m_ss;
	devcb_write_line	m_sck_out_cb;
	devcb_write_line	m_sda_out_cb;

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

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void run_cop(unsigned count) override;

	void add_pcop_state();
	void add_ncop_state();

	void set_ncope(bool state) { m_ncope = state ? 1 : 0; }
	DECLARE_WRITE8_MEMBER(coprst_w);
	DECLARE_READ8_MEMBER(copcr_r);
	DECLARE_WRITE8_MEMBER(copcr_w);
	DECLARE_WRITE8_MEMBER(copr_w);

	bool    copcr_copf() const  { return BIT(m_copcr, 4); }
	bool    copcr_cme() const   { return BIT(m_copcr, 3); }
	bool    copcr_pcope() const { return BIT(m_copcr, 2); }
	u8      copcr_cm() const    { return m_copcr & 0x03; }

	// COP watchdogs
	u32                 m_pcop_cnt, m_ncop_cnt;
	u8                  m_coprst, m_copcr;
	u8                  m_ncope;
};


// ======================> m68hc05c4_device

class m68hc05c4_device : public m68hc05_device
{
public:
	m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c4_map(address_map &map);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05c8_device

class m68hc05c8_device : public m68hc05_device
{
public:
	m68hc05c8_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c8_map(address_map &map);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc705c8a_device

class m68hc705c8a_device : public m68hc705_device
{
public:
	m68hc705c8a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void c8a_map(address_map &map);

	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc705j1a_device

class m68hc705j1a_device : public m68hc705_device
{
public:
	m68hc705j1a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void j1a_map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05l9_device

class m68hc05l9_device : public m68hc05_device
{
public:
	m68hc05l9_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void l9_map(address_map &map);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68hc05l11_device

class m68hc05l11_device : public m68hc05_device
{
public:
	m68hc05l11_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	void l11_map(address_map &map);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


/****************************************************************************
 * 68HC05 section
 ****************************************************************************/

#define M68HC05_IRQ_LINE            (M6805_IRQ_LINE + 0)
#define M68HC05_TCAP_LINE           (M6805_IRQ_LINE + 1)

#endif // MAME_CPU_M6805_M68HC05_H
