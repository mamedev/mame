/* ASG 971222 -- rewrote this interface */
#pragma once

#ifndef __I286INTF_H__
#define __I286INTF_H__

#include "i86.h"

enum
{
	I286_PC = 0,

	/* 8-bit registers */
	I286_AL,
	I286_AH,
	I286_BL,
	I286_BH,
	I286_CL,
	I286_CH,
	I286_DL,
	I286_DH,

	/* 16-bit registers */
	I286_AX,
	I286_BX,
	I286_CX,
	I286_DX,
	I286_BP,
	I286_SP,
	I286_SI,
	I286_DI,
	I286_IP,

	/* segment registers */
	I286_CS,
	I286_CS_BASE,
	I286_CS_LIMIT,
	I286_CS_FLAGS,
	I286_SS,
	I286_SS_BASE,
	I286_SS_LIMIT,
	I286_SS_FLAGS,
	I286_DS,
	I286_DS_BASE,
	I286_DS_LIMIT,
	I286_DS_FLAGS,
	I286_ES,
	I286_ES_BASE,
	I286_ES_LIMIT,
	I286_ES_FLAGS,

	/* other */
	I286_FLAGS,
	I286_MSW,

	I286_GDTR_BASE,
	I286_GDTR_LIMIT,
	I286_IDTR_BASE,
	I286_IDTR_LIMIT,
	I286_TR,
	I286_TR_BASE,
	I286_TR_LIMIT,
	I286_TR_FLAGS,
	I286_LDTR,
	I286_LDTR_BASE,
	I286_LDTR_LIMIT,
	I286_LDTR_FLAGS,
};

#define TRAP(fault, code)  (UINT32)(((fault&0xffff)<<16)|(code&0xffff))

/* Public functions */
DECLARE_LEGACY_CPU_DEVICE(I80286, i80286);

#endif /* __I286INTF_H__ */
