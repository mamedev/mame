// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_regs.cpp

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#include "emu.h"
#include "voodoo.h"
#include "voodoo_banshee.h"

using namespace voodoo;

u32 register_table_entry::write(voodoo_device_base &device, u32 chipmask, u32 regnum, u32 data) const
{
	// statistics if enabled
	if (DEBUG_STATS)
		device.m_stats.reg_writes++;

	// log if enabled
	if (LOG_REGISTERS)
	{
		if (regnum < voodoo_regs::reg_fvertexAx || regnum > voodoo_regs::reg_fdWdY)
			device.logerror("VOODOO.REG:%s(%d) write = %08X\n", m_name, chipmask, data);
		else
			device.logerror("VOODOO.REG:%s(%d) write = %f\n", m_name, chipmask, double(u2f(data)));
	}
	return m_write(chipmask & m_chipmask_flags, regnum, data & m_mask);
}

u32 register_table_entry::read(voodoo_device_base &device, u32 chipmask, u32 regnum) const
{
	u32 result = m_read(chipmask & m_chipmask_flags, regnum) & m_mask;
	if (LOG_REGISTERS)
	{
		// don't log multiple identical status reads from the same address
		bool logit = true;
		if (regnum == voodoo_regs::reg_vdstatus)
		{
			offs_t pc = device.m_cpu->pc();
			if (pc == device.m_last_status_pc && result == device.m_last_status_value)
				logit = false;
			device.m_last_status_pc = pc;
			device.m_last_status_value = result;
		}
		if (regnum == voodoo_regs::reg_cmdFifoRdPtr)
			logit = false;

		if (logit)
			device.logerror("VOODOO.REG:%s read = %08X\n", m_name, result);
	}
	return result;
}



//**************************************************************************
//  ALIAS MAP OF FIRST 64 REGISTERS
//**************************************************************************

u8 const voodoo_regs::s_alias_map[0x40] =
{
	voodoo_regs::reg_vdstatus,   0x004/4,                     voodoo_regs::reg_vertexAx,   voodoo_regs::reg_vertexAy,
	voodoo_regs::reg_vertexBx,   voodoo_regs::reg_vertexBy,   voodoo_regs::reg_vertexCx,   voodoo_regs::reg_vertexCy,
	voodoo_regs::reg_startR,     voodoo_regs::reg_dRdX,       voodoo_regs::reg_dRdY,       voodoo_regs::reg_startG,
	voodoo_regs::reg_dGdX,       voodoo_regs::reg_dGdY,       voodoo_regs::reg_startB,     voodoo_regs::reg_dBdX,
	voodoo_regs::reg_dBdY,       voodoo_regs::reg_startZ,     voodoo_regs::reg_dZdX,       voodoo_regs::reg_dZdY,
	voodoo_regs::reg_startA,     voodoo_regs::reg_dAdX,       voodoo_regs::reg_dAdY,       voodoo_regs::reg_startS,
	voodoo_regs::reg_dSdX,       voodoo_regs::reg_dSdY,       voodoo_regs::reg_startT,     voodoo_regs::reg_dTdX,
	voodoo_regs::reg_dTdY,       voodoo_regs::reg_startW,     voodoo_regs::reg_dWdX,       voodoo_regs::reg_dWdY,

	voodoo_regs::reg_triangleCMD,0x084/4,                     voodoo_regs::reg_fvertexAx,  voodoo_regs::reg_fvertexAy,
	voodoo_regs::reg_fvertexBx,  voodoo_regs::reg_fvertexBy,  voodoo_regs::reg_fvertexCx,  voodoo_regs::reg_fvertexCy,
	voodoo_regs::reg_fstartR,    voodoo_regs::reg_fdRdX,      voodoo_regs::reg_fdRdY,      voodoo_regs::reg_fstartG,
	voodoo_regs::reg_fdGdX,      voodoo_regs::reg_fdGdY,      voodoo_regs::reg_fstartB,    voodoo_regs::reg_fdBdX,
	voodoo_regs::reg_fdBdY,      voodoo_regs::reg_fstartZ,    voodoo_regs::reg_fdZdX,      voodoo_regs::reg_fdZdY,
	voodoo_regs::reg_fstartA,    voodoo_regs::reg_fdAdX,      voodoo_regs::reg_fdAdY,      voodoo_regs::reg_fstartS,
	voodoo_regs::reg_fdSdX,      voodoo_regs::reg_fdSdY,      voodoo_regs::reg_fstartT,    voodoo_regs::reg_fdTdX,
	voodoo_regs::reg_fdTdY,      voodoo_regs::reg_fstartW,    voodoo_regs::reg_fdWdX,      voodoo_regs::reg_fdWdY
};


//**************************************************************************
//  REGISTER TABLE HELPERS
//**************************************************************************

#define REGISTER_TABLE_BEGIN \
	static_register_table_entry<REGISTER_TABLE_TYPE> const REGISTER_TABLE_TYPE::s_register_table[256] = {

#define REGISTER_TABLE_END \
	};

constexpr u32 make_mask(int maskbits)
{
	if (maskbits == 0)
		return 0;
	if (maskbits == 32)
		return 0xffffffff;
	return (1 << maskbits) - 1;
}

#define REGISTER_ENTRY(name, reader, writer, bits, chips, sync, fifo) \
	{ make_mask(bits), register_table_entry::CHIPMASK_##chips | register_table_entry::SYNC_##sync | register_table_entry::FIFO_##fifo, #name, &REGISTER_TABLE_TYPE::reg_##writer##_w, &REGISTER_TABLE_TYPE::reg_##reader##_r },

#define RESERVED_ENTRY REGISTER_ENTRY(reserved, invalid, invalid, 32, FBI, NOSYNC, FIFO)

#define RESERVED_ENTRY_x8 RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY



//**************************************************************************
//  VOODOO 1 REGISTER MAP
//**************************************************************************

#define REGISTER_TABLE_TYPE voodoo_1_device

REGISTER_TABLE_BEGIN
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(status,          status,      unimplemented,32,FBI,      NOSYNC,   FIFO)    // 000
	RESERVED_ENTRY                                                                             // 004
	REGISTER_ENTRY(vertexAx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 008
	REGISTER_ENTRY(vertexAy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 00c
	REGISTER_ENTRY(vertexBx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 010
	REGISTER_ENTRY(vertexBy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 014
	REGISTER_ENTRY(vertexCx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 018
	REGISTER_ENTRY(vertexCy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 01c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(startR,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 020
	REGISTER_ENTRY(startG,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 024
	REGISTER_ENTRY(startB,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 028
	REGISTER_ENTRY(startZ,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 02c
	REGISTER_ENTRY(startA,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 030
	REGISTER_ENTRY(startS,          invalid,     starts,      32, TREX,     NOSYNC,   FIFO)    // 034
	REGISTER_ENTRY(startT,          invalid,     startt,      32, TREX,     NOSYNC,   FIFO)    // 038
	REGISTER_ENTRY(startW,          invalid,     startw,      32, FBI_TREX, NOSYNC,   FIFO)    // 03c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 040
	REGISTER_ENTRY(dGdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 044
	REGISTER_ENTRY(dBdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 048
	REGISTER_ENTRY(dZdX,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 04c
	REGISTER_ENTRY(dAdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 050
	REGISTER_ENTRY(dSdX,            invalid,     dsdx,        32, TREX,     NOSYNC,   FIFO)    // 054
	REGISTER_ENTRY(dTdX,            invalid,     dtdx,        32, TREX,     NOSYNC,   FIFO)    // 058
	REGISTER_ENTRY(dWdX,            invalid,     dwdx,        32, FBI_TREX, NOSYNC,   FIFO)    // 05c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 060
	REGISTER_ENTRY(dGdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 064
	REGISTER_ENTRY(dBdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 068
	REGISTER_ENTRY(dZdY,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 06c
	REGISTER_ENTRY(dAdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 070
	REGISTER_ENTRY(dSdY,            invalid,     dsdy,        32, TREX,     NOSYNC,   FIFO)    // 074
	REGISTER_ENTRY(dTdY,            invalid,     dtdy,        32, TREX,     NOSYNC,   FIFO)    // 078
	REGISTER_ENTRY(dWdY,            invalid,     dwdy,        32, FBI_TREX, NOSYNC,   FIFO)    // 07c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(triangleCMD,     invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 080
	RESERVED_ENTRY                                                                             // 084
	REGISTER_ENTRY(fvertexAx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 088
	REGISTER_ENTRY(fvertexAy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 08c
	REGISTER_ENTRY(fvertexBx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 090
	REGISTER_ENTRY(fvertexBy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 094
	REGISTER_ENTRY(fvertexCx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 098
	REGISTER_ENTRY(fvertexCy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 09c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fstartR,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a0
	REGISTER_ENTRY(fstartG,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a4
	REGISTER_ENTRY(fstartB,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a8
	REGISTER_ENTRY(fstartZ,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ac
	REGISTER_ENTRY(fstartA,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0b0
	REGISTER_ENTRY(fstartS,         invalid,     fstarts,     32, TREX,     NOSYNC,   FIFO)    // 0b4
	REGISTER_ENTRY(fstartT,         invalid,     fstartt,     32, TREX,     NOSYNC,   FIFO)    // 0b8
	REGISTER_ENTRY(fstartW,         invalid,     fstartw,     32, FBI_TREX, NOSYNC,   FIFO)    // 0bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c0
	REGISTER_ENTRY(fdGdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c4
	REGISTER_ENTRY(fdBdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c8
	REGISTER_ENTRY(fdZdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0cc
	REGISTER_ENTRY(fdAdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0d0
	REGISTER_ENTRY(fdSdX,           invalid,     fdsdx,       32, TREX,     NOSYNC,   FIFO)    // 0d4
	REGISTER_ENTRY(fdTdX,           invalid,     fdtdx,       32, TREX,     NOSYNC,   FIFO)    // 0d8
	REGISTER_ENTRY(fdWdX,           invalid,     fdwdx,       32, FBI_TREX, NOSYNC,   FIFO)    // 0dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e0
	REGISTER_ENTRY(fdGdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e4
	REGISTER_ENTRY(fdBdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e8
	REGISTER_ENTRY(fdZdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ec
	REGISTER_ENTRY(fdAdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0f0
	REGISTER_ENTRY(fdSdY,           invalid,     fdsdy,       32, TREX,     NOSYNC,   FIFO)    // 0f4
	REGISTER_ENTRY(fdTdY,           invalid,     fdtdy,       32, TREX,     NOSYNC,   FIFO)    // 0f8
	REGISTER_ENTRY(fdWdY,           invalid,     fdwdy,       32, FBI_TREX, NOSYNC,   FIFO)    // 0fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(ftriangleCMD,    invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 100
	REGISTER_ENTRY(fbzColorPath,    passive,     passive,     28, FBI_TREX, NOSYNC,   FIFO)    // 104
	REGISTER_ENTRY(fogMode,         passive,     passive,      6, FBI_TREX, NOSYNC,   FIFO)    // 108
	REGISTER_ENTRY(alphaMode,       passive,     passive,     32, FBI_TREX, NOSYNC,   FIFO)    // 10c
	REGISTER_ENTRY(fbzMode,         passive,     passive,     21, FBI_TREX,   SYNC,   FIFO)    // 110
	REGISTER_ENTRY(lfbMode,         passive,     passive,     17, FBI_TREX,   SYNC,   FIFO)    // 114
	REGISTER_ENTRY(clipLeftRight,   passive,     passive,     26, FBI_TREX,   SYNC,   FIFO)    // 118
	REGISTER_ENTRY(clipLowYHighY,   passive,     passive,     26, FBI_TREX,   SYNC,   FIFO)    // 11c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nopCMD,          invalid,     nop,          1, FBI_TREX,   SYNC,   FIFO)    // 120
	REGISTER_ENTRY(fastfillCMD,     invalid,     fastfill,     0, FBI,        SYNC,   FIFO)    // 124
	REGISTER_ENTRY(swapbufferCMD,   invalid,     swapbuffer,   9, FBI,        SYNC,   FIFO)    // 128
	REGISTER_ENTRY(fogColor,        invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 12c
	REGISTER_ENTRY(zaColor,         invalid,     passive,     32, FBI,        SYNC,   FIFO)    // 130
	REGISTER_ENTRY(chromaKey,       invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 134
	RESERVED_ENTRY                                                                             // 138
	RESERVED_ENTRY                                                                             // 13c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(stipple,         passive,     passive,     32, FBI,        SYNC,   FIFO)    // 140
	REGISTER_ENTRY(color0,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 144
	REGISTER_ENTRY(color1,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 148
	REGISTER_ENTRY(fbiPixelsIn,     stats,       invalid,     24, FBI,          NA,     NA)    // 14c
	REGISTER_ENTRY(fbiChromaFail,   stats,       invalid,     24, FBI,          NA,     NA)    // 150
	REGISTER_ENTRY(fbiZfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 154
	REGISTER_ENTRY(fbiAfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 158
	REGISTER_ENTRY(fbiPixelsOut,    stats,       invalid,     24, FBI,          NA,     NA)    // 15c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[0],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 160
	REGISTER_ENTRY(fogTable[1],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 164
	REGISTER_ENTRY(fogTable[2],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 168
	REGISTER_ENTRY(fogTable[3],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 16c
	REGISTER_ENTRY(fogTable[4],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 170
	REGISTER_ENTRY(fogTable[5],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 174
	REGISTER_ENTRY(fogTable[6],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 178
	REGISTER_ENTRY(fogTable[7],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 17c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[8],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 180
	REGISTER_ENTRY(fogTable[9],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 184
	REGISTER_ENTRY(fogTable[10],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 188
	REGISTER_ENTRY(fogTable[11],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 18c
	REGISTER_ENTRY(fogTable[12],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 190
	REGISTER_ENTRY(fogTable[13],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 194
	REGISTER_ENTRY(fogTable[14],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 198
	REGISTER_ENTRY(fogTable[15],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 19c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[16],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a0
	REGISTER_ENTRY(fogTable[17],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a4
	REGISTER_ENTRY(fogTable[18],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a8
	REGISTER_ENTRY(fogTable[19],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1ac
	REGISTER_ENTRY(fogTable[20],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b0
	REGISTER_ENTRY(fogTable[21],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b4
	REGISTER_ENTRY(fogTable[22],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b8
	REGISTER_ENTRY(fogTable[23],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[24],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c0
	REGISTER_ENTRY(fogTable[25],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c4
	REGISTER_ENTRY(fogTable[26],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c8
	REGISTER_ENTRY(fogTable[27],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1cc
	REGISTER_ENTRY(fogTable[28],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d0
	REGISTER_ENTRY(fogTable[29],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d4
	REGISTER_ENTRY(fogTable[30],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d8
	REGISTER_ENTRY(fogTable[31],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                // 1e0-1fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fbiInit4,        passive,     fbiinit,     28, FBI,      NOSYNC, NOFIFO)    // 200
	REGISTER_ENTRY(vRetrace,        vretrace,    invalid,     12, FBI,          NA,     NA)    // 204
	REGISTER_ENTRY(backPorch,       passive,     video,       24, FBI,      NOSYNC, NOFIFO)    // 208
	REGISTER_ENTRY(videoDimensions, passive,     video,       26, FBI,      NOSYNC, NOFIFO)    // 20c
	REGISTER_ENTRY(fbiInit0,        passive,     fbiinit,     31, FBI,      NOSYNC, NOFIFO)    // 210
	REGISTER_ENTRY(fbiInit1,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 214
	REGISTER_ENTRY(fbiInit2,        fbiinit2,    fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 218
	REGISTER_ENTRY(fbiInit3,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 21c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(hSync,           invalid,     video,       26, FBI,      NOSYNC, NOFIFO)    // 220
	REGISTER_ENTRY(vSync,           invalid,     video,       28, FBI,      NOSYNC, NOFIFO)    // 224
	REGISTER_ENTRY(clutData,        invalid,     clut,        30, FBI,      NOSYNC, NOFIFO)    // 228
	REGISTER_ENTRY(dacData,         invalid,     dac,         12, FBI,      NOSYNC, NOFIFO)    // 22c
	REGISTER_ENTRY(maxRgbDelta,     invalid,     unimplemented,24, FBI,      NOSYNC, NOFIFO)    // 230
	RESERVED_ENTRY                                                                             // 234
	RESERVED_ENTRY                                                                             // 238
	RESERVED_ENTRY                                                                             // 23c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 240-25c
	RESERVED_ENTRY_x8                                                                          // 260-27c
	RESERVED_ENTRY_x8                                                                          // 280-29c
	RESERVED_ENTRY_x8                                                                          // 2a0-2bc
	RESERVED_ENTRY_x8                                                                          // 2c0-2dc
	RESERVED_ENTRY_x8                                                                          // 2e0-2fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(textureMode,     invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 300
	REGISTER_ENTRY(tLOD,            invalid,     texture,     28, TREX,     NOSYNC,   FIFO)    // 304
	REGISTER_ENTRY(tDetail,         invalid,     texture,     17, TREX,     NOSYNC,   FIFO)    // 308
	REGISTER_ENTRY(texBaseAddr,     invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 30c
	REGISTER_ENTRY(texBaseAddr_1,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 310
	REGISTER_ENTRY(texBaseAddr_2,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 314
	REGISTER_ENTRY(texBaseAddr_3_8, invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 318
	REGISTER_ENTRY(trexInit0,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 31c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(trexInit1,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 320
	REGISTER_ENTRY(nccTable0[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 324
	REGISTER_ENTRY(nccTable0[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 328
	REGISTER_ENTRY(nccTable0[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 32c
	REGISTER_ENTRY(nccTable0[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 330
	REGISTER_ENTRY(nccTable0[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 334
	REGISTER_ENTRY(nccTable0[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 338
	REGISTER_ENTRY(nccTable0[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 33c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable0[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 340
	REGISTER_ENTRY(nccTable0[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 344
	REGISTER_ENTRY(nccTable0[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 348
	REGISTER_ENTRY(nccTable0[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 34c
	REGISTER_ENTRY(nccTable0[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 350
	REGISTER_ENTRY(nccTable1[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 354
	REGISTER_ENTRY(nccTable1[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 358
	REGISTER_ENTRY(nccTable1[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 35c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 360
	REGISTER_ENTRY(nccTable1[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 364
	REGISTER_ENTRY(nccTable1[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 368
	REGISTER_ENTRY(nccTable1[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 36c
	REGISTER_ENTRY(nccTable1[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 370
	REGISTER_ENTRY(nccTable1[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 374
	REGISTER_ENTRY(nccTable1[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 378
	REGISTER_ENTRY(nccTable1[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 37c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 380
	RESERVED_ENTRY                                                                             // 384
	RESERVED_ENTRY                                                                             // 388
	RESERVED_ENTRY                                                                             // 38c
	RESERVED_ENTRY                                                                             // 390
	RESERVED_ENTRY                                                                             // 394
	RESERVED_ENTRY                                                                             // 398
	RESERVED_ENTRY                                                                             // 39c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 3a0-3bc
	RESERVED_ENTRY_x8                                                                          // 3c0-3dc
	RESERVED_ENTRY_x8                                                                          // 3e0-3fc
REGISTER_TABLE_END

#undef REGISTER_TABLE_TYPE


//**************************************************************************
//  VOODOO 2 REGISTER MAP
//**************************************************************************

#define REGISTER_TABLE_TYPE voodoo_2_device

REGISTER_TABLE_BEGIN
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(status,          status,      invalid,     32, FBI,      NOSYNC,   FIFO)    // 000
	REGISTER_ENTRY(intrCtrl,        passive,     intrctrl,    32, FBI,      NOSYNC, NOFIFO)    // 004 - cmdFIFO mode
	REGISTER_ENTRY(vertexAx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 008
	REGISTER_ENTRY(vertexAy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 00c
	REGISTER_ENTRY(vertexBx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 010
	REGISTER_ENTRY(vertexBy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 014
	REGISTER_ENTRY(vertexCx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 018
	REGISTER_ENTRY(vertexCy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 01c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(startR,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 020
	REGISTER_ENTRY(startG,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 024
	REGISTER_ENTRY(startB,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 028
	REGISTER_ENTRY(startZ,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 02c
	REGISTER_ENTRY(startA,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 030
	REGISTER_ENTRY(startS,          invalid,     starts,      32, TREX,     NOSYNC,   FIFO)    // 034
	REGISTER_ENTRY(startT,          invalid,     startt,      32, TREX,     NOSYNC,   FIFO)    // 038
	REGISTER_ENTRY(startW,          invalid,     startw,      32, FBI_TREX, NOSYNC,   FIFO)    // 03c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 040
	REGISTER_ENTRY(dGdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 044
	REGISTER_ENTRY(dBdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 048
	REGISTER_ENTRY(dZdX,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 04c
	REGISTER_ENTRY(dAdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 050
	REGISTER_ENTRY(dSdX,            invalid,     dsdx,        32, TREX,     NOSYNC,   FIFO)    // 054
	REGISTER_ENTRY(dTdX,            invalid,     dtdx,        32, TREX,     NOSYNC,   FIFO)    // 058
	REGISTER_ENTRY(dWdX,            invalid,     dwdx,        32, FBI_TREX, NOSYNC,   FIFO)    // 05c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 060
	REGISTER_ENTRY(dGdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 064
	REGISTER_ENTRY(dBdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 068
	REGISTER_ENTRY(dZdY,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 06c
	REGISTER_ENTRY(dAdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 070
	REGISTER_ENTRY(dSdY,            invalid,     dsdy,        32, TREX,     NOSYNC,   FIFO)    // 074
	REGISTER_ENTRY(dTdY,            invalid,     dtdy,        32, TREX,     NOSYNC,   FIFO)    // 078
	REGISTER_ENTRY(dWdY,            invalid,     dwdy,        32, FBI_TREX, NOSYNC,   FIFO)    // 07c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(triangleCMD,     invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 080
	RESERVED_ENTRY                                                                             // 084
	REGISTER_ENTRY(fvertexAx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 088
	REGISTER_ENTRY(fvertexAy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 08c
	REGISTER_ENTRY(fvertexBx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 090
	REGISTER_ENTRY(fvertexBy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 094
	REGISTER_ENTRY(fvertexCx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 098
	REGISTER_ENTRY(fvertexCy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 09c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fstartR,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a0
	REGISTER_ENTRY(fstartG,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a4
	REGISTER_ENTRY(fstartB,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a8
	REGISTER_ENTRY(fstartZ,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ac
	REGISTER_ENTRY(fstartA,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0b0
	REGISTER_ENTRY(fstartS,         invalid,     fstarts,     32, TREX,     NOSYNC,   FIFO)    // 0b4
	REGISTER_ENTRY(fstartT,         invalid,     fstartt,     32, TREX,     NOSYNC,   FIFO)    // 0b8
	REGISTER_ENTRY(fstartW,         invalid,     fstartw,     32, FBI_TREX, NOSYNC,   FIFO)    // 0bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c0
	REGISTER_ENTRY(fdGdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c4
	REGISTER_ENTRY(fdBdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c8
	REGISTER_ENTRY(fdZdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0cc
	REGISTER_ENTRY(fdAdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0d0
	REGISTER_ENTRY(fdSdX,           invalid,     fdsdx,       32, TREX,     NOSYNC,   FIFO)    // 0d4
	REGISTER_ENTRY(fdTdX,           invalid,     fdtdx,       32, TREX,     NOSYNC,   FIFO)    // 0d8
	REGISTER_ENTRY(fdWdX,           invalid,     fdwdx,       32, FBI_TREX, NOSYNC,   FIFO)    // 0dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e0
	REGISTER_ENTRY(fdGdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e4
	REGISTER_ENTRY(fdBdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e8
	REGISTER_ENTRY(fdZdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ec
	REGISTER_ENTRY(fdAdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0f0
	REGISTER_ENTRY(fdSdY,           invalid,     fdsdy,       32, TREX,     NOSYNC,   FIFO)    // 0f4
	REGISTER_ENTRY(fdTdY,           invalid,     fdtdy,       32, TREX,     NOSYNC,   FIFO)    // 0f8
	REGISTER_ENTRY(fdWdY,           invalid,     fdwdy,       32, FBI_TREX, NOSYNC,   FIFO)    // 0fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(ftriangleCMD,    invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 100
	REGISTER_ENTRY(fbzColorPath,    passive,     passive,     30, FBI_TREX, NOSYNC,   FIFO)    // 104
	REGISTER_ENTRY(fogMode,         passive,     passive,      8, FBI_TREX, NOSYNC,   FIFO)    // 108
	REGISTER_ENTRY(alphaMode,       passive,     passive,     32, FBI_TREX, NOSYNC,   FIFO)    // 10c
	REGISTER_ENTRY(fbzMode,         passive,     passive,     22, FBI_TREX,   SYNC,   FIFO)    // 110
	REGISTER_ENTRY(lfbMode,         passive,     passive,     17, FBI_TREX,   SYNC,   FIFO)    // 114
	REGISTER_ENTRY(clipLeftRight,   passive,     passive,     28, FBI_TREX,   SYNC,   FIFO)    // 118
	REGISTER_ENTRY(clipLowYHighY,   passive,     passive,     28, FBI_TREX,   SYNC,   FIFO)    // 11c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nopCMD,          invalid,     nop,          2, FBI_TREX,   SYNC,   FIFO)    // 120
	REGISTER_ENTRY(fastfillCMD,     invalid,     fastfill,     0, FBI,        SYNC,   FIFO)    // 124
	REGISTER_ENTRY(swapbufferCMD,   invalid,     swapbuffer,  10, FBI,        SYNC,   FIFO)    // 128
	REGISTER_ENTRY(fogColor,        invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 12c
	REGISTER_ENTRY(zaColor,         invalid,     passive,     32, FBI,        SYNC,   FIFO)    // 130
	REGISTER_ENTRY(chromaKey,       invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 134
	REGISTER_ENTRY(chromaRange,     invalid,     passive,     29, FBI,        SYNC,   FIFO)    // 138
	REGISTER_ENTRY(userIntrCMD,     invalid,     userintr,    10, FBI,        SYNC,   FIFO)    // 13c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(stipple,         passive,     passive,     32, FBI,        SYNC,   FIFO)    // 140
	REGISTER_ENTRY(color0,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 144
	REGISTER_ENTRY(color1,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 148
	REGISTER_ENTRY(fbiPixelsIn,     stats,       invalid,     24, FBI,          NA,     NA)    // 14c
	REGISTER_ENTRY(fbiChromaFail,   stats,       invalid,     24, FBI,          NA,     NA)    // 150
	REGISTER_ENTRY(fbiZfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 154
	REGISTER_ENTRY(fbiAfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 158
	REGISTER_ENTRY(fbiPixelsOut,    stats,       invalid,     24, FBI,          NA,     NA)    // 15c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[0],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 160
	REGISTER_ENTRY(fogTable[1],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 164
	REGISTER_ENTRY(fogTable[2],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 168
	REGISTER_ENTRY(fogTable[3],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 16c
	REGISTER_ENTRY(fogTable[4],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 170
	REGISTER_ENTRY(fogTable[5],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 174
	REGISTER_ENTRY(fogTable[6],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 178
	REGISTER_ENTRY(fogTable[7],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 17c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[8],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 180
	REGISTER_ENTRY(fogTable[9],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 184
	REGISTER_ENTRY(fogTable[10],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 188
	REGISTER_ENTRY(fogTable[11],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 18c
	REGISTER_ENTRY(fogTable[12],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 190
	REGISTER_ENTRY(fogTable[13],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 194
	REGISTER_ENTRY(fogTable[14],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 198
	REGISTER_ENTRY(fogTable[15],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 19c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[16],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a0
	REGISTER_ENTRY(fogTable[17],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a4
	REGISTER_ENTRY(fogTable[18],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a8
	REGISTER_ENTRY(fogTable[19],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1ac
	REGISTER_ENTRY(fogTable[20],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b0
	REGISTER_ENTRY(fogTable[21],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b4
	REGISTER_ENTRY(fogTable[22],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b8
	REGISTER_ENTRY(fogTable[23],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[24],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c0
	REGISTER_ENTRY(fogTable[25],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c4
	REGISTER_ENTRY(fogTable[26],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c8
	REGISTER_ENTRY(fogTable[27],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1cc
	REGISTER_ENTRY(fogTable[28],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d0
	REGISTER_ENTRY(fogTable[29],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d4
	REGISTER_ENTRY(fogTable[30],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d8
	REGISTER_ENTRY(fogTable[31],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(cmdFifoBaseAddr, passive,     cmdfifo,     26, FBI,        SYNC, NOFIFO)    // 1e0 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoBump,     passive,     unimplemented,16,FBI,        SYNC, NOFIFO)    // 1e4 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoRdPtr,    cmdfifoptr,  cmdfifoptr,  32, FBI,        SYNC, NOFIFO)    // 1e8 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoAMin,     passive,     cmdfifo,     32, FBI,        SYNC, NOFIFO)    // 1ec - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoAMax,     passive,     cmdfifo,     32, FBI,        SYNC, NOFIFO)    // 1f0 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoDepth,    cmdfifodepth,cmdfifodepth,16, FBI,        SYNC, NOFIFO)    // 1f4 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoHoles,    cmdfifoholes,cmdfifoholes,16, FBI,        SYNC, NOFIFO)    // 1f8 - cmdFIFO mode
	RESERVED_ENTRY                                                                             // 1fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fbiInit4,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 200
	REGISTER_ENTRY(vRetrace,        vretrace,    invalid,     13, FBI,          NA,     NA)    // 204
	REGISTER_ENTRY(backPorch,       passive,     video,       25, FBI,      NOSYNC, NOFIFO)    // 208
	REGISTER_ENTRY(videoDimensions, passive,     video,       27, FBI,      NOSYNC, NOFIFO)    // 20c
	REGISTER_ENTRY(fbiInit0,        passive,     fbiinit,     31, FBI,      NOSYNC, NOFIFO)    // 210
	REGISTER_ENTRY(fbiInit1,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 214
	REGISTER_ENTRY(fbiInit2,        fbiinit2,    fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 218
	REGISTER_ENTRY(fbiInit3,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 21c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(hSync,           invalid,     video,       27, FBI,      NOSYNC, NOFIFO)    // 220
	REGISTER_ENTRY(vSync,           invalid,     video,       29, FBI,      NOSYNC, NOFIFO)    // 224
	REGISTER_ENTRY(clutData,        invalid,     clut,        30, FBI,      NOSYNC, NOFIFO)    // 228
	REGISTER_ENTRY(dacData,         invalid,     dac,         14, FBI,      NOSYNC, NOFIFO)    // 22c
	REGISTER_ENTRY(maxRgbDelta,     invalid,     unimplemented,24,FBI,      NOSYNC, NOFIFO)    // 230
	REGISTER_ENTRY(hBorder,         invalid,     video,       25, FBI,      NOSYNC, NOFIFO)    // 234 - cmdFIFO mode
	REGISTER_ENTRY(vBorder,         invalid,     video,       25, FBI,      NOSYNC, NOFIFO)    // 238 - cmdFIFO mode
	REGISTER_ENTRY(borderColor,     invalid,     video,       24, FBI,      NOSYNC, NOFIFO)    // 23c - cmdFIFO mode
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(hvRetrace,       hvretrace,   invalid,     27, FBI,          NA,     NA)    // 240
	REGISTER_ENTRY(fbiInit5,        passive,     fbiinit5_7,  32, FBI,      NOSYNC, NOFIFO)    // 244 - cmdFIFO mode
	REGISTER_ENTRY(fbiInit6,        passive,     fbiinit5_7,  31, FBI,      NOSYNC, NOFIFO)    // 248 - cmdFIFO mode
	REGISTER_ENTRY(fbiInit7,        passive,     fbiinit5_7,  28, FBI,      NOSYNC, NOFIFO)    // 24c - cmdFIFO mode
	RESERVED_ENTRY                                                                             // 250
	RESERVED_ENTRY                                                                             // 254
	REGISTER_ENTRY(fbiSwapHistory,  passive,     invalid,     32, FBI,          NA,     NA)    // 258
	REGISTER_ENTRY(fbiTrianglesOut, passive,     invalid,     24, FBI,          NA,     NA)    // 25c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sSetupMode,      invalid,     passive,     20, FBI,      NOSYNC,   FIFO)    // 260
	REGISTER_ENTRY(sVx,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 264
	REGISTER_ENTRY(sVy,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 268
	REGISTER_ENTRY(sARGB,           invalid,     sargb,       32, FBI,      NOSYNC,   FIFO)    // 26c
	REGISTER_ENTRY(sRed,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 270
	REGISTER_ENTRY(sGreen,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 274
	REGISTER_ENTRY(sBlue,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 278
	REGISTER_ENTRY(sAlpha,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 27c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sVz,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 280
	REGISTER_ENTRY(sWb,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 284
	REGISTER_ENTRY(sWtmu0,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 288
	REGISTER_ENTRY(sS_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 28c
	REGISTER_ENTRY(sT_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 290
	REGISTER_ENTRY(sWtmu1,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 294
	REGISTER_ENTRY(sS_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 298
	REGISTER_ENTRY(sT_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 29c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sDrawTriCMD,     invalid,     draw_tri,     1, FBI,      NOSYNC,   FIFO)    // 2a0
	REGISTER_ENTRY(sBeginTriCMD,    invalid,     begin_tri,    1, FBI,      NOSYNC,   FIFO)    // 2a4
	RESERVED_ENTRY                                                                             // 2a8
	RESERVED_ENTRY                                                                             // 2ac
	RESERVED_ENTRY                                                                             // 2b0
	RESERVED_ENTRY                                                                             // 2b4
	RESERVED_ENTRY                                                                             // 2b8
	RESERVED_ENTRY                                                                             // 2bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(bltSrcBaseAddr,  passive,     passive,     22, FBI,      NOSYNC,   FIFO)    // 2c0
	REGISTER_ENTRY(bltDstBaseAddr,  passive,     passive,     22, FBI,      NOSYNC,   FIFO)    // 2c4
	REGISTER_ENTRY(bltXYStrides,    passive,     passive,     28, FBI,      NOSYNC,   FIFO)    // 2c8
	REGISTER_ENTRY(bltSrcChromaRange,passive,    passive,     32, FBI,      NOSYNC,   FIFO)    // 2cc
	REGISTER_ENTRY(bltDstChromaRange,passive,    passive,     32, FBI,      NOSYNC,   FIFO)    // 2d0
	REGISTER_ENTRY(bltClipX,        passive,     passive,     26, FBI,      NOSYNC,   FIFO)    // 2d4
	REGISTER_ENTRY(bltClipY,        passive,     passive,     26, FBI,      NOSYNC,   FIFO)    // 2d8
	RESERVED_ENTRY                                                                             // 2dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(bltSrcXY,        passive,     passive,     27, FBI,      NOSYNC,   FIFO)    // 2e0
	REGISTER_ENTRY(bltDstXY,        passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2e4
	REGISTER_ENTRY(bltSize,         passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2e8
	REGISTER_ENTRY(bltRop,          passive,     passive,     16, FBI,      NOSYNC,   FIFO)    // 2ec
	REGISTER_ENTRY(bltColor,        passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2f0
	RESERVED_ENTRY                                                                             // 2f4
	REGISTER_ENTRY(bltCommand,      passive,     unimplemented,32,FBI,      NOSYNC,   FIFO)    // 2f8
	REGISTER_ENTRY(bltData,         invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(textureMode,     invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 300
	REGISTER_ENTRY(tLOD,            invalid,     texture,     28, TREX,     NOSYNC,   FIFO)    // 304
	REGISTER_ENTRY(tDetail,         invalid,     texture,     22, TREX,     NOSYNC,   FIFO)    // 308
	REGISTER_ENTRY(texBaseAddr,     invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 30c
	REGISTER_ENTRY(texBaseAddr_1,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 310
	REGISTER_ENTRY(texBaseAddr_2,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 314
	REGISTER_ENTRY(texBaseAddr_3_8, invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 318
	REGISTER_ENTRY(trexInit0,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 31c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(trexInit1,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 320
	REGISTER_ENTRY(nccTable0[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 324
	REGISTER_ENTRY(nccTable0[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 328
	REGISTER_ENTRY(nccTable0[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 32c
	REGISTER_ENTRY(nccTable0[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 330
	REGISTER_ENTRY(nccTable0[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 334
	REGISTER_ENTRY(nccTable0[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 338
	REGISTER_ENTRY(nccTable0[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 33c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable0[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 340
	REGISTER_ENTRY(nccTable0[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 344
	REGISTER_ENTRY(nccTable0[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 348
	REGISTER_ENTRY(nccTable0[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 34c
	REGISTER_ENTRY(nccTable0[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 350
	REGISTER_ENTRY(nccTable1[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 354
	REGISTER_ENTRY(nccTable1[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 358
	REGISTER_ENTRY(nccTable1[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 35c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 360
	REGISTER_ENTRY(nccTable1[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 364
	REGISTER_ENTRY(nccTable1[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 368
	REGISTER_ENTRY(nccTable1[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 36c
	REGISTER_ENTRY(nccTable1[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 370
	REGISTER_ENTRY(nccTable1[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 374
	REGISTER_ENTRY(nccTable1[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 378
	REGISTER_ENTRY(nccTable1[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 37c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 380
	RESERVED_ENTRY                                                                             // 384
	RESERVED_ENTRY                                                                             // 388
	RESERVED_ENTRY                                                                             // 38c
	RESERVED_ENTRY                                                                             // 390
	RESERVED_ENTRY                                                                             // 394
	RESERVED_ENTRY                                                                             // 398
	RESERVED_ENTRY                                                                             // 39c

	RESERVED_ENTRY_x8                                                                          // 3a0-3bc
	RESERVED_ENTRY_x8                                                                          // 3c0-3dc
	RESERVED_ENTRY_x8                                                                          // 3e0-3fc
REGISTER_TABLE_END

#undef REGISTER_TABLE_TYPE


//**************************************************************************
//  VOODOO BANSHEE/VOODOO 3 REGISTER MAP
//**************************************************************************

#define REGISTER_TABLE_TYPE voodoo_banshee_device_base

REGISTER_TABLE_BEGIN
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(status,          status,      invalid,     32, FBI,      NOSYNC,   FIFO)    // 000
	REGISTER_ENTRY(intrCtrl,        passive,     intrctrl,    32, FBI,      NOSYNC, NOFIFO)    // 004 - cmdFIFO mode
	REGISTER_ENTRY(vertexAx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 008
	REGISTER_ENTRY(vertexAy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 00c
	REGISTER_ENTRY(vertexBx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 010
	REGISTER_ENTRY(vertexBy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 014
	REGISTER_ENTRY(vertexCx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 018
	REGISTER_ENTRY(vertexCy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 01c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(startR,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 020
	REGISTER_ENTRY(startG,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 024
	REGISTER_ENTRY(startB,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 028
	REGISTER_ENTRY(startZ,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 02c
	REGISTER_ENTRY(startA,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 030
	REGISTER_ENTRY(startS,          invalid,     starts,      32, TREX,     NOSYNC,   FIFO)    // 034
	REGISTER_ENTRY(startT,          invalid,     startt,      32, TREX,     NOSYNC,   FIFO)    // 038
	REGISTER_ENTRY(startW,          invalid,     startw,      32, FBI_TREX, NOSYNC,   FIFO)    // 03c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 040
	REGISTER_ENTRY(dGdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 044
	REGISTER_ENTRY(dBdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 048
	REGISTER_ENTRY(dZdX,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 04c
	REGISTER_ENTRY(dAdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 050
	REGISTER_ENTRY(dSdX,            invalid,     dsdx,        32, TREX,     NOSYNC,   FIFO)    // 054
	REGISTER_ENTRY(dTdX,            invalid,     dtdx,        32, TREX,     NOSYNC,   FIFO)    // 058
	REGISTER_ENTRY(dWdX,            invalid,     dwdx,        32, FBI_TREX, NOSYNC,   FIFO)    // 05c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 060
	REGISTER_ENTRY(dGdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 064
	REGISTER_ENTRY(dBdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 068
	REGISTER_ENTRY(dZdY,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 06c
	REGISTER_ENTRY(dAdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 070
	REGISTER_ENTRY(dSdY,            invalid,     dsdy,        32, TREX,     NOSYNC,   FIFO)    // 074
	REGISTER_ENTRY(dTdY,            invalid,     dtdy,        32, TREX,     NOSYNC,   FIFO)    // 078
	REGISTER_ENTRY(dWdY,            invalid,     dwdy,        32, FBI_TREX, NOSYNC,   FIFO)    // 07c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(triangleCMD,     invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 080
	RESERVED_ENTRY                                                                             // 084
	REGISTER_ENTRY(fvertexAx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 088
	REGISTER_ENTRY(fvertexAy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 08c
	REGISTER_ENTRY(fvertexBx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 090
	REGISTER_ENTRY(fvertexBy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 094
	REGISTER_ENTRY(fvertexCx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 098
	REGISTER_ENTRY(fvertexCy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 09c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fstartR,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a0
	REGISTER_ENTRY(fstartG,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a4
	REGISTER_ENTRY(fstartB,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a8
	REGISTER_ENTRY(fstartZ,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ac
	REGISTER_ENTRY(fstartA,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0b0
	REGISTER_ENTRY(fstartS,         invalid,     fstarts,     32, TREX,     NOSYNC,   FIFO)    // 0b4
	REGISTER_ENTRY(fstartT,         invalid,     fstartt,     32, TREX,     NOSYNC,   FIFO)    // 0b8
	REGISTER_ENTRY(fstartW,         invalid,     fstartw,     32, FBI_TREX, NOSYNC,   FIFO)    // 0bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c0
	REGISTER_ENTRY(fdGdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c4
	REGISTER_ENTRY(fdBdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c8
	REGISTER_ENTRY(fdZdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0cc
	REGISTER_ENTRY(fdAdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0d0
	REGISTER_ENTRY(fdSdX,           invalid,     fdsdx,       32, TREX,     NOSYNC,   FIFO)    // 0d4
	REGISTER_ENTRY(fdTdX,           invalid,     fdtdx,       32, TREX,     NOSYNC,   FIFO)    // 0d8
	REGISTER_ENTRY(fdWdX,           invalid,     fdwdx,       32, FBI_TREX, NOSYNC,   FIFO)    // 0dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e0
	REGISTER_ENTRY(fdGdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e4
	REGISTER_ENTRY(fdBdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e8
	REGISTER_ENTRY(fdZdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ec
	REGISTER_ENTRY(fdAdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0f0
	REGISTER_ENTRY(fdSdY,           invalid,     fdsdy,       32, TREX,     NOSYNC,   FIFO)    // 0f4
	REGISTER_ENTRY(fdTdY,           invalid,     fdtdy,       32, TREX,     NOSYNC,   FIFO)    // 0f8
	REGISTER_ENTRY(fdWdY,           invalid,     fdwdy,       32, FBI_TREX, NOSYNC,   FIFO)    // 0fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(ftriangleCMD,    invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 100
	REGISTER_ENTRY(fbzColorPath,    passive,     passive,     30, FBI_TREX, NOSYNC,   FIFO)    // 104
	REGISTER_ENTRY(fogMode,         passive,     passive,      8, FBI_TREX, NOSYNC,   FIFO)    // 108
	REGISTER_ENTRY(alphaMode,       passive,     passive,     32, FBI_TREX, NOSYNC,   FIFO)    // 10c
	REGISTER_ENTRY(fbzMode,         passive,     passive,     22, FBI_TREX,   SYNC,   FIFO)    // 110
	REGISTER_ENTRY(lfbMode,         passive,     passive,     17, FBI_TREX,   SYNC,   FIFO)    // 114
	REGISTER_ENTRY(clipLeftRight,   passive,     passive,     32, FBI_TREX,   SYNC,   FIFO)    // 118
	REGISTER_ENTRY(clipLowYHighY,   passive,     passive,     32, FBI_TREX,   SYNC,   FIFO)    // 11c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nopCMD,          invalid,     nop,          2, FBI_TREX,   SYNC,   FIFO)    // 120
	REGISTER_ENTRY(fastfillCMD,     invalid,     fastfill,     0, FBI,        SYNC,   FIFO)    // 124
	REGISTER_ENTRY(swapbufferCMD,   invalid,     swapbuffer,  10, FBI,        SYNC,   FIFO)    // 128
	REGISTER_ENTRY(fogColor,        invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 12c
	REGISTER_ENTRY(zaColor,         invalid,     passive,     32, FBI,        SYNC,   FIFO)    // 130
	REGISTER_ENTRY(chromaKey,       invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 134
	REGISTER_ENTRY(chromaRange,     invalid,     passive,     28, FBI,        SYNC,   FIFO)    // 138
	REGISTER_ENTRY(userIntrCMD,     invalid,     userintr,    10, FBI,        SYNC,   FIFO)    // 13c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(stipple,         passive,     passive,     32, FBI,        SYNC,   FIFO)    // 140
	REGISTER_ENTRY(color0,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 144
	REGISTER_ENTRY(color1,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 148
	REGISTER_ENTRY(fbiPixelsIn,     stats,       invalid,     24, FBI,          NA,     NA)    // 14c
	REGISTER_ENTRY(fbiChromaFail,   stats,       invalid,     24, FBI,          NA,     NA)    // 150
	REGISTER_ENTRY(fbiZfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 154
	REGISTER_ENTRY(fbiAfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 158
	REGISTER_ENTRY(fbiPixelsOut,    stats,       invalid,     24, FBI,          NA,     NA)    // 15c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[0],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 160
	REGISTER_ENTRY(fogTable[1],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 164
	REGISTER_ENTRY(fogTable[2],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 168
	REGISTER_ENTRY(fogTable[3],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 16c
	REGISTER_ENTRY(fogTable[4],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 170
	REGISTER_ENTRY(fogTable[5],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 174
	REGISTER_ENTRY(fogTable[6],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 178
	REGISTER_ENTRY(fogTable[7],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 17c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[8],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 180
	REGISTER_ENTRY(fogTable[9],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 184
	REGISTER_ENTRY(fogTable[10],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 188
	REGISTER_ENTRY(fogTable[11],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 18c
	REGISTER_ENTRY(fogTable[12],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 190
	REGISTER_ENTRY(fogTable[13],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 194
	REGISTER_ENTRY(fogTable[14],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 198
	REGISTER_ENTRY(fogTable[15],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 19c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[16],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a0
	REGISTER_ENTRY(fogTable[17],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a4
	REGISTER_ENTRY(fogTable[18],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a8
	REGISTER_ENTRY(fogTable[19],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1ac
	REGISTER_ENTRY(fogTable[20],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b0
	REGISTER_ENTRY(fogTable[21],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b4
	REGISTER_ENTRY(fogTable[22],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b8
	REGISTER_ENTRY(fogTable[23],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[24],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c0
	REGISTER_ENTRY(fogTable[25],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c4
	REGISTER_ENTRY(fogTable[26],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c8
	REGISTER_ENTRY(fogTable[27],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1cc
	REGISTER_ENTRY(fogTable[28],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d0
	REGISTER_ENTRY(fogTable[29],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d4
	REGISTER_ENTRY(fogTable[30],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d8
	REGISTER_ENTRY(fogTable[31],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY                                                                             // 1e0
	RESERVED_ENTRY                                                                             // 1e4
	RESERVED_ENTRY                                                                             // 1e8
	REGISTER_ENTRY(colBufferAddr,   passive,     colbufbase,  24, FBI,        SYNC,   FIFO)    // 1ec
	REGISTER_ENTRY(colBufferStride, passive,     colbufstride,16, FBI,        SYNC,   FIFO)    // 1f0
	REGISTER_ENTRY(auxBufferAddr,   passive,     auxbufbase,  24, FBI,        SYNC,   FIFO)    // 1f4
	REGISTER_ENTRY(auxBufferStride, passive,     auxbufstride,16, FBI,        SYNC,   FIFO)    // 1f8
	RESERVED_ENTRY                                                                             // 1fc

	REGISTER_ENTRY(clipLeftRight1,  passive,     passive,     32, FBI,        SYNC,   FIFO)    // 200
	REGISTER_ENTRY(clipTopBottom1,  passive,     passive,     32, FBI,        SYNC,   FIFO)    // 204
	RESERVED_ENTRY                                                                             // 208
	RESERVED_ENTRY                                                                             // 20c
	RESERVED_ENTRY                                                                             // 210
	RESERVED_ENTRY                                                                             // 214
	RESERVED_ENTRY                                                                             // 218
	RESERVED_ENTRY                                                                             // 21c

	RESERVED_ENTRY_x8                                                                          // 220-23c

	RESERVED_ENTRY                                                                             // 240
	RESERVED_ENTRY                                                                             // 244
	RESERVED_ENTRY                                                                             // 248
	REGISTER_ENTRY(swapPending,     invalid,     swappending, 32, FBI,      NOSYNC, NOFIFO)    // 24c
	REGISTER_ENTRY(leftOverlayBuf,  invalid,     overbuffer,  32, FBI,      NOSYNC,   FIFO)    // 250
	REGISTER_ENTRY(rightOverlayBuf, invalid,     overbuffer,  32, FBI,      NOSYNC,   FIFO)    // 254
	REGISTER_ENTRY(fbiSwapHistory,  passive,     invalid,     32, FBI,          NA,     NA)    // 258
	REGISTER_ENTRY(fbiTrianglesOut, passive,     invalid,     24, FBI,          NA,     NA)    // 25c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sSetupMode,      invalid,     passive,     20, FBI,      NOSYNC,   FIFO)    // 260
	REGISTER_ENTRY(sVx,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 264
	REGISTER_ENTRY(sVy,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 268
	REGISTER_ENTRY(sARGB,           invalid,     sargb,       32, FBI,      NOSYNC,   FIFO)    // 26c
	REGISTER_ENTRY(sRed,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 270
	REGISTER_ENTRY(sGreen,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 274
	REGISTER_ENTRY(sBlue,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 278
	REGISTER_ENTRY(sAlpha,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 27c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sVz,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 280
	REGISTER_ENTRY(sWb,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 284
	REGISTER_ENTRY(sWtmu0,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 288
	REGISTER_ENTRY(sS_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 28c
	REGISTER_ENTRY(sT_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 290
	REGISTER_ENTRY(sWtmu1,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 294
	REGISTER_ENTRY(sS_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 298
	REGISTER_ENTRY(sT_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 29c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sDrawTriCMD,     invalid,     draw_tri,    32, FBI,      NOSYNC,   FIFO)    // 2a0
	REGISTER_ENTRY(sBeginTriCMD,    invalid,     begin_tri,   32, FBI,      NOSYNC,   FIFO)    // 2a4
	RESERVED_ENTRY                                                                             // 2a8
	RESERVED_ENTRY                                                                             // 2ac
	RESERVED_ENTRY                                                                             // 2b0
	RESERVED_ENTRY                                                                             // 2b4
	RESERVED_ENTRY                                                                             // 2b8
	RESERVED_ENTRY                                                                             // 2bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 2c0-2dc
	RESERVED_ENTRY_x8                                                                          // 2e0-2fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(textureMode,     invalid,     texture,     31, TREX,     NOSYNC,   FIFO)    // 300
	REGISTER_ENTRY(tLOD,            invalid,     texture,     28, TREX,     NOSYNC,   FIFO)    // 304
	REGISTER_ENTRY(tDetail,         invalid,     texture,     22, TREX,     NOSYNC,   FIFO)    // 308
	REGISTER_ENTRY(texBaseAddr,     invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 30c
	REGISTER_ENTRY(texBaseAddr_1,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 310
	REGISTER_ENTRY(texBaseAddr_2,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 314
	REGISTER_ENTRY(texBaseAddr_3_8, invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 318
	REGISTER_ENTRY(trexInit0,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 31c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(trexInit1,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 320
	REGISTER_ENTRY(nccTable0[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 324
	REGISTER_ENTRY(nccTable0[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 328
	REGISTER_ENTRY(nccTable0[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 32c
	REGISTER_ENTRY(nccTable0[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 330
	REGISTER_ENTRY(nccTable0[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 334
	REGISTER_ENTRY(nccTable0[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 338
	REGISTER_ENTRY(nccTable0[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 33c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable0[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 340
	REGISTER_ENTRY(nccTable0[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 344
	REGISTER_ENTRY(nccTable0[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 348
	REGISTER_ENTRY(nccTable0[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 34c
	REGISTER_ENTRY(nccTable0[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 350
	REGISTER_ENTRY(nccTable1[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 354
	REGISTER_ENTRY(nccTable1[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 358
	REGISTER_ENTRY(nccTable1[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 35c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 360
	REGISTER_ENTRY(nccTable1[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 364
	REGISTER_ENTRY(nccTable1[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 368
	REGISTER_ENTRY(nccTable1[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 36c
	REGISTER_ENTRY(nccTable1[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 370
	REGISTER_ENTRY(nccTable1[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 374
	REGISTER_ENTRY(nccTable1[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 378
	REGISTER_ENTRY(nccTable1[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 37c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 380
	RESERVED_ENTRY                                                                             // 384
	RESERVED_ENTRY                                                                             // 388
	RESERVED_ENTRY                                                                             // 38c
	RESERVED_ENTRY                                                                             // 390
	RESERVED_ENTRY                                                                             // 394
	RESERVED_ENTRY                                                                             // 398
	RESERVED_ENTRY                                                                             // 39c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 3a0-3bc
	RESERVED_ENTRY_x8                                                                          // 3c0-3dc
	RESERVED_ENTRY_x8                                                                          // 3e0-3fc
REGISTER_TABLE_END





/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_banshee_io_reg_name[0x40] =
{
	/* 0x000 */
	"status",       "pciInit0",     "sipMonitor",   "lfbMemoryConfig",
	"miscInit0",    "miscInit1",    "dramInit0",    "dramInit1",
	"agpInit",      "tmuGbeInit",   "vgaInit0",     "vgaInit1",
	"dramCommand",  "dramData",     "reserved38",   "reserved3c",

	/* 0x040 */
	"pllCtrl0",     "pllCtrl1",     "pllCtrl2",     "dacMode",
	"dacAddr",      "dacData",      "rgbMaxDelta",  "vidProcCfg",
	"hwCurPatAddr", "hwCurLoc",     "hwCurC0",      "hwCurC1",
	"vidInFormat",  "vidInStatus",  "vidSerialParallelPort","vidInXDecimDeltas",

	/* 0x080 */
	"vidInDecimInitErrs","vidInYDecimDeltas","vidPixelBufThold","vidChromaMin",
	"vidChromaMax", "vidCurrentLine","vidScreenSize","vidOverlayStartCoords",
	"vidOverlayEndScreenCoord","vidOverlayDudx","vidOverlayDudxOffsetSrcWidth","vidOverlayDvdy",
	"vga[b0]",      "vga[b4]",      "vga[b8]",      "vga[bc]",

	/* 0x0c0 */
	"vga[c0]",      "vga[c4]",      "vga[c8]",      "vga[cc]",
	"vga[d0]",      "vga[d4]",      "vga[d8]",      "vga[dc]",
	"vidOverlayDvdyOffset","vidDesktopStartAddr","vidDesktopOverlayStride","vidInAddr0",
	"vidInAddr1",   "vidInAddr2",   "vidInStride",  "vidCurrOverlayStartAddr"
};


/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_banshee_agp_reg_name[0x50] =
{
	/* 0x000 */
	"agpReqSize",   "agpHostAddressLow","agpHostAddressHigh","agpGraphicsAddress",
	"agpGraphicsStride","agpMoveCMD","reserved18",  "reserved1c",
	"cmdBaseAddr0", "cmdBaseSize0", "cmdBump0",     "cmdRdPtrL0",
	"cmdRdPtrH0",   "cmdAMin0",     "reserved38",   "cmdAMax0",

	/* 0x040 */
	"reserved40",   "cmdFifoDepth0","cmdHoleCnt0",  "reserved4c",
	"cmdBaseAddr1", "cmdBaseSize1", "cmdBump1",     "cmdRdPtrL1",
	"cmdRdPtrH1",   "cmdAMin1",     "reserved68",   "cmdAMax1",
	"reserved70",   "cmdFifoDepth1","cmdHoleCnt1",  "reserved7c",

	/* 0x080 */
	"cmdFifoThresh","cmdHoleInt",   "reserved88",   "reserved8c",
	"reserved90",   "reserved94",   "reserved98",   "reserved9c",
	"reserveda0",   "reserveda4",   "reserveda8",   "reservedac",
	"reservedb0",   "reservedb4",   "reservedb8",   "reservedbc",

	/* 0x0c0 */
	"reservedc0",   "reservedc4",   "reservedc8",   "reservedcc",
	"reservedd0",   "reservedd4",   "reservedd8",   "reserveddc",
	"reservede0",   "reservede4",   "reservede8",   "reservedec",
	"reservedf0",   "reservedf4",   "reservedf8",   "reservedfc",

	/* 0x100 */
	"yuvBaseAddress","yuvStride",   "reserved108",  "reserved10c",
	"reserved110",  "reserved114",  "reserved118",  "reserved11c",
	"crc1",         "reserved124",  "reserved128",  "reserved12c",
	"crc2",         "reserved134",  "reserved138",  "reserved13c"
};
