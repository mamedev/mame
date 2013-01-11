/***************************************************************************

    am29000.h
    Interface file for the portable AMD Am29000 emulator.
    Written by Phil Bennett

***************************************************************************/

#pragma once

#ifndef __AM29000_H__
#define __AM29000_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	AM29000_PC = 1,
	AM29000_VAB,
	AM29000_OPS,
	AM29000_CPS,
	AM29000_CFG,
	AM29000_CHA,
	AM29000_CHD,
	AM29000_CHC,
	AM29000_RBP,
	AM29000_TMC,
	AM29000_TMR,
	AM29000_PC0,
	AM29000_PC1,
	AM29000_PC2,
	AM29000_MMU,
	AM29000_LRU,
	AM29000_IPC,
	AM29000_IPA,
	AM29000_IPB,
	AM29000_Q,
	AM29000_ALU,
	AM29000_BP,
	AM29000_FC,
	AM29000_CR,
	AM29000_FPE,
	AM29000_INTE,
	AM29000_FPS,
	AM29000_R1,
	AM29000_R64,
	AM29000_R65,
	AM29000_R66,
	AM29000_R67,
	AM29000_R68,
	AM29000_R69,
	AM29000_R70,
	AM29000_R71,
	AM29000_R72,
	AM29000_R73,
	AM29000_R74,
	AM29000_R75,
	AM29000_R76,
	AM29000_R77,
	AM29000_R78,
	AM29000_R79,
	AM29000_R80,
	AM29000_R81,
	AM29000_R82,
	AM29000_R83,
	AM29000_R84,
	AM29000_R85,
	AM29000_R86,
	AM29000_R87,
	AM29000_R88,
	AM29000_R89,
	AM29000_R90,
	AM29000_R91,
	AM29000_R92,
	AM29000_R93,
	AM29000_R94,
	AM29000_R95,
	AM29000_R96,
	AM29000_R97,
	AM29000_R98,
	AM29000_R99,
	AM29000_R100,
	AM29000_R101,
	AM29000_R102,
	AM29000_R103,
	AM29000_R104,
	AM29000_R105,
	AM29000_R106,
	AM29000_R107,
	AM29000_R108,
	AM29000_R109,
	AM29000_R110,
	AM29000_R111,
	AM29000_R112,
	AM29000_R113,
	AM29000_R114,
	AM29000_R115,
	AM29000_R116,
	AM29000_R117,
	AM29000_R118,
	AM29000_R119,
	AM29000_R120,
	AM29000_R121,
	AM29000_R122,
	AM29000_R123,
	AM29000_R124,
	AM29000_R125,
	AM29000_R126,
	AM29000_R127,
	AM29000_R128,
	AM29000_R129,
	AM29000_R130,
	AM29000_R131,
	AM29000_R132,
	AM29000_R133,
	AM29000_R134,
	AM29000_R135,
	AM29000_R136,
	AM29000_R137,
	AM29000_R138,
	AM29000_R139,
	AM29000_R140,
	AM29000_R141,
	AM29000_R142,
	AM29000_R143,
	AM29000_R144,
	AM29000_R145,
	AM29000_R146,
	AM29000_R147,
	AM29000_R148,
	AM29000_R149,
	AM29000_R150,
	AM29000_R151,
	AM29000_R152,
	AM29000_R153,
	AM29000_R154,
	AM29000_R155,
	AM29000_R156,
	AM29000_R157,
	AM29000_R158,
	AM29000_R159,
	AM29000_R160,
	AM29000_R161,
	AM29000_R162,
	AM29000_R163,
	AM29000_R164,
	AM29000_R165,
	AM29000_R166,
	AM29000_R167,
	AM29000_R168,
	AM29000_R169,
	AM29000_R170,
	AM29000_R171,
	AM29000_R172,
	AM29000_R173,
	AM29000_R174,
	AM29000_R175,
	AM29000_R176,
	AM29000_R177,
	AM29000_R178,
	AM29000_R179,
	AM29000_R180,
	AM29000_R181,
	AM29000_R182,
	AM29000_R183,
	AM29000_R184,
	AM29000_R185,
	AM29000_R186,
	AM29000_R187,
	AM29000_R188,
	AM29000_R189,
	AM29000_R190,
	AM29000_R191,
	AM29000_R192,
	AM29000_R193,
	AM29000_R194,
	AM29000_R195,
	AM29000_R196,
	AM29000_R197,
	AM29000_R198,
	AM29000_R199,
	AM29000_R200,
	AM29000_R201,
	AM29000_R202,
	AM29000_R203,
	AM29000_R204,
	AM29000_R205,
	AM29000_R206,
	AM29000_R207,
	AM29000_R208,
	AM29000_R209,
	AM29000_R210,
	AM29000_R211,
	AM29000_R212,
	AM29000_R213,
	AM29000_R214,
	AM29000_R215,
	AM29000_R216,
	AM29000_R217,
	AM29000_R218,
	AM29000_R219,
	AM29000_R220,
	AM29000_R221,
	AM29000_R222,
	AM29000_R223,
	AM29000_R224,
	AM29000_R225,
	AM29000_R226,
	AM29000_R227,
	AM29000_R228,
	AM29000_R229,
	AM29000_R230,
	AM29000_R231,
	AM29000_R232,
	AM29000_R233,
	AM29000_R234,
	AM29000_R235,
	AM29000_R236,
	AM29000_R237,
	AM29000_R238,
	AM29000_R239,
	AM29000_R240,
	AM29000_R241,
	AM29000_R242,
	AM29000_R243,
	AM29000_R244,
	AM29000_R245,
	AM29000_R246,
	AM29000_R247,
	AM29000_R248,
	AM29000_R249,
	AM29000_R250,
	AM29000_R251,
	AM29000_R252,
	AM29000_R253,
	AM29000_R254,
	AM29000_R255
};


/***************************************************************************
    SPECIAL PURPOSE REGISTER INDICES
***************************************************************************/

enum
{   SPR_VAB  = 0,
	SPR_OPS  = 1,
	SPR_CPS  = 2,
	SPR_CFG  = 3,
	SPR_CHA  = 4,
	SPR_CHD  = 5,
	SPR_CHC  = 6,
	SPR_RBP  = 7,
	SPR_TMC  = 8,
	SPR_TMR  = 9,
	SPR_PC0  = 10,
	SPR_PC1  = 11,
	SPR_PC2  = 12,
	SPR_MMU  = 13,
	SPR_LRU  = 14,
	SPR_IPC  = 128,
	SPR_IPA  = 129,
	SPR_IPB  = 130,
	SPR_Q    = 131,
	SPR_ALU  = 132,
	SPR_BP   = 133,
	SPR_FC   = 134,
	SPR_CR   = 135,
	SPR_FPE  = 160,
	SPR_INTE = 161,
	SPR_FPS  = 162,
};


enum
{
	SPACE_INSTRUCTION = 0,
	SPACE_DATA,
	SPACE_IO,
	SPACE_COPROCESSOR,
};


/***************************************************************************
    EXCEPTION VECTORS
***************************************************************************/

enum
{
	EXCEPTION_ILLEGAL_OPCODE            = 0,
	EXCEPTION_UNALIGNED_ACCESS          = 1,
	EXCEPTION_OUT_OF_RANGE              = 2,
	EXCEPTION_COPRO_NOT_PRESENT         = 3,
	EXCEPTION_COPRO_EXCEPTION           = 4,
	EXCEPTION_PROTECTION_VIOLATION      = 5,
	EXCEPTION_INST_ACCESS_VIOLATION     = 6,
	EXCEPTION_DATA_ACCESS_VIOLATION     = 7,
	EXCEPTION_USER_INST_TLB_MISS        = 8,
	EXCEPTION_USER_DATA_TLB_MISS        = 9,
	EXCEPTION_SUPER_INST_TLB_MISS       = 10,
	EXCEPTION_SUPER_DATA_TLB_MISS       = 11,
	EXCEPTION_INST_TLB_PROT_VIOLATION   = 12,
	EXCEPTION_DATA_TLB_PROT_VIOLATION   = 13,
	EXCEPTION_TIMER                     = 14,
	EXCEPTION_TRACE                     = 15,
	EXCEPTION_INTR0                     = 16,
	EXCEPTION_INTR1                     = 17,
	EXCEPTION_INTR2                     = 18,
	EXCEPTION_INTR3                     = 19,
	EXCEPTION_TRAP0                     = 20,
	EXCEPTION_TRAP1                     = 21,
	EXCEPTION_FLOATING_POINT_EXCEPTION  = 22,
	EXCEPTION_MULTM                     = 30,
	EXCEPTION_MULTMU                    = 31,
	EXCEPTION_MULTIPLY                  = 32,
	EXCEPTION_DIVIDE                    = 33,
	EXCEPTION_MULTIPLU                  = 44,      // TODO: FINISH ME
};


/***************************************************************************
    SPECIAL PURPOSE REGISTER FIELDS
***************************************************************************/

#define PRL_AM29000             (0 << 3)
#define PRL_AM29050             (1 << 3)
#define PRL_REV_A               (0)
#define PRL_REV_B               (1)
#define PRL_REV_C               (2)
#define PRL_REV_D               (3)

#define PROCESSOR_REL_FIELD     (PRL_AM29000 | PRL_REV_D)

#define VAB_SHIFT               (16)
#define VAB_MASK                (0xffff)

#define CPS_CA                  (1 << 15)
#define CPS_IP                  (1 << 14)
#define CPS_TE                  (1 << 13)
#define CPS_TP                  (1 << 12)
#define CPS_TU                  (1 << 11)
#define CPS_FZ                  (1 << 10)
#define CPS_LK                  (1 << 9)
#define CPS_RE                  (1 << 8)
#define CPS_WM                  (1 << 7)
#define CPS_PD                  (1 << 6)
#define CPS_PI                  (1 << 5)
#define CPS_SM                  (1 << 4)
#define CPS_IM_SHIFT            (2)
#define CPS_IM_MASK             (3)
#define CPS_DI                  (1 << 1)
#define CPS_DA                  (1 << 0)

#define CFG_PRL_MASK            (0xff)
#define CFG_PRL_SHIFT           (24)
#define CFG_DW                  (1 << 5)
#define CFG_VF                  (1 << 4)
#define CFG_RV                  (1 << 3)
#define CFG_BO                  (1 << 2)
#define CFG_CP                  (1 << 1)
#define CFG_CD                  (1 << 0)

#define CHC_CE_CNTL_MASK        (0xff)
#define CHC_CE_CNTL_SHIFT       (24)
#define CHC_CR_MASK             (0xff)
#define CHC_CR_SHIFT            (16)
#define CHC_LS                  (1 << 15)
#define CHC_ML                  (1 << 14)
#define CHC_ST                  (1 << 13)
#define CHC_LA                  (1 << 12)
#define CHC_TF                  (1 << 11)
#define CHC_TR_MASK             (0xff)
#define CHC_TR_SHIFT            (2)
#define CHC_NN                  (1 << 1)
#define CHC_CV                  (1 << 0)

#define RBP_MASK                (0xffff)

#define TCV_MASK                (0x00ffffff)
#define TCV_SHIFT               (0)

#define TMR_OV                  (1 << 26)
#define TMR_IN                  (1 << 25)
#define TMR_IE                  (1 << 24)
#define TMR_TRV_MASK            (0x00ffffff)
#define TMR_TRV_SHIFT           (0)

#define PC_MASK                 (0xfffffffc)

#define MMU_PS_MASK             (3)
#define MMU_PS_SHIFT            (8)
#define MMU_PID_MASK            (0xff)
#define MMU_PID_SHIFT           (0)

#define LRU_MASK                (0x3f)
#define LRU_SHIFT               (1)

#define ALU_DF_SHIFT            (11)
#define ALU_DF                  (1 << 11)
#define ALU_V_SHIFT             (10)
#define ALU_V                   (1 << 10)
#define ALU_N_SHIFT             (9)
#define ALU_N                   (1 << 9)
#define ALU_Z_SHIFT             (8)
#define ALU_Z                   (1 << 8)
#define ALU_C_SHIFT             (7)
#define ALU_C                   (1 << 7)
#define ALU_BP_MASK             (3)
#define ALU_BP_SHIFT            (5)
#define ALU_FC_MASK             (0x1f)
#define ALU_FC_SHIFT            (0)

#define IPX_MASK                (0xff)
#define IPX_SHIFT               (2)




/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define AM29000_INTR0       0
#define AM29000_INTR1       1
#define AM29000_INTR2       2
#define AM29000_INTR3       3


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(AM29000, am29000);

#endif /* __AM29000_H__ */
