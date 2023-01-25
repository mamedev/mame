// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"


uint32_t arcompact_device::arcompact_handle19_0x_helper(uint16_t op, const char* optext, int shift, int format)
{
	arcompact_log("unimplemented %s %04x (0x19_0x group)", optext, op);
	return m_pc + 2;
}

// #######################################################################################################################
//                                 IIII ISS
// LD_S r0,[gp,s11]                1100 100s ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_LD_S_r0_gp_s11(uint16_t op)
{
	return arcompact_handle19_0x_helper(op, "LD_S", 2, 0);
}

// #######################################################################################################################
//                                 IIII ISS
// LDB_S r0,[gp,s9]                1100 101s ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDB_S_r0_gp_s9(uint16_t op)
{
	return arcompact_handle19_0x_helper(op, "LDB_S", 0, 0);
}

// #######################################################################################################################
//                                 IIII ISS
// LDW_S r0,[gp,s10]               1100 110s ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_LDW_S_r0_gp_s10(uint16_t op)
{
	return arcompact_handle19_0x_helper(op, "LDW_S", 1, 0);
}

// #######################################################################################################################
//                                 IIII ISS
// ADD_S r0,gp,s11                 1100 111s ssss ssss
// #######################################################################################################################

uint32_t arcompact_device::handleop_ADD_S_r0_gp_s11(uint16_t op)
{
	return arcompact_handle19_0x_helper(op, "ADD_S", 2, 1);
}

