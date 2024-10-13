// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_TLCS900_TMP95C063_H
#define MAME_CPU_TLCS900_TMP95C063_H

#pragma once

#include "tlcs900.h"

DECLARE_DEVICE_TYPE(TMP95C063, tmp95c063_device)


class tmp95c063_device : public tlcs900h_device
{
	static constexpr uint8_t PORT_1 = 0; // 8 bit I/O. Shared with d8-d15
	static constexpr uint8_t PORT_2 = 1; // 8 bit output only. Shared with a16-a23
	static constexpr uint8_t PORT_5 = 2; // 6 bit I/O
	static constexpr uint8_t PORT_6 = 3; // 8 bit I/O. Shared with cs1, cs3 & dram control
	static constexpr uint8_t PORT_7 = 4; // 8 bit I/O
	static constexpr uint8_t PORT_8 = 5; // 8 bit I/O. Shared with SCOUT, WAIT, NMI2, INT0-INT3
	static constexpr uint8_t PORT_9 = 6; // 8 bit I/O. Shared with clock input and output for the 8-bit timers
	static constexpr uint8_t PORT_A = 7; // 8 bit I/O. Shared with serial channels 0/1
	static constexpr uint8_t PORT_B = 8; // 8 bit I/O. Shared with 16bit timers
	static constexpr uint8_t PORT_C = 9; // 8 bit input only. Shared with analogue inputs
	static constexpr uint8_t PORT_D = 10; // 5 bit I/O. Shared with INT8
	static constexpr uint8_t PORT_E = 11; // 8 bit I/O.
	static constexpr uint8_t NUM_PORTS = 12;

public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port_read[PORT_1].bind(); }
	auto port1_write() { return m_port_write[PORT_1].bind(); }
	auto port2_write() { return m_port_write[PORT_2].bind(); }
	auto port5_read()  { return m_port_read[PORT_5].bind(); }
	auto port5_write() { return m_port_write[PORT_5].bind(); }
	auto port6_read()  { return m_port_read[PORT_6].bind(); }
	auto port6_write() { return m_port_write[PORT_6].bind(); }
	auto port7_read()  { return m_port_read[PORT_7].bind(); }
	auto port7_write() { return m_port_write[PORT_7].bind(); }
	auto port8_read()  { return m_port_read[PORT_8].bind(); }
	auto port8_write() { return m_port_write[PORT_8].bind(); }
	auto port9_read()  { return m_port_read[PORT_9].bind(); }
	auto port9_write() { return m_port_write[PORT_9].bind(); }
	auto porta_read()  { return m_port_read[PORT_A].bind(); }
	auto porta_write() { return m_port_write[PORT_A].bind(); }
	auto portb_read()  { return m_port_read[PORT_B].bind(); }
	auto portb_write() { return m_port_write[PORT_B].bind(); }
	auto portc_read()  { return m_port_read[PORT_C].bind(); }
	auto portd_read()  { return m_port_read[PORT_D].bind(); }
	auto portd_write() { return m_port_write[PORT_D].bind(); }
	auto porte_read()  { return m_port_read[PORT_E].bind(); }
	auto porte_write() { return m_port_write[PORT_E].bind(); }
	template <size_t Bit> auto an_read() { return m_an_read[Bit].bind(); }

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

private:
	template <uint8_t> uint8_t port_r();
	template <uint8_t> void port_w(uint8_t data);
	template <uint8_t> void port_cr_w(uint8_t data);
	template <uint8_t> void port_fc_w(uint8_t data);
	uint8_t t8run_r();
	void t8run_w(uint8_t data);
	void treg01_w(offs_t offset, uint8_t data);
	uint8_t t01mod_r();
	void t01mod_w(uint8_t data);
	uint8_t t02ffcr_r();
	void t02ffcr_w(uint8_t data);
	void treg23_w(offs_t offset, uint8_t data);
	uint8_t t23mod_r();
	void t23mod_w(uint8_t data);
	uint8_t trdc_r();
	void trdc_w(uint8_t data);
	void treg45_w(offs_t offset, uint8_t data);
	uint8_t t45mod_r();
	void t45mod_w(uint8_t data);
	uint8_t t46ffcr_r();
	void t46ffcr_w(uint8_t data);
	void treg67_w(offs_t offset, uint8_t data);
	uint8_t t67mod_r();
	void t67mod_w(uint8_t data);
	void treg89_w(offs_t offset, uint8_t data);
	uint8_t cap12_r(offs_t offset);
	uint8_t t8mod_r();
	void t8mod_w(uint8_t data);
	uint8_t t8ffcr_r();
	void t8ffcr_w(uint8_t data);
	uint8_t t89cr_r();
	void t89cr_w(uint8_t data);
	uint8_t t16run_r();
	void t16run_w(uint8_t data);
	void tregab_w(offs_t offset, uint8_t data);
	uint8_t cap34_r(offs_t offset);
	uint8_t t9mod_r();
	void t9mod_w(uint8_t data);
	uint8_t t9ffcr_r();
	void t9ffcr_w(uint8_t data);
	uint8_t pgreg_r(offs_t offset);
	void pgreg_w(offs_t offset, uint8_t data);
	uint8_t pg01cr_r();
	void pg01cr_w(uint8_t data);
	uint8_t wdmod_r();
	void wdmod_w(uint8_t data);
	void wdcr_w(uint8_t data);
	uint8_t sc0buf_r();
	void sc0buf_w(uint8_t data);
	uint8_t sc0cr_r();
	void sc0cr_w(uint8_t data);
	uint8_t sc0mod_r();
	void sc0mod_w(uint8_t data);
	uint8_t br0cr_r();
	void br0cr_w(uint8_t data);
	uint8_t sc1buf_r();
	void sc1buf_w(uint8_t data);
	uint8_t sc1cr_r();
	void sc1cr_w(uint8_t data);
	uint8_t sc1mod_r();
	void sc1mod_w(uint8_t data);
	uint8_t br1cr_r();
	void br1cr_w(uint8_t data);
	uint8_t ode_r();
	void ode_w(uint8_t data);
	uint8_t admod1_r();
	void admod1_w(uint8_t data);
	uint8_t admod2_r();
	void admod2_w(uint8_t data);
	uint8_t adreg_r(offs_t offset);
	uint8_t inte_r(offs_t offset);
	void inte_w(offs_t offset, uint8_t data);
	void iimc_w(uint8_t data);
	void dmav_w(offs_t offset, uint8_t data);
	void bcs_w(offs_t offset, uint8_t data);
	void bexcs_w(uint8_t data);
	uint8_t msar_r(offs_t offset);
	void msar_w(offs_t offset, uint8_t data);
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

	void internal_mem(address_map &map) ATTR_COLD;

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<8> m_an_read;

	// I/O Ports
	devcb_read8::array<NUM_PORTS> m_port_read;
	devcb_write8::array<NUM_PORTS> m_port_write;
	uint8_t   m_port_latch[NUM_PORTS];
	uint8_t   m_port_control[NUM_PORTS];
	uint8_t   m_port_function[NUM_PORTS];

	// Timer Control
	uint8_t   m_t8run;
	uint8_t   m_t8_reg[8];
	uint8_t   m_t8_mode[4];
	uint8_t   m_t8_invert[2];
	uint8_t   m_trdc;
	uint16_t  m_t16_reg[4];
	uint16_t  m_t16_cap[4];
	uint8_t   m_t16_mode[2];
	uint8_t   m_t16_invert[2];
	uint8_t   m_t89cr;
	uint8_t   m_t16run;

	// Pattern Generator
	uint8_t   m_pgreg[2];
	uint8_t   m_pg01cr;

	// Watchdog Timer
	uint8_t   m_watchdog_mode;

	// Serial Channel
	uint8_t   m_serial_control[2];
	uint8_t   m_serial_mode[2];
	uint8_t   m_baud_rate[2];
	uint8_t   m_od_enable;

	// A/D Converter Control
	uint8_t   m_ad_mode1;
	uint8_t   m_ad_mode2;
	uint16_t  m_ad_result[4];

	// Interrupt Control
	uint8_t   m_int_reg[0xf];
	uint8_t   m_iimc;
	uint8_t   m_dma_vector[4];

	// Chip Select/Wait Control
	uint8_t   m_block_cs[4];
	uint8_t   m_external_cs;
	uint8_t   m_mem_start_reg[4];
	uint8_t   m_mem_start_mask[4];

	// DRAM Control
	uint8_t   m_dram_refresh[2];
	uint8_t   m_dram_access[2];

	// D/A Converter Control
	uint8_t   m_da_drive;
};

#endif // MAME_CPU_TLCS900_TMP95C063_H
