// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "ht1130d.h"

ht1130_disassembler::ht1130_disassembler()
	: util::disasm_interface()
{
}

u32 ht1130_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t ht1130_disassembler::disassemble(std::ostream &stream, offs_t pc, const ht1130_disassembler::data_buffer &opcodes, const ht1130_disassembler::data_buffer &params)
{
	u8 inst = opcodes.r8(pc);

	switch (inst)
	{
	case 0b00001000: // ADC A,[R1R0] : Add data memory contents and carry to the accumulator
	{
		stream << "ADC A,[R1R0]";
		return 1;
	}

	case 0b00001001: // ADD A,[R1R0] : Add data memory contents to the accumulator
	{
		stream << "ADD A,[R1R0]";
		return 1;
	}

	case 0b00011010: // AND A,[R1R0] : Logical AND accumulator with data memory
	{
		stream << "AND A,[R1R0]";
		return 1;
	}

	case 0b00011101: // AND [R1R0],A : Logical AND data memory with accumulator
	{
		stream << "AND [R1R0],A";
		return 1;
	}

	case 0b00101010: // CLC : Clear carry flag
	{
		stream << "CLC";
		return 1;
	}

	case 0b00110110: // DAA : Decimal-Adjust accumulator
	{
		stream << "DAA";
		return 1;
	}

	case 0b00111111: // DEC A : Decrement accumulator
	{
		stream << "DEC A";
		return 1;
	}

	case 0b00001101: // DEC [R1R0] : Decrement data memory
	{
		stream << "DEC [R1R0]";
		return 1;
	}

	case 0b00001111: // DEC [R3R2] : Decrement data memory
	{
		stream << "DEC [R3R2]";
		return 1;
	}

	case 0b00101101: // DI : Disable interrupt
	{
		stream << "DI";
		return 1;
	}

	case 0b00101100: // EI : Enable interrupt
	{
		stream << "EI";
		return 1;
	}

	case 0b00110010: // IN A,PM : Input port to accumulator
	{
		stream << "IN A,PM";
		return 1;
	}

	case 0b00110011: // IN A,PS : Input port to accumulator
	{
		stream << "IN A,PS";
		return 1;
	}

	case 0b00110100: // IN A,PP : Input port to accumulator
	{
		stream << "IN A,PP";
		return 1;
	}

	case 0b00110001: // INC A : Increment accumulator
	{
		stream << "INC A";
		return 1;
	}

	case 0b00001100: // INC [R1R0] : Increment data memory
	{
		stream << "INC [R1R0]";
		return 1;
	}

	case 0b00001110: // INC [R3R2] : Increment data memory
	{
		stream << "INC [R3R2]";
		return 1;
	}

	case 0b00111011: // MOV A,TMRH : Move timer high nibble to accumulator
	{
		stream << "MOV A,TMRH";
		return 1;
	}

	case 0b00111010: // MOV A,TMRL : Move timer low nibble to accumulator
	{
		stream << "MOV A,TMRL";
		return 1;
	}

	case 0b00000100: // MOV A,[R1R0] : Move data memory to accumulator
	{
		stream << "MOV A,[R1R0]";
		return 1;
	}

	case 0b00000110: // MOV A,[R3R2] : Move data memory to accumulator
	{
		stream << "MOV A,[R3R2]";
		return 1;
	}

	case 0b00111101: // MOV TMRH,A : Move accumulator to timer high nibble
	{
		stream << "MOV TMRH,A";
		return 1;
	}

	case 0b00111100: // MOV TMRL,A : Move accumulator to timer low nibble
	{
		stream << "MOV TMRL,A";
		return 1;
	}

	case 0b00000101: // MOV [R1R0],A : Move accumulator to data memory
	{
		stream << "MOV [R1R0],A";
		return 1;
	}

	case 0b00000111: // MOV [R3R2],A : Move accumulator to data memory
	{
		stream << "MOV [R3R2],A";
		return 1;
	}

	case 0b00111110: // NOP : No operation
	{
		stream << "NOP";
		return 1;
	}

	case 0b00011100: // OR A,[R1R0] : Logical OR accumulator with data memory
	{
		stream << "OR A,[R1R0]";
		return 1;
	}

	case 0b00011111: // OR [R1R0],A : Logically OR data memory with accumulator
	{
		stream << "OR [R1R0],A";
		return 1;
	}

	case 0b00110000: // OUT PA,A : Output accumulator data to port A
	{
		stream << "OUT PA,A";
		return 1;
	}

	case 0b01001110: // READ MR0A : Read ROM code of current page to M(R1,R0) and ACC
	{
		stream << "READ MR0A";
		return 1;
	}

	case 0b01001100: // READ R4A : Read ROM code of current page to R4 and accumulator
	{
		stream << "READ R4A";
		return 1;
	}

	case 0b01001111: // READF MR0A : Read ROM Code of page F to M(R1,R0) and ACC
	{
		stream << "READF MR0A";
		return 1;
	}

	case 0b01001101: // READF R4A : Read ROM code of page F to R4 and accumulator
	{
		stream << "READF R4A";
		return 1;
	}

	case 0b00101110: // RET : Return from subroutine or interrupt
	{
		stream << "RET";
		return 1;
	}

	case 0b00101111: // RETI : Return from interrupt subroutine
	{
		stream << "RETI";
		return 1;
	}

	case 0b00000001: // RL A : Rotate accumulator left
	{
		stream << "RL A";
		return 1;
	}

	case 0b00000011: // RLC A : Rotate accumulator left through carry
	{
		stream << "RLC A";
		return 1;
	}

	case 0b00000000: // RR A : Rotate accumulator right
	{
		stream << "RR A";
		return 1;
	}

	case 0b00000010: // RRC A : Rotate accumulator right through carry
	{
		stream << "RRC A";
		return 1;
	}

	case 0b00001010: // SBC A,[R1R0] : Subtract data memory contents and carry from ACC
	{
		stream << "SBC A,[R1R0]";
		return 1;
	}

	case 0b01001011: // SOUND A : Activate SOUND channel with accumulator
	{
		stream << "SOUND A";
		return 1;
	}

	case 0b01001001: // SOUND LOOP : Turn on sound repeat cycle
	{
		stream << "SOUND LOOP";
		return 1;
	}

	case 0b01001010: // SOUND OFF : Turn off sound
	{
		stream << "SOUND OFF";
		return 1;
	}

	case 0b01001000: // SOUND ONE : Turn on sound 1 cycle
	{
		stream << "SOUND ONE";
		return 1;
	}

	case 0b00101011: // STC : Set carry flag
	{
		stream << "STC";
		return 1;
	}

	case 0b00001011: // SUB A,[R1R0] : Subtract data memory contents from accumulator
	{
		stream << "SUB A,[R1R0]";
		return 1;
	}

	case 0b00111001: // TIMER OFF : Set timer to stop counting
	{
		stream << "TIMER OFF";
		return 1;
	}

	case 0b00111000: // TIMER ON : Set timer to start counting
	{
		stream << "TIMER ON";
		return 1;
	}

	case 0b00011011: // XOR A,[R1R0] : Logical XOR accumulator with data memory
	{
		stream << "XOR A,[R1R0]";
		return 1;
	}

	case 0b00011110: // XOR [R1R0],A : Logical XOR data memory with accumulator
	{
		stream << "XOR [R1R0],A";
		return 1;
	}

	//// Opcodes with XH Immediates

	case 0b01000000: // (with 4-bit immediate) : ADD A,XH : Add immediate data to the accumulator
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "ADD A,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000010: // (with 4-bit immediate) : AND A,XH : Logical AND immediate data to accumulator
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "AND A,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000110: // (with 4-bit immediate) : MOV R4,XH : Move immediate data to R4
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "MOV R4,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000100: // (with 4-bit immediate) : OR A,XH :  Logical OR immediate data to accumulator
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "OR A,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000001: // (with 4-bit immediate) : SUB A,XH : Subtract immediate data from accumulator0
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "SUB A,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000011: // (with 4-bit immediate) : XOR A,XH : Logical XOR immediate data to accumulator
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "XOR A,0x%02x", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	//case 0b0111dddd: // MOV A,XH : Move immediate data to accumulator
	case 0b01110000: case 0b01110001: case 0b01110010: case 0b01110011:
	case 0b01110100: case 0b01110101: case 0b01110110: case 0b01110111:
	case 0b01111000: case 0b01111001: case 0b01111010: case 0b01111011:
	case 0b01111100: case 0b01111101: case 0b01111110: case 0b01111111:
	{
		u8 operand = inst & 0x0f;
		util::stream_format(stream, "MOV A,0x%02x", operand);
		return 1;
	}

	// Ops using registers

	// case 0b0001nnn1: DEC Rn : Decrement register (R0-R4)
	case 0b00010001: case 0b00010011: case 0b00010101: case 0b00010111:
	case 0b00011001:
	{
		u8 reg = (inst & 0x0e) >> 1;
		util::stream_format(stream, "DEC R%d", reg);
		return 1;
	}

	// case 0b0001nnn0: INC Rn : Increment register
	case 0b00010000: case 0b00010010: case 0b00010100: case 0b00010110:
	case 0b00011000:
	{
		u8 reg = (inst & 0x0e) >> 1;
		util::stream_format(stream, "INC R%d", reg);
		return 1;
	}

	// case 0b0010nnn1: MOV A,Rn : Move register to accumulator
	case 0b00100001: case 0b00100011: case 0b00100101: case 0b00100111:
	case 0b00101001:
	{
		u8 reg = (inst & 0x0e) >> 1;
		util::stream_format(stream, "MOV A,R%d", reg);
		return 1;
	}

	//case 0b0010nnn0: MOV Rn,A : Move accumulator to register
	case 0b00100000: case 0b00100010: case 0b00100100: case 0b00100110:
	case 0b00101000:
	{
		u8 reg = (inst & 0x0e) >> 1;
		util::stream_format(stream, "MOV R%d, A", reg);
		return 1;
	}

	// dual move ops

	//case 0b0101dddd: // (with 4-bit immediate) : MOV R1R0,XXH : Move immediate data to R1 and R0
	case 0b01010000: case 0b01010001: case 0b01010010: case 0b01010011:
	case 0b01010100: case 0b01010101: case 0b01010110: case 0b01010111:
	case 0b01011000: case 0b01011001: case 0b01011010: case 0b01011011:
	case 0b01011100: case 0b01011101: case 0b01011110: case 0b01011111:
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			uint8_t fulldata = (inst & 0x0f) | (operand << 4);
			util::stream_format(stream, "MOV R1R0,0x%02x", fulldata);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	//case 0b0110dddd: // (with 4-bit immediate) : MOV R3R2,XXH : Move immediate data to R3 and R2
	case 0b01100000: case 0b01100001: case 0b01100010: case 0b01100011:
	case 0b01100100: case 0b01100101: case 0b01100110: case 0b01100111:
	case 0b01101000: case 0b01101001: case 0b01101010: case 0b01101011:
	case 0b01101100: case 0b01101101: case 0b01101110: case 0b01101111:
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			uint8_t fulldata = (inst & 0x0f) | (operand << 4);
			util::stream_format(stream, "MOV R3R2,0x%02x", fulldata);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	// Jump / Call opcodes with full addresses

	// case 0b1111aaaa: // (with 8-bit immediate) : CALL address : Subroutine call
	case 0b11110000: case 0b11110001: case 0b11110010: case 0b11110011:
	case 0b11110100: case 0b11110101: case 0b11110110: case 0b11110111:
	case 0b11111000: case 0b11111001: case 0b11111010: case 0b11111011:
	case 0b11111100: case 0b11111101: case 0b11111110: case 0b11111111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x0f) << 8) | operand;
		util::stream_format(stream, "CALL %03x", fulladdr);
		return 2;
	}

	// case 0b1110aaaa: // (with 8-bit immediate) : JMP address : Direct jump
	case 0b11100000: case 0b11100001: case 0b11100010: case 0b11100011:
	case 0b11100100: case 0b11100101: case 0b11100110: case 0b11100111:
	case 0b11101000: case 0b11101001: case 0b11101010: case 0b11101011:
	case 0b11101100: case 0b11101101: case 0b11101110: case 0b11101111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x0f) << 8) | operand;
		util::stream_format(stream, "JMP %03x", fulladdr);
		return 2;
	}

	// Jump / Call opcodes with partial address

	// case 0b11000aaa: (with 8-bit immediate) : JC address : Jump if carry is set
	case 0b11000000: case 0b11000001: case 0b11000010: case 0b11000011:
	case 0b11000100: case 0b11000101: case 0b11000110: case 0b11000111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JC %03x", fulladdr);
		return 2;
	}

	// case 0b11001aaa: (with 8-bit immediate) : JNC address : Jump if carry is not set
	case 0b11001000: case 0b11001001: case 0b11001010: case 0b11001011:
	case 0b11001100: case 0b11001101: case 0b11001110: case 0b11001111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JNC %03x", fulladdr);
		return 2;
	}

	// case 0b10111aaa: (with 8-bit immediate) : JNZ A,address : Jump if accumulator is not 0
	case 0b10111000: case 0b10111001: case 0b10111010: case 0b10111011:
	case 0b10111100: case 0b10111101: case 0b10111110: case 0b10111111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JNZ A,%03x", fulladdr);
		return 2;
	}

	// case 0b10100aaa: (with 8-bit immediate) : JNZ R0,address : Jump if register is not 0
	case 0b10100000: case 0b10100001: case 0b10100010: case 0b10100011:
	case 0b10100100: case 0b10100101: case 0b10100110: case 0b10100111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JNZ R0,%03x", fulladdr);
		return 2;
	}

	// case 0b10101aaa: (with 8-bit immediate) : JNZ R1,address : Jump if register is not 0
	case 0b10101000: case 0b10101001: case 0b10101010: case 0b10101011:
	case 0b10101100: case 0b10101101: case 0b10101110: case 0b10101111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JNZ R1,%03x", fulladdr);
		return 2;
	}

	// case 0b11011aaa: (with 8-bit immediate) : JNZ R4,address : Jump if register is not 0
	case 0b11011000: case 0b11011001: case 0b11011010: case 0b11011011:
	case 0b11011100: case 0b11011101: case 0b11011110: case 0b11011111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JNZ R4,%03x", fulladdr);
		return 2;
	}

	// case 0b11010aaa: (with 8-bit immediate) : JTMR address : Jump if time-out
	case 0b11010000: case 0b11010001: case 0b11010010: case 0b11010011:
	case 0b11010100: case 0b11010101: case 0b11010110: case 0b11010111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JTMR %03x", fulladdr);
		return 2;
	}

	// case 0b10110aaa: (with 8-bit immediate) : JZ A,address : Jump if accumulator is 0
	case 0b10110000: case 0b10110001: case 0b10110010: case 0b10110011:
	case 0b10110100: case 0b10110101: case 0b10110110: case 0b10110111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		util::stream_format(stream, "JZ A,%03x", fulladdr);
		return 2;
	}

	//case 0b100nnaaa: // (with 8-bit immediate) : JAn address : Jump if accumulator bit n is set
	case 0b10000000: case 0b10000001: case 0b10000010: case 0b10000011:
	case 0b10000100: case 0b10000101: case 0b10000110: case 0b10000111:
	case 0b10001000: case 0b10001001: case 0b10001010: case 0b10001011:
	case 0b10001100: case 0b10001101: case 0b10001110: case 0b10001111:
	case 0b10010000: case 0b10010001: case 0b10010010: case 0b10010011:
	case 0b10010100: case 0b10010101: case 0b10010110: case 0b10010111:
	case 0b10011000: case 0b10011001: case 0b10011010: case 0b10011011:
	case 0b10011100: case 0b10011101: case 0b10011110: case 0b10011111:
	{
		u8 operand = opcodes.r8(pc + 1);
		uint16_t fulladdr = ((inst & 0x07) << 8) | operand | (pc & 0x800);
		uint8_t bit = (inst & 0x18)>>3;
		util::stream_format(stream, "JA%d %03x", bit, fulladdr);
		return 2;
	}

	// other Ops
	case 0b01000101: // (with 4 bit immediate) : SOUND n : Activate SOUND channel n
	{
		u8 operand = opcodes.r8(pc + 1);
		if (!(operand & 0xf0))
		{
			util::stream_format(stream, "SOUND %d", operand);
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}


	case 0b00110111: // (with 00111110) : HALT Halt system clock
	{
		u8 operand = opcodes.r8(pc + 1);
		if (operand == 0b00111110)
		{
			stream << "HALT";
		}
		else
		{
			util::stream_format(stream, "<ill %02x %02x>", inst, operand);
		}
		return 2;
	}

	case 0b01000111: // (with 8-bit immediate) : TIMER XXH : Set immediate data to timer counter
	{
		u8 operand = opcodes.r8(pc + 1);
		util::stream_format(stream, "TIMER %02x", operand);
		return 2;
	}

	default:
	{
		util::stream_format(stream, "<ill %02x>", inst);
		return 1;
	}
	}

	return 1;
}
