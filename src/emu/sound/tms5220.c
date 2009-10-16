/**********************************************************************************************

     TMS5200/5220 simulator

     Written for MAME by Frank Palazzolo
     With help from Neill Corlett
     Additional tweaking by Aaron Giles
     TMS6100 Speech Rom support added by Raphael Nabet
     PRNG code by Jarek Burczynski backported from tms5110.c by Lord Nightmare
     Chirp/excitation table fixes by Lord Nightmare
     Various fixes by Lord Nightmare
     Modularization by Lord Nightmare
     Sub-interpolation-cycle parameter updating added by Lord Nightmare

     Much information regarding these lpc encoding comes from US patent 4,209,844
     US patent 4,331,836 describes the complete 51xx chip
     US patent 4,335,277 describes the complete 52xx chip
     Special Thanks to Larry Brantingham for answering questions regarding the chip details

   TMS5200/TMS5220/TMS5220C:

                 +-----------------+
        D7(d0)   |  1           28 |  /RS
        ADD1     |  2           27 |  /WS
        ROMCLK   |  3           26 |  D6(d1)
        VDD(-5)  |  4           25 |  ADD2
        VSS(+5)  |  5           24 |  D5(d2)
        OSC      |  6           23 |  ADD4
        T11      |  7           22 |  D4(d3)
        SPKR     |  8           21 |  ADD8/DATA
        I/O      |  9           20 |  TEST
        PROMOUT  | 10           19 |  D3(d4)
        VREF(GND)| 11           18 |  /READY
        D2(d5)   | 12           17 |  /INT
        D1(d6)   | 13           16 |  M1
        D0(d7)   | 14           15 |  M0
                 +-----------------+
Note the standard naming for d* data bits with 7 as MSB and 0 as LSB is in lowercase.
TI's naming has D7 as LSB and D0 as MSB and is in uppercase

TODO:
    Implement a ready callback for pc interfaces
    - this will be quite a challenge since for it to be really accurate
      the whole emulation has to run in sync (lots of timers) with the
      cpu cores.
    If a command is still executing, /READY will be kept high until the command has
    finished if the next command is written.
    TMS5220C: see below.
    tomcat has a 5220 which is not hooked up at all

Notes:
    Looping has the tms5220 hookep up directly to the cpu. However currently the
    tms9900 cpu core does not support a ready line.

    Email from Lord Nightmare having a lot more detail follows:

    Yes, there is at least two more differences:
The 5220 is 'noisier' when playing unvoiced frames than the 5220C is; I
think the 5220C may use a different energy table (or use one value lower
in the normal energy table) than the 5220 does, possibly only when
playing unvoiced frames, but I can't prove this without a decap; the
5220C's PROMOUT pin (for dumping the lpc tables as played) is
nonfunctional due to changed design or a die bug.
In addition, the NOP commands on the FIFO interface have been changed
and data passed in the low bits has a meaning regarding frame length:
commands :
(lsb is right, msb is left; x means don't care)
x0x0xbcc : (5220: NOP) (5220C: select frame length by cc, and b selects
whether every frame is preceeded by 2 bits to select the frame length
(instead of using the value set by cc)) (default is 00 for frame length
(200 samples) and 0 for whether the preceeding 2 bits are enabled (off))
x001xxxx: sends eight read bit commands (M0 high M1 low) to VSM and
reads the resulting bits serially into a temporary register, which
becomes readable as the next byte read from the tms52xx once ready goes
active.

Note the bit order of the byte read from the TMS52xx is BACKWARDS as
compared to the actual data order as in the rom on the VSM chips! This
was IMHO a rather silly design decision of TI. (I asked Larry
Brantingham about this but he wasn't involved with the TMS52xx chips,
just the 5100)
There's ASCII data in the TI 99/4 speech module VSMs which has the bit
order reversed on purpose because of this!

x011xxxx: sends a read and branch command (M0 high, M1 high) to force
VSM to set its data pointer to whatever the data is at its current
pointer location is)
x100aaaa: send a load address command (M0 low M1 high) to vsm with the 4
'a' bits
x101xxxx: 'speak' begins speaking from current address in the data
pointer of the vsms
x110xxxx: 'speak external' begins speaking from the 16 byte fifo
x111xxxx: resets the speech synthesis core immediately; clears the bit
offset counter in the fifo to the nearest byte 'forward ', but does NOT
clear the fifo, annoyingly!

Its also possible but inconclusive that the chirp table was changed.
The LPC tables between the 5220 and 5220C are MOSTLY the same of not
completely so, but as mentioned above the energy table has some sort of
difference.


As for which games used which chips, from the TMS5220 wikipedia page:

TMS5200 AKA TMC0285: (1981 to 1983ish when supplies ran low)
    Arcade: Zaccaria's 'money money' and 'jack rabbit'; Bally/Midway's
'Discs of Tron' (all environmental cabs and a few upright cabs; the code
exists on all versions for the speech though, and upright cabs can be
upgraded to add it by hacking on a 'Squawk & Talk' pinball speech board
(which is also TMS5200 based) with a few modded components)
    Pinball: All Bally/Midway machines which uses the 'Squawk & Talk' board.
    Home computer: TI 99/4 PHP1500 Speech module (along with two VSM
serial chips); Street Electronics Corp.'s Apple II 'Echo 2' Speech
synthesizer (early cards only)

TMS5220: (mostly on things made between 1982 and 1984-1985 when supplies
ran low)
    Arcade: Bally/Midway's 'NFL Football'; Atari's 'Star Wars',
'Firefox', 'Return of the Jedi', 'Road Runner', 'The Empire Strikes
Back' (all verified with schematics); Venture Line's 'Looping' and 'Sky
Bumper' (need verify for both); Olympia's 'Portraits' (need verify);
Exidy's 'Victory' and 'Victor Banana' (need verify for both)
    Pinball: Several (don't know names offhand, have not checked schematics)
    Home computer: Street Electronics Corp.'s Apple II 'Echo 2' Speech
synthesizer (later cards only); Texas Instruments' 'Speak and Learn'
scanner wand unit.

TMS5220C AKA TSP5220C: (on stuff made from 1984 to 1992 or so)
    Arcade: Atari's 'Indiana Jones and the Temple of Doom', '720',
'Gauntlet', 'Gauntlet II', 'A.P.B.', 'Paperboy', 'RoadBlasters',
'Vindicators Pt II'(verify?), and 'Escape from the Planet of the Robot
Monsters' (all verified except for vindicators pt 2)
    Pinball: Several (less common than the tms5220? (not sure about
this), mostly on later pinballs with LPC speech)
    Home computer: Street Electronics Corp.'s 'ECHO' parallel/hobbyist
module (6511 based), IBM PS/2 Speech adapter (parallel port connection
device), PES Speech adapter (serial port connection)


***********************************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "tms5220.h"


#define VERBOSE (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define MAX_SAMPLE_CHUNK	512
#define FIFO_SIZE 16


enum _tms5220_variant
{
	variant_tms5220c,	/* TMS5220_IS_TMS5220C  TMS5220_IS_TSP5220C */
	variant_tms5220,	/* TMS5220_IS_TMS5220 */
	variant_tmc0285		/* TMS5220_IS_TMS5200, TMS5220_IS_CD2501 */
};
typedef enum _tms5220_variant tms5220_variant;



/* Pull in the ROM tables */
#include "tms5220r.c"

/*
  Changes by R. Nabet
   * added Speech ROM support
   * modified code so that the beast only start speaking at the start of next frame, like the data
     sheet says
*/


typedef struct _tms5220_state tms5220_state;
struct _tms5220_state
{
	/* callbacks */
	devcb_resolved_write_line	irq_func;

	/* these contain data that describes the 128-bit data FIFO */
	UINT8 fifo[FIFO_SIZE];
	UINT8 fifo_head;
	UINT8 fifo_tail;
	UINT8 fifo_count;
	UINT8 fifo_bits_taken;


	/* these contain global status bits */
	/*
        R Nabet : speak_external is only set when a speak external command is going on.
        tms5220_speaking is set whenever a speak or speak external command is going on.
        Note that we really need to do anything in tms5220_process and play samples only when
        tms5220_speaking is true.  Else, we can play nothing as well, which is a
        speed-up...
    */
	UINT8 tms5220_speaking;	/* Speak or Speak External command in progress */
	UINT8 speak_external;	/* Speak External command in progress */
	UINT8 talk_status; 		/* tms5220 is really currently speaking */
	UINT8 first_frame;		/* we have just started speaking, and we are to parse the first frame */
	UINT8 last_frame;		/* we are doing the frame of sound */
	UINT8 buffer_low;		/* FIFO has less than 8 bytes in it */
	UINT8 buffer_empty;		/* FIFO is empty*/
	UINT8 irq_pin;			/* state of the IRQ pin (output) */

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

	UINT16 previous_energy;         /* needed for lattice filter to match patent */

	UINT8 interp_count;		/* number of samples within each sub-interpolation period, ranges from 0-24 */
	UINT8 sample_count;		/* number of samples within the ENTIRE interpolation period, ranges from 0-199 */
	UINT16 pitch_count;		/* pitch counter; provides chirp rom address */

	INT32 u[11];
	INT32 x[10];

	INT32 RNG;      /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */
	INT8 excitation_data;

	/* R Nabet : These have been added to emulate speech Roms */
	UINT8 schedule_dummy_read;			/* set after each load address, so that next read operation
                                              is preceded by a dummy read */

	UINT8 data_register;				/* data register, used by read command */
	UINT8 RDB_flag;					/* whether we should read data register or status register */

	/* io_ready: page 3 of the datasheet specifies that READY will be asserted until
     * data is available or processed by the system.
     */

	UINT8 io_ready;

	/* flag for "true" timing involving rs/ws */

	UINT8 true_timing;

	/* rsws - state, rs bit 1, ws bit 0 */
	UINT8 rs_ws;
	UINT8 read_latch;
	UINT8 write_latch;

	/* flag for variant tmc0285/tms5200 emulation */
	/* The TMC0285 AKA TMS5200 is an earlier variant of the TMS5220 used in
       the early releases of the Speech Module for the TI-99/4(a) computer,
       in Zaccaria's 'Money Money', and in a few other places.
       The TMS5200 has a different set of LPC coefficients, and a different
       chirp table than the 5220 (which is not yet dumped)
       Due to the vast superiority of the quality of the TMS5220, TI may have
       sold the remaining stocks of TMS5200s at a discount, and provided a
       special encoder to use the older tables.
       Other than those differences, the two chips are identical.
       Another variant of the TMS5220 is the TMS5220C/TSP5220C, which replaces
       the X0X0 'NOP' opcode with an opcode to select the number of
       interpolations per frame to either be defined at each frame, or be fixed
       at either 8, 6, 4, or 2. The TMS5200/5220 is always fixed at 8.
     */
	tms5220_variant variant;
    /* The TMS52xx has two different ways of providing output data: the
       analog speaker pin (which was usually used) and the Digital I/O pin.
       The internal DAC used to feed the analog pin is only 8 bits, and has the
       funny clipping/clamping logic, while the digital pin gives full 12? bit
       resolution of the output data.
     */
	UINT8 digital_select;
	const device_config *device;

	const tms5220_interface *intf;
	sound_stream *stream;
	int clock;
};


INLINE tms5220_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_TMS5220 ||
		   sound_get_type(device) == SOUND_TMC0285 ||
		   sound_get_type(device) == SOUND_TMS5200);
	return (tms5220_state *)device->token;
}



/* Static function prototypes */
static void process_command(tms5220_state *tms);
static int parse_frame(tms5220_state *tms, int the_first_frame);
static void check_buffer_low(tms5220_state *tms);
static void set_interrupt_state(tms5220_state *tms, int state);
static INT16 lattice_filter(tms5220_state *tms);
static INT16 clip_and_wrap(INT16 cliptemp);
static STREAM_UPDATE( tms5220_update );


static void register_for_save_states(tms5220_state *tms)
{
	state_save_register_device_item_array(tms->device, 0, tms->fifo);
	state_save_register_device_item(tms->device, 0, tms->fifo_head);
	state_save_register_device_item(tms->device, 0, tms->fifo_tail);
	state_save_register_device_item(tms->device, 0, tms->fifo_count);
	state_save_register_device_item(tms->device, 0, tms->fifo_bits_taken);

	state_save_register_device_item(tms->device, 0, tms->tms5220_speaking);
	state_save_register_device_item(tms->device, 0, tms->speak_external);
	state_save_register_device_item(tms->device, 0, tms->talk_status);
	state_save_register_device_item(tms->device, 0, tms->first_frame);
	state_save_register_device_item(tms->device, 0, tms->last_frame);
	state_save_register_device_item(tms->device, 0, tms->buffer_low);
	state_save_register_device_item(tms->device, 0, tms->buffer_empty);
	state_save_register_device_item(tms->device, 0, tms->irq_pin);

	state_save_register_device_item(tms->device, 0, tms->old_energy);
	state_save_register_device_item(tms->device, 0, tms->old_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->old_k);

	state_save_register_device_item(tms->device, 0, tms->new_energy);
	state_save_register_device_item(tms->device, 0, tms->new_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->new_k);

	state_save_register_device_item(tms->device, 0, tms->current_energy);
	state_save_register_device_item(tms->device, 0, tms->current_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->current_k);

	state_save_register_device_item(tms->device, 0, tms->target_energy);
	state_save_register_device_item(tms->device, 0, tms->target_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->target_k);

	state_save_register_device_item(tms->device, 0, tms->previous_energy);

	state_save_register_device_item(tms->device, 0, tms->interp_count);
	state_save_register_device_item(tms->device, 0, tms->sample_count);
	state_save_register_device_item(tms->device, 0, tms->pitch_count);

	state_save_register_device_item_array(tms->device, 0, tms->u);
	state_save_register_device_item_array(tms->device, 0, tms->x);

	state_save_register_device_item(tms->device, 0, tms->RNG);
	state_save_register_device_item(tms->device, 0, tms->excitation_data);

	state_save_register_device_item(tms->device, 0, tms->schedule_dummy_read);
	state_save_register_device_item(tms->device, 0, tms->data_register);
	state_save_register_device_item(tms->device, 0, tms->RDB_flag);
	state_save_register_device_item(tms->device, 0, tms->digital_select);

	state_save_register_device_item(tms->device, 0, tms->io_ready);
}


/**********************************************************************************************

     tms5220_data_write -- handle a write to the TMS5220

***********************************************************************************************/

static void tms5220_data_write(tms5220_state *tms, int data)
{
    /* add this byte to the FIFO */
    if (tms->fifo_count < FIFO_SIZE)
    {
        tms->fifo[tms->fifo_tail] = data;
        tms->fifo_tail = (tms->fifo_tail + 1) % FIFO_SIZE;
        tms->fifo_count++;

		/* if we were speaking, then we're no longer empty */
		if (tms->speak_external)
			tms->buffer_empty = 0;

        LOG(("Added byte to FIFO (size=%2d)\n", tms->fifo_count));
    }
    else
    {
        LOG(("Ran out of room in the FIFO!\n"));
    }

    /* update the buffer low state */
    check_buffer_low(tms);

	if (! tms->speak_external)
		/* R Nabet : we parse commands at once.  It is necessary for such commands as read. */
		process_command (tms/*data*/);
}


/**********************************************************************************************

     tms5220_status_read -- read status or data from the TMS5220

      From the data sheet:
        bit D0(bit 7) = TS - Talk Status is active (high) when the VSP is processing speech data.
                Talk Status goes active at the initiation of a Speak command or after nine
                bytes of data are loaded into the FIFO following a Speak External command. It
                goes inactive (low) when the stop code (Energy=1111) is processed, or
                immediately by a buffer empty condition or a reset command.
        bit D1(bit 6) = BL - Buffer Low is active (high) when the FIFO buffer is more than half empty.
                Buffer Low is set when the "Last-In" byte is shifted down past the half-full
                boundary of the stack. Buffer Low is cleared when data is loaded to the stack
                so that the "Last-In" byte lies above the half-full boundary and becomes the
                ninth data byte of the stack.
        bit D2(bit 5) = BE - Buffer Empty is active (high) when the FIFO buffer has run out of data
                while executing a Speak External command. Buffer Empty is set when the last bit
                of the "Last-In" byte is shifted out to the Synthesis Section. This causes
                Talk Status to be cleared. Speed is terminated at some abnormal point and the
                Speak External command execution is terminated.

***********************************************************************************************/

static int tms5220_status_read(tms5220_state *tms)
{
	if (tms->RDB_flag)
	{	/* if last command was read, return data register */
		tms->RDB_flag = FALSE;
		return(tms->data_register);
	}
	else
	{	/* read status */

		/* clear the interrupt pin */
		set_interrupt_state(tms, 0);

		LOG(("Status read: TS=%d BL=%d BE=%d\n", tms->talk_status, tms->buffer_low, tms->buffer_empty));

		return (tms->talk_status << 7) | (tms->buffer_low << 6) | (tms->buffer_empty << 5);
	}
}



/**********************************************************************************************

     tms5220_ready_read -- returns the ready state of the TMS5220

***********************************************************************************************/

static int tms5220_ready_read(tms5220_state *tms)
{
	LOG(("io_ready %d\n", tms->io_ready));
    return (tms->fifo_count < FIFO_SIZE-1) && tms->io_ready;
}


/**********************************************************************************************

     tms5220_cycles_to_ready -- returns the number of cycles until ready is asserted

***********************************************************************************************/

static int tms5220_cycles_to_ready(tms5220_state *tms)
{
	int answer;


	if (tms5220_ready_read(tms))
		answer = 0;
	else
	{
		int val;

		answer = 200-tms->sample_count+8;

		/* total number of bits available in current byte is (8 - tms->fifo_bits_taken) */
		/* if more than 4 are available, we need to check the energy */
		if (tms->fifo_bits_taken < 4)
		{
			/* read energy */
			val = (tms->fifo[tms->fifo_head] >> tms->fifo_bits_taken) & 0xf;
			if (val == 0)
				/* 0 -> silence frame: we will only read 4 bits, and we will
                therefore need to read another frame before the FIFO is not
                full any more */
				answer += 200;
			/* 15 -> stop frame, we will only read 4 bits, but the FIFO will
            we cleared */
			/* otherwise, we need to parse the repeat flag (1 bit) and the
            pitch (6 bits), so everything will be OK. */
		}
	}

	return answer;
}


/**********************************************************************************************

     tms5220_int_read -- returns the interrupt state of the TMS5220

***********************************************************************************************/

static int tms5220_int_read(tms5220_state *tms)
{
    return tms->irq_pin;
}



/**********************************************************************************************

     tms5220_process -- fill the buffer with a specific number of samples

***********************************************************************************************/

static void tms5220_process(tms5220_state *tms, INT16 *buffer, unsigned int size)
{
    int buf_count=0;
    int i, interp_period, bitout;

tryagain:

    /* if we're not speaking, parse commands */
	/*while (!tms->speak_external && tms->fifo_count > 0)
        process_command(tms);*/

    /* if we're empty and still not speaking, fill with nothingness */
	if ((!tms->tms5220_speaking) && (!tms->last_frame))
        goto empty;

    /* if we're to speak, but haven't started, wait for the 9th byte */
	if (!tms->talk_status && tms->speak_external)
    {
        if (tms->fifo_count < 9)
           goto empty;

        tms->talk_status = 1;
		tms->first_frame = 1;	/* will cause the first frame to be parsed */
		tms->buffer_empty = 0;
	}

    /* loop until the buffer is full or we've stopped speaking */
	while ((size > 0) && tms->talk_status)
    {

        /* if we're ready for a new frame */
        if ((tms->interp_count == 0) && (tms->sample_count == 0))
        {
            /* Parse a new frame */
			if (!parse_frame(tms, tms->first_frame))
				break;
			tms->first_frame = 0;

            /* Set old target as new start of frame */
            tms->current_energy = tms->old_energy;
            tms->current_pitch = tms->old_pitch;
            for (i = 0; i < 10; i++)
                tms->current_k[i] = tms->old_k[i];

            /* is this a zero energy frame? */
            if (tms->current_energy == 0)
            {
                /*mame_printf_debug("processing frame: zero energy\n");*/
                tms->target_energy = 0;
                tms->target_pitch = tms->current_pitch;
                for (i = 0; i < 10; i++)
                    tms->target_k[i] = tms->current_k[i];
            }

            /* is this a stop frame? */
            else if (tms->current_energy == (energytable[15] >> 6))
            {
                /*mame_printf_debug("processing frame: stop frame\n");*/
                tms->current_energy = energytable[0] >> 6;
                tms->target_energy = tms->current_energy;
				/*tms->interp_count = tms->sample_count =*/ tms->pitch_count = 0;
				tms->last_frame = 0;
				if (tms->tms5220_speaking)
					/* new speech command in progress */
					tms->first_frame = 1;
				else
				{
					/* really stop speaking */
					tms->talk_status = 0;

					/* generate an interrupt if necessary */
					set_interrupt_state(tms, 1);
				}

                /* try to fetch commands again */
                goto tryagain;
            }
            else
            {
                /* is this the ramp down frame? */
                if (tms->new_energy == (energytable[15] >> 6))
                {
                    /*mame_printf_debug("processing frame: ramp down\n");*/
                    tms->target_energy = 0;
                    tms->target_pitch = tms->current_pitch;
                    for (i = 0; i < 10; i++)
                        tms->target_k[i] = tms->current_k[i];
                }
                /* Reset the step size */
                else
                {
                    /*mame_printf_debug("processing frame: Normal\n");*/
                    /*mame_printf_debug("*** Energy = %d\n",tms->current_energy);*/
                    /*mame_printf_debug("proc: %d %d\n",last_fbuf_head,fbuf_head);*/

                    tms->target_energy = tms->new_energy;
                    tms->target_pitch = tms->new_pitch;

                    for (i = 0; i < 4; i++)
                        tms->target_k[i] = tms->new_k[i];
                    if (tms->current_pitch == 0)
                        for (i = 4; i < 10; i++)
                        {
                            tms->target_k[i] = tms->current_k[i] = 0;
                        }
                    else
                        for (i = 4; i < 10; i++)
                            tms->target_k[i] = tms->new_k[i];
                }
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
		  tms->current_energy += ((tms->target_energy - tms->current_energy) >> interp_coeff[interp_period]);
            	  break;
                case 3: /* PC=1, B cycle, nothing happens (update pitch) */
		  break;
                case 4: /* PC=2, A cycle, update pitch (calc K1) */
            	  if (tms->old_pitch != 0)
                  tms->current_pitch += ((tms->target_pitch - tms->current_pitch) >> interp_coeff[interp_period]);
		  break;
                case 5: /* PC=2, B cycle, nothing happens (update K1) */
		  break;
		case 6: /* PC=3, A cycle, update K1 (calc K2) */
		  tms->current_k[0] += ((tms->target_k[0] - tms->current_k[0]) >> interp_coeff[interp_period]);
            	  break;
                case 7: /* PC=3, B cycle, nothing happens (update K2) */
		  break;
		case 8: /* PC=4, A cycle, update K2 (calc K3) */
		  tms->current_k[1] += ((tms->target_k[1] - tms->current_k[1]) >> interp_coeff[interp_period]);
            	  break;
                case 9: /* PC=4, B cycle, nothing happens (update K3) */
		  break;
		case 10: /* PC=5, A cycle, update K3 (calc K4) */
		  tms->current_k[2] += ((tms->target_k[2] - tms->current_k[2]) >> interp_coeff[interp_period]);
            	  break;
                case 11: /* PC=5, B cycle, nothing happens (update K4) */
		  break;
		case 12: /* PC=6, A cycle, update K4 (calc K5) */
		  tms->current_k[3] += ((tms->target_k[3] - tms->current_k[3]) >> interp_coeff[interp_period]);
            	  break;
                case 13: /* PC=6, B cycle, nothing happens (update K5) */
		  break;
		case 14: /* PC=7, A cycle, update K5 (calc K6) */
		  tms->current_k[4] += ((tms->target_k[4] - tms->current_k[4]) >> interp_coeff[interp_period]);
            	  break;
                case 15: /* PC=7, B cycle, nothing happens (update K6) */
		  break;
		case 16: /* PC=8, A cycle, update K6 (calc K7) */
		  tms->current_k[5] += ((tms->target_k[5] - tms->current_k[5]) >> interp_coeff[interp_period]);
            	  break;
                case 17: /* PC=8, B cycle, nothing happens (update K7) */
		  break;
		case 18: /* PC=9, A cycle, update K7 (calc K8) */
		  tms->current_k[6] += ((tms->target_k[6] - tms->current_k[6]) >> interp_coeff[interp_period]);
            	  break;
                case 19: /* PC=9, B cycle, nothing happens (update K8) */
		  break;
		case 20: /* PC=10, A cycle, update K8 (calc K9) */
		  tms->current_k[7] += ((tms->target_k[7] - tms->current_k[7]) >> interp_coeff[interp_period]);
            	  break;
                case 21: /* PC=10, B cycle, nothing happens (update K9) */
		  break;
		case 22: /* PC=11, A cycle, update K9 (calc K10) */
		  tms->current_k[8] += ((tms->target_k[8] - tms->current_k[8]) >> interp_coeff[interp_period]);
            	  break;
                case 23: /* PC=11, B cycle, nothing happens (update K10) */
		  break;
		case 24: /* PC=12, A cycle, update K10 (do nothing) */
		  tms->current_k[9] += ((tms->target_k[9] - tms->current_k[9]) >> interp_coeff[interp_period]);
            	  break;
	    }
        }

        if (tms->old_energy == 0)
        {
            /* generate silent samples here */
			tms->excitation_data = 0x00; /* This is NOT correct, the current_energy is forced to zero when we                         just passed a zero energy frame because thats what the tables hold for that value.
                        However, this code does no harm. Will be removed later. */
        }
        else if (tms->old_pitch == 0)
        {
            /* generate unvoiced samples here */
			if (tms->RNG & 1)
				tms->excitation_data = -0x40; /* according to the patent it is (either + or -) half of the maximum value in the chirp table, so +-64 */
			else
				tms->excitation_data = 0x40;
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
          if (tms->pitch_count > 50)
              tms->excitation_data = chirptable[50];
          else /*tms->pitch_count <= 50*/
              tms->excitation_data = chirptable[tms->pitch_count];
        }

        /* Update LFSR *20* times every sample, like patent shows */
	for (i=0; i<20; i++)
	{
            bitout = ((tms->RNG >> 12) & 1) ^
                     ((tms->RNG >> 10) & 1) ^
                     ((tms->RNG >>  9) & 1) ^
                     ((tms->RNG >>  0) & 1);
            tms->RNG >>= 1;
            tms->RNG |= bitout << 12;
	}

		buffer[buf_count] = clip_and_wrap(lattice_filter(tms)); /* execute lattice filter and clipping/wrapping */

        if (tms->digital_select == 0) /* if digital is NOT selected... */
		  buffer[buf_count] &= 0xff00; /* mask out all but the 8 dac bits */

        /* Update all counts */

        size--;
        tms->sample_count = (tms->sample_count + 1) % 200;

        if (tms->current_pitch != 0)
            tms->pitch_count = (tms->pitch_count + 1) % tms->current_pitch;
        else
            tms->pitch_count = 0;

        tms->interp_count = (tms->interp_count + 1) % 25;
        buf_count++;
    }

empty:

    while (size > 0)
    {
		tms->sample_count = (tms->sample_count + 1) % 200;
		tms->interp_count = (tms->interp_count + 1) % 25;
		buffer[buf_count] = 0x00;	/* should be (-1 << 8) ??? (cf note in data sheet, p 10, table 4) */
        buf_count++;
        size--;
    }
}

/**********************************************************************************************

     clip_and_wrap -- clips and wraps the 14 bit return value from the lattice filter to its final 10 bit value (-512 to 511), and upshifts this to 16 bits

***********************************************************************************************/

static INT16 clip_and_wrap(INT16 cliptemp)
{
        /* clipping & wrapping, just like the patent shows */

	if (cliptemp > 2047) cliptemp = -2048 + (cliptemp-2047);
	else if (cliptemp < -2048) cliptemp = 2047 - (cliptemp+2048);

	if (cliptemp > 511) { mame_printf_debug ("cliptemp > 511\n");
	    return 127<<8; }
        else if (cliptemp < -512) { mame_printf_debug ("cliptemp < -512\n");
	    return -128<<8; }
        else
            return cliptemp << 6;
}


/**********************************************************************************************

     lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
     excitation data, and specific values of all the current k constants,  and returns the
     resulting sample.
     Note: the current_k processing here by dividing the result by 32768 is necessary, as the stored
     parameters in the lookup table are the 10 bit coefficients but are pre-multiplied by 512 for
     ease of storage. This is undone on the real chip by a shifter here, after the multiply.

***********************************************************************************************/

static INT16 lattice_filter(tms5220_state *tms)
{
   /* Lattice filter here */
   /* Aug/05/07: redone as unrolled loop, for clarity - LN*/
   /* Copied verbatim from table I in US patent 4,209,804:
      notation equivalencies from table:
      Yn(i) == tms->u[n-1]
      Kn = tms->current_k[n-1]
      bn = tms->x[n-1]
    */
        tms->u[10] = (tms->excitation_data * tms->previous_energy) >> 8; /* Y(11) */
        tms->u[9] = tms->u[10] - ((tms->current_k[9] * tms->x[9]) / 32768);
        tms->u[8] = tms->u[9] - ((tms->current_k[8] * tms->x[8]) / 32768);
        tms->x[9] = tms->x[8] + ((tms->current_k[8] * tms->u[8]) / 32768);
        tms->u[7] = tms->u[8] - ((tms->current_k[7] * tms->x[7]) / 32768);
        tms->x[8] = tms->x[7] + ((tms->current_k[7] * tms->u[7]) / 32768);
        tms->u[6] = tms->u[7] - ((tms->current_k[6] * tms->x[6]) / 32768);
        tms->x[7] = tms->x[6] + ((tms->current_k[6] * tms->u[6]) / 32768);
        tms->u[5] = tms->u[6] - ((tms->current_k[5] * tms->x[5]) / 32768);
        tms->x[6] = tms->x[5] + ((tms->current_k[5] * tms->u[5]) / 32768);
        tms->u[4] = tms->u[5] - ((tms->current_k[4] * tms->x[4]) / 32768);
        tms->x[5] = tms->x[4] + ((tms->current_k[4] * tms->u[4]) / 32768);
        tms->u[3] = tms->u[4] - ((tms->current_k[3] * tms->x[3]) / 32768);
        tms->x[4] = tms->x[3] + ((tms->current_k[3] * tms->u[3]) / 32768);
        tms->u[2] = tms->u[3] - ((tms->current_k[2] * tms->x[2]) / 32768);
        tms->x[3] = tms->x[2] + ((tms->current_k[2] * tms->u[2]) / 32768);
        tms->u[1] = tms->u[2] - ((tms->current_k[1] * tms->x[1]) / 32768);
        tms->x[2] = tms->x[1] + ((tms->current_k[1] * tms->u[1]) / 32768);
        tms->u[0] = tms->u[1] - ((tms->current_k[0] * tms->x[0]) / 32768);
        tms->x[1] = tms->x[0] + ((tms->current_k[0] * tms->u[0]) / 32768);
        tms->x[0] = tms->u[0];
        tms->previous_energy = tms->current_energy;
        return tms->u[0];
}


/**********************************************************************************************

     process_command -- extract a byte from the FIFO and interpret it as a command

***********************************************************************************************/

static void process_command(tms5220_state *tms)
{
    unsigned char cmd;

    /* if there are stray bits, ignore them */
	if (tms->fifo_bits_taken)
	{
		tms->fifo_bits_taken = 0;
        tms->fifo_count--;
        tms->fifo_head = (tms->fifo_head + 1) % FIFO_SIZE;
    }

    /* grab a full byte from the FIFO */
    if (tms->fifo_count > 0)
    {
		cmd = tms->fifo[tms->fifo_head];
		tms->fifo_count--;
		tms->fifo_head = (tms->fifo_head + 1) % FIFO_SIZE;

		/* parse the command */
		switch (cmd & 0x70)
		{
		case 0x10 : /* read byte */
			if (tms->schedule_dummy_read)
			{
				tms->schedule_dummy_read = FALSE;
				if (tms->intf->read)
					(*tms->intf->read)(tms->device, 1);
			}
			if (tms->intf->read)
				tms->data_register = (*tms->intf->read)(tms->device, 8);	/* read one byte from speech ROM... */
			tms->RDB_flag = TRUE;
			break;

		case 0x30 : /* read and branch */
			LOG(("read and branch command received\n"));
			tms->RDB_flag = FALSE;
			if (tms->intf->read_and_branch)
				(*tms->intf->read_and_branch)(tms->device);
			break;

		case 0x40 : /* load address */
			/* tms5220 data sheet says that if we load only one 4-bit nibble, it won't work.
              This code does not care about this. */
			if (tms->intf->load_address)
				(*tms->intf->load_address)(tms->device, cmd & 0x0f);
			tms->schedule_dummy_read = TRUE;
			break;

		case 0x50 : /* speak */
			if (tms->schedule_dummy_read)
			{
				tms->schedule_dummy_read = FALSE;
				if (tms->intf->read)
					(*tms->intf->read)(tms->device, 1);
			}
			tms->tms5220_speaking = 1;
			tms->speak_external = 0;
			if (! tms->last_frame)
			{
				tms->first_frame = 1;
			}
			tms->talk_status = 1;  /* start immediately */
			break;

		case 0x60 : /* speak external */
			tms->tms5220_speaking = tms->speak_external = 1;
			tms->RDB_flag = FALSE;

            /* according to the datasheet, this will cause an interrupt due to a BE condition */
            if (!tms->buffer_empty)
            {
                tms->buffer_empty = 1;
                set_interrupt_state(tms, 1);
            }

			tms->talk_status = 0;	/* wait to have 8 bytes in buffer before starting */
			break;

		case 0x70 : /* reset */
			if (tms->schedule_dummy_read)
			{
				tms->schedule_dummy_read = FALSE;
				if (tms->intf->read)
					(*tms->intf->read)(tms->device, 1);
			}
			device_reset(tms->device);
			break;
        }
    }

    /* update the buffer low state */
    check_buffer_low(tms);
}



/**********************************************************************************************

     extract_bits -- extract a specific number of bits from the FIFO

***********************************************************************************************/

static int extract_bits(tms5220_state *tms, int count)
{
    int val = 0;

	if (tms->speak_external)
	{
		/* extract from FIFO */
    	while (count--)
    	{
        	val = (val << 1) | ((tms->fifo[tms->fifo_head] >> tms->fifo_bits_taken) & 1);
        	tms->fifo_bits_taken++;
        	if (tms->fifo_bits_taken >= 8)
        	{
        	    tms->fifo_count--;
        	    tms->fifo_head = (tms->fifo_head + 1) % FIFO_SIZE;
        	    tms->fifo_bits_taken = 0;
        	}
    	}
    }
	else
	{
		/* extract from speech ROM */
		if (tms->intf->read)
			val = (* tms->intf->read)(tms->device, count);
	}

    return val;
}



/**********************************************************************************************

     parse_frame -- parse a new frame's worth of data; returns 0 if not enough bits in buffer

***********************************************************************************************/

static int parse_frame(tms5220_state *tms, int the_first_frame)
{
	int bits = 0;	/* number of bits in FIFO (speak external only) */
	int indx, i, rep_flag;

	if (! the_first_frame)
	{
    /* remember previous frame */
    tms->old_energy = tms->new_energy;
    tms->old_pitch = tms->new_pitch;
    for (i = 0; i < 10; i++)
        tms->old_k[i] = tms->new_k[i];
	}

    /* clear out the new frame */
    tms->new_energy = 0;
    tms->new_pitch = 0;
    for (i = 0; i < 10; i++)
        tms->new_k[i] = 0;

    /* if the previous frame was a stop frame, don't do anything */
	if ((! the_first_frame) && (tms->old_energy == (energytable[15] >> 6)))
		return 1;
//  WARNING: This code below breaks Victory's power-on test! If you change it
//  make sure you test Victory.
//  {
//      LOG(("Buffer Empty set - Last frame stop frame\n");

//      tms->buffer_empty = 1;
//      return 1;
//  }

	if (tms->speak_external)
    	/* count the total number of bits available */
		bits = tms->fifo_count * 8 - tms->fifo_bits_taken;

    /* attempt to extract the energy index */
	if (tms->speak_external)
	{
    bits -= 4;
    if (bits < 0)
        goto ranout;
	}
    indx = extract_bits(tms, 4);
    tms->new_energy = energytable[indx] >> 6;

	/* if the index is 0 or 15, we're done */
	if (indx == 0 || indx == 15)
	{
		LOG(("  (4-bit energy=%d frame)\n",tms->new_energy));

		/* clear tms->fifo if stop frame encountered */
		if (indx == 15)
		{
			tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;
			tms->speak_external = tms->tms5220_speaking = 0;
			tms->last_frame = 1;
		}
		goto done;
	}

    /* attempt to extract the repeat flag */
	if (tms->speak_external)
	{
    bits -= 1;
    if (bits < 0)
        goto ranout;
	}
    rep_flag = extract_bits(tms, 1);

    /* attempt to extract the pitch */
	if (tms->speak_external)
	{
    bits -= 6;
    if (bits < 0)
        goto ranout;
	}
    indx = extract_bits(tms, 6);
    tms->new_pitch = pitchtable[indx] / 256;

    /* if this is a repeat frame, just copy the k's */
    if (rep_flag)
    {
        for (i = 0; i < 10; i++)
            tms->new_k[i] = tms->old_k[i];

        LOG(("  (11-bit energy=%d pitch=%d rep=%d frame)\n", tms->new_energy, tms->new_pitch, rep_flag));
        goto done;
    }

    /* if the pitch index was zero, we need 4 k's */
    if (indx == 0)
    {
        /* attempt to extract 4 K's */
		if (tms->speak_external)
		{
        bits -= 18;
        if (bits < 0)
            goto ranout;
		}
        tms->new_k[0] = k1table[extract_bits(tms, 5)];
        tms->new_k[1] = k2table[extract_bits(tms, 5)];
        tms->new_k[2] = k3table[extract_bits(tms, 4)];
		if (tms->variant == variant_tmc0285)
			tms->new_k[3] = k3table[extract_bits(tms, 4)];	/* ??? */
		else
			tms->new_k[3] = k4table[extract_bits(tms, 4)];

        LOG(("  (29-bit energy=%d pitch=%d rep=%d 4K frame)\n", tms->new_energy, tms->new_pitch, rep_flag));
        goto done;
    }

    /* else we need 10 K's */
	if (tms->speak_external)
	{
    bits -= 39;
    if (bits < 0)
        goto ranout;
	}

    tms->new_k[0] = k1table[extract_bits(tms, 5)];
    tms->new_k[1] = k2table[extract_bits(tms, 5)];
    tms->new_k[2] = k3table[extract_bits(tms, 4)];
	if (tms->variant == variant_tmc0285)
		tms->new_k[3] = k3table[extract_bits(tms, 4)];	/* ??? */
	else
		tms->new_k[3] = k4table[extract_bits(tms, 4)];
    tms->new_k[4] = k5table[extract_bits(tms, 4)];
    tms->new_k[5] = k6table[extract_bits(tms, 4)];
    tms->new_k[6] = k7table[extract_bits(tms, 4)];
    tms->new_k[7] = k8table[extract_bits(tms, 3)];
    tms->new_k[8] = k9table[extract_bits(tms, 3)];
    tms->new_k[9] = k10table[extract_bits(tms, 3)];

    LOG(("  (50-bit energy=%d pitch=%d rep=%d 10K frame)\n", tms->new_energy, tms->new_pitch, rep_flag));

done:
	if (tms->speak_external)
		LOG(("Parsed a frame successfully in FIFO - %d bits remaining\n", bits));
	else
		LOG(("Parsed a frame successfully in ROM\n"));

	if (the_first_frame)
	{
		/* if this is the first frame, no previous frame to take as a starting point */
		tms->old_energy = tms->new_energy;
		tms->old_pitch = tms->new_pitch;
		for (i = 0; i < 10; i++)
			tms->old_k[i] = tms->new_k[i];
    }

    /* update the tms->buffer_low status */
    check_buffer_low(tms);
    return 1;

ranout:

    LOG(("Ran out of bits on a parse!\n"));

    /* this is an error condition; mark the buffer empty and turn off speaking */
    tms->buffer_empty = 1;
	tms->talk_status = tms->speak_external = tms->tms5220_speaking = the_first_frame = tms->last_frame = 0;
    tms->fifo_count = tms->fifo_head = tms->fifo_tail = 0;

	tms->RDB_flag = FALSE;

    /* generate an interrupt if necessary */
    set_interrupt_state(tms, 1);
    return 0;
}



/**********************************************************************************************

     check_buffer_low -- check to see if the buffer low flag should be on or off

***********************************************************************************************/

static void check_buffer_low(tms5220_state *tms)
{
    /* did we just become low? */
    if (tms->fifo_count <= 8)
    {
        /* generate an interrupt if necessary */
        if (!tms->buffer_low)
            set_interrupt_state(tms, 1);
        tms->buffer_low = 1;

        LOG(("Buffer low set\n"));
    }

    /* did we just become full? */
    else
    {
        tms->buffer_low = 0;

        LOG(("Buffer low cleared\n"));
    }
}



/**********************************************************************************************

     set_interrupt_state -- generate an interrupt

***********************************************************************************************/

static void set_interrupt_state(tms5220_state *tms, int state)
{
    if (tms->irq_func.write && state != tms->irq_pin)
    	devcb_call_write_line(&tms->irq_func, !state);
    tms->irq_pin = state;
}


/**********************************************************************************************

     DEVICE_START( tms5220 ) -- allocate buffers and reset the 5220

***********************************************************************************************/

static DEVICE_START( tms5220 )
{
	static const tms5220_interface dummy = { DEVCB_NULL };
	tms5220_state *tms = get_safe_token(device);

	/* set the interface and device */
	tms->intf = device->static_config ? (const tms5220_interface *)device->static_config : &dummy;
	tms->device = device;
	tms->clock = device->clock;

	/* resolve */

	devcb_resolve_write_line(&tms->irq_func, &tms->intf->irq_func, device);

	/* initialize a stream */
	tms->stream = stream_create(device, 0, 1, device->clock / 80, tms, tms5220_update);

	/* not during reset which is called frm within a write! */
	tms->io_ready = 1;
	tms->true_timing = 0;

	tms->variant = variant_tms5220;
	register_for_save_states(tms);
}

static DEVICE_START( tms5220c )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms->variant = variant_tms5220c;
}

static DEVICE_START( tmc0285 )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms->variant = variant_tmc0285;
}


static DEVICE_START( tms5200 )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms->variant = variant_tmc0285;
}


static DEVICE_RESET( tms5220 )
{
	tms5220_state *tms = get_safe_token(device);

	/* initialize the FIFO */
	/*memset(tms->fifo, 0, sizeof(tms->fifo));*/
	tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;

	/* initialize the chip state */
	/* Note that we do not actually clear IRQ on start-up : IRQ is even raised if tms->buffer_empty or tms->buffer_low are 0 */
	tms->tms5220_speaking = tms->speak_external = tms->talk_status = tms->first_frame = tms->last_frame = tms->irq_pin = 0;
	set_interrupt_state(tms, 0);
	tms->buffer_empty = tms->buffer_low = 1;

	tms->RDB_flag = FALSE;

	/* initialize the energy/pitch/k states */
	tms->old_energy = tms->new_energy = tms->current_energy = tms->target_energy = 0;
	tms->old_pitch = tms->new_pitch = tms->current_pitch = tms->target_pitch = 0;
	memset(tms->old_k, 0, sizeof(tms->old_k));
	memset(tms->new_k, 0, sizeof(tms->new_k));
	memset(tms->current_k, 0, sizeof(tms->current_k));
	memset(tms->target_k, 0, sizeof(tms->target_k));

	/* initialize the sample generators */
	tms->interp_count = tms->sample_count = tms->pitch_count = 0;
        tms->RNG = 0x1FFF;
	memset(tms->u, 0, sizeof(tms->u));
	memset(tms->x, 0, sizeof(tms->x));

	if (tms->intf->load_address)
		(*tms->intf->load_address)(tms->device, 0);

	tms->schedule_dummy_read = TRUE;
}

/**********************************************************************************************

     True timing

***********************************************************************************************/

static TIMER_CALLBACK( io_ready_cb )
{
	tms5220_state *tms = (tms5220_state *) ptr;
	if (param)
	{
		switch (tms->rs_ws)
		{
		case 0x02:
			/* Write */
		    /* bring up to date first */
			LOG(("Service write %02x\n", tms->write_latch));
		    stream_update(tms->stream);
		    tms5220_data_write(tms, tms->write_latch);
		    break;
		case 0x01:
			/* Read */
		    /* bring up to date first */
		    stream_update(tms->stream);
		    tms->read_latch = tms5220_status_read(tms);
			break;
		case 0x03:
			/* High Impedance */
		case 0x00:
			/* illegal */
			break;
		}
	}
	tms->io_ready = param;
}

WRITE_LINE_DEVICE_HANDLER( tms5220_rsq_w )
{
	tms5220_state *tms = get_safe_token(device);
	UINT8 new_val;

	tms->true_timing = 1;
	state &= 0x01;

	new_val = (tms->rs_ws & 0x01) | (state<<1);
	if (new_val != tms->rs_ws)
	{
		tms->rs_ws = new_val;
		if (new_val == 0)
		{
			if (tms->variant == variant_tms5220c)
				device_reset(device);
			else
				/* illegal */
				LOG(("tms5220_rs_w: illegal\n"));
			return;
		}
		else if ( new_val == 3)
		{
			/* high impedance */
			tms->read_latch = 0xff;
			return;
		}
		if (state)
		{
			/* low to high */
		}
		else
		{
			/* high to low - schedule ready cycle*/
			LOG(("Schedule write ready\n"));
			tms->io_ready = 1;
			/* 100 nsec from data sheet */
			timer_set(tms->device->machine, ATTOTIME_IN_NSEC(100), tms, 0, io_ready_cb);
			/* 25 usec in datasheet, but zaccaria won't work */
			timer_set(tms->device->machine, ATTOTIME_IN_USEC(100), tms, 1, io_ready_cb);
		}
	}
}

WRITE_LINE_DEVICE_HANDLER( tms5220_wsq_w )
{
	tms5220_state *tms = get_safe_token(device);
	UINT8 new_val;

	tms->true_timing = 1;
	state &= 0x01;
	new_val = (tms->rs_ws & 0x02) | (state<<0);
	if (new_val != tms->rs_ws)
	{
		tms->rs_ws = new_val;
		if (new_val == 0)
		{
			if (tms->variant == variant_tms5220c)
				device_reset(device);
			else
				/* illegal */
				LOG(("tms5220_ws_w: illegal\n"));
			return;
		}
		else if ( new_val == 3)
		{
			/* high impedance */
			tms->read_latch = 0xff;
			return;
		}
		if (state)
		{
			/* low to high  */
		}
		else
		{
			/* high to low - schedule ready cycle*/
			tms->io_ready = 1;
			timer_set(tms->device->machine, ATTOTIME_IN_NSEC(100), tms, 0, io_ready_cb);
			timer_set(tms->device->machine, ATTOTIME_IN_USEC(25), tms, 1, io_ready_cb);
		}
	}
}

/**********************************************************************************************

     tms5220_data_w -- write data to the sound chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( tms5220_data_w )
{
	tms5220_state *tms = get_safe_token(device);

	if (!tms->true_timing)
	{
		/* bring up to date first */
	    stream_update(tms->stream);
	    tms5220_data_write(tms, data);
	}
	else
	{
		/* actually in a write ? */
		if (!(tms->rs_ws == 0x02))
			LOG(("tms5220_data_w: data written outside ws, status: %02x!\n", tms->rs_ws));
		tms->write_latch = data;
	}
}



/**********************************************************************************************

     tms5220_status_r -- read status or data from the sound chip

***********************************************************************************************/

READ8_DEVICE_HANDLER( tms5220_status_r )
{
	tms5220_state *tms = get_safe_token(device);
	if (!tms->true_timing)
	{
	   /* bring up to date first */
	    stream_update(tms->stream);
	    return tms5220_status_read(tms);
	}
	else
	{
		/* actually in a read ? */
		if (tms->rs_ws == 0x01)
			return tms->read_latch;
		else
			LOG(("tms5220_status_r: data read outside rs!\n"));
		return 0xff;
	}
}



/**********************************************************************************************

     tms5220_ready_r -- return the not ready status from the sound chip

***********************************************************************************************/

READ_LINE_DEVICE_HANDLER( tms5220_readyq_r )
{
	tms5220_state *tms = get_safe_token(device);
    /* bring up to date first */
    stream_update(tms->stream);
    return !tms5220_ready_read(tms);
}



/**********************************************************************************************

     tms5220_time_to_ready -- return the time in seconds until the ready line is asserted

***********************************************************************************************/

double tms5220_time_to_ready(const device_config *device)
{
	tms5220_state *tms = get_safe_token(device);
	double cycles;

	/* bring up to date first */
	stream_update(tms->stream);
	cycles = tms5220_cycles_to_ready(tms);
	return cycles * 80.0 / tms->clock;
}



/**********************************************************************************************

     tms5220_int_r -- return the int status from the sound chip

***********************************************************************************************/

READ_LINE_DEVICE_HANDLER( tms5220_intq_r )
{
	tms5220_state *tms = get_safe_token(device);
    /* bring up to date first */
    stream_update(tms->stream);
    return !tms5220_int_read(tms);
}



/**********************************************************************************************

     tms5220_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( tms5220_update )
{
	tms5220_state *tms = (tms5220_state *)param;
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		tms5220_process(tms, sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
	}
}



/**********************************************************************************************

     tms5220_set_frequency -- adjusts the playback frequency

***********************************************************************************************/

void tms5220_set_frequency(const device_config *device, int frequency)
{
	tms5220_state *tms = get_safe_token(device);
	stream_set_sample_rate(tms->stream, frequency / 80);
	tms->clock = frequency;
}



/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##tms5220##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"TMS5220"
#define DEVTEMPLATE_FAMILY				"TI Speech"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##tms5220c##s
#define DEVTEMPLATE_DERIVED_FEATURES	DT_HAS_START
#define DEVTEMPLATE_DERIVED_NAME		"TMS5220C"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##tmc0285##s
#define DEVTEMPLATE_DERIVED_FEATURES	DT_HAS_START
#define DEVTEMPLATE_DERIVED_NAME		"TMC0285"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##tms5200##s
#define DEVTEMPLATE_DERIVED_FEATURES	DT_HAS_START
#define DEVTEMPLATE_DERIVED_NAME		"TMS5200"
#include "devtempl.h"
