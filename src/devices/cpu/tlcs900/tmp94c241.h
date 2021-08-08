// license:BSD-3-Clause
// copyright-holders:AJR,Wilbert Pol,Felipe Sanches
/****************************************************************************

    Toshiba TMP94C241 microcontroller

****************************************************************************/

#ifndef MAME_CPU_TLCS900_TMP94C241_H
#define MAME_CPU_TLCS900_TMP94C241_H

#pragma once

#include "tlcs900.h"

#define TMP94C241_INTE45   0
#define TMP94C241_INTE67   1
#define TMP94C241_INTE89   2
#define TMP94C241_INTEAB   3
#define TMP94C241_INTET01  4
#define TMP94C241_INTET23  5
#define TMP94C241_INTET45  6
#define TMP94C241_INTET67  7
#define TMP94C241_INTET89  8
#define TMP94C241_INTETAB  9
#define TMP94C241_INTES0   10
#define TMP94C241_INTES1   11
#define TMP94C241_INTETC01 12
#define TMP94C241_INTETC23 13
#define TMP94C241_INTETC45 14
#define TMP94C241_INTETC67 15
#define TMP94C241_INTE0AD  16
#define TMP94C241_INTNMWDT 17

// Flip-Flop operations:
#define FF_INVERT	0b00
#define FF_SET		0b01
#define FF_CLEAR	0b10
#define FF_DONTCARE 0b11

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tmp94c241_device

class tmp94c241_device : public tlcs900h_device
{
public:
	// device type constructor
	tmp94c241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto port0_read()  { return m_port0_read.bind(); }
	auto port0_write() { return m_port0_write.bind(); }
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_read()  { return m_port2_read.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port3_read()  { return m_port3_read.bind(); }
	auto port3_write() { return m_port3_write.bind(); }
	auto port4_read()  { return m_port4_read.bind(); }
	auto port4_write() { return m_port4_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_read()  { return m_port6_read.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }
	auto portc_read()  { return m_portc_read.bind(); }
	auto portc_write() { return m_portc_write.bind(); }
	auto portd_read()  { return m_portd_read.bind(); }
	auto portd_write() { return m_portd_write.bind(); }
	auto porte_read()  { return m_porte_read.bind(); }
	auto porte_write() { return m_porte_write.bind(); }
	auto portf_read()  { return m_portf_read.bind(); }
	auto portf_write() { return m_portf_write.bind(); }
	auto portg_read()  { return m_portg_read.bind(); }
	auto porth_read()  { return m_porth_read.bind(); }
	auto porth_write() { return m_porth_write.bind(); }
	auto portz_read()  { return m_portz_read.bind(); }
	auto portz_write() { return m_portz_write.bind(); }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

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
	uint8_t p0_r();
	void p0_w(uint8_t data);
	void p0cr_w(uint8_t data);
	void p0fc_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	void p1cr_w(uint8_t data);
	void p1fc_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
	void p2cr_w(uint8_t data);
	void p2fc_w(uint8_t data);
	uint8_t p3_r();
	void p3_w(uint8_t data);
	void p3cr_w(uint8_t data);
	void p3fc_w(uint8_t data);
	uint8_t p4_r();
	void p4_w(uint8_t data);
	void p4cr_w(uint8_t data);
	void p4fc_w(uint8_t data);
	uint8_t p5_r();
	void p5_w(uint8_t data);
	void p5cr_w(uint8_t data);
	void p5fc_w(uint8_t data);
	uint8_t p6_r();
	void p6_w(uint8_t data);
	void p6cr_w(uint8_t data);
	void p6fc_w(uint8_t data);
	uint8_t p7_r();
	void p7_w(uint8_t data);
	void p7cr_w(uint8_t data);
	void p7fc_w(uint8_t data);
	uint8_t p8_r();
	void p8_w(uint8_t data);
	void p8cr_w(uint8_t data);
	void p8fc_w(uint8_t data);
	uint8_t pa_r();
	void pa_w(uint8_t data);
	void pacr_w(uint8_t data);
	void pafc_w(uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	void pbcr_w(uint8_t data);
	void pbfc_w(uint8_t data);
	uint8_t pc_r();
	void pc_w(uint8_t data);
	void pccr_w(uint8_t data);
	void pcfc_w(uint8_t data);
	uint8_t pd_r();
	void pd_w(uint8_t data);
	void pdcr_w(uint8_t data);
	void pdfc_w(uint8_t data);
	uint8_t pe_r();
	void pe_w(uint8_t data);
	void pecr_w(uint8_t data);
	void pefc_w(uint8_t data);
	uint8_t pf_r();
	void pf_w(uint8_t data);
	void pfcr_w(uint8_t data);
	void pffc_w(uint8_t data);
	uint8_t pg_r();
	uint8_t ph_r();
	void ph_w(uint8_t data);
	void phcr_w(uint8_t data);
	void phfc_w(uint8_t data);
	uint8_t pz_r();
	void pz_w(uint8_t data);
	void pzcr_w(uint8_t data);
	uint8_t t8run_r();
	void t8run_w(uint8_t data);
	void treg01_w(offs_t offset, uint8_t data);
	void treg23_w(offs_t offset, uint8_t data);
	uint8_t t01mod_r();
	void t01mod_w(uint8_t data);
	uint8_t t02ffcr_r();
	void t02ffcr_w(uint8_t data);
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
	void treg45_w(offs_t offset, uint16_t data);
	void treg67_w(offs_t offset, uint16_t data);
	void treg89_w(offs_t offset, uint16_t data);
	void tregab_w(offs_t offset, uint16_t data);
	uint8_t cap45_r(offs_t offset);
	uint8_t cap67_r(offs_t offset);
	uint8_t cap89_r(offs_t offset);
	uint8_t capab_r(offs_t offset);
	uint8_t t16run_r();
	void t16run_w(uint8_t data);
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
	uint8_t intnmwdt_r(offs_t offset);
	void intnmwdt_w(offs_t offset, uint8_t data);
	void iimc_w(uint8_t data);
	void intclr_w(uint8_t data);
	void dmav_w(offs_t offset, uint8_t data);
	void b0cs_w(offs_t offset, uint8_t data);
	void b1cs_w(offs_t offset, uint8_t data);
	void b2cs_w(offs_t offset, uint8_t data);
	void b3cs_w(offs_t offset, uint8_t data);
	void b4cs_w(offs_t offset, uint8_t data);
	void b5cs_w(offs_t offset, uint8_t data);
	uint8_t msar0_r();
	void msar0_w(offs_t offset, uint8_t data);
	uint8_t mamr0_r();
	void mamr0_w(offs_t offset, uint8_t data);
	uint8_t msar1_r();
	void msar1_w(offs_t offset, uint8_t data);
	uint8_t mamr1_r();
	void mamr1_w(offs_t offset, uint8_t data);
	uint8_t msar2_r();
	void msar2_w(offs_t offset, uint8_t data);
	uint8_t mamr2_r();
	void mamr2_w(offs_t offset, uint8_t data);
	uint8_t msar3_r();
	void msar3_w(offs_t offset, uint8_t data);
	uint8_t mamr3_r();
	void mamr3_w(offs_t offset, uint8_t data);
	uint8_t msar4_r();
	void msar4_w(offs_t offset, uint8_t data);
	uint8_t mamr4_r();
	void mamr4_w(offs_t offset, uint8_t data);
	uint8_t msar5_r();
	void msar5_w(offs_t offset, uint8_t data);
	uint8_t mamr5_r();
	void mamr5_w(offs_t offset, uint8_t data);
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

	// Port 0: 8 bit I/O. Shared with d0-d7
	devcb_read8    m_port0_read;
	devcb_write8   m_port0_write;

	// Port 1: 8 bit I/O. Shared with d8-d15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit I/O. Shared with d16-d23
	devcb_read8    m_port2_read;
	devcb_write8   m_port2_write;

	// Port 3: 8 bit I/O. Shared with d24-d31
	devcb_read8    m_port3_read;
	devcb_write8   m_port3_write;

	// Port 4: 8 bit I/O. Shared with a0-a7
	devcb_read8    m_port4_read;
	devcb_write8   m_port4_write;

	// Port 5: 8 bit I/O. Shared with a8-a15
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 8 bit I/O. Shared with a16-a23
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O. Shared with external memory & bus signals 
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 7 bit I/O. Shared with chip-select signals
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port A: 5 bit I/O. Shared with external DRAM (channel 0)
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 5 bit I/O. Shared with external DRAM (channel 1)
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;

	// Port C: 2 bit I/O. Shared with outputs for 8-bit or 16-bit timers
	devcb_read8    m_portc_read;
	devcb_write8   m_portc_write;

	// Port D: 6 bit I/O. Shared with 16-bit timer I/O and interrupt input
	devcb_read8    m_portd_read;
	devcb_write8   m_portd_write;

	// Port E: 6 bit I/O. Shared with 8-bit or 16-bit timer output and interrupt input
	devcb_read8    m_porte_read;
	devcb_write8   m_porte_write;

	// Port F: 6 bit I/O. Shared with I/O functions of serial interface
	devcb_read8    m_portf_read;
	devcb_write8   m_portf_write;

	// Port G: 8 bit input-only. Shared with AD converter.
	devcb_read8    m_portg_read;

	// Port H: 5 bit I/O. Shared with /INT0 and micro DMA
	devcb_read8    m_porth_read;
	devcb_write8   m_porth_write;

	// Port Z: 8 bit I/O.
	devcb_read8    m_portz_read;
	devcb_write8   m_portz_write;

	// analogue inputs, sampled at 10 bits
	devcb_read16::array<8> m_an_read;

	// I/O Port Control
	uint8_t   m_port_latch[0x13];
	uint8_t   m_port_control[0x13];
	uint8_t   m_port_function[0x13];
	
	// Timer Control
	uint8_t   m_t8run;
	uint8_t   m_t8_reg[8];
	uint8_t   m_tmod[8];
	uint8_t   m_ffcr[5];
	uint8_t   m_trdc;
	uint16_t  m_t16_reg[8];
	uint16_t  m_t16_cap[8];
	uint8_t   m_t16run;
	uint16_t  m_timer16[4];

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
	uint8_t   m_int_reg[18];
	uint8_t   m_iimc;
	uint8_t   m_dma_vector[4];

	// Chip Select/Wait Control
	uint16_t   m_block_cs[6];
	uint8_t   m_external_cs;
	uint8_t   m_mem_start_reg[6];
	uint8_t   m_mem_start_mask[6];

	// DRAM Control
	uint8_t   m_dram_refresh[2];
	uint8_t   m_dram_access[2];

	// D/A Converter Control
	uint8_t   m_da_drive;
};

// device type declaration
DECLARE_DEVICE_TYPE(TMP94C241, tmp94c241_device)

#endif // MAME_CPU_TLCS900_TMP94C241_H
