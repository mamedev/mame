// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh2common.h
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#pragma once

#ifndef __SH2COMN_H__
#define __SH2COMN_H__



// do we use a timer for the DMA, or have it in CPU_EXECUTE
#define USE_TIMER_FOR_DMA

#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

#define SH2_CODE_XOR(a)     ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))

enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

#define T   0x00000001
#define S   0x00000002
#define I   0x000000f0
#define Q   0x00000100
#define M   0x00000200

#define AM  0xc7ffffff

#define FLAGS   (M|Q|I|S|T)

#define Rn  ((opcode>>8)&15)
#define Rm  ((opcode>>4)&15)

#define CPU_TYPE_SH1    (0)
#define CPU_TYPE_SH2    (1)

#define REGFLAG_R(n)                                        (1 << (n))

/* register flags 1 */
#define REGFLAG_PR                      (1 << 0)
#define REGFLAG_MACL                        (1 << 1)
#define REGFLAG_MACH                        (1 << 2)
#define REGFLAG_GBR                     (1 << 3)
#define REGFLAG_VBR                     (1 << 4)
#define REGFLAG_SR                      (1 << 5)

#define CHECK_PENDING_IRQ(message)              \
do {                                            \
	int irq = -1;                               \
	if (m_sh2_state->pending_irq & (1 <<  0)) irq = 0;  \
	if (m_sh2_state->pending_irq & (1 <<  1)) irq = 1;  \
	if (m_sh2_state->pending_irq & (1 <<  2)) irq = 2;  \
	if (m_sh2_state->pending_irq & (1 <<  3)) irq = 3;  \
	if (m_sh2_state->pending_irq & (1 <<  4)) irq = 4;  \
	if (m_sh2_state->pending_irq & (1 <<  5)) irq = 5;  \
	if (m_sh2_state->pending_irq & (1 <<  6)) irq = 6;  \
	if (m_sh2_state->pending_irq & (1 <<  7)) irq = 7;  \
	if (m_sh2_state->pending_irq & (1 <<  8)) irq = 8;  \
	if (m_sh2_state->pending_irq & (1 <<  9)) irq = 9;  \
	if (m_sh2_state->pending_irq & (1 << 10)) irq = 10; \
	if (m_sh2_state->pending_irq & (1 << 11)) irq = 11; \
	if (m_sh2_state->pending_irq & (1 << 12)) irq = 12; \
	if (m_sh2_state->pending_irq & (1 << 13)) irq = 13; \
	if (m_sh2_state->pending_irq & (1 << 14)) irq = 14; \
	if (m_sh2_state->pending_irq & (1 << 15)) irq = 15; \
	if ((m_sh2_state->internal_irq_level != -1) && (m_sh2_state->internal_irq_level > irq)) irq = m_sh2_state->internal_irq_level; \
	if (irq >= 0)                               \
		sh2_exception(message,irq);         \
} while(0)


#endif /* __SH2COMN_H__ */
