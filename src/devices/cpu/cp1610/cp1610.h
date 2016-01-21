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

#pragma once

#ifndef __CP1610_H__
#define __CP1610_H__

enum
{
	CP1610_R0=1, CP1610_R1, CP1610_R2, CP1610_R3,
	CP1610_R4, CP1610_R5, CP1610_R6, CP1610_R7
};

#define CP1610_INT_NONE     0
#define CP1610_INT_INTRM    1                   /* Maskable */
#define CP1610_RESET        INPUT_LINE_RESET    /* Non-Maskable */
#define CP1610_INT_INTR     INPUT_LINE_NMI      /* Non-Maskable */

#define MCFG_CP1610_BEXT_CALLBACK(_read) \
	downcast<cp1610_cpu_device *>(device)->set_bext_callback(DEVCB_##_read);


class cp1610_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	cp1610_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	template<class _read> void set_bext_callback(_read rd)
	{
		m_read_bext.set_callback(rd);
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 7; }
	virtual UINT32 execute_input_lines() const override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT16  m_r[8];   /* registers */
	UINT8   m_flags;  /* flags */
	int     m_intr_enabled;
	UINT16  m_intr_vector;
	int     m_reset_state;
	int     m_intr_state;
	int     m_intrm_state;
	int     m_reset_pending;
	int     m_intr_pending;
	int     m_intrm_pending;
	int     m_mask_interrupts;
	address_space *m_program;
	int m_icount;

	devcb_read8 m_read_bext;

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
	void cp1610_jsr(int r, UINT16 addr);
	void cp1610_jsre(int r, UINT16 addr);
	void cp1610_jsrd(int r, UINT16 addr);
	void cp1610_j(UINT16 addr);
	void cp1610_je(UINT16 addr);
	void cp1610_jd(UINT16 addr);
	void cp1610_do_sdbd();
	void cp1610_do_jumps();
};


extern const device_type CP1610;


CPU_DISASSEMBLE( cp1610 );

#endif /* __CP1610_H__ */
