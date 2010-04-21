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
     Preliminary MASSIVE merge of tms5110 and tms5220 cores by Lord Nightmare
     Lattice Filter, Multiplier, and clipping redone by Lord Nightmare
     TMS5220C multi-rate feature added by Lord Nightmare

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
    tomcat has a 5220 which is not hooked up at all

Progress list for drivers using old vs new interface:
starwars: uses new interface (couriersud)
gauntlet: uses new interface (couriersud
atarisy1: uses new interface (Lord Nightmare)
atarisy2: uses new interface (Lord Nightmare)
atarijsa: uses new interface (Lord Nightmare)
firefox: uses new interface (couriersud)
mhavoc: uses old interface, and is in the machine file instead of the driver.
monymony(zaccaria.c): uses new interface (couriersud)
victory(audio/exidy.c): uses new interface (couriersud)
looping: uses old interface
portraits: uses *NO* interface; the i/o cpu hasn't been hooked to anything!
dotron and midwayfb(mcr.c): uses old interface

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

TMS5220: (mostly on things made between 1982 and 1984-1985)
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

  Changes by R. Nabet (from 2001)
   * added Speech ROM support
   * modified code so that the beast only start speaking at the start of next frame, like the data
     sheet says

***********************************************************************************************/

#include "emu.h"
#include "streams.h"
#include "tms5220.h"

/* *****optional defines***** */

/* this ignores pitch, and uses a sawtooth wave for the voiced and unvoiced waveforms as a test, if not set */
#define NORMALMODE 1

/* if not defined, output the waveform as if it was tapped on the i/o pin */
#define DO_CLIP_AND_WRAP 1

/* if defined, interpolation is only using the said slot of the 8,8,8,4,4,4,2,1 slots
   i.e. setting to 7 effectively disables interpolation, as it adds 1/1 of the difference
   between current and target to the current each frame */
#undef OVERRIDE_INTERPOLATION

/* defines how many times to leftshift the chirp rom before feeding to the lattice filter. default is 0 */
#define CHIRPROM_LEFTSHIFT 0

/* *****debugging defines***** */
#undef VERBOSE
// above is general, somewhat obsolete
#undef DEBUG_FIFO
// above debugs fifo stuff: writes, reads and flag updates
#undef DEBUG_FRAME_DUMP
// above dumps the contents of each decoded speech frame as hex
#undef DEBUG_FRAME_INFO
// above dumps information about each decoded speech frame
#undef DEBUG_FRAME_ERRORS
// above dumps info if a frame ran out of data
#undef DEBUG_COMMAND_DUMP
// above dumps all non-speech-data command writes
#undef DEBUG_PIN_READS
// above spams the errorlog with i/o ready messages whenever the ready or irq pin is read
#undef DEBUG_GENERATION
// above dumps some debug information related to the sample generation loop, i.e. when ramp frames happen
#undef DEBUG_IO_READY
// above debugs the io ready callback
#undef DEBUG_RS_WS
// above debugs the new up-and-coming tms5220_data_r and data_w access methods which actually respect rs and ws

#define MAX_SAMPLE_CHUNK	512
#define FIFO_SIZE 16

/* Variants */

#define TMS5220_IS_5220C	(4)
#define TMS5220_IS_5200		(5)
#define TMS5220_IS_5220		(6)

#define TMS5220_IS_TMC0285	TMS5220_IS_5200

UINT8 reload_table[4] = { 0, 50, 100, 150 }; //is the sample count reload for 5220c only; 5200 and 5220 always reload with 0

typedef struct _tms5220_state tms5220_state;
struct _tms5220_state
{
	/* coefficient tables */
	int variant;				/* Variant of the 5xxx - see tms5110r.h */

	/* coefficient tables */
	const struct tms5100_coeffs *coeff;

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
        speaking_now is set whenever a speak or speak external command is going on.
        Note that we really need to do anything in tms5220_process and play samples only when
        speaking_now is true.  Else, we can play nothing as well, which is a
        speed-up...
    */
	UINT8 speaking_now;		/* Speak or Speak External command in progress */
	UINT8 speak_external;	/* Speak External command in progress, writes go to FIFO. */
	UINT8 talk_status;		/* TS status bit is 1, i.e. speak or speak external is in progress and we have not encountered a stop frame yet; talk_status differs from speaking_now in that speaking_now is set as soon as a speak or speak external command is started; talk_status does NOT go active until after 8 bytes are written to the fifo on a speak external command, otherwise the two are the same. TS is cleared when a STOP command has just been processed in the speech stream, and a new command is about to be processed. */
	UINT8 buffer_low;		/* FIFO has less than 8 bytes in it */
	UINT8 buffer_empty;		/* FIFO is empty*/
	UINT8 irq_pin;			/* state of the IRQ pin (output) */

	/* these contain data describing the current and previous voice frames */
#define OLD_FRAME_STOP_FLAG (tms->old_frame_energy == COEFF_ENERGY_SENTINEL) // 1 if this was a stop (E = 0xF) frame
#define OLD_FRAME_SILENCE_FLAG (tms->old_frame_energy == 0) // 1 if this was a silence (E=0) frame
#define OLD_FRAME_UNVOICED_FLAG (tms->old_frame_pitch == 0) // 1 if unvoiced, 0 if voiced
	UINT16 old_frame_energy;
	UINT16 old_frame_pitch;
	INT32 old_frame_k[10];

#define NEW_FRAME_STOP_FLAG (tms->new_frame_energy == COEFF_ENERGY_SENTINEL)  // ditto as above
#define NEW_FRAME_SILENCE_FLAG (tms->new_frame_energy == 0) // ditto as above
#define NEW_FRAME_UNVOICED_FLAG (tms->new_frame_pitch == 0) // ditto as above
	UINT16 new_frame_energy;
	UINT16 new_frame_pitch;
	INT32 new_frame_k[10];


	/* these are all used to contain the current state of the sound generation */
	UINT16 current_energy;
	UINT16 current_pitch;
	INT32 current_k[10];

	UINT16 target_energy;
	UINT16 target_pitch;
	INT32 target_k[10];

	UINT16 previous_energy;         /* needed for lattice filter to match patent */

	//UINT8 interp_period; /* TODO: the current interpolation period, counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,4,2,1 */
	UINT8 interp_count;		/* number of samples within each sub-interpolation period, ranges from 0-24 */
	UINT8 sample_count;		/* number of samples within the ENTIRE interpolation period, ranges from 0-199 */
	UINT8 tms5220c_rate; /* only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command */
	UINT16 pitch_count;		/* pitch counter; provides chirp rom address */

	INT32 u[11];
	INT32 x[10];

	INT32 RNG;      /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */
	INT16 excitation_data;

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

    /* The TMS52xx has two different ways of providing output data: the
       analog speaker pin (which was usually used) and the Digital I/O pin.
       The internal DAC used to feed the analog pin is only 8 bits, and has the
       funny clipping/clamping logic, while the digital pin gives full 12? bit
       resolution of the output data.
     */
	UINT8 digital_select;
	running_device *device;

	const tms5220_interface *intf;
	sound_stream *stream;
	int clock;
};


/* Pull in the ROM tables */
#include "tms5110r.c"


INLINE tms5220_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_TMS5220 ||
		   sound_get_type(device) == SOUND_TMS5220C ||
		   sound_get_type(device) == SOUND_TMC0285 ||
		   sound_get_type(device) == SOUND_TMS5200);
	return (tms5220_state *)device->token;
}

/* Static function prototypes */
static void process_command(tms5220_state *tms, unsigned char data);
static void parse_frame(tms5220_state *tms);
static void update_status_and_ints(tms5220_state *tms);
static void set_interrupt_state(tms5220_state *tms, int state);
static INT16 lattice_filter(tms5220_state *tms);
static INT16 clip_and_wrap(INT16 cliptemp);
static STREAM_UPDATE( tms5220_update );

void tms5220_set_variant(tms5220_state *tms, int variant)
{
	switch (variant)
	{
		case TMS5220_IS_5220C:
			tms->coeff = &tms5220c_coeff;
			break;
		case TMS5220_IS_5200:
			tms->coeff = &tms5200_coeff;
			break;
		case TMS5220_IS_5220:
			tms->coeff = &tms5220_coeff;
			break;
		default:
			fatalerror("Unknown variant in tms5220_set_variant\n");
	}

	tms->variant = variant;
}


static void register_for_save_states(tms5220_state *tms)
{
	state_save_register_device_item_array(tms->device, 0, tms->fifo);
	state_save_register_device_item(tms->device, 0, tms->fifo_head);
	state_save_register_device_item(tms->device, 0, tms->fifo_tail);
	state_save_register_device_item(tms->device, 0, tms->fifo_count);
	state_save_register_device_item(tms->device, 0, tms->fifo_bits_taken);

	state_save_register_device_item(tms->device, 0, tms->speaking_now);
	state_save_register_device_item(tms->device, 0, tms->speak_external);
	state_save_register_device_item(tms->device, 0, tms->talk_status);
	state_save_register_device_item(tms->device, 0, tms->buffer_low);
	state_save_register_device_item(tms->device, 0, tms->buffer_empty);
	state_save_register_device_item(tms->device, 0, tms->irq_pin);

	state_save_register_device_item(tms->device, 0, tms->old_frame_energy);
	state_save_register_device_item(tms->device, 0, tms->old_frame_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->old_frame_k);

	state_save_register_device_item(tms->device, 0, tms->new_frame_energy);
	state_save_register_device_item(tms->device, 0, tms->new_frame_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->new_frame_k);

	state_save_register_device_item(tms->device, 0, tms->current_energy);
	state_save_register_device_item(tms->device, 0, tms->current_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->current_k);

	state_save_register_device_item(tms->device, 0, tms->target_energy);
	state_save_register_device_item(tms->device, 0, tms->target_pitch);
	state_save_register_device_item_array(tms->device, 0, tms->target_k);

	state_save_register_device_item(tms->device, 0, tms->previous_energy);

	state_save_register_device_item(tms->device, 0, tms->interp_count);
	state_save_register_device_item(tms->device, 0, tms->sample_count);
	state_save_register_device_item(tms->device, 0, tms->tms5220c_rate);
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
	if (tms->speak_external) // If we're in speak external mode
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
#ifdef DEBUG_FIFO
			logerror("data_write: Added byte to FIFO (current count=%2d)\n", tms->fifo_count);
		}
		else
		{

			logerror("data_write: Ran out of room in the FIFO!\n");
			// at this point, /READY should remain HIGH/inactive until the fifo has at least one byte open in it.
#endif
		}

		update_status_and_ints(tms);
	}
	else //(! tms->speak_external)
		/* R Nabet : we parse commands at once.  It is necessary for such commands as read. */
		process_command(tms,data);
}

/**********************************************************************************************

     update_status_and_ints -- check to see if the buffer low flag should be on or off

***********************************************************************************************/

static void update_status_and_ints(tms5220_state *tms)
{
	/* update flags and set ints if needed */
	/* BL is set if neither byte 9 nor 8 of the fifo are in use; this
    translates to having fifo_count (which ranges from 0 bytes in use to 16
    bytes used) being less than or equal to 8. Victory/Victorba depends on this. */
    if (tms->fifo_count <= 8)
    {
        /* generate an interrupt if necessary; if /BL was inactive and is now active, set int. */
        if (!tms->buffer_low)
            set_interrupt_state(tms, 1);
        tms->buffer_low = 1;
	}
	else
		tms->buffer_low = 0;

	/* BE is set if neither byte 15 nor 14 of the fifo are in use; this
    translates to having fifo_count equal to exactly 0 */
	if (tms->fifo_count == 0)
	{
	    /* generate an interrupt if necessary; if /BE was inactive and is now active, set int. */
        if (!tms->buffer_empty)
            set_interrupt_state(tms, 1);
        tms->buffer_empty = 1;
    }
	else
		tms->buffer_empty = 0;

	/* TS is talk status and is set elsewhere in the fifo parser and in
    the SPEAK command handler; however, if /BE is true during speak external
    mode, it is immediately unset here. */
	if ((tms->speak_external == 1) && (tms->buffer_empty == 1))
	{
		/* generate an interrupt if necessary; if /TS was active and is now inactive, set int. */
        if (tms->talk_status == 1)
            set_interrupt_state(tms, 1);
		tms->talk_status = 0;
	}
	/* Note that TS being unset will also generate an interrupt when a STOP
    frame is encountered; this is handled in the sample generator code and not here */
}

/**********************************************************************************************

     extract_bits -- extract a specific number of bits from the current input stream (FIFO or VSM)

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
				tms->fifo[tms->fifo_head] = 0; // zero the newly depleted fifo head byte
				tms->fifo_head = (tms->fifo_head + 1) % FIFO_SIZE;
				tms->fifo_bits_taken = 0;
				update_status_and_ints(tms);
			}
		}
	}
	else
	{
		/* extract from VSM (speech ROM) */
		if (tms->intf->read)
			val = (* tms->intf->read)(tms->device, count);
	}

    return val;
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

		/* clear the interrupt pin on status read */
		set_interrupt_state(tms, 0);
#ifdef DEBUG_PIN_READS
		logerror("Status read: TS=%d BL=%d BE=%d\n", tms->talk_status, tms->buffer_low, tms->buffer_empty);
#endif

		return (tms->talk_status << 7) | (tms->buffer_low << 6) | (tms->buffer_empty << 5);
	}
}



/**********************************************************************************************

     tms5220_ready_read -- returns the ready state of the TMS5220

***********************************************************************************************/

static int tms5220_ready_read(tms5220_state *tms)
{
#ifdef DEBUG_PIN_READS
	logerror("ready_read: ready pin read, io_ready is %d, fifo count is %d\n", tms->io_ready, tms->fifo_count);
#endif
    return ((tms->fifo_count < FIFO_SIZE)||(!tms->speak_external)) && tms->io_ready;
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
#ifdef DEBUG_PIN_READS
	logerror("int_read: irq pin read, state is %d\n", tms->irq_pin);
#endif
    return tms->irq_pin;
}



/**********************************************************************************************

     tms5220_process -- fill the buffer with a specific number of samples

***********************************************************************************************/

static void tms5220_process(tms5220_state *tms, INT16 *buffer, unsigned int size)
{
    int buf_count=0;
    int i, interp_period, bitout;

//tryagain:

    /* if we're empty and still not speaking, fill with nothingness */
	if (!tms->speaking_now)
        goto empty;

    /* if speak external is set, but talk status is not (yet) set,
    wait for buffer low to clear */
	if (!tms->talk_status && tms->speak_external)
    {
        if (tms->buffer_low == 1)
           goto empty;

		/* we now have enough bytes; clear out the new frame parameters (it will become old frame just before the first call to parse_frame() ) */
		tms->new_frame_energy = 0;
		tms->new_frame_pitch = 0;
		for (i = 0; i < tms->coeff->num_k; i++)
			tms->new_frame_k[i] = 0;

        tms->talk_status = 1;
	}

    /* loop until the buffer is full or we've stopped speaking */
	while ((size > 0) && tms->talk_status)
    {

        /* if we're ready for a new frame */
        if ((tms->interp_count == 0) && (tms->sample_count == 0))
        {

			/* remember previous frame energy, pitch, and coefficients */
			tms->old_frame_energy = tms->new_frame_energy;
			tms->old_frame_pitch = tms->new_frame_pitch;
			for (i = 0; i < tms->coeff->num_k; i++)
				tms->old_frame_k[i] = tms->new_frame_k[i];

			/* if the old frame was a stop frame, exit and do not process any more frames */
			if (OLD_FRAME_STOP_FLAG)
			{
#ifdef DEBUG_GENERATION
				logerror("tms5220_process: processing frame: stop frame\n");
#endif
				tms->speaking_now = tms->talk_status = tms->speak_external = 0;
				set_interrupt_state(tms, 1); // TS went inactive, so int is raised
				tms->sample_count = reload_table[tms->tms5220c_rate&0x3];
				update_status_and_ints(tms);
				goto empty;
			}


			/* Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[] */
			parse_frame(tms);


			/* Set old frame targets as starting point of new frame */
			tms->current_energy = tms->old_frame_energy;
			tms->current_pitch = tms->old_frame_pitch;
			for (i = 0; i < tms->coeff->num_k; i++)
				tms->current_k[i] = tms->old_frame_k[i];


			/* in all cases where interpolation would be inhibited, set the target
               value equal to the current value.
               Interpolation inhibit cases:
               * Old frame was voiced, new is unvoiced
               * Old frame was silence/zero energy, new is unvoiced
               * Old frame was unvoiced, new is voiced
               * New frame is a silence/zero energy frame - ? not sure
               * New frame is a stop frame - ? not sure
            */
			if ( ((NEW_FRAME_SILENCE_FLAG == 0) && (NEW_FRAME_STOP_FLAG == 0))
			&& ( ((OLD_FRAME_UNVOICED_FLAG == 0) && (NEW_FRAME_UNVOICED_FLAG == 1))
				|| ((OLD_FRAME_SILENCE_FLAG == 1) && (NEW_FRAME_UNVOICED_FLAG == 1))
				|| ((OLD_FRAME_UNVOICED_FLAG == 1) && (NEW_FRAME_UNVOICED_FLAG == 0)) )
				)
			{
#ifdef DEBUG_GENERATION
				logerror("processing frame: interpolation inhibited\n");
				logerror("*** current Energy = %d\n",tms->current_energy);
				logerror("*** next frame Energy = %d\n",tms->new_frame_energy);
#endif
				tms->target_energy = tms->current_energy;
				tms->target_pitch = tms->current_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->current_k[i];
			}
			/* in cases where the new frame is a STOP or SILENCE frame,
               ramp the energy down to 0, leaving everything else alone. */
			else if ((NEW_FRAME_SILENCE_FLAG == 1) || (NEW_FRAME_STOP_FLAG == 1))
			{
#ifdef DEBUG_GENERATION
				logerror("processing frame: ramp-down (energy = 0 or STOP)\n";
#endif
				tms->target_energy = 0;
				tms->target_pitch = tms->current_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->current_k[i];
			}
			else // normal frame, normal interpolation
			{
#ifdef DEBUG_GENERATION
				logerror("processing frame: Normal\n");
				logerror("*** current Energy = %d\n",tms->current_energy);
				logerror("*** new (target) Energy = %d\n",tms->new_frame_energy);
#endif

				tms->target_energy = tms->new_frame_energy;
				tms->target_pitch = tms->new_frame_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->target_k[i] = tms->new_frame_k[i];
			}
		}
		else // Not a new frame, just interpolate the existing frame.
		{
#ifdef OVERRIDE_INTERPOLATION
			interp_period = OVERRIDE_INTERPOLATION;
#else
			interp_period = tms->sample_count / 25;
#endif
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
		if (OLD_FRAME_UNVOICED_FLAG == 1)
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
#ifdef NORMALMODE
          if (tms->pitch_count > 50)
              tms->excitation_data = tms->coeff->chirptable[50]<<CHIRPROM_LEFTSHIFT;
          else /*tms->pitch_count <= 50*/
              tms->excitation_data = tms->coeff->chirptable[tms->pitch_count]<<CHIRPROM_LEFTSHIFT;
#else
			 tms->excitation_data = tms->pitch_count - 64;
#endif
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

        //if (tms->digital_select == 0) /* if digital is NOT selected... */
		//  buffer[buf_count] &= 0xff00; /* mask out all but the 8 dac bits */

        /* Update all counts */

        size--;
        tms->sample_count = (tms->sample_count + 1) % 200;
#ifdef NORMALMODE
        if (tms->current_pitch != 0)
            tms->pitch_count = (tms->pitch_count + 1) % tms->current_pitch;
        else
            tms->pitch_count = 51; // blank spot in the chirp rom
#else
		    tms->pitch_count = (tms->pitch_count + 1) % 128;
#endif

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
    /* clipping & wrapping, just like the patent shows:
       first of all the result should be clamped to 14 bits, between -16384 and 16383
    */
	while (cliptemp > 16383) cliptemp -= 32768;
	while (cliptemp < -16384) cliptemp += 32768;
	/* the top 10 bits of this result are visible on the digital output IO pin.
       next, if the top 3 bits of the 14 bit result are all the same, the lowest of those 3 bits plus the next 7 bits are the signed analog output, otherwise the low bits are all forced to match the inverse of the topmost bit, i.e.:
       1x xxxx xxxx xxxx -> 0b10000000
       11 1bcd efgh xxxx -> 0b1bcdefgh
       00 0bcd efgh xxxx -> 0b0bcdefgh
       0x xxxx xxxx xxxx -> 0b01111111
       */
#ifdef DO_CLIP_AND_WRAP
	if (cliptemp > 2047) cliptemp = 2047;
	else if (cliptemp < -2048) cliptemp = -2048;
	/* at this point the analog output is tapped*/
	return cliptemp << 4;
#else
	return cliptemp << 1;
#endif
}


/**********************************************************************************************

     ti_matrix_multiply -- does the proper multiply and shift as the TI chips do.
     a is the k coefficient and is clamped to 10 bits (9 bits plus a sign)
     b is the running result and is clamped to 14 bits.
     output is 14 bits, but note the result LSB bit is always 1.

**********************************************************************************************/
static INT16 matrix_multiply(INT16 a, INT16 b)
{
	INT16 result;
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	result = ((a*b)>>9)|1;
#ifdef VERBOSE
	if (result>16383) fprintf(stderr,"matrix multiplier overflowed! a: %x, b: %x", a, b);
	if (result<-16384) fprintf(stderr,"matrix multiplier underflowed! a: %x, b: %x", a, b);
#endif
	return result;
}

/**********************************************************************************************

     lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
     excitation data, and specific values of all the current k constants,  and returns the
     resulting sample.

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
		tms->u[10] = matrix_multiply(tms->current_energy, (tms->excitation_data*64));  //Y(11)
		//tms->u[10] = matrix_multiply((tms->excitation_data*64), tms->current_energy); // wrong but sounds better
        tms->u[9] = tms->u[10] - matrix_multiply(tms->current_k[9], tms->x[9]);
        tms->u[8] = tms->u[9] - matrix_multiply(tms->current_k[8], tms->x[8]);
        tms->x[9] = tms->x[8] + matrix_multiply(tms->current_k[8], tms->u[8]);
        tms->u[7] = tms->u[8] - matrix_multiply(tms->current_k[7], tms->x[7]);
        tms->x[8] = tms->x[7] + matrix_multiply(tms->current_k[7], tms->u[7]);
        tms->u[6] = tms->u[7] - matrix_multiply(tms->current_k[6], tms->x[6]);
        tms->x[7] = tms->x[6] + matrix_multiply(tms->current_k[6], tms->u[6]);
        tms->u[5] = tms->u[6] - matrix_multiply(tms->current_k[5], tms->x[5]);
        tms->x[6] = tms->x[5] + matrix_multiply(tms->current_k[5], tms->u[5]);
        tms->u[4] = tms->u[5] - matrix_multiply(tms->current_k[4], tms->x[4]);
        tms->x[5] = tms->x[4] + matrix_multiply(tms->current_k[4], tms->u[4]);
        tms->u[3] = tms->u[4] - matrix_multiply(tms->current_k[3], tms->x[3]);
        tms->x[4] = tms->x[3] + matrix_multiply(tms->current_k[3], tms->u[3]);
        tms->u[2] = tms->u[3] - matrix_multiply(tms->current_k[2], tms->x[2]);
        tms->x[3] = tms->x[2] + matrix_multiply(tms->current_k[2], tms->u[2]);
        tms->u[1] = tms->u[2] - matrix_multiply(tms->current_k[1], tms->x[1]);
        tms->x[2] = tms->x[1] + matrix_multiply(tms->current_k[1], tms->u[1]);
        tms->u[0] = tms->u[1] - matrix_multiply(tms->current_k[0], tms->x[0]);
        tms->x[1] = tms->x[0] + matrix_multiply(tms->current_k[0], tms->u[0]);
        tms->x[0] = tms->u[0];
        tms->previous_energy = tms->current_energy;
        return tms->u[0];
}


/**********************************************************************************************

     process_command -- extract a byte from the FIFO and interpret it as a command

***********************************************************************************************/

static void process_command(tms5220_state *tms, unsigned char cmd)
{
#ifdef DEBUG_COMMAND_DUMP
		logerror("process_command called with parameter %02X\n",cmd);
#endif
		/* parse the command */
		switch (cmd & 0x70)
		{
		case 0x10 : /* read byte */
			if (tms->talk_status == 0) /* TALKST must be clear for RDBY */
			{
				if (tms->schedule_dummy_read)
				{
					tms->schedule_dummy_read = FALSE;
					if (tms->intf->read)
						(*tms->intf->read)(tms->device, 1);
				}
				if (tms->intf->read)
					tms->data_register = (*tms->intf->read)(tms->device, 8);	/* read one byte from speech ROM... */
				tms->RDB_flag = TRUE;
			}
			break;

		case 0x00: case 0x20: /* set rate (tms5220c only), otherwise NOP */
			if (tms->variant == SUBTYPE_TMS5220C)
			{
				tms->tms5220c_rate = cmd&0x0F;
			}
		break;

		case 0x30 : /* read and branch */
			if (tms->talk_status == 0) /* TALKST must be clear for RB */
			{
#ifdef VERBOSE
				logerror("read and branch command received\n");
#endif
				tms->RDB_flag = FALSE;
				if (tms->intf->read_and_branch)
					(*tms->intf->read_and_branch)(tms->device);
			}
			break;

		case 0x40 : /* load address */
			if (tms->talk_status == 0) /* TALKST must be clear for LA */
			{
				/* tms5220 data sheet says that if we load only one 4-bit nibble, it won't work.
                  This code does not care about this. */
				if (tms->intf->load_address)
					(*tms->intf->load_address)(tms->device, cmd & 0x0f);
				tms->schedule_dummy_read = TRUE;
			}
			break;

		case 0x50 : /* speak */
			if (tms->schedule_dummy_read)
			{
				tms->schedule_dummy_read = FALSE;
				if (tms->intf->read)
					(*tms->intf->read)(tms->device, 1);
			}
			tms->speaking_now = 1;
			tms->speak_external = 0;
			tms->talk_status = 1;  /* start immediately */
			break;

		case 0x60 : /* speak external */
			if (tms->talk_status == 0) /* TALKST must be clear for SPKEXT */
			{
				//SPKEXT going active activates SPKEE which clears the fifo
				tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;
				tms->speaking_now = tms->speak_external = 1;
				tms->RDB_flag = FALSE;
			}
			break;

		case 0x70 : /* reset */
			if (tms->schedule_dummy_read)
			{
				tms->schedule_dummy_read = FALSE;
				if (tms->intf->read)
					(*tms->intf->read)(tms->device, 1);
			}
			tms->device->reset();
			break;
    }

    /* update the buffer low state */
    update_status_and_ints(tms);
}

/******************************************************************************************

     parse_frame -- parse a new frame's worth of data; returns 0 if not enough bits in buffer

******************************************************************************************/

static void parse_frame(tms5220_state *tms)
{
	int bits, indx, i, rep_flag;
#ifdef DEBUG_FRAME_DUMP
	int ene;
#endif

	// todo: we actually don't care how many bits are left in the fifo here; the frame subpart will be processed normally, and any bits extracted 'past the end' of the fifo will be read as zeroes; the fifo being emptied will set the /BE latch which will halt speech exactly as if a stop frame had been encountered (instead of whatever partial frame was read); the same exact circuitry is used for both on the real chip, see us patent 4335277 sheet 16, gates 232a (decode stop frame) and 232b (decode /BE plus DDIS (decode disable) which is active during speak external).
	if (tms->speak_external)
	{
		/* count the total number of bits available */
		bits = ((tms->fifo_count)*8)-tms->fifo_bits_taken;
	}
	else
	{
		/* we're talking out of a vsm rom, hence we have an effectively infinite number of bits which are pullable. extract_bits will toggle M0 for us and grab what bits are needed */
		bits = 131072; // == arbitrary large number
	}
	/* if the chip is a tms5220C, and the rate mode is set to that each frame (0x04 bit set)
    has a 2 bit rate preceeding it, grab two bits here and store them as the rate; */
	if ((tms->variant == SUBTYPE_TMS5220C) && (tms->tms5220c_rate & 0x04))
	{
		bits -= 2;
		if (bits < 0) goto ranout;
		indx = extract_bits(tms, 2);
		tms->sample_count = reload_table[indx];
	}
	else // non-5220C and 5220C in fixed rate mode
	tms->sample_count = reload_table[tms->tms5220c_rate&0x3];

	/* attempt to extract the energy index */
	bits -= tms->coeff->energy_bits;
	if (bits < 0) goto ranout;
	indx = extract_bits(tms,tms->coeff->energy_bits);
	tms->new_frame_energy = tms->coeff->energytable[indx];
#ifdef DEBUG_FRAME_DUMP
	ene = indx;
#endif

	/* if the energy index is 0 or 15, we're done */

	if ((indx == 0) || (indx == 15))
	{
#ifdef DEBUG_FRAME_INFO
		logerror("  (4-bit energy=%d frame)\n",tms->new_frame_energy);
#endif
		return;
	}

	/* attempt to extract the repeat flag */
	bits -= 1;
	if (bits < 0) goto ranout;
	rep_flag = extract_bits(tms,1);

	/* attempt to extract the pitch */
	bits -= tms->coeff->pitch_bits;
	if (bits < 0) goto ranout;
	indx = extract_bits(tms,tms->coeff->pitch_bits);
	tms->new_frame_pitch = tms->coeff->pitchtable[indx];

	/* if this is a repeat frame, just copy the k's */
	if (rep_flag)
	{
	//actually, we do nothing because the k's were already loaded (on parsing the previous frame)

#ifdef DEBUG_FRAME_INFO
		logerror("  (10-bit energy=%d pitch=%d rep=%d frame)\n", tms->new_frame_energy, tms->new_frame_pitch, rep_flag);
#endif
		return;
	}


	/* if the pitch index was zero, we need 4 k's */
	if (indx == 0)
	{
		/* attempt to extract 4 K's */
		bits -= 18;
		if (bits < 0) goto ranout;
		for (i = 0; i < 4; i++)
			tms->new_frame_k[i] = tms->coeff->ktable[i][extract_bits(tms,tms->coeff->kbits[i])];

	/* and clear the rest of the new_frame_k[] */
		for (i = 4; i < tms->coeff->num_k; i++)
			tms->new_frame_k[i] = 0;
#ifdef DEBUG_FRAME_INFO
		logerror("  (29-bit energy=%d pitch=%d rep=%d 4K frame)\n", tms->new_frame_energy, tms->new_frame_pitch, rep_flag);
#endif
		return;
	}

	/* else we need 10 K's */
	bits -= 39;
	if (bits < 0) goto ranout;
#ifdef DEBUG_FRAME_DUMP
	logerror("FrameDump %02d ", ene);
	for (i = 0; i < tms->coeff->num_k; i++)
	{
		int x;
		x = extract_bits(tms, tms->coeff->kbits[i]);
		tms->new_frame_k[i] = tms->coeff->ktable[i][x];
		logerror("%02d ", x);
	}
	logerror("\n");
#else
	for (i = 0; i < tms->coeff->num_k; i++)
	{
		int x;
		x = extract_bits(tms, tms->coeff->kbits[i]);
		tms->new_frame_k[i] = tms->coeff->ktable[i][x];
	}
#endif
#ifdef DEBUG_FRAME_INFO
	logerror("  (50-bit energy=%d pitch=%d rep=%d 10K frame))\n", tms->new_frame_energy, tms->new_frame_pitch, rep_flag);
	if (tms->speak_external)
		logerror("Parsed a frame successfully in FIFO - %d bits remaining\n", bits);
	else
		logerror("Parsed a frame successfully in ROM\n");
#endif
	return;

	ranout:
#ifdef DEBUG_FRAME_ERRORS
    logerror("Ran out of bits on a parse!\n");
#endif
    /* this is an error condition; mark the buffer empty and turn off speaking */
    tms->buffer_empty = 1;
	tms->talk_status = tms->speak_external = tms->speaking_now = 0;
    tms->fifo_count = tms->fifo_head = tms->fifo_tail = 0;

	tms->RDB_flag = FALSE;

    /* generate an interrupt since TS went low */
    set_interrupt_state(tms, 1);
    return;

}

/**********************************************************************************************

     set_interrupt_state -- generate an interrupt

***********************************************************************************************/

static void set_interrupt_state(tms5220_state *tms, int state)
{
#ifdef DEBUG_PIN_READS
	logerror("irq pin set to state %d\n", state);
#endif
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

	tms->intf = device->baseconfig().static_config ? (const tms5220_interface *)device->baseconfig().static_config : &dummy;
	//tms->table = *device->region;

	tms->device = device;
	tms5220_set_variant(tms, TMS5220_IS_5220);
	tms->clock = device->clock;

	assert_always(tms != NULL, "Error creating TMS5220 chip");

	/* resolve irq line */
	devcb_resolve_write_line(&tms->irq_func, &tms->intf->irq_func, device);

	/* initialize a stream */
	tms->stream = stream_create(device, 0, 1, device->clock / 80, tms, tms5220_update);

	/*if (tms->table == NULL)
    {
        assert_always(tms->intf->M0_callback != NULL, "Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5220 to call for a new single bit\n  needed to generate the speech when in VSM mode\n  Aborting startup...\n");
        tms->M0_callback = tms->intf->M0_callback;
        tms->set_load_address = tms->intf->load_address;
    }
    else
    {
        tms->M0_callback = speech_rom_read_bit;
        tms->set_load_address = speech_rom_set_addr;
    }*/

	/* not during reset which is called frm within a write! */
	tms->io_ready = 1;
	tms->true_timing = 0;
	tms->rs_ws = 0x03; // rs and ws are assumed to be inactive on device startup

	register_for_save_states(tms);
}

static DEVICE_START( tms5220c )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms5220_set_variant(tms, TMS5220_IS_5220C);
}

static DEVICE_START( tmc0285 )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms5220_set_variant(tms, TMS5220_IS_TMC0285);
}


static DEVICE_START( tms5200 )
{
	tms5220_state *tms = get_safe_token(device);
	DEVICE_START_CALL( tms5220 );
	tms5220_set_variant(tms, TMS5220_IS_5200);
}


static DEVICE_RESET( tms5220 )
{
	tms5220_state *tms = get_safe_token(device);

	/* initialize the FIFO */
	/*memset(tms->fifo, 0, sizeof(tms->fifo));*/
	tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;

	/* initialize the chip state */
	/* Note that we do not actually clear IRQ on start-up : IRQ is even raised if tms->buffer_empty or tms->buffer_low are 0 */
	tms->speaking_now = tms->speak_external = tms->talk_status = tms->irq_pin = 0;
	set_interrupt_state(tms, 0);
	tms->buffer_empty = tms->buffer_low = 1;

	tms->RDB_flag = FALSE;

	/* initialize the energy/pitch/k states */
	tms->old_frame_energy = tms->new_frame_energy = tms->current_energy = tms->target_energy = 0;
	tms->old_frame_pitch = tms->new_frame_pitch = tms->current_pitch = tms->target_pitch = 0;
	memset(tms->old_frame_k, 0, sizeof(tms->old_frame_k));
	memset(tms->new_frame_k, 0, sizeof(tms->new_frame_k));
	memset(tms->current_k, 0, sizeof(tms->current_k));
	memset(tms->target_k, 0, sizeof(tms->target_k));

	/* initialize the sample generators */
	tms->interp_count = tms->tms5220c_rate = tms->pitch_count = 0;
	tms->sample_count = reload_table[tms->tms5220c_rate&0x3];
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
#ifdef DEBUG_IO_READY
			logerror("Serviced write: %02x\n", tms->write_latch);
			//fprintf(stderr, "Processed write data: %02X\n", tms->write_latch);
#endif
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
#ifdef DEBUG_RS_WS
	logerror("/RS written with data: %d\n", state);
#endif
	new_val = (tms->rs_ws & 0x01) | (state<<1);
	if (new_val != tms->rs_ws)
	{
		tms->rs_ws = new_val;
		if (new_val == 0)
		{
			if (tms->variant == SUBTYPE_TMS5220C)
				device->reset();
#ifdef DEBUG_RS_WS
			else
				/* illegal */
				logerror("tms5220_rs_w: illegal\n");
#endif
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
#ifdef DEBUG_RS_WS
			logerror("Schedule write ready\n");
#endif
			//tms->io_ready = 1;
			///* 100 nsec from data sheet */
			//timer_set(tms->device->machine, ATTOTIME_IN_NSEC(100), tms, 0, io_ready_cb);
			tms->io_ready = 0;
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
#ifdef DEBUG_RS_WS
	logerror("/WS written with data: %d\n", state);
#endif
	new_val = (tms->rs_ws & 0x02) | (state<<0);
	if (new_val != tms->rs_ws)
	{
		tms->rs_ws = new_val;
		if (new_val == 0)
		{
			if (tms->variant == SUBTYPE_TMS5220C)
				device->reset();
#ifdef DEBUG_RS_WS
			else
				/* illegal */
				logerror("tms5220_ws_w: illegal\n");
#endif
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
			///* high to low - schedule ready cycle*/
			//tms->io_ready = 1;
			//timer_set(tms->device->machine, ATTOTIME_IN_NSEC(100), tms, 0, io_ready_cb);
			tms->io_ready = 0;
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
#ifdef DEBUG_RS_WS
	logerror("tms5220_data_w: data %02x\n", data);
#endif
	if (!tms->true_timing)
	{
		/* bring up to date first */
	    stream_update(tms->stream);
	    tms5220_data_write(tms, data);
	}
	else
	{
		/* actually in a write ? */
#ifdef DEBUG_RS_WS
		if (!(tms->rs_ws == 0x02))
			logerror("tms5220_data_w: data written outside ws, status: %02x!\n", tms->rs_ws);
#endif
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
#ifdef DEBUG_RS_WS
		else
			logerror("tms5220_status_r: data read outside rs!\n");
#endif
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

double tms5220_time_to_ready(running_device *device)
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

void tms5220_set_frequency(running_device *device, int frequency)
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
