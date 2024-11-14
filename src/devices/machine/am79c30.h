// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    AMD Am79C30A Digital Subscriber Controller (DSC)

*****************************************************************************

                                    LS2
              EAR1  --------------+  |  +--------------  LS1
              EAR2  -----------+  |  |  |  +-----------  AREF
              AINA  --------+  |  |  |  |  |  +--------  LIN1
              AINB  -----+  |  |  |  |  |  |  |  +-----  LIN2
              AVss  --+  |  |  |  |  |  |  |  |  |  +--  HSW
                     _|__|__|__|__|__|__|__|__|__|__|_
                    / 6  5  4  3  2  1 44 43 42 41 40 \
                   /----------------------------------|
            CAP1 --|  7              *             39 |-- LOUT1
            CAP2 --|  8                            38 |-- LOUT2
            AVcc --|  9                            37 |-- AVss
            DVcc --| 10                            36 |-- DVss
           RESET --| 11                            35 |-- _INT
             _CS --| 12          Am79C30A          34 |-- XTAL1
             _RD --| 13       (not to scale)       33 |-- XTAL2
             _WR --| 14                            32 |-- MCLK
            DVss --| 15                            31 |-- SFS
              A2 --| 16                            30 |-- SCLK
              A1 --| 17                            29 |-- SBOUT
                   |----------------------------------|
                   \ 18 19 20 21 22 23 24 25 26 27 28 /
                    --|--|--|--|--|--|--|--|--|--|--|-
                 A0 --+  |  |  |  |  |  |  |  |  |  +--  SBIN
                 D7 -----+  |  |  |  |  |  |  |  +-----  D0
                 D6 --------+  |  |  |  |  |  +--------  D1
                 D5 -----------+  |  |  |  +-----------  D2
                 D4 --------------+  |  +--------------  D3
                                BCL/CH2STRB

        Pins 2–8 are reserved (not to be connected) on the Am79C32A.

****************************************************************************/

#ifndef MAME_MACHINE_AM79C30_H
#define MAME_MACHINE_AM79C30_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> am79c30a_device

class am79c30a_device : public device_t
{
public:
	// device type constructor
	am79c30a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto int_callback() { return m_int_callback.bind(); }

	// microprocessor interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 ir_r();
	void dctb_w(u8 data);
	u8 dcrb_r();
	u8 dsr1_r();
	u8 dsr2_r();
	u8 der_r();
	void cr_w(u8 data);
	u8 dr_r();
	void dr_w(u8 data);
	u8 bbrb_r();
	void bbtb_w(u8 data);
	u8 bcrb_r();
	void bctb_w(u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// lookup tables
	static const char *const s_mcr_channels[16];

	// indirect read/write helpers
	void set_init(u8 data);
	void set_init2(u8 data);
	u8 get_lsr();
	void set_lpr(u8 data);
	void set_lmr1(u8 data);
	void set_lmr2(u8 data);
	void set_mf(u8 data);
	void set_mfqb(u8 data);
	void set_mcr(unsigned n, u8 data);
	void set_mcr4(u8 data);
	void set_x_coeff(unsigned n, u8 data, bool msb);
	void set_r_coeff(unsigned n, u8 data, bool msb);
	void set_gx_coeff(u8 data, bool msb);
	void set_gr_coeff(u8 data, bool msb);
	void set_ger_coeff(u8 data, bool msb);
	void set_stgr(u8 data, bool msb);
	void set_ftgr(unsigned n, u8 data);
	void set_atgr(unsigned n, u8 data);
	void set_mmr1(u8 data);
	void set_mmr2(u8 data);
	void set_mmr3(u8 data);
	void set_stra(u8 data);
	void set_strf(u8 data);
	void set_tar(u8 data, bool msb);
	void set_frar(unsigned n, u8 data);
	void set_srar(unsigned n, u8 data);
	void set_drlr(u8 data, bool msb);
	void set_dtcr(u8 data, bool msb);
	void set_rngr(u8 data, bool msb);
	void set_dmr1(u8 data);
	void set_dmr2(u8 data);
	void set_dmr3(u8 data);
	void set_dmr4(u8 data);
	void set_efcr(u8 data);
	void set_ppcr1(u8 data);
	u8 get_ppsr();
	void set_ppier(u8 data);
	void set_mtdr(u8 data);
	u8 get_mrdr();
	void set_citdr0(u8 data);
	void set_citdr1(u8 data);
	void set_ppcr2(u8 data);
	void set_ppcr3(u8 data);

	// callback objects
	devcb_write_line m_int_callback;

	// init registers
	u8 m_init;
	u8 m_init2;

	// interrupt register
	u8 m_ir;

	// LIU registers
	u8 m_lsr;
	u8 m_lpr;
	u8 m_lmr1;
	u8 m_lmr2;
	u8 m_mf;
	u8 m_mfsb;
	u8 m_mfqb;

	// MUX registers
	u8 m_mcr[3];
	u8 m_mcr4;

	// MAP registers
	u16 m_x_coeff[8];
	u16 m_r_coeff[8];
	u16 m_gx_coeff;
	u16 m_gr_coeff;
	u16 m_ger_coeff;
	u16 m_stgr;
	u8 m_ftgr[2];
	u8 m_atgr[2];
	u8 m_mmr1;
	u8 m_mmr2;
	u8 m_mmr3;
	u8 m_stra;
	u8 m_strf;
	u8 m_peakx;
	u8 m_peakr;

	// DLC registers
	u16 m_tar;
	u8 m_frar[4];
	u8 m_srar[4];
	u16 m_drlr;
	u16 m_dtcr;
	u16 m_drcr;
	u16 m_rngr;
	u8 m_dmr1;
	u8 m_dmr2;
	u8 m_dmr3;
	u8 m_dmr4;
	u8 m_asr;
	u8 m_dsr1;
	u8 m_dsr2;
	u8 m_der;
	u8 m_efcr;

	// PP registers
	u8 m_ppcr1;
	u8 m_ppsr;
	u8 m_ppier;
	u8 m_mtdr;
	u8 m_mrdr;
	u8 m_citdr0;
	u8 m_cirdr0;
	u8 m_citdr1;
	u8 m_cirdr1;
	u8 m_ppcr2;
	u8 m_ppcr3;

	// MPI registers
	u8 m_cr;
	u8 m_byte_seq;
};

// device type declarations
DECLARE_DEVICE_TYPE(AM79C30A, am79c30a_device)

#endif // MAME_MACHINE_AM79C30_H
