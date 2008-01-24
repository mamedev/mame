/*******************************************************************************

Equites           (c) 1984 Alpha Denshi Co./Sega
Bull Fighter      (c) 1984 Alpha Denshi Co./Sega
The Koukouyakyuh  (c) 1985 Alpha Denshi Co.
Splendor Blast    (c) 1985 Alpha Denshi Co.
High Voltage      (c) 1985 Alpha Denshi Co.

drivers by Acho A. Tang

*******************************************************************************/
// Directives

#include "driver.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/dac.h"

#define MCU_RTNMSB 0x80

/******************************************************************************/
// Imports

extern UINT16 *equites_workram;

/******************************************************************************/
// Locals

static const UINT16 e_ent_addr[8][4] =
{
	{0xce25, 0xcd3c, 0xcd89, 0xcdd5},
	{     0, 0xce6e, 0xcebe, 0xcff2},
	{     0, 0xcf07, 0xcf55, 0xcfa2},
	{     0, 0xcff2, 0xd03a,      0},
	{     0, 0xd08e, 0xd0e1, 0xd132},
	{     0, 0xd17a,      0,      0},
	{     0, 0xd1c0, 0xcff2,      0},
	{     0,      0,      0,      0},
};

static const UINT16 e_exit_pos[8][4] =
{
	{0x1878, 0x4878, 0x7878, 0x3878},
	{     0, 0x3878, 0x4878, 0x4878},
	{     0, 0x3878, 0x0060, 0x4878},
	{     0, 0x0060, 0x4878,      0},
	{     0, 0x7878, 0x4878, 0x4878},
	{     0, 0x4878,      0,      0},
	{     0, 0x3878, 0x4878,      0},
	{     0,      0,      0,      0},
};

static const UINT16 e_swap_addr[4][4] =
{
	{     0, 0x92ec,      0, 0x92ec},
	{     0, 0x92ec,      0, 0x92ec},
	{     0,      0,      0, 0x92ec},
	{     0, 0x92d2,      0, 0x92d2},
};

static const UINT16 e_respawn_addr[4][4] =
{
	{     0, 0x9382,      0, 0x9382},
	{     0, 0x9382,      0, 0x9382},
	{     0,      0,      0, 0x9382},
	{     0, 0x0cc8,      0, 0x9382},
};

static const UINT16 h_respawn_addr[2][4] =
{
	{0x1026, 0x0fb6, 0x0fb6, 0x0fb6},
	{0x11ac, 0x20b0, 0x1c44, 0x1996},
};

static const UINT16 s_respawn_addr[3][6] =
{
	{0x0b6a, 0x0b52, 0x29da, 0x0846, 0x1610, 0x0c84}, // game over seq
	{0x347e, 0x0e10, 0x0c2c, 0x0c2c, 0x0c2c, 0x0c2c}, // depth seq
	{0x0c2e, 0x1fbe, 0x166a, 0x0c84, 0x0c2c, 0x0c2c}, // level change seq
};

static const UINT16 s_lvdata_addr[6] = {0xccc2, 0xd04a, 0xd408, 0xd796, 0xdaa8, 0xdbd6};

static const UINT16 s_objdata_addr[6] = {0xb7ce, 0xba64, 0xbdbc, 0xc0f2, 0xc446, 0xc810};

static const UINT8 s_spawn_list[8] = {0x07, 0x08, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26};

static const UINT8 s_pow_list[8] = {0, 0, 2, 2, 2, 2, 2, 1};

static struct MRULE
{
	struct MRULE *next;
	UINT16 pc;
	UINT8 data;
	UINT8 mode;
} *mrulepool;

static struct MRULELIST
{
	struct MRULE *head;
	struct MRULE *tail;
} *mrulemap;

/******************************************************************************/
// Exports

UINT16 *equites_8404ram;

/******************************************************************************/
// Local Functions

static TIMER_CALLBACK( equites_synth_callback )
{
	static int parity = 0;

	if (parity^=1)
		cpunum_set_input_line(machine, 1, I8085_INTR_LINE, HOLD_LINE);
	cpunum_set_input_line(machine, 1, I8085_RST75_LINE, HOLD_LINE);
}

// Optimized Mersenne Twister - courtesy of Shawn J. Cokus, University of Washington

typedef UINT32 uint32;

#define N              (624)                 // length of state vector
#define M              (397)                 // a period parameter
#define K              (0x9908B0DFU)         // a magic constant
#define hiBit(u)       ((u) & 0x80000000U)   // mask all but highest   bit of u
#define loBit(u)       ((u) & 0x00000001U)   // mask all but lowest    bit of u
#define loBits(u)      ((u) & 0x7FFFFFFFU)   // mask     the highest   bit of u
#define mixBits(u, v)  (hiBit(u)|loBits(v))  // move hi bit of u to hi bit of v

static uint32   state[N+1];     // state vector + 1 extra to not violate ANSI C
static uint32   *next;          // next random value is computed from here
static int      left = -1;      // can *next++ this many times before reloading

static void seedMT(uint32 seed)
{
	register uint32 x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
	register int    j;

	for(left=0, *s++=x, j=N; --j;
		*s++ = (x*=69069U) & 0xFFFFFFFFU);
}

static uint32 reloadMT(void)
{
	register uint32 *p0=state, *p2=state+2, *pM=state+M, s0, s1;
	register int    j;

	if(left < -1)
		seedMT(4357U);

	left=N-1, next=state+1;

	for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
		*p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

	for(pM=state, j=M; --j; s0=s1, s1=*p2++)
		*p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

	s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
	s1 ^= (s1 >> 11);
	s1 ^= (s1 <<  7) & 0x9D2C5680U;
	s1 ^= (s1 << 15) & 0xEFC60000U;
	return(s1 ^ (s1 >> 18));
}

INLINE uint32 randomMT(void)
{
	uint32 y;

	if(--left < 0)
		return(reloadMT());

	y  = *next++;
	y ^= (y >> 11);
	y ^= (y <<  7) & 0x9D2C5680U;
	y ^= (y << 15) & 0xEFC60000U;
	return(y ^ (y >> 18));
}

/******************************************************************************/
// Export Functions

void equites_8404init(void)
{
	UINT8 *byte_ptr;

	byte_ptr = auto_malloc(0x8000);
	memset(byte_ptr, 0, 0x8000);

	mrulemap = (struct MRULELIST *)byte_ptr; // pointer table to rule lists
	mrulepool = (struct MRULE *)(byte_ptr + 0x4000); // rules pool

	timer_pulse(ATTOTIME_IN_HZ(106), NULL, 0, equites_synth_callback); // hand tuned

	seedMT(mame_rand(Machine));
}

void equites_8404rule(unsigned pc, int offset, int data)
{
	struct MRULELIST *listptr;

	mrulepool->pc = pc;

	if (data >= 0)
		mrulepool->data = data & 0xff;
	else
	{
		data = -data;
		if (data > 0x0f) data = (data>>4 & 0x0f) | MCU_RTNMSB;
		mrulepool->mode = data;
	}

	listptr = mrulemap + (offset>>1);

	if (!listptr->head)
		listptr->head = mrulepool;
	else
		listptr->tail->next = mrulepool;

	listptr->tail = mrulepool++;
}

/******************************************************************************/
// Export Handlers

READ16_HANDLER(equites_8404_r)
{
	int pc, data, mode, col, row;
	struct MRULE *ruleptr;

	if (ACCESSING_LSB)
	{
		pc = activecpu_get_pc();

		//logerror("%04x: 8404 reads offset %04x\n", pc, offset<<1);

		ruleptr = mrulemap[offset].head;
		while (ruleptr)
		{
			if (pc == ruleptr->pc)
			{
				mode = ruleptr->mode;
				if (!mode) return (ruleptr->data);
				switch (mode & 0xf)
				{
					case 1:
						col = equites_8404ram[0x47d>>1] & 3;
						row = equites_8404ram[0x47f>>1] & 7;
						data = e_ent_addr[row][col];
					break;

					case 2:
						col = equites_8404ram[0x47d>>1] & 3;
						row = equites_8404ram[0x47f>>1] & 7;
						data = e_exit_pos[row][col];
					break;

					case 3:
						data = equites_workram[0x130>>1];
					break;

					case 4:
						col = equites_8404ram[0x27b>>1] & 3;
						row = equites_8404ram[0x279>>1] & 3;
						data = e_respawn_addr[row][col];
					break;

					case 5:
						col = equites_8404ram[0x27b>>1] & 3;
						row = equites_8404ram[0x279>>1] & 3;
						data = e_swap_addr[row][col];
					break;

					case 6:
						col = offset - (0x493>>1);
						row = (equites_8404ram[0x495>>1] & 0xff) ? 1 : 0;
						data = h_respawn_addr[row][col>>1];
					break;

					case 7:
						col = offset - (0x5e3>>1);
						if ((equites_8404ram[0xf>>1] & 0xf) == 2)
						{
							if (col >= 0xb) equites_8404ram[0xf>>1] = 0;
							row = 2;
						}
						else
							row = (equites_8404ram[0x4f9>>1] & 0xff) ? 1 : 0;
						data = s_respawn_addr[row][col>>1];
					break;

					case 8:
						data = s_pow_list[randomMT()&7];
					break;

					case 9:
						data = s_spawn_list[randomMT()&7];
					break;

					case 0xa:
						data = s_objdata_addr[equites_workram[0x90>>1]%6];
					break;

					case 0xb:
						data = s_lvdata_addr[equites_workram[0x90>>1]%6];
					break;

					case 0xc:
						data = equites_workram[0x90>>1]%6;
					break;

					default:
						return (equites_8404ram[offset]);
				}
				if (mode & MCU_RTNMSB) data >>= 8;
				return (data & 0xff);
			}
			ruleptr = ruleptr->next;
		}
	}
	return (equites_8404ram[offset]);
}

WRITE8_HANDLER(equites_5232_w)
{
	if (offset < 0x08 && data) data |= 0x80; // gets around a current 5232 emulation restriction
	MSM5232_0_w(offset, data);
}

WRITE8_HANDLER(equites_8910control_w)
{
	AY8910_control_port_0_w(0, data);
}

WRITE8_HANDLER(equites_8910data_w)
{
	AY8910_write_port_0_w(0, data);
}

static WRITE8_HANDLER(equites_8910porta_w)
{
	// sync with one or more MSM5232 channels. MIDI out?
}

static WRITE8_HANDLER(equites_8910portb_w)
{
	// sync with one or more MSM5232 channels. MIDI out?
}

WRITE8_HANDLER(equites_dac0_w)
{
	DAC_signed_data_w(0, data<<2);
}

WRITE8_HANDLER(equites_dac1_w)
{
	DAC_signed_data_w(1, data<<2);
}

/******************************************************************************/
// Alpha "Soundboard 7" Chip Definitions

const struct MSM5232interface equites_5232intf =
{
	{ 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6 } // needs verification
};

const struct AY8910interface equites_8910intf =
{
	0,
	0,
	equites_8910porta_w,
	equites_8910portb_w
};

/******************************************************************************/
