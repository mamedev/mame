// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_CPU_DSP56156_TABLES_H
#define MAME_CPU_DSP56156_TABLES_H

#pragma once

#include "dsp56156.h"

#include <cstdio>
#include <cstdlib>

namespace DSP_56156 {

enum bitsModified {BM_NONE = 0x0, BM_LOW = 0x1, BM_MIDDLE = 0x2, BM_HIGH = 0x4};

enum bfShift {BBB_UPPER, BBB_MIDDLE, BBB_LOWER, BBB_INVALID};
enum reg_id {iX,  iX0, iX1,
				iY,  iY0, iY1,
				iA,  iA0, iA1, iA2,
				iB,  iB0, iB1, iB2,
				iR0, iR1, iR2, iR3,
				iN0, iN1, iN2, iN3,
				iM0, iM1, iM2, iM3,
				iLC, iSR, iOMR, iSP, iSSH, iSSL, iLA, iMR, iCCR,
				iF, iFHAT,
				iINVALID, iWEIRD};

enum op_mnem {oCC, oGE, oNE, oPL,
				oNN, oEC, oLC, oGT,
				oCS, oLT, oEQ, oMI,
				oNR, oES, oLS, oLE,
				oSS, oSU, oUU, oINVALID};


bfShift decode_BBB_table(uint16_t BBB);
void decode_cccc_table(const uint16_t cccc, op_mnem& mnemonic);
void decode_DDDDD_table(const uint16_t DDDDD, reg_id& SD);
void decode_DD_table(const uint16_t DD, reg_id& SD);
void decode_DDF_table(const uint16_t DD, const uint16_t F, reg_id& S, reg_id& D);
void decode_EE_table(const uint16_t EE, reg_id& D);
void decode_F_table(const uint16_t F, reg_id& SD);
void decode_h0hF_table(const uint16_t h0h, uint16_t F, reg_id& S, reg_id& D);
void decode_HH_table(const uint16_t HH, reg_id& SD);
void decode_HHH_table(const uint16_t HHH, reg_id& SD);
void decode_IIIIx_table(const uint16_t IIII, const uint16_t x, reg_id& S, reg_id& D);
void decode_JJJF_table(const uint16_t JJJ, const uint16_t F, reg_id& S, reg_id& D);
void decode_JJF_table(const uint16_t JJ, const uint16_t F, reg_id& S, reg_id& D);
void decode_JF_table(const uint16_t J, const uint16_t F, reg_id& S, reg_id& D);
void decode_kSign_table(const uint16_t k, std::string& plusMinus);
void decode_KKK_table(const uint16_t KKK, reg_id& D1, reg_id& D2);
void decode_NN_table(uint16_t NN, reg_id& ret);
void decode_TT_table(uint16_t TT, reg_id& ret);
void decode_QQF_table(const uint16_t QQ, const uint16_t F, reg_id& S1, reg_id& S2, reg_id& D);
void decode_QQF_special_table(const uint16_t QQ, const uint16_t F, reg_id& S1, reg_id& S2, reg_id& D);
void decode_QQQF_table(const uint16_t QQQ, const uint16_t F, reg_id& S1, reg_id& S2, reg_id& D);
void decode_RR_table(uint16_t RR, reg_id& ret);
void decode_rr_table(uint16_t rr, reg_id& ret);
void decode_s_table(const uint16_t s, op_mnem& arithmetic);
void decode_ss_table(const uint16_t ss, op_mnem& arithmetic);
void decode_uuuuF_table(const uint16_t uuuu, const uint16_t F, std::string& arg, reg_id& S, reg_id& D);
void decode_Z_table(const uint16_t Z, std::string& ea);

void assemble_ea_from_m_table(const uint16_t m, const int n, std::string& ea);
void assemble_eas_from_mm_table(uint16_t mm, int n1, int n2, std::string& ea1, std::string& ea2);
void assemble_ea_from_MM_table(uint16_t MM, int n, std::string& ea);
void assemble_ea_from_q_table(uint16_t q, int n, std::string& ea);
void assemble_ea_from_t_table(uint16_t t, uint16_t val, std::string& ea);
void assemble_ea_from_z_table(uint16_t z, int n, std::string& ea);
void assemble_D_from_P_table(uint16_t P, uint16_t ppppp, std::string& D);
void assemble_arguments_from_W_table(uint16_t W, char ma, const reg_id& SD, const std::string& ea, std::string& S, std::string& D);
void assemble_arguments_from_W_table(uint16_t W, char ma, const std::string& SD, const std::string& ea, std::string& S, std::string& D);
void assemble_reg_from_W_table(uint16_t W, char ma, const reg_id& SD, const int8_t xx, std::string& S, std::string& D);
void assemble_address_from_IO_short_address(uint16_t pp, std::string& ea);

int8_t get_6_bit_signed_value(uint16_t bits);

// Helpers
uint16_t dsp56156_op_maskn(uint16_t cur, uint16_t mask);
inline uint16_t BITSn(uint16_t cur, uint16_t mask) { return dsp56156_op_maskn(cur, mask); }

bool registerOverlap(const reg_id& r0, const size_t bmd, const reg_id& r1);

uint16_t regValue16(dsp56156_core* cpustate, const reg_id& reg);
void setReg16(dsp56156_core* cpustate, const uint16_t& value, const reg_id& reg);

std::string regIdAsString(const reg_id& regId);
std::string opMnemonicAsString(const op_mnem& mnem);
reg_id stringAsRegID(const std::string& str);
uint8_t regIDAsNum(const reg_id& regId);

} // namespace DSP_56156

#endif // MAME_CPU_DSP56156_TABLES_H
