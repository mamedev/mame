#include "driver.h"
#include "includes/pgm.h"

/*** ASIC27a (Puzzle Star) -- ARM but with no external rom? behaves a bit like KOV ***/




/*** (PSTARS) ***/
static UINT16 PSTARSKEY;
static UINT16 PSTARSINT[2];
static UINT32 PSTARS_REGS[16];
static UINT32 PSTARS_VAL;

static UINT16 pstar_e7,pstar_b1,pstar_ce;
static UINT16 pstar_ram[3];

static const int Pstar_ba[0x1E]={
	0x02,0x00,0x00,0x01,0x00,0x03,0x00,0x00, //0
	0x02,0x00,0x06,0x00,0x22,0x04,0x00,0x03, //8
	0x00,0x00,0x06,0x00,0x20,0x07,0x00,0x03, //10
	0x00,0x21,0x01,0x00,0x00,0x63
};

static const int Pstar_b0[0x10]={
	0x09,0x0A,0x0B,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x00,0x00,0x00,0x00
};

static const int Pstar_ae[0x10]={
	0x5D,0x86,0x8C ,0x8B,0xE0,0x8B,0x62,0xAF,
	0xB6,0xAF,0x10A,0xAF,0x00,0x00,0x00,0x00
};

static const int Pstar_a0[0x10]={
	0x02,0x03,0x04,0x05,0x06,0x01,0x0A,0x0B,
	0x0C,0x0D,0x0E,0x09,0x00,0x00,0x00,0x00,
};

static const int Pstar_9d[0x10]={
	0x05,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static const int Pstar_90[0x10]={
	0x0C,0x10,0x0E,0x0C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const int Pstar_8c[0x23]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,
	0x03,0x03,0x03
};

static const int Pstar_80[0x1a3]={
	0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,
	0x05,0x05,0x06,0x06,0x03,0x03,0x04,0x04,
	0x05,0x05,0x05,0x05,0x06,0x06,0x07,0x07,
	0x03,0x03,0x04,0x04,0x05,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x06,0x06,
	0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,
	0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,
	0x07,0x07,0x08,0x08,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,
	0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,
	0x09,0x09,0x09,0x09,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x09,0x09,0x09,
	0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x08,0x09,0x09,0x05,0x05,0x06,0x06,
	0x06,0x07,0x07,0x08,0x08,0x08,0x08,0x09,
	0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x09,0x09,0x09,0x06,0x06,0x07,0x03,
	0x07,0x06,0x07,0x07,0x08,0x07,0x05,0x04,
	0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
	0x06,0x06,0x06,0x06,0x03,0x04,0x04,0x04,
	0x04,0x05,0x05,0x06,0x06,0x06,0x06,0x07,
	0x04,0x04,0x05,0x05,0x06,0x06,0x06,0x06,
	0x06,0x07,0x07,0x08,0x05,0x05,0x06,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x05,0x05,0x05,0x07,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x09,0x09,
	0x09,0x09,0x03,0x04,0x04,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x07,0x07,0x08,0x08,
	0x08,0x09,0x09,0x09,0x03,0x04,0x05,0x05,
	0x04,0x03,0x04,0x04,0x04,0x05,0x05,0x04,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};




READ16_HANDLER (PSTARS_protram_r)
{
	if (offset == 4)		//region
		return input_port_read(space->machine, "Region");
	else if (offset >= 0x10)  //timer
	{
	  logerror("PSTARS ACCESS COUNTER %6X\n",pstar_ram[offset-0x10]);
		return pstar_ram[offset-0x10]--;
	}
	return 0x0000;
}

READ16_HANDLER (PSTARS_r16)
{
	if(offset==0)
	{
		UINT16 d=PSTARS_VAL&0xffff;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
//      logerror("PSTARS A27 R  %6X\n",PSTARS_VAL);
		return d;
	}
	else if(offset==1)
	{
		UINT16 d=PSTARS_VAL>>16;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
		return d;

	}
	return 0xff;
}

WRITE16_HANDLER (PSTARS_w16)
{
	if(offset==0)
	{
		PSTARSINT[0]=data;
		return;
	}

	if(offset==1)
	{
		UINT16 realkey;
		if((data>>8)==0xff)
			PSTARSKEY=0xff00;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		{
			PSTARSKEY+=0x100;
			PSTARSKEY&=0xff00;
		 if(PSTARSKEY==0xff00)PSTARSKEY=0x100;
		 }
		data^=realkey;
		PSTARSINT[1]=data;
		PSTARSINT[0]^=realkey;

		switch(PSTARSINT[1]&0xff)
			{
				case 0x99:
				{
					PSTARSKEY=0x100;
					PSTARS_VAL=0x880000;

				}
				break;

				case 0xE0:
					{
						PSTARS_VAL=0xa00000+(PSTARSINT[0]<<6);
					}
					break;
				case 0xDC:
					{
						PSTARS_VAL=0xa00800+(PSTARSINT[0]<<6);
					}
					break;
				case 0xD0:
					{
						PSTARS_VAL=0xa01000+(PSTARSINT[0]<<5);
					}
					break;

				case 0xb1:
					{
						pstar_b1=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xbf:
					{
						PSTARS_VAL=pstar_b1*PSTARSINT[0];
					}
					break;

				case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
					{
						PSTARS_VAL=0;
					}
					break;
				case 0xce: //TODO:TIMER  0,1,2
					{
						pstar_ce=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xcf: //TODO:TIMER  0,1,2
					{
						pstar_ram[pstar_ce]=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;


				case 0xe7:
					{
						pstar_e7=(PSTARSINT[0]>>12)&0xf;
						PSTARS_REGS[pstar_e7]&=0xffff;
						PSTARS_REGS[pstar_e7]|=(PSTARSINT[0]&0xff)<<16;
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xe5:
					{

						PSTARS_REGS[pstar_e7]&=0xff0000;
						PSTARS_REGS[pstar_e7]|=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xf8: //@73C
	   			{
	    			PSTARS_VAL=PSTARS_REGS[PSTARSINT[0]&0xf]&0xffffff;
	   			}
	   			break;


				case 0xba:
	   			{
	    			PSTARS_VAL=Pstar_ba[PSTARSINT[0]];
	   			}
	   			break;
				case 0xb0:
	   			{
	    			PSTARS_VAL=Pstar_b0[PSTARSINT[0]];
	   			}
	   			break;
				case 0xae:
	   			{
	    			PSTARS_VAL=Pstar_ae[PSTARSINT[0]];
	   			}
	   			break;
				case 0xa0:
	   			{
	    			PSTARS_VAL=Pstar_a0[PSTARSINT[0]];
	   			}
	   			break;
				case 0x9d:
	   			{
	    			PSTARS_VAL=Pstar_9d[PSTARSINT[0]];
	   			}
	   			break;
				case 0x90:
	   			{
	    			PSTARS_VAL=Pstar_90[PSTARSINT[0]];
	   			}
	   			break;
				case 0x8c:
	   			{
	    			PSTARS_VAL=Pstar_8c[PSTARSINT[0]];
	   			}
	   			break;
				case 0x80:
	   			{
	    			PSTARS_VAL=Pstar_80[PSTARSINT[0]];
	   			}
	   			break;
				default:
					 PSTARS_VAL=0x890000;
				   logerror("PSTARS PC(%06x) UNKNOWN %4X %4X\n",cpu_get_pc(space->cpu),PSTARSINT[1],PSTARSINT[0]);

		}

	}
}

/*** ASIC 3 (oriental legends protection) ****************************************/

static UINT8 asic3_reg, asic3_latch[3], asic3_x, asic3_y, asic3_z, asic3_h1, asic3_h2;
static UINT16 asic3_hold;

static UINT32 bt(UINT32 v, int bit)
{
	return (v & (1<<bit)) != 0;
}

static void asic3_compute_hold(running_machine *machine)
{
	// The mode is dependant on the region
	static const int modes[4] = { 1, 1, 3, 2 };
	int mode = modes[input_port_read(machine, "Region") & 3];

	switch(mode) {
	case 1:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 1)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 14);
		break;
	case 2:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 7)^bt(asic3_hold, 6)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	case 3:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	}
}

READ16_HANDLER( pgm_asic3_r )
{
	UINT8 res = 0;
	/* region is supplied by the protection device */

	switch(asic3_reg) {
	case 0x00: res = (asic3_latch[0] & 0xf7) | ((input_port_read(space->machine, "Region") << 3) & 0x08); break;
	case 0x01: res = asic3_latch[1]; break;
	case 0x02: res = (asic3_latch[2] & 0x7f) | ((input_port_read(space->machine, "Region") << 6) & 0x80); break;
	case 0x03:
		res = (bt(asic3_hold, 15) << 0)
			| (bt(asic3_hold, 12) << 1)
			| (bt(asic3_hold, 13) << 2)
			| (bt(asic3_hold, 10) << 3)
			| (bt(asic3_hold,  7) << 4)
			| (bt(asic3_hold,  9) << 5)
			| (bt(asic3_hold,  2) << 6)
			| (bt(asic3_hold,  5) << 7);
		break;
	case 0x20: res = 0x49; break;
	case 0x21: res = 0x47; break;
	case 0x22: res = 0x53; break;
	case 0x24: res = 0x41; break;
	case 0x25: res = 0x41; break;
	case 0x26: res = 0x7f; break;
	case 0x27: res = 0x41; break;
	case 0x28: res = 0x41; break;
	case 0x2a: res = 0x3e; break;
	case 0x2b: res = 0x41; break;
	case 0x2c: res = 0x49; break;
	case 0x2d: res = 0xf9; break;
	case 0x2e: res = 0x0a; break;
	case 0x30: res = 0x26; break;
	case 0x31: res = 0x49; break;
	case 0x32: res = 0x49; break;
	case 0x33: res = 0x49; break;
	case 0x34: res = 0x32; break;
	}

	return res;
}

WRITE16_HANDLER( pgm_asic3_w )
{
	if(ACCESSING_BITS_0_7)
	{
		if(asic3_reg < 3)
			asic3_latch[asic3_reg] = data << 1;
		else if(asic3_reg == 0xa0) {
			asic3_hold = 0;
		} else if(asic3_reg == 0x40) {
			asic3_h2 = asic3_h1;
			asic3_h1 = data;
		} else if(asic3_reg == 0x48) {
			asic3_x = 0;
			if(!(asic3_h2 & 0x0a))
				asic3_x |= 8;
			if(!(asic3_h2 & 0x90))
				asic3_x |= 4;
			if(!(asic3_h1 & 0x06))
				asic3_x |= 2;
			if(!(asic3_h1 & 0x90))
				asic3_x |= 1;
		} else if(asic3_reg >= 0x80 && asic3_reg <= 0x87) {
			asic3_y = asic3_reg & 7;
			asic3_z = data;
			asic3_compute_hold(space->machine);
		}
	}
}

WRITE16_HANDLER( pgm_asic3_reg_w )
{
	if(ACCESSING_BITS_0_7)
		asic3_reg = data & 0xff;
}

/*** Knights of Valour / Sango / PhotoY2k Protection (from ElSemi) (ASIC28) ***/

static UINT16 ASIC28KEY;
static UINT16 ASIC28REGS[10];
static UINT16 ASICPARAMS[256];
static UINT16 ASIC28RCNT=0;
static const UINT32 B0TABLE[16]={2,0,1,4,3}; //maps char portraits to tables

// photo2yk bonus stage
static const UINT32 AETABLE[16]={0x00,0x0a,0x14,
		0x01,0x0b,0x15,
		0x02,0x0c,0x16};

//Not sure if BATABLE is complete
static const UINT32 BATABLE[0x40]=
	{0x00,0x29,0x2c,0x35,0x3a,0x41,0x4a,0x4e,  //0x00
     0x57,0x5e,0x77,0x79,0x7a,0x7b,0x7c,0x7d, //0x08
     0x7e,0x7f,0x80,0x81,0x82,0x85,0x86,0x87, //0x10
     0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x90,  //0x18
     0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,
     0x9e,0xa3,0xd4,0xa9,0xaf,0xb5,0xbb,0xc1};

static UINT32 E0REGS[16];


READ16_HANDLER (sango_protram_r)
{


	// at offset == 4 is region (supplied by device)
	// 0 = china
	// 1 = taiwan
	// 2 = japan
	// 3 = korea
	// 4 = hong kong
	// 5 = world

	if (offset == 4)	return input_port_read(space->machine, "Region");

	// otherwise it doesn't seem to use the ram for anything important, we return 0 to avoid test mode corruption
	// kovplus reads from offset 000e a lot ... why?
#ifdef MAME_DEBUG
	popmessage ("protection ram r %04x",offset);
#endif
	return 0x0000;
}

static UINT32 photoy2k_seqpos;
static UINT32 photoy2k_trf[3], photoy2k_soff;

#define BITSWAP10(val,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
                ((BIT(val, B9) <<  9) | \
                 (BIT(val, B8) <<  8) | \
                 (BIT(val, B7) <<  7) | \
                 (BIT(val, B6) <<  6) | \
                 (BIT(val, B5) <<  5) | \
                 (BIT(val, B4) <<  4) | \
                 (BIT(val, B3) <<  3) | \
                 (BIT(val, B2) <<  2) | \
                 (BIT(val, B1) <<  1) | \
                 (BIT(val, B0) <<  0))

static UINT32 photoy2k_spritenum(void)
{
	UINT32 base = photoy2k_seqpos & 0xffc00;
	UINT32 low  = photoy2k_seqpos & 0x003ff;

	switch((photoy2k_seqpos >> 10) & 0xf) {
	case 0x0:
	case 0xa:
		return base | (BITSWAP10(low, 0,8,3,1,5,9,4,2,6,7) ^ 0x124);
	case 0x1:
	case 0xb:
		return base | (BITSWAP10(low, 5,1,7,4,0,8,3,6,9,2) ^ 0x088);
	case 0x2:
	case 0x8:
		return base | (BITSWAP10(low, 3,5,9,7,6,4,1,8,2,0) ^ 0x011);
	case 0x3:
	case 0x9:
		return base | (BITSWAP10(low, 1,8,3,6,0,4,5,2,9,7) ^ 0x154);
	case 0x4:
	case 0xe:
		return base | (BITSWAP10(low, 2,1,7,4,5,8,3,6,9,0) ^ 0x0a9);
	case 0x5:
	case 0xf:
		return base | (BITSWAP10(low, 9,4,6,8,2,1,7,5,3,0) ^ 0x201);
	case 0x6:
	case 0xd:
		return base | (BITSWAP10(low, 4,6,0,8,9,7,3,5,1,2) ^ 0x008);
	case 0x7:
	case 0xc:
		return base | (BITSWAP10(low, 8,9,3,2,0,1,6,7,5,4) ^ 0x000);
	}
	return 0;
}

READ16_HANDLER (ASIC28_r16)
//UINT16 ASIC28_r16(UINT32 addr)
{
	UINT32 val=(ASIC28REGS[1]<<16)|(ASIC28REGS[0]);

//logerror("Asic28 Read PC = %06x Command = %02x ??\n",cpu_get_pc(space->cpu), ASIC28REGS[1]);

	switch(ASIC28REGS[1]&0xff)
	{

		case 0x20: // PhotoY2k spritenum conversion 4/4
			if(!ASIC28RCNT)
				logerror("ASIC28: PhotoY2K spr4 %04x %06x (%06x)\n", val & 0xffff, photoy2k_trf[2], cpu_get_pc(space->cpu));
			val = photoy2k_soff >> 16;
			break;

		case 0x21: // PhotoY2k spritenum conversion 3/4
			if(!ASIC28RCNT) {
				photoy2k_trf[2] = val & 0xffff;
				logerror("ASIC28: PhotoY2K spr3 %04x %06x (%06x)\n", val & 0xffff, photoy2k_trf[1], cpu_get_pc(space->cpu));
				if(photoy2k_trf[0] < 0x3c00)
					photoy2k_soff = pgmy2ks[photoy2k_trf[0]];
				else
					photoy2k_soff = 0;
				logerror("ASIC28: spriteval %04x, %06x -> %06x\n", photoy2k_trf[0], (photoy2k_trf[2]<<16)|photoy2k_trf[1], photoy2k_soff);
			}
			val = photoy2k_soff & 0xffff;
			break;

		case 0x22: // PhotoY2k spritenum conversion 2/4
			if(!ASIC28RCNT) {
				photoy2k_trf[1] = val & 0xffff;
				logerror("ASIC28: PhotoY2K spr2 %04x %06x (%06x)\n", val & 0xffff, photoy2k_trf[0], cpu_get_pc(space->cpu));
			}
			val = photoy2k_trf[0] | 0x880000;
			break;

		case 0x23: // PhotoY2k spritenum conversion 1/4
			if(!ASIC28RCNT) {
				photoy2k_trf[0] = val & 0xffff;
				logerror("ASIC28: PhotoY2K spr1 %04x (%06x)\n", val & 0xffff, cpu_get_pc(space->cpu));
			}
			val = 0x880000;
			break;

		case 0x30: // PhotoY2k next element
			if(!ASIC28RCNT)
				photoy2k_seqpos++;
			val = photoy2k_spritenum();
			if(!ASIC28RCNT)
				logerror("ASIC28: PhotoY2K seq_next  %05x -> %06x (%06x)\n", photoy2k_seqpos, val, cpu_get_pc(space->cpu));
			break;

		case 0x32: // PhotoY2k start of sequence
			if(!ASIC28RCNT)
				photoy2k_seqpos = (val & 0xffff) << 4;
			val = photoy2k_spritenum();
			if(!ASIC28RCNT)
				logerror("ASIC28: PhotoY2K seq_start %05x -> %06x (%06x)\n", photoy2k_seqpos, val, cpu_get_pc(space->cpu));
			break;

		case 0x99:
			val=0x880000;
			break;

		case 0x9d:	// spr palette
			val=0xa00000+((ASIC28REGS[0]&0x1f)<<6);
			break;

		case 0xae:  //Photo Y2k Bonus stage
			val=AETABLE[ASIC28REGS[0]&0xf];
			break;

		case 0xb0:
			val=B0TABLE[ASIC28REGS[0]&0xf];
			break;

		case 0xb4:
			{
				int v2=ASIC28REGS[0]&0x0f;
				int v1=(ASIC28REGS[0]&0x0f00)>>8;
//              UINT16 tmp=E0REGS[v2];
				//E0REGS[v2]=E0REGS[v1];
				//E0REGS[v1]=tmp;
				if(ASIC28REGS[0]==0x102)
					E0REGS[1]=E0REGS[0];
				else
					E0REGS[v1]=E0REGS[v2];

				val=0x880000;
			}
			break;

		case 0xba:
			val=BATABLE[ASIC28REGS[0]&0x3f];
			if(ASIC28REGS[0]>0x2f)
			{
//              PutMessage("Unmapped BA com, report ElSemi",60);
				popmessage	("Unmapped BA com %02x, contact ElSemi / MameDev", ASIC28REGS[0]);
			}
			break;

		case 0xc0:
			val=0x880000;
			break;

		case 0xc3:	//TXT tile position Uses C0 to select column
			{
				val=0x904000+(ASICPARAMS[0xc0]+ASICPARAMS[0xc3]*64)*4;
			}
			break;

		case 0xcb:
			val=0x880000;
			break;

		case 0xcc: //BG
   			{
   	 		int y=ASICPARAMS[0xcc];
    		if(y&0x400)    //y is signed (probably x too and it also applies to TXT, but I've never seen it used)
     			y=-(0x400-(y&0x3ff));
    		val=0x900000+(((ASICPARAMS[0xcb]+(y)*64)*4)/*&0x1fff*/);
   			}
   			break;

		case 0xd0:	//txt palette
			val=0xa01000+(ASIC28REGS[0]<<5);
			break;

		case 0xd6:	//???? check it
			{
				int v2=ASIC28REGS[0]&0xf;
//              int v1=(ASIC28REGS[0]&0xf0)>>4;
				E0REGS[0]=E0REGS[v2];
				//E0REGS[v2]=0;
				val=0x880000;
			}
			break;

		case 0xdc:	//bg palette
			val=0xa00800+(ASIC28REGS[0]<<6);
			break;

		case 0xe0:	//spr palette
			val=0xa00000+((ASIC28REGS[0]&0x1f)<<6);
			break;

		case 0xe5:
			val=0x880000;
			break;

		case 0xe7:
			val=0x880000;
			break;

		case 0xf0:
			{
				val=0x00C000;
			}
			break;

		case 0xf8:
			val=E0REGS[ASIC28REGS[0]&0xf]&0xffffff;
			break;

		case 0xfc:	//Adjust damage level to char experience level
			{
			val=(ASICPARAMS[0xfc]*ASICPARAMS[0xfe])>>6;
			break;
			}

		case 0xfe:	//todo
			val=0x880000;
			break;


		default:
			{
				val=0x880000;
			}
	}

//  if(addr==0x500000)
	if(offset==0)
	{
		UINT16 d=val&0xffff;
		UINT16 realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		d^=realkey;
		return d;
	}
//  else if(addr==0x500002)
	else if(offset==1)
	{
		UINT16 d=val>>16;
		UINT16 realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		d^=realkey;
		ASIC28RCNT++;
		if(!(ASIC28RCNT&0xf))
		{
			ASIC28KEY+=0x100;
			ASIC28KEY&=0xFF00;
		}
		return d;
	}
	return 0xff;
}

WRITE16_HANDLER (ASIC28_w16)
//void ASIC28_w16(UINT32 addr,UINT16 data)
{
//  if(addr==0x500000)
	if(offset==0)
	{
		UINT16 realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		data^=realkey;
		ASIC28REGS[0]=data;
		return;
	}
//  if(addr==0x500002)
	if(offset==1)
	{
		UINT16 realkey;

		ASIC28KEY=data&0xff00;

		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		data^=realkey;
		ASIC28REGS[1]=data;
//      ErrorLogMessage("ASIC28 CMD %X  PARAM %X",ASIC28REGS[1],ASIC28REGS[0]);
		logerror("ASIC28 CMD %04x  PARAM %04x\n",ASIC28REGS[1],ASIC28REGS[0]);

		ASICPARAMS[ASIC28REGS[1]&0xff]=ASIC28REGS[0];
		if(ASIC28REGS[1]==0xE7)
		{
			UINT32 E0R=(ASICPARAMS[0xE7]>>12)&0xf;
			E0REGS[E0R]&=0xffff;
			E0REGS[E0R]|=ASIC28REGS[0]<<16;
		}
		if(ASIC28REGS[1]==0xE5)
		{
			UINT32 E0R=(ASICPARAMS[0xE7]>>12)&0xf;
			E0REGS[E0R]&=0xff0000;
			E0REGS[E0R]|=ASIC28REGS[0];
		}
		ASIC28RCNT=0;
	}
}

/* Dragon World 2 */

#define DW2BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))
//Use this handler for reading from 0xd80000-0xd80002
READ16_HANDLER (dw2_d80000_r )
{
//addr&=0xff;
// if(dw2reg<0x20) //NOT SURE!!
	{
		//The value at 0x80EECE is computed in the routine at 0x107c18
		UINT16 d=pgm_mainram[0xEECE/2];
		UINT16 d2=0;
		d=(d>>8)|(d<<8);
		DW2BITSWAP(d,d2,7 ,0);
		DW2BITSWAP(d,d2,4 ,1);
		DW2BITSWAP(d,d2,5 ,2);
		DW2BITSWAP(d,d2,2 ,3);
		DW2BITSWAP(d,d2,15,4);
		DW2BITSWAP(d,d2,1 ,5);
		DW2BITSWAP(d,d2,10,6);
		DW2BITSWAP(d,d2,13,7);
		// ... missing bitswaps here (8-15) there is not enough data to know them
		// the code only checks the lowest 8 bytes
		return d2;
	}
}

/* Dragon World 3

Dragon World 3 has 2 protection chips
ASIC022 and ASIC025
one of them also has an external data rom (encrypted?)

code below is ElSemi's preliminary code, it doesn't work properly and isn't used, much of the protection isn't understood */

#if 0
AddWriteArea(0xda0000,0xdaffff,0,dw3_w8,dw3_w16,dw3_w32);
AddReadArea (0xda0000,0xdaffff,0,dw3_r8,dw3_r16,dw3_r32);

#define DW3BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))

UINT16 dw3_Rw[8];
UINT8 *dw3_R=(UINT8 *) dw3_Rw;

UINT8 dw3_r8(UINT32 addr)
{
	if(addr>=0xDA5610 && addr<=0xDA5613)
		return *((UINT8 *) (dw3_R+((addr-0xDA5610)^1)));
	return 0;
}

UINT16 dw3_r16(UINT32 addr)
{
	if(addr>=0xDA5610 && addr<=0xDA5613)
		return *((UINT16 *) (dw3_R+(addr-0xDA5610)));
	return 0;
}

UINT32 dw3_r32(UINT32 addr)
{
	return 0;
}

void dw3_w8(UINT32 addr,UINT8 val)
{
	if(addr==0xDA5610)
		dw3_R[1]=val;
	if(addr==0xDA5611)
		dw3_R[0]=val;
	if(addr==0xDA5612)
		dw3_R[3]=val;
	if(addr==0xDA5613)
		dw3_R[2]=val;
}

void dw3_w16(UINT32 addr,UINT16 val)
{
	if(addr>=0xDA5610 && addr<=0xDA5613)
	{
		UINT16 *s=((UINT16 *) (dw3_R+(addr-0xDA5610)));
		*s=val;
		if(addr==0xDA5610)
		{
			if(val==1)
			{
				UINT16 v1=dw3_Rw[1];
				UINT16 v2=0;
				DW3BITSWAP(v1,v2,0,0);
				DW3BITSWAP(v1,v2,1,1);
				DW3BITSWAP(v1,v2,7,2);
				DW3BITSWAP(v1,v2,6,3);
				DW3BITSWAP(v1,v2,5,4);
				DW3BITSWAP(v1,v2,4,5);
				DW3BITSWAP(v1,v2,3,6);
				DW3BITSWAP(v1,v2,2,7);

				dw3_Rw[1]=v2;
			}
		}
	}

}


void dw3_w32(UINT32 addr,UINT32 val)
{

}
#endif



