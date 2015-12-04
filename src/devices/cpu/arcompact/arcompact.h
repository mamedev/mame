// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact Core

\*********************************/

#pragma once

#ifndef __ARCOMPACT_H__
#define __ARCOMPACT_H__

#define ARCOMPACT_RETTYPE UINT32
#define OPS_32 UINT32 op
#define OPS_16 UINT16 op
#define PARAMS op
#define LIMM_REG 62
#define ARCOMPACT_OPERATION ((op & 0xf800) >> 11)


#define ARCOMPACT_HANDLER04_P11_TYPE(name) \
ARCOMPACT_RETTYPE arcompact_handle##name##_p11(OPS_32) \
{ \
	int M = (op & 0x00000020) >> 5; \
		\
	switch (M) \
	{ \
		case 0x00: return arcompact_handle##name##_p11_m0(PARAMS); \
		case 0x01: return arcompact_handle##name##_p11_m1(PARAMS); \
	} \
		\
	return 0; \
}
#define ARCOMPACT_HANDLER04_TYPE(name) \
ARCOMPACT_RETTYPE arcompact_handle##name(OPS_32) \
{ \
	int p = (op & 0x00c00000) >> 22; \
	\
	switch (p) \
	{ \
		case 0x00: return arcompact_handle##name##_p00(PARAMS); \
		case 0x01: return arcompact_handle##name##_p01(PARAMS); \
		case 0x02: return arcompact_handle##name##_p10(PARAMS); \
		case 0x03: return arcompact_handle##name##_p11(PARAMS); \
	} \
	\
	return 0; \
}

#define ARCOMPACT_HANDLER04_TYPE_PM(name) \
	ARCOMPACT_RETTYPE arcompact_handle##name##_p00(OPS_32); \
	ARCOMPACT_RETTYPE arcompact_handle##name##_p01(OPS_32); \
	ARCOMPACT_RETTYPE arcompact_handle##name##_p10(OPS_32); \
	ARCOMPACT_RETTYPE arcompact_handle##name##_p11_m0(OPS_32); \
	ARCOMPACT_RETTYPE arcompact_handle##name##_p11_m1(OPS_32); \
	ARCOMPACT_HANDLER04_P11_TYPE(name); \
	ARCOMPACT_HANDLER04_TYPE(name);

class arcompact_device : public cpu_device
{
public:
	// construction/destruction
	arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( arcompact_auxreg002_LPSTART_r);
	DECLARE_WRITE32_MEMBER(arcompact_auxreg002_LPSTART_w);
	DECLARE_READ32_MEMBER( arcompact_auxreg003_LPEND_r);
	DECLARE_WRITE32_MEMBER(arcompact_auxreg003_LPEND_w);

	DECLARE_READ32_MEMBER( arcompact_auxreg00a_STATUS32_r);
	DECLARE_READ32_MEMBER( arcompact_auxreg025_INTVECTORBASE_r);
	DECLARE_WRITE32_MEMBER( arcompact_auxreg025_INTVECTORBASE_w);


protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 5; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);



	// Dispatch
	ARCOMPACT_RETTYPE arcompact_handle00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle0c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle19(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03(OPS_16);

	// Handler

	ARCOMPACT_RETTYPE arcompact_handle00_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle00_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_00_00dasm(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_00_01dasm(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle03(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_01(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_03(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_04(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_05(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_06(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_09(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_0d(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_0e(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_12(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_13(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_14(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_15(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_16(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_17(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_18(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_1d(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_20(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_29(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2a(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_01(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2f_02(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2f_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_06(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2f_07(OPS_32);
//  ARCOMPACT_RETTYPE arcompact_handle04_2f_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_37(OPS_32);
	//ARCOMPACT_RETTYPE arcompact_handle05_00(OPS_32);
	//ARCOMPACT_RETTYPE arcompact_handle05_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_29(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle0b(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle0c_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0c_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0c_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0c_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0d_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0d_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0d_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0d_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0e_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0e_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0e_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0e_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_10(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_11(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_12(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_13(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_14(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_15(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_16(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_18(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_19(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_1f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle10(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle11(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle12(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle13(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle14(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle15(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle16(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle17_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_11(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_11(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle19_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle19_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle19_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle19_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1c_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1c_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1d_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1d_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle1f(OPS_16);

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* illegal opcode handlers (disassembly)                                                                                             *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	ARCOMPACT_RETTYPE arcompact_handle01_01_00_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_00_0d(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle01_01_01_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle01_01_01_0d(OPS_32);


	ARCOMPACT_RETTYPE arcompact_handle04_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_1f(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle04_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_27(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle04_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2e(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle04_2f_0d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_12(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_13(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_14(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_15(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_16(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_17(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_18(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_1f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_20(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_27(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_29(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_37(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3e(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_12(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_13(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_14(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_15(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_16(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_17(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_18(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_1f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_20(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_27(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_29(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_37(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_3f_3f(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_2f_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_12(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_13(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_14(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_15(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_16(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_17(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_18(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_1f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_20(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_27(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_29(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_37(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3e(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_00(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_01(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_02(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_03(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_04(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_05(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_06(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_07(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_08(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_12(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_13(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_14(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_15(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_16(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_17(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_18(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_1f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_20(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_27(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_28(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_29(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_2f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_37(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_3f_3f(OPS_32);


	ARCOMPACT_RETTYPE arcompact_handle04_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle04_3f(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_09(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_0f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_10(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_11(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_12(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_13(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_14(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_15(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_16(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_17(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_18(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_19(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_1f(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_20(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_21(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_22(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_23(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_24(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_25(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_26(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_27(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_2a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_2e(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle05_30(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_31(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_32(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_33(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_34(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_35(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_36(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_37(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_38(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_39(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3a(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3b(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3c(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3d(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3e(OPS_32);
	ARCOMPACT_RETTYPE arcompact_handle05_3f(OPS_32);

	ARCOMPACT_RETTYPE arcompact_handle0f_00_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_07_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_01(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_08(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_09(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_0a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle0f_17(OPS_16);

	ARCOMPACT_RETTYPE arcompact_handle18_05_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_05_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_08(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_09(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_0f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_10(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_12(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_13(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_14(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_15(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_16(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_17(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_18(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_19(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_06_1f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_00(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_02(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_03(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_04(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_05(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_06(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_07(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_08(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_09(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_0f(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_10(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_12(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_13(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_14(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_15(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_16(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_17(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_18(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_19(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1a(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1b(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1c(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1d(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1e(OPS_16);
	ARCOMPACT_RETTYPE arcompact_handle18_07_1f(OPS_16);

	ARCOMPACT_RETTYPE arcompact_01_01_00_helper(OPS_32, const char* optext);
	ARCOMPACT_RETTYPE arcompact_01_01_01_helper(OPS_32, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle04_helper(OPS_32, const char* optext, int ignore_dst, int b_reserved);
	ARCOMPACT_RETTYPE arcompact_handle04_2f_helper(OPS_32, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle04_3x_helper(OPS_32, int dsize, int extend);
	ARCOMPACT_RETTYPE arcompact_handle05_2f_0x_helper(OPS_32, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle0c_helper(OPS_16, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle0d_helper(OPS_16, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle0e_0x_helper(OPS_16, const char* optext, int revop);
	ARCOMPACT_RETTYPE arcompact_handle0f_00_0x_helper(OPS_16, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle0f_0x_helper(OPS_16, const char* optext, int nodst);
	ARCOMPACT_RETTYPE arcompact_handle_ld_helper(OPS_16, const char* optext, int shift, int swap);
	ARCOMPACT_RETTYPE arcompact_handle_l7_0x_helper(OPS_16, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle18_0x_helper(OPS_16, const char* optext, int st);
	ARCOMPACT_RETTYPE arcompact_handle19_0x_helper(OPS_16, const char* optext, int shift, int format);
	ARCOMPACT_RETTYPE arcompact_handle1e_0x_helper(OPS_16, const char* optext);
	ARCOMPACT_RETTYPE arcompact_handle1e_03_0x_helper(OPS_16, const char* optext);


	UINT32 handle_jump_to_addr(int delay, int link, UINT32 address, UINT32 next_addr);
	UINT32 handle_jump_to_register(int delay, int link, UINT32 reg, UINT32 next_addr, int flag);

	ARCOMPACT_RETTYPE get_insruction(OPS_32);

	ARCOMPACT_HANDLER04_TYPE_PM(04_00)
	ARCOMPACT_HANDLER04_TYPE_PM(04_02)
	ARCOMPACT_HANDLER04_TYPE_PM(04_04)
	ARCOMPACT_HANDLER04_TYPE_PM(04_05)
	ARCOMPACT_HANDLER04_TYPE_PM(04_06)
	ARCOMPACT_HANDLER04_TYPE_PM(04_07)
	ARCOMPACT_HANDLER04_TYPE_PM(04_0a)
	ARCOMPACT_HANDLER04_TYPE_PM(04_0e)
	ARCOMPACT_HANDLER04_TYPE_PM(04_0f)
	ARCOMPACT_HANDLER04_TYPE_PM(04_13)
	ARCOMPACT_HANDLER04_TYPE_PM(04_14)
	ARCOMPACT_HANDLER04_TYPE_PM(04_15)
	ARCOMPACT_HANDLER04_TYPE_PM(04_16)
	ARCOMPACT_HANDLER04_TYPE_PM(04_17)
	ARCOMPACT_HANDLER04_TYPE_PM(04_18)
	ARCOMPACT_HANDLER04_TYPE_PM(04_19)
	ARCOMPACT_HANDLER04_TYPE_PM(04_20)
	ARCOMPACT_HANDLER04_TYPE_PM(04_21)
	ARCOMPACT_HANDLER04_TYPE_PM(04_2a)
	ARCOMPACT_HANDLER04_TYPE_PM(04_2b)

	ARCOMPACT_HANDLER04_TYPE_PM(04_2f_02)
	ARCOMPACT_HANDLER04_TYPE_PM(04_2f_03)
	ARCOMPACT_HANDLER04_TYPE_PM(04_2f_07)
	ARCOMPACT_HANDLER04_TYPE_PM(04_2f_08)

	ARCOMPACT_HANDLER04_TYPE_PM(05_00)
	ARCOMPACT_HANDLER04_TYPE_PM(05_01)


private:
	const address_space_config m_program_config;
	const address_space_config m_io_config;

	UINT32 m_pc;

	address_space *m_program;
	address_space  *m_io;

	int m_icount;

	UINT32 m_debugger_temp;

	void unimplemented_opcode(UINT16 op);

	inline  UINT32 READ32(UINT32 address) { return m_program->read_dword(address << 2); }
	inline void WRITE32(UINT32 address, UINT32 data) { m_program->write_dword(address << 2, data); }
	inline UINT16 READ16(UINT32 address) { return m_program->read_word(address << 1); }
	inline void WRITE16(UINT32 address, UINT16 data){   m_program->write_word(address << 1, data); }
	inline UINT8 READ8(UINT32 address) { return m_program->read_byte(address << 0); }
	inline void WRITE8(UINT32 address, UINT8 data){     m_program->write_byte(address << 0, data); }

	inline  UINT64 READAUX(UINT64 address) { return m_io->read_dword(address *4); }
	inline void WRITEAUX(UINT64 address, UINT32 data) { m_io->write_dword(address *4, data); }


	int check_condition(UINT8 condition);

	UINT32 m_regs[0x40];

	int m_delayactive;
	int m_delaylinks;
	UINT32 m_delayjump;

//  f  e  d  c| b  a  9  8| 7  6  5  4| 3  2  1  0
//  -  -  -  L| Z  N  C  V| U DE AE A2|A1 E2 E1  H
	UINT32 m_status32;

	UINT32 m_LP_START;
	UINT32 m_LP_END;
	UINT32 m_INTVECTORBASE;

};

#define V_OVERFLOW_FLAG (0x00000100)
#define C_CARRY_FLAG (0x00000200)
#define N_NEGATIVE_FLAG (0x00000400)
#define Z_ZERO_FLAG (0x00000800)

// V = overflow (set if signed operation would overflow)
#define STATUS32_SET_V   (m_status32 |=  V_OVERFLOW_FLAG)
#define STATUS32_CLEAR_V (m_status32 &= ~V_OVERFLOW_FLAG)
#define STATUS32_CHECK_V (m_status32 &   V_OVERFLOW_FLAG)

// C = carry (unsigned op, carry set is same condition as LO Lower Than, carry clear is same condition as HS Higher Same)
#define STATUS32_SET_C   (m_status32 |=  C_CARRY_FLAG)
#define STATUS32_CLEAR_C (m_status32 &= ~C_CARRY_FLAG)
#define STATUS32_CHECK_C (m_status32 &   C_CARRY_FLAG)

// N = negative (set if most significant bit of result is set)
#define STATUS32_SET_N   (m_status32 |=  N_NEGATIVE_FLAG)
#define STATUS32_CLEAR_N (m_status32 &= ~N_NEGATIVE_FLAG)
#define STATUS32_CHECK_N (m_status32 &   N_NEGATIVE_FLAG)

// Z = zero (set if result is zero, ie both values the same for CMP)
#define STATUS32_SET_Z   (m_status32 |=  Z_ZERO_FLAG)
#define STATUS32_CLEAR_Z (m_status32 &= ~Z_ZERO_FLAG)
#define STATUS32_CHECK_Z (m_status32 &   Z_ZERO_FLAG)

// Condition 0x0c (LE)
#define CONDITION_LE ((STATUS32_CHECK_Z) || (STATUS32_CHECK_N && !STATUS32_CHECK_V) ||  (!STATUS32_CHECK_N && STATUS32_CHECK_V)) // Z or (N and /V) or (/N and V)
#define CONDITION_EQ (STATUS32_CHECK_Z)
#define CONDITION_CS (STATUS32_CHECK_C)
#define CONDITION_LT ((STATUS32_CHECK_N && !STATUS32_CHECK_V) || (!STATUS32_CHECK_N && STATUS32_CHECK_V))
#define CONDITION_MI (STATUS32_CHECK_N)

extern const device_type ARCA5;


#endif /* __ARCOMPACT_H__ */
