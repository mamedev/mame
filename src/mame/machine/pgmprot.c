#include "driver.h"
#include "includes/pgm.h"



/*** ASIC27a (Puzzle Star) -- ARM but with no external rom? behaves a bit like KOV ***/


/*** (pstarS) ***/

static const int pstar_ba[0x1E]={
	0x02,0x00,0x00,0x01,0x00,0x03,0x00,0x00, //0
	0x02,0x00,0x06,0x00,0x22,0x04,0x00,0x03, //8
	0x00,0x00,0x06,0x00,0x20,0x07,0x00,0x03, //10
	0x00,0x21,0x01,0x00,0x00,0x63
};

static const int pstar_b0[0x10]={
	0x09,0x0A,0x0B,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x00,0x00,0x00,0x00
};

static const int pstar_ae[0x10]={
	0x5D,0x86,0x8C ,0x8B,0xE0,0x8B,0x62,0xAF,
	0xB6,0xAF,0x10A,0xAF,0x00,0x00,0x00,0x00
};

static const int pstar_a0[0x10]={
	0x02,0x03,0x04,0x05,0x06,0x01,0x0A,0x0B,
	0x0C,0x0D,0x0E,0x09,0x00,0x00,0x00,0x00,
};

static const int pstar_9d[0x10]={
	0x05,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static const int pstar_90[0x10]={
	0x0C,0x10,0x0E,0x0C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const int pstar_8c[0x23]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,
	0x03,0x03,0x03
};

static const int pstar_80[0x1a3]={
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




READ16_HANDLER( pstars_protram_r )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if (offset == 4)		//region
		return input_port_read(space->machine, "Region");
	else if (offset >= 0x10)  //timer
	{
		logerror("PSTARS ACCESS COUNTER %6X\n", state->pstar_ram[offset - 0x10]);
		return state->pstar_ram[offset - 0x10]--;
	}
	return 0x0000;
}

READ16_HANDLER( pstars_r )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if (offset == 0)
	{
		UINT16 d = state->pstars_val & 0xffff;
		UINT16 realkey = state->pstars_key >> 8;
		realkey |= state->pstars_key;
		d ^= realkey;
//      logerror("PSTARS A27 R  %6X\n", state->pstars_val);
		return d;
	}
	else if (offset == 1)
	{
		UINT16 d = state->pstars_val >> 16;
		UINT16 realkey = state->pstars_key >> 8;
		realkey |= state->pstars_key;
		d ^= realkey;
		return d;

	}
	return 0xff;
}

WRITE16_HANDLER( pstars_w )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if (offset == 0)
	{
		state->pstars_int[0] = data;
		return;
	}

	if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			state->pstars_key = 0xff00;
		realkey = state->pstars_key >> 8;
		realkey |= state->pstars_key;
		{
			state->pstars_key += 0x100;
			state->pstars_key &= 0xff00;
			if (state->pstars_key == 0xff00)
				state->pstars_key = 0x100;
		}
		data ^= realkey;
		state->pstars_int[1] = data;
		state->pstars_int[0] ^= realkey;

		switch (state->pstars_int[1] & 0xff)
		{
		case 0x99:
			state->pstars_key = 0x100;
			state->pstars_val = 0x880000;
			break;

		case 0xe0:
			state->pstars_val = 0xa00000 + (state->pstars_int[0] << 6);
			break;

		case 0xdc:
			state->pstars_val = 0xa00800 + (state->pstars_int[0] << 6);
			break;

		case 0xd0:
			state->pstars_val = 0xa01000 + (state->pstars_int[0] << 5);
			break;

		case 0xb1:
			state->pstar_b1 = state->pstars_int[0];
			state->pstars_val = 0x890000;
			break;

		case 0xbf:
			state->pstars_val = state->pstar_b1 * state->pstars_int[0];
			break;

		case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
			state->pstars_val = 0;
			break;

		case 0xce: //TODO:TIMER  0,1,2
			state->pstar_ce = state->pstars_int[0];
			state->pstars_val=0x890000;
			break;

		case 0xcf: //TODO:TIMER  0,1,2
			state->pstar_ram[state->pstar_ce] = state->pstars_int[0];
			state->pstars_val = 0x890000;
			break;

		case 0xe7:
			state->pstar_e7 = (state->pstars_int[0] >> 12) & 0xf;
			state->pstars_regs[state->pstar_e7] &= 0xffff;
			state->pstars_regs[state->pstar_e7] |= (state->pstars_int[0] & 0xff) << 16;
			state->pstars_val = 0x890000;
			break;

		case 0xe5:
			state->pstars_regs[state->pstar_e7] &= 0xff0000;
			state->pstars_regs[state->pstar_e7] |= state->pstars_int[0];
			state->pstars_val = 0x890000;
			break;

		case 0xf8: //@73C
			state->pstars_val = state->pstars_regs[state->pstars_int[0] & 0xf] & 0xffffff;
			break;

		case 0xba:
			state->pstars_val = pstar_ba[state->pstars_int[0]];
			break;

		case 0xb0:
			state->pstars_val = pstar_b0[state->pstars_int[0]];
			break;

		case 0xae:
			state->pstars_val = pstar_ae[state->pstars_int[0]];
			break;

		case 0xa0:
			state->pstars_val = pstar_a0[state->pstars_int[0]];
   			break;

		case 0x9d:
			state->pstars_val = pstar_9d[state->pstars_int[0]];
			break;

		case 0x90:
			state->pstars_val = pstar_90[state->pstars_int[0]];
			break;

		case 0x8c:
			state->pstars_val = pstar_8c[state->pstars_int[0]];
			break;

		case 0x80:
	   		state->pstars_val = pstar_80[state->pstars_int[0]];
			break;

		default:
			state->pstars_val = 0x890000;
			logerror("PSTARS PC(%06x) UNKNOWN %4X %4X\n", cpu_get_pc(space->cpu), state->pstars_int[1], state->pstars_int[0]);

		}

	}
}

/*** ASIC 3 (oriental legends protection) ****************************************/

static void asic3_compute_hold(running_machine *machine)
{
	pgm_state *state = (pgm_state *)machine->driver_data;

	// The mode is dependent on the region
	static const int modes[4] = { 1, 1, 3, 2 };
	int mode = modes[input_port_read(machine, "Region") & 3];

	switch (mode)
	{
	case 1:
		state->asic3_hold =
			(state->asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->asic3_hold, 15) ^ BIT(state->asic3_hold, 10) ^ BIT(state->asic3_hold, 8) ^ BIT(state->asic3_hold, 5)
			 ^ BIT(state->asic3_z, state->asic3_y)
			 ^ (BIT(state->asic3_x, 0) << 1) ^ (BIT(state->asic3_x, 1) << 6) ^ (BIT(state->asic3_x, 2) << 10) ^ (BIT(state->asic3_x, 3) << 14);
		break;
	case 2:
		state->asic3_hold =
			(state->asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->asic3_hold, 15) ^ BIT(state->asic3_hold, 7) ^ BIT(state->asic3_hold, 6) ^ BIT(state->asic3_hold, 5)
			 ^ BIT(state->asic3_z, state->asic3_y)
			 ^ (BIT(state->asic3_x, 0) << 4) ^ (BIT(state->asic3_x, 1) << 6) ^ (BIT(state->asic3_x, 2) << 10) ^ (BIT(state->asic3_x, 3) << 12);
		break;
	case 3:
		state->asic3_hold =
			(state->asic3_hold << 1)
			 ^ 0x2bad
			 ^ BIT(state->asic3_hold, 15) ^ BIT(state->asic3_hold, 10) ^ BIT(state->asic3_hold, 8) ^ BIT(state->asic3_hold, 5)
			 ^ BIT(state->asic3_z, state->asic3_y)
			 ^ (BIT(state->asic3_x, 0) << 4) ^ (BIT(state->asic3_x, 1) << 6) ^ (BIT(state->asic3_x, 2) << 10) ^ (BIT(state->asic3_x, 3) << 12);
		break;
	}
}

READ16_HANDLER( pgm_asic3_r )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;
	UINT8 res = 0;
	/* region is supplied by the protection device */

	switch (state->asic3_reg)
	{
	case 0x00: res = (state->asic3_latch[0] & 0xf7) | ((input_port_read(space->machine, "Region") << 3) & 0x08); break;
	case 0x01: res = state->asic3_latch[1]; break;
	case 0x02: res = (state->asic3_latch[2] & 0x7f) | ((input_port_read(space->machine, "Region") << 6) & 0x80); break;
	case 0x03:
		res = (BIT(state->asic3_hold, 15) << 0)
			| (BIT(state->asic3_hold, 12) << 1)
			| (BIT(state->asic3_hold, 13) << 2)
			| (BIT(state->asic3_hold, 10) << 3)
			| (BIT(state->asic3_hold, 7) << 4)
			| (BIT(state->asic3_hold, 9) << 5)
			| (BIT(state->asic3_hold, 2) << 6)
			| (BIT(state->asic3_hold, 5) << 7);
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
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if(ACCESSING_BITS_0_7)
	{
		if (state->asic3_reg < 3)
			state->asic3_latch[state->asic3_reg] = data << 1;
		else if (state->asic3_reg == 0xa0)
			state->asic3_hold = 0;
		else if (state->asic3_reg == 0x40)
		{
			state->asic3_h2 = state->asic3_h1;
			state->asic3_h1 = data;
		}
		else if (state->asic3_reg == 0x48)
		{
			state->asic3_x = 0;
			if (!(state->asic3_h2 & 0x0a))
				state->asic3_x |= 8;
			if (!(state->asic3_h2 & 0x90))
				state->asic3_x |= 4;
			if (!(state->asic3_h1 & 0x06))
				state->asic3_x |= 2;
			if (!(state->asic3_h1 & 0x90))
				state->asic3_x |= 1;
		}
		else if(state->asic3_reg >= 0x80 && state->asic3_reg <= 0x87)
		{
			state->asic3_y = state->asic3_reg & 7;
			state->asic3_z = data;
			asic3_compute_hold(space->machine);
		}
	}
}

WRITE16_HANDLER( pgm_asic3_reg_w )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if(ACCESSING_BITS_0_7)
		state->asic3_reg = data & 0xff;
}

/*** Knights of Valour / Sango / PhotoY2k Protection (from ElSemi) (ASIC28) ***/

static const UINT32 B0TABLE[16] = {2, 0, 1, 4, 3}; //maps char portraits to tables

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



READ16_HANDLER( sango_protram_r )
{
	// at offset == 4 is region (supplied by device)
	// 0 = china
	// 1 = taiwan
	// 2 = japan
	// 3 = korea
	// 4 = hong kong
	// 5 = world

	if (offset == 4)
		return input_port_read(space->machine, "Region");

	// otherwise it doesn't seem to use the ram for anything important, we return 0 to avoid test mode corruption
	// kovplus reads from offset 000e a lot ... why?
#ifdef MAME_DEBUG
	popmessage ("protection ram r %04x",offset);
#endif
	return 0x0000;
}

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


READ16_HANDLER( asic28_r )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;
	UINT32 val = (state->asic28_regs[1] << 16) | (state->asic28_regs[0]);

	//logerror("Asic28 Read PC = %06x Command = %02x ??\n", cpu_get_pc(space->cpu), state->asic28_regs[1]);

	switch (state->asic28_regs[1] & 0xff)
	{
		case 0x99:
			val = 0x880000;
			break;

		case 0x9d:	// spr palette
			val = 0xa00000 + ((state->asic28_regs[0] & 0x1f) << 6);
			break;

		case 0xb0:
			val = B0TABLE[state->asic28_regs[0] & 0xf];
			break;

		case 0xb4:
			{
				//UINT16 tmp = state->eoregs[v2];
				int v2 = state->asic28_regs[0] & 0x0f;
				int v1 = (state->asic28_regs[0] & 0x0f00) >> 8;
				//state->eoregs[v2] = state->eoregs[v1];
				//state->eoregs[v1] = tmp;
				if (state->asic28_regs[0] == 0x102)
					state->eoregs[1] = state->eoregs[0];
				else
					state->eoregs[v1] = state->eoregs[v2];

				val = 0x880000;
			}
			break;

		case 0xba:
			val = BATABLE[state->asic28_regs[0] & 0x3f];
			if (state->asic28_regs[0] > 0x2f)
				popmessage("Unmapped BA com %02x, contact ElSemi / MameDev", state->asic28_regs[0]);
			break;

		case 0xc0:
			val = 0x880000;
			break;

		case 0xc3:	//TXT tile position Uses C0 to select column
			val = 0x904000 + (state->asic_params[0xc0] + state->asic_params[0xc3] * 64) * 4;
			break;

		case 0xcb:
			val = 0x880000;
			break;

		case 0xcc: //BG
   			{
   	 		int y = state->asic_params[0xcc];
			if (y & 0x400)    //y is signed (probably x too and it also applies to TXT, but I've never seen it used)
				y =- (0x400 - (y & 0x3ff));
			val = 0x900000 + (((state->asic_params[0xcb] + (y) * 64) * 4) /*&0x1fff*/);
   			}
   			break;

		case 0xd0:	//txt palette
			val = 0xa01000 + (state->asic28_regs[0] << 5);
			break;

		case 0xd6:	//???? check it
			{
				int v2 = state->asic28_regs[0] & 0xf;
				//int v1 = (state->asic28_regs[0] & 0xf0) >> 4;
				state->eoregs[0] = state->eoregs[v2];
				//state->eoregs[v2] = 0;
				val = 0x880000;
			}
			break;

		case 0xdc:	//bg palette
			val = 0xa00800 + (state->asic28_regs[0] << 6);
			break;

		case 0xe0:	//spr palette
			val = 0xa00000 + ((state->asic28_regs[0] & 0x1f) << 6);
			break;

		case 0xe5:
			val = 0x880000;
			break;

		case 0xe7:
			val = 0x880000;
			break;

		case 0xf0:
			val = 0x00C000;
			break;

		case 0xf8:
			val = state->eoregs[state->asic28_regs[0] & 0xf] & 0xffffff;
			break;

		case 0xfc:	//Adjust damage level to char experience level
			{
			val = (state->asic_params[0xfc] * state->asic_params[0xfe]) >> 6;
			break;
			}

		case 0xfe:	//todo
			val = 0x880000;
			break;


		default:
			val = 0x880000;
	}

	if(offset == 0)
	{
		UINT16 d = val & 0xffff;
		UINT16 realkey = state->asic28_key >> 8;
		realkey |= state->asic28_key;
		d ^= realkey;
		return d;
	}
	else if (offset == 1)
	{
		UINT16 d = val >> 16;
		UINT16 realkey = state->asic28_key >> 8;
		realkey |= state->asic28_key;
		d ^= realkey;
		state->asic28_rcnt++;
		if (!(state->asic28_rcnt & 0xf))
		{
			state->asic28_key += 0x100;
			state->asic28_key &= 0xff00;
		}
		return d;
	}
	return 0xff;
}

WRITE16_HANDLER( asic28_w )
{
	pgm_state *state = (pgm_state *)space->machine->driver_data;

	if (offset == 0)
	{
		UINT16 realkey =state->asic28_key >> 8;
		realkey |= state->asic28_key;
		data ^= realkey;
		state->asic28_regs[0] = data;
		return;
	}
	if (offset == 1)
	{
		UINT16 realkey;

		state->asic28_key = data & 0xff00;

		realkey = state->asic28_key >> 8;
		realkey |= state->asic28_key;
		data ^= realkey;
		state->asic28_regs[1] = data;
		logerror("ASIC28 CMD %04x  PARAM %04x\n", state->asic28_regs[1], state->asic28_regs[0]);

		state->asic_params[state->asic28_regs[1] & 0xff] = state->asic28_regs[0];
		if (state->asic28_regs[1] == 0xE7)
		{
			UINT32 E0R = (state->asic_params[0xe7] >> 12) & 0xf;
			state->eoregs[E0R] &= 0xffff;
			state->eoregs[E0R] |= state->asic28_regs[0] << 16;
		}
		if (state->asic28_regs[1]==0xE5)
		{
			UINT32 E0R = (state->asic_params[0xe7] >> 12) & 0xf;
			state->eoregs[E0R] &= 0xff0000;
			state->eoregs[E0R] |= state->asic28_regs[0];
		}
		state->asic28_rcnt = 0;
	}
}

/* Dragon World 2 */

#define DW2BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))
//Use this handler for reading from 0xd80000-0xd80002
READ16_HANDLER( dw2_d80000_r )
{
// addr&=0xff;
// if(dw2reg<0x20) //NOT SURE!!
	{
		//The value at 0x80EECE is computed in the routine at 0x107c18
		UINT16 d = pgm_mainram[0xEECE/2];
		UINT16 d2 = 0;
		d = (d >> 8) | (d << 8);
		DW2BITSWAP(d, d2, 7,  0);
		DW2BITSWAP(d, d2, 4,  1);
		DW2BITSWAP(d, d2, 5,  2);
		DW2BITSWAP(d, d2, 2,  3);
		DW2BITSWAP(d, d2, 15, 4);
		DW2BITSWAP(d, d2, 1,  5);
		DW2BITSWAP(d, d2, 10, 6);
		DW2BITSWAP(d, d2, 13, 7);
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
