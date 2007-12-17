/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright 1997, Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "driver.h"
#include "starwars.h"
#include "video/avgdvg.h"


/* Control select values for ADC_R */
#define kPitch		0
#define kYaw		1
#define kThrust		2

/* Constants for mathbox operations */
#define NOP			0x00
#define LAC			0x01
#define READ_ACC	0x02
#define M_HALT		0x04
#define INC_BIC		0x08
#define CLEAR_ACC	0x10
#define LDC			0x20
#define LDB			0x40
#define LDA			0x80

/* Debugging flag */
#define MATHDEBUG	0


UINT8 *starwars_mathram;
UINT8 *starwars_ram_overlay;

/* Local variables */
static UINT8 control_num = kPitch;

static int MPA; /* PROM address counter */
static int BIC; /* Block index counter  */

static UINT16 dvd_shift, quotient_shift; /* Divider shift registers */
static UINT16 divisor, dividend;         /* Divider latches */

/* Store decoded PROM elements */
static UINT8 *PROM_STR; /* Storage for instruction strobe only */
static UINT8 *PROM_MAS; /* Storage for direct address only */
static UINT8 *PROM_AM; /* Storage for address mode select only */


/* Local function prototypes */
static void run_mbox(void);



/*************************************
 *
 *  X2212 nvram store
 *
 *************************************/

WRITE8_HANDLER( starwars_nstore_w )
{
	memcpy (generic_nvram, starwars_ram_overlay, generic_nvram_size);
}

/*************************************
 *
 *  Output latch
 *
 *************************************/

WRITE8_HANDLER( starwars_out_w )
{
	switch (offset & 7)
	{
		case 0:		/* Coin counter 1 */
			coin_counter_w(0, data);
			break;

		case 1:		/* Coin counter 2 */
			coin_counter_w(1, data);
			break;

		case 2:		/* LED 3 */
			set_led_status(2, ~data & 0x80);
			break;

		case 3:		/* LED 2 */
			set_led_status(1, ~data & 0x80);
			break;

		case 4:		/* bank switch */
			memory_set_bank(1, (data >> 7) & 1);
			if (starwars_is_esb)
				memory_set_bank(2, (data >> 7) & 1);
			break;
		case 5:		/* reset PRNG */
			break;

		case 6:		/* LED 1 */
			set_led_status(0, ~data & 0x80);
			break;

		case 7:		/* NVRAM array recall */
			memcpy (starwars_ram_overlay, generic_nvram, generic_nvram_size);
			break;
	}
}



/*************************************
 *
 *  Input port 1
 *
 *************************************/

READ8_HANDLER( starwars_input_1_r )
{
	int x = readinputport(1);

	/* Kludge to enable Starwars Mathbox Self-test                  */
	/* The mathbox looks like it's running, from this address... :) */
	if (activecpu_get_pc() == 0xf978 || activecpu_get_pc() == 0xf655)
		x |= 0x80;

	/* set the AVG done flag */
	if (avgdvg_done())
		x |= 0x40;
	else
		x &= ~0x40;

	return x;
}



/*************************************
 *
 *  ADC input and control
 *
 *************************************/

READ8_HANDLER( starwars_adc_r )
{
	/* pitch */
	if (control_num == kPitch)
		return readinputport(4);

	/* yaw */
	else if (control_num == kYaw)
		return readinputport(5);

	/* default to unused thrust */
	else
		return 0;
}


WRITE8_HANDLER( starwars_adc_select_w )
{
	control_num = offset;
}



/*************************************
 *
 *  Mathbox initialization
 *
 *************************************/

void swmathbox_init(void)
{
	UINT8 *src = memory_region(REGION_USER2);
	int cnt, val;

	PROM_STR = auto_malloc(1024 * sizeof(PROM_STR[0]));
	PROM_MAS = auto_malloc(1024 * sizeof(PROM_MAS[0]));
	PROM_AM = auto_malloc(1024 * sizeof(PROM_AM[0]));

	for (cnt = 0; cnt < 1024; cnt++)
	{
		/* translate PROMS into 16 bit code */
		val  = (src[0x0c00 + cnt]      ) & 0x000f; /* Set LS nibble */
		val |= (src[0x0800 + cnt] <<  4) & 0x00f0;
		val |= (src[0x0400 + cnt] <<  8) & 0x0f00;
		val |= (src[0x0000 + cnt] << 12) & 0xf000; /* Set MS nibble */

		/* perform pre-decoding */
		PROM_STR[cnt] = (val >> 8) & 0x00ff;
		PROM_MAS[cnt] =  val       & 0x007f;
		PROM_AM[cnt]  = (val >> 7) & 0x0001;
	}
}



/*************************************
 *
 *  Mathbox reset
 *
 *************************************/

void swmathbox_reset(void)
{
	MPA = BIC = 0;
}



/*************************************
 *
 *  Mathbox execution
 *
 *************************************/

void run_mbox(void)
{
	static INT16 A, B, C;
	static INT32 ACC;

	int RAMWORD = 0;
	int MA_byte;
	int tmp;
	int M_STOP = 100000; /* Limit on number of instructions allowed before halt */
	int MA;
	int IP15_8, IP7, IP6_0; /* Instruction PROM values */


	logerror("Running Mathbox...\n");

	/* loop until finished */
	while (M_STOP > 0)
	{
		/* fetch the current instruction data */
		IP15_8 = PROM_STR[MPA];
		IP7    = PROM_AM[MPA];
		IP6_0  = PROM_MAS[MPA];

#if (MATHDEBUG)
		mame_printf_debug("\n(MPA:%x), Strobe: %x, IP7: %d, IP6_0:%x\n",MPA, IP15_8, IP7, IP6_0);
		mame_printf_debug("(BIC: %x), A: %x, B: %x, C: %x, ACC: %x\n",BIC,A,B,C,ACC);
#endif

		/* construct the current RAM address */
		if (IP7 == 0)
			MA = (IP6_0 & 3) | ((BIC & 0x01ff) << 2);	/* MA10-2 set to BIC8-0 */
		else
			MA = IP6_0;

		/* convert RAM offset to eight bit addressing (2kx8 rather than 1k*16)
            and apply base address offset */

		MA_byte = MA << 1;
		RAMWORD = (starwars_mathram[MA_byte + 1] & 0x00ff) | ((starwars_mathram[MA_byte] & 0x00ff) << 8);

//      logerror("MATH ADDR: %x, CPU ADDR: %x, RAMWORD: %x\n", MA, MA_byte, RAMWORD);

		/*
         * RAMWORD is the sixteen bit Math RAM value for the selected address
         * MA_byte is the base address of this location as seen by the main CPU
         * IP is the 16 bit instruction word from the PROM. IP7_0 have already
         * been used in the address selection stage
         * IP15_8 provide the instruction strobes
         */


		/* The accumulator is built from two ls299 (msb) and two ls164
         * (lsb). You can only read/write the 16 msb. The lsb are
         * used while adding up multiplication results giving better
         * accuracy.
         */

		/* 0x10 - CLEAR_ACC */
		if (IP15_8 & CLEAR_ACC)
		{
			ACC = 0;
		}

		/* 0x01 - LAC (also clears lsb)*/
		if (IP15_8 & LAC)
			ACC = (RAMWORD << 16);

		/* 0x02 - READ_ACC */
		if (IP15_8 & READ_ACC)
		{
			starwars_mathram[MA_byte+1] = ((ACC >> 16) & 0xff);
			starwars_mathram[MA_byte  ] = ((ACC >> 24) & 0xff);
		}

		/* 0x04 - M_HALT */
		if (IP15_8 & M_HALT)
			M_STOP = 0;

		/* 0x08 - INC_BIC */
		if (IP15_8 & INC_BIC)
			BIC = (BIC + 1) & 0x1ff; /* Restrict to 9 bits */

		/* 0x20 - LDC*/
		if (IP15_8 & LDC)
		{
			C = RAMWORD;

			/* This is a serial subtractor - multiplier (74ls384) -
             * accumulator. For the full calculation 33 GMCLK pulses
             * are generated. The calculation performed is:
             *
             * ACC = ACC + (A - B) * C
             *
             * 1. pulse: Bit 0 of A and B are subtracted. Bit 0 of the
             * multiplication between multiplicand C and 0 is
             * calculated (bit 0 of A-B is not yet at the multiplier
             * input). Bit 0 of ACC is added to 0 (again, 'real' results
             * from the previous operations are no yet there).
             *
             * 2. pulse: Bit 1 of A-B is calculated. Bit 1 of
             * mutliplication is calculated based on bit 0 of A-B and
             * bit 1 of C. Bit 1 of ACC is added to the multiplication
             * result from first pulse.
             *
             * 3. pulse: Bit 2 of A-B is calculated. Bit 2 of
             * mutliplication is calculated based on bit 1 of A-B and
             * bit 2 of C. Bit 2 of ACC is added to the multiplication
             * between bit 1 of C and bit 0 of A-B.
             *
             * etc.
             *
             * This pipeline causes the shifts between A-B, C and ACC.
             * The 32 bit ACC and one bit adder form a ring so it
             * takes 33 clock pulses to do a full rotation.
             */

			ACC += (((INT32)(A - B) << 1) * C) << 1;

			/* A and B are sign extended (requred by the ls384). After
             * multiplication they just contain the sign.
             */
			A = (A & 0x8000)? 0xffff: 0;
			B = (B & 0x8000)? 0xffff: 0;
		}

		/* 0x40 - LDB */
		if (IP15_8 & LDB)
			B = RAMWORD;

		/* 0x80 - LDA */
		if (IP15_8 & LDA)
			A = RAMWORD;

		/*
         * Now update the PROM address counter
         * Done like this because the top two bits are not part of the counter
         * This means that each of the four pages should wrap around rather than
         * leaking from one to another.  It may not matter, but I've put it in anyway
         */
		tmp = MPA + 1;
		MPA = (MPA & 0x0300) | (tmp & 0x00ff); /* New MPA value */

		M_STOP--; /* Decrease count */
	}
}



/*************************************
 *
 *  Pseudo-RNG read
 *
 *************************************/

READ8_HANDLER( swmathbx_prng_r )
{
	/*
     * The PRNG is a modified 23 bit LFSR. Taps are at 4 and 22 so the
     * resulting LFSR polynomial is,
     *
     * x^5 + x^{23} + 1
     *
     * which is prime. It has a loop length of 8388607. The feedback
     * bit is inverted so the PRNG can start with 0. Only 8 bits from
     * bit 8 to 15 can be read by the CPU. The PRNG runs constantly at
     * a clock speed of 3 MHz.
     */

	/* Use MAME's PRNG for now */
	return mame_rand(Machine);
}



/*************************************
 *
 *  Mathbox divider
 *
 *************************************/

READ8_HANDLER( swmathbx_reh_r )
{
	return (quotient_shift & 0xff00) >> 8;
}


READ8_HANDLER( swmathbx_rel_r )
{
	return quotient_shift & 0x00ff;
}


WRITE8_HANDLER( swmathbx_w )
{
	int i;

	data &= 0xff;	/* ASG 971002 -- make sure we only get bytes here */
	switch (offset)
	{
		case 0:	/* mw0 */
			MPA = data << 2;	/* Set starting PROM address */
			run_mbox();			/* and run the Mathbox */
			break;

		case 1:	/* mw1 */
			BIC = (BIC & 0x00ff) | ((data & 0x01) << 8);
			break;

		case 2:	/* mw2 */
			BIC = (BIC & 0x0100) | data;
			break;

		case 4: /* dvsrh */
			divisor = (divisor & 0x00ff) | (data << 8);
			dvd_shift = dividend;
			quotient_shift = 0;
			break;

		case 5: /* dvsrl */
			/* Note: Divide is triggered by write to low byte.  This is */
			/*       dependant on the proper 16 bit write order in the  */
			/*       6809 emulation (high bytes, then low byte).        */
			/*       If the Tie fighters look corrupt, he byte order of */
			/*       the 16 bit writes in the 6809 are backwards        */

			divisor = (divisor & 0xff00) | data;

			/*
             * Simple restoring division as shown in the
             * schematics. The algorithm produces the same "wrong"
             * results as the hardware if divisor < 2*dividend or
             * divisor > 0x8000.
             */
			for (i = 1; i < 16; i++)
			{
				quotient_shift <<= 1;
				if (((INT32)dvd_shift + (divisor ^ 0xffff) + 1) & 0x10000)
				{
					quotient_shift |= 1;
					dvd_shift = (dvd_shift + (divisor ^ 0xffff) + 1) << 1;
				}
				else
				{
					dvd_shift <<= 1;
				}
			}
			break;

		case 6: /* dvddh */
			dividend = (dividend & 0x00ff) | (data << 8);
			break;

		case 7: /* dvddl */
			dividend = (dividend & 0xff00) | (data);
			break;

		default:
			break;
	}
}
