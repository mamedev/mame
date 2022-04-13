// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "unspdasm.h"

void unsp_device::execute_remaining(const uint16_t op)
{
	uint32_t lres = 0;
	uint16_t r0 = 0;
	uint16_t r1 = 0;
	uint32_t r2 = 0;

	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t opa = (op >> 9) & 7;
	const uint16_t op1 = (op >> 6) & 7;
	const uint16_t opn = (op >> 3) & 7;
	const uint16_t opb = op & 7;

	const uint8_t lower_op = (op1 << 4) | op0;

	if (lower_op == 0x2d) // Push
	{
		r0 = opn;
		r1 = opa;
		m_core->m_icount -= 4 + 2 * r0;

		while (r0--)
		{
			push(m_core->m_r[r1--], &m_core->m_r[opb]);
		}
		return;
	}
	else if (lower_op == 0x29)
	{
		if (op == 0x9a98) // reti
		{
			m_core->m_icount -= 8;
			if (m_core->m_ine)
			{
				set_fr(pop(&m_core->m_r[REG_SP]));
			}

			m_core->m_r[REG_SR] = pop(&m_core->m_r[REG_SP]);
			m_core->m_r[REG_PC] = pop(&m_core->m_r[REG_SP]);

			if (m_core->m_fiq)
			{
				m_core->m_fiq = 0;
			}
			else if (m_core->m_irq)
			{
				m_core->m_irq = 0;
			}
			return;
		}
		else // pop
		{
			r0 = opn;
			r1 = opa;
			m_core->m_icount -= 4 + 2 * r0;

			while (r0--)
			{
				m_core->m_r[++r1] = pop(&m_core->m_r[opb]);
			}
			return;
		}
	}

	// At this point, we should be dealing solely with ALU ops.

	r0 = m_core->m_r[opa];

	switch (op1)
	{
	case 0x00: // r, [bp+imm6]
		m_core->m_icount -= 6;

		r2 = (uint16_t)(m_core->m_r[REG_BP] + (op & 0x3f));
		if (op0 != 0x0d)
			r1 = read16(r2);
		break;

	case 0x01: // r, imm6
		m_core->m_icount -= 2;

		r1 = op & 0x3f;
		break;

	case 0x03: // Indirect
	{
		m_core->m_icount -= (opa == 7 ? 7 : 6);

		const uint8_t lsbits = opn & 3;
		if (opn & 4)
		{
			switch (lsbits)
			{
			case 0: // r, [<ds:>r]
				r2 = UNSP_LREG_I(opb);
				if (op0 != 0x0d)
					r1 = read16(r2);
				break;

			case 1: // r, [<ds:>r--]
				r2 = UNSP_LREG_I(opb);
				if (op0 != 0x0d)
					r1 = read16(r2);
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] - 1);
				if (m_core->m_r[opb] == 0xffff)
					m_core->m_r[REG_SR] -= 0x0400;
				break;
			case 2: // r, [<ds:>r++]
				r2 = UNSP_LREG_I(opb);
				if (op0 != 0x0d)
					r1 = read16(r2);
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
				if (m_core->m_r[opb] == 0x0000)
					m_core->m_r[REG_SR] += 0x0400;
				break;
			case 3: // r, [<ds:>++r]
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
				if (m_core->m_r[opb] == 0x0000)
					m_core->m_r[REG_SR] += 0x0400;
				r2 = UNSP_LREG_I(opb);
				if (op0 != 0x0d)
					r1 = read16(r2);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (lsbits)
			{
			case 0: // r, [r]
				r2 = m_core->m_r[opb];
				if (op0 != 0x0d)
					r1 = read16(r2);
				break;
			case 1: // r, [r--]
				r2 = m_core->m_r[opb];
				if (op0 != 0x0d)
					r1 = read16(r2);
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] - 1);
				break;
			case 2: // r, [r++]
				r2 = m_core->m_r[opb];
				if (op0 != 0x0d)
					r1 = read16(r2);
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
				break;
			case 3: // r, [++r]
				m_core->m_r[opb] = (uint16_t)(m_core->m_r[opb] + 1);
				r2 = m_core->m_r[opb];
				if (op0 != 0x0d)
					r1 = read16(r2);
				break;
			default:
				break;
			}
		}
		break;
	}

	case 0x04: // 16-bit ops
		switch (opn)
		{
		case 0x00: // r
			m_core->m_icount -= (opa == 7 ? 5 : 3);
			r1 = m_core->m_r[opb];
			break;

		case 0x01: // imm16
			m_core->m_icount -= (opa == 7 ? 5 : 4);
			r0 = m_core->m_r[opb];
			r1 = read16(UNSP_LPC);
			add_lpc(1);
			break;

		case 0x02: // [imm16]
			m_core->m_icount -= (opa == 7 ? 8 : 7);
			r0 = m_core->m_r[opb];
			r2 = read16(UNSP_LPC);
			add_lpc(1);

			if (op0 != 0x0d)
			{
				r1 = read16(r2);
			}
			break;

		case 0x03: // store [imm16], r
			m_core->m_icount -= (opa == 7 ? 8 : 7);
			r1 = r0;
			r0 = m_core->m_r[opb];
			r2 = read16(UNSP_LPC);
			add_lpc(1);
			// additional special case 'if (op1 == 0x04 && opn == 0x03)' write logic below
			break;

		default: // Shifted ops
		{
			m_core->m_icount -= (opa == 7 ? 5 : 3);
			uint32_t shift = (m_core->m_r[opb] << 4) | m_core->m_sb;
			if (shift & 0x80000)
				shift |= 0xf00000;
			shift >>= (opn - 3);
			m_core->m_sb = shift & 0x0f;
			r1 = (uint16_t)(shift >> 4);
			break;
		}
		}
		break;

	case 0x05: // More shifted ops
		m_core->m_icount -= (opa == 7 ? 5 : 3);

		if (opn & 4) // Shift right
		{
			const uint32_t shift = ((m_core->m_r[opb] << 4) | m_core->m_sb) >> (opn - 3);
			m_core->m_sb = shift & 0x0f;
			r1 = (uint16_t)(shift >> 4);
		}
		else // Shift left
		{
			const uint32_t shift = ((m_core->m_sb << 16) | m_core->m_r[opb]) << (opn + 1);
			m_core->m_sb = (shift >> 16) & 0x0f;
			r1 = (uint16_t)shift;
		}
		break;

	case 0x06: // Rotated ops
	{
		m_core->m_icount -= (opa == 7 ? 5 : 3);

		uint32_t shift = (((m_core->m_sb << 16) | m_core->m_r[opb]) << 4) | m_core->m_sb;
		if (opn & 4) // Rotate right
		{
			shift >>= (opn - 3);
			m_core->m_sb = shift & 0x0f;
		}
		else
		{
			shift <<= (opn + 1);
			m_core->m_sb = (shift >> 20) & 0x0f;
		}
		r1 = (uint16_t)(shift >> 4);
		break;
	}

	case 0x07: // Direct 6
		m_core->m_icount -= (opa == 7 ? 6 : 5);
		r2 = op & 0x3f;
		r1 = read16(r2);
		break;

	default:
		break;
	}

	bool write = do_basic_alu_ops(op0, lres, r0, r1, r2, (opa != 7) ? true : false);

	if (write)
	{
		if (op1 == 0x04 && opn == 0x03) // store [imm16], r
			write16(r2, lres);
		else
			m_core->m_r[opa] = (uint16_t)lres;
	}
}


bool unsp_device::do_basic_alu_ops(const uint16_t &op0, uint32_t &lres, uint16_t &r0, uint16_t &r1, uint32_t &r2, bool update_flags)
{
	switch (op0)
	{
	case 0x00: // Add
	{
		lres = r0 + r1;
		if (update_flags)
			update_nzsc(lres, r0, r1);
		break;
	}
	case 0x01: // Add w/ carry
	{
		uint32_t c = (m_core->m_r[REG_SR] & UNSP_C) ? 1 : 0;
		lres = r0 + r1 + c;
		if (update_flags)
			update_nzsc(lres, r0, r1);
		break;
	}
	case 0x02: // Subtract
		lres = r0 + (uint16_t)(~r1) + uint32_t(1);
		if (update_flags)
			update_nzsc(lres, r0, ~r1);
		break;
	case 0x03: // Subtract w/ carry
	{
		uint32_t c = (m_core->m_r[REG_SR] & UNSP_C) ? 1 : 0;
		lres = r0 + (uint16_t)(~r1) + c;
		if (update_flags)
			update_nzsc(lres, r0, ~r1);
		break;
	}
	case 0x04: // Compare
		lres = r0 + (uint16_t)(~r1) + uint32_t(1);
		if (update_flags)
			update_nzsc(lres, r0, ~r1);
		return false;
	case 0x06: // Negate
		lres = -r1;
		if (update_flags)
			update_nz(lres);
		break;
	case 0x08: // XOR
		lres = r0 ^ r1;
		if (update_flags)
			update_nz(lres);
		break;
	case 0x09: // Load
		lres = r1;
		if (update_flags)
			update_nz(lres);
		break;
	case 0x0a: // OR
		lres = r0 | r1;
		if (update_flags)
			update_nz(lres);
		break;
	case 0x0b: // AND
		lres = r0 & r1;
		if (update_flags)
			update_nz(lres);
		break;
	case 0x0c: // Test
		lres = r0 & r1;
		if (update_flags)
			update_nz(lres);
		return false;
	case 0x0d: // Store
		write16(r2, r0);
		return false;

	default:
		// pcp87xx 'Elevator Action' explicitly jumps into the middle of an earlier opcode (off-by-one error in the code)
		// It looks like the illegal op should have no meaningful effect so just log rather than fatalerroring
		logerror("UNSP: illegal ALU optype %02x at %04x\n", op0, UNSP_LPC);
		return false;
	}

	return true;
}
