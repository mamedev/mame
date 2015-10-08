// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR disassembler

    Written by Ryan Holtz
*/

#include "emu.h"
#include "avr8.h"

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
#define KCONST22(op)    (((((UINT32)(op) >> 3) & 0x003e) | ((UINT32)(op) & 0x0001)) << 16)
#define QCONST6(op)     ((((op) >> 8) & 0x0020) | (((op) >> 7) & 0x0018) | ((op) & 0x0007))
#define ACONST5(op)     (((op) >> 3) & 0x001f)
#define ACONST6(op)     ((((op) >> 5) & 0x0030) | ((op) & 0x000f))
#define MULCONST2(op)   ((((op) >> 6) & 0x0002) | (((op) >> 3) & 0x0001))

CPU_DISASSEMBLE( avr8 )
{
	char *output = buffer;
	int pos = 0;
	UINT32 op = oprom[pos++];
	op |= oprom[pos++] << 8;
	UINT32 addr = 0;
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

	switch(op & 0xf000)
	{
		case 0x0000:
			switch(op & 0x0f00)
			{
				case 0x0000:
					output += sprintf( output, "NOP" );
					break;
				case 0x0100:
					output += sprintf( output, "MOVW    R%d:R%d, R%d:R%d", (RD4(op) << 1)+1, RD4(op) << 1, (RR4(op) << 1)+1, RR4(op) << 1 );
					break;
				case 0x0200:
					output += sprintf( output, "MULS    R%d, R%d", 16+RD4(op), 16+RR4(op) );
					break;
				case 0x0300:
					switch(MULCONST2(op))
					{
						case 0:
							output += sprintf( output, "MULSU   R%d, R%d", 16+RD3(op), 16+RR3(op) );
							break;
						case 1:
							output += sprintf( output, "FMUL    R%d, R%d", 16+RD3(op), 16+RR3(op) );
							break;
						case 2:
							output += sprintf( output, "FMULS   R%d, R%d", 16+RD3(op), 16+RR3(op) );
							break;
						case 3:
							output += sprintf( output, "FMULSU  R%d, R%d", 16+RD3(op), 16+RR3(op) );
							break;
					}
					break;
				case 0x0400:
				case 0x0500:
				case 0x0600:
				case 0x0700:
					output += sprintf( output, "CPC     R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0800:
				case 0x0900:
				case 0x0a00:
				case 0x0b00:
					output += sprintf( output, "SBC     R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0c00:
				case 0x0d00:
				case 0x0e00:
				case 0x0f00:
					output += sprintf( output, "ADD     R%d, R%d", RD5(op), RR5(op) );
					break;
			}
			break;
		case 0x1000:
			switch(op & 0x0c00)
			{
				case 0x0000:
					output += sprintf( output, "CPSE    R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0400:
					output += sprintf( output, "CP      R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0800:
					output += sprintf( output, "SUB     R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0c00:
					output += sprintf( output, "ADC     R%d, R%d", RD5(op), RR5(op) );
					break;
			}
			break;
		case 0x2000:
			switch(op & 0x0c00)
			{
				case 0x0000:
					output += sprintf( output, "AND     R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0400:
					output += sprintf( output, "EOR     R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0800:
					output += sprintf( output, "OR      R%d, R%d", RD5(op), RR5(op) );
					break;
				case 0x0c00:
					output += sprintf( output, "MOV     R%d, R%d", RD5(op), RR5(op) );
					break;
			}
			break;
		case 0x3000:
			output += sprintf( output, "CPI     R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
			break;
		case 0x4000:
			output += sprintf( output, "SBCI    R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
			break;
		case 0x5000:
				output += sprintf( output, "SUBI    R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
			break;
		case 0x6000:
				output += sprintf( output, "ORI     R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
			break;
		case 0x7000:
				output += sprintf( output, "ANDI    R%d, 0x%02x", 16+RD4(op), KCONST8(op) );
			break;
		case 0x8000:
		case 0xa000:
			switch(op & 0x0208)
			{
				case 0x0000:
					output += sprintf( output, "LD(D)   R%d, Z+%02x", RD5(op), QCONST6(op) );
					break;
				case 0x0008:
					output += sprintf( output, "LD(D)   R%d, Y+%02x", RD5(op), QCONST6(op) );
					break;
				case 0x0200:
					output += sprintf( output, "ST(D)   Z+%02x, R%d", QCONST6(op), RD5(op) );
					break;
				case 0x0208:
					output += sprintf( output, "ST(D)   Y+%02x, R%d", QCONST6(op), RD5(op) );
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
							op |= oprom[pos++];
							op |= oprom[pos++] << 8;
							output += sprintf( output, "LDS     R%d, (0x%04x)", RD5(op >> 16), op & 0x0000ffff );
							break;
						case 0x0001:
							output += sprintf( output, "LD      R%d, Z+", RD5(op) );
							break;
						case 0x0002:
							output += sprintf( output, "LD      R%d,-Z", RD5(op) );
							break;
						case 0x0004:
							output += sprintf( output, "LPM     R%d, Z", RD5(op) );
							break;
						case 0x0005:
							output += sprintf( output, "LPM     R%d, Z+", RD5(op) );
							break;
						case 0x0006:
							output += sprintf( output, "ELPM    R%d, Z", RD5(op) );
							break;
						case 0x0007:
							output += sprintf( output, "ELPM    R%d, Z+", RD5(op) );
							break;
						case 0x0009:
							output += sprintf( output, "LD      R%d, Y+", RD5(op) );
							break;
						case 0x000a:
							output += sprintf( output, "LD      R%d,-Y", RD5(op) );
							break;
						case 0x000c:
							output += sprintf( output, "LD      R%d, X", RD5(op) );
							break;
						case 0x000d:
							output += sprintf( output, "LD      R%d, X+", RD5(op) );
							break;
						case 0x000e:
							output += sprintf( output, "LD      R%d,-X", RD5(op) );
							break;
						case 0x000f:
							output += sprintf( output, "POP     R%d", RD5(op) );
							break;
						default:
							output += sprintf( output, "Undefined (%08x)", op );
							break;
					}
					break;
				case 0x0200:
				case 0x0300:
					switch(op & 0x000f)
					{
						case 0x0000:
							op <<= 16;
							op |= oprom[pos++];
							op |= oprom[pos++] << 8;
							output += sprintf( output, "STS     (0x%04x), R%d", op & 0x0000ffff, RD5(op >> 16) );
							break;
						case 0x0001:
							output += sprintf( output, "ST       Z+, R%d", RD5(op) );
							break;
						case 0x0002:
							output += sprintf( output, "ST      -Z , R%d", RD5(op) );
							break;
						case 0x0009:
							output += sprintf( output, "ST       Y+, R%d", RD5(op) );
							break;
						case 0x000a:
							output += sprintf( output, "ST      -Y , R%d", RD5(op) );
							break;
						case 0x000c:
							output += sprintf( output, "ST       X , R%d", RD5(op) );
							break;
						case 0x000d:
							output += sprintf( output, "ST       X+, R%d", RD5(op) );
							break;
						case 0x000e:
							output += sprintf( output, "ST      -X , R%d", RD5(op) );
							break;
						case 0x000f:
							output += sprintf( output, "PUSH    R%d", RD5(op) );
							break;
						default:
							output += sprintf( output, "Undefined (%08x)", op );
							break;
					}
					break;
				case 0x0400:
					switch(op & 0x000f)
					{
						case 0x0000:
							output += sprintf( output, "COM     R%d", RD5(op) );
							break;
						case 0x0001:
							output += sprintf( output, "NEG     R%d", RD5(op) );
							break;
						case 0x0002:
							output += sprintf( output, "SWAP    R%d", RD5(op) );
							break;
						case 0x0003:
							output += sprintf( output, "INC     R%d", RD5(op) );
							break;
						case 0x0005:
							output += sprintf( output, "ASR     R%d", RD5(op) );
							break;
						case 0x0006:
							output += sprintf( output, "LSR     R%d", RD5(op) );
							break;
						case 0x0007:
							output += sprintf( output, "ROR     R%d", RD5(op) );
							break;
						case 0x0008:
							switch(op & 0x00f0)
							{
								case 0x0000:
									output += sprintf( output, "SEC" );
									break;
								case 0x0010:
									output += sprintf( output, "SEZ" );
									break;
								case 0x0020:
									output += sprintf( output, "SEN" );
									break;
								case 0x0030:
									output += sprintf( output, "SEV" );
									break;
								case 0x0040:
									output += sprintf( output, "SES" );
									break;
								case 0x0050:
									output += sprintf( output, "SEH" );
									break;
								case 0x0060:
									output += sprintf( output, "SET" );
									break;
								case 0x0070:
									output += sprintf( output, "SEI" );
									break;
								case 0x0080:
									output += sprintf( output, "CLC" );
									break;
								case 0x0090:
									output += sprintf( output, "CLZ" );
									break;
								case 0x00a0:
									output += sprintf( output, "CLN" );
									break;
								case 0x00b0:
									output += sprintf( output, "CLV" );
									break;
								case 0x00c0:
									output += sprintf( output, "CLS" );
									break;
								case 0x00d0:
									output += sprintf( output, "CLH" );
									break;
								case 0x00e0:
									output += sprintf( output, "CLT" );
									break;
								case 0x00f0:
									output += sprintf( output, "CLI" );
									break;
								default:
									output += sprintf( output, "Undefined (%08x)", op );
									break;
							}
							break;
						case 0x0009:
							switch(op & 0x00f0)
							{
								case 0x0000:
									output += sprintf( output, "IJMP" );
									break;
								case 0x0010:
									output += sprintf( output, "EIJMP" );
									break;
								default:
									output += sprintf( output, "Undefined (%08x)", op );
									break;
							}
							break;
						case 0x000a:
							output += sprintf( output, "DEC     R%d", RD5(op) );
							break;
						case 0x000c:
						case 0x000d:
							addr = KCONST22(op) << 16;
							addr |= oprom[pos++];
							addr |= oprom[pos++] << 8;
							output += sprintf( output, "JMP     0x%06x", addr << 1 );
							break;
						case 0x000e:
						case 0x000f:
							addr = KCONST22(op) << 16;
							addr |= oprom[pos++];
							addr |= oprom[pos++] << 8;
							output += sprintf( output, "CALL    0x%06x", addr << 1 );
							break;
						default:
							output += sprintf( output, "Undefined (%08x)", op );
							break;
					}
					break;
				case 0x0500:
					switch(op & 0x000f)
					{
						case 0x0000:
							output += sprintf( output, "COM     R%d", RD5(op) );
							break;
						case 0x0001:
							output += sprintf( output, "NEG     R%d", RD5(op) );
							break;
						case 0x0002:
							output += sprintf( output, "SWAP    R%d", RD5(op) );
							break;
						case 0x0003:
							output += sprintf( output, "INC     R%d", RD5(op) );
							break;
						case 0x0005:
							output += sprintf( output, "ASR     R%d", RD5(op) );
							break;
						case 0x0006:
							output += sprintf( output, "LSR     R%d", RD5(op) );
							break;
						case 0x0007:
							output += sprintf( output, "ROR     R%d", RD5(op) );
							break;
						case 0x0008:
							switch(op & 0x00f0)
							{
								case 0x0000:
									output += sprintf( output, "RET" );
									break;
								case 0x0010:
									output += sprintf( output, "RETI" );
									break;
								case 0x0080:
									output += sprintf( output, "SLEEP" );
									break;
								case 0x0090:
									output += sprintf( output, "BREAK" );
									break;
								case 0x00a0:
									output += sprintf( output, "WDR" );
									break;
								case 0x00c0:
									output += sprintf( output, "LPM" );
									break;
								case 0x00d0:
									output += sprintf( output, "ELPM" );
									break;
								case 0x00e0:
									output += sprintf( output, "SPM" );
									break;
								case 0x00f0:
									output += sprintf( output, "SPM     Z+" );
									break;
								default:
									output += sprintf( output, "Undefined (%08x)", op );
									break;
							}
							break;
						case 0x0009:
							switch(op & 0x00f0)
							{
								case 0x0000:
									output += sprintf( output, "ICALL" );
									break;
								case 0x0010:
									output += sprintf( output, "EICALL" );
									break;
								default:
									output += sprintf( output, "Undefined (%08x)", op );
									break;
							}
							break;
						case 0x000a:
							output += sprintf( output, "DEC     R%d", RD5(op) );
							break;
						case 0x000c:
						case 0x000d:
							op <<= 16;
							op |= oprom[pos++];
							op |= oprom[pos++] << 8;
							output += sprintf( output, "JMP     0x%06x", KCONST22(op) << 1 );
							break;
						case 0x000e:
						case 0x000f:
							op <<= 16;
							op |= oprom[pos++];
							op |= oprom[pos++] << 8;
							output += sprintf( output, "CALL    0x%06x", KCONST22(op) << 1 );
							break;
					}
					break;
				case 0x0600:
					output += sprintf( output, "ADIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
					break;
				case 0x0700:
					output += sprintf( output, "SBIW    R%d:R%d, 0x%02x", 24+(RD2(op) << 1)+1, 24+(RD2(op) << 1), KCONST6(op) );
					break;
				case 0x0800:
			if (ACONST5(op) < 0x20)
			output += sprintf( output, "CBI     %s, %s", register_names[ACONST5(op)], register_bit_names[ACONST5(op)][RR3(op)] );
			else
			output += sprintf( output, "CBI     0x%02x, %d", ACONST5(op), RR3(op) );
					break;
				case 0x0900:
			if (ACONST5(op) < 0x20)
			output += sprintf( output, "SBIC    %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)] );
			else
			output += sprintf( output, "SBIC    0x%02x, %d", ACONST5(op), RR3(op) );
					break;
				case 0x0a00:
			if (ACONST5(op) < 0x20)
			output += sprintf( output, "SBI     %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)] );
			else
			output += sprintf( output, "SBI     0x%02x, %d", ACONST5(op), RR3(op) );
					break;
				case 0x0b00:
			if (ACONST5(op) < 0x20)
			output += sprintf( output, "SBIS    %s, %s", register_names[ACONST5(op)],  register_bit_names[ACONST5(op)][RR3(op)] );
			else
			output += sprintf( output, "SBIS    0x%02x, %d", ACONST5(op), RR3(op) );
					break;
				case 0x0c00:
				case 0x0d00:
				case 0x0e00:
				case 0x0f00:
					output += sprintf( output, "MUL     R%d, R%d", RD5(op), RR5(op) );
					break;
			}
			break;
		case 0xb000:
			if(op & 0x0800)
			{
		if (ACONST6(op) < 0x40 ) {
			output += sprintf( output, "OUT     %s, R%d", register_names[ACONST6(op)], RD5(op) );
		} else {
			output += sprintf( output, "OUT     0x%02x, R%d", ACONST6(op), RD5(op) );
		}
			}
			else
			{
		if (ACONST6(op) < 0x40 ) {
			output += sprintf( output, "IN      R%d, %s", RD5(op), register_names[ACONST6(op)] );
		} else {
			output += sprintf( output, "IN      R%d, 0x%02x", RD5(op), ACONST6(op) );
		}
			}
			break;
		case 0xc000:
		//I'm not sure if this is correct. why pc + ... : pc + 8 + ... ?
			output += sprintf( output, "RJMP    %08x", (((op & 0x0800) ? pc + ((op & 0x0fff) | 0xfffff000) : pc + 8 + (op & 0x0fff)) << 0) );
			break;
		case 0xd000:
			output += sprintf( output, "RCALL   %08x", (((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff)) << 1) );
			break;
		case 0xe000:
			output += sprintf( output, "LDI     R%d, 0x%02x", 16 + RD4(op), KCONST8(op) );
			break;
		case 0xf000:
			switch(op & 0x0c00)
			{
				case 0x0000:
					switch(op & 0x0007)
					{
						case 0x0000:
							output += sprintf( output, "BRLO    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0001:
							output += sprintf( output, "BREQ    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0002:
							output += sprintf( output, "BRMI    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0003:
							output += sprintf( output, "BRVS    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0004:
							output += sprintf( output, "BRLT    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0005:
							output += sprintf( output, "BRHS    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0006:
							output += sprintf( output, "BRTS    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0007:
							output += sprintf( output, "BRIE    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
					}
					break;
				case 0x0400:
					switch(op & 0x0007)
					{
						case 0x0000:
							output += sprintf( output, "BRSH    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0001:
							output += sprintf( output, "BRNE    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0002:
							output += sprintf( output, "BRPL    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0003:
							output += sprintf( output, "BRVC    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0004:
							output += sprintf( output, "BRGE    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0005:
							output += sprintf( output, "BRHC    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0006:
							output += sprintf( output, "BRTC    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
						case 0x0007:
							output += sprintf( output, "BRID    %08x", (((op & 0x0200) ? ((KCONST7(op) & 0x007f) | 0xff80) : KCONST7(op)) << 1) );
							break;
					}
					break;
				case 0x0800:
					if(op & 0x0200)
					{
						output += sprintf( output, "BST     R%d, %d", RD5(op), RR3(op) );
					}
					else
					{
						output += sprintf( output, "BLD     R%d, %d", RD5(op), RR3(op) );
					}
					break;
				case 0x0c00:
					if(op & 0x0200)
					{
						output += sprintf( output, "SBRS    R%d, %d", RD5(op), RR3(op) );
					}
					else
					{
						output += sprintf( output, "SBRC    R%d, %d", RD5(op), RR3(op) );
					}
					break;
			}
			break;
	}

	return pos | DASMFLAG_SUPPORTED;
}
