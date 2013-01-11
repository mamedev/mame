/**********************************************************************************************

     TMS5110 simulator (modified from TMS5220 by Jarek Burczynski)

     Written for MAME by Frank Palazzolo
     With help from Neill Corlett
     Additional tweaking by Aaron Giles
     Various fixes by Lord Nightmare
     Additional enhancements by Couriersud
     Sub-interpolation-cycle parameter updating added by Lord Nightmare

     Todo:
        - implement CS
        - implement missing commands
        - TMS5110_CMD_TEST_TALK is only partially implemented

     TMS5100:

                 +-----------------+
        TST      |  1           28 |  CS
        PDC      |  2           27 |  CTL8
        ROM CK   |  3           26 |  ADD8
        CPU CK   |  4           25 |  CTL1
        VDD      |  5           24 |  ADD1
        CR OSC   |  6           23 |  CTL2
        RC OSC   |  7           22 |  ADD2
        T11      |  8           21 |  ADD4
        NC       |  9           20 |  CTL4
        I/O      | 10           19 |  M1
        SPK1     | 11           18 |  NC
        SPK2     | 12           17 |  NC
        PROM OUT | 13           16 |  NC
        VSS      | 14           15 |  M0
                 +-----------------+

        T11: Sync for serial data out


    M58817

    The following connections could be derived from radar scope schematics.
    The M58817 is not 100% pin compatible to the 5100, but really close.

                 +-----------------+
        (NC)     |  1           28 |  CS
        PDC      |  2           27 |  CTL8
        ROM CK   |  3           26 |  ADD8 (to 58819)
        (NC)     |  4           25 |  CTL1
        (VDD,-5) |  5           24 |  ADD1 (to 58819)
        (GND)    |  6           23 |  CTL2
        Xin      |  7           22 |  ADD2 (to 58819)
        Xout     |  8           21 |  ADD4 (to 58819)
        (NC)     |  9           20 |  CTL4
        (VDD,-5) | 10           19 |  Status back to CPU
        (NC)     | 11           18 |  C1 (to 58819)
        SPKR     | 12           17 |  (NC)
        SPKR     | 13           16 |  C0 (to 58819)
        (NC)     | 14           15 |  (5V)
                 +-----------------+

***********************************************************************************************/

#include "emu.h"
#include "tms5110.h"

#define MAX_SAMPLE_CHUNK        512
#define FIFO_SIZE               64 // TODO: technically the tms51xx chips don't have a fifo at all

/* Variants */

#define TMS5110_IS_5110A    (1)
#define TMS5110_IS_5100     (2)
#define TMS5110_IS_5110     (3)

#define TMS5110_IS_CD2801   TMS5110_IS_5100
#define TMS5110_IS_TMC0281  TMS5110_IS_5100

#define TMS5110_IS_CD2802   TMS5110_IS_5110
#define TMS5110_IS_M58817   TMS5110_IS_5110

/* States for CTL */

#define CTL_STATE_INPUT         (0)
#define CTL_STATE_OUTPUT        (1)
#define CTL_STATE_NEXT_OUTPUT   (2)

struct tms5110_state
{
	/* coefficient tables */
	int variant;                /* Variant of the 5110 - see tms5110.h */

	/* coefficient tables */
	const struct tms5100_coeffs *coeff;

	/* these contain data that describes the 64 bits FIFO */
	UINT8 fifo[FIFO_SIZE];
	UINT8 fifo_head;
	UINT8 fifo_tail;
	UINT8 fifo_count;

	/* these contain global status bits */
	UINT8 PDC;
	UINT8 CTL_pins;
	UINT8 speaking_now;
	UINT8 talk_status;
	UINT8 state;

	/* Rom interface */
	UINT32 address;
	UINT8  next_is_address;
	UINT8  schedule_dummy_read;
	UINT8  addr_bit;

	/* external callback */
	int (*M0_callback)(device_t *);
	void (*set_load_address)(device_t *, int);

	/* callbacks */
	devcb_resolved_write_line m0_func;      /* the M0 line */
	devcb_resolved_write_line m1_func;      /* the M1 line */
	devcb_resolved_write8 addr_func;        /* Write to ADD1,2,4,8 - 4 address bits */
	devcb_resolved_read_line data_func;     /* Read one bit from ADD8/Data - voice data */
	devcb_resolved_write_line romclk_func;  /* rom clock - Only used to drive the data lines */


	device_t *device;

	/* these contain data describing the current and previous voice frames */
	UINT16 old_energy;
	UINT16 old_pitch;
	INT32 old_k[10];

	UINT16 new_energy;
	UINT16 new_pitch;
	INT32 new_k[10];


	/* these are all used to contain the current state of the sound generation */
	UINT16 current_energy;
	UINT16 current_pitch;
	INT32 current_k[10];

	UINT16 target_energy;
	UINT16 target_pitch;
	INT32 target_k[10];

	UINT8 interp_count;       /* number of interp periods (0-7) */
	UINT8 sample_count;       /* sample number within interp (0-24) */
	INT32 pitch_count;

	INT32 x[11];

	INT32 RNG;  /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */

	const tms5110_interface *intf;
	const UINT8 *table;
	sound_stream *stream;
	INT32 speech_rom_bitnum;

	emu_timer *romclk_hack_timer;
	UINT8 romclk_hack_timer_started;
	UINT8 romclk_hack_state;
};

struct tmsprom_state
{
	/* Rom interface */
	UINT32 address;
	/* ctl lines */
	UINT8  m0;
	UINT8  enable;
	UINT32  base_address;
	UINT8  bit;

	int prom_cnt;

	int    clock;
	const UINT8 *rom;
	const UINT8 *prom;

	devcb_resolved_write_line pdc_func;     /* tms pdc func */
	devcb_resolved_write8 ctl_func;         /* tms ctl func */

	device_t *device;
	emu_timer *romclk_timer;

	const tmsprom_interface *intf;
};

/* Pull in the ROM tables */
#include "tms5110r.c"



INLINE tms5110_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS5110 ||
			device->type() == TMS5100 ||
			device->type() == TMS5110A ||
			device->type() == CD2801 ||
			device->type() == TMC0281 ||
			device->type() == CD2802 ||
			device->type() == M58817);
	return (tms5110_state *)downcast<tms5110_device *>(device)->token();
}

INLINE tmsprom_state *get_safe_token_prom(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMSPROM);
	return (tmsprom_state *)downcast<tmsprom_device *>(device)->token();
}

/* Static function prototypes */
static void tms5110_set_variant(tms5110_state *tms, int variant);
static void tms5110_PDC_set(tms5110_state *tms, int data);
static void tms5110_process(tms5110_state *tms, INT16 *buffer, unsigned int size);
static void parse_frame(tms5110_state *tms);
static STREAM_UPDATE( tms5110_update );
static TIMER_CALLBACK( romclk_hack_timer_cb );


#define DEBUG_5110  0

void tms5110_set_variant(tms5110_state *tms, int variant)
{
	switch (variant)
	{
		case TMS5110_IS_5110A:
			tms->coeff = &tms5110a_coeff;
			break;
		case TMS5110_IS_5100:
			tms->coeff = &pat4209836_coeff;
			break;
		case TMS5110_IS_5110:
			tms->coeff = &pat4403965_coeff;
			break;
		default:
			fatalerror("Unknown variant in tms5110_create\n");
	}

	tms->variant = variant;
}

static void new_int_write(tms5110_state *tms, UINT8 rc, UINT8 m0, UINT8 m1, UINT8 addr)
{
	if (!tms->m0_func.isnull())
		tms->m0_func(m0);
	if (!tms->m1_func.isnull())
		tms->m1_func(m1);
	if (!tms->addr_func.isnull())
		tms->addr_func(0, addr);
	if (!tms->romclk_func.isnull())
	{
		//printf("rc %d\n", rc);
		tms->romclk_func(rc);
	}
}

static void new_int_write_addr(tms5110_state *tms, UINT8 addr)
{
	new_int_write(tms, 1, 0, 1, addr);
	new_int_write(tms, 0, 0, 1, addr);
	new_int_write(tms, 1, 0, 0, addr);
	new_int_write(tms, 0, 0, 0, addr);
}

static UINT8 new_int_read(tms5110_state *tms)
{
	new_int_write(tms, 1, 1, 0, 0);
	new_int_write(tms, 0, 1, 0, 0);
	new_int_write(tms, 1, 0, 0, 0);
	new_int_write(tms, 0, 0, 0, 0);
	if (!tms->data_func.isnull())
		return tms->data_func();
	return 0;
}

static void register_for_save_states(tms5110_state *tms)
{
	tms->device->save_item(NAME(tms->fifo));
	tms->device->save_item(NAME(tms->fifo_head));
	tms->device->save_item(NAME(tms->fifo_tail));
	tms->device->save_item(NAME(tms->fifo_count));

	tms->device->save_item(NAME(tms->PDC));
	tms->device->save_item(NAME(tms->CTL_pins));
	tms->device->save_item(NAME(tms->speaking_now));
	tms->device->save_item(NAME(tms->talk_status));
	tms->device->save_item(NAME(tms->state));

	tms->device->save_item(NAME(tms->old_energy));
	tms->device->save_item(NAME(tms->old_pitch));
	tms->device->save_item(NAME(tms->old_k));

	tms->device->save_item(NAME(tms->new_energy));
	tms->device->save_item(NAME(tms->new_pitch));
	tms->device->save_item(NAME(tms->new_k));

	tms->device->save_item(NAME(tms->current_energy));
	tms->device->save_item(NAME(tms->current_pitch));
	tms->device->save_item(NAME(tms->current_k));

	tms->device->save_item(NAME(tms->target_energy));
	tms->device->save_item(NAME(tms->target_pitch));
	tms->device->save_item(NAME(tms->target_k));

	tms->device->save_item(NAME(tms->interp_count));
	tms->device->save_item(NAME(tms->sample_count));
	tms->device->save_item(NAME(tms->pitch_count));

	tms->device->save_item(NAME(tms->next_is_address));
	tms->device->save_item(NAME(tms->address));
	tms->device->save_item(NAME(tms->schedule_dummy_read));
	tms->device->save_item(NAME(tms->addr_bit));

	tms->device->save_item(NAME(tms->x));

	tms->device->save_item(NAME(tms->RNG));
}




/******************************************************************************************

     FIFO_data_write -- handle bit data write to the TMS5110 (as a result of toggling M0 pin)

******************************************************************************************/
static void FIFO_data_write(tms5110_state *tms, int data)
{
	/* add this bit to the FIFO */
	if (tms->fifo_count < FIFO_SIZE)
	{
		tms->fifo[tms->fifo_tail] = (data&1); /* set bit to 1 or 0 */

		tms->fifo_tail = (tms->fifo_tail + 1) % FIFO_SIZE;
		tms->fifo_count++;

		if (DEBUG_5110) logerror("Added bit to FIFO (size=%2d)\n", tms->fifo_count);
	}
	else
	{
		if (DEBUG_5110) logerror("Ran out of room in the FIFO!\n");
	}
}

/******************************************************************************************

     extract_bits -- extract a specific number of bits from the FIFO

******************************************************************************************/

static int extract_bits(tms5110_state *tms, int count)
{
	int val = 0;

	while (count--)
	{
		val = (val << 1) | (tms->fifo[tms->fifo_head] & 1);
		tms->fifo_count--;
		tms->fifo_head = (tms->fifo_head + 1) % FIFO_SIZE;
	}
	return val;
}

static void request_bits(tms5110_state *tms, int no)
{
int i;
	for (i=0; i<no; i++)
	{
		if (tms->M0_callback)
		{
			int data = (*tms->M0_callback)(tms->device);
			FIFO_data_write(tms, data);
		}
		else
		{
			//if (DEBUG_5110) logerror("-->ERROR: TMS5110 missing M0 callback function\n");
			UINT8 data = new_int_read(tms);
			FIFO_data_write(tms, data);
		}
	}
}

static void perform_dummy_read(tms5110_state *tms)
{
	if (tms->schedule_dummy_read)
	{
		if (tms->M0_callback)
		{
			int data = (*tms->M0_callback)(tms->device);

			if (DEBUG_5110) logerror("TMS5110 performing dummy read; value read = %1i\n", data&1);
		}
		else
		{
			int data = new_int_read(tms);

			if (DEBUG_5110) logerror("TMS5110 performing dummy read; value read = %1i\n", data&1);
			//if (DEBUG_5110) logerror("-->ERROR: TMS5110 missing M0 callback function\n");
		}
		tms->schedule_dummy_read = FALSE;
	}
}




/**********************************************************************************************

     tms5110_process -- fill the buffer with a specific number of samples

***********************************************************************************************/

void tms5110_process(tms5110_state *tms, INT16 *buffer, unsigned int size)
{
	int buf_count=0;
	int i, interp_period, bitout;
	INT16 Y11, cliptemp;

	/* if we're not speaking, fill with nothingness */
	if (!tms->speaking_now)
		goto empty;

	/* if we're to speak, but haven't started */
	if (!tms->talk_status)
	{
	/* a "dummy read" is mentioned in the tms5200 datasheet */
	/* The Bagman speech roms data are organized in such a way that
	** the bit at address 0 is NOT a speech data. The bit at address 1
	** is the speech data. It seems that the tms5110 performs a dummy read
	** just before it executes a SPEAK command.
	** This has been moved to command logic ...
	**  perform_dummy_read(tms);
	*/

		/* clear out the new frame parameters (it will become old frame just before the first call to parse_frame() ) */
		tms->new_energy = 0;
		tms->new_pitch = 0;
		for (i = 0; i < tms->coeff->num_k; i++)
			tms->new_k[i] = 0;

		tms->talk_status = 1;
	}


	/* loop until the buffer is full or we've stopped speaking */
	while ((size > 0) && tms->speaking_now)
	{
		int current_val;

		/* if we're ready for a new frame */
		if ((tms->interp_count == 0) && (tms->sample_count == 0))
		{
			/* remember previous frame */
			tms->old_energy = tms->new_energy;
			tms->old_pitch = tms->new_pitch;
			for (i = 0; i < tms->coeff->num_k; i++)
				tms->old_k[i] = tms->new_k[i];


			/* if the old frame was a stop frame, exit and do not process any more frames */
			if (tms->old_energy == COEFF_ENERGY_SENTINEL)
			{
				if (DEBUG_5110) logerror("processing frame: stop frame\n");
				tms->target_energy = tms->current_energy = 0;
				tms->speaking_now = tms->talk_status = 0;
				tms->interp_count = tms->sample_count = tms->pitch_count = 0;
				goto empty;
			}


			/* Parse a new frame into the new_energy, new_pitch and new_k[] */
			parse_frame(tms);


			/* Set old target as new start of frame */
			tms->current_energy = tms->old_energy;
			tms->current_pitch = tms->old_pitch;

			for (i = 0; i < tms->coeff->num_k; i++)
				tms->current_k[i] = tms->old_k[i];


			/* is this the stop (ramp down) frame? */
			if (tms->new_energy == COEFF_ENERGY_SENTINEL)
			{
				/*logerror("processing frame: ramp down\n");*/
				tms->target_energy = 0;
				tms->target_pitch = tms->old_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->old_k[i];
			}
			else if ((tms->old_energy == 0) && (tms->new_energy != 0)) /* was the old frame a zero-energy frame? */
			{
				/* if so, and if the new frame is non-zero energy frame then the new parameters
				   should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/

				/*logerror("processing non-zero energy frame after zero-energy frame\n");*/
				tms->target_energy = tms->new_energy;
				tms->target_pitch = tms->current_pitch = tms->new_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->current_k[i] = tms->new_k[i];
			}
			else if ((tms->old_pitch == 0) && (tms->new_pitch != 0))    /* is this a change from unvoiced to voiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: UNVOICED->VOICED frame change\n");*/
				tms->target_energy = tms->new_energy;
				tms->target_pitch = tms->current_pitch = tms->new_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->current_k[i] = tms->new_k[i];
			}
			else if ((tms->old_pitch != 0) && (tms->new_pitch == 0))    /* is this a change from voiced to unvoiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: VOICED->UNVOICED frame change\n");*/
				tms->target_energy = tms->new_energy;
				tms->target_pitch = tms->current_pitch = tms->new_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->current_k[i] = tms->new_k[i];
			}
			else
			{
				/*logerror("processing frame: Normal\n");*/
				/*logerror("*** Energy = %d\n",current_energy);*/
				/*logerror("proc: %d %d\n",last_fbuf_head,fbuf_head);*/

				tms->target_energy = tms->new_energy;
				tms->target_pitch = tms->new_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->new_k[i];
			}
		}
		else
		{
			interp_period = tms->sample_count / 25;
			switch(tms->interp_count)
			{
				/*         PC=X  X cycle, rendering change (change for next cycle which chip is actually doing) */
				case 0: /* PC=0, A cycle, nothing happens (calc energy) */
				break;
				case 1: /* PC=0, B cycle, nothing happens (update energy) */
				break;
				case 2: /* PC=1, A cycle, update energy (calc pitch) */
				tms->current_energy += ((tms->target_energy - tms->current_energy) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 3: /* PC=1, B cycle, nothing happens (update pitch) */
				break;
				case 4: /* PC=2, A cycle, update pitch (calc K1) */
				tms->current_pitch += ((tms->target_pitch - tms->current_pitch) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 5: /* PC=2, B cycle, nothing happens (update K1) */
				break;
				case 6: /* PC=3, A cycle, update K1 (calc K2) */
				tms->current_k[0] += ((tms->target_k[0] - tms->current_k[0]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 7: /* PC=3, B cycle, nothing happens (update K2) */
				break;
				case 8: /* PC=4, A cycle, update K2 (calc K3) */
				tms->current_k[1] += ((tms->target_k[1] - tms->current_k[1]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 9: /* PC=4, B cycle, nothing happens (update K3) */
				break;
				case 10: /* PC=5, A cycle, update K3 (calc K4) */
				tms->current_k[2] += ((tms->target_k[2] - tms->current_k[2]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 11: /* PC=5, B cycle, nothing happens (update K4) */
				break;
				case 12: /* PC=6, A cycle, update K4 (calc K5) */
				tms->current_k[3] += ((tms->target_k[3] - tms->current_k[3]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 13: /* PC=6, B cycle, nothing happens (update K5) */
				break;
				case 14: /* PC=7, A cycle, update K5 (calc K6) */
				tms->current_k[4] += ((tms->target_k[4] - tms->current_k[4]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 15: /* PC=7, B cycle, nothing happens (update K6) */
				break;
				case 16: /* PC=8, A cycle, update K6 (calc K7) */
				tms->current_k[5] += ((tms->target_k[5] - tms->current_k[5]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 17: /* PC=8, B cycle, nothing happens (update K7) */
				break;
				case 18: /* PC=9, A cycle, update K7 (calc K8) */
				tms->current_k[6] += ((tms->target_k[6] - tms->current_k[6]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 19: /* PC=9, B cycle, nothing happens (update K8) */
				break;
				case 20: /* PC=10, A cycle, update K8 (calc K9) */
				tms->current_k[7] += ((tms->target_k[7] - tms->current_k[7]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 21: /* PC=10, B cycle, nothing happens (update K9) */
				break;
				case 22: /* PC=11, A cycle, update K9 (calc K10) */
				tms->current_k[8] += ((tms->target_k[8] - tms->current_k[8]) >> tms->coeff->interp_coeff[interp_period]);
				break;
				case 23: /* PC=11, B cycle, nothing happens (update K10) */
				break;
				case 24: /* PC=12, A cycle, update K10 (do nothing) */
				tms->current_k[9] += ((tms->target_k[9] - tms->current_k[9]) >> tms->coeff->interp_coeff[interp_period]);
				break;
			}
		}


		/* calculate the output */

		if (tms->current_energy == 0)
		{
			/* generate silent samples here */
			current_val = 0x00;
		}
		else if (tms->old_pitch == 0)
		{
			/* generate unvoiced samples here */
			if (tms->RNG&1)
				current_val = -64; /* according to the patent it is (either + or -) half of the maximum value in the chirp table */
			else
				current_val = 64;

		}
		else
		{
						/* generate voiced samples here */
			/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			 * function has a chirp/peak and then a long chain of zeroes.
			 * The last entry of the chirp rom is at address 0b110011 (50d), the 51st sample,
			 * and if the address reaches that point the ADDRESS incrementer is
			 * disabled, forcing all samples beyond 50d to be == 50d
			 * (address 50d holds zeroes)
			 */

		/*if (tms->coeff->subtype & (SUBTYPE_TMS5100 | SUBTYPE_M58817))*/

		if (tms->pitch_count > 50)
			current_val = tms->coeff->chirptable[50];
		else
			current_val = tms->coeff->chirptable[tms->pitch_count];
		}

		/* Update LFSR *20* times every sample, like patent shows */
		for (i=0; i<20; i++)
		{
			bitout = ((tms->RNG>>12)&1) ^
					((tms->RNG>>10)&1) ^
					((tms->RNG>> 9)&1) ^
					((tms->RNG>> 0)&1);
			tms->RNG >>= 1;
			tms->RNG |= (bitout<<12);
		}

		/* Lattice filter here */

		Y11 = (current_val * 64 * tms->current_energy) / 512;

		for (i = tms->coeff->num_k - 1; i >= 0; i--)
		{
			Y11 = Y11 - ((tms->current_k[i] * tms->x[i]) / 512);
			tms->x[i+1] = tms->x[i] + ((tms->current_k[i] * Y11) / 512);
		}

		tms->x[0] = Y11;


		/* clipping & wrapping, just like the patent */

		/* YL10 - YL4 ==> DA6 - DA0 */
		cliptemp = Y11 / 16;

		/* M58817 seems to be different */
		if (tms->coeff->subtype & (SUBTYPE_M58817))
			cliptemp = cliptemp / 2;

		if (cliptemp > 511) cliptemp = -512 + (cliptemp-511);
		else if (cliptemp < -512) cliptemp = 511 - (cliptemp+512);

		if (cliptemp > 127)
			buffer[buf_count] = 127*256;
		else if (cliptemp < -128)
			buffer[buf_count] = -128*256;
		else
			buffer[buf_count] = cliptemp *256;

		/* Update all counts */

		tms->sample_count = (tms->sample_count + 1) % 200;

		if (tms->current_pitch != 0)
		{
			tms->pitch_count++;
			if (tms->pitch_count >= tms->current_pitch)
				tms->pitch_count = 0;
		}
		else
			tms->pitch_count = 0;

		tms->interp_count = (tms->interp_count + 1) % 25;

		buf_count++;
		size--;
	}

empty:

	while (size > 0)
	{
		tms->sample_count = (tms->sample_count + 1) % 200;
		tms->interp_count = (tms->interp_count + 1) % 25;

		buffer[buf_count] = 0x00;
		buf_count++;
		size--;
	}
}



/******************************************************************************************

     PDC_set -- set Processor Data Clock. Execute CTL_pins command on hi-lo transition.

******************************************************************************************/

void tms5110_PDC_set(tms5110_state *tms, int data)
{
	if (tms->PDC != (data & 0x1) )
	{
		tms->PDC = data & 0x1;
		if (tms->PDC == 0) /* toggling 1->0 processes command on CTL_pins */
		{
			/* first pdc toggles output, next toggles input */
			switch (tms->state)
			{
			case CTL_STATE_INPUT:
				/* continue */
				break;
			case CTL_STATE_NEXT_OUTPUT:
				tms->state = CTL_STATE_OUTPUT;
				return;
			case CTL_STATE_OUTPUT:
				tms->state = CTL_STATE_INPUT;
				return;
			}
			/* the only real commands we handle now are SPEAK and RESET */
			if (tms->next_is_address)
			{
				tms->next_is_address = FALSE;
				tms->address = tms->address | ((tms->CTL_pins & 0x0F)<<tms->addr_bit);
				tms->addr_bit = (tms->addr_bit + 4) % 12;
				tms->schedule_dummy_read = TRUE;
				if (tms->set_load_address)
					tms->set_load_address(tms->device, tms->address);
				new_int_write_addr(tms, tms->CTL_pins & 0x0F);
			}
			else
			{
				switch (tms->CTL_pins & 0xe) /*CTL1 - don't care*/
				{
				case TMS5110_CMD_SPEAK:
					perform_dummy_read(tms);
					tms->speaking_now = 1;

					//should FIFO be cleared now ?????
					break;

				case TMS5110_CMD_RESET:
					perform_dummy_read(tms);
					tms->device->reset();
					break;

				case TMS5110_CMD_READ_BIT:
					if (tms->schedule_dummy_read)
						perform_dummy_read(tms);
					else
					{
						request_bits(tms, 1);
						tms->CTL_pins = (tms->CTL_pins & 0x0E) | extract_bits(tms, 1);
					}
					break;

				case TMS5110_CMD_LOAD_ADDRESS:
					tms->next_is_address = TRUE;
					break;

				case TMS5110_CMD_READ_BRANCH:
					new_int_write(tms, 0,1,1,0);
					new_int_write(tms, 1,1,1,0);
					new_int_write(tms, 0,1,1,0);
					new_int_write(tms, 0,0,0,0);
					new_int_write(tms, 1,0,0,0);
					new_int_write(tms, 0,0,0,0);
					tms->schedule_dummy_read = FALSE;
					break;

				case TMS5110_CMD_TEST_TALK:
					tms->state = CTL_STATE_NEXT_OUTPUT;
					break;

				default:
					logerror("tms5110.c: unknown command: 0x%02x\n", tms->CTL_pins);
					break;
				}

			}
		}
	}
}



/******************************************************************************************

     parse_frame -- parse a new frame's worth of data; returns 0 if not enough bits in buffer

******************************************************************************************/

static void parse_frame(tms5110_state *tms)
{
	int bits, indx, i, rep_flag;
#if (DEBUG_5110)
	int ene;
#endif

	/* count the total number of bits available */
	bits = tms->fifo_count;


	/* attempt to extract the energy index */
	bits -= tms->coeff->energy_bits;
	if (bits < 0)
	{
		request_bits( tms,-bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	indx = extract_bits(tms,tms->coeff->energy_bits);
	tms->new_energy = tms->coeff->energytable[indx];
#if (DEBUG_5110)
	ene = indx;
#endif

	/* if the energy index is 0 or 15, we're done */

	if ((indx == 0) || (indx == 15))
	{
		if (DEBUG_5110) logerror("  (4-bit energy=%d frame)\n",tms->new_energy);

	/* clear the k's */
		if (indx == 0)
		{
			for (i = 0; i < tms->coeff->num_k; i++)
				tms->new_k[i] = 0;
		}

		/* clear fifo if stop frame encountered */
		if (indx == 15)
		{
			if (DEBUG_5110) logerror("  (4-bit energy=%d STOP frame)\n",tms->new_energy);
			tms->fifo_head = tms->fifo_tail = tms->fifo_count = 0;
		}
		return;
	}


	/* attempt to extract the repeat flag */
	bits -= 1;
	if (bits < 0)
	{
		request_bits( tms,-bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	rep_flag = extract_bits(tms,1);

	/* attempt to extract the pitch */
	bits -= tms->coeff->pitch_bits;
	if (bits < 0)
	{
		request_bits( tms,-bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	indx = extract_bits(tms,tms->coeff->pitch_bits);
	tms->new_pitch = tms->coeff->pitchtable[indx];

	/* if this is a repeat frame, just copy the k's */
	if (rep_flag)
	{
	//actually, we do nothing because the k's were already loaded (on parsing the previous frame)

		if (DEBUG_5110) logerror("  (10-bit energy=%d pitch=%d rep=%d frame)\n", tms->new_energy, tms->new_pitch, rep_flag);
		return;
	}


	/* if the pitch index was zero, we need 4 k's */
	if (indx == 0)
	{
		/* attempt to extract 4 K's */
		bits -= 18;
		if (bits < 0)
		{
		request_bits( tms,-bits ); /* toggle M0 to receive needed bits */
		bits = 0;
		}
		for (i = 0; i < 4; i++)
			tms->new_k[i] = tms->coeff->ktable[i][extract_bits(tms,tms->coeff->kbits[i])];

	/* and clear the rest of the new_k[] */
		for (i = 4; i < tms->coeff->num_k; i++)
			tms->new_k[i] = 0;

		if (DEBUG_5110) logerror("  (28-bit energy=%d pitch=%d rep=%d 4K frame)\n", tms->new_energy, tms->new_pitch, rep_flag);
		return;
	}

	/* else we need 10 K's */
	bits -= 39;
	if (bits < 0)
	{
			request_bits( tms,-bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
#if (DEBUG_5110)
	printf("FrameDump %02d ", ene);
	for (i = 0; i < tms->coeff->num_k; i++)
	{
		int x;
		x = extract_bits(tms, tms->coeff->kbits[i]);
		tms->new_k[i] = tms->coeff->ktable[i][x];
		printf("%02d ", x);
	}
	printf("\n");
#else
	for (i = 0; i < tms->coeff->num_k; i++)
	{
		int x;
		x = extract_bits(tms, tms->coeff->kbits[i]);
		tms->new_k[i] = tms->coeff->ktable[i][x];
	}
#endif
	if (DEBUG_5110) logerror("  (49-bit energy=%d pitch=%d rep=%d 10K frame)\n", tms->new_energy, tms->new_pitch, rep_flag);

}



#if 0
/*This is an example word TEN taken from the TMS5110A datasheet*/
static const unsigned int example_word_TEN[619]={
/* 1*/1,0,0,0,  0,  0,0,0,0,0,  1,1,0,0,0,  0,0,0,1,0,  0,1,1,1,    0,1,0,1,
/* 2*/1,0,0,0,  0,  0,0,0,0,0,  1,0,0,1,0,  0,0,1,1,0,  0,0,1,1,    0,1,0,1,
/* 3*/1,1,0,0,  0,  1,0,0,0,0,  1,0,1,0,0,  0,1,0,1,0,  0,1,0,0,    1,0,1,0,    1,0,0,0,    1,0,0,1,    0,1,0,1,    0,0,1,  0,1,0,  0,1,1,
/* 4*/1,1,1,0,  0,  0,1,1,1,1,  1,0,1,0,1,  0,1,1,1,0,  0,1,0,1,    0,1,1,1,    0,1,1,1,    1,0,1,1,    1,0,1,0,    0,1,1,  0,1,0,  0,1,1,
/* 5*/1,1,1,0,  0,  1,0,0,0,0,  1,0,1,0,0,  0,1,1,1,0,  0,1,0,1,    1,0,1,0,    1,0,0,0,    1,1,0,0,    1,0,1,1,    1,0,0,  0,1,0,  0,1,1,
/* 6*/1,1,1,0,  0,  1,0,0,0,1,  1,0,1,0,1,  0,1,1,0,1,  0,1,1,0,    0,1,1,1,    0,1,1,1,    1,0,1,0,    1,0,1,0,    1,1,0,  0,0,1,  1,0,0,
/* 7*/1,1,1,0,  0,  1,0,0,1,0,  1,0,1,1,1,  0,1,1,1,0,  0,1,1,1,    0,1,1,1,    0,1,0,1,    0,1,1,0,    1,0,0,1,    1,1,0,  0,1,0,  0,1,1,
/* 8*/1,1,1,0,  1,  1,0,1,0,1,
/* 9*/1,1,1,0,  0,  1,1,0,0,1,  1,0,1,1,1,  0,1,0,1,1,  1,0,1,1,    0,1,1,1,    0,1,0,0,    1,0,0,0,    1,0,0,0,    1,1,0,  0,1,1,  0,1,1,
/*10*/1,1,0,1,  0,  1,1,0,1,0,  1,0,1,0,1,  0,1,1,0,1,  1,0,1,1,    0,1,0,1,    0,1,0,0,    1,0,0,0,    1,0,1,0,    1,1,0,  0,1,0,  1,0,0,
/*11*/1,0,1,1,  0,  1,1,0,1,1,  1,0,0,1,1,  1,0,0,1,0,  0,1,1,0,    0,0,1,1,    0,1,0,1,    1,0,0,1,    1,0,1,0,    1,0,0,  0,1,1,  0,1,1,
/*12*/1,0,0,0,  0,  1,1,1,0,0,  1,0,0,1,1,  0,0,1,1,0,  0,1,0,0,    0,1,1,0,    1,1,0,0,    0,1,0,1,    1,0,0,0,    1,0,0,  0,1,0,  1,0,1,
/*13*/0,1,1,1,  1,  1,1,1,0,1,
/*14*/0,1,1,1,  0,  1,1,1,1,0,  1,0,0,1,1,  0,0,1,1,1,  0,1,0,1,    0,1,0,1,    1,1,0,0,    0,1,1,1,    1,0,0,0,    1,0,0,  0,1,0,  1,0,1,
/*15*/0,1,1,0,  0,  1,1,1,1,0,  1,0,1,0,1,  0,0,1,1,0,  0,1,0,0,    0,0,1,1,    1,1,0,0,    1,0,0,1,    0,1,1,1,    1,0,1,  0,1,0,  1,0,1,
/*16*/1,1,1,1
};
#endif


static int speech_rom_read_bit(device_t *device)
{
	tms5110_state *tms = get_safe_token(device);

	int r;

	if (tms->speech_rom_bitnum<0)
		r = 0;
	else
		r = (tms->table[tms->speech_rom_bitnum >> 3] >> (0x07 - (tms->speech_rom_bitnum & 0x07))) & 1;

	tms->speech_rom_bitnum++;

	return r;
}

static void speech_rom_set_addr(device_t *device, int addr)
{
	tms5110_state *tms = get_safe_token(device);

	tms->speech_rom_bitnum = addr * 8 - 1;
}

/******************************************************************************

     DEVICE_START( tms5110 ) -- allocate buffers and reset the 5110

******************************************************************************/


static DEVICE_START( tms5110 )
{
	static const tms5110_interface dummy = { NULL, NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL};
	tms5110_state *tms = get_safe_token(device);

	assert_always(tms != NULL, "Error creating TMS5110 chip");

	assert_always(device->static_config() != NULL, "No config");

	tms->intf = device->static_config() ? (const tms5110_interface *)device->static_config() : &dummy;
	tms->table = *device->region();

	tms->device = device;
	tms5110_set_variant(tms, TMS5110_IS_5110A);

	/* resolve lines */
	tms->m0_func.resolve(tms->intf->m0_func, *device);
	tms->m1_func.resolve(tms->intf->m1_func, *device);
	tms->romclk_func.resolve(tms->intf->romclk_func, *device);
	tms->addr_func.resolve(tms->intf->addr_func, *device);
	tms->data_func.resolve(tms->intf->data_func, *device);

	/* initialize a stream */
	tms->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock() / 80, tms, tms5110_update);

	if (tms->table == NULL)
	{
#if 0
		assert_always(tms->intf->M0_callback != NULL, "Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5110 to call for a single bits\n  needed to generate the speech\n  Aborting startup...\n");
#endif
		tms->M0_callback = tms->intf->M0_callback;
		tms->set_load_address = tms->intf->load_address;
	}
	else
	{
		tms->M0_callback = speech_rom_read_bit;
		tms->set_load_address = speech_rom_set_addr;
	}

	tms->state = CTL_STATE_INPUT; /* most probably not defined */
	tms->romclk_hack_timer = device->machine().scheduler().timer_alloc(FUNC(romclk_hack_timer_cb), (void *) device);

	register_for_save_states(tms);
}

static DEVICE_START( tms5100 )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_5100);
}

static DEVICE_START( tms5110a )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_5110A);
}

static DEVICE_START( cd2801 )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_CD2801);
}

static DEVICE_START( tmc0281 )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_TMC0281);
}

static DEVICE_START( cd2802 )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_CD2802);
}

static DEVICE_START( m58817 )
{
	tms5110_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5110 );
	tms5110_set_variant(tms, TMS5110_IS_M58817);
}


static DEVICE_RESET( tms5110 )
{
	tms5110_state *tms = get_safe_token(device);

	/* initialize the FIFO */
	memset(tms->fifo, 0, sizeof(tms->fifo));
	tms->fifo_head = tms->fifo_tail = tms->fifo_count = 0;

	/* initialize the chip state */
	tms->speaking_now = tms->talk_status = 0;
	tms->CTL_pins = 0;
		tms->RNG = 0x1fff;

	/* initialize the energy/pitch/k states */
	tms->old_energy = tms->new_energy = tms->current_energy = tms->target_energy = 0;
	tms->old_pitch = tms->new_pitch = tms->current_pitch = tms->target_pitch = 0;
	memset(tms->old_k, 0, sizeof(tms->old_k));
	memset(tms->new_k, 0, sizeof(tms->new_k));
	memset(tms->current_k, 0, sizeof(tms->current_k));
	memset(tms->target_k, 0, sizeof(tms->target_k));

	/* initialize the sample generators */
	tms->interp_count = tms->sample_count = tms->pitch_count = 0;
	memset(tms->x, 0, sizeof(tms->x));
	tms->next_is_address = FALSE;
	tms->address = 0;
	if (tms->table != NULL || tms->M0_callback != NULL)
	{
		/* legacy interface */
		tms->schedule_dummy_read = TRUE;

	}
	else
	{
		/* no dummy read! This makes bagman and ad2083 speech fail
		 * with the new cycle and transition exact interfaces
		 */
		tms->schedule_dummy_read = FALSE;
	}
	tms->addr_bit = 0;
}



/******************************************************************************

     tms5110_ctl_w -- write Control Command to the sound chip
commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_DEVICE_HANDLER( tms5110_ctl_w )
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();
	tms->CTL_pins = data & 0xf;
}


/******************************************************************************

     tms5110_pdc_w -- write to PDC pin on the sound chip

******************************************************************************/

WRITE_LINE_DEVICE_HANDLER( tms5110_pdc_w )
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();
	tms5110_PDC_set(tms, state);
}



/******************************************************************************

     tms5110_ctl_r -- read status from the sound chip

        bit 0 = TS - Talk Status is active (high) when the VSP is processing speech data.
                Talk Status goes active at the initiation of a SPEAK command.
                It goes inactive (low) when the stop code (Energy=1111) is processed, or
                immediately(?????? not TMS5110) by a RESET command.
        TMS5110 datasheets mention this is only available as a result of executing
                TEST TALK command.

                FIXME: data read not implemented, CTL1 only available after TALK command

******************************************************************************/

READ8_DEVICE_HANDLER( tms5110_ctl_r )
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();
	if (tms->state == CTL_STATE_OUTPUT)
	{
		//if (DEBUG_5110) logerror("Status read (status=%2d)\n", tms->talk_status);
		return (tms->talk_status << 0); /*CTL1 = still talking ? */
	}
	else
	{
		//if (DEBUG_5110) logerror("Status read (not in output mode)\n");
		return (0);
	}
}

READ8_DEVICE_HANDLER( m58817_status_r )
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();
	return (tms->talk_status << 0); /*CTL1 = still talking ? */
}

/******************************************************************************

     tms5110_romclk_hack_r -- read status of romclk

******************************************************************************/

static TIMER_CALLBACK( romclk_hack_timer_cb )
{
	tms5110_state *tms = get_safe_token((device_t *) ptr);
	tms->romclk_hack_state = !tms->romclk_hack_state;
}

READ8_DEVICE_HANDLER( tms5110_romclk_hack_r )
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();

	/* create and start timer if necessary */
	if (!tms->romclk_hack_timer_started)
	{
		tms->romclk_hack_timer_started = TRUE;
		tms->romclk_hack_timer->adjust(attotime::from_hz(device->clock() / 40), 0, attotime::from_hz(device->clock() / 40));
	}
	return tms->romclk_hack_state;
}



/******************************************************************************

     tms5110_ready_r -- return the not ready status from the sound chip

******************************************************************************/

int tms5110_ready_r(device_t *device)
{
	tms5110_state *tms = get_safe_token(device);

	/* bring up to date first */
	tms->stream->update();
	return (tms->fifo_count < FIFO_SIZE-1);
}



/******************************************************************************

     tms5110_update -- update the sound chip so that it is in sync with CPU execution

******************************************************************************/

static STREAM_UPDATE( tms5110_update )
{
	tms5110_state *tms = (tms5110_state *)param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5110_process(tms, sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
	}
}



/******************************************************************************

     tms5110_set_frequency -- adjusts the playback frequency

******************************************************************************/

void tms5110_set_frequency(device_t *device, int frequency)
{
	tms5110_state *tms = get_safe_token(device);
	tms->stream->set_sample_rate(frequency / 80);
}


/*
 *
 * General Interface design (Bagman)
 *
 *                         +------------------------------------------------------------------------+
 *                         |                                                                        |
 *       +-------------+   |           +-------------+       +-------------+       +-------------+  |
 *       | TMS5100     |   |           | Counters    |       | Rom(s)      |       | Decoder     |  |
 *       |        ADD8 |<--+           | LS393s      |       |             |       |             |  |
 *       |             |               |             |       |             |       |        Out  |--+
 *       |          M0 |---+           |     Address |======>| Address     |       | IN1         |
 *       |             |   |           |             |       |       Data  |======>| ...         |
 *   M   |             |   +---------->| Clk         |       |             |       | IN8         |
 *   A-->| CTL1        |               |             |       |             |       |             |
 *   P-->| CTL2        |          +--->| Reset       |       |             |       |             |
 *   P-->| CTL3        |          |    |             |       |             |       |    A  B  C  |
 *   E-->| CTL4        |          |    +-------------+       +-------------+       +-------------+
 *   D-->| PDC         |          |                                                     ^  ^  ^
 *       |             |          +-------------------------------------------------+   |  |  |
 *       |             |                                                            |   Bit Select
 *       |      ROMCLK |---+           +-------------+       +-------------+        |
 *       |             |   |           | Counter     |       | PROM        |        |
 *       +-------------+   |           | LS393       |       |          D1 |   M  --+ Reset Bit
 *                         |           |          Q0 |------>| A0          |   A
 *                         +---------->| Clk      Q1 |------>| A1          |   P ==>  CTL1 ... CTL4
 *                                     |          Q2 |------>| A2          |   P -->  PDC
 *                                     | Reset    Q3 |------>| A3          |   E  --+ Stop Bit
 *                                     |             |   +-->| A4       D8 |   D    |
 *                                     +-------------+   |   +-------------+        |
 *                                                       |                          |
 *                                                       |   +---+                  |
 *                                                       |   |   |<-----------------+
 *                                                       +---| & |
 *                                                           |   |<-------- Enable
 *                                                           +---+
 *
 */

/******************************************************************************

     DEVICE_START( tmsprom ) -- allocate buffers initialize

******************************************************************************/

static void register_for_save_states_prom(tmsprom_state *tms)
{
	tms->device->save_item(NAME(tms->address));
	tms->device->save_item(NAME(tms->base_address));
	tms->device->save_item(NAME(tms->bit));
	tms->device->save_item(NAME(tms->enable));
	tms->device->save_item(NAME(tms->prom_cnt));
	tms->device->save_item(NAME(tms->m0));
}


static void update_prom_cnt(tmsprom_state *tms)
{
	UINT8 prev_val = tms->prom[tms->prom_cnt] | 0x0200;
	if (tms->enable && (prev_val & (1<<tms->intf->stop_bit)))
		tms->prom_cnt |= 0x10;
	else
		tms->prom_cnt &= 0x0f;
}

static TIMER_CALLBACK( tmsprom_step )
{
	device_t *device = (device_t *)ptr;
	tmsprom_state *tms = get_safe_token_prom(device);

	/* only 16 bytes needed ... The original dump is bad. This
	 * is what is needed to get speech to work. The prom data has
	 * been updated and marked as BAD_DUMP. The information below
	 * is given for reference once another dump should surface.
	 *
	 * static const int prom[16] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00,
	 *              0x02, 0x00, 0x40, 0x00, 0x04, 0x06, 0x04, 0x84 };
	 */
	UINT16 ctrl;

	update_prom_cnt(tms);
	ctrl = (tms->prom[tms->prom_cnt] | 0x200);

	//if (tms->enable && tms->prom_cnt < 0x10) printf("ctrl %04x, enable %d cnt %d\n", ctrl, tms->enable, tms->prom_cnt);
	tms->prom_cnt = ((tms->prom_cnt + 1) & 0x0f) | (tms->prom_cnt & 0x10);

	if (ctrl & (1 << tms->intf->reset_bit))
		tms->address = 0;

	tms->ctl_func(0, BITSWAP8(ctrl,0,0,0,0,tms->intf->ctl8_bit,
			tms->intf->ctl4_bit,tms->intf->ctl2_bit,tms->intf->ctl1_bit));

	tms->pdc_func((ctrl >> tms->intf->pdc_bit) & 0x01);
}

static DEVICE_START( tmsprom )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	assert_always(tms != NULL, "Error creating TMSPROM chip");

	tms->intf = (const tmsprom_interface *) device->static_config();
	assert_always(tms->intf != NULL, "Error creating TMSPROM chip: No configuration");

	/* resolve lines */
	tms->pdc_func.resolve(tms->intf->pdc_func, *device);
	tms->ctl_func.resolve(tms->intf->ctl_func, *device);

	tms->rom = *device->region();
	assert_always(tms->rom != NULL, "Error creating TMSPROM chip: No rom region found");
	tms->prom = device->machine().root_device().memregion(tms->intf->prom_region)->base();
	assert_always(tms->rom != NULL, "Error creating TMSPROM chip: No prom region found");

	tms->device = device;
	tms->clock = device->clock();

	tms->romclk_timer = device->machine().scheduler().timer_alloc(FUNC(tmsprom_step), device);
	tms->romclk_timer->adjust(attotime::zero, 0, attotime::from_hz(tms->clock));

	tms->bit = 0;
	tms->base_address = 0;
	tms->address = 0;
	tms->enable = 0;
	tms->m0 = 0;
	tms->prom_cnt = 0;
	register_for_save_states_prom(tms);
}

WRITE_LINE_DEVICE_HANDLER( tmsprom_m0_w )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	/* falling edge counts */
	if (tms->m0 && !state)
	{
		tms->address += 1;
		tms->address &= (tms->intf->rom_size-1);
	}
	tms->m0 = state;
}

READ_LINE_DEVICE_HANDLER( tmsprom_data_r )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	return (tms->rom[tms->base_address + tms->address] >> tms->bit) & 0x01;
}


WRITE8_DEVICE_HANDLER( tmsprom_rom_csq_w )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	if (!data)
		tms->base_address = offset * tms->intf->rom_size;
}

WRITE8_DEVICE_HANDLER( tmsprom_bit_w )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	tms->bit = data;
}

WRITE_LINE_DEVICE_HANDLER( tmsprom_enable_w )
{
	tmsprom_state *tms = get_safe_token_prom(device);

	if (state != tms->enable)
	{
		tms->enable = state;
		update_prom_cnt(tms);

		/* the following is needed for ad2084.
		 * It is difficult to derive the actual connections from
		 * pcb pictures but the reset pin of the LS393 driving
		 * the prom address line is connected somewhere.
		 *
		 * This does not affect bagman. It just simulates that a
		 * write to ads3 is always happening when the four lower
		 * counter bits are 0!
		 */
		if (state)
			tms->prom_cnt &= 0x10;
	}
}


/*-------------------------------------------------
    TMS 5110 device definition
-------------------------------------------------*/

const device_type TMS5110 = &device_creator<tms5110_device>;

tms5110_device::tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS5110, "TMS5110", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tms5110_state);
}
tms5110_device::tms5110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tms5110_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tms5110_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5110_device::device_start()
{
	DEVICE_START_NAME( tms5110 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms5110_device::device_reset()
{
	DEVICE_RESET_NAME( tms5110 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5110_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMS5100 = &device_creator<tms5100_device>;

tms5100_device::tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5100, "TMS5100", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5100_device::device_start()
{
	DEVICE_START_NAME( tms5100 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5100_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMS5110A = &device_creator<tms5110a_device>;

tms5110a_device::tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5110A, "TMS5110A", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5110a_device::device_start()
{
	DEVICE_START_NAME( tms5110a )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5110a_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type CD2801 = &device_creator<cd2801_device>;

cd2801_device::cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2801, "CD2801", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cd2801_device::device_start()
{
	DEVICE_START_NAME( cd2801 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cd2801_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMC0281 = &device_creator<tmc0281_device>;

tmc0281_device::tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMC0281, "TMC0281", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc0281_device::device_start()
{
	DEVICE_START_NAME( tmc0281 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tmc0281_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type CD2802 = &device_creator<cd2802_device>;

cd2802_device::cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2802, "CD2802", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cd2802_device::device_start()
{
	DEVICE_START_NAME( cd2802 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cd2802_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type M58817 = &device_creator<m58817_device>;

m58817_device::m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, M58817, "M58817", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m58817_device::device_start()
{
	DEVICE_START_NAME( m58817 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void m58817_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}



const device_type TMSPROM = &device_creator<tmsprom_device>;

tmsprom_device::tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMSPROM, "TMSPROM", tag, owner, clock)
{
	m_token = global_alloc_clear(tmsprom_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tmsprom_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmsprom_device::device_start()
{
	DEVICE_START_NAME( tmsprom )(this);
}
