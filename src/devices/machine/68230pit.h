// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstr??m
/**********************************************************************
*
*   Motorola MC68230 PI/T Parallel Interface and Timer
*
*                           _____   _____
*                   D5   1 |*    \_/     | 48  D4
*                   D6   2 |             | 47  D3
*                   D7   3 |             | 46  D2
*                  PA0   4 |             | 45  D1
*                  PA1   5 |             | 44  D0
*                  PA2   6 |             | 43  R/W*
*                  PA3   7 |             | 42  DTACK*
*                  PA4   8 |             | 41  CS*
*                  PA5   9 |             | 40  CLK
*                  PA6  10 |             | 39  RESET*
*                  PA7  11 |             | 38  VSS
*                  Vcc  12 |   TS68230   | 37  PC7/TIACK*
*                   H1  13 |   SC87845   | 36  PC6/PIACK*
*                   H2  14 |             | 35  PC5/PIRQ*
*                   H3  15 |             | 34  PC4/DMAREQ*
*                   H4  16 |             | 33  PC3/TOUT
*                  PB0  17 |             | 32  PC2/TIN
*                  PB1  18 |             | 31  PC1
*                  PB2  19 |             | 30  PC0
*                  PB3  20 |             | 29  RS1
*                  PB4  21 |             | 28  RS2
*                  PB5  22 |             | 27  RS3
*                  PB6  23 |             | 26  RS4
*                  PB7  24 |_____________| 25  RS5
*
**********************************************************************/

#pragma once

#ifndef __68230PIT_H__
#define __68230PIT_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PIT68230_PA_INPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pa_in_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_PA_OUTPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pa_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_PB_INPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pb_in_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_PB_OUTPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pb_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_PC_INPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pc_in_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_PC_OUTPUT_CB(_devcb) \
		devcb = &pit68230_device::set_pc_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_H1_CB(_devcb) \
		devcb = &pit68230_device::set_h1_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_H2_CB(_devcb) \
		devcb = &pit68230_device::set_h2_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_H3_CB(_devcb) \
		devcb = &pit68230_device::set_h3_out_callback (*device, DEVCB_##_devcb);

#define MCFG_PIT68230_H4_CB(_devcb) \
		devcb = &pit68230_device::set_h4_out_callback (*device, DEVCB_##_devcb);

/*-----------------------------------------------------------------------
 * Registers                RS1-RS5   R/W Description
 * -------------------------------------------------------------------------*/
#define PIT_68230_PGCR        0x00 /* RW Port General Control register   */
#define PIT_68230_PSRR        0x01 /* RW Port Service Request register   */
#define PIT_68230_PADDR       0x02 /* RW Port A Data Direction register  */
#define PIT_68230_PBDDR       0x03 /* RW Port B Data Direction register  */
#define PIT_68230_PCDDR       0x04 /* RW Port C Data Direction register  */
#define PIT_68230_PIVR        0x05 /* RW Port Interrupt vector register  */
#define PIT_68230_PACR        0x06 /* RW Port A Control register         */
#define PIT_68230_PBCR        0x07 /* RW Port B Control register         */
#define PIT_68230_PADR        0x08 /* RW Port A Data register            */
#define PIT_68230_PBDR        0x09 /* RW Port B Data register            */
#define PIT_68230_PAAR        0x0a /* RO Port A Alternate register       */
#define PIT_68230_PBAR        0x0b /* RO Port B Alternate register       */
#define PIT_68230_PCDR        0x0c /* RW Port C Data register            */
#define PIT_68230_PSR         0x0d /* RW Port Status register            */
#define PIT_68230_TCR         0x10 /* RW Timer Control Register          */
#define PIT_68230_TIVR        0x11 /* RW Timer Interrupt Vector Register */
#define PIT_68230_CPRH        0x13 /* RW Counter Preload Register High   */
#define PIT_68230_CPRM        0x14 /* RW Counter Preload Register Middle */
#define PIT_68230_CPRL        0x15 /* RW Counter Preload Register Low    */
#define PIT_68230_CNTRH       0x17 /* RO Counter Register High           */
#define PIT_68230_CNTRM       0x18 /* RO Counter Register Middle         */
#define PIT_68230_CNTRL       0x19 /* RO Counter Register Low            */
#define PIT_68230_TSR         0x1A /* RW Timer Status Register           */

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class pit68230_device :  public device_t//, public device_execute_interface
{
	public:
	// construction/destruction
	pit68230_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, const char *shortname, const char *source);
	pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<class _Object> static devcb_base &set_pa_in_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pa_in_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_pa_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pa_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_pb_in_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pb_in_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_pb_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pb_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_pc_in_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pc_in_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_pc_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_pc_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_h1_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_h1_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_h2_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_h2_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_h3_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_h3_out_cb.set_callback (object); }
	template<class _Object> static devcb_base &set_h4_out_callback (device_t &device, _Object object){ return downcast<pit68230_device &>(device).m_h4_out_cb.set_callback (object); }

	DECLARE_WRITE8_MEMBER (write);
	DECLARE_READ8_MEMBER (read);

	void h1_set (uint8_t state);
	void portb_setbit (uint8_t bit, uint8_t state);

	void wr_pitreg_pgcr(uint8_t data);
	void wr_pitreg_psrr(uint8_t data);
	void wr_pitreg_paddr(uint8_t data);
	void wr_pitreg_pbddr(uint8_t data);
	void wr_pitreg_pcddr(uint8_t data);
	void wr_pitreg_pivr(uint8_t data);
	void wr_pitreg_pacr(uint8_t data);
	void wr_pitreg_pbcr(uint8_t data);
	void wr_pitreg_padr(uint8_t data);
	void wr_pitreg_pbdr(uint8_t data);
	void wr_pitreg_paar(uint8_t data);
	void wr_pitreg_pbar(uint8_t data);
	void wr_pitreg_pcdr(uint8_t data);
	void wr_pitreg_psr(uint8_t data);
	void wr_pitreg_tcr(uint8_t data);
	void wr_pitreg_tivr(uint8_t data);
	void wr_pitreg_cprh(uint8_t data);
	void wr_pitreg_cprm(uint8_t data);
	void wr_pitreg_cprl(uint8_t data);
	void wr_pitreg_tsr(uint8_t data);

	uint8_t rr_pitreg_pgcr();
	uint8_t rr_pitreg_psrr();
	uint8_t rr_pitreg_paddr();
	uint8_t rr_pitreg_pbddr();
	uint8_t rr_pitreg_pcddr();
	uint8_t rr_pitreg_pivr();
	uint8_t rr_pitreg_pacr();
	uint8_t rr_pitreg_pbcr();
	uint8_t rr_pitreg_padr();
	uint8_t rr_pitreg_pbdr();
	uint8_t rr_pitreg_paar();
	uint8_t rr_pitreg_pbar();
	uint8_t rr_pitreg_pcdr();
	uint8_t rr_pitreg_psr();
	uint8_t rr_pitreg_tcr();
	uint8_t rr_pitreg_tivr();
	uint8_t rr_pitreg_cprh();
	uint8_t rr_pitreg_cprm();
	uint8_t rr_pitreg_cprl();
	uint8_t rr_pitreg_cntrh();
	uint8_t rr_pitreg_cntrm();
	uint8_t rr_pitreg_cntrl();
	uint8_t rr_pitreg_tsr();

protected:

	enum {
		REG_TCR_ENABLE          = 0x01,
		REG_TCR_CC_MASK         = 0x06,
		REG_TCR_CC_PC2_CLK_PSC  = 0x00,
		REG_TCR_CC_TEN_CLK_PSC  = 0x02,
		REG_TCR_CC_TIN_PSC      = 0x04,
		REG_TCR_CC_TIN_RAW      = 0x06,
		REG_TCR_ZR              = 0x08,
		REG_TCR_ZD              = 0x10,
		REG_TCR_TOUT_TIACK_MASK = 0xe0, // 1 1 1
		REG_TCR_PC3_PC7         = 0x00, // 0 0 0
		REG_TCR_PC3_PC7_DC      = 0x20, // 0 0 1
		REG_TCR_TOUT_PC7_SQ     = 0x40, // 0 1 0
		REG_TCR_TOUT_PC7_SQ_DC  = 0x60, // 0 1 1
		REG_TCR_TOUT_TIACK      = 0x80, // 1 0 0
		REG_TCR_TOUT_TIACK_INT  = 0xa0, // 1 0 1
		REG_TCR_TOUT_PC7        = 0xc0, // 1 1 0
		REG_TCR_TOUT_PC7_INT    = 0xe0, // 1 1 1
	};

	// device-level overrides
	virtual void device_start () override;
	virtual void device_reset () override;
	virtual void device_timer (emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	int m_icount;

	devcb_write8        m_pa_out_cb;
	devcb_read8         m_pa_in_cb;
	devcb_write8        m_pb_out_cb;
	devcb_read8         m_pb_in_cb;
	devcb_write8        m_pc_out_cb;
	devcb_read8         m_pc_in_cb;
	devcb_write_line    m_h1_out_cb;
	devcb_write_line    m_h2_out_cb;
	devcb_write_line    m_h3_out_cb;
	devcb_write_line    m_h4_out_cb;

	// peripheral ports
	uint8_t m_pgcr;           // Port General Control register
	uint8_t m_psrr;           // Port Service Request register
	uint8_t m_paddr;          // Port A Data Direction register
	uint8_t m_pbddr;          // Port B Data Direction register
	uint8_t m_pcddr;          // Port C Data Direction register
	uint8_t m_pivr;           // Ports Interrupt vector
	uint8_t m_pacr;           // Port A Control register
	uint8_t m_pbcr;           // Port B Control register
	uint8_t m_padr;           // Port A Data register
	uint8_t m_pbdr;           // Port B Data register
	uint8_t m_pcdr;           // Port C Data register
	uint8_t m_psr;            // Port Status Register
	uint8_t m_tcr;        // Timer Control Register
	uint8_t m_tivr;       // Timer Interrupt Vector register
	int m_cpr;          // Counter Preload Registers (3 x 8 = 24 bits)
	int   m_cntr;       // - The 24 bit Counter
	uint8_t m_tsr;        // Timer Status Register

	// Timers
	emu_timer *pit_timer;

	enum
	{
		TIMER_ID_PIT
	};
};

// device type definition
extern const device_type PIT68230;

#endif /* __68230PIT_H__ */
