// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_M6805_M68705_H
#define MAME_CPU_M6805_M68705_H

#pragma once

#include "m6805.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(M68705P3, m68705p3_device)
DECLARE_DEVICE_TYPE(M68705P5, m68705p5_device)
DECLARE_DEVICE_TYPE(M68705R3, m68705r3_device)
DECLARE_DEVICE_TYPE(M68705U3, m68705u3_device)


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> m68705_device

#define MCFG_M68705_PORTA_R_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_r<0>(DEVCB_##obj);

#define MCFG_M68705_PORTB_R_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_r<1>(DEVCB_##obj);

#define MCFG_M68705_PORTC_R_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_r<2>(DEVCB_##obj);

#define MCFG_M68705_PORTD_R_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_r<3>(DEVCB_##obj);

#define MCFG_M68705_PORTA_W_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_w<0>(DEVCB_##obj);

#define MCFG_M68705_PORTB_W_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_w<1>(DEVCB_##obj);

#define MCFG_M68705_PORTC_W_CB(obj) \
	downcast<m68705_device &>(*device).set_port_cb_w<2>(DEVCB_##obj);


class m68705_device : public m6805_base_device, public device_nvram_interface
{
public:
	// configuration helpers
	template<std::size_t N, typename Object> devcb_base &set_port_cb_r(Object &&obj) { return m_port_cb_r[N].set_callback(std::forward<Object>(obj)); }
	template<std::size_t N, typename Object> devcb_base &set_port_cb_w(Object &&obj) { return m_port_cb_w[N].set_callback(std::forward<Object>(obj)); }
	auto porta_r_cb() { return m_port_cb_r[0].bind(); }
	auto portb_r_cb() { return m_port_cb_r[1].bind(); }
	auto portc_r_cb() { return m_port_cb_r[2].bind(); }
	auto portd_r_cb() { return m_port_cb_r[3].bind(); }
	auto porta_w_cb() { return m_port_cb_w[0].bind(); }
	auto portb_w_cb() { return m_port_cb_w[1].bind(); }
	auto portc_w_cb() { return m_port_cb_w[2].bind(); }

protected:
	// state index constants
	enum
	{
		M68705_A = M6805_A,
		M68705_PC = M6805_PC,
		M68705_S = M6805_S,
		M68705_X = M6805_X,
		M68705_CC = M6805_CC,
		M68705_IRQ_STATE = M6805_IRQ_STATE,

		M68705_LATCHA = 0x10,
		M68705_LATCHB,
		M68705_LATCHC,
		M68705_LATCHD,
		M68705_DDRA,
		M68705_DDRB,
		M68705_DDRC,
		M68705_DDRD,

		M68705_PS,
		M68705_TDR,
		M68705_TCR,

		M68705_PCR,
		M68705_PLD,
		M68705_PLA,

		M68705_MOR
	};

	enum
	{
		PORT_COUNT = 4
	};

	m68705_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			u32 addr_width,
			address_map_constructor internal_map);

	template <offs_t B> DECLARE_READ8_MEMBER(eprom_r);
	template <offs_t B> DECLARE_WRITE8_MEMBER(eprom_w);

	template <std::size_t N> void set_port_open_drain(bool value);
	template <std::size_t N> void set_port_mask(u8 mask);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_input_w) { m_port_input[N] = data & ~m_port_mask[N]; }
	template <std::size_t N> DECLARE_READ8_MEMBER(port_r);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_latch_w);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_ddr_w);
	template <std::size_t N> void port_cb_w();

	DECLARE_READ8_MEMBER(tdr_r);
	DECLARE_WRITE8_MEMBER(tdr_w);
	DECLARE_READ8_MEMBER(tcr_r);
	DECLARE_WRITE8_MEMBER(tcr_w);

	DECLARE_READ8_MEMBER(misc_r);
	DECLARE_WRITE8_MEMBER(misc_w);

	DECLARE_READ8_MEMBER(pcr_r);
	DECLARE_WRITE8_MEMBER(pcr_w);

	DECLARE_READ8_MEMBER(acr_r);
	DECLARE_WRITE8_MEMBER(acr_w);
	DECLARE_READ8_MEMBER(arr_r);
	DECLARE_WRITE8_MEMBER(arr_w);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	virtual void interrupt() override;
	virtual void burn_cycles(unsigned count) override;

	u8 *const get_user_rom() const { return &m_user_rom[0]; }
	virtual u8 get_mask_options() const = 0;

	template <std::size_t N> void add_port_latch_state();
	template <std::size_t N> void add_port_ddr_state();
	void add_timer_state();
	void add_eprom_state();

private:
	bool    tcr_tir() const     { return BIT(m_tcr, 7); }
	bool    tcr_tim() const     { return BIT(m_tcr, 6); }
	bool    tcr_tin() const     { return BIT(m_tcr, 5); }
	bool    tcr_tie() const     { return BIT(m_tcr, 4); }
	bool    tcr_topt() const    { return BIT(m_tcr, 3); }
	u8      tcr_ps() const      { return m_tcr & 0x07; }

	bool    pcr_vpon() const    { return !BIT(m_pcr, 2); }
	bool    pcr_pge() const     { return !BIT(m_pcr, 1); }
	bool    pcr_ple() const     { return !BIT(m_pcr, 0); }

	required_region_ptr<u8> m_user_rom;

	// digital I/O
	bool            m_port_open_drain[PORT_COUNT];
	u8              m_port_mask[PORT_COUNT];
	u8              m_port_input[PORT_COUNT];
	u8              m_port_latch[PORT_COUNT];
	u8              m_port_ddr[PORT_COUNT];
	devcb_read8     m_port_cb_r[PORT_COUNT];
	devcb_write8    m_port_cb_w[PORT_COUNT];

	// timer/counter
	u8  m_prescaler;
	u8  m_tdr;
	u8  m_tcr;

	// EPROM control
	u8  m_vihtp;
	u8  m_pcr;
	u8  m_pl_data;
	u16 m_pl_addr;
};


// ======================> m68705p_device

class m68705p_device : public m68705_device
{
public:
	DECLARE_WRITE8_MEMBER(pa_w) { port_input_w<0>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pb_w) { port_input_w<1>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pc_w) { port_input_w<2>(space, offset, data, mem_mask); }

protected:
	void p_map(address_map &map);

	m68705p_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68705u_device

class m68705u_device : public m68705_device
{
public:
	DECLARE_WRITE8_MEMBER(pa_w) { port_input_w<0>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pb_w) { port_input_w<1>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pc_w) { port_input_w<2>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pd_w) { port_input_w<3>(space, offset, data, mem_mask); } // TODO: PD6 is also /INT2

protected:
	void u_map(address_map &map);

	m68705u_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			address_map_constructor internal_map);
	m68705u_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68705r_device

class m68705r_device : public m68705u_device
{
public:
	// TODO: voltage inputs for ADC (shared with digital port D pins)

protected:
	void r_map(address_map &map);

	m68705r_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type);

	virtual void device_start() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


// ======================> m68705p3_device

class m68705p3_device : public m68705p_device
{
public:
	m68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual u8 get_mask_options() const override;
};


// ======================> m68705p5_device

class m68705p5_device : public m68705p_device
{
public:
	m68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual u8 get_mask_options() const override;
};


// ======================> m68705r3_device

class m68705r3_device : public m68705r_device
{
public:
	m68705r3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual u8 get_mask_options() const override;
};


// ======================> m68705u3_device

class m68705u3_device : public m68705u_device
{
public:
	m68705u3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;

	virtual u8 get_mask_options() const override;
};


/****************************************************************************
 * 68705 section
 ****************************************************************************/

#define M68705_IRQ_LINE             (M6805_IRQ_LINE + 0)
#define M68705_INT_TIMER            (M6805_IRQ_LINE + 1)
#define M68705_VPP_LINE             (M6805_IRQ_LINE + 2)
#define M68705_VIHTP_LINE           (M6805_IRQ_LINE + 3)

#endif // MAME_CPU_M6805_M68705_H
