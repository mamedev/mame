// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR disassembler

    Written by Ryan Holtz
*/

#include "emu.h"
#include "avr8dasm.h"

#define RD2(op)         (((op) >> 4) & 0x0003)
#define RD3(op)         (((op) >> 4) & 0x0007)
#define RD4(op)         (((op) >> 4) & 0x000f)
#define RD5(op)         (((op) >> 4) & 0x001f)
#define RR3(op)         ((op) & 0x0007)
#define RR4(op)         ((op) & 0x000f)
#define RR5(op)         ((((op) >> 5) & 0x0010) | ((op) & 0x000f))
#define KCONST6(op)     ((((op) >> 2) & 0x0030) | ((op) & 0x000f))
#define KCONST7(op)     (((op) >> 3) & 0x007f)
#define KCONST8(op)     ((((op) >> 4) & 0x00f0) | ((op) & 0x000f))
#define KCONST22(op)    (((((uint32_t)(op) >> 3) & 0x003e) | ((uint32_t)(op) & 0x0001)) << 16)
#define QCONST6(op)     ((((op) >> 8) & 0x0020) | (((op) >> 7) & 0x0018) | ((op) & 0x0007))
#define ACONST5(op)     (((op) >> 3) & 0x001f)
#define ACONST6(op)     ((((op) >> 5) & 0x0030) | ((op) & 0x000f))
#define MULCONST2(op)   ((((op) >> 6) & 0x0002) | (((op) >> 3) & 0x0001))

u32 avr8_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t avr8_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t base_pc = pc;
	uint32_t op = opcodes.r16(pc);
	pc += 2;
	uint32_t addr;
	const char* register_names[0x40] = {"PINA", "DDRA", "PORTA", "PINB", "DDRB", "PORTB", "PINC", "DDRC", "PORTC", "PIND", "DDRD", "PORTD", "PINE", "DDRE", "PORTE", "PINF", "DDRF", "PORTF", "PING", "DDRG", "PORTG", "TIFR0", "TIFR1", "TIFR2","TIFR3", "TIFR4", "TIFR5", "PCIFR", "EIFR", "EIMSK", "GPIOR0", "EECR", "EEDR", "EEARL", "EEARH", "GTCCR", "TCCR0A", "TCCR0B", "TCNT0", "OCR0A", "OCR0B", "0x29", "GPIOR1", "GPIOR2", "SPCR", "SPSR", "SPDR", "0x2F", "ACSR", "OCDR", "0x32", "SMCR", "MCUSR", "MCUCR", "0x36", "SPMCSR", "0x38", "0x39", "0x3A", "RAMPZ", "EIND", "SPL", "SPH", "SREG"};

	const char* register_bit_names[0x40][8] = {
	/* PINA   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRA   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTA  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PINB   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRB   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTB  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PINC   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRC   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTC  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PIND   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRD   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTD  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PINE   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRE   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTE  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PINF   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRF   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTF  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PING   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* DDRG   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* PORTG  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* TIFR0  */ { "TOV0", "OCF0A", "OCF0B",     "3",     "4",     "5",     "6",     "7"},
	/* TIFR1  */ { "TOV1", "OCF1A", "OCF1B", "OCF1C",     "4",  "ICF1",     "6",     "7"},
	/* TIFR2  */ { "TOV2", "OCF2A", "OCF2B",     "3",     "4",     "5",     "6",     "7"},
	/* TIFR3  */ { "TOV3", "OCF3A", "OCF3B", "OCF3C",     "4",  "ICF3",     "6",     "7"},
	/* TIFR4  */ { "TOV4", "OCF4A", "OCF4B", "OCF4C",     "4",  "ICF4",     "6",     "7"},
	/* TIFR5  */ { "TOV5", "OCF5A", "OCF5B", "OCF5C",     "4",  "ICF5",     "6",     "7"},
	/* PCIFR  */ {"PCIF0", "PCIF1", "PCIF2",     "3",     "4",     "5",     "6",     "7"},
	/* EIFR   */ {"INTF0", "INTF1", "INTF2", "INTF3", "INTF4", "INTF5", "INTF6", "INTF7"},
	/* EIMSK  */ { "INT0",  "INT1",  "INT2",  "INT3",  "INT4",  "INT5",  "INT6",  "INT7"},
	/* GPIOR0 */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* EECR   */ { "EERE",  "EEPE", "EEMPE", "EERIE", "EEPM0", "EEPM1",     "6",     "7"},
	/* EEDR   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* EEARL  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* EEARH  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* GTCCR  */ {"PSRSYNC", "PSRASY",  "2",     "3",     "4",     "5",     "6",   "TSM"},
	/* TCCR0A */ {"WGM00", "WGM01",     "2",     "3","COM0B0","COM0B1","COM0A0","COM0A1"},
	/* TCCR0B */ {  "CS0",   "CS1",   "CS2", "WGM02",     "4",     "5", "FOC0B", "FOC0A"},
	/* TCNT0  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* OCR0A  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* OCR0B  */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* 0x29   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* GPIOR1 */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* GPIOR2 */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* SPCR   */ { "SPR0",  "SPR1",  "CPHA",  "CPOL",  "MSTR",  "DORD",   "SPE",  "SPIE"},
	/* SPSR   */ {"SPI2X",     "1",     "2",     "3",     "4",     "5",  "WCOL",  "SPIF"},
	/* SPDR   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* 0x2F   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* ACSR   */ {"ACIS0", "ACIS1",  "ACIC",  "ACIE",   "ACI",   "ACO",  "ACBG",   "ACD"},
	/* OCDR   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* 0x32   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* SMCR   */ {   "SE",   "SM0",   "SM1",   "SM2",     "4",     "5",     "6",     "7"},
	/* MCUSR  */ { "PORF", "EXTRF",  "BORF",  "WDRF",  "JTRF",     "5",     "6",     "7"},
	/* MCUCR  */ { "IVCE", "IVSEL",     "2",     "3",   "PUD",     "5",     "6",   "JTD"},
	/* 0x36   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* SPMCSR */ {"SPMEN", "PGERS", "PGWRT","BLBSET","RWWSRE", "SIGRD", "RWWSB", "SPMIE"},
	/* 0x38   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* 0x39   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* 0x3A   */ {    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* RAMPZ  */ {"RAMPZ0","RAMPZ1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* EIND   */ {"EIND0",     "1",     "2",     "3",     "4",     "5",     "6",     "7"},
	/* SPL    */ {  "SP0",   "SP1",   "SP2",   "SP3",   "SP4",   "SP5",   "SP6",   "SP7"},
	/* SPH    */ {  "SP8",   "SP9",  "SP10",  "SP11",  "SP12",  "SP13",  "SP14",  "SP15"},
	/* SREG   */ {    "C",     "Z",     "N",     "V",     "S",     "H",     "T",     "I"}};

	offs_t flags = 0;
	switch(op & 0xf000)
	{
	case 0x0000:
		switch(op & 0x0f00)
		{
		case 0x0000:
			util::stream_format(stream, "NOP");
			break;
		case 0x0100:
			util::stream_format(stream, "MOVW    R%d:R%d, R%d:R%d", (RD4(op) << 1)+1, RD4(op) << 1, (RR4(op) << 1)+1, RR4(op) << 1);
			break;
		case 0x0200:
			util::stream_format(stream, "MULS    R%d, R%d", 16+RD4(op), 16+RR4(op));
			break;
		case 0x0300:
			switch(MULCONST2(op))
			{
			case 0:
				util::stream_format(stream, "MULSU   R%d, R%d", 16+RD3(op), 16+RR3(op));
				break;
			case 1:
				util::stream_format(stream, "FMUL    R%d, R%d", 16+RD3(op), 16+RR3(op));
				break;
			case 2:
				util::stream_format(stream, "FMULS   R%d, R%d", 16+RD3(op), 16+RR3(op));
				break;
			case 3:
				util::stream_format(stream, "FMULSU  R%d, R%d", 16+RD3(op), 16+RR3(op));
				break;
			}
			break;
		case 0x0400:
		case 0x0500:
		case 0x0600:
		case 0x0700:
			util::stream_format(stream, "CPC     R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0800:
		case 0x0900:
		case 0x0a00:
		case 0x0b00:
			util::stream_format(stream, "SBC     R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0c00:
		case 0x0d00:
		case 0x0e00:
		case 0x0f00:
			util::stream_format(stream, "ADD     R%d, R%d", RD5(op), RR5(op));
			break;
		}
		break;
	case 0x1000:
		switch(op & 0x0c00)
		{
		case 0x0000:
			util::stream_format(stream, "CPSE    R%d, R%d", RD5(op), RR5(op));
			flags = STEP_COND;
			break;
		case 0x0400:
			util::stream_format(stream, "CP      R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0800:
			util::stream_format(stream, "SUB     R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0c00:
			util::stream_format(stream, "ADC     R%d, R%d", RD5(op), RR5(op));
			break;
		}
		break;
	case 0x2000:
		switch(op & 0x0c00)
	{
		case 0x0000:
			util::stream_format(stream, "AND     R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0400:
			util::stream_format(stream, "EOR     R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0800:
			util::stream_format(stream, "OR      R%d, R%d", RD5(op), RR5(op));
			break;
		case 0x0c00:
			util::stream_format(stream, "MOV     R%d, R%d", RD5(op), RR5(op));
			break;
		}
		break;
	case 0x3000:
		util::stream_format(stream, "CPI     R%d, 0x%02x", 16+RD4(op), KCONST8(op));
		break;
	case 0x4000:
		util::stream_format(stream, "SBCI    R%d, 0x%02x", 16+RD4(op), KCONST8(op));
		break;
	case 0x5000:
		util::stream_format(stream, "SUBI    R%d, 0x%02x", 16+RD4(op), KCONST8(op));
		break;
	case 0x6000:
		util::stream_format(stream, "ORI     R%d, 0x%02x", 16+RD4(op), KCONST8(op));
		break;
	case 0x7000:
		util::stream_format(stream, "ANDI    R%d, 0x%02x", 16+RD4(op), KCONST8(op));
		break;
	case 0x8000:
	case 0xa000:
		switch(op & 0x0208)
		{
		case 0x0000:
			util::stream_format(stream, "LD(D)   R%d, Z+%02x", RD5(op), QCONST6(op));
			break;
		case 0x0008:
			util::stream_format(stream, "LD(D)   R%d, Y+%02x", RD5(op), QCONST6(op));
			break;
		case 0x0200:
			util::stream_format(stream, "ST(D)   Z+%02x, R%d", QCONST6(op), RD5(op));
			break;
		case 0x0208:
			util::stream_format(stream, "ST(D)   Y+%02x, R%d", QCONST6(op), RD5(op));
			break;
		}
		break;
	case 0x9000:
		switch(op & 0x0f00)
		{
		case 0x0000:
		case 0x0100:
			switch(op & 0x000f)
			{
			case 0x0000:
				op <<= 16;
				op |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "LDS     R%d, (0x%04x)", RD5(op >> 16), op & 0x0000ffff);
				break;
			case 0x0001:
				util::stream_format(stream, "LD      R%d, Z+", RD5(op));
				break;
			case 0x0002:
				util::stream_format(stream, "LD      R%d,-Z", RD5(op));
				break;
			case 0x0004:
				util::stream_format(stream, "LPM     R%d, Z", RD5(op));
				break;
			case 0x0005:
				util::stream_format(stream, "LPM     R%d, Z+", RD5(op));
				break;
			case 0x0006:
				util::stream_format(stream, "ELPM    R%d, Z", RD5(op));
				break;
			case 0x0007:
				util::stream_format(stream, "ELPM    R%d, Z+", RD5(op));
				break;
			case 0x0009:
				util::stream_format(stream, "LD      R%d, Y+", RD5(op));
				break;
			case 0x000a:
				util::stream_format(stream, "LD      R%d,-Y", RD5(op));
				break;
			case 0x000c:
				util::stream_format(stream, "LD      R%d, X", RD5(op));
				break;
			case 0x000d:
				util::stream_format(stream, "LD      R%d, X+", RD5(op));
				break;
			case 0x000e:
				util::stream_format(stream, "LD      R%d,-X", RD5(op));
				break;
			case 0x000f:
				util::stream_format(stream, "POP     R%d", RD5(op));
				break;
			default:
				util::stream_format(stream, "Undefined (%08x)", op);
				break;
			}
			break;
		case 0x0200:
		case 0x0300:
			switch(op & 0x000f)
			{
			case 0x0000:
				op <<= 16;
				op |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "STS     (0x%04x), R%d", op & 0x0000ffff, RD5(op >> 16));
				break;
			case 0x0001:
				util::stream_format(stream, "ST       Z+, R%d", RD5(op));
				break;
			case 0x0002:
				util::stream_format(stream, "ST      -Z , R%d", RD5(op));
				break;
			case 0x0009:
				util::stream_format(stream, "ST       Y+, R%d", RD5(op));
				break;
			case 0x000a:
				util::stream_format(stream, "ST      -Y , R%d", RD5(op));
				break;
			case 0x000c:
				util::stream_format(stream, "ST       X , R%d", RD5(op));
				break;
			case 0x000d:
				util::stream_format(stream, "ST       X+, R%d", RD5(op));
				break;
			case 0x000e:
				util::stream_format(stream, "ST      -X , R%d", RD5(op));
				break;
			case 0x000f:
				util::stream_format(stream, "PUSH    R%d", RD5(op));
				break;
			default:
				util::stream_format(stream, "Undefined (%08x)", op);
				break;
			}
			break;
		case 0x0400:
			switch(op & 0x000f)
			{
			case 0x0000:
				util::stream_format(stream, "COM     R%d", RD5(op));
				break;
			case 0x0001:
				util::stream_format(stream, "NEG     R%d", RD5(op));
				break;
			case 0x0002:
				util::stream_format(stream, "SWAP    R%d", RD5(op));
				break;
			case 0x0003:
				util::stream_format(stream, "INC     R%d", RD5(op));
				break;
			case 0x0005:
				util::stream_format(stream, "ASR     R%d", RD5(op));
				break;
			case 0x0006:
				util::stream_format(stream, "LSR     R%d", RD5(op));
				break;
			case 0x0007:
				util::stream_format(stream, "ROR     R%d", RD5(op));
				break;
			case 0x0008:
				switch(op & 0x00f0)
				{
				case 0x0000:
					util::stream_format(stream, "SEC");
					break;
				case 0x0010:
					util::stream_format(stream, "SEZ");
					break;
				case 0x0020:
					util::stream_format(stream, "SEN");
					break;
				case 0x0030:
					util::stream_format(stream, "SEV");
					break;
				case 0x0040:
					util::stream_format(stream, "SES");
					break;
				case 0x0050:
					util::stream_format(stream, "SEH");
					break;
				case 0x0060:
					util::stream_format(stream, "SET");
					break;
				case 0x0070:
					util::stream_format(stream, "SEI");
					break;
				case 0x0080:
					util::stream_format(stream, "CLC");
					break;
				case 0x0090:
					util::stream_format(stream, "CLZ");
					break;
				case 0x00a0:
					util::stream_format(stream, "CLN");
					break;
				case 0x00b0:
					util::stream_format(stream, "CLV");
					break;
				case 0x00c0:
					util::stream_format(stream, "CLS");
					break;
				case 0x00d0:
					util::stream_format(stream, "CLH");
					break;
				case 0x00e0:
					util::stream_format(stream, "CLT");
					break;
				case 0x00f0:
					util::stream_format(stream, "CLI");
					break;
				default:
					util::stream_format(stream, "Undefined (%08x)", op);
					break;
				}
				break;
			case 0x0009:
				switch(op & 0x00f0)
				{
				case 0x0000:
					util::stream_format(stream, "IJMP");
					break;
				case 0x0010:
					util::stream_format(stream, "EIJMP");
					break;
				default:
					util::stream_format(stream, "Undefined (%08x)", op);
					break;
				}
				break;
			case 0x000a:
				util::stream_format(stream, "DEC     R%d", RD5(op));
				break;
			case 0x000c:
			case 0x000d:
				addr = KCONST22(op) << 16;
				addr |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "JMP     0x%06x", addr << 1);
				break;
			case 0x000e:
			case 0x000f:
				addr = KCONST22(op) << 16;
				addr |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "CALL    0x%06x", addr << 1);
				flags = STEP_OVER;
				break;
			default:
				util::stream_format(stream, "Undefined (%08x)", op);
				break;
			}
			break;
		case 0x0500:
			switch(op & 0x000f)
			{
			case 0x0000:
				util::stream_format(stream, "COM     R%d", RD5(op));
				break;
			case 0x0001:
				util::stream_format(stream, "NEG     R%d", RD5(op));
				break;
			case 0x0002:
				util::stream_format(stream, "SWAP    R%d", RD5(op));
				break;
			case 0x0003:
				util::stream_format(stream, "INC     R%d", RD5(op));
				break;
			case 0x0005:
				util::stream_format(stream, "ASR     R%d", RD5(op));
				break;
			case 0x0006:
				util::stream_format(stream, "LSR     R%d", RD5(op));
				break;
			case 0x0007:
				util::stream_format(stream, "ROR     R%d", RD5(op));
				break;
			case 0x0008:
				switch(op & 0x00f0)
				{
				case 0x0000:
					util::stream_format(stream, "RET");
					flags = STEP_OUT;
					break;
				case 0x0010:
					util::stream_format(stream, "RETI");
					flags = STEP_OUT;
					break;
				case 0x0080:
					util::stream_format(stream, "SLEEP");
					break;
				case 0x0090:
					util::stream_format(stream, "BREAK");
					break;
				case 0x00a0:
					util::stream_format(stream, "WDR");
					break;
				case 0x00c0:
					util::stream_format(stream, "LPM");
					break;
				case 0x00d0:
					util::stream_format(stream, "ELPM");
					break;
				case 0x00e0:
					util::stream_format(stream, "SPM");
					break;
				case 0x00f0:
					util::stream_format(stream, "SPM     Z+");
					break;
				default:
					util::stream_format(stream, "Undefined (%08x)", op);
					break;
				}
				break;
			case 0x0009:
				switch(op & 0x00f0)
				{
				case 0x0000:
					util::stream_format(stream, "ICALL");
					flags = STEP_OVER;
					break;
				case 0x0010:
					util::stream_format(stream, "EICALL");
					flags = STEP_OVER;
					break;
				default:
					util::stream_format(stream, "Undefined (%08x)", op);
					break;
				}
				break;
			case 0x000a:
				util::stream_format(stream, "DEC     R%d", RD5(op));
				break;
			case 0x000c:
			case 0x000d:
				op <<= 16;
				op |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "JMP     0x%06x", KCONST22(op) << 1);
				break;
			case 0x000e:
			case 0x000f:
				op <<= 16;
				op |= opcodes.r16(pc);
				pc += 2;
				util::stream_format(stream, "CALL    0x%06x", KCONST22(op) << 1);
				flags = STEP_OVER;
				break;
			}
			break;
		case 0x0600:
			util::stream_format(stream, "ADIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op));
			break;
		case 0x0700:
			util::stream_format(stream, "SBIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op));
			break;
		case 0x0800:
			util::stream_format(stream, "CBI     %s, %s", register_names[ACONST5(op)], register_bit_names[ACONST5(op)][RR3(op)]);
			break;
		case 0x0900:
			util::stream_format(stream, "SBIC    %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)]);
			flags = STEP_COND;
			break;
		case 0x0a00:
			util::stream_format(stream, "SBI     %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)]);
			break;
		case 0x0b00:
			util::stream_format(stream, "SBIS    %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)]);
			flags = STEP_COND;
			break;
		case 0x0c00:
		case 0x0d00:
		case 0x0e00:
		case 0x0f00:
			util::stream_format(stream, "MUL     R%d, R%d", RD5(op), RR5(op));
			break;
		}
		break;
	case 0xb000:
		if(op & 0x0800)
			util::stream_format(stream, "OUT     %s, R%d", register_names[ACONST6(op)], RD5(op));
		else
			util::stream_format(stream, "IN      R%d, %s", RD5(op), register_names[ACONST6(op)]);
		break;
	case 0xc000:
		util::stream_format(stream, "RJMP    %08x", (((op & 0x0800) ? pc + ((op & 0x0fff) | 0xfffff000) : pc + 2 + (op & 0x0fff)) << 0));
		break;
	case 0xd000:
		util::stream_format(stream, "RCALL   %08x", (((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff)) << 1));
		flags = STEP_OVER;
		break;
	case 0xe000:
		util::stream_format(stream, "LDI     R%d, 0x%02x", 16 + RD4(op), KCONST8(op));
		break;
	case 0xf000:
		switch(op & 0x0c00)
		{
		case 0x0000:
			switch(op & 0x0007)
			{
			case 0x0000:
				util::stream_format(stream, "BRLO    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0001:
				util::stream_format(stream, "BREQ    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0002:
				util::stream_format(stream, "BRMI    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0003:
				util::stream_format(stream, "BRVS    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0004:
				util::stream_format(stream, "BRLT    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0005:
				util::stream_format(stream, "BRHS    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0006:
				util::stream_format(stream, "BRTS    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0007:
				util::stream_format(stream, "BRIE    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			}
			flags = STEP_COND;
			break;
		case 0x0400:
			switch(op & 0x0007)
			{
			case 0x0000:
				util::stream_format(stream, "BRSH    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0001:
				util::stream_format(stream, "BRNE    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0002:
				util::stream_format(stream, "BRPL    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0003:
				util::stream_format(stream, "BRVC    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0004:
				util::stream_format(stream, "BRGE    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0005:
				util::stream_format(stream, "BRHC    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0006:
				util::stream_format(stream, "BRTC    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			case 0x0007:
				util::stream_format(stream, "BRID    %08x", (((op & 0x0200) ? (KCONST7(op) | 0xff80) : KCONST7(op)) << 1));
				break;
			}
			flags = STEP_COND;
			break;
		case 0x0800:
			if(op & 0x0200)
				util::stream_format(stream, "BST     R%d, %d", RD5(op), RR3(op));
			else
				util::stream_format(stream, "BLD     R%d, %d", RD5(op), RR3(op));
			break;
		case 0x0c00:
			if(op & 0x0200)
				util::stream_format(stream, "SBRS    R%d, %d", RD5(op), RR3(op));
			else
				util::stream_format(stream, "SBRC    R%d, %d", RD5(op), RR3(op));
			flags = STEP_COND;
			break;
		}
		break;
	}

	return (pc - base_pc) | flags | SUPPORTED;
}
