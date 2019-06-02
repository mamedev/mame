// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

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
		// Ext Register Ra = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		logerror("(Ext) %s = %s %s %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														 , aluops[aluop]
														 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);

		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x02:
	{
		// register decoding could be incorrect here

		// Ext Push/Pop
		if (ximm & 0x8000)
		{
			// just skip for now, as nothing really even uses the extended registers yet
			/*
			uint8_t rb =   (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx   = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;
			size -= 1;

			if ((rx-size) >= 0)
			    logerror("(Ext) push %s, %s to [%s]\n",
			           extregs[rx-size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			else
			    logerror("(Ext) push <BAD>\n");
			*/
			//unimplemented_opcode(op, ximm);
			return;
		}
		else
		{
			// just skip for now, as nothing really even uses the extended registers yet
			/*
			uint8_t rb =   (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx   = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;
			size -= 1;


			if ((rx-size) >= 0)
			    logerror("(Ext) pop %s, %s from [%s]\n",
			           extregs[rx-size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			else
			    logerror("(Ext) pop <BAD>\n");
			*/
			//unimplemented_opcode(op, ximm);
			return;
		}
		return;
	}
	case 0x04:  case 0x14:
	{
		uint16_t ximm_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra=Rb op IMM16
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		logerror("(Ext) %s = %s %s %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , aluops[aluop]
														   , ximm_2);

		unimplemented_opcode(op, ximm, ximm_2);
		return;
	}

	case 0x06:
	case 0x16:
	{
		uint16_t ximm_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra=Rb op [A16]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		logerror("(Ext) %s = %s %s [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , aluops[aluop]
															 , ximm_2);

		unimplemented_opcode(op, ximm, ximm_2);
		return;
	}

	case 0x07:
	case 0x17:
	{
		uint16_t ximm_2 = read16(UNSP_LPC);
		add_lpc(1);

		//[A16] = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		logerror("(Ext) [0x4x] = %s %s %s\n", ximm_2
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , aluops[aluop]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);

		unimplemented_opcode(op, ximm, ximm_2);
		return;
	}

	case 0x08: case 0x09:
	{
		// Ext Indirect Rx=Rx op [Ry@]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		logerror("(Ext) %s=%s %s", extregs[rx], extregs[rx], aluops[aluop]);
		logerror(forms[form], extregs[ry]);
		logerror("\n");
		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x0a: case 0x0b:
	{
		// Ext DS_Indirect Rx=Rx op ds:[Ry@]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		logerror("(Ext) %s=%s %s ds:", extregs[rx], extregs[rx], aluops[aluop]);
		logerror(forms[form], extregs[ry]);
		logerror("\n");
		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	{
		// Ext IM6 Rx=Rx op IM6
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		logerror("(Ext) %s=%s %s %02x\n", extregs[rx], extregs[rx], aluops[aluop], imm6 );
		unimplemented_opcode(op, ximm);
		return;
	}

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	{
		// Ext Base+Disp6 Rx=Rx op [BP+IM6]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		logerror("(Ext) %s=%s %s [BP+%02x]\n", extregs[rx], extregs[rx], aluops[aluop], imm6 );
		unimplemented_opcode(op, ximm);
		return;
	}

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	{
		// Ext A6 Rx=Rx op [A6]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t a6 = (ximm & 0x003f) >> 0;

		logerror("(Ext) %s=%s %s [%02x]\n", extregs[rx], extregs[rx], aluops[aluop], a6 );
		unimplemented_opcode(op, ximm);
		return;
	}
	}

	// illegal?
	unimplemented_opcode(op, ximm);
	return;
}
