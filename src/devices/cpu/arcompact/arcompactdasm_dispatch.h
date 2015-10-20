// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact disassembler

\*********************************/

#define DASM_OPS_16 char *output, offs_t pc, UINT16 op, const UINT8* oprom
#define DASM_OPS_32 char *output, offs_t pc, UINT32 op, const UINT8* oprom
#define DASM_PARAMS output, pc, op, oprom

#define LIMM_REG 62

#define GET_LIMM_32 \
	limm = oprom[6] | (oprom[7] << 8); \
	limm |= (oprom[4] << 16) | (oprom[5] << 24);


int arcompact_handle00_dasm(DASM_OPS_32);
int arcompact_handle01_dasm(DASM_OPS_32);
int arcompact_handle01_00_dasm(DASM_OPS_32);
int arcompact_handle01_01_dasm(DASM_OPS_32);
int arcompact_handle01_01_00_dasm(DASM_OPS_32);
int arcompact_handle01_01_01_dasm(DASM_OPS_32);
int arcompact_handle04_dasm(DASM_OPS_32);
int arcompact_handle04_2f_dasm(DASM_OPS_32);
int arcompact_handle04_2f_3f_dasm(DASM_OPS_32);
int arcompact_handle05_dasm(DASM_OPS_32);

int arcompact_handle05_2f_dasm(DASM_OPS_32);
int arcompact_handle05_2f_3f_dasm(DASM_OPS_32);


int arcompact_handle0c_dasm(DASM_OPS_16);
int arcompact_handle0d_dasm(DASM_OPS_16);
int arcompact_handle0e_dasm(DASM_OPS_16);
int arcompact_handle0f_dasm(DASM_OPS_16);
int arcompact_handle0f_00_dasm(DASM_OPS_16);
int arcompact_handle0f_00_07_dasm(DASM_OPS_16);
int arcompact_handle17_dasm(DASM_OPS_16);
int arcompact_handle18_dasm(DASM_OPS_16);
int arcompact_handle18_05_dasm(DASM_OPS_16);
int arcompact_handle18_06_dasm(DASM_OPS_16);
int arcompact_handle18_07_dasm(DASM_OPS_16);
int arcompact_handle19_dasm(DASM_OPS_16);
int arcompact_handle1c_dasm(DASM_OPS_16);
int arcompact_handle1d_dasm(DASM_OPS_16);
int arcompact_handle1e_dasm(DASM_OPS_16);
int arcompact_handle1e_03_dasm(DASM_OPS_16);
