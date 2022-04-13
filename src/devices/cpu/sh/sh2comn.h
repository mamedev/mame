// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh2common.h
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH2_SH2COMN_H
#define MAME_CPU_SH2_SH2COMN_H

#pragma once

enum
{
	ICF  = 0x80,
	OCFA = 0x08,
	OCFB = 0x04,
	OVF  = 0x02,
	CCLRA = 0x01
};

#define SH12_AM  0xc7ffffff

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


#endif // MAME_CPU_SH2_SH2COMN_H
