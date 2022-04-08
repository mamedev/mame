// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "unspdasm.h"

// TODO: the call conventions and general use of 'do_basic_alu_ops' from unsp_other.cpp could be cleaned up
// at the moment things like the 'Store' opcode make things more complex than they need to be, and require
// the passing of a store address each time, they make the regular use of the code a little awkward too tho.

void unsp_device::execute_extended_group(uint16_t op)
{
	// shouldn't get here anyway
	logerror("<UNKNOWN EXTENDED>\n");
	unimplemented_opcode(op);
	return;
}

void unsp_20_device::execute_extended_group(uint16_t op)
{
	uint16_t ximm = read16(UNSP_LPC);
	add_lpc(1);

	switch ((ximm & 0x01f0) >> 4)
	{
	case 0x00: case 0x10:
	{
		// Ext Register
		// Ra = Ra op Rb
		// A  = B  op C
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		uint16_t b = m_core->m_r[ra];
		uint16_t c = m_core->m_r[rb];
		uint32_t lres;
		uint32_t storeaddr = b; // dest address for STORE

		bool write = do_basic_alu_ops(aluop, lres, b, c, storeaddr, (ra != 7));

		if (write)
		{
			// a = Ra
			m_core->m_r[ra] = (uint16_t)lres;
		}

		return;
	}
	case 0x02:
	{
		// Ext Push/Pop
		if (ximm & 0x8000)
		{
			uint8_t rb = (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;

			while (size--)
			{
				push(m_core->m_r[((rx--)&7) + 8], &m_core->m_r[rb]);
			}
			return;
		}
		else
		{
			uint8_t rb = (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;

			while (size--)
			{
				m_core->m_r[((++rx)&7) + 8] = pop(&m_core->m_r[rb]);
			}
			return;
		}
		return;
	}
	case 0x04:  case 0x14:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra = Rb op IMM16
		// A  = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;


		uint16_t b = m_core->m_r[rb];
		uint32_t lres;
		uint32_t storeaddr = 0;// m_core->m_r[ra]; // dest address for STORE (invalid anyway?)

		if (aluop == 0xd)
		{
			// store is invalid?
			unimplemented_opcode(op, ximm, imm16_2);
		}

		bool write = do_basic_alu_ops(aluop, lres, b, imm16_2, storeaddr, (ra != 7));

		if (write)
		{
			// a = Ra
			m_core->m_r[ra] = (uint16_t)lres;
		}

		return;
	}

	case 0x06:
	case 0x16:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra = Rb op [A16]
		// A  = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		uint16_t b = m_core->m_r[rb];
		uint16_t c = read16(imm16_2);
		uint32_t storeaddr = imm16_2; // dest address for STORE
		uint32_t lres;

		bool write = do_basic_alu_ops(aluop, lres, b, c, storeaddr, (ra != 7));

		if (write)
		{
			// a = Ra
			m_core->m_r[ra] = (uint16_t)lres;
		}

		return;
	}

	case 0x07:
	case 0x17:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		//[A16] = Ra op Rb
		// A    = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		uint16_t a = imm16_2;
		uint16_t b = m_core->m_r[ra];
		uint16_t c = m_core->m_r[rb];
		uint32_t lres;
		uint32_t r2 = a;

		bool write = do_basic_alu_ops(aluop, lres, b, c, r2, true);

		if (write)
		{
			write16(imm16_2, lres);
		}

		return;
	}

	case 0x08: case 0x09: // Ext Indirect, Rx = Rx op [Ry@]
	case 0x0a: case 0x0b: // Ext Indirect, Rx = Rx op ds:[Ry@]
	{
		uint16_t r0 = 0;
		uint16_t r1 = 0;
		uint32_t r2 = 0;
		uint32_t lres = 0;

		uint16_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) + 8;
		uint8_t rx = ((ximm & 0x0e00) >> 9) + 8;
		uint8_t use_ds = BIT(ximm, 5);
		uint8_t form = (ximm & 0x0018) >> 3;

		r0 = m_core->m_r[rx];

		switch (form)
		{
			case 0x0: // Rx, [<ds:>Ry]
				r2 = use_ds ? UNSP_LREG_I(ry) : m_core->m_r[ry];
				if (aluop != 0x0d)
					r1 = read16(r2);
				break;
			case 0x1: // Rx, [<ds:>Ry--]
				r2 = use_ds ? UNSP_LREG_I(ry) : m_core->m_r[ry];
				if (aluop != 0x0d)
					r1 = read16(r2);
				m_core->m_r[ry] = (uint16_t)(m_core->m_r[ry] - 1);
				if (m_core->m_r[ry] == 0xffff && use_ds)
					m_core->m_r[REG_SR] -= 0x0400;
				break;
			case 0x2: // Rx, [<ds:>Ry++]
				r2 = use_ds ? UNSP_LREG_I(ry) : m_core->m_r[ry];
				if (aluop != 0x0d)
					r1 = read16(r2);
				m_core->m_r[ry] = (uint16_t)(m_core->m_r[ry] + 1);
				if (m_core->m_r[ry] == 0x0000 && use_ds)
					m_core->m_r[REG_SR] += 0x0400;
				break;
			case 0x3: // Rx, [<ds:>++Ry]
				m_core->m_r[ry] = (uint16_t)(m_core->m_r[ry] + 1);
				if (m_core->m_r[ry] == 0x0000 && use_ds)
					m_core->m_r[REG_SR] += 0x0400;
				r2 = use_ds ? UNSP_LREG_I(ry) : m_core->m_r[ry];
				if (aluop != 0x0d)
					r1 = read16(r2);
				break;
		}

		if (do_basic_alu_ops(aluop, lres, r0, r1, r2, (aluop != 7) ? true : false))
		{
			m_core->m_r[rx] = (uint16_t)lres;
		}

		return;
	}
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	{
		// Ext IM6
		// Rx = Rx op IM6
		// A  = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		uint16_t b = m_core->m_r[rx + 8];
		uint16_t c = imm6;

		uint32_t storeaddr = 0; // dest address for STORE
		uint32_t lres;

		if (aluop == 0xd)
		{
			// store is invalid?
			unimplemented_opcode(op, ximm);
		}

		bool write = do_basic_alu_ops(aluop, lres, b, c, storeaddr, true);

		if (write)
		{
			// A = Rx
			m_core->m_r[rx + 8] = (uint16_t)lres;
		}

		return;
	}

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	{
		// Ext Base+Disp6
		// Rx = Rx op [BP+IM6]
		// A  = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		uint32_t addr = (uint16_t)(m_core->m_r[REG_BP] + (imm6 & 0x3f));

		uint16_t b = m_core->m_r[rx + 8];
		uint16_t c = read16(addr);

		uint32_t storeaddr = addr; // dest address for STORE
		uint32_t lres;

		bool write = do_basic_alu_ops(aluop, lres, b, c, storeaddr, true);

		if (write)
		{
			// A = Rx
			m_core->m_r[rx + 8] = (uint16_t)lres;
		}

		return;
	}

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	{
		// Ext A6
		// Rx = Rx op [A6]
		// A  = B  op C

		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t a6 = (ximm & 0x003f) >> 0;

		uint16_t b = m_core->m_r[rx + 8];
		uint16_t c = read16(a6);

		uint32_t storeaddr = a6; // dest address for STORE
		uint32_t lres;

		bool write = do_basic_alu_ops(aluop, lres, b, c, storeaddr, true);

		if (write)
		{
			// A = Rx
			m_core->m_r[rx + 8] = (uint16_t)lres;
		}

		return;
	}
	}

	// illegal?
	unimplemented_opcode(op, ximm);
	return;
}
