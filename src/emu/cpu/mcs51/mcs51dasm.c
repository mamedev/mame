// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff
/*****************************************************************************
 *
 *   i8051dasm.c
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************
 * Symbol Memory Name Tables borrowed from:
 * D52 8052 Disassembler - Copyright Jeffery L. Post
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mcs51.h"

enum
{
	FEATURE_NONE            = 0x00,
	FEATURE_I8052           = 0x01,
	FEATURE_CMOS            = 0x02,
	FEATURE_I80C52          = 0x04,
	FEATURE_DS5002FP        = 0x08,
	FEATURE_I83C751         = 0x08
};


#define SHOW_MEMORY_NAMES   1

#ifdef SHOW_MEMORY_NAMES

/*Display the memory address names for data & bit address access*/

//SFR Names

static const struct {
	int feature;
	int addr;
	const char *name;
} mem_name_feature[] =
{
	{ FEATURE_NONE, 0x00, "rb0r0" },
	{ FEATURE_NONE, 0x01, "rb0r1" },
	{ FEATURE_NONE, 0x02, "rb0r2" },
	{ FEATURE_NONE, 0x03, "rb0r3" },
	{ FEATURE_NONE, 0x04, "rb0r4" },
	{ FEATURE_NONE, 0x05, "rb0r5" },
	{ FEATURE_NONE, 0x06, "rb0r6" },
	{ FEATURE_NONE, 0x07, "rb0r7" },
	{ FEATURE_NONE, 0x08, "rb1r0" },
	{ FEATURE_NONE, 0x09, "rb1r1" },
	{ FEATURE_NONE, 0x0a, "rb1r2" },
	{ FEATURE_NONE, 0x0b, "rb1r3" },
	{ FEATURE_NONE, 0x0c, "rb1r4" },
	{ FEATURE_NONE, 0x0d, "rb1r5" },
	{ FEATURE_NONE, 0x0e, "rb1r6" },
	{ FEATURE_NONE, 0x0f, "rb1r7" },
	{ FEATURE_NONE, 0x10, "rb2r0" },
	{ FEATURE_NONE, 0x11, "rb2r1" },
	{ FEATURE_NONE, 0x12, "rb2r2" },
	{ FEATURE_NONE, 0x13, "rb2r3" },
	{ FEATURE_NONE, 0x14, "rb2r4" },
	{ FEATURE_NONE, 0x15, "rb2r5" },
	{ FEATURE_NONE, 0x16, "rb2r6" },
	{ FEATURE_NONE, 0x17, "rb2r7" },
	{ FEATURE_NONE, 0x18, "rb3r0" },
	{ FEATURE_NONE, 0x19, "rb3r1" },
	{ FEATURE_NONE, 0x1a, "rb3r2" },
	{ FEATURE_NONE, 0x1b, "rb3r3" },
	{ FEATURE_NONE, 0x1c, "rb3r4" },
	{ FEATURE_NONE, 0x1d, "rb3r5" },
	{ FEATURE_NONE, 0x1e, "rb3r6" },
	{ FEATURE_NONE, 0x1f, "rb3r7" },

	{ FEATURE_NONE, 0x80, "p0"      },
	{ FEATURE_NONE, 0x81, "sp"      },
	{ FEATURE_NONE, 0x82, "dpl"     },
	{ FEATURE_NONE, 0x83, "dph"     },
	{ FEATURE_NONE, 0x87, "pcon"    },
	{ FEATURE_NONE, 0x88, "tcon"    },
	{ FEATURE_NONE, 0x89, "tmod"    },
	{ FEATURE_NONE, 0x8a, "tl0"     },
	{ FEATURE_NONE, 0x8b, "tl1"     },
	{ FEATURE_NONE, 0x8c, "th0"     },
	{ FEATURE_NONE, 0x8d, "th1"     },
	{ FEATURE_NONE, 0x90, "p1"      },
	{ FEATURE_NONE, 0x98, "scon"    },
	{ FEATURE_NONE, 0x99, "sbuf"    },
	{ FEATURE_NONE, 0xa0, "p2"      },
	{ FEATURE_NONE, 0xa8, "ie"      },
	{ FEATURE_NONE, 0xb0, "p3"      },
	{ FEATURE_NONE, 0xb8, "ip"      },
	{ FEATURE_NONE, 0xd0, "psw"     },
	{ FEATURE_NONE, 0xe0, "acc"     },
	{ FEATURE_NONE, 0xf0, "b"       },

	{ FEATURE_I8052, 0xc8, "t2con"  },
	{ FEATURE_I8052, 0xca, "rcap2l" },
	{ FEATURE_I8052, 0xcb, "rcap2h" },
	{ FEATURE_I8052, 0xcc, "tl2"    },
	{ FEATURE_I8052, 0xcd, "th2"    },

	{ FEATURE_I80C52, 0xb7, "iph"   },
	{ FEATURE_I80C52, 0xa9, "saddr" },
	{ FEATURE_I80C52, 0xb9, "saden" },

	{ FEATURE_DS5002FP, 0x8e, "pwcm"    },
	{ FEATURE_DS5002FP, 0x8f, "pwmp"    },
	{ FEATURE_DS5002FP, 0xc1, "crcr"    },
	{ FEATURE_DS5002FP, 0xc2, "crcl"    },
	{ FEATURE_DS5002FP, 0xc3, "crch"    },
	{ FEATURE_DS5002FP, 0xc6, "mcon"    },
	{ FEATURE_DS5002FP, 0xc7, "ta"      },
	{ FEATURE_DS5002FP, 0xcf, "rnr"     },
	{ FEATURE_DS5002FP, 0xd8, "rpctl"   },
	{ FEATURE_DS5002FP, 0xd9, "rps"     },

	{ FEATURE_I83C751,  0x98, "i2con"   },
	{ FEATURE_I83C751,  0x99, "i2dat"   },
	{ FEATURE_I83C751,  0xd8, "i2cfg"   },
	{ FEATURE_I83C751,  0xf8, "i2sta"   },  /* read only */

	/* bit addresses */

	{ FEATURE_NONE, 0x188, "it0"        },
	{ FEATURE_NONE, 0x189, "ie0"        },
	{ FEATURE_NONE, 0x18a, "it1"        },
	{ FEATURE_NONE, 0x18b, "ie1"        },
	{ FEATURE_NONE, 0x18c, "tr0"        },
	{ FEATURE_NONE, 0x18d, "tf0"        },
	{ FEATURE_NONE, 0x18e, "tr1"        },
	{ FEATURE_NONE, 0x18f, "tf1"        },

	{ FEATURE_NONE, 0x198, "ri"         },
	{ FEATURE_NONE, 0x199, "ti"         },
	{ FEATURE_NONE, 0x19a, "rb8"        },
	{ FEATURE_NONE, 0x19b, "tb8"        },
	{ FEATURE_NONE, 0x19c, "ren"        },
	{ FEATURE_NONE, 0x19d, "sm2"        },
	{ FEATURE_NONE, 0x19e, "sm1"        },
	{ FEATURE_NONE, 0x19f, "sm0"        },

	{ FEATURE_I83C751, 0x198, "xstp"        }, /* read: no function */
	{ FEATURE_I83C751, 0x199, "xstr"        }, /* read: MASTER */
	{ FEATURE_I83C751, 0x19a, "cstp"        }, /* read: STP */
	{ FEATURE_I83C751, 0x19b, "cstr"        }, /* read: STR */
	{ FEATURE_I83C751, 0x19c, "carl"        }, /* read: ARL */
	{ FEATURE_I83C751, 0x19d, "cdr"     }, /* read: DRDY */
	{ FEATURE_I83C751, 0x19e, "idle"        }, /* read: ATN */
	{ FEATURE_I83C751, 0x19f, "cxa"     }, /* read: RDAT */

	{ FEATURE_NONE, 0x1a8, "ex0"        },
	{ FEATURE_NONE, 0x1a9, "et0"        },
	{ FEATURE_NONE, 0x1aa, "ex1"        },
	{ FEATURE_NONE, 0x1ab, "et1"        },
	{ FEATURE_NONE, 0x1ac, "es"         },
	{ FEATURE_NONE, 0x1ad, "ie.5"       },
	{ FEATURE_NONE, 0x1ae, "ie.6"       },
	{ FEATURE_NONE, 0x1af, "ea"         },

	{ FEATURE_I83C751, 0x1ac, "ei2"     },
	{ FEATURE_I8052, 0x1ad, "et2"       },

	/* FIXME: port 3 - depends on external circuits and not really
	 * implemented in the core. TBD */
	{ FEATURE_NONE, 0x1b0, "rxd"        },
	{ FEATURE_NONE, 0x1b1, "txd"        },
	{ FEATURE_NONE, 0x1b2, "int0"       },
	{ FEATURE_NONE, 0x1b3, "int1"       },
	{ FEATURE_NONE, 0x1b4, "t0"         },
	{ FEATURE_NONE, 0x1b5, "t1"         },
	{ FEATURE_NONE, 0x1b6, "wr"         },
	{ FEATURE_NONE, 0x1b7, "rd"         },

	{ FEATURE_NONE, 0x1b8, "px0"        },
	{ FEATURE_NONE, 0x1b9, "pt0"        },
	{ FEATURE_NONE, 0x1ba, "px1"        },
	{ FEATURE_NONE, 0x1bb, "pt1"        },
	{ FEATURE_NONE, 0x1bc, "ps"         },
	{ FEATURE_NONE, 0x1bd, "ip.5"       },
	{ FEATURE_NONE, 0x1be, "ip.6"       },
	{ FEATURE_NONE, 0x1bf, "ip.7"       },

	{ FEATURE_I8052, 0x1bd, "pt2"       },
	{ FEATURE_I83C751, 0x1bc, "pi2"     },

	{ FEATURE_I8052, 0x1c8, "cprl2"     },
	{ FEATURE_I8052, 0x1c9, "ct2"       },
	{ FEATURE_I8052, 0x1ca, "tr2"       },
	{ FEATURE_I8052, 0x1cb, "exen2"     },
	{ FEATURE_I8052, 0x1cc, "tclk"      },
	{ FEATURE_I8052, 0x1cd, "rclk"      },
	{ FEATURE_I8052, 0x1ce, "exf2"      },
	{ FEATURE_I8052, 0x1cf, "tf2"       },

	{ FEATURE_NONE, 0x1d0, "p"          },
	{ FEATURE_NONE, 0x1d1, "psw.1"      },
	{ FEATURE_NONE, 0x1d2, "ov"         },
	{ FEATURE_NONE, 0x1d3, "rs0"        },
	{ FEATURE_NONE, 0x1d4, "rs1"        },
	{ FEATURE_NONE, 0x1d5, "f0"         },
	{ FEATURE_NONE, 0x1d6, "ac"         },
	{ FEATURE_NONE, 0x1d7, "cy"         },

	{ FEATURE_DS5002FP, 0x1d8, "rg0"    },
	{ FEATURE_DS5002FP, 0x1d9, "rpc"    },
	{ FEATURE_DS5002FP, 0x1da, "dma"    },
	{ FEATURE_DS5002FP, 0x1db, "ibi"    },
	{ FEATURE_DS5002FP, 0x1dc, "ae"     },
	{ FEATURE_DS5002FP, 0x1dd, "exbs"   },
	{ FEATURE_DS5002FP, 0x1de, "d8.6"   },
	{ FEATURE_DS5002FP, 0x1df, "rnr"    },

	{ FEATURE_I83C751, 0x1d8, "ct0"     },
	{ FEATURE_I83C751, 0x1d9, "ct1"     },
	{ FEATURE_I83C751, 0x1da, "i2cfg.2" },
	{ FEATURE_I83C751, 0x1db, "i2cfg.3" },
	{ FEATURE_I83C751, 0x1dc, "tirun"   },
	{ FEATURE_I83C751, 0x1dd, "clrti"   },
	{ FEATURE_I83C751, 0x1de, "masterq" },
	{ FEATURE_I83C751, 0x1df, "slaven"  },

	{ FEATURE_I83C751, 0x1f8, "xstp"    },
	{ FEATURE_I83C751, 0x1f9, "xstr"    },
	{ FEATURE_I83C751, 0x1fa, "makstp"  },
	{ FEATURE_I83C751, 0x1fb, "makstr"  },
	{ FEATURE_I83C751, 0x1fc, "xactv"   },
	{ FEATURE_I83C751, 0x1fd, "xdata"   },
	{ FEATURE_I83C751, 0x1fe, "idle"    },
	{ FEATURE_I83C751, 0x1ff, "i2sta.7" },

	/* unknown
	 * "ibf",    "obf",    "idsm",   "obfc",    e8 - eb
	 * "ma0",    "ma1",    "mb0",    "mb1",     ec - ef
	 */

	{ -1 }
};

static void init_mem_names(int feature_set, const char **mem_names)
{
	int i;
	int feature;

	/* Set defaults / i8051 */
	for (i = 0; feature = mem_name_feature[i].feature, feature >= 0; i++)
	{
		if ( feature == FEATURE_NONE )
			mem_names[mem_name_feature[i].addr] = mem_name_feature[i].name;
	}

	/* Set specific memory names */
	for (i = 0; feature = mem_name_feature[i].feature, feature >= 0; i++)
	{
		if (feature & feature_set)
			mem_names[mem_name_feature[i].addr] = mem_name_feature[i].name;
	}

}

static const char *get_data_address( const char **mem_names, UINT8 arg )
{
	static char buffer_array[4][32];
	static int whichbuf;
	char *buffer = &buffer_array[++whichbuf % 4][0];

	if (mem_names[arg] == NULL)
		sprintf(buffer,"$%02X",arg);
	else
		sprintf(buffer,"%s", mem_names[arg]);
	return buffer;
}

static const char *get_bit_address( const char **mem_names, UINT8 arg )
{
	static char buffer[32];

	if(arg < 0x80)
	{
		//Bit address 0-7F can be referred to as 20.0, 20.1, to 20.7 for address 0, and 2f.0,2f.1 to 2f.7 for address 7f
		if(arg < 0x7f)
			sprintf(buffer,"$%02X.%d",(arg >> 3) | 0x20, arg & 0x07);
		else
			sprintf(buffer,"$%02X",arg);
	}
	else
	{
		if (mem_names[arg | 0x100] == NULL)
		{
			if (mem_names[arg & 0xf8] == NULL)
				sprintf(buffer,"$%02X.%d", arg & 0xf8, arg & 0x07);
			else
				sprintf(buffer,"%s.%d", mem_names[arg & 0xf8], arg & 0x07);
		}
		else
			sprintf(buffer,"%s", mem_names[arg | 0x100]);
	}
	return buffer;
}

#else

/*Just display the actual memory address for data & bit address access*/

static const char *get_data_address( UINT8 arg )
{
	static char buffer[32];
	sprintf(buffer,"$%02X",arg);
	return buffer;
}

static const char *get_bit_address( UINT8 arg )
{
	static char buffer[32];
	sprintf(buffer,"$%02X",arg);
	return buffer;
}

#endif

static offs_t mcs51_dasm( const char **mem_names, char *dst, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	unsigned PC = pc;
	const char *sym, *sym2;
	UINT8 op, data;
	UINT16 addr;
	INT8 rel;

	op = oprom[PC++ - pc];
	switch( op )
	{
		//NOP
		case 0x00:              /* 1: 0000 0000 */
			sprintf(dst, "nop");
			break;

		//AJMP code addr        /* 1: aaa0 0001 */
		case 0x01:
		case 0x21:
		case 0x41:
		case 0x61:
		case 0x81:
		case 0xa1:
		case 0xc1:
		case 0xe1:
			addr = opram[PC++ - pc];
			addr|= (PC & 0xf800) | ((op & 0xe0) << 3);
			sprintf(dst, "ajmp  $%04X", addr);
			break;

		//LJMP code addr
		case 0x02:              /* 1: 0000 0010 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "ljmp  $%04X", addr);
			break;

		//RR A
		case 0x03:              /* 1: 0000 0011 */
			sprintf(dst, "rr    a");
			break;

		//INC A
		case 0x04:              /* 1: 0000 0100 */
			sprintf(dst, "inc   a");
			break;

		//INC data addr
		case 0x05:              /* 1: 0000 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "inc   %s", sym);
			break;

		//INC @R0/@R1           /* 1: 0000 011i */
		case 0x06:
		case 0x07:
			sprintf(dst, "inc   @r%d", op&1);
			break;

		//INC R0 to R7          /* 1: 0000 1rrr */
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			sprintf(dst, "inc   r%d", op&7);
			break;

		//JBC bit addr, code addr
		case 0x10:              /* 1: 0001 0000 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jbc   %s,$%04X", sym, PC + rel);
			break;

		//ACALL code addr       /* 1: aaa1 0001 */
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xb1:
		case 0xd1:
		case 0xf1:
			sprintf(dst, "acall $%04X", (PC & 0xf800) | ((op & 0xe0) << 3) | opram[PC - pc]);
			PC++;
			flags = DASMFLAG_STEP_OVER;
			break;

		//LCALL code addr
		case 0x12:              /* 1: 0001 0010 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "lcall $%04X", addr);
			flags = DASMFLAG_STEP_OVER;
			break;

		//RRC A
		case 0x13:              /* 1: 0001 0011 */
			sprintf(dst, "rrc   a");
			break;

		//DEC A
		case 0x14:              /* 1: 0001 0100 */
			sprintf(dst, "dec   a");
			break;

		//DEC data addr
		case 0x15:              /* 1: 0001 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "dec   %s", sym);
			break;

		//Unable to test
		//DEC @R0/@R1           /* 1: 0001 011i */
		case 0x16:
		case 0x17:
			sprintf(dst, "dec   @r%d", op&1);
			break;

		//DEC R0 to R7          /* 1: 0001 1rrr */
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			sprintf(dst, "dec   r%d", op&7);
			break;

		//JB  bit addr, code addr
		case 0x20:              /* 1: 0010 0000 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jb    %s,$%04X", sym, (PC + rel));
			break;

		//RET
		case 0x22:              /* 1: 0010 0010 */
			sprintf(dst, "ret");
			flags = DASMFLAG_STEP_OUT;
			break;

		//RL A
		case 0x23:              /* 1: 0010 0011 */
			sprintf(dst, "rl    a");
			break;

		//ADD A, #data
		case 0x24:              /* 1: 0010 0100 */
			sprintf(dst, "add   a,#$%02X", opram[PC++ - pc]);
			break;

		//ADD A, data addr
		case 0x25:              /* 1: 0010 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "add   a,%s", sym);
			break;

		//Unable to Test
		//ADD A, @R0/@R1        /* 1: 0010 011i */
		case 0x26:
		case 0x27:
			sprintf(dst, "add   a,@r%d", op&1);
			break;

		//ADD A, R0 to R7       /* 1: 0010 1rrr */
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			sprintf(dst, "add   a,r%d", op&7);
			break;

		//JNB bit addr, code addr
		case 0x30:              /* 1: 0011 0000 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jnb   %s,$%04X", sym, (PC + rel));
			break;

		//RETI
		case 0x32:              /* 1: 0011 0010 */
			sprintf(dst, "reti");
			flags = DASMFLAG_STEP_OUT;
			break;

		//RLC A
		case 0x33:              /* 1: 0011 0011 */
			sprintf(dst, "rlc   a");
			break;

		//ADDC A, #data
		case 0x34:              /* 1: 0011 0100 */
			sprintf(dst, "addc  a,#$%02X", opram[PC++ - pc]);
			break;

		//ADDC A, data addr
		case 0x35:              /* 1: 0011 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "addc  a,%s", sym);
			break;

		//ADDC A, @R0/@R1       /* 1: 0011 011i */
		case 0x36:
		case 0x37:
			sprintf(dst, "addc  a,@r%d", op&1);
			break;

		//ADDC A, R0 to R7      /* 1: 0011 1rrr */
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			sprintf(dst, "addc  a,r%d", op&7);
			break;

		//JC code addr
		case 0x40:              /* 1: 0100 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jc    $%04X", PC + rel);
			break;

		//ORL data addr, A
		case 0x42:              /* 1: 0100 0010 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "orl   %s,a", sym);
			break;

		//ORL data addr, #data
		case 0x43:              /* 1: 0100 0011 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "orl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//Unable to Test
		//ORL A, #data
		case 0x44:              /* 1: 0100 0100 */
			sprintf(dst, "orl   a,#$%02X", opram[PC++ - pc]);
			break;

		//ORL A, data addr
		case 0x45:              /* 1: 0100 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "orl   a,%s", sym);
			break;

		//ORL A, @RO/@R1        /* 1: 0100 011i */
		case 0x46:
		case 0x47:
			sprintf(dst, "orl   a,@r%d", op&1);
			break;

		//ORL A, RO to R7       /* 1: 0100 1rrr */
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			sprintf(dst, "orl   a,r%d", op&7);
			break;

		//JNC code addr
		case 0x50:              /* 1: 0101 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jnc   $%04X", PC + rel);
			break;

		//Unable to test
		//ANL data addr, A
		case 0x52:              /* 1: 0101 0010 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "anl   %s,a", sym);
			break;

		//Unable to test
		//ANL data addr, #data
		case 0x53:              /* 1: 0101 0011 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "anl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//ANL A, #data
		case 0x54:              /* 1: 0101 0100 */
			sprintf(dst, "anl   a,#$%02X", opram[PC++ - pc]);
			break;

		//ANL A, data addr
		case 0x55:              /* 1: 0101 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "anl   a,%s", sym);
			break;

		//Unable to test
		//ANL A, @RO/@R1        /* 1: 0101 011i */
		case 0x56:
		case 0x57:
			sprintf(dst, "anl   a,@r%d", op&1);
			break;

		//ANL A, RO to R7       /* 1: 0101 1rrr */
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			sprintf(dst, "anl   a,r%d", op&7);
			break;

		//JZ code addr
		case 0x60:              /* 1: 0110 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jz    $%04X", PC + rel);
			break;

		//Unable to test
		//XRL data addr, A
		case 0x62:              /* 1: 0110 0010 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "xrl   %s,a", sym);
			break;

		//XRL data addr, #data
		case 0x63:              /* 1: 0110 0011 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "xrl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//XRL A, #data
		case 0x64:              /* 1: 0110 0100 */
			sprintf(dst, "xrl   a,#$%02X", opram[PC++ - pc]);
			break;

		//XRL A, data addr
		case 0x65:              /* 1: 0110 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "xrl   a,%s", sym);
			break;

		//Unable to test
		//XRL A, @R0/@R1        /* 1: 0110 011i */
		case 0x66:
		case 0x67:
			sprintf(dst, "xrl   a,@r%d", op&1);
			break;

		//XRL A, R0 to R7       /* 1: 0110 1rrr */
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			sprintf(dst, "xrl   a,r%d", op&7);
			break;

		//JNZ code addr
		case 0x70:              /* 1: 0111 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jnz   $%04X", PC + rel);
			break;

		//Unable to test
		//ORL C, bit addr
		case 0x72:              /* 1: 0111 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "orl   c,%s", sym);
			break;

		//Unable to test
		//JMP @A+DPTR
		case 0x73:              /* 1: 0111 0011 */
			sprintf(dst, "jmp   @a+dptr");
			break;

		//MOV A, #data
		case 0x74:              /* 1: 0111 0100 */
			sprintf(dst, "mov   a,#$%02X", opram[PC++ - pc]);
			break;

		//MOV data addr, #data
		case 0x75:              /* 1: 0111 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//Unable to test
		//MOV @R0/@R1, #data    /* 1: 0111 011i */
		case 0x76:
		case 0x77:
			sprintf(dst, "mov   @r%d,#$%02X", op&1, opram[PC++ - pc]);
			break;

		//MOV R0 to R7, #data   /* 1: 0111 1rrr */
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			sprintf(dst, "mov   r%d,#$%02X", (op & 7), opram[PC++ - pc]);
			break;

		//SJMP code addr
		case 0x80:              /* 1: 1000 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "sjmp  $%04X", PC + rel);
			break;

		//ANL C, bit addr
		case 0x82:              /* 1: 1000 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "anl   c,%s", sym);
			break;

		//MOVC A, @A + PC
		case 0x83:              /* 1: 1000 0011 */
			sprintf(dst, "movc  a,@a+pc");
			break;

		//DIV AB
		case 0x84:              /* 1: 1000 0100 */
			sprintf(dst, "div   ab");
			break;

		//MOV data addr, data addr  (Note: 1st address is src, 2nd is dst, but the mov command works as mov dst,src)
		case 0x85:              /* 1: 1000 0101 */
			sym  = get_data_address(mem_names, opram[PC++ - pc]);
			sym2 = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,%s", sym2, sym);
			break;

		//Unable to test
		//MOV data addr, @R0/@R1/* 1: 1000 011i */
		case 0x86:
		case 0x87:
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,@r%d", sym, op&1);
			break;

		//MOV data addr,R0 to R7/* 1: 1000 1rrr */
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,r%d", sym, op&7);
			break;

		//MOV DPTR, #data16
		case 0x90:              /* 1: 1001 0000 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "mov   dptr,#$%04X", addr);
			break;

		//MOV bit addr, C
		case 0x92:              /* 1: 1001 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,c", sym);
			break;

		//MOVC A, @A + DPTR
		case 0x93:              /* 1: 1001 0011 */
			sprintf(dst, "movc  a,@a+dptr");
			break;

		//SUBB A, #data
		case 0x94:              /* 1: 1001 0100 */
			sprintf(dst, "subb  a,#$%02X", opram[PC++ - pc]);
			break;

		//SUBB A, data addr
		case 0x95:              /* 1: 1001 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "subb  a,%s", sym);
			break;

		//Unable to test
		//SUBB A, @R0/@R1       /* 1: 1001 011i */
		case 0x96:
		case 0x97:
			sprintf(dst, "subb  a,@r%d", op&1);
			break;

		//SUBB A, R0 to R7      /* 1: 1001 1rrr */
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
			sprintf(dst, "subb  a,r%d", op&7);
			break;

		//Unable to test
		//ORL C, /bit addr
		case 0xa0:                /* 1: 1010 0000 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "orl   c,/%s", sym);
			break;

		//MOV C, bit addr
		case 0xa2:                /* 1: 1010 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   c,%s", sym);
			break;

		//INC DPTR
		case 0xa3:                /* 1: 1010 0011 */
			sprintf(dst, "inc   dptr");
			break;

		//MUL AB
		case 0xa4:                /* 1: 1010 0100 */
			sprintf(dst, "mul   ab");
			break;

		//reserved
		case 0xa5:                /* 1: 1010 0101 */
			sprintf(dst, "ill/rsv");
			break;

		//Unable to test
		//MOV @R0/@R1, data addr  /* 1: 1010 011i */
		case 0xa6:
		case 0xa7:
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   @r%d,%s", op&1, sym);
			break;

		//MOV R0 to R7, data addr /* 1: 1010 1rrr */
		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   r%d,%s", op&7, sym);
			break;

		//ANL C,/bit addr
		case 0xb0:                       /* 1: 1011 0000 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "anl   c,/%s", sym);
			break;

		//CPL bit addr
		case 0xb2:                       /* 1: 1011 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "cpl   %s", sym);
			break;

		//Unable to test
		//CPL C
		case 0xb3:                       /* 1: 1011 0011 */
			sprintf(dst, "cpl   c");
			break;

		//CJNE A, #data, code addr
		case 0xb4:                       /* 1: 1011 0100 */
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  a,#$%02X,$%04X", data, PC + rel);
			break;

		//CJNE A, data addr, code addr
		case 0xb5:                       /* 1: 1011 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  a,%s,$%04X", sym, PC + rel);
			break;

		//Unable to test
		//CJNE @R0/@R1, #data, code addr /* 1: 1011 011i */
		case 0xb6:
		case 0xb7:
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  @r%d,#$%02X,$%04X", op&1, data, PC + rel);
			break;

		//CJNE R0 to R7, #data, code addr/* 1: 1011 1rrr */
		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  r%d,#$%02X,$%04X", op&7, data, PC + rel);
			break;

		//PUSH data addr
		case 0xc0:                      /* 1: 1100 0000 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "push  %s", sym);
			break;

		//CLR bit addr
		case 0xc2:                      /* 1: 1100 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "clr   %s", sym);
			break;

		//CLR C
		case 0xc3:                      /* 1: 1100 0011 */
			sprintf(dst, "clr   c");
			break;

		//SWAP A
		case 0xc4:                      /* 1: 1100 0100 */
			sprintf(dst, "swap  a");
			break;

		//XCH A, data addr
		case 0xc5:                      /* 1: 1100 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "xch   a,%s", sym);
			break;

		//XCH A, @RO/@R1                /* 1: 1100 011i */
		case 0xc6:
		case 0xc7:
			sprintf(dst, "xch   a,@r%d", op&1);
			break;

		//XCH A, RO to R7               /* 1: 1100 1rrr */
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			sprintf(dst, "xch   a,r%d", op&7);
			break;

		//POP data addr
		case 0xd0:                      /* 1: 1101 0000 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "pop   %s", sym);
			break;

		//SETB bit addr
		case 0xd2:                      /* 1: 1101 0010 */
			sym = get_bit_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "setb  %s", sym);
			break;

		//SETB C
		case 0xd3:                      /* 1: 1101 0011 */
			sprintf(dst, "setb  c");
			break;

		//Unable to test
		//DA A
		case 0xd4:                      /* 1: 1101 0100 */
			sprintf(dst, "da   a");
			break;

		//DJNZ data addr, code addr
		case 0xd5:                      /* 1: 1101 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "djnz  %s,$%04X", sym, PC + rel);
			flags = DASMFLAG_STEP_OVER;
			break;

		//XCHD A, @R0/@R1               /* 1: 1101 011i */
		case 0xd6:
		case 0xd7:
			sprintf(dst, "xchd  a,@r%d", op&1);
			break;

		//DJNZ R0 to R7,code addr       /* 1: 1101 1rrr */
		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:
			rel = opram[PC++ - pc];
			sprintf(dst, "djnz  r%d,$%04X", op&7, (PC + rel));
			flags = DASMFLAG_STEP_OVER;
			break;

		//MOVX A,@DPTR
		case 0xe0:                      /* 1: 1110 0000 */
			sprintf(dst, "movx  a,@dptr");
			break;

		//Unable to test
		//MOVX A, @R0/@R1               /* 1: 1110 001i */
		case 0xe2:
		case 0xe3:
			sprintf(dst, "movx  a,@r%d", op&1);
			break;

		//CLR A
		case 0xe4:                      /* 1: 1110 0100 */
			sprintf(dst, "clr   a");
			break;

		//MOV A, data addr
		case 0xe5:                      /* 1: 1110 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   a,%s", sym);
			break;

		//Unable to test
		//MOV A,@RO/@R1                 /* 1: 1110 011i */
		case 0xe6:
		case 0xe7:
			sprintf(dst, "mov   a,@r%d", op&1);
			break;

		//MOV A,R0 to R7                /* 1: 1110 1rrr */
		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:
			sprintf(dst, "mov   a,r%d", op&7);
			break;

		//MOVX @DPTR,A
		case 0xf0:                      /* 1: 1111 0000 */
			sprintf(dst, "movx  @dptr,a");
			break;

		//Unable to test
		//MOVX @R0/@R1,A                /* 1: 1111 001i */
		case 0xf2:
		case 0xf3:
			sprintf(dst, "movx  @r%d,a", op&1);
			break;

		//CPL A
		case 0xf4:                      /* 1: 1111 0100 */
			sprintf(dst, "cpl   a");
			break;

		//MOV data addr, A
		case 0xf5:                      /* 1: 1111 0101 */
			sym = get_data_address(mem_names, opram[PC++ - pc]);
			sprintf(dst, "mov   %s,a", sym);
			break;

		//MOV @R0/@R1, A                /* 1: 1111 011i */
		case 0xf6:
		case 0xf7:
			sprintf(dst, "mov   @r%d,a", op&1);
			break;

		//MOV R0 to R7, A               /* 1: 1111 1rrr */
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:
			sprintf(dst, "mov   r%d,a", op&7);
			break;

		default:
			sprintf(dst, "illegal");
	}
	return (PC - pc) | flags | DASMFLAG_SUPPORTED;

}

CPU_DISASSEMBLE( i8051 )
{
	static const char *mem_names[0x200];
	static int mem_names_initialized = 0;

	if (!mem_names_initialized)
	{
		init_mem_names( FEATURE_NONE, mem_names);
		mem_names_initialized = 1;
	}
	return mcs51_dasm(mem_names, buffer, pc, oprom, opram);
}

CPU_DISASSEMBLE( i8052 )
{
	static const char *mem_names[0x200];
	static int mem_names_initialized = 0;

	if (!mem_names_initialized)
	{
		init_mem_names( FEATURE_I8052, mem_names);
		mem_names_initialized = 1;
	}
	return mcs51_dasm(mem_names, buffer, pc, oprom, opram);
}

CPU_DISASSEMBLE( i80c51 )
{
	static const char *mem_names[0x200];
	static int mem_names_initialized = 0;

	if (!mem_names_initialized)
	{
		init_mem_names( FEATURE_CMOS, mem_names);
		mem_names_initialized = 1;
	}
	return mcs51_dasm(mem_names, buffer, pc, oprom, opram);
}

CPU_DISASSEMBLE( i80c52 )
{
	static const char *mem_names[0x200];
	static int mem_names_initialized = 0;

	if (!mem_names_initialized)
	{
		init_mem_names( FEATURE_I8052 | FEATURE_CMOS | FEATURE_I80C52, mem_names);
		mem_names_initialized = 1;
	}
	return mcs51_dasm(mem_names, buffer, pc, oprom, opram);
}

CPU_DISASSEMBLE( ds5002fp )
{
	static const char *mem_names[0x200];
	static int mem_names_initialized = 0;

	if (!mem_names_initialized)
	{
		init_mem_names( FEATURE_DS5002FP | FEATURE_CMOS, mem_names);
		mem_names_initialized = 1;
	}
	return mcs51_dasm(mem_names, buffer, pc, oprom, opram);
}
