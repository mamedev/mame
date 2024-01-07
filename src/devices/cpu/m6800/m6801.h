// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_CPU_M6800_M6801_H
#define MAME_CPU_M6800_M6801_H

#pragma once

#include "cpu/m6800/m6800.h"


enum
{
	M6801_IRQ_LINE = M6800_IRQ_LINE,
	M6801_TIN_LINE, // P20/TIN Input Capture line (edge sense). Active edge is selectable by internal reg.
	M6801_IS3_LINE, // SC1/IOS/IS3 (P54/IS on HD6301Y)
	M6801_STBY_LINE // STBY pin, or internal standby
};

enum
{
	M6803_IRQ_LINE = M6800_IRQ_LINE
};

enum
{
	HD6301_IRQ_LINE = M6800_IRQ_LINE
};

enum
{
	M6801_MODE_0 = 0,
	M6801_MODE_1,
	M6801_MODE_2,
	M6801_MODE_3,
	M6801_MODE_4,
	M6801_MODE_5,
	M6801_MODE_6,
	M6801_MODE_7
};


class m6801_cpu_device : public m6800_cpu_device, public device_nvram_interface
{
public:
	m6801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// port 1-4 I/O, DDR is passed through mem_mask
	auto in_p1_cb() { return m_in_port_func[0].bind(); }
	auto out_p1_cb() { return m_out_port_func[0].bind(); }
	auto in_p2_cb() { return m_in_port_func[1].bind(); }
	auto out_p2_cb() { return m_out_port_func[1].bind(); }
	auto in_p3_cb() { return m_in_port_func[2].bind(); }
	auto out_p3_cb() { return m_out_port_func[2].bind(); }
	auto in_p4_cb() { return m_in_port_func[3].bind(); }
	auto out_p4_cb() { return m_out_port_func[3].bind(); }

	auto out_sc2_cb() { return m_out_sc2_func.bind(); }
	auto out_ser_tx_cb() { return m_out_sertx_func.bind(); }

	void nvram_set_battery(int state) { m_nvram_battery = bool(state); } // default is 1 (nvram_enable_backup needs to be true)
	void nvram_set_default_value(uint8_t val) { m_nvram_defval = val; } // default is 0
	auto standby_cb() { return m_standby_func.bind(); } // notifier (not an output pin)
	int standby() { return suspended(SUSPEND_REASON_CLOCK) ? 1 : 0; }

	void m6801_clock_serial();

protected:
	m6801_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const m6800_cpu_device::op_func *insn, const uint8_t *cycles, address_map_constructor internal, int nvram_bytes);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_execute_interface implementation
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
	virtual uint32_t execute_input_lines() const noexcept override { return 6; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return m6800_cpu_device::execute_input_edge_triggered(inputnum) || inputnum == M6801_STBY_LINE; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void p1_ddr_w(uint8_t data);
	uint8_t p1_data_r();
	void p1_data_w(uint8_t data);
	void p2_ddr_w(uint8_t data);
	uint8_t p2_data_r();
	void p2_data_w(uint8_t data);
	void p3_ddr_w(uint8_t data);
	virtual uint8_t p3_data_r();
	virtual void p3_data_w(uint8_t data);
	uint8_t p3_csr_r();
	void p3_csr_w(uint8_t data);
	void p4_ddr_w(uint8_t data);
	uint8_t p4_data_r();
	void p4_data_w(uint8_t data);

	uint8_t tcsr_r();
	void tcsr_w(uint8_t data);
	uint8_t ch_r();
	uint8_t cl_r();
	void ch_w(uint8_t data);
	void cl_w(uint8_t data);
	uint8_t ocrh_r();
	uint8_t ocrl_r();
	void ocrh_w(uint8_t data);
	void ocrl_w(uint8_t data);
	uint8_t icrh_r();
	uint8_t icrl_r();

	uint8_t sci_rmcr_r();
	void sci_rmcr_w(uint8_t data);
	uint8_t sci_trcsr_r();
	void sci_trcsr_w(uint8_t data);
	uint8_t sci_rdr_r();
	void sci_tdr_w(uint8_t data);

	virtual uint8_t rcr_r();
	virtual void rcr_w(uint8_t data);
	uint8_t ff_r();

	void m6801_io(address_map &map);
	void m6801_mem(address_map &map);
	void m6801u4_mem(address_map &map);
	void hd6801_mem(address_map &map);
	void m6803_mem(address_map &map);

	devcb_read8::array<4> m_in_port_func;
	devcb_write8::array<4> m_out_port_func;

	devcb_write_line m_out_sc2_func;
	devcb_write_line m_out_sertx_func;
	devcb_write_line m_standby_func;

	required_shared_ptr<uint8_t> m_internal_ram;
	const int m_nvram_bytes;
	uint8_t m_nvram_defval;
	bool m_nvram_battery;
	int m_sclk_divider;

	/* internal registers */
	uint8_t  m_port_ddr[4];
	uint8_t  m_port_data[4];
	uint8_t  m_p3csr;          // Port 3 Control/Status Register
	uint8_t  m_tcsr;           /* Timer Control and Status Register */
	uint8_t  m_pending_tcsr;   /* pending IRQ flag for clear IRQflag process */
	uint8_t  m_irq2;           /* IRQ2 flags */
	uint8_t  m_ram_ctrl;
	PAIR     m_counter;        /* free running counter */
	PAIR     m_output_compare; /* output compare       */
	uint16_t m_input_capture;  /* input capture        */
	bool     m_pending_isf_clear;
	int      m_port3_latched;
	bool     m_port2_written;

	uint8_t  m_trcsr, m_rmcr, m_rdr, m_tdr, m_rsr, m_tsr;
	int      m_rxbits, m_txbits, m_txstate, m_trcsr_read_tdre, m_trcsr_read_orfe, m_trcsr_read_rdrf, m_tx, m_ext_serclock;
	bool     m_use_ext_serclock;

	uint8_t  m_latch09;
	int      m_is3_state;

	PAIR     m_timer_over;
	emu_timer *m_sci_timer;
	uint32_t m_timer_next;     /* point of next timer event */

	static const uint8_t cycles_6803[256];
	static const uint8_t cycles_63701[256];
	static const op_func m6803_insn[256];
	static const op_func hd63701_insn[256];

	virtual void m6800_check_irq2() override;
	virtual void increment_counter(int amount) override;
	virtual void eat_cycles() override;
	virtual void cleanup_counters() override;

	virtual void modified_tcsr();
	virtual void set_timer_event();
	virtual void modified_counters();
	virtual void check_timer_event();
	virtual void set_rmcr(uint8_t data);
	virtual void write_port2();
	int m6800_rx();
	void serial_transmit();
	void serial_receive();
	TIMER_CALLBACK_MEMBER( sci_tick );
	void set_os3(int state);
};


class m6801u4_cpu_device : public m6801_cpu_device
{
public:
	m6801u4_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class hd6801v0_cpu_device : public m6801_cpu_device
{
public:
	hd6801v0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class m6803_cpu_device : public m6801_cpu_device
{
public:
	m6803_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


class m6803e_cpu_device : public m6801_cpu_device
{
public:
	m6803e_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return cycles; }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


class mc68120_device : public m6801_cpu_device
{
public:
	mc68120_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// dual-ported RAM access
	uint8_t dpram_r(offs_t offset);
	void dpram_w(offs_t offset, uint8_t data);

protected:
	mc68120_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);
};


class mc68121_device : public mc68120_device
{
public:
	mc68121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class hd6301_cpu_device : public m6801_cpu_device
{
protected:
	hd6301_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const m6800_cpu_device::op_func *insn, const uint8_t *cycles, address_map_constructor internal, int nvram_bytes);

	// device_nvram_interface implementation
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void take_trap() override;
};


// DP-40 package: HD6301V1P,  HD63A01V1P,  HD63B01V1P
// FP-54 package: HD6301V1F,  HD63A01V1F,  HD63B01V1F
// CG-40 package: HD6301V1CG, HD63A01V1CG, HD63B01V1CG
// CP-52 package: HD6301V1CP, HD63A01V1CP, HD63B01V1CP
// CP-44 package: HD6301V1L,  HD63A01V1L,  HD63B01V1L
// Not fully emulated yet
class hd6301v1_cpu_device : public hd6301_cpu_device
{
public:
	hd6301v1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// DC-40 package: HD63701V0C, HD63A701V0C, HD63B701V0C
// Not fully emulated yet
class hd63701v0_cpu_device : public hd6301_cpu_device
{
public:
	hd63701v0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// DP-40 package: HD6303RP,  HD63A03RP,  HD63B03RP
// FP-54 package: HD6303RF,  HD63A03RF,  HD63B03RF
// CG-40 package: HD6303RCG, HD63A03RCG, HD63B03RCG
// Not fully emulated yet
class hd6303r_cpu_device : public hd6301_cpu_device
{
public:
	hd6303r_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class hd6301x_cpu_device : public hd6301_cpu_device
{
public:
	// port 5-7 I/O, DDR is passed through mem_mask
	auto in_p5_cb() { return m_in_portx_func[0].bind(); }
	auto out_p5_cb() { return m_out_portx_func[0].bind(); }
	auto in_p6_cb() { return m_in_portx_func[1].bind(); }
	auto out_p6_cb() { return m_out_portx_func[1].bind(); }
	auto out_p7_cb() { return m_out_portx_func[2].bind(); }

protected:
	hd6301x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal, int nvram_bytes);

	// device_nvram_interface implementation
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	void hd6301x_io(address_map &map);
	void hd6303x_io(address_map &map);
	void hd6301x_mem(address_map &map);
	void hd6303x_mem(address_map &map);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void write_port2() override;

	void p2_ddr_2bit_w(uint8_t data);
	void p3_ddr_1bit_w(uint8_t data);
	virtual uint8_t p3_data_r() override;
	virtual void p3_data_w(uint8_t data) override;
	virtual uint8_t p5_data_r();
	void p6_ddr_w(uint8_t data);
	virtual uint8_t p6_data_r();
	virtual void p6_data_w(uint8_t data);
	uint8_t p7_data_r();
	void p7_data_w(uint8_t data);
	virtual uint8_t rcr_r() override;

	uint8_t tcsr2_r();
	void tcsr2_w(uint8_t data);
	uint8_t ocr2h_r();
	void ocr2h_w(uint8_t data);
	uint8_t ocr2l_r();
	void ocr2l_w(uint8_t data);

	void increment_t2cnt(int amount);
	uint8_t t2cnt_r();
	void t2cnt_w(uint8_t data);
	void tconr_w(uint8_t data);
	uint8_t tcsr3_r();
	void tcsr3_w(uint8_t data);

	virtual void m6800_check_irq2() override;
	virtual void modified_tcsr() override;
	virtual void set_timer_event() override;
	virtual void modified_counters() override;
	virtual void increment_counter(int amount) override;
	virtual void check_timer_event() override;
	virtual void cleanup_counters() override;
	virtual void set_rmcr(uint8_t data) override;

	devcb_read8::array<2> m_in_portx_func;
	devcb_write8::array<3> m_out_portx_func;

	uint8_t m_portx_ddr[2];
	uint8_t m_portx_data[3];

	uint8_t m_tcsr2;
	uint8_t m_pending_tcsr2;
	PAIR    m_output_compare2;

	uint8_t m_t2cnt;
	uint8_t m_tconr;
	uint8_t m_tcsr3;
	bool m_tout3;
	bool m_t2cnt_written;
};


// DP-64S package: HD6301X0P,  HD63A01X0P,  HD63B01X0P
// FP-80  package: HD6301X0F,  HD63A01X0F,  HD63B01X0F
// CP-68  package: HD6301X0CP, HD63A01X0CP, HD63B01X0CP
// Not fully emulated yet
class hd6301x0_cpu_device : public hd6301x_cpu_device
{
public:
	hd6301x0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hd6301x0_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal, int nvram_bytes);

	virtual void device_reset() override;
};


// DC-64S package: HD63701X0C, HD63A701X0C, HD63B701X0C
// Not fully emulated yet
class hd63701x0_cpu_device : public hd6301x0_cpu_device
{
public:
	hd63701x0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// DP-64S package: HD6303XP,  HD63A03XP,  HD63B03XP
// FP-80  package: HD6303XF,  HD63A03XF,  HD63B03XF
// CP-68  package: HD6303XCP, HD63A03XCP, HD63B03XCP
// Not fully emulated yet
class hd6303x_cpu_device : public hd6301x_cpu_device
{
public:
	hd6303x_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class hd6301y_cpu_device : public hd6301x_cpu_device
{
protected:
	hd6301y_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal, int nvram_bytes);

	void hd6301y_io(address_map &map);
	void hd6303y_io(address_map &map);
	void hd6301y_mem(address_map &map);
	void hd6303y_mem(address_map &map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	void p1_ddr_1bit_w(uint8_t data);
	void p5_ddr_w(uint8_t data);
	virtual uint8_t p5_data_r() override;
	void p5_data_w(uint8_t data);
	virtual uint8_t p6_data_r() override;
	virtual void p6_data_w(uint8_t data) override;
	uint8_t p6_csr_r();
	void p6_csr_w(uint8_t data);
	virtual uint8_t rcr_r() override;
	virtual void rcr_w(uint8_t data) override;

	virtual void m6800_check_irq2() override;
	void clear_pending_isf();

	uint8_t m_p6csr;
};


// DP-64S package: HD6301Y0P,  HD63A01Y0P,  HD63B01Y0P,  HD63C01Y0P
// FP-64  package: HD6301Y0F,  HD63A01Y0F,  HD63B01Y0F,  HD63C01Y0F
// FP-64A package: HD6301Y0H,  HD63A01Y0H,  HD63B01Y0H,  HD63C01Y0H
// CP-68  package: HD6301Y0CP, HD63A01Y0CP, HD63B01Y0CP, HD63C01Y0CP
// Not fully emulated yet
class hd6301y0_cpu_device : public hd6301y_cpu_device
{
public:
	hd6301y0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// DC-64S package: HD63701Y0C, HD63A701Y0C, HD63B701Y0C, HD63C701Y0C
// Not fully emulated yet
class hd63701y0_cpu_device : public hd6301y_cpu_device
{
public:
	hd63701y0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// DP-64S package: HD6303YP,  HD63A03YP,  HD63B03YP,  HD63C03YP
// FP-64  package: HD6303YF,  HD63A03YF,  HD63B03YF,  HD63C03YF
// FP-64A package: HD6303YH,  HD63A03YH,  HD63B03YH,  HD63C03YH
// CP-68  package: HD6303YCP, HD63A03YCP, HD63B03YCP, HD63C03YCP
// Not fully emulated yet
class hd6303y_cpu_device : public hd6301y_cpu_device
{
public:
	hd6303y_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(M6801, m6801_cpu_device)
DECLARE_DEVICE_TYPE(M6801U4, m6801u4_cpu_device)
DECLARE_DEVICE_TYPE(HD6801V0, hd6801v0_cpu_device)
DECLARE_DEVICE_TYPE(M6803, m6803_cpu_device)
DECLARE_DEVICE_TYPE(M6803E, m6803e_cpu_device)
DECLARE_DEVICE_TYPE(MC68120, mc68120_device)
DECLARE_DEVICE_TYPE(MC68121, mc68121_device)
DECLARE_DEVICE_TYPE(HD6301V1, hd6301v1_cpu_device)
DECLARE_DEVICE_TYPE(HD6301X0, hd6301x0_cpu_device)
DECLARE_DEVICE_TYPE(HD6301Y0, hd6301y0_cpu_device)
DECLARE_DEVICE_TYPE(HD63701V0, hd63701v0_cpu_device)
DECLARE_DEVICE_TYPE(HD63701X0, hd63701x0_cpu_device)
DECLARE_DEVICE_TYPE(HD63701Y0, hd63701y0_cpu_device)
DECLARE_DEVICE_TYPE(HD6303R, hd6303r_cpu_device)
DECLARE_DEVICE_TYPE(HD6303X, hd6303x_cpu_device)
DECLARE_DEVICE_TYPE(HD6303Y, hd6303y_cpu_device)

#endif // MAME_CPU_M6800_M6801_H
