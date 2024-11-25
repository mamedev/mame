// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*****************************************************************************
 *
 *   cp1610.h
 *   Portable General Instruments CP1610 emulator interface
 *
 *   Copyright Frank Palazzolo, all rights reserved.
 *
 *****************************************************************************/

#ifndef MAME_CPU_CP1610_CP1610_H
#define MAME_CPU_CP1610_CP1610_H

#pragma once

#define CP1610_INT_INTRM    1                   /* Maskable */
#define CP1610_INT_INTR     INPUT_LINE_NMI      /* Non-Maskable */

class cp1610_cpu_device :  public cpu_device
{
public:
	// public because drivers R7 through state interface on machine reset - where does the initial R7 actually come from?
	enum
	{
		CP1610_R0=1, CP1610_R1, CP1610_R2, CP1610_R3,
		CP1610_R4, CP1610_R5, CP1610_R6, CP1610_R7
	};

	// construction/destruction
	cp1610_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	auto bext() { return m_read_bext.bind(); }
	auto iab() { return m_read_iab.bind(); }
	auto intak() { return m_write_intak.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == CP1610_INT_INTR; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	uint16_t  m_r[8];   /* registers */
	uint8_t   m_flags;  /* flags */
	bool    m_intr_enabled;
	int     m_intr_state;
	int     m_intrm_state;
	bool    m_reset_pending;
	bool    m_intr_pending;
	bool    m_intrm_pending;
	bool    m_mask_interrupts;
	address_space *m_program;
	int m_icount;

	devcb_read8 m_read_bext;
	devcb_read16 m_read_iab;
	devcb_write16 m_write_intak;

	void cp1610_illegal();
	void cp1610_hlt();
	void cp1610_eis();
	void cp1610_dis();
	void cp1610_tci();
	void cp1610_clrc();
	void cp1610_setc();
	void cp1610_incr(int n);
	void cp1610_decr(int n);
	void cp1610_comr(int n);
	void cp1610_negr(int n);
	void cp1610_adcr(int n);
	void cp1610_gswd(int n);
	void cp1610_nop();
	void cp1610_sin();
	void cp1610_rswd(int n);
	void cp1610_swap(int r);
	void cp1610_dswap(int r);
	void cp1610_sll_1(int r);
	void cp1610_sll_2(int r);
	void cp1610_rlc_1(int r);
	void cp1610_rlc_2(int r);
	void cp1610_sllc_1(int r);
	void cp1610_sllc_2(int r);
	void cp1610_slr_1(int r);
	void cp1610_slr_2(int r);
	void cp1610_sar_1(int r);
	void cp1610_sar_2(int r);
	void cp1610_rrc_1(int r);
	void cp1610_rrc_2(int r);
	void cp1610_sarc_1(int r);
	void cp1610_sarc_2(int r);
	void cp1610_tstr(int n);
	void cp1610_movr(int s, int d);
	void cp1610_addr(int s, int d);
	void cp1610_subr(int s, int d);
	void cp1610_cmpr(int s, int d);
	void cp1610_andr(int s, int d);
	void cp1610_xorr(int s, int d);
	void cp1610_clrr(int d);
	void cp1610_b(int dir);
	void cp1610_nopp(int dir);
	void cp1610_bc(int dir);
	void cp1610_bnc(int dir);
	void cp1610_bov(int dir);
	void cp1610_bnov(int dir);
	void cp1610_bpl(int dir);
	void cp1610_bmi(int dir);
	void cp1610_bze(int dir);
	void cp1610_bnze(int dir);
	void cp1610_blt(int dir);
	void cp1610_bge(int dir);
	void cp1610_ble(int dir);
	void cp1610_bgt(int dir);
	void cp1610_busc(int dir);
	void cp1610_besc(int dir);
	void cp1610_bext(int ext, int dir);
	void cp1610_mvo(int s);
	void cp1610_mvoat(int s, int m);
	void cp1610_mvoat_i(int s, int m);
	void cp1610_mvoi(int s);
	void cp1610_mvi(int d);
	void cp1610_mviat(int m, int d);
	void cp1610_mviat_i(int m, int d);
	void cp1610_pulr(int d);
	void cp1610_mvii(int d);
	void cp1610_add(int d);
	void cp1610_addat(int m, int d);
	void cp1610_addat_i(int m, int d);
	void cp1610_addat_d(int m, int d);
	void cp1610_addi(int d);
	void cp1610_sub(int d);
	void cp1610_subat(int m, int d);
	void cp1610_subat_i(int m, int d);
	void cp1610_subat_d(int m, int d);
	void cp1610_subi(int d);
	void cp1610_cmp(int d);
	void cp1610_cmpat(int m, int d);
	void cp1610_cmpat_i(int m, int d);
	void cp1610_cmpat_d(int m, int d);
	void cp1610_cmpi(int d);
	void cp1610_and(int d);
	void cp1610_andat(int m, int d);
	void cp1610_andat_i(int m, int d);
	void cp1610_andat_d(int m, int d);
	void cp1610_andi(int d);
	void cp1610_xor(int d);
	void cp1610_xorat(int m, int d);
	void cp1610_xorat_i(int m, int d);
	void cp1610_xorat_d(int m, int d);
	void cp1610_xori(int d);
	void cp1610_sdbd_mviat(int r, int d);
	void cp1610_sdbd_mviat_i(int r, int d);
	void cp1610_sdbd_mviat_d(int r, int d);
	void cp1610_sdbd_mvii(int d);
	void cp1610_sdbd_addat(int r, int d);
	void cp1610_sdbd_addat_i(int r, int d);
	void cp1610_sdbd_addat_d(int r, int d);
	void cp1610_sdbd_addi(int d);
	void cp1610_sdbd_subat(int r, int d);
	void cp1610_sdbd_subat_i(int r, int d);
	void cp1610_sdbd_subat_d(int r, int d);
	void cp1610_sdbd_subi(int d);
	void cp1610_sdbd_cmpat(int r, int d);
	void cp1610_sdbd_cmpat_i(int r, int d);
	void cp1610_sdbd_cmpat_d(int r, int d);
	void cp1610_sdbd_cmpi(int d);
	void cp1610_sdbd_andat(int r, int d);
	void cp1610_sdbd_andat_i(int r, int d);
	void cp1610_sdbd_andat_d(int r, int d);
	void cp1610_sdbd_andi(int d);
	void cp1610_sdbd_xorat(int r, int d);
	void cp1610_sdbd_xorat_i(int r, int d);
	void cp1610_sdbd_xorat_d(int r, int d);
	void cp1610_sdbd_xori(int d);
	void cp1610_jsr(int r, uint16_t addr);
	void cp1610_jsre(int r, uint16_t addr);
	void cp1610_jsrd(int r, uint16_t addr);
	void cp1610_j(uint16_t addr);
	void cp1610_je(uint16_t addr);
	void cp1610_jd(uint16_t addr);
	void cp1610_do_sdbd();
	void cp1610_do_jumps();
};


DECLARE_DEVICE_TYPE(CP1610, cp1610_cpu_device)

#endif // MAME_CPU_CP1610_CP1610_H
