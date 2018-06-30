// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek
#include "emu.h"
#include "scudspdasm.h"

const char *const scudsp_disassembler::ALU_Commands[] =
{
	"    ",     /* 0000 */
	"AND ",  /* 0001 */
	"OR  ",   /* 0010 */
	"XOR ",  /* 0011 */
	"ADD ",  /* 0100 */
	"SUB ",  /* 0101 */
	"AD2 ",  /* 0110 */
	"ALU?",  /* 0111 */
	"SR  ",   /* 1000 */
	"RR  ",   /* 1001 */
	"SL  ",   /* 1010 */
	"RL  ",   /* 1011 */
	"ALU?",  /* 1100 */
	"ALU?",  /* 1101 */
	"ALU?",  /* 1110 */
	"RL8 ",  /* 1111 */
};

const char *const scudsp_disassembler::X_Commands[] =
{
	"",             /* 000 */
	"",             /* 001 */   /* NOP? check instruction @ 0x0B */
	"MOV MUL,P",    /* 010 */
	"MOV %s,P",     /* 011 */
	"MOV %s,X",     /* 100 */
};

const char *const scudsp_disassembler::Y_Commands[] =
{
	"",             /* 000 */
	"CLR A",        /* 001 */
	"MOV ALU,A",    /* 010 */
	"MOV %s,A",     /* 011 */
	"MOV %s,Y",     /* 100 */
};

const char *const scudsp_disassembler::D1_Commands[] =
{
	"",                 /* 00 */
	"MOV %I8,%d",       /* 01 */
	"???",              /* 10 */
	"MOV %S,%d",        /* 11 */
};

const char *const scudsp_disassembler::SourceMemory[] =
{
	"M0",           /* 000 */
	"M1",           /* 001 */
	"M2",           /* 010 */
	"M3",           /* 011 */
	"MC0",          /* 100 */
	"MC1",          /* 101 */
	"MC2",          /* 110 */
	"MC3",          /* 111 */
};

const char *const scudsp_disassembler::SourceMemory2[] =
{
	"M0",           /* 0000 */
	"M1",           /* 0001 */
	"M2",           /* 0010 */
	"M3",           /* 0011 */
	"MC0",          /* 0100 */
	"MC1",          /* 0101 */
	"MC2",          /* 0110 */
	"MC3",          /* 0111 */
	"???",          /* 1000 */
	"ALL",          /* 1001 */
	"ALH",          /* 1010 */
	"???",          /* 1011 */
	"???",          /* 1100 */
	"???",          /* 1101 */
	"???",          /* 1110 */
	"???",          /* 1111 */
};

const char *const scudsp_disassembler::DestMemory[] =
{
	"MC0",          /* 0000 */
	"MC1",          /* 0001 */
	"MC2",          /* 0010 */
	"MC3",          /* 0011 */
	"RX",           /* 0100 */
	"PL",           /* 0101 */
	"RA0",          /* 0110 */
	"WA0",          /* 0111 */
	"???",          /* 1000 */
	"???",          /* 1001 */
	"LOP",          /* 1010 */
	"TOP",          /* 1011 */
	"CT0",          /* 1100 */
	"CT1",          /* 1101 */
	"CT2",          /* 1110 */
	"CT3",          /* 1111 */
};

const char *const scudsp_disassembler::DestDMAMemory[] =
{
	"M0",           /* 000 */
	"M1",           /* 001 */
	"M2",           /* 010 */
	"M3",           /* 011 */
	"PRG",          /* 100 */
	"???",          /* 101 */
	"???",          /* 110 */
	"???",          /* 111 */
};

const char *const scudsp_disassembler::MVI_Command[] =
{
	"MVI %I,%d",    /* 0 */
	"MVI %I,%d,%f", /* 1 */
};

const char *const scudsp_disassembler::JMP_Command[] =
{
	"JMP %IA",
	"JMP %f,%IA",
};

const char *const scudsp_disassembler::DMA_Command[] =
{
	"DMA%H%A D0,%M,%I",
	"DMA%H%A %s,D0,%I",
	"DMA%H%A D0,%M,%s",
	"DMA%H%A %s,D0,%s",
};


std::string scudsp_disassembler::scudsp_dasm_prefix( const char* format, uint32_t *data )
{
	std::string result;
	for ( ; *format; format++ )
	{
		if ( *format == '%' )
		{
			switch( *++format )
			{
				case 'H':
					if ( *data )
						result += 'H';
					break;
				case 'A':
					if ( *data == 0 )
						result += '0';
					else if ( *data != 1 )
						result += util::string_format("%d",  1 << (*data - 1) );
					break;
				case 's':
					result += SourceMemory[ *data & 0x7 ];
					break;
				case 'd':
					result += DestMemory[ *data & 0xf ];
					break;
				case 'S':
					result += SourceMemory2[ *data & 0xf ];
					break;
				case 'I':
					++format;
					if ( *format == '8' )
						result += util::string_format("#$%x", *data );
					else if ( *format == 'A' )
						result += util::string_format("$%X", *data );
					else
					{
						--format;
						result += util::string_format("#$%X", *data );
					}
					break;
				case 'f':
					if ( !(*data & 0x20) )
						result += 'N';
					switch( *data & 0xf )
					{
						case 0x3:
							result += "ZS";
							break;
						case 0x2:
							result += 'S';
							break;
						case 0x4:
							result += 'C';
							break;
						case 0x8:
							result += "T0";
							break;
						case 0x1:
							result += 'Z';
							break;
						default:
							result += '?';
							break;
					}
					break;
				case 'M':
					result += DestDMAMemory[ *data ];
					break;

			}
			data++;
		}
		else
			result += *format;
	}
	return result;
}

u32 scudsp_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t scudsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t op = opcodes.r32(pc);
	unsigned size = 1;
	uint32_t data[4];

	switch( op >> 30 )
	{
		case 0:
			if ( (op & 0x3F8E3000) == 0 )
			{
				util::stream_format(stream, "%-10s", "NOP");
				break;
			}

			/* ALU */
			util::stream_format(stream, "%s", ALU_Commands[ (op & 0x3c000000) >> 26]);

			/* X-Bus */
			data[0] = (op & 0x700000) >> 20;
			if ( op & 0x2000000 )
				stream << scudsp_dasm_prefix( X_Commands[ 4 ], data );

			stream << scudsp_dasm_prefix( X_Commands[ (op & 0x1800000) >> 23 ], data );

			data[0] = (op & 0x1C000 ) >> 14 ;
			if ( op & 0x80000 )
				stream << scudsp_dasm_prefix( Y_Commands[4], data );

			stream << scudsp_dasm_prefix( Y_Commands[ (op & 0x60000) >> 17 ], data );

			/* D1-Bus */
			switch( (op & 0x3000) >> 12 )
			{
				case 0x1:
					data[0] = (op & 0xFF);
					data[1] = ((op & 0xF00) >> 8);
					break;
				case 0x3:
					data[0] = (op & 0xF);
					data[1] = ((op & 0xF00) >> 8);
					break;
			}

			stream << scudsp_dasm_prefix( D1_Commands[ (op & 0x3000) >> 12 ], data);
			break;
		case 2:
			if ( (op & 0x2000000) )
			{
				data[0] = op & 0x7FFFF;
				data[1] = (op & 0x3C000000) >> 26;
				data[2] = (op & 0x3F80000 ) >> 19;
				stream << scudsp_dasm_prefix( MVI_Command[1], data); /* TODO: bad mem*/
			}
			else
			{
				data[0] = op & 0x1FFFFFF;
				data[1] = (op & 0x3C000000) >> 26;
				stream << scudsp_dasm_prefix( MVI_Command[0], data ); /* TODO: bad mem*/
			}
			break;
		case 3:
			switch((op >> 28) & 3)
			{
				case 0:
					data[0] = (op &  0x4000) >> 14; /* H */
					data[1] = (op & 0x38000) >> 15; /* A */
					data[2] = (op & 0x700) >> 8; /* Mem */
					data[3] = (op & 0xff);
					stream << scudsp_dasm_prefix( DMA_Command[(op & 0x3000) >> 12], data );
					break;
				case 1:
					if ( op & 0x3F80000 )
					{
						data[0] = (op & 0x3F80000) >> 19;
						data[1] = op & 0xff;
						stream << scudsp_dasm_prefix( JMP_Command[1], data );
					}
					else
					{
						data[0] = op & 0xff;
						stream << scudsp_dasm_prefix( JMP_Command[0], data );
					}
					break;
				case 2:
					stream << (op & 0x8000000 ? "LPS" : "BTM");
					break;
				case 3:
					stream << (op & 0x8000000 ? "ENDI" : "END");
					break;
			}
			break;

		default:
			stream << "???";
			break;
	}

	return size;
}
