// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

void unsp_device::execute_jumps(const uint16_t op)
{
	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t op1 = (op >> 6) & 7;
	const uint32_t opimm = op & 0x3f;

	switch (op0)
	{
	case 0: // JB
		if (!(m_core->m_r[REG_SR] & UNSP_C))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 1: // JAE
		if (m_core->m_r[REG_SR] & UNSP_C)
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 2: // JGE
		if (!(m_core->m_r[REG_SR] & UNSP_S))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 3: // JL
		if (m_core->m_r[REG_SR] & UNSP_S)
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 4: // JNE
		if (!(m_core->m_r[REG_SR] & UNSP_Z))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 5: // JE
		if (m_core->m_r[REG_SR] & UNSP_Z)
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 6: // JPL
		if (!(m_core->m_r[REG_SR] & UNSP_N))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 7: // JMI
		if (m_core->m_r[REG_SR] & UNSP_N)
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 8: // JBE
		if ((m_core->m_r[REG_SR] & (UNSP_Z | UNSP_C)) != UNSP_C) // branch if (!UNSP_Z && !UNSP_C) || UNSP_Z
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 9: // JA
		if ((m_core->m_r[REG_SR] & (UNSP_Z | UNSP_C)) == UNSP_C) // branch if !UNSP_Z && UNSP_C
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 10: // JLE
		if (m_core->m_r[REG_SR] & (UNSP_Z | UNSP_S))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 11: // JG
		if (!(m_core->m_r[REG_SR] & (UNSP_Z | UNSP_S)))
		{
			m_core->m_icount -= 4;
			add_lpc((op1 == 0) ? opimm : (0 - opimm));
		}
		else
		{
			m_core->m_icount -= 2;
		}
		return;
	case 14: // JMP
		add_lpc((op1 == 0) ? opimm : (0 - opimm));
		m_core->m_icount -= 4;
		return;
	default:
		unimplemented_opcode(op);
		return;
	}
}
