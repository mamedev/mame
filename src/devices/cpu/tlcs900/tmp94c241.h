// license:BSD-3-Clause
// copyright-holders:AJR,Felipe Sanches
/****************************************************************************

    Toshiba TMP94C241 microcontroller

****************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP94C241_H
#define MAME_CPU_TLCS900_TMP94C241_H

#pragma once

#include "tlcs900.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tmp94c241_device

class tmp94c241_device : public tlcs900h_device
{
	static constexpr uint8_t PORT_0 = 0; // 8 bit I/O. Shared with d0-d7
	static constexpr uint8_t PORT_1 = 1; // 8 bit I/O. Shared with d8-d15
	static constexpr uint8_t PORT_2 = 2; // 8 bit I/O. Shared with d16-d23
	static constexpr uint8_t PORT_3 = 3; // 8 bit I/O. Shared with d24-d31
	static constexpr uint8_t PORT_4 = 4; // 8 bit I/O. Shared with a0-a7
	static constexpr uint8_t PORT_5 = 5; // 8 bit I/O. Shared with a8-a15
	static constexpr uint8_t PORT_6 = 6; // 8 bit I/O. Shared with a16-a23
	static constexpr uint8_t PORT_7 = 7; // 8 bit I/O. Shared with external memory & bus signals
	static constexpr uint8_t PORT_8 = 8; // 7 bit I/O. Shared with chip-select signals
	static constexpr uint8_t PORT_A = 9; // 5 bit I/O. Shared with external DRAM (channel 0)
	static constexpr uint8_t PORT_B = 10; // 5 bit I/O. Shared with external DRAM (channel 1)
	static constexpr uint8_t PORT_C = 11; // 2 bit I/O. Shared with outputs for 8-bit or 16-bit timers
	static constexpr uint8_t PORT_D = 12; // 6 bit I/O. Shared with 16-bit timer I/O and interrupt input
	static constexpr uint8_t PORT_E = 13; // 6 bit I/O. Shared with 8-bit or 16-bit timer output and interrupt input
	static constexpr uint8_t PORT_F = 14; // 6 bit I/O. Shared with I/O functions of serial interface
	static constexpr uint8_t PORT_G = 15; // 8 bit input-only. Shared with AD converter.
	static constexpr uint8_t PORT_H = 16; // 5 bit I/O. Shared with /INT0 and micro DMA
	static constexpr uint8_t PORT_Z = 17; // 8 bit I/O.
	static constexpr uint8_t NUM_PORTS = 18;

	static constexpr uint8_t TREG0 = 0;
	static constexpr uint8_t TREG1 = 1;
	static constexpr uint8_t TREG2 = 2;
	static constexpr uint8_t TREG3 = 3;
	static constexpr uint8_t TREG4 = 0;
	static constexpr uint8_t TREG5 = 1;
	static constexpr uint8_t TREG6 = 2;
	static constexpr uint8_t TREG7 = 3;
	static constexpr uint8_t TREG8 = 4;
	static constexpr uint8_t TREG9 = 5;
	static constexpr uint8_t TREGA = 6;
	static constexpr uint8_t TREGB = 7;

	static constexpr uint8_t CAP4 = 0;
	static constexpr uint8_t CAP5 = 1;
	static constexpr uint8_t CAP6 = 2;
	static constexpr uint8_t CAP7 = 3;
	static constexpr uint8_t CAP8 = 4;
	static constexpr uint8_t CAP9 = 5;
	static constexpr uint8_t CAPA = 6;
	static constexpr uint8_t CAPB = 7;

public:
	// device type constructor
	tmp94c241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto port0_read()  { return m_port_read[PORT_0].bind(); }
	auto port0_write() { return m_port_write[PORT_0].bind(); }
	auto port1_read()  { return m_port_read[PORT_1].bind(); }
	auto port1_write() { return m_port_write[PORT_1].bind(); }
	auto port2_read()  { return m_port_read[PORT_2].bind(); }
	auto port2_write() { return m_port_write[PORT_2].bind(); }
	auto port3_read()  { return m_port_read[PORT_3].bind(); }
	auto port3_write() { return m_port_write[PORT_3].bind(); }
	auto port4_read()  { return m_port_read[PORT_4].bind(); }
	auto port4_write() { return m_port_write[PORT_4].bind(); }
	auto port5_read()  { return m_port_read[PORT_5].bind(); }
	auto port5_write() { return m_port_write[PORT_5].bind(); }
	auto port6_read()  { return m_port_read[PORT_6].bind(); }
	auto port6_write() { return m_port_write[PORT_6].bind(); }
	auto port7_read()  { return m_port_read[PORT_7].bind(); }
	auto port7_write() { return m_port_write[PORT_7].bind(); }
	auto port8_read()  { return m_port_read[PORT_8].bind(); }
	auto port8_write() { return m_port_write[PORT_8].bind(); }
	auto porta_read()  { return m_port_read[PORT_A].bind(); }
	auto porta_write() { return m_port_write[PORT_A].bind(); }
	auto portb_read()  { return m_port_read[PORT_B].bind(); }
	auto portb_write() { return m_port_write[PORT_B].bind(); }
	auto portc_read()  { return m_port_read[PORT_C].bind(); }
	auto portc_write() { return m_port_write[PORT_C].bind(); }
	auto portd_read()  { return m_port_read[PORT_D].bind(); }
	auto portd_write() { return m_port_write[PORT_D].bind(); }
	auto porte_read()  { return m_port_read[PORT_E].bind(); }
	auto porte_write() { return m_port_write[PORT_E].bind(); }
	auto portf_read()  { return m_port_read[PORT_F].bind(); }
	auto portf_write() { return m_port_write[PORT_F].bind(); }
	auto portg_read()  { return m_port_read[PORT_G].bind(); }
	auto porth_read()  { return m_port_read[PORT_H].bind(); }
	auto porth_write() { return m_port_write[PORT_H].bind(); }
	auto portz_read()  { return m_port_read[PORT_Z].bind(); }
	auto portz_write() { return m_port_write[PORT_Z].bind(); }

protected:
	// device_t implementation
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_set_input(int inputnum, int state) override;

	// tlcs900_device overrides
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void change_timer_flipflop(uint8_t flipflop, uint8_t operation);

	// Ports
	template <uint8_t P> uint8_t port_r();
	template <uint8_t P> void port_w(uint8_t data);
	template <uint8_t P> void port_cr_w(uint8_t data);
	template <uint8_t P> void port_fc_w(uint8_t data);

	// Special Function Registers
	template <uint8_t N> void bNcs_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template <uint8_t Timer> uint16_t cap_r();
	template <uint8_t N> uint8_t mamr_r();
	template <uint8_t N> void mamr_w(uint8_t data);
	template <uint8_t N> uint8_t msar_r();
	template <uint8_t N> void msar_w(uint8_t data);
	template <uint8_t Timer> void treg_8_w(uint8_t data);
	template <uint8_t Timer> void treg_16_w(uint16_t data);
	template <uint8_t Channel> uint8_t scNbuf_r();
	template <uint8_t Channel> void scNbuf_w(uint8_t data);
	template <uint8_t Channel> uint8_t scNcr_r();
	template <uint8_t Channel> void scNcr_w(uint8_t data);
	template <uint8_t Channel> uint8_t scNmod_r();
	template <uint8_t Channel> void scNmod_w(uint8_t data);
	template <uint8_t Channel> uint8_t brNcr_r();
	template <uint8_t Channel> void brNcr_w(uint8_t data);
	uint8_t t8run_r();
	void t8run_w(uint8_t data);
	uint8_t t01mod_r();
	void t01mod_w(uint8_t data);
	uint8_t tffcr_r();
	void tffcr_w(uint8_t data);
	uint8_t t23mod_r();
	void t23mod_w(uint8_t data);
	uint8_t trdc_r();
	void trdc_w(uint8_t data);
	uint8_t t4mod_r();
	uint8_t t6mod_r();
	uint8_t t8mod_r();
	uint8_t tamod_r();
	void t4mod_w(uint8_t data);
	void t6mod_w(uint8_t data);
	void t8mod_w(uint8_t data);
	void tamod_w(uint8_t data);
	uint8_t t4ffcr_r();
	void t4ffcr_w(uint8_t data);
	uint8_t t8ffcr_r();
	void t8ffcr_w(uint8_t data);
	uint8_t t6ffcr_r();
	void t6ffcr_w(uint8_t data);
	uint8_t taffcr_r();
	void taffcr_w(uint8_t data);
	uint8_t t16run_r();
	void t16run_w(uint8_t data);
	uint8_t wdmod_r();
	void wdmod_w(uint8_t data);
	void wdcr_w(uint8_t data);
	uint8_t ode_r();
	void ode_w(uint8_t data);
	uint8_t admod1_r();
	void admod1_w(uint8_t data);
	uint8_t admod2_r();
	void admod2_w(uint8_t data);
	uint8_t adreg_r(offs_t offset);
	uint8_t inte_r(offs_t offset);
	void inte_w(offs_t offset, uint8_t data);
	uint8_t intnmwdt_r(offs_t offset);
	void intnmwdt_w(offs_t offset, uint8_t data);
	void iimc_w(uint8_t data);
	void intclr_w(uint8_t data);
	void dmav_w(offs_t offset, uint8_t data);
	uint8_t drefcr1_r();
	void drefcr1_w(uint8_t data);
	uint8_t dmemcr1_r();
	void dmemcr1_w(uint8_t data);
	uint8_t drefcr3_r();
	void drefcr3_w(uint8_t data);
	uint8_t dmemcr3_r();
	void dmemcr3_w(uint8_t data);
	uint8_t dadrv_r();
	void dadrv_w(uint8_t data);
	void dareg_w(offs_t offset, uint8_t data);

	void internal_mem(address_map &map);

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<8> m_an_read;

	// I/O Ports
	devcb_read8::array<NUM_PORTS> m_port_read;
	devcb_write8::array<NUM_PORTS> m_port_write;
	uint8_t m_port_latch[NUM_PORTS];
	uint8_t m_port_control[NUM_PORTS];
	uint8_t m_port_function[NUM_PORTS];

	// Timer Control
	bool m_timer_flipflops[12];
	uint8_t m_t8run;
	uint8_t m_t01mod;
	uint8_t m_t23mod;
	uint8_t m_t4mod;
	uint8_t m_t6mod;
	uint8_t m_t8mod;
	uint8_t m_tamod;
	uint8_t m_tffcr;
	uint8_t m_t4ffcr;
	uint8_t m_t6ffcr;
	uint8_t m_t8ffcr;
	uint8_t m_taffcr;
	uint8_t m_trdc;
	uint8_t m_t16run;
	uint8_t m_treg_8[4]; // TREG0 - TREG3
	uint16_t m_treg_16[8]; //TREG4 - TREGB
	uint16_t m_t16_cap[8]; //CAP4 - CAPB
	uint16_t m_timer_16[4]; // UPCOUNTERA - UPCOUNTERB

	// Watchdog Timer
	uint8_t m_watchdog_mode;

	// Serial Channel
	uint8_t m_serial_control[2];
	uint8_t m_serial_mode[2];
	uint8_t m_baud_rate[2];
	uint8_t m_od_enable;

	// A/D Converter Control
	uint8_t m_ad_mode1;
	uint8_t m_ad_mode2;
	uint16_t m_ad_result[4];

	// Interrupt Control
	uint8_t m_int_reg[18];
	uint8_t m_iimc;
	uint8_t m_dma_vector[4];

	// Chip Select/Wait Control
	uint16_t m_block_cs[6];
	uint8_t m_external_cs;
	uint8_t m_msar[6];
	uint8_t m_mamr[6];

	// DRAM Control
	uint8_t m_dram_refresh[2];
	uint8_t m_dram_access[2];

	// D/A Converter Control
	uint8_t m_da_drive;
};

// device type declaration
DECLARE_DEVICE_TYPE(TMP94C241, tmp94c241_device)

#endif // MAME_CPU_TLCS900_TMP94C241_H
