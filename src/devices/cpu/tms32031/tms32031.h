// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tms32031.h

    TMS32031/2 emulator

***************************************************************************/

#pragma once

#ifndef __TMS32031_H__
#define __TMS32031_H__


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define TMS_3203X_LOG_OPCODE_USAGE  (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// interrupts
const int TMS3203X_IRQ0     = 0;        // IRQ0
const int TMS3203X_IRQ1     = 1;        // IRQ1
const int TMS3203X_IRQ2     = 2;        // IRQ2
const int TMS3203X_IRQ3     = 3;        // IRQ3
const int TMS3203X_XINT0    = 4;        // serial 0 transmit interrupt
const int TMS3203X_RINT0    = 5;        // serial 0 receive interrupt
const int TMS3203X_XINT1    = 6;        // serial 1 transmit interrupt
const int TMS3203X_RINT1    = 7;        // serial 1 receive interrupt
const int TMS3203X_TINT0    = 8;        // timer 0 interrupt
const int TMS3203X_TINT1    = 9;        // timer 1 interrupt
const int TMS3203X_DINT     = 10;       // DMA interrupt
const int TMS3203X_DINT0    = 10;       // DMA 0 interrupt (32032 only)
const int TMS3203X_DINT1    = 11;       // DMA 1 interrupt (32032 only)
const int TMS3203X_MCBL     = 12;       // Microcomputer/boot loader mode

// register enumeration
enum
{
	TMS3203X_PC=1,
	TMS3203X_R0,
	TMS3203X_R1,
	TMS3203X_R2,
	TMS3203X_R3,
	TMS3203X_R4,
	TMS3203X_R5,
	TMS3203X_R6,
	TMS3203X_R7,
	TMS3203X_R0F,
	TMS3203X_R1F,
	TMS3203X_R2F,
	TMS3203X_R3F,
	TMS3203X_R4F,
	TMS3203X_R5F,
	TMS3203X_R6F,
	TMS3203X_R7F,
	TMS3203X_AR0,
	TMS3203X_AR1,
	TMS3203X_AR2,
	TMS3203X_AR3,
	TMS3203X_AR4,
	TMS3203X_AR5,
	TMS3203X_AR6,
	TMS3203X_AR7,
	TMS3203X_DP,
	TMS3203X_IR0,
	TMS3203X_IR1,
	TMS3203X_BK,
	TMS3203X_SP,
	TMS3203X_ST,
	TMS3203X_IE,
	TMS3203X_IF,
	TMS3203X_IOF,
	TMS3203X_RS,
	TMS3203X_RE,
	TMS3203X_RC
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS3203X_MCBL(_mode) \
	tms3203x_device::set_mcbl_mode(*device, _mode);

#define MCFG_TMS3203X_XF0_CB(_devcb) \
	devcb = &tms3203x_device::set_xf0_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS3203X_XF1_CB(_devcb) \
	devcb = &tms3203x_device::set_xf1_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS3203X_IACK_CB(_devcb) \
	devcb = &tms3203x_device::set_iack_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tms3203x_device

class tms3203x_device : public cpu_device
{
	struct tmsreg
	{
		// constructors
		tmsreg() { i32[0] = i32[1] = 0; }
		tmsreg(double value) { from_double(value); }
		tmsreg(INT32 mantissa, INT8 exponent) { set_mantissa(mantissa); set_exponent(exponent); }

		// getters
		UINT32 integer() const { return i32[0]; }
		INT32 mantissa() const { return i32[0]; }
		INT8 exponent() const { return i32[1]; }
		void set_mantissa(INT32 man) { i32[0] = man; }
		void set_exponent(INT8 exp) { i32[1] = exp; }

		// exporters
		float as_float() const;
		double as_double() const;

		// importers
		void from_double(double);

		UINT32      i32[2];
	};

protected:
	enum
	{
		CHIP_TYPE_TMS32031,
		CHIP_TYPE_TMS32032
	};

	// construction/destruction
	tms3203x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 chiptype, address_map_constructor internal_map, const char *shortname, const char *source);
	virtual ~tms3203x_device();

public:
	// inline configuration helpers
	static void set_mcbl_mode(device_t &device, bool mode) { downcast<tms3203x_device &>(device).m_mcbl_mode = mode; }
	template<class _Object> static devcb_base &set_xf0_callback(device_t &device, _Object object) { return downcast<tms3203x_device &>(device).m_xf0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_xf1_callback(device_t &device, _Object object) { return downcast<tms3203x_device &>(device).m_xf1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_iack_callback(device_t &device, _Object object) { return downcast<tms3203x_device &>(device).m_iack_cb.set_callback(object); }

	// public interfaces
	static float fp_to_float(UINT32 floatdata);
	static double fp_to_double(UINT32 floatdata);
	static UINT32 float_to_fp(float fval);
	static UINT32 double_to_fp(double dval);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual const rom_entry *device_rom_region() const;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// memory helpers
	DECLARE_DIRECT_UPDATE_MEMBER(direct_handler);
	UINT32 ROPCODE(offs_t pc);
	UINT32 RMEM(offs_t addr);
	void WMEM(offs_t addr, UINT32 data);

	// misc helpers
	void check_irqs();
	void execute_one();
	void update_special(int dreg);
	bool condition(int which);

	// floating point helpers
	void double_to_dsp_with_flags(double val, tmsreg &result);
	void int2float(tmsreg &srcdst);
	void float2int(tmsreg &srcdst, bool setflags);
	void negf(tmsreg &dst, tmsreg &src);
	void addf(tmsreg &dst, tmsreg &src1, tmsreg &src2);
	void subf(tmsreg &dst, tmsreg &src1, tmsreg &src2);
	void mpyf(tmsreg &dst, tmsreg &src1, tmsreg &src2);
	void norm(tmsreg &dst, tmsreg &src);

	// memory addressing
	UINT32 mod00_d(UINT32 op, UINT8 ar);
	UINT32 mod01_d(UINT32 op, UINT8 ar);
	UINT32 mod02_d(UINT32 op, UINT8 ar);
	UINT32 mod03_d(UINT32 op, UINT8 ar);
	UINT32 mod04_d(UINT32 op, UINT8 ar);
	UINT32 mod05_d(UINT32 op, UINT8 ar);
	UINT32 mod06_d(UINT32 op, UINT8 ar);
	UINT32 mod07_d(UINT32 op, UINT8 ar);

	UINT32 mod00_1(UINT32 op, UINT8 ar);
	UINT32 mod01_1(UINT32 op, UINT8 ar);
	UINT32 mod02_1(UINT32 op, UINT8 ar);
	UINT32 mod03_1(UINT32 op, UINT8 ar);
	UINT32 mod04_1(UINT32 op, UINT8 ar);
	UINT32 mod05_1(UINT32 op, UINT8 ar);
	UINT32 mod06_1(UINT32 op, UINT8 ar);
	UINT32 mod07_1(UINT32 op, UINT8 ar);

	UINT32 mod08(UINT32 op, UINT8 ar);
	UINT32 mod09(UINT32 op, UINT8 ar);
	UINT32 mod0a(UINT32 op, UINT8 ar);
	UINT32 mod0b(UINT32 op, UINT8 ar);
	UINT32 mod0c(UINT32 op, UINT8 ar);
	UINT32 mod0d(UINT32 op, UINT8 ar);
	UINT32 mod0e(UINT32 op, UINT8 ar);
	UINT32 mod0f(UINT32 op, UINT8 ar);

	UINT32 mod10(UINT32 op, UINT8 ar);
	UINT32 mod11(UINT32 op, UINT8 ar);
	UINT32 mod12(UINT32 op, UINT8 ar);
	UINT32 mod13(UINT32 op, UINT8 ar);
	UINT32 mod14(UINT32 op, UINT8 ar);
	UINT32 mod15(UINT32 op, UINT8 ar);
	UINT32 mod16(UINT32 op, UINT8 ar);
	UINT32 mod17(UINT32 op, UINT8 ar);

	UINT32 mod18(UINT32 op, UINT8 ar);
	UINT32 mod19(UINT32 op, UINT8 ar);
	UINT32 modillegal(UINT32 op, UINT8 ar);

	UINT32 mod00_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod01_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod02_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod03_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod04_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod05_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod06_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod07_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);

	UINT32 mod08_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod09_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0a_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0b_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0c_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0d_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0e_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod0f_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);

	UINT32 mod10_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod11_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod12_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod13_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod14_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod15_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod16_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod17_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod18_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 mod19_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);
	UINT32 modillegal_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr);

	// instructions
	void illegal(UINT32 op);
	void unimplemented(UINT32 op);

	void absf_reg(UINT32 op);
	void absf_dir(UINT32 op);
	void absf_ind(UINT32 op);
	void absf_imm(UINT32 op);
	void absi_reg(UINT32 op);
	void absi_dir(UINT32 op);
	void absi_ind(UINT32 op);
	void absi_imm(UINT32 op);
	void addc_reg(UINT32 op);
	void addc_dir(UINT32 op);
	void addc_ind(UINT32 op);
	void addc_imm(UINT32 op);
	void addf_reg(UINT32 op);
	void addf_dir(UINT32 op);
	void addf_ind(UINT32 op);
	void addf_imm(UINT32 op);
	void addi_reg(UINT32 op);
	void addi_dir(UINT32 op);
	void addi_ind(UINT32 op);
	void addi_imm(UINT32 op);
	void and_reg(UINT32 op);
	void and_dir(UINT32 op);
	void and_ind(UINT32 op);
	void and_imm(UINT32 op);
	void andn_reg(UINT32 op);
	void andn_dir(UINT32 op);
	void andn_ind(UINT32 op);
	void andn_imm(UINT32 op);
	void ash_reg(UINT32 op);
	void ash_dir(UINT32 op);
	void ash_ind(UINT32 op);
	void ash_imm(UINT32 op);
	void cmpf_reg(UINT32 op);
	void cmpf_dir(UINT32 op);
	void cmpf_ind(UINT32 op);
	void cmpf_imm(UINT32 op);
	void cmpi_reg(UINT32 op);
	void cmpi_dir(UINT32 op);
	void cmpi_ind(UINT32 op);
	void cmpi_imm(UINT32 op);
	void fix_reg(UINT32 op);
	void fix_dir(UINT32 op);
	void fix_ind(UINT32 op);
	void fix_imm(UINT32 op);
	void float_reg(UINT32 op);
	void float_dir(UINT32 op);
	void float_ind(UINT32 op);
	void float_imm(UINT32 op);
	void idle(UINT32 op);
	void lde_reg(UINT32 op);
	void lde_dir(UINT32 op);
	void lde_ind(UINT32 op);
	void lde_imm(UINT32 op);
	void ldf_reg(UINT32 op);
	void ldf_dir(UINT32 op);
	void ldf_ind(UINT32 op);
	void ldf_imm(UINT32 op);
	void ldfi_dir(UINT32 op);
	void ldfi_ind(UINT32 op);
	void ldi_reg(UINT32 op);
	void ldi_dir(UINT32 op);
	void ldi_ind(UINT32 op);
	void ldi_imm(UINT32 op);
	void ldii_dir(UINT32 op);
	void ldii_ind(UINT32 op);
	void ldm_reg(UINT32 op);
	void ldm_dir(UINT32 op);
	void ldm_ind(UINT32 op);
	void ldm_imm(UINT32 op);
	void lsh_reg(UINT32 op);
	void lsh_dir(UINT32 op);
	void lsh_ind(UINT32 op);
	void lsh_imm(UINT32 op);
	void mpyf_reg(UINT32 op);
	void mpyf_dir(UINT32 op);
	void mpyf_ind(UINT32 op);
	void mpyf_imm(UINT32 op);
	void mpyi_reg(UINT32 op);
	void mpyi_dir(UINT32 op);
	void mpyi_ind(UINT32 op);
	void mpyi_imm(UINT32 op);
	void negb_reg(UINT32 op);
	void negb_dir(UINT32 op);
	void negb_ind(UINT32 op);
	void negb_imm(UINT32 op);
	void negf_reg(UINT32 op);
	void negf_dir(UINT32 op);
	void negf_ind(UINT32 op);
	void negf_imm(UINT32 op);
	void negi_reg(UINT32 op);
	void negi_dir(UINT32 op);
	void negi_ind(UINT32 op);
	void negi_imm(UINT32 op);
	void nop_reg(UINT32 op);
	void nop_ind(UINT32 op);
	void norm_reg(UINT32 op);
	void norm_dir(UINT32 op);
	void norm_ind(UINT32 op);
	void norm_imm(UINT32 op);
	void not_reg(UINT32 op);
	void not_dir(UINT32 op);
	void not_ind(UINT32 op);
	void not_imm(UINT32 op);
	void pop(UINT32 op);
	void popf(UINT32 op);
	void push(UINT32 op);
	void pushf(UINT32 op);
	void or_reg(UINT32 op);
	void or_dir(UINT32 op);
	void or_ind(UINT32 op);
	void or_imm(UINT32 op);
	void maxspeed(UINT32 op);
	void rnd_reg(UINT32 op);
	void rnd_dir(UINT32 op);
	void rnd_ind(UINT32 op);
	void rnd_imm(UINT32 op);
	void rol(UINT32 op);
	void rolc(UINT32 op);
	void ror(UINT32 op);
	void rorc(UINT32 op);
	void rtps_reg(UINT32 op);
	void rtps_dir(UINT32 op);
	void rtps_ind(UINT32 op);
	void rtps_imm(UINT32 op);
	void stf_dir(UINT32 op);
	void stf_ind(UINT32 op);
	void stfi_dir(UINT32 op);
	void stfi_ind(UINT32 op);
	void sti_dir(UINT32 op);
	void sti_ind(UINT32 op);
	void stii_dir(UINT32 op);
	void stii_ind(UINT32 op);
	void sigi(UINT32 op);
	void subb_reg(UINT32 op);
	void subb_dir(UINT32 op);
	void subb_ind(UINT32 op);
	void subb_imm(UINT32 op);
	void subc_reg(UINT32 op);
	void subc_dir(UINT32 op);
	void subc_ind(UINT32 op);
	void subc_imm(UINT32 op);
	void subf_reg(UINT32 op);
	void subf_dir(UINT32 op);
	void subf_ind(UINT32 op);
	void subf_imm(UINT32 op);
	void subi_reg(UINT32 op);
	void subi_dir(UINT32 op);
	void subi_ind(UINT32 op);
	void subi_imm(UINT32 op);
	void subrb_reg(UINT32 op);
	void subrb_dir(UINT32 op);
	void subrb_ind(UINT32 op);
	void subrb_imm(UINT32 op);
	void subrf_reg(UINT32 op);
	void subrf_dir(UINT32 op);
	void subrf_ind(UINT32 op);
	void subrf_imm(UINT32 op);
	void subri_reg(UINT32 op);
	void subri_dir(UINT32 op);
	void subri_ind(UINT32 op);
	void subri_imm(UINT32 op);
	void tstb_reg(UINT32 op);
	void tstb_dir(UINT32 op);
	void tstb_ind(UINT32 op);
	void tstb_imm(UINT32 op);
	void xor_reg(UINT32 op);
	void xor_dir(UINT32 op);
	void xor_ind(UINT32 op);
	void xor_imm(UINT32 op);
	void iack_dir(UINT32 op);
	void iack_ind(UINT32 op);
	void addc3_regreg(UINT32 op);
	void addc3_indreg(UINT32 op);
	void addc3_regind(UINT32 op);
	void addc3_indind(UINT32 op);
	void addf3_regreg(UINT32 op);
	void addf3_indreg(UINT32 op);
	void addf3_regind(UINT32 op);
	void addf3_indind(UINT32 op);
	void addi3_regreg(UINT32 op);
	void addi3_indreg(UINT32 op);
	void addi3_regind(UINT32 op);
	void addi3_indind(UINT32 op);
	void and3_regreg(UINT32 op);
	void and3_indreg(UINT32 op);
	void and3_regind(UINT32 op);
	void and3_indind(UINT32 op);
	void andn3_regreg(UINT32 op);
	void andn3_indreg(UINT32 op);
	void andn3_regind(UINT32 op);
	void andn3_indind(UINT32 op);
	void ash3_regreg(UINT32 op);
	void ash3_indreg(UINT32 op);
	void ash3_regind(UINT32 op);
	void ash3_indind(UINT32 op);
	void cmpf3_regreg(UINT32 op);
	void cmpf3_indreg(UINT32 op);
	void cmpf3_regind(UINT32 op);
	void cmpf3_indind(UINT32 op);
	void cmpi3_regreg(UINT32 op);
	void cmpi3_indreg(UINT32 op);
	void cmpi3_regind(UINT32 op);
	void cmpi3_indind(UINT32 op);
	void lsh3_regreg(UINT32 op);
	void lsh3_indreg(UINT32 op);
	void lsh3_regind(UINT32 op);
	void lsh3_indind(UINT32 op);
	void mpyf3_regreg(UINT32 op);
	void mpyf3_indreg(UINT32 op);
	void mpyf3_regind(UINT32 op);
	void mpyf3_indind(UINT32 op);
	void mpyi3_regreg(UINT32 op);
	void mpyi3_indreg(UINT32 op);
	void mpyi3_regind(UINT32 op);
	void mpyi3_indind(UINT32 op);
	void or3_regreg(UINT32 op);
	void or3_indreg(UINT32 op);
	void or3_regind(UINT32 op);
	void or3_indind(UINT32 op);
	void subb3_regreg(UINT32 op);
	void subb3_indreg(UINT32 op);
	void subb3_regind(UINT32 op);
	void subb3_indind(UINT32 op);
	void subf3_regreg(UINT32 op);
	void subf3_indreg(UINT32 op);
	void subf3_regind(UINT32 op);
	void subf3_indind(UINT32 op);
	void subi3_regreg(UINT32 op);
	void subi3_indreg(UINT32 op);
	void subi3_regind(UINT32 op);
	void subi3_indind(UINT32 op);
	void tstb3_regreg(UINT32 op);
	void tstb3_indreg(UINT32 op);
	void tstb3_regind(UINT32 op);
	void tstb3_indind(UINT32 op);
	void xor3_regreg(UINT32 op);
	void xor3_indreg(UINT32 op);
	void xor3_regind(UINT32 op);
	void xor3_indind(UINT32 op);
	void ldfu_reg(UINT32 op);
	void ldfu_dir(UINT32 op);
	void ldfu_ind(UINT32 op);
	void ldfu_imm(UINT32 op);
	void ldflo_reg(UINT32 op);
	void ldflo_dir(UINT32 op);
	void ldflo_ind(UINT32 op);
	void ldflo_imm(UINT32 op);
	void ldfls_reg(UINT32 op);
	void ldfls_dir(UINT32 op);
	void ldfls_ind(UINT32 op);
	void ldfls_imm(UINT32 op);
	void ldfhi_reg(UINT32 op);
	void ldfhi_dir(UINT32 op);
	void ldfhi_ind(UINT32 op);
	void ldfhi_imm(UINT32 op);
	void ldfhs_reg(UINT32 op);
	void ldfhs_dir(UINT32 op);
	void ldfhs_ind(UINT32 op);
	void ldfhs_imm(UINT32 op);
	void ldfeq_reg(UINT32 op);
	void ldfeq_dir(UINT32 op);
	void ldfeq_ind(UINT32 op);
	void ldfeq_imm(UINT32 op);
	void ldfne_reg(UINT32 op);
	void ldfne_dir(UINT32 op);
	void ldfne_ind(UINT32 op);
	void ldfne_imm(UINT32 op);
	void ldflt_reg(UINT32 op);
	void ldflt_dir(UINT32 op);
	void ldflt_ind(UINT32 op);
	void ldflt_imm(UINT32 op);
	void ldfle_reg(UINT32 op);
	void ldfle_dir(UINT32 op);
	void ldfle_ind(UINT32 op);
	void ldfle_imm(UINT32 op);
	void ldfgt_reg(UINT32 op);
	void ldfgt_dir(UINT32 op);
	void ldfgt_ind(UINT32 op);
	void ldfgt_imm(UINT32 op);
	void ldfge_reg(UINT32 op);
	void ldfge_dir(UINT32 op);
	void ldfge_ind(UINT32 op);
	void ldfge_imm(UINT32 op);
	void ldfnv_reg(UINT32 op);
	void ldfnv_dir(UINT32 op);
	void ldfnv_ind(UINT32 op);
	void ldfnv_imm(UINT32 op);
	void ldfv_reg(UINT32 op);
	void ldfv_dir(UINT32 op);
	void ldfv_ind(UINT32 op);
	void ldfv_imm(UINT32 op);
	void ldfnuf_reg(UINT32 op);
	void ldfnuf_dir(UINT32 op);
	void ldfnuf_ind(UINT32 op);
	void ldfnuf_imm(UINT32 op);
	void ldfuf_reg(UINT32 op);
	void ldfuf_dir(UINT32 op);
	void ldfuf_ind(UINT32 op);
	void ldfuf_imm(UINT32 op);
	void ldfnlv_reg(UINT32 op);
	void ldfnlv_dir(UINT32 op);
	void ldfnlv_ind(UINT32 op);
	void ldfnlv_imm(UINT32 op);
	void ldflv_reg(UINT32 op);
	void ldflv_dir(UINT32 op);
	void ldflv_ind(UINT32 op);
	void ldflv_imm(UINT32 op);
	void ldfnluf_reg(UINT32 op);
	void ldfnluf_dir(UINT32 op);
	void ldfnluf_ind(UINT32 op);
	void ldfnluf_imm(UINT32 op);
	void ldfluf_reg(UINT32 op);
	void ldfluf_dir(UINT32 op);
	void ldfluf_ind(UINT32 op);
	void ldfluf_imm(UINT32 op);
	void ldfzuf_reg(UINT32 op);
	void ldfzuf_dir(UINT32 op);
	void ldfzuf_ind(UINT32 op);
	void ldfzuf_imm(UINT32 op);
	void ldiu_reg(UINT32 op);
	void ldiu_dir(UINT32 op);
	void ldiu_ind(UINT32 op);
	void ldiu_imm(UINT32 op);
	void ldilo_reg(UINT32 op);
	void ldilo_dir(UINT32 op);
	void ldilo_ind(UINT32 op);
	void ldilo_imm(UINT32 op);
	void ldils_reg(UINT32 op);
	void ldils_dir(UINT32 op);
	void ldils_ind(UINT32 op);
	void ldils_imm(UINT32 op);
	void ldihi_reg(UINT32 op);
	void ldihi_dir(UINT32 op);
	void ldihi_ind(UINT32 op);
	void ldihi_imm(UINT32 op);
	void ldihs_reg(UINT32 op);
	void ldihs_dir(UINT32 op);
	void ldihs_ind(UINT32 op);
	void ldihs_imm(UINT32 op);
	void ldieq_reg(UINT32 op);
	void ldieq_dir(UINT32 op);
	void ldieq_ind(UINT32 op);
	void ldieq_imm(UINT32 op);
	void ldine_reg(UINT32 op);
	void ldine_dir(UINT32 op);
	void ldine_ind(UINT32 op);
	void ldine_imm(UINT32 op);
	void ldilt_reg(UINT32 op);
	void ldilt_dir(UINT32 op);
	void ldilt_ind(UINT32 op);
	void ldilt_imm(UINT32 op);
	void ldile_reg(UINT32 op);
	void ldile_dir(UINT32 op);
	void ldile_ind(UINT32 op);
	void ldile_imm(UINT32 op);
	void ldigt_reg(UINT32 op);
	void ldigt_dir(UINT32 op);
	void ldigt_ind(UINT32 op);
	void ldigt_imm(UINT32 op);
	void ldige_reg(UINT32 op);
	void ldige_dir(UINT32 op);
	void ldige_ind(UINT32 op);
	void ldige_imm(UINT32 op);
	void ldinv_reg(UINT32 op);
	void ldinv_dir(UINT32 op);
	void ldinv_ind(UINT32 op);
	void ldinv_imm(UINT32 op);
	void ldiuf_reg(UINT32 op);
	void ldiuf_dir(UINT32 op);
	void ldiuf_ind(UINT32 op);
	void ldiuf_imm(UINT32 op);
	void ldinuf_reg(UINT32 op);
	void ldinuf_dir(UINT32 op);
	void ldinuf_ind(UINT32 op);
	void ldinuf_imm(UINT32 op);
	void ldiv_reg(UINT32 op);
	void ldiv_dir(UINT32 op);
	void ldiv_ind(UINT32 op);
	void ldiv_imm(UINT32 op);
	void ldinlv_reg(UINT32 op);
	void ldinlv_dir(UINT32 op);
	void ldinlv_ind(UINT32 op);
	void ldinlv_imm(UINT32 op);
	void ldilv_reg(UINT32 op);
	void ldilv_dir(UINT32 op);
	void ldilv_ind(UINT32 op);
	void ldilv_imm(UINT32 op);
	void ldinluf_reg(UINT32 op);
	void ldinluf_dir(UINT32 op);
	void ldinluf_ind(UINT32 op);
	void ldinluf_imm(UINT32 op);
	void ldiluf_reg(UINT32 op);
	void ldiluf_dir(UINT32 op);
	void ldiluf_ind(UINT32 op);
	void ldiluf_imm(UINT32 op);
	void ldizuf_reg(UINT32 op);
	void ldizuf_dir(UINT32 op);
	void ldizuf_ind(UINT32 op);
	void ldizuf_imm(UINT32 op);
	void execute_delayed(UINT32 newpc);
	void br_imm(UINT32 op);
	void brd_imm(UINT32 op);
	void call_imm(UINT32 op);
	void rptb_imm(UINT32 op);
	void swi(UINT32 op);
	void brc_reg(UINT32 op);
	void brcd_reg(UINT32 op);
	void brc_imm(UINT32 op);
	void brcd_imm(UINT32 op);
	void dbc_reg(UINT32 op);
	void dbcd_reg(UINT32 op);
	void dbc_imm(UINT32 op);
	void dbcd_imm(UINT32 op);
	void callc_reg(UINT32 op);
	void callc_imm(UINT32 op);
	void trap(int trapnum);
	void trapc(UINT32 op);
	void retic_reg(UINT32 op);
	void retsc_reg(UINT32 op);
	void mpyaddf_0(UINT32 op);
	void mpyaddf_1(UINT32 op);
	void mpyaddf_2(UINT32 op);
	void mpyaddf_3(UINT32 op);
	void mpysubf_0(UINT32 op);
	void mpysubf_1(UINT32 op);
	void mpysubf_2(UINT32 op);
	void mpysubf_3(UINT32 op);
	void mpyaddi_0(UINT32 op);
	void mpyaddi_1(UINT32 op);
	void mpyaddi_2(UINT32 op);
	void mpyaddi_3(UINT32 op);
	void mpysubi_0(UINT32 op);
	void mpysubi_1(UINT32 op);
	void mpysubi_2(UINT32 op);
	void mpysubi_3(UINT32 op);
	void stfstf(UINT32 op);
	void stisti(UINT32 op);
	void ldfldf(UINT32 op);
	void ldildi(UINT32 op);
	void absfstf(UINT32 op);
	void absisti(UINT32 op);
	void addf3stf(UINT32 op);
	void addi3sti(UINT32 op);
	void and3sti(UINT32 op);
	void ash3sti(UINT32 op);
	void fixsti(UINT32 op);
	void floatstf(UINT32 op);
	void ldfstf(UINT32 op);
	void ldisti(UINT32 op);
	void lsh3sti(UINT32 op);
	void mpyf3stf(UINT32 op);
	void mpyi3sti(UINT32 op);
	void negfstf(UINT32 op);
	void negisti(UINT32 op);
	void notsti(UINT32 op);
	void or3sti(UINT32 op);
	void subf3stf(UINT32 op);
	void subi3sti(UINT32 op);
	void xor3sti(UINT32 op);

	// configuration
	const address_space_config      m_program_config;
	UINT32                          m_chip_type;

	union int_double
	{
		double d;
		float f[2];
		UINT32 i[2];
	};

	// core registers
	UINT32              m_pc;
	tmsreg              m_r[36];
	UINT32              m_bkmask;

	// internal stuff
	UINT16              m_irq_state;
	bool                m_delayed;
	bool                m_irq_pending;
	bool                m_is_idling;
	int                 m_icount;

	UINT32              m_iotemp;
	address_space *     m_program;
	direct_read_data *  m_direct;
	UINT32 *            m_bootrom;

	bool                m_mcbl_mode;
	devcb_write8        m_xf0_cb;
	devcb_write8        m_xf1_cb;
	devcb_write8        m_iack_cb;

	// tables
	static void (tms3203x_device::*const s_tms32031ops[])(UINT32 op);
	static UINT32 (tms3203x_device::*const s_indirect_d[0x20])(UINT32, UINT8);
	static UINT32 (tms3203x_device::*const s_indirect_1[0x20])(UINT32, UINT8);
	static UINT32 (tms3203x_device::*const s_indirect_1_def[0x20])(UINT32, UINT8, UINT32 *&);

#if (TMS_3203X_LOG_OPCODE_USAGE)
	UINT32              m_hits[0x200*4];
#endif
};


// ======================> tms32031_device

class tms32031_device : public tms3203x_device
{
public:
	// construction/destruction
	tms32031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> tms32032_device

class tms32032_device : public tms3203x_device
{
public:
	// construction/destruction
	tms32032_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type TMS32031;
extern const device_type TMS32032;



#endif /* __TMS32031_H__ */
