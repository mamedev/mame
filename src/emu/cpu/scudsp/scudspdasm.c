// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek
#include "emu.h"
#include "debugger.h"
#include "scudsp.h"

static const char *const ALU_Commands[] =
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

static const char *const X_Commands[] =
{
	"",             /* 000 */
	"",             /* 001 */   /* NOP? check instruction @ 0x0B */
	"MOV MUL,P",    /* 010 */
	"MOV %s,P",     /* 011 */
	"MOV %s,X",     /* 100 */
};

static const char *const Y_Commands[] =
{
	"",             /* 000 */
	"CLR A",        /* 001 */
	"MOV ALU,A",    /* 010 */
	"MOV %s,A",     /* 011 */
	"MOV %s,Y",     /* 100 */
};

static const char *const D1_Commands[] =
{
	"",                 /* 00 */
	"MOV %I8,%d",       /* 01 */
	"???",              /* 10 */
	"MOV %S,%d",        /* 11 */
};

static const char *const SourceMemory[] =
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

static const char *const SourceMemory2[] =
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

static const char *const DestMemory[] =
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

static const char *const DestDMAMemory[] =
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

static const char *const MVI_Command[] =
{
	"MVI %I,%d",    /* 0 */
	"MVI %I,%d,%f", /* 1 */
};

static const char *const JMP_Command[] =
{
	"JMP %IA",
	"JMP %f,%IA",
};

static const char *const DMA_Command[] =
{
	"DMA%H%A D0,%M,%I",
	"DMA%H%A %s,D0,%I",
	"DMA%H%A D0,%M,%s",
	"DMA%H%A %s,D0,%s",
};


static void scudsp_dasm_prefix( const char* format, char* buffer, UINT32 *data )
{
	for ( ; *format; format++ )
	{
		if ( *format == '%' )
		{
			switch( *++format )
			{
				case 'H':
					if ( *data )
					{
						strcpy( buffer, "H" );
					}
					else
					{
						*buffer = 0;
					}
					break;
				case 'A':
					if ( *data == 0 )
					{
						strcpy( buffer, "0" );
					}
					else if ( *data == 1 )
					{
						*buffer = 0;
					}
					else
					{
						sprintf( buffer, "%d", 1 << (*data - 1) );
					}
					break;
				case 's':
					strcpy( buffer, SourceMemory[ *data & 0x7 ] );
					break;
				case 'd':
					strcpy( buffer, DestMemory[ *data & 0xf ] );
					break;
				case 'S':
					strcpy( buffer, SourceMemory2[ *data & 0xf ] );
					break;
				case 'I':
					++format;
					if ( *format == '8' )
					{
						sprintf( buffer, "#$%x", *data );
					}
					else if ( *format == 'A' )
					{
						sprintf( buffer, "$%X", *data );
					}
					else
					{
						--format;
						sprintf( buffer, "#$%X", *data );
					}
					break;
				case 'f':
					if ( !(*data & 0x20) )
					{
						strcpy( buffer, "N" );
						buffer++;
					}
					switch( *data & 0xf )
					{
						case 0x3:
							strcpy( buffer, "ZS" );
							break;
						case 0x2:
							strcpy( buffer, "S" );
							break;
						case 0x4:
							strcpy( buffer, "C" );
							break;
						case 0x8:
							strcpy( buffer, "T0" );
							break;
						case 0x1:
							strcpy( buffer, "Z" );
							break;
						default:
							strcpy( buffer, "?" );
							break;
					}
					break;
				case 'M':
					strcpy( buffer, DestDMAMemory[ *data ] );
					break;

			}
			data++;
			buffer += strlen( buffer );
		}
		else
		{
			*buffer++ = *format;
		}
	}
	*buffer = 0;
}


CPU_DISASSEMBLE( scudsp )
{
	UINT32 op = oprom[0]<<24|oprom[1]<<16|oprom[2]<<8|oprom[3]<<0;
	unsigned size = 1;
//  const char *sym, *sym2;
	char *my_buffer = buffer;
	char temp_buffer[64];
	UINT32 data[4];

	switch( op >> 30 )
	{
		case 0:
			if ( (op & 0x3F8E3000) == 0 )
			{
				sprintf( buffer, "%-10s", "NOP" );
				break;
			}

			/* ALU */
			sprintf(my_buffer, "%s", ALU_Commands[ (op & 0x3c000000) >> 26] );
			my_buffer += strlen( my_buffer );

			/* X-Bus */
			data[0] = (op & 0x700000) >> 20;
			if ( op & 0x2000000 )
			{
				scudsp_dasm_prefix( X_Commands[ 4 ], temp_buffer, data );
			}
			else
			{
				*temp_buffer = 0;
			}
			sprintf( my_buffer, "%s", temp_buffer );
			my_buffer += strlen( my_buffer );

			scudsp_dasm_prefix( X_Commands[ (op & 0x1800000) >> 23 ], temp_buffer,  data );
			sprintf( my_buffer, "%s", temp_buffer );
			my_buffer += strlen( my_buffer );

			data[0] = (op & 0x1C000 ) >> 14 ;
			if ( op & 0x80000 )
			{
				scudsp_dasm_prefix( Y_Commands[4], temp_buffer, data );
			}
			else
			{
				*temp_buffer = 0;
			}
			sprintf( my_buffer, "%s", temp_buffer );
			my_buffer += strlen( my_buffer );

			scudsp_dasm_prefix( Y_Commands[ (op & 0x60000) >> 17 ], temp_buffer,  data );
			sprintf( my_buffer, "%s", temp_buffer );
			my_buffer += strlen( my_buffer );

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

			scudsp_dasm_prefix( D1_Commands[ (op & 0x3000) >> 12 ], temp_buffer, data );
			sprintf( my_buffer, "%s", temp_buffer );
			break;
		case 2:
			if ( (op & 0x2000000) )
			{
				data[0] = op & 0x7FFFF;
				data[1] = (op & 0x3C000000) >> 26;
				data[2] = (op & 0x3F80000 ) >> 19;
				scudsp_dasm_prefix( MVI_Command[1], buffer, data ); /* TODO: bad mem*/
			}
			else
			{
				data[0] = op & 0x1FFFFFF;
				data[1] = (op & 0x3C000000) >> 26;
				scudsp_dasm_prefix( MVI_Command[0], buffer, data ); /* TODO: bad mem*/
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
					scudsp_dasm_prefix( DMA_Command[(op & 0x3000) >> 12], buffer, data );
					break;
				case 1:
					if ( op & 0x3F80000 )
					{
						data[0] = (op & 0x3F80000) >> 19;
						data[1] = op & 0xff;
						scudsp_dasm_prefix( JMP_Command[1], buffer, data );
					}
					else
					{
						data[0] = op & 0xff;
						scudsp_dasm_prefix( JMP_Command[0], buffer, data );
					}
					break;
				case 2:
					sprintf(buffer, op & 0x8000000 ? "LPS" : "BTM");
					break;
				case 3:
					sprintf(buffer, op & 0x8000000 ? "ENDI" : "END");
					break;
			}
			break;

		default:
			sprintf(buffer, "???");
			break;
	}

	return size;
}
