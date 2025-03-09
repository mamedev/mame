// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_TLCS900_TMP95C061_H
#define MAME_CPU_TLCS900_TMP95C061_H

#pragma once

#include "tlcs900.h"

DECLARE_DEVICE_TYPE(TMP95C061, tmp95c061_device)


class tmp95c061_device : public tlcs900h_device
{
	static constexpr uint8_t PORT_1 = 0; // 8 bit I/O. Shared with D8-D15
	static constexpr uint8_t PORT_2 = 1; // 8 bit output only. Shared with A16-A23
	static constexpr uint8_t PORT_5 = 2; // 4 bit I/O. Shared with HWR, BUSRQ, BUSAK, RW
	static constexpr uint8_t PORT_6 = 3; // 6 bit I/O. Shared with CS0, CS1, CS3/LCAS, RAS, REFOUT
	static constexpr uint8_t PORT_7 = 4; // 8 bit I/O. Shared with PG0-OUT, PG1-OUT
	static constexpr uint8_t PORT_8 = 5; // 6 bit I/O. Shared with TXD0, TXD1, RXD0, RXD1, CTS0, SCLK0, SCLK1
	static constexpr uint8_t PORT_9 = 6; // 4 bit input only. Shared with AN0-AN3
	static constexpr uint8_t PORT_A = 7; // 4 bit I/O. Shared with WAIT, TI0, TO1, TO2
	static constexpr uint8_t PORT_B = 8; // 8 bit I/O. Shared with TI4/INT4, TI5/INT5, TI6/INT6, TI7/INT7, TO4, TO5, TO6
	static constexpr uint8_t NUM_PORTS = 9;

public:
	// construction/destruction
	tmp95c061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port_read[PORT_1].bind(); }
	auto port1_write() { return m_port_write[PORT_1].bind(); }
	auto port2_write() { return m_port_write[PORT_2].bind(); }
	auto port5_read()  { return m_port_read[PORT_5].bind(); }
	auto port5_write() { return m_port_write[PORT_5].bind(); }
	auto port6_write() { return m_port_write[PORT_6].bind(); }
	auto port7_read()  { return m_port_read[PORT_7].bind(); }
	auto port7_write() { return m_port_write[PORT_7].bind(); }
	auto port8_read()  { return m_port_read[PORT_8].bind(); }
	auto port8_write() { return m_port_write[PORT_8].bind(); }
	auto port9_read()  { return m_port_read[PORT_9].bind(); }
	auto porta_read()  { return m_port_read[PORT_A].bind(); }
	auto porta_write() { return m_port_write[PORT_A].bind(); }
	auto portb_read()  { return m_port_read[PORT_B].bind(); }
	auto portb_write() { return m_port_write[PORT_B].bind(); }
	template <size_t Bit> auto an_read() { return m_an_read[Bit].bind(); }

protected:
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

	void tlcs900_change_tff( int which, int change );
	int tlcs900_process_hdma( int channel );
	void update_porta();

private:
	template <uint8_t> uint8_t port_r();
	template <uint8_t> void port_w(uint8_t data);
	template <uint8_t> void port_cr_w(uint8_t data);
	template <uint8_t> void port_fc_w(uint8_t data);
	uint8_t trun_r();
	void trun_w(uint8_t data);
	void treg01_w(offs_t offset, uint8_t data);
	void t01mod_w(uint8_t data);
	uint8_t tffcr_r();
	void tffcr_w(uint8_t data);
	void treg23_w(offs_t offset, uint8_t data);
	void t23mod_w(uint8_t data);
	uint8_t trdc_r();
	void trdc_w(uint8_t data);
	void treg45_w(offs_t offset, uint8_t data);
	uint8_t cap12_r(offs_t offset);
	uint8_t t4mod_r();
	void t4mod_w(uint8_t data);
	uint8_t t4ffcr_r();
	void t4ffcr_w(uint8_t data);
	uint8_t t45cr_r();
	void t45cr_w(uint8_t data);
	void treg67_w(offs_t offset, uint8_t data);
	uint8_t cap34_r(offs_t offset);
	uint8_t t5mod_r();
	void t5mod_w(uint8_t data);
	uint8_t t5ffcr_r();
	void t5ffcr_w(uint8_t data);
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
	uint8_t adreg_r(offs_t offset);
	uint8_t admod_r();
	void admod_w(uint8_t data);
	uint8_t inte_r(offs_t offset);
	void inte_w(offs_t offset, uint8_t data);
	void iimc_w(uint8_t data);
	void dmav_w(offs_t offset, uint8_t data);
	void bcs_w(offs_t offset, uint8_t data);
	void bexcs_w(uint8_t data);
	uint8_t msar01_r(offs_t offset);
	void msar01_w(offs_t offset, uint8_t data);
	uint8_t msar23_r(offs_t offset);
	void msar23_w(offs_t offset, uint8_t data);
	uint8_t drefcr_r();
	void drefcr_w(uint8_t data);
	uint8_t dmemcr_r();
	void dmemcr_w(uint8_t data);

	void internal_mem(address_map &map) ATTR_COLD;

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<4> m_an_read;

	// I/O Ports
	devcb_read8::array<NUM_PORTS> m_port_read;
	devcb_write8::array<NUM_PORTS> m_port_write;
	uint8_t   m_port_latch[NUM_PORTS];
	uint8_t   m_port_control[NUM_PORTS];
	uint8_t   m_port_function[NUM_PORTS];

	// Timer Control
	uint8_t   m_trun;
	uint8_t   m_t8_reg[4];
	uint8_t   m_t8_mode[2];
	uint8_t   m_t8_invert;
	uint8_t   m_trdc;
	uint8_t   m_to1;
	uint8_t   m_to3;
	uint16_t  m_t16_reg[4];
	uint16_t  m_t16_cap[4];
	uint8_t   m_t16_mode[2];
	uint8_t   m_t16_invert[2];
	uint8_t   m_t45cr;

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
	uint16_t  m_ad_result[4];
	uint8_t   m_ad_mode;

	// Interrupt Control
	uint8_t   m_int_reg[0xb];
	uint8_t   m_iimc;
	uint8_t   m_dma_vector[4];

	// Chip Select/Wait Control
	uint8_t   m_block_cs[4];
	uint8_t   m_external_cs;
	uint8_t   m_mem_start_reg[4];
	uint8_t   m_mem_start_mask[4];

	// DRAM Control
	uint8_t   m_dram_refresh;
	uint8_t   m_dram_access;
};

#endif // MAME_CPU_TLCS900_TMP95C061_H
