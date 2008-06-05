/***************************************************************************

    ppcfe.c

    Front-end for PowerPC recompiler

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stddef.h>
#include "cpuintrf.h"
#include "ppcfe.h"
#include "ppccom.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int describe_instruction_13(powerpc_state *ppc, UINT32 op, opcode_desc *desc);
static int describe_instruction_1f(powerpc_state *ppc, UINT32 op, opcode_desc *desc);
static int describe_instruction_3b(powerpc_state *ppc, UINT32 op, opcode_desc *desc);
static int describe_instruction_3f(powerpc_state *ppc, UINT32 op, opcode_desc *desc);



/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

/*-------------------------------------------------
    ppcfe_describe - build a description
    of a single instruction
-------------------------------------------------*/

int ppcfe_describe(void *param, opcode_desc *desc)
{
	powerpc_state *ppc = param;
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = op >> 26;

	/* all instructions are 4 bytes and default to a single cycle each */
	desc->length = 4;
	desc->cycles = 1;

	/* parse the instruction */
	switch (opswitch)
	{
		case 0x02:	/* TDI - 64-bit only */
		case 0x1e:	/* 0x1e group - 64-bit only */
		case 0x3a:	/* 0x3a group - 64-bit only */
		case 0x3e:	/* 0x3e group - 64-bit only */
			return FALSE;
		
		case 0x03:	/* TWI */
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;
	
		case 0x07:	/* MULLI */
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			return TRUE;

		case 0x0e:	/* ADDI */
		case 0x0f:	/* ADDIS */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			return TRUE;
	
		case 0x0a:	/* CMPLI */
		case 0x0b:	/* CMPI */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_XER;
			desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
			return TRUE;
	
		case 0x08:	/* SUBFIC */
		case 0x0c:	/* ADDIC */
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op)) | REGFLAG_XER;
			return TRUE;
	
		case 0x0d:	/* ADDIC. */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_XER;
			desc->gpr.modified |= REGFLAG_R(G_RT(op)) | REGFLAG_CR(0) | REGFLAG_XER;
			return TRUE;
	
		case 0x10:	/* BCx */
			desc->gpr.used |= REGFLAG_CR(G_BI(op) / 4);
			if (op & M_LK) desc->gpr.modified |= REGFLAG_LR;
			if (!(G_BO(op) & 4))
			{
				desc->gpr.used |= REGFLAG_CTR;
				desc->gpr.modified |= REGFLAG_CTR;
			}
			if ((G_BO(op) & 0x14) == 0x14)
				desc->flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc->flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc->targetpc = (INT16)(G_BD(op) << 2) + ((op & M_AA) ? 0 : desc->pc);
			return TRUE;
	
		case 0x11:	/* SC */
			if (!(ppc->cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return FALSE;
			desc->flags |= OPFLAG_WILL_CAUSE_EXCEPTION;
			return TRUE;
	
		case 0x12:	/* Bx */
			if (op & M_LK) desc->gpr.modified |= REGFLAG_LR;
			desc->flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc->targetpc = ((INT32)(G_LI(op) << 8) >> 6) + ((op & M_AA) ? 0 : desc->pc);
			return TRUE;
		
		case 0x13:	/* 0x13 group */
			return describe_instruction_13(ppc, op, desc);
			
		case 0x14:	/* RLWIMIx */
			desc->gpr.used |= REGFLAG_R(G_RS(op)) | REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;
		
		case 0x15:	/* RLWINMx */
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;
		
		case 0x17:	/* RLWNMx */
			desc->gpr.used |= REGFLAG_R(G_RS(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;
		
		case 0x18:	/* ORI */
		case 0x19:	/* ORIS */
		case 0x1a:	/* XORI */
		case 0x1b:	/* XORIS */
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			return TRUE;
	
		case 0x1c:	/* ANDI. */
		case 0x1d:	/* ANDIS. */
			desc->gpr.used |= REGFLAG_R(G_RS(op)) | REGFLAG_XER;
			desc->gpr.modified |= REGFLAG_R(G_RA(op)) | REGFLAG_CR(0);
			return TRUE;
		
		case 0x1f:	/* 0x1f group */
			return describe_instruction_1f(ppc, op, desc);
		
		case 0x20:	/* LWZ */
		case 0x22:	/* LBZ */
		case 0x28:	/* LHZ */
		case 0x2a:	/* LHA */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;
		
		case 0x21:	/* LWZU */
		case 0x23:	/* LBZU */
		case 0x29:	/* LHZU */
		case 0x2b:	/* LHAU */
			if (G_RA(op) == 0)
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op)) | REGFLAG_R(G_RA(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;
		
		case 0x24:	/* STW */
		case 0x26:	/* STB */
		case 0x2c:	/* STH */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x25:	/* STWU */
		case 0x27:	/* STBU */
		case 0x2d:	/* STHU */
			if (G_RA(op) == 0)
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x2e:	/* LMW */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->gpr.modified |= ((REGFLAG_R(31) << 1) - 1) & ~(REGFLAG_R(G_RD(op)) - 1);
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;
		
		case 0x2f:	/* STMW */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | (((REGFLAG_R(31) << 1) - 1) & ~(REGFLAG_R(G_RS(op)) - 1));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x30:	/* LFS */
		case 0x32:	/* LFD */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;
		
		case 0x31:	/* LFSU */
		case 0x33:	/* LFDU */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;
		
		case 0x34:	/* STFS */
		case 0x36:	/* STFD */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->fpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x35:	/* STFSU */
		case 0x37:	/* STFDU */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			desc->fpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x3b:	/* 0x3b group */
			return describe_instruction_3b(ppc, op, desc);
		
		case 0x3f:	/* 0x3f group */
			return describe_instruction_3f(ppc, op, desc);
	}

	return FALSE;
}


/*-------------------------------------------------
    describe_instruction_13 - build a
    description of a single instruction in the
    0x13 group
-------------------------------------------------*/

static int describe_instruction_13(powerpc_state *ppc, UINT32 op, opcode_desc *desc)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;
	
	switch (opswitch)
	{
		case 0x000:	/* MTCRF */
			desc->gpr.used |= REGFLAG_CR(G_CRFS(op));
			desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
			return TRUE;
			
		case 0x010:	/* BCLRx */
			desc->gpr.used |= REGFLAG_CR(G_BI(op) / 4) | REGFLAG_LR;
			if (op & M_LK) desc->gpr.modified |= REGFLAG_LR;
			if (!(G_BO(op) & 4))
			{
				desc->gpr.used |= REGFLAG_CTR;
				desc->gpr.modified |= REGFLAG_CTR;
			}
			if ((G_BO(op) & 0x14) == 0x14)
				desc->flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc->flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc->targetpc = BRANCH_TARGET_DYNAMIC;
			return TRUE;

		case 0x021:	/* CRNOR */
		case 0x081:	/* CRANDC */
		case 0x0c1:	/* CRXOR */
		case 0x0e1:	/* CRNAND */
		case 0x101:	/* CRAND */
		case 0x121:	/* CREQV */
		case 0x1a1:	/* CRORC */
		case 0x1c1:	/* CROR */
			desc->gpr.used |= REGFLAG_CR(G_CRBA(op) / 4) | REGFLAG_CR(G_CRBB(op) / 4);
			desc->gpr.modified |= REGFLAG_CR(G_CRBD(op) / 4);
			return TRUE;
			
		case 0x032:	/* RFI */
			if (!(ppc->cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return FALSE;
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CHANGE_MODES | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			desc->targetpc = BRANCH_TARGET_DYNAMIC;
			return TRUE;
			
		case 0x033:	/* RFCI */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CHANGE_MODES | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			desc->targetpc = BRANCH_TARGET_DYNAMIC;
			return TRUE;
			
		case 0x096:	/* ISYNC */
			if (!(ppc->cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return FALSE;
			return TRUE;
			
		case 0x210:	/* BCCTRx */
			desc->gpr.used |= REGFLAG_CR(G_BI(op) / 4) | REGFLAG_CTR;
			if (op & M_LK) desc->gpr.modified |= REGFLAG_LR;
			if ((G_BO(op) & 0x14) == 0x14)
				desc->flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc->flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc->targetpc = BRANCH_TARGET_DYNAMIC;
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    describe_instruction_1f - build a
    description of a single instruction in the
    0x1f group
-------------------------------------------------*/

static int describe_instruction_1f(powerpc_state *ppc, UINT32 op, opcode_desc *desc)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;
	
	switch (opswitch)
	{
		case 0x009:	/* MULHDUx - 64-bit only */
		case 0x015:	/* LDX - 64-bit only */
		case 0x01b:	/* SLDx - 64-bit only */
		case 0x035:	/* LDUX - 64-bit only */
		case 0x03a:	/* CNTLZDx - 64-bit only */
		case 0x044:	/* TD - 64-bit only */
		case 0x049:	/* MULHDx - 64-bit only */
		case 0x054:	/* LDARX - 64-bit only */
		case 0x095:	/* STDX - 64-bit only */
		case 0x0b5:	/* STDUX - 64-bit only */
		case 0x0d6:	/* STDCX. - 64-bit only */
		case 0x0e9:	/* MULLD - 64-bit only */
		case 0x2e9:	/* MULLDO - 64-bit only */
		case 0x155:	/* LWAX - 64-bit only */
		case 0x175:	/* LWAUX - 64-bit only */
		case 0x33a: /* SRADIx - 64-bit only */
		case 0x33b: /* SRADIx - 64-bit only */
		case 0x1b2:	/* SLBIE - 64-bit only */
		case 0x1c9:	/* DIVDUx - 64-bit only */
		case 0x3c9:	/* DIVDUOx - 64-bit only */
		case 0x1e9:	/* DIVDx - 64-bit only */
		case 0x3e9:	/* DIVDOx - 64-bit only */
		case 0x1f2:	/* SLBIA - 64-bit only */
		case 0x21b:	/* SRDx - 64-bit only */
		case 0x31a:	/* SRADx - 64-bit only */
		case 0x3da:	/* EXTSW - 64-bit only */
			return FALSE;

		case 0x000:	/* CMP */
		case 0x020:	/* CMPL */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_XER;
			desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
			return TRUE;

		case 0x004:	/* TW */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x008:	/* SUBFCx */
		case 0x00a:	/* ADDCx */
		case 0x00b:	/* MULHWUx */
		case 0x028:	/* SUBFx */
		case 0x04b:	/* MULHWx */
		case 0x088:	/* SUBFEx */
		case 0x08a:	/* ADDEx */
		case 0x0c8:	/* SUBFZEx */
		case 0x0ca:	/* ADDZEx */
		case 0x0e8:	/* SUBFMEx */
		case 0x0ea:	/* ADDMEx */
		case 0x0eb:	/* MULLWx */
		case 0x10a:	/* ADDx */
		case 0x1cb:	/* DIVWUx */
		case 0x1eb:	/* DIVWx */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;

		case 0x208:	/* SUBFCOx */
		case 0x20a:	/* ADDCOx */
		case 0x228:	/* SUBFOx */
		case 0x288:	/* SUBFEOx */
		case 0x28a:	/* ADDEOx */
		case 0x2c8:	/* SUBFZEOx */
		case 0x2ca:	/* ADDZEOx */
		case 0x2e8:	/* SUBFMEOx */
		case 0x2ea:	/* ADDMEOx */
		case 0x2eb:	/* MULLWOx */
		case 0x30a:	/* ADDOx */
		case 0x3cb:	/* DIVWUOx */
		case 0x3eb:	/* DIVWOx */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op)) | REGFLAG_XER;
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;

		case 0x013:	/* MFCR */
			desc->gpr.used |= ((REGFLAG_CR(7) << 1) - 1) & ~(REGFLAG_CR(0) - 1);
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			return TRUE;
			
		case 0x136:	/* ECIWX */
			if (!(ppc->cap & PPCCAP_VEA))
				return FALSE;
		case 0x014:	/* LWARX */
		case 0x017:	/* LWZX */
		case 0x057:	/* LBZX */
		case 0x117:	/* LHZX */
		case 0x157:	/* LHAX */
		case 0x216:	/* LWBRX */
		case 0x316:	/* LHBRX */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x018:	/* SLWx */
		case 0x01c:	/* ANDx */
		case 0x03c:	/* ANDCx */
		case 0x07c:	/* NORx */
		case 0x11c:	/* EQVx */
		case 0x13c:	/* XORx */
		case 0x19c:	/* ORCx */
		case 0x1bc:	/* ORx */
		case 0x1dc:	/* NANDx */
		case 0x218:	/* SRWx */
		case 0x318:	/* SRAWx */
			desc->gpr.used |= REGFLAG_RZ(G_RS(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			return TRUE;

		case 0x01a:	/* CNTLZWx */
		case 0x39a:	/* EXTSHx */
		case 0x3ba:	/* EXTSBx */
			desc->gpr.used |= REGFLAG_RZ(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			return TRUE;

		case 0x036:	/* DCBST */
		case 0x056:	/* DCBF */
		case 0x0f6:	/* DCBTST */
		case 0x116:	/* DCBT */
		case 0x2f6:	/* DCBA */
		case 0x3d6:	/* ICBI */
			if (!(ppc->cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			return TRUE;

		case 0x1d6:	/* DCBI */
			if (!(ppc->cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x037:	/* LWZUX */
		case 0x077:	/* LBZUX */
		case 0x137:	/* LHZUX */
		case 0x177:	/* LHAUX */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op)) | REGFLAG_R(G_RA(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x153:	/* MFSPR */
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			if (G_SPR(op) & 0x200)
				desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x053:	/* MFMSR */
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return TRUE;

		case 0x253:	/* MFSR */
		case 0x293:	/* MFSRIN */
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x173:	/* MFTB */
			if (!(ppc->cap & PPCCAP_VEA))
				return FALSE;
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			return TRUE;
			
		case 0x068:	/* NEGx */
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;

		case 0x268:	/* NEGOx */
			desc->gpr.used |= REGFLAG_R(G_RA(op));
			desc->gpr.modified |= REGFLAG_R(G_RD(op)) | REGFLAG_XER;
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(0);
			}
			return TRUE;

		case 0x090:	/* MTCRF */
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			if (G_CRM(op) & 0x01) desc->gpr.modified |= REGFLAG_CR(0);
			if (G_CRM(op) & 0x02) desc->gpr.modified |= REGFLAG_CR(1);
			if (G_CRM(op) & 0x04) desc->gpr.modified |= REGFLAG_CR(2);
			if (G_CRM(op) & 0x08) desc->gpr.modified |= REGFLAG_CR(3);
			if (G_CRM(op) & 0x10) desc->gpr.modified |= REGFLAG_CR(4);
			if (G_CRM(op) & 0x20) desc->gpr.modified |= REGFLAG_CR(5);
			if (G_CRM(op) & 0x40) desc->gpr.modified |= REGFLAG_CR(6);
			if (G_CRM(op) & 0x80) desc->gpr.modified |= REGFLAG_CR(7);
			return TRUE;
			
		case 0x092:	/* MTMSR */
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_CHANGE_MODES | OPFLAG_END_SEQUENCE;
			return TRUE;

		case 0x0d2:	/* MTSR */
			if (!(ppc->cap & PPCCAP_OEA))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x1d3:	/* MTSPR */
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			if (G_SPR(op) & 0x200)
				desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x1b6:	/* ECOWX */
			if (!(ppc->cap & PPCCAP_VEA))
				return FALSE;
		case 0x096:	/* STWCX. */
		case 0x097:	/* STWX */
		case 0x0d7:	/* STBX */
		case 0x197:	/* STHX */
		case 0x296:	/* STWBRX */
		case 0x396:	/* STHBRX */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;

		case 0x0b7:	/* STWUX */
		case 0x0f7:	/* STBUX */
		case 0x1b7:	/* STHUX */
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_R(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;

		case 0x0f2:	/* MTSRIN */
			if (!(ppc->cap & PPCCAP_OEA))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RS(op)) | REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x132:	/* TLBIE */
			if (!(ppc->cap & PPCCAP_OEA))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x172:	/* TLBIA */
			if (!(ppc->cap & PPCCAP_OEA) || (ppc->cap & PPCCAP_603_MMU))
				return FALSE;
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;
		
		case 0x3d2:	/* TLBLD */
		case 0x3f2:	/* TLBLI */
			if (!(ppc->cap & PPCCAP_603_MMU))
				return FALSE;
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x200:	/* MCRXR */
			desc->gpr.used |= REGFLAG_XER;
			desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
			return TRUE;

		case 0x215:	/* LSWX */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_XER;
			desc->gpr.modified |= ((REGFLAG_R(31) << 1) - 1) & ~(REGFLAG_R(0) - 1);
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x217:	/* LFSX */
		case 0x257:	/* LFDX */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x236:	/* TLBSYNC */
			if (!(ppc->cap & PPCCAP_OEA))
				return FALSE;
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;
			
		case 0x256:	/* SYNC */
			return TRUE;

		case 0x356:	/* EIEIO */
			if (!(ppc->cap & PPCCAP_VEA))
				return FALSE;
			return TRUE;
			
		case 0x237:	/* LFSUX */
		case 0x277:	/* LFDUX */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x255:	/* LSWI */
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= ((REGFLAG_R(31) << 1) - 1) & ~(REGFLAG_R(0) - 1);
			desc->flags |= OPFLAG_READS_MEMORY;
			return TRUE;

		case 0x295:	/* STSWX */
		case 0x2d5:	/* STSWI */
			desc->gpr.used |= (((REGFLAG_R(31) << 1) - 1) & ~(REGFLAG_R(0) - 1)) | REGFLAG_XER;
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;

		case 0x297:	/* STFSX */
		case 0x2d7:	/* STFDX */
		case 0x3d7:	/* STFIWX */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->fpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;

		case 0x2b7:	/* STFSUX */
		case 0x2f7:	/* STFDUX */
			if (!(ppc->cap & PPCCAP_FPU))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_RZ(G_RA(op));
			desc->fpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;

		case 0x338:	/* SRAWIx */
			desc->gpr.used |= REGFLAG_RZ(G_RS(op));
			desc->gpr.modified |= REGFLAG_R(G_RA(op));
			return TRUE;

		case 0x3f6:	/* DCBZ */
			if (!(ppc->cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_WRITES_MEMORY;
			return TRUE;
		
		case 0x106:	/* ICBT */
		case 0x1c6:	/* DCCCI */
		case 0x3c6:	/* ICCCI */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x1e6:	/* DCREAD */
		case 0x3e6:	/* ICREAD */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->gpr.used |= REGFLAG_RZ(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->gpr.modified |= REGFLAG_R(G_RT(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x143:	/* MFDCR */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->gpr.modified |= REGFLAG_R(G_RD(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return TRUE;

		case 0x1c3:	/* MTDCR */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return TRUE;
		
		case 0x083:	/* WRTEE */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			desc->gpr.used |= REGFLAG_R(G_RS(op));
			desc->flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return TRUE;
		
		case 0x0a3:	/* WRTEEI */
			if (!(ppc->cap & PPCCAP_4XX))
				return FALSE;
			if (op & MSR_EE)
				desc->flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    describe_instruction_3b - build a
    description of a single instruction in the
    0x3b group
-------------------------------------------------*/

static int describe_instruction_3b(powerpc_state *ppc, UINT32 op, opcode_desc *desc)
{
	UINT32 opswitch = (op >> 1) & 0x1f;

	if (!(ppc->cap & PPCCAP_FPU))
		return FALSE;
	
	switch (opswitch)
	{
		case 0x12:	/* FDIVSx */
		case 0x14:	/* FSUBSx */
		case 0x15:	/* FADDSx */
		case 0x19:	/* FMULSx */
			desc->fpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(1);
			}
			return TRUE;

		case 0x16:	/* FSQRTSx */
		case 0x18:	/* FRESx */
			desc->fpr.used |= REGFLAG_R(G_RB(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(1);
			}
			return TRUE;

		case 0x1c:	/* FMSUBSx */
		case 0x1d:	/* FMADDSx */
		case 0x1e:	/* FNMSUBSx */
		case 0x1f:	/* FNMADDSx */
			desc->fpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_R(G_REGC(op));
			desc->fpr.modified |= REGFLAG_R(G_RD(op));
			if (op & M_RC)
			{
				desc->gpr.used |= REGFLAG_XER;
				desc->gpr.modified |= REGFLAG_CR(1);
			}
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    describe_instruction_3f - build a
    description of a single instruction in the
    0x3f group
-------------------------------------------------*/

static int describe_instruction_3f(powerpc_state *ppc, UINT32 op, opcode_desc *desc)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;
	
	if (!(ppc->cap & PPCCAP_FPU))
		return FALSE;

	if (opswitch & 0x10)
	{
		opswitch &= 0x1f;
		switch (opswitch)
		{
			case 0x12:	/* FDIVx */
			case 0x14:	/* FSUBx */
			case 0x15:	/* FADDx */
			case 0x19:	/* FMULx */
				desc->fpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
				desc->fpr.modified |= REGFLAG_R(G_RD(op));
				if (op & M_RC)
				{
					desc->gpr.used |= REGFLAG_XER;
					desc->gpr.modified |= REGFLAG_CR(1);
				}
				return TRUE;

			case 0x16:	/* FSQRTx */
			case 0x1a:	/* FSQRTEx */
				desc->fpr.used |= REGFLAG_R(G_RB(op));
				desc->fpr.modified |= REGFLAG_R(G_RD(op));
				if (op & M_RC)
				{
					desc->gpr.used |= REGFLAG_XER;
					desc->gpr.modified |= REGFLAG_CR(1);
				}
				return TRUE;

			case 0x17:	/* FSELx */
			case 0x1c:	/* FMSUBx */
			case 0x1d:	/* FMADDx */
			case 0x1e:	/* FNMSUBx */
			case 0x1f:	/* FNMADDx */
				desc->fpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op)) | REGFLAG_R(G_REGC(op));
				desc->fpr.modified |= REGFLAG_R(G_RD(op));
				if (op & M_RC)
				{
					desc->gpr.used |= REGFLAG_XER;
					desc->gpr.modified |= REGFLAG_CR(1);
				}
				return TRUE;
		}
	}
	else
	{
		switch (opswitch)
		{
			case 0x32e:	/* FCTIDx - 64-bit only */
			case 0x32f:	/* FCTIDZx - 64-bit only */
			case 0x34e:	/* FCFIDx - 64-bit only */
				return FALSE;

			case 0x000:	/* FCMPU */
			case 0x020:	/* FCMPO */
				desc->gpr.used |= REGFLAG_XER;
				desc->fpr.used |= REGFLAG_R(G_RA(op)) | REGFLAG_R(G_RB(op));
				desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
				return TRUE;

			case 0x00c:	/* FRSPx */
			case 0x00e:	/* FCTIWx */
			case 0x00f:	/* FCTIWZx */
			case 0x028:	/* FNEGx */
			case 0x048:	/* FMRx */
			case 0x088:	/* FNABSx */
			case 0x108:	/* FABSx */
				desc->fpr.used |= REGFLAG_R(G_RB(op));
				desc->fpr.modified |= REGFLAG_R(G_RD(op));
				if (op & M_RC)
				{
					desc->gpr.used |= REGFLAG_XER;
					desc->gpr.modified |= REGFLAG_CR(1);
				}
				return TRUE;

			case 0x026:	/* MTFSB1x */
			case 0x046:	/* MTFSB0x */
				desc->fpr.modified |= REGFLAG_FPSCR;
				return TRUE;
			
			case 0x040:	/* MCRFS */
				desc->fpr.used |= REGFLAG_FPSCR;
				desc->gpr.modified |= REGFLAG_CR(G_CRFD(op));
				return TRUE;

			case 0x086:	/* MTFSFIx */
				desc->fpr.modified |= REGFLAG_FPSCR;
				return TRUE;

			case 0x247:	/* MFFSx */
				desc->fpr.used |= REGFLAG_FPSCR;
				desc->fpr.modified |= REGFLAG_R(G_RD(op));
				return TRUE;

			case 0x2c7:	/* MTFSFx */
				desc->fpr.used |= REGFLAG_R(G_RB(op));
				desc->fpr.modified |= REGFLAG_FPSCR;
				return TRUE;
		}
	}

	return FALSE;
}
