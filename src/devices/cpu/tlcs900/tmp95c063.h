// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_TLCS900_TMP95C063_H
#define MAME_CPU_TLCS900_TMP95C063_H

#pragma once

#include "tlcs900.h"

DECLARE_DEVICE_TYPE(TMP95C063, tmp95c063_device)


class tmp95c063_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_read()  { return m_port6_read.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto port9_read()  { return m_port9_read.bind(); }
	auto port9_write() { return m_port9_write.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }
	auto portc_read()  { return m_portc_read.bind(); }
	auto portd_read()  { return m_portd_read.bind(); }
	auto portd_write() { return m_portd_write.bind(); }
	auto porte_read()  { return m_porte_read.bind(); }
	auto porte_write() { return m_porte_write.bind(); }
	template <size_t Bit> auto an_read() { return m_an_read[Bit].bind(); }

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

private:
	uint8_t p1_r();
	void p1_w(uint8_t data);
	void p1cr_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	void p2fc_w(uint8_t data);
	uint8_t p5_r();
	void p5_w(uint8_t data);
	void p5cr_w(uint8_t data);
	void p5fc_w(uint8_t data);
	uint8_t p6_r();
	void p6_w(uint8_t data);
	void p6fc_w(uint8_t data);
	uint8_t p7_r();
	void p7_w(uint8_t data);
	void p7cr_w(uint8_t data);
	void p7fc_w(uint8_t data);
	uint8_t p8_r();
	void p8_w(uint8_t data);
	void p8cr_w(uint8_t data);
	void p8fc_w(uint8_t data);
	uint8_t p9_r();
	void p9_w(uint8_t data);
	void p9cr_w(uint8_t data);
	void p9fc_w(uint8_t data);
	uint8_t pa_r();
	void pa_w(uint8_t data);
	void pacr_w(uint8_t data);
	void pafc_w(uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	void pbcr_w(uint8_t data);
	void pbfc_w(uint8_t data);
	uint8_t pc_r();
	uint8_t pd_r();
	void pd_w(uint8_t data);
	void pdcr_w(uint8_t data);
	uint8_t pe_r();
	void pe_w(uint8_t data);
	void pecr_w(uint8_t data);
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

	void internal_mem(address_map &map);

	// Port 1: 8 bit I/O. Shared with d8-d15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit output only. Shared with a16-a23
	devcb_write8   m_port2_write;

	// Port 5: 6 bit I/O
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 8 bit I/O. Shared with cs1, cs3 & dram control
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 8 bit I/O. Shared with SCOUT, WAIT, NMI2, INT0-INT3
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port 9: 8 bit I/O. Shared with clock input and output for the 8-bit timers
	devcb_read8    m_port9_read;
	devcb_write8   m_port9_write;

	// Port A: 8 bit I/O. Shared with serial channels 0/1
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 8 bit I/O. Shared with 16bit timers
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;

	// Port C: 8 bit input only. Shared with analogue inputs
	devcb_read8    m_portc_read;

	// Port D: 5 bit I/O. Shared with int8_t
	devcb_read8    m_portd_read;
	devcb_write8   m_portd_write;

	// Port E: 8 bit I/O.
	devcb_read8    m_porte_read;
	devcb_write8   m_porte_write;

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<8> m_an_read;

	// I/O Port Control
	uint8_t   m_port_latch[0xf];
	uint8_t   m_port_control[0xf];
	uint8_t   m_port_function[0xf];

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
