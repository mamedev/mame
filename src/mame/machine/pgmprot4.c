/***********************************************************************
 PGM 022 + 025 PGM protection emulation

 these are simulations of the IGS 022 and 025 protection combination
 used on the following PGM games

 The Killing Blade
 Dragon World 3*
 Dragon World 3 EX*

 * preliminary, not working

 ----

 IGS022 is an encrypted DMA device, most likely an MCU of some sort

 IGS025 is some kind of state machine / logic device which the game
 uses for various securit checks, and to determine the region of the
 game based on string sequences.

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

/* The IGS022 is an MCU which performs encrypted DMA used by
 - The Killing Blade
 - Dragon World 3

 There is also an automatic transfer which happens on startup using params stored in the data ROM.
 This has been verified on real hardware running without any 68k game program.

*/


static void IGS022_do_dma(running_machine& machine, UINT16 src, UINT16 dst, UINT16 size, UINT16 mode)
{
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();
	UINT16 param;
	/*
    P_SRC =0x300290 (offset from prot rom base)
    P_DST =0x300292 (words from 0x300000)
    P_SIZE=0x300294 (words)
    P_MODE=0x300296

    Mode 5 direct
    Mode 6 swap nibbles and bytes

    1,2,3 table based ops
    */

	//mame_printf_debug("src %04x dst %04x size %04x mode %04x\n", src, dst, size, mode);

	//if (src&1) mame_printf_debug("odd offset\n");

	param = mode >> 8;
	mode &=0xf;  // what are the other bits?


	if ((mode == 0) || (mode == 1) || (mode == 2) || (mode == 3))
	{
		/* mode3 applies a xor from a 0x100 byte table to the data being
           transferred

           the table is stored at the start of the protection rom.

           the param used with the mode gives a start offset into the table

           odd offsets seem to change the table slightly (see rawDataOdd)

       */

		/*
        unsigned char rawDataOdd[256] = {
            0xB6, 0xA8, 0xB1, 0x5D, 0x2C, 0x5D, 0x4F, 0xC1,
            0xCF, 0x39, 0x3A, 0xB7, 0x65, 0x85, 0xD9, 0xEE,
            0xDB, 0x7B, 0x5F, 0x81, 0x03, 0x6D, 0xEB, 0x07,
            0x0F, 0xB5, 0x61, 0x59, 0xCD, 0x60, 0x06, 0x21,
            0xA0, 0x99, 0xDD, 0x27, 0x42, 0xD7, 0xC5, 0x5B,
            0x3B, 0xC6, 0x4F, 0xA2, 0x20, 0xF6, 0x61, 0x61,
            0x8C, 0x46, 0x8C, 0xCA, 0xE0, 0x0E, 0x2C, 0xE9,
            0xBA, 0x0F, 0x45, 0x6D, 0x36, 0x1C, 0x18, 0x37,
            0xE7, 0x85, 0x89, 0xA4, 0x94, 0x46, 0x30, 0x9B,
            0xB2, 0xF4, 0x41, 0x55, 0xA5, 0x63, 0x1C, 0xEF,
            0xB7, 0x18, 0xB3, 0xB1, 0xD4, 0x72, 0xA0, 0x1C,
            0x0B, 0x97, 0x02, 0xB6, 0xC5, 0x1F, 0x1B, 0x94,
            0xC3, 0x83, 0xAA, 0xAC, 0xD9, 0x44, 0x09, 0xD7,
            0x6C, 0xDB, 0x07, 0xA9, 0xAD, 0x64, 0x83, 0xF1,
            0x92, 0x09, 0xCD, 0x0E, 0x99, 0x2F, 0xBC, 0xF8,
            0x3C, 0x63, 0x8F, 0x0A, 0x33, 0x03, 0x84, 0x91,
            0x6C, 0xAC, 0x3A, 0x15, 0xCB, 0x67, 0xC7, 0x69,
            0xA1, 0x92, 0x99, 0x74, 0xEE, 0x90, 0x0D, 0xBE,
            0x57, 0x30, 0xD1, 0xBA, 0xE5, 0xDE, 0xFA, 0xD6,
            0x83, 0x8C, 0xE4, 0x43, 0x36, 0x5E, 0xCD, 0x84,
            0x1A, 0x18, 0x31, 0xB9, 0x20, 0x48, 0xE3, 0xA8,
            0x89, 0x32, 0xF0, 0x90, 0x21, 0x80, 0x33, 0xAE,
            0x3C, 0xA6, 0xB8, 0x8C, 0x72, 0x17, 0xD1, 0x0C,
            0x1A, 0x29, 0xFA, 0x38, 0x87, 0xC9, 0x6E, 0xC7,
            0x05, 0xDE, 0x85, 0x6E, 0x92, 0x7E, 0xD4, 0xED,
            0x5C, 0xD3, 0x03, 0xD4, 0xFE, 0xCB, 0x6C, 0x19,
            0x7A, 0x83, 0x79, 0x5B, 0xF6, 0x71, 0xBA, 0xF4,
            0x37, 0x53, 0xC9, 0xC1, 0xDE, 0xDB, 0xDE, 0xB1,
            0x64, 0x17, 0x31, 0x0E, 0xD7, 0xA2, 0x13, 0x8E,
            0x52, 0x8D, 0xCB, 0x19, 0x3D, 0x0B, 0x31, 0x58,
            0x4A, 0xDE, 0x0C, 0x01, 0x2B, 0x85, 0x2D, 0xE5,
            0x13, 0x22, 0x48, 0xB6, 0xF3, 0x2D, 0x00, 0x9A
        };
        */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();

		for (x = 0; x < size; x++)
		{
			//UINT16 *RAMDUMP = (UINT16*)space->machine().region("user2")->base();
			//UINT16 dat = RAMDUMP[dst + x];

			UINT16 dat2 = PROTROM[src + x];

			UINT8 extraoffset = param&0xfe; // the lowest bit changed the table addressing in tests, see 'rawDataOdd' table instead.. it's still related to the main one, not identical
			UINT8* dectable = (UINT8*)machine.region("igs022data")->base();//rawDataEven; // the basic decryption table is at the start of the mcu data rom! at least in killbld
			UINT16 extraxor = ((dectable[((x*2)+0+extraoffset)&0xff]) << 8) | (dectable[((x*2)+1+extraoffset)&0xff] << 0);

			dat2 = ((dat2 & 0x00ff)<<8) | ((dat2 & 0xff00)>>8);

			//  mode==0 plain
			if (mode==3) dat2 ^= extraxor;
			if (mode==2) dat2 += extraxor;
			if (mode==1) dat2 -= extraxor;

			//if (dat!=dat2)
			//  printf("Mode %04x Param %04x Mismatch %04x %04x\n", mode, param, dat, dat2);

			state->m_sharedprotram[dst + x] = dat2;
		}

		/* Killing Blade: hack, patches out some additional security checks... we need to emulate them instead! */
		// different region IGS025 devices supply different sequences - we currently only have the china sequence for Killing Blade
		//if ((mode==3) && (param==0x54) && (src*2==0x2120) && (dst*2==0x2600)) state->m_sharedprotram[0x2600 / 2] = 0x4e75;

	}
	if (mode == 4)
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* looks almost like a fixed value xor, but isn't */
	}
	else if (mode == 5)
	{
		/* mode 5 seems to be a straight copy */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];


			state->m_sharedprotram[dst + x] = dat;
		}
	}
	else if (mode == 6)
	{
		/* mode 6 seems to swap bytes and nibbles */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];

			dat = ((dat & 0xf000) >> 12)|
				  ((dat & 0x0f00) >> 4)|
				  ((dat & 0x00f0) << 4)|
				  ((dat & 0x000f) << 12);

			state->m_sharedprotram[dst + x] = dat;
		}
	}
	else if (mode == 7)
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* weird mode, the params get left in memory? - maybe it's a NOP? */
	}
	else
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* invalid? */

	}
}

// the internal MCU boot code automatically does this DMA
// and puts the version # of the data rom in ram
static void IGS022_reset(running_machine& machine)
{
	int i;
	UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();
	UINT16 tmp;

	// fill ram with A5 patern
	for (i = 0; i < 0x4000/2; i++)
		state->m_sharedprotram[i] = 0xa55a;

	// the auto-dma
	UINT16 src = PROTROM[0x100 / 2];
	UINT32 dst = PROTROM[0x102 / 2];
	UINT16 size = PROTROM[0x104/ 2];
	UINT16 mode = PROTROM[0x106 / 2];

	src = ((src & 0xff00) >> 8) | ((src & 0x00ff) << 8);
	dst = ((dst & 0xff00) >> 8) | ((dst & 0x00ff) << 8);
	size = ((size & 0xff00) >> 8) | ((size & 0x00ff) << 8);
	mode &= 0xff;

	src >>= 1;

	printf("Auto-DMA %04x %04x %04x %04x\n",src,dst,size,mode);

	IGS022_do_dma(machine,src,dst,size,mode);

	// there is also a version ID? (or is it some kind of checksum) that is stored in the data rom, and gets copied..
	// Dragon World 3 checks it
	tmp = PROTROM[0x114/2];
	tmp = ((tmp & 0xff00) >> 8) | ((tmp & 0x00ff) << 8);
	state->m_sharedprotram[0x2a2/2] = tmp;
}

static void IGS022_handle_command(running_machine& machine)
{
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();
	UINT16 cmd = state->m_sharedprotram[0x200/2];
	//mame_printf_debug("command %04x\n", cmd);
	if (cmd == 0x6d)	//Store values to asic ram
	{
		UINT32 p1 = (state->m_sharedprotram[0x298/2] << 16) | state->m_sharedprotram[0x29a/2];
		UINT32 p2 = (state->m_sharedprotram[0x29c/2] << 16) | state->m_sharedprotram[0x29e/2];

		if ((p2 & 0xffff) == 0x9)	//Set value
		{
			int reg = (p2 >> 16) & 0xffff;
			if (reg & 0x200)
				state->m_kb_regs[reg & 0xff] = p1;
		}
		if ((p2 & 0xffff) == 0x6)	//Add value
		{
			int src1 = (p1 >> 16) & 0xff;
			int src2 = (p1 >> 0) & 0xff;
			int dst = (p2 >> 16) & 0xff;
			state->m_kb_regs[dst] = state->m_kb_regs[src2] - state->m_kb_regs[src1];
		}
		if ((p2 & 0xffff) == 0x1)	//Add Imm?
		{
			int reg = (p2 >> 16) & 0xff;
			int imm = (p1 >> 0) & 0xffff;
			state->m_kb_regs[reg] += imm;
		}
		if ((p2 & 0xffff) == 0xa)	//Get value
		{
			int reg = (p1 >> 16) & 0xFF;
			state->m_sharedprotram[0x29c/2] = (state->m_kb_regs[reg] >> 16) & 0xffff;
			state->m_sharedprotram[0x29e/2] = state->m_kb_regs[reg] & 0xffff;
		}
	}
	if(cmd == 0x4f)	//memcpy with encryption / scrambling
	{
		UINT16 src = state->m_sharedprotram[0x290 / 2] >> 1; // ?
		UINT32 dst = state->m_sharedprotram[0x292 / 2];
		UINT16 size = state->m_sharedprotram[0x294 / 2];
		UINT16 mode = state->m_sharedprotram[0x296 / 2];

		IGS022_do_dma(machine, src,dst,size,mode);
	}

}


static WRITE16_HANDLER( killbld_igs025_prot_w )
{
//  mame_printf_debug("killbrd prot r\n");
//  return 0;
	pgm_022_025_state *state = space->machine().driver_data<pgm_022_025_state>();
	offset &= 0xf;

	if (offset == 0)
		state->m_kb_cmd = data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, data);
		if (state->m_kb_cmd == 0)
			state->m_kb_reg = data;
		else if (state->m_kb_cmd == 2)
		{
			if (data == 1)	//Execute cmd
			{
				IGS022_handle_command(space->machine());
				state->m_kb_reg++;
			}
		}
		else if (state->m_kb_cmd == 4)
			state->m_kb_ptr = data;
		else if (state->m_kb_cmd == 0x20)
			state->m_kb_ptr++;
	}
}

static READ16_HANDLER( killbld_igs025_prot_r )
{
//  mame_printf_debug("killbld prot w\n");
	pgm_022_025_state *state = space->machine().driver_data<pgm_022_025_state>();
	UINT16 res ;

	offset &= 0xf;
	res = 0;

	if (offset == 1)
	{
		if (state->m_kb_cmd == 1)
		{
			res = state->m_kb_reg & 0x7f;
		}
		else if (state->m_kb_cmd == 5)
		{

			UINT8 kb_region_sequence[11] = {0x17, 0x14, 0x91, 0x89, 0x21, 0xD5, 0x7C, 0x65, 0x8F, 0x8E, 0xE1};
			UINT8 ret;

			// this isn't properly understood.. should be some kind of bitswap / xor / shift..based on values written to 0x22/0x23 etc.?
			// return hardcoded china sequence results for now, avoids rom patch
			if (state->m_kb_region_sequence_position < 11)
			{
				ret = kb_region_sequence[state->m_kb_region_sequence_position];
				state->m_kb_region_sequence_position++;
			}
			else
			{
				UINT32 protvalue = 0x89911400 | input_port_read(space->machine(), "Region");
				ret = (protvalue >> (8 * (state->m_kb_ptr - 1))) & 0xff;
			}

			res = 0x3f00 | ret;  // always 0x3fxx in logged behavior...

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, res);
	return res;
}




static MACHINE_RESET( killbld )
{
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();

	MACHINE_RESET_CALL(pgm);
	/* fill the protection ram with a5 + auto dma */
	IGS022_reset(machine);

	// Reset IGS025 stuff
	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

}




DRIVER_INIT( killbld )
{
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();

	pgm_basic_init(machine);
	pgm_killbld_decrypt(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xd40000, 0xd40003, FUNC(killbld_igs025_prot_r), FUNC(killbld_igs025_prot_w));

	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

	state->save_item(NAME(state->m_kb_region_sequence_position));
	state->save_item(NAME(state->m_kb_cmd));
	state->save_item(NAME(state->m_kb_reg));
	state->save_item(NAME(state->m_kb_ptr));
	state->save_item(NAME(state->m_kb_regs));
}

static MACHINE_RESET( dw3 )
{
	pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();


	MACHINE_RESET_CALL(pgm);
	/* fill the protection ram with a5 + auto dma */
	IGS022_reset(machine);

	/* game won't boot unless various values are in protection RAM
     - these should almost certainly end up there as the result of executing the protection
       commands are startup, but which, and how? */

//  state->m_sharedprotram[0x200/2] = 0x006d;
	state->m_sharedprotram[0x202/2] = 0x007c; // it cares about this, operation status flag?

//  state->m_sharedprotram[0x20c/2] = 0x0000;
//  state->m_sharedprotram[0x20e/2] = 0x0007;
//  state->m_sharedprotram[0x210/2] = 0x0000;
//  state->m_sharedprotram[0x212/2] = 0x0004;
//  state->m_sharedprotram[0x214/2] = 0x0000;
//  state->m_sharedprotram[0x216/2] = 0x0007;
//  state->m_sharedprotram[0x218/2] = 0x0000;
//  state->m_sharedprotram[0x21a/2] = 0x0004;

//  state->m_sharedprotram[0x288/2] = 0x0000;
//  state->m_sharedprotram[0x28a/2] = 0x00c2;
//  state->m_sharedprotram[0x28c/2] = 0x0000;
//  state->m_sharedprotram[0x28e/2] = 0x00c2;
//  state->m_sharedprotram[0x290/2] = 0x0500;
//  state->m_sharedprotram[0x292/2] = 0x1000;
//  state->m_sharedprotram[0x294/2] = 0x00c3;
//  state->m_sharedprotram[0x296/2] = 0x7104;
//  state->m_sharedprotram[0x298/2] = 0x0000;
//  state->m_sharedprotram[0x29a/2] = 0x0003;
//  state->m_sharedprotram[0x29c/2] = 0x0108;
//  state->m_sharedprotram[0x29e/2] = 0x0009;

//  state->m_sharedprotram[0x2a2/2] = 0x84f6; // it cares about this, it's the version number of the data rom, copied automatically!

//  state->m_sharedprotram[0x2ac/2] = 0x006d;
//  state->m_sharedprotram[0x2ae/2] = 0x0000;

//  state->m_sharedprotram[0x2b0/2] = 0xaf56;


	// Reset IGS025 stuff
	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

}



static int reg;
static int ptr=0;

#define DW3BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))
static UINT8 dw3_swap;
static WRITE16_HANDLER( drgw3_igs025_prot_w )
{
	pgm_022_025_state *state = space->machine().driver_data<pgm_022_025_state>();

	offset&=0xf;

	if(offset==0)
		state->m_kb_cmd=data;
	else //offset==2
	{
		printf("%06X: ASIC25 W CMD %X  VAL %X\n",cpu_get_pc(&space->device()),state->m_kb_cmd,data);
		if(state->m_kb_cmd==0)
			reg=data;
		else if(state->m_kb_cmd==3)	//??????????
		{
			dw3_swap = data;

			printf("SWAP %02x\n",dw3_swap);
		}
		//else if(kb_cmd==4)
		//  ptr=data;
		else if(state->m_kb_cmd==0x20)
			ptr++;
	}
}

static READ16_HANDLER( drgw3_igs025_prot_r )
{
//  mame_printf_debug("killbld prot w\n");
	pgm_022_025_state *state = space->machine().driver_data<pgm_022_025_state>();

	UINT16 res ;

	offset&=0xf;
	res=0;

	if(offset==1)
	{
		if(state->m_kb_cmd==0)	//swap
		{
				UINT8 v1=(dw3_swap+1)&0x7F;
				UINT8 v2=0;
				DW3BITSWAP(v1,v2,7,0);
				DW3BITSWAP(v1,v2,6,1);
				DW3BITSWAP(v1,v2,5,2);
				DW3BITSWAP(v1,v2,4,3);
				DW3BITSWAP(v1,v2,3,4);
				DW3BITSWAP(v1,v2,2,5);
				DW3BITSWAP(v1,v2,1,6);
				DW3BITSWAP(v1,v2,0,7);

				res=v2;

		}
		else if(state->m_kb_cmd==1)
		{
			res=reg&0x7f;
		}
		else if(state->m_kb_cmd==5)
		{
			UINT32 protvalue;
			protvalue = 0x60000|input_port_read(space->machine(), "Region");
			res=(protvalue>>(8*(ptr-1)))&0xff;


		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n",cpu_get_pc(&space->device()),state->m_kb_cmd,res);
	return res;
}


DRIVER_INIT( drgw3 )
{
	pgm_basic_init(machine);

/*
    pgm_022_025_state *state = machine.driver_data<pgm_022_025_state>();

    {
        int x;
        UINT16 *RAMDUMP = (UINT16*)machine.region("user2")->base();
        for (x=0;x<(0x4000/2);x++)
        {
            state->m_sharedprotram[x] = RAMDUMP[x];
            if((x>=0x100)&&(x<0x110)) printf("data 0x%4x, offset:%x\n",state->m_sharedprotram[x],x);
        }
    }
*/
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xDA5610, 0xDA5613, FUNC(drgw3_igs025_prot_r), FUNC(drgw3_igs025_prot_w));

	pgm_dw3_decrypt(machine);
}


static ADDRESS_MAP_START( killbld_mem, AS_PROGRAM, 16, pgm_022_025_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x2fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_SHARE("sharedprotram") // Shared with protection device
ADDRESS_MAP_END



MACHINE_CONFIG_START( pgm_022_025_kb, pgm_022_025_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(killbld_mem)

	MCFG_MACHINE_RESET(killbld)
MACHINE_CONFIG_END


MACHINE_CONFIG_START( pgm_022_025_dw, pgm_022_025_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(killbld_mem)

	MCFG_MACHINE_RESET(dw3)
MACHINE_CONFIG_END


INPUT_PORTS_START( killbld )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x00ff, 0x0021, "Region (not currently working)" ) // different regions supply different protection code sequences, we only have the China one ATM
	PORT_DIPSETTING(      0x0016, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( China ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( Japan ) )
//  PORT_DIPSETTING(      0x001a, "1a" ) // invalid
//  PORT_DIPSETTING(      0x001b, "1b" ) // invalid
//  PORT_DIPSETTING(      0x001c, "1c" ) // invalid
//  PORT_DIPSETTING(      0x001d, "1d" ) // invalid
//  PORT_DIPSETTING(      0x001e, "1e" ) // invalid
//  PORT_DIPSETTING(      0x001f, "1f" ) // invalid
	PORT_DIPSETTING(      0x0020, DEF_STR( Korea ) )
	PORT_DIPSETTING(      0x0021, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( dw3 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0006, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "0" )
	PORT_CONFSETTING(      0x0001, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_CONFSETTING(      0x0003, "3" )
	PORT_CONFSETTING(      0x0004, "4" )
	PORT_CONFSETTING(      0x0005, "5" )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0007, "7" )

INPUT_PORTS_END
