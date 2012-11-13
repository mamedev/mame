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
     Massive rewrite and reorganization by Lord Nightmare
     Additional IP, PC, subcycle timing rewrite by Lord Nightmare

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
    * Ever since the big rewrite, there are glitches on certain frame transitions
      for example in the word 'robots' during the eprom attract mode,
      I (LN) am not entirely sure why the real chip doesn't have these as well.
      Needs more real hardware testing/dumps for comparison.
    * Ever since the timing rewrite, the above problem is slightly worse. This
      time, however, it is probably a 'real' bug, which I (LN) am in the process
      of tracking down.
      i.e. the word 'congratulations' in victory when you get a high score.
    * Implement a ready callback for pc interfaces
    - this will be quite a challenge since for it to be really accurate
      the whole emulation has to run in sync (lots of timers) with the
      cpu cores.
    * If a command is still executing, /READY will be kept high until the command has
      finished if the next command is written.
    * tomcat has a 5220 which is not hooked up at all
    * Is the TS=0 forcing energy to 0 for next frame in the interpolator actually correct? I'm (LN) guessing no. The patent schematics state that TS=0 shuts off the output dac completely, though doesn't affect the I/O pin.

Pedantic detail from observation of real chip:
The 5200 and 5220 chips outputs the following coefficients over PROMOUT while
'idle' and not speaking, in this order:
e[0 or f] p[0] k1[0] k2[0] k3[0] k4[0] k5[f] k6[f] k7[f] k8[7] k9[7] k10[7]

Patent notes (important timing info for interpolation):
* TCycle ranges from 1 to 20, is clocked based on the clock input or RC clock
  to the chip / 4. This emulation core completely ignores TCycle, as it isn't
  very relevant.
    Every full TCycle count (i.e. overflow from 20 to 1), Subcycle is
    incremented.
* Subcycle ranges from 0 to 2, reload is 0 in SPKSLOW mode, 1 normally, and
  corresponds to whether an interpolation value is being calculated (0 or 1)
  or being written to ram (2). 0 and 1 correspond to 'A' cycles on the
  patent, while 2 corresponds to 'B' cycles.
    Every Subcycle full count (i.e. overflow from 2 to (0 or 1)), PC is
    incremented. (NOTE: if PC=12, overflow happens on the 1->2 transition,
    not 2->0; PC=12 has no B cycle.)
* PC ranges from 0 to 12, and corresponds to the parameter being interpolated
  or otherwise read from rom using PROMOUT.
  The order is:
  0 = Energy
  1 = Pitch
  2 = K1
  3 = K2
  ...
  11 = K10
  12 = nothing
    Every PC full count (i.e. overflow from 12 to 0), IP (aka interp_period)
    is incremented.
* IP (aka interp_period) ranges from 0 to 7, and corresponds with the amount
  of rightshift that the difference between current and target for a given
  parameter will have applied to it, before being added to the current
  parameter. Note that when interpolation is inhibited, only IP=0 will
  cause any change to the current values of the coefficients.
  The order is, after new frame parse (last ip was 0 before parse):
  1 = >>3 (/8)
  2 = >>3 (/8)
  3 = >>3 (/8)
  4 = >>2 (/4)
  5 = >>2 (/4)
  6 = >>1 (/2) (NOTE: the patent has an error regarding this value on one table implying it should be /4, but circuit simulation of parts of the patent shows that the /2 value is correct.)
  7 = >>1 (/2)
  0 = >>0 (/1, forcing current values to equal target values)
    Every IP full count, a new frame is parsed, but ONLY on the 0->*
    transition.
    NOTE: on TMS5220C ONLY, the datasheet IMPLIES the following:
    Upon new frame parse (end of IP=0), the IP is forced to a value depending
    on the TMS5220C-specific rate setting. For rate settings 0, 1, 2, 3, it
    will be forced to 1, 3, 5 or 7 respectively. On non-TMS5220 chips, it
    counts as expected (IP=1 follows IP=0) always.
    This means, the tms5220c with rates set to n counts IP as follows:
    (new frame parse is indicated with a #)
    Rate    IP Count
    00      7 0#1 2 3 4 5 6 7 0#1 2 3 4 5 6 7    <- non-tms5220c chips always follow this pattern
    01      7 0#3 4 5 6 7 0#3 4 5 6 7 0#3 4 5
    10      7 0#5 6 7 0#5 6 7 0#5 6 7 0#5 6 7
    11      7 0#7 0#7 0#7 0#7 0#7 0#7 0#7 0#7

Most of the following is based on figure 8c of 4,331,836, which is the
  TMS5100/TMC0280 patent, but the same information applies to the TMS52xx
  as well.

OLDP is a status flag which controls whether unvoiced or voiced excitation is
  being generated. It is latched from "P=0" at IP=7 PC=12 T=16.
  (This means that, during normal operation, between IP=7 PC=12 T16 and
  IP=0 PC=1 T=?, OLDP and P=0 are the same)
"P=0" is a status flag which is set if the index value for pitch for the new
  frame being parsed (which will become the new target frame) is zero.
  It is used for determining whether interpolation of the next frame is
  inhibited or not. It is updated at IP=0 PC=1 T=?. See next section.
OLDE is a status flag which is only used for determining whether
  interpolation is inhibited or not.
  It is latched from "E=0" at IP=7 PC=12 T=16.
  (This means that, during normal operation, between IP=7 PC=12 T16 and
  IP=0 PC=0 T=16, OLDE and E=0 are the same)
"E=0" is a status flag which is set if the index value for energy for the new
  frame being parsed (which will become the new target frame) is zero.
  It is used for determining whether interpolation of the next frame is
  inhibited or not. It is updated at IP=0 PC=0 T=16. See next section.

Interpolation is inhibited (i.e. interpolation at IP frames will not happen
  except for IP=0) under the following circumstances:
  "P=0" != "OLDP" ("P=0" = 1, and OLDP = 0; OR "P=0" = 0, and OLDP = 1)
    This means the new frame is unvoiced and the old one was voiced, or vice
    versa.
  "OLDE" = 1 and "E=0" = 0
    This means the new frame is not silent, and the old frame was silent.



****Documentation of chip commands:***
    x0x0xbcc : on 5200/5220: NOP (does nothing); on 5220C: Select frame length by cc, and b selects whether every frame is preceded by 2 bits to select the frame length (instead of using the value set by cc); the default (and after a reset command) is as if '0x00' was written, i.e. for frame length (200 samples) and 0 for whether the preceding 2 bits are enabled (off)

    x001xxxx: READ BYTE (RDBY) Sends eight read bit commands (M0 high M1 low) to VSM and reads the resulting bits serially into a temporary register, which becomes readable as the next byte read from the tms52xx once ready goes active. Note the bit order of the byte read from the TMS52xx is BACKWARDS as compared to the actual data order as in the rom on the VSM chips; the read byte command of the tms5100 reads the bits in the 'correct' order. This was IMHO a rather silly design decision of TI. (I (LN) asked Larry Brantingham about this but he wasn't involved with the TMS52xx chips, just the 5100); There's ASCII data in the TI 99/4 speech module VSMs which has the bit order reversed on purpose because of this!
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x011xxxx: READ AND BRANCH (RB) Sends a read and branch command (M0 high, M1 high) to force VSM to set its data pointer to whatever the data is at its current pointer location is)
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x100aaaa: LOAD ADDRESS (LA) Send a load address command (M0 low M1 high) to VSM with the 4 'a' bits; Note you need to send four or five of these in sequence to actually specify an address to the vsm.
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x101xxxx: SPEAK (SPK) Begins speaking, pulling spech data from the current address pointer location of the VSM modules.

    x110xxxx: SPEAK EXTERNAL (SPKEXT) Clears the FIFO using SPKEE line, then sets TALKD (TALKST remains zero) until 8 bytes have been written to the FIFO, at which point it begins speaking, pulling data from the 16 byte fifo.
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x111xxxx: RESET (RST) Resets the speech synthesis core immediately, and clears the FIFO.


    Other chip differences:
    The 5220 is 'noisier' when playing unvoiced frames than the 5220C is; I (LN) think the 5220C may use a different energy table (or use one value lower in the normal energy table) than the 5220 does, possibly only when playing unvoiced frames, but I can't prove this without a decap; the 5220C's PROMOUT pin (for dumping the lpc tables as played) is non-functional due to a changed design or a die bug (or may need special timing to know exactly when to read it, different than the 5200 and 5220 which are both easily readable).
    In addition, the NOP commands on the FIFO interface have been changed on the 5220C and data passed in the low bits has a meaning regarding frame length, see above.

    It is also possible but inconclusive that the chirp table was changed; The LPC tables between the 5220 and 5220C are MOSTLY the same of not completely so, but as mentioned above the energy table has some sort of difference.


***MAME Driver specific notes:***

    Looping has the tms5220 hooked up directly to the cpu. However currently the
    tms9900 cpu core does not support a ready line.

    Victory's initial audio selftest is pretty brutal to the FIFO: it sends a
    sequence of bytes to the FIFO and checks the status bits after each one; if
    even one bit is in the wrong state (i.e. speech starts one byte too early or
    late), the test fails!
    The sample in Victory 'Shields up!' after you activate shields, the 'up' part
    of the sample is missing the STOP frame at the end of it; this causes the
    speech core to run out of bits to parse from the FIFO, cutting the sample off
    by one frame. This appears to be an original game code bug.

Progress list for drivers using old vs new interface:
starwars: uses new interface (couriersud)
gauntlet: uses new interface (couriersud)
atarisy1: uses new interface (Lord Nightmare)
atarisy2: uses new interface (Lord Nightmare)
atarijsa: uses new interface (Lord Nightmare)
firefox: uses new interface (couriersud)
mhavoc: uses old interface, and is in the machine file instead of the driver.
monymony/jackrabt(zaccaria.c): uses new interface (couriersud)
victory(audio/exidy.c): uses new interface (couriersud)
looping: uses old interface
portraits: uses *NO* interface; the i/o cpu hasn't been hooked to anything!
dotron and midwayfb(mcr.c): uses old interface


As for which games used which chips:

TMS5200 AKA TMC0285: (1980 to 1983)
    Arcade: Zaccaria's 'money money' and 'jack rabbit'; Bally/Midway's
'Discs of Tron' (all environmental cabs and a few upright cabs; the code
exists on all versions for the speech though, and upright cabs can be
upgraded to add it by hacking on a 'Squawk & Talk' pinball speech board
(which is also TMS5200 based) with a few modded components)
    Pinball: All Bally/Midway machines which uses the 'Squawk & Talk' board.
    Home computer: TI 99/4 PHP1500 Speech module (along with two VSM
serial chips); Street Electronics Corp.'s Apple II 'Echo 2' Speech
synthesizer (early cards only)

TMS5220: (mostly on things made between 1981 and 1984-1985)
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

#include "emu.h"
#include "tms5220.h"

/* *****optional defines***** */

/* Hacky improvements which don't match patent: */
/* Interpolation shift logic:
 * One of the following two lines should be used, and the other commented
 * The second line is more accurate mathematically but not accurate to the patent
 */
#define INTERP_SHIFT >> tms->coeff->interp_coeff[tms->interp_period]
//define INTERP_SHIFT / (1<<tms->coeff->interp_coeff[tms->interp_period])

/* Excitation hacks */
/* The real chip uses an 8-bit excitation (shifted up to the top 8 bits) for
   both voiced and unvoiced speech.
   According to the patent, the voiced speech comes from a 51 entry rom
   And the unvoiced speech is ~0x3F(i.e. 0xC0) or 0x40 depending on an LFSR */
/* HACK: if defined, change the unvoiced excitation to use ~0x7F and 0x80
 * if not defined, acts as shown in patent */
#undef UNVOICED_HACK

/* Other hacks */
/* HACK?: if defined, outputs the low 4 bits of the lattice filter to the i/o
 * or clip logic, even though the real hardware doesn't do this...
 * ...actually the tms5220c might legitamately do this! */
#undef ALLOW_4_LSB

/* HACK: if defined, uses impossibly perfect 'straight line' interpolation */
#undef PERFECT_INTERPOLATION_HACK


/* *****configuration of chip connection stuff***** */
/* must be defined; if 0, output the waveform as if it was tapped on the speaker pin as usual, if 1, output the waveform as if it was tapped on the i/o pin (volume is much lower in the latter case) */
#define FORCE_DIGITAL 0

/* must be defined; if 1, normal speech (one A cycle, one B cycle per interpolation step); if 0; speak as if SPKSLOW was used (two A cycles, one B cycle per interpolation step) */
#define FORCE_SUBC_RELOAD 1


/* *****debugging defines***** */
#undef VERBOSE
// above is general, somewhat obsolete, catch all for debugs which don't fit elsewhere
#undef DEBUG_DUMP_INPUT_DATA
// above dumps the data input to the tms52xx to stdout, useful for making logged data dumps for real hardware tests
#undef DEBUG_FIFO
// above debugs fifo stuff: writes, reads and flag updates
#undef DEBUG_PARSE_FRAME_DUMP
// above dumps each frame to stderr: be sure to select one of the options below if you define it!
#undef DEBUG_PARSE_FRAME_DUMP_BIN
// dumps each speech frame as binary
#undef DEBUG_PARSE_FRAME_DUMP_HEX
// dumps each speech frame as hex
#undef DEBUG_FRAME_ERRORS
// above dumps info if a frame ran out of data
#undef DEBUG_COMMAND_DUMP
// above dumps all non-speech-data command writes
#undef DEBUG_PIN_READS
// above spams the errorlog with i/o ready messages whenever the ready or irq pin is read
#undef DEBUG_GENERATION
// above dumps debug information related to the sample generation loop, i.e. whether interpolation is inhibited or not, and what the current and target values for each frame are.
#undef DEBUG_GENERATION_VERBOSE
// above dumps MUCH MORE debug information related to the sample generation loop, namely the excitation, energy, pitch, k*, and output values for EVERY SINGLE SAMPLE during a frame.
#undef DEBUG_LATTICE
// above dumps the lattice filter state data each sample.
#undef DEBUG_CLIP
// above dumps info to stderr whenever the analog clip hardware is (or would be) clipping the signal.
#undef DEBUG_IO_READY
// above debugs the io ready callback
#undef DEBUG_RS_WS
// above debugs the tms5220_data_r and data_w access methods which actually respect rs and ws

#define MAX_SAMPLE_CHUNK	512

/* Variants */

#define TMS5220_IS_5220C	(4)
#define TMS5220_IS_5200		(5)
#define TMS5220_IS_5220		(6)

#define TMS5220_IS_TMC0285	TMS5220_IS_5200

static const UINT8 reload_table[4] = { 0, 2, 4, 6 }; //sample count reload for 5220c only; 5200 and 5220 always reload with 0; keep in mind this is loaded on IP=0 PC=12 subcycle=1 so it immediately will increment after one sample, effectively being 1,3,5,7 as in the comments above.

struct tms5220_state
{
	/* coefficient tables */
	int variant;				/* Variant of the 5xxx - see tms5110r.h */

	/* coefficient tables */
	const struct tms5100_coeffs *coeff;

	/* callbacks */
	devcb_resolved_write_line	irq_func;
	devcb_resolved_write_line	readyq_func;

	/* these contain data that describes the 128-bit data FIFO */
	UINT8 fifo[FIFO_SIZE];
	UINT8 fifo_head;
	UINT8 fifo_tail;
	UINT8 fifo_count;
	UINT8 fifo_bits_taken;


	/* these contain global status bits */
	UINT8 speaking_now;		/* True only if actual speech is being generated right now. Is set when a speak vsm command happens OR when speak external happens and buffer low becomes nontrue; Is cleared when speech halts after the last stop frame or the last frame after talk status is otherwise cleared.*/
	UINT8 speak_external;	/* If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO. */
	UINT8 talk_status;		/* If 1, TS status bit is 1, i.e. speak or speak external is in progress and we have not encountered a stop frame yet; talk_status differs from speaking_now in that speaking_now is set as soon as a speak or speak external command is started; talk_status does NOT go active until after 8 bytes are written to the fifo on a speak external command, otherwise the two are the same. TS is cleared by 3 things: 1. when a STOP command has just been processed as a new frame in the speech stream; 2. if the fifo runs out in speak external mode; 3. on power-up/during a reset command; When it gets cleared, speak_external is also cleared, an interrupt is generated, and speaking_now will be cleared when the next frame starts. */
	UINT8 buffer_low;		/* If 1, FIFO has less than 8 bytes in it */
	UINT8 buffer_empty;		/* If 1, FIFO is empty */
	UINT8 irq_pin;			/* state of the IRQ pin (output) */
	UINT8 ready_pin;		/* state of the READY pin (output) */

	/* these contain data describing the current and previous voice frames */
#define OLD_FRAME_SILENCE_FLAG tms->OLDE // 1 if E=0, 0 otherwise.
#define OLD_FRAME_UNVOICED_FLAG tms->OLDP // 1 if P=0 (unvoiced), 0 if voiced
	UINT8 OLDE;
	UINT8 OLDP;

#define NEW_FRAME_STOP_FLAG (tms->new_frame_energy_idx == 0xF) // 1 if this is a stop (Energy = 0xF) frame
#define NEW_FRAME_SILENCE_FLAG (tms->new_frame_energy_idx == 0) // ditto as above
#define NEW_FRAME_UNVOICED_FLAG (tms->new_frame_pitch_idx == 0) // ditto as above
	UINT8 new_frame_energy_idx;
	UINT8 new_frame_pitch_idx;
	UINT8 new_frame_k_idx[10];


	/* these are all used to contain the current state of the sound generation */
#ifndef PERFECT_INTERPOLATION_HACK
	INT16 current_energy;
	INT16 current_pitch;
	INT16 current_k[10];

	INT16 target_energy;
	INT16 target_pitch;
	INT16 target_k[10];
#else
	UINT8 old_frame_energy_idx;
	UINT8 old_frame_pitch_idx;
	UINT8 old_frame_k_idx[10];

	INT32 current_energy;
	INT32 current_pitch;
	INT32 current_k[10];

	INT32 target_energy;
	INT32 target_pitch;
	INT32 target_k[10];
#endif

	UINT16 previous_energy;	/* needed for lattice filter to match patent */

	UINT8 subcycle;			/* contains the current subcycle for a given PC: 0 is A' (only used on SPKSLOW mode on 51xx), 1 is A, 2 is B */
	UINT8 subc_reload;		/* contains 1 for normal speech, 0 when SPKSLOW is active */
	UINT8 PC;				/* current parameter counter (what param is being interpolated), ranges from 0 to 12 */
	/* TODO/NOTE: the current interpolation period, counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,4,2,1 */
	UINT8 interp_period;		/* the current interpolation period */
	UINT8 inhibit;			/* If 1, interpolation is inhibited until the DIV1 period */
	UINT8 tms5220c_rate;	/* only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command */
	UINT16 pitch_count;		/* pitch counter; provides chirp rom address */

	INT32 u[11];
	INT32 x[10];

	UINT16 RNG;				/* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */
	INT16 excitation_data;

	/* R Nabet : These have been added to emulate speech Roms */
	UINT8 schedule_dummy_read;			/* set after each load address, so that next read operation is preceded by a dummy read */
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
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	UINT8 digital_select;
	device_t *device;

	const tms5220_interface *intf;
	sound_stream *stream;
	int clock;
};


// Pull in the ROM tables
#include "tms5110r.c"


INLINE tms5220_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS5220 ||
		   device->type() == TMS5220C ||
		   device->type() == TMC0285 ||
		   device->type() == TMS5200);
	return (tms5220_state *)downcast<tms5220_device *>(device)->token();
}

/* Static function prototypes */
static void process_command(tms5220_state *tms, unsigned char data);
static void parse_frame(tms5220_state *tms);
static void update_status_and_ints(tms5220_state *tms);
static void set_interrupt_state(tms5220_state *tms, int state);
static INT32 lattice_filter(tms5220_state *tms);
static INT16 clip_analog(INT16 clip);
static void update_ready_state(tms5220_state *tms);
static STREAM_UPDATE( tms5220_update );

static void tms5220_set_variant(tms5220_state *tms, int variant)
{
	switch (variant)
	{
		case TMS5220_IS_5220C:
			tms->coeff = &tms5220c_coeff;
			break;
		case TMS5220_IS_5200:
			tms->coeff = &tms5200_coeff;
			//tms->coeff = &pat4335277_coeff;
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
	tms->device->save_item(NAME(tms->fifo));
	tms->device->save_item(NAME(tms->fifo_head));
	tms->device->save_item(NAME(tms->fifo_tail));
	tms->device->save_item(NAME(tms->fifo_count));
	tms->device->save_item(NAME(tms->fifo_bits_taken));

	tms->device->save_item(NAME(tms->speaking_now));
	tms->device->save_item(NAME(tms->speak_external));
	tms->device->save_item(NAME(tms->talk_status));
	tms->device->save_item(NAME(tms->buffer_low));
	tms->device->save_item(NAME(tms->buffer_empty));
	tms->device->save_item(NAME(tms->irq_pin));
	tms->device->save_item(NAME(tms->ready_pin));

	tms->device->save_item(NAME(tms->OLDE));
	tms->device->save_item(NAME(tms->OLDP));

	tms->device->save_item(NAME(tms->new_frame_energy_idx));
	tms->device->save_item(NAME(tms->new_frame_pitch_idx));
	tms->device->save_item(NAME(tms->new_frame_k_idx));
#ifdef PERFECT_INTERPOLATION_HACK
	tms->device->save_item(NAME(tms->old_frame_energy_idx));
	tms->device->save_item(NAME(tms->old_frame_pitch_idx));
	tms->device->save_item(NAME(tms->old_frame_k_idx));
#endif
	tms->device->save_item(NAME(tms->current_energy));
	tms->device->save_item(NAME(tms->current_pitch));
	tms->device->save_item(NAME(tms->current_k));

	tms->device->save_item(NAME(tms->target_energy));
	tms->device->save_item(NAME(tms->target_pitch));
	tms->device->save_item(NAME(tms->target_k));

	tms->device->save_item(NAME(tms->previous_energy));

	tms->device->save_item(NAME(tms->subcycle));
	tms->device->save_item(NAME(tms->subc_reload));
	tms->device->save_item(NAME(tms->PC));
	tms->device->save_item(NAME(tms->interp_period));
	tms->device->save_item(NAME(tms->inhibit));
	tms->device->save_item(NAME(tms->tms5220c_rate));
	tms->device->save_item(NAME(tms->pitch_count));

	tms->device->save_item(NAME(tms->u));
	tms->device->save_item(NAME(tms->x));

	tms->device->save_item(NAME(tms->RNG));
	tms->device->save_item(NAME(tms->excitation_data));

	tms->device->save_item(NAME(tms->schedule_dummy_read));
	tms->device->save_item(NAME(tms->data_register));
	tms->device->save_item(NAME(tms->RDB_flag));
	tms->device->save_item(NAME(tms->digital_select));

	tms->device->save_item(NAME(tms->io_ready));
}


/**********************************************************************************************

	  printbits helper function: takes a long int input and prints the resulting bits to stderr

***********************************************************************************************/

#ifdef DEBUG_PARSE_FRAME_DUMP_BIN
static void printbits(long data, int num)
{
	int i;
	for (i=(num-1); i>=0; i--)
		fprintf(stderr,"%0ld", (data>>i)&1);
}
#endif
#ifdef DEBUG_PARSE_FRAME_DUMP_HEX
static void printbits(long data, int num)
{
	switch((num-1)&0xFC)
	{
		case 0:
			fprintf(stderr,"%0lx", data);
			break;
		case 4:
			fprintf(stderr,"%02lx", data);
			break;
		case 8:
			fprintf(stderr,"%03lx", data);
			break;
		case 12:
			fprintf(stderr,"%04lx", data);
			break;
		default:
			fprintf(stderr,"%04lx", data);
			break;
	}
}
#endif

/**********************************************************************************************

	 tms5220_data_write -- handle a write to the TMS5220

***********************************************************************************************/

static void tms5220_data_write(tms5220_state *tms, int data)
{
#ifdef DEBUG_DUMP_INPUT_DATA
	fprintf(stdout, "%c",data);
#endif
	if (tms->speak_external) // If we're in speak external mode
	{
		// add this byte to the FIFO
		if (tms->fifo_count < FIFO_SIZE)
		{
			tms->fifo[tms->fifo_tail] = data;
			tms->fifo_tail = (tms->fifo_tail + 1) % FIFO_SIZE;
			tms->fifo_count++;
#ifdef DEBUG_FIFO
			logerror("data_write: Added byte to FIFO (current count=%2d)\n", tms->fifo_count);
#endif
			update_status_and_ints(tms);
			if ((tms->talk_status == 0) && (tms->buffer_low == 0)) // we just unset buffer low with that last write, and talk status *was* zero...
			{
			int i;
#ifdef DEBUG_FIFO
			logerror("data_write triggered talk status to go active!\n");
#endif
				// ...then we now have enough bytes to start talking; clear out the new frame parameters (it will become old frame just before the first call to parse_frame() )
				// TODO: the 3 lines below (and others) are needed for victory to not fail its selftest due to a sample ending too late, may require additional investigation
				tms->subcycle = tms->subc_reload;
				tms->PC = 0;
				tms->interp_period = reload_table[tms->tms5220c_rate&0x3]; // is this correct? should this be always 7 instead, so that the new frame is loaded quickly?
				tms->new_frame_energy_idx = 0;
				tms->new_frame_pitch_idx = 0;
				for (i = 0; i < 4; i++)
					tms->new_frame_k_idx[i] = 0;
				for (i = 4; i < 7; i++)
					tms->new_frame_k_idx[i] = 0xF;
				for (i = 7; i < tms->coeff->num_k; i++)
					tms->new_frame_k_idx[i] = 0x7;
				tms->talk_status = tms->speaking_now = 1;
			}
		}
		else
		{
#ifdef DEBUG_FIFO
			logerror("data_write: Ran out of room in the tms52xx FIFO! this should never happen!\n");
			// at this point, /READY should remain HIGH/inactive until the fifo has at least one byte open in it.
#endif
		}


	}
	else //(! tms->speak_external)
		// R Nabet : we parse commands at once.  It is necessary for such commands as read.
		process_command(tms,data);
}

/**********************************************************************************************

	 update_status_and_ints -- check to see if the various flags should be on or off
	 Description of flags, and their position in the status register:
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
				eighth data byte of the stack.
		bit D2(bit 5) = BE - Buffer Empty is active (high) when the FIFO buffer has run out of data
				while executing a Speak External command. Buffer Empty is set when the last bit
				of the "Last-In" byte is shifted out to the Synthesis Section. This causes
				Talk Status to be cleared. Speech is terminated at some abnormal point and the
				Speak External command execution is terminated.

***********************************************************************************************/

static void update_status_and_ints(tms5220_state *tms)
{
	/* update flags and set ints if needed */

	update_ready_state(tms);

	/* BL is set if neither byte 9 nor 8 of the fifo are in use; this
	translates to having fifo_count (which ranges from 0 bytes in use to 16
	bytes used) being less than or equal to 8. Victory/Victorba depends on this. */
	if (tms->fifo_count <= 8)
	{
		// generate an interrupt if necessary; if /BL was inactive and is now active, set int.
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
		// generate an interrupt if necessary; if /BE was inactive and is now active, set int.
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
		// generate an interrupt: /TS was active, and is now inactive.
		if (tms->talk_status == 1)
		{
			tms->talk_status = tms->speak_external = 0;
			set_interrupt_state(tms, 1);
		}
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
		// extract from FIFO
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
		// extract from VSM (speech ROM)
		if (tms->intf->read)
			val = (* tms->intf->read)(tms->device, count);
	}

	return val;
}

/**********************************************************************************************

	 tms5220_status_read -- read status or data from the TMS5220

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
	 NOTE: this function is deprecated and is known to be VERY inaccurate.
	 Use at your own peril!

***********************************************************************************************/

static int tms5220_cycles_to_ready(tms5220_state *tms)
{
	int answer;


	if (tms5220_ready_read(tms))
		answer = 0;
	else
	{
		int val;
		int samples_per_frame = tms->subc_reload?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
		int current_sample = ((tms->PC*(3-tms->subc_reload))+((tms->subc_reload?38:25)*tms->interp_period));
		answer = samples_per_frame-current_sample+8;

		// total number of bits available in current byte is (8 - tms->fifo_bits_taken)
		// if more than 4 are available, we need to check the energy
		if (tms->fifo_bits_taken < 4)
		{
			// read energy
			val = (tms->fifo[tms->fifo_head] >> tms->fifo_bits_taken) & 0xf;
			if (val == 0)
				/* 0 -> silence frame: we will only read 4 bits, and we will
				 * therefore need to read another frame before the FIFO is not
				 * full any more */
				answer += tms->subc_reload?200:304;
			/* 15 -> stop frame, we will only read 4 bits, but the FIFO will
			 * we cleared; otherwise, we need to parse the repeat flag (1 bit)
			 * and the pitch (6 bits), so everything will be OK. */
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
	int i, bitout, zpar;
	INT32 this_sample;

	/* the following gotos are probably safe to remove */
	/* if we're empty and still not speaking, fill with nothingness */
	if (!tms->speaking_now)
		goto empty;

	/* if speak external is set, but talk status is not (yet) set,
	wait for buffer low to clear */
	if (!tms->talk_status && tms->speak_external && tms->buffer_low)
		goto empty;

	/* loop until the buffer is full or we've stopped speaking */
	while ((size > 0) && tms->speaking_now)
	{
		/* if it is the appropriate time to update the old energy/pitch idxes,
		 * i.e. when IP=7, PC=12, T=17, subcycle=2, do so. Since IP=7 PC=12 T=17
		 * is JUST BEFORE the transition to IP=0 PC=0 T=0 sybcycle=(0 or 1),
		 * which happens 4 T-cycles later), we change on the latter.*/
		if ((tms->interp_period == 0) && (tms->PC == 0) && (tms->subcycle < 2))
		{
			tms->OLDE = (tms->new_frame_energy_idx == 0);
			tms->OLDP = (tms->new_frame_pitch_idx == 0);
		}

		/* if we're ready for a new frame to be applied, i.e. when IP=0, PC=12, Sub=1
		 * (In reality, the frame was really loaded incrementally during the entire IP=0
		 * PC=x time period, but it doesn't affect anything until IP=0 PC=12 happens)
		 */
		if ((tms->interp_period == 0) && (tms->PC == 12) && (tms->subcycle == 1))
		{
			// HACK for regression testing, be sure to comment out before release!
			//tms->RNG = 0x1234;
			// end HACK

			/* appropriately override the interp count if needed; this will be incremented after the frame parse! */
			tms->interp_period = reload_table[tms->tms5220c_rate&0x3];

#ifdef PERFECT_INTERPOLATION_HACK
			/* remember previous frame energy, pitch, and coefficients */
			tms->old_frame_energy_idx = tms->new_frame_energy_idx;
			tms->old_frame_pitch_idx = tms->new_frame_pitch_idx;
			for (i = 0; i < tms->coeff->num_k; i++)
				tms->old_frame_k_idx[i] = tms->new_frame_k_idx[i];
#endif

			/* if the talk status was clear last frame, halt speech now. */
			if (tms->talk_status == 0)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"tms5220_process: processing frame: talk status = 0 caused by stop frame or buffer empty, halting speech.\n");
#endif
				tms->speaking_now = 0; // finally halt speech
				goto empty;
			}


			/* Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[] */
			parse_frame(tms);
#ifdef DEBUG_PARSE_FRAME_DUMP
			fprintf(stderr,"\n");
#endif

			/* if the new frame is a stop frame, set an interrupt and set talk status to 0 */
			if (NEW_FRAME_STOP_FLAG == 1)
				{
					tms->talk_status = tms->speak_external = 0;
					set_interrupt_state(tms, 1);
					update_status_and_ints(tms);
				}

			/* in all cases where interpolation would be inhibited, set the inhibit flag; otherwise clear it.
			   Interpolation inhibit cases:
			 * Old frame was voiced, new is unvoiced
			 * Old frame was silence/zero energy, new has nonzero energy
			 * Old frame was unvoiced, new is voiced
			 */
			if ( ((OLD_FRAME_UNVOICED_FLAG == 0) && (NEW_FRAME_UNVOICED_FLAG == 1))
				|| ((OLD_FRAME_UNVOICED_FLAG == 1) && (NEW_FRAME_UNVOICED_FLAG == 0))
				|| ((OLD_FRAME_SILENCE_FLAG == 1) && (NEW_FRAME_SILENCE_FLAG == 0)) )
				tms->inhibit = 1;
			else // normal frame, normal interpolation
				tms->inhibit = 0;

			/* load new frame targets from tables, using parsed indices */
			tms->target_energy = tms->coeff->energytable[tms->new_frame_energy_idx];
			tms->target_pitch = tms->coeff->pitchtable[tms->new_frame_pitch_idx];
			zpar = NEW_FRAME_UNVOICED_FLAG; // find out if parameters k5-k10 should be zeroed
			for (i = 0; i < 4; i++)
				tms->target_k[i] = tms->coeff->ktable[i][tms->new_frame_k_idx[i]];
			for (i = 4; i < tms->coeff->num_k; i++)
				tms->target_k[i] = (tms->coeff->ktable[i][tms->new_frame_k_idx[i]] * (1-zpar));

#ifdef DEBUG_GENERATION
			/* Debug info for current parsed frame */
			fprintf(stderr, "OLDE: %d; OLDP: %d; ", tms->OLDE, tms->OLDP);
			fprintf(stderr,"Processing frame: ");
			if (tms->inhibit == 0)
				fprintf(stderr, "Normal Frame\n");
			else
				fprintf(stderr,"Interpolation Inhibited\n");
			fprintf(stderr,"*** current Energy, Pitch and Ks =      %04d,   %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",tms->current_energy, tms->current_pitch, tms->current_k[0], tms->current_k[1], tms->current_k[2], tms->current_k[3], tms->current_k[4], tms->current_k[5], tms->current_k[6], tms->current_k[7], tms->current_k[8], tms->current_k[9]);
			fprintf(stderr,"*** target Energy(idx), Pitch, and Ks = %04d(%x),%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",tms->target_energy, tms->new_frame_energy_idx, tms->target_pitch, tms->target_k[0], tms->target_k[1], tms->target_k[2], tms->target_k[3], tms->target_k[4], tms->target_k[5], tms->target_k[6], tms->target_k[7], tms->target_k[8], tms->target_k[9]);
#endif

			/* if TS is now 0, ramp the energy down to 0. Is this really correct to hardware? */
			if (tms->talk_status == 0)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"Talk status is 0, forcing target energy to 0\n");
#endif
				tms->target_energy = 0;
			}
		}
		else // Not a new frame, just interpolate the existing frame.
		{
			int inhibit_state = ((tms->inhibit==1)&&(tms->interp_period != 0)); // disable inhibit when reaching the last interp period, but don't overwrite the tms->inhibit value
#ifdef PERFECT_INTERPOLATION_HACK
			int samples_per_frame = tms->subc_reload?175:266; // either (13 A cycles + 12 B cycles) * 7 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 7 interps for SPKSLOW
			//int samples_per_frame = tms->subc_reload?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
			int current_sample = (tms->subcycle - tms->subc_reload)+(tms->PC*(3-tms->subc_reload))+((tms->subc_reload?25:38)*((tms->interp_period-1)&7));

			zpar = OLD_FRAME_UNVOICED_FLAG;
			//fprintf(stderr, "CS: %03d", current_sample);
			// reset the current energy, pitch, etc to what it was at frame start
			tms->current_energy = tms->coeff->energytable[tms->old_frame_energy_idx];
			tms->current_pitch = tms->coeff->pitchtable[tms->old_frame_pitch_idx];
			for (i = 0; i < 4; i++)
				tms->current_k[i] = tms->coeff->ktable[i][tms->old_frame_k_idx[i]];
			for (i = 4; i < tms->coeff->num_k; i++)
				tms->current_k[i] = (tms->coeff->ktable[i][tms->old_frame_k_idx[i]] * (1-zpar));
			// now adjust each value to be exactly correct for each of the samples per frame
			if (tms->interp_period != 0) // if we're still interpolating...
			{
				tms->current_energy += (((tms->target_energy - tms->current_energy)*(1-inhibit_state))*current_sample)/samples_per_frame;
				tms->current_pitch += (((tms->target_pitch - tms->current_pitch)*(1-inhibit_state))*current_sample)/samples_per_frame;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->current_k[i] += (((tms->target_k[i] - tms->current_k[i])*(1-inhibit_state))*current_sample)/samples_per_frame;
			}
			else // we're done, play this frame for 1/8 frame.
			{
				tms->current_energy = tms->target_energy;
				tms->current_pitch = tms->target_pitch;
				for (i = 0; i < tms->coeff->num_k; i++)
					tms->current_k[i] = tms->target_k[i];
			}
#else
			//Updates to parameters only happen on subcycle '2' (B cycle) of PCs.
			if (tms->subcycle == 2)
			{
				switch(tms->PC)
				{
					case 0: /* PC = 0, B cycle, write updated energy */
					tms->current_energy += (((tms->target_energy - tms->current_energy)*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 1: /* PC = 1, B cycle, write updated pitch */
					tms->current_pitch += (((tms->target_pitch - tms->current_pitch)*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
					/* PC = 2 through 11, B cycle, write updated K1 through K10 */
					tms->current_k[tms->PC-2] += (((tms->target_k[tms->PC-2] - tms->current_k[tms->PC-2])*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 12: /* PC = 12, do nothing */
					break;
				}
			}
#endif
		}

		// calculate the output
		if (OLD_FRAME_UNVOICED_FLAG == 1)
		{
			// generate unvoiced samples here
#ifndef UNVOICED_HACK
			if (tms->RNG & 1)
				tms->excitation_data = ~0x3F; /* according to the patent it is (either + or -) half of the maximum value in the chirp table, so either 01000000(0x40) or 11000000(0xC0)*/
			else
				tms->excitation_data = 0x40;
#else // hack to tweak unvoiced strength, doesn't match patent
			if (tms->RNG & 1)
				tms->excitation_data = 0;
			else
				tms->excitation_data = 0x40;
#endif
		}
		else /* (OLD_FRAME_UNVOICED_FLAG == 0) */
		{
			// generate voiced samples here
			/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			 * function has a chirp/peak and then a long chain of zeroes.
			 * The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
			 * and if the address reaches that point the ADDRESS incrementer is
			 * disabled, forcing all samples beyond 51d to be == 51d
			 * (address 51d holds zeroes, which may or may not be inverted to -1)
			 */
			if (tms->pitch_count >= 51)
				tms->excitation_data = tms->coeff->chirptable[51];
			else /*tms->pitch_count < 51*/
				tms->excitation_data = tms->coeff->chirptable[tms->pitch_count];
		}

		// Update LFSR *20* times every sample (once per T cycle), like patent shows
	for (i=0; i<20; i++)
	{
		bitout = ((tms->RNG >> 12) & 1) ^
				((tms->RNG >>  3) & 1) ^
				((tms->RNG >>  2) & 1) ^
				((tms->RNG >>  0) & 1);
		tms->RNG <<= 1;
		tms->RNG |= bitout;
	}
		this_sample = lattice_filter(tms); /* execute lattice filter */
#ifdef DEBUG_GENERATION_VERBOSE
		//fprintf(stderr,"C:%01d; ",tms->subcycle);
		fprintf(stderr,"IP:%01d PC:%02d X:%04d E:%03d P:%03d Pc:%03d ",tms->interp_period, tms->PC, tms->excitation_data, tms->current_energy, tms->current_pitch, tms->pitch_count);
		//fprintf(stderr,"X:%04d E:%03d P:%03d Pc:%03d ", tms->excitation_data, tms->current_energy, tms->current_pitch, tms->pitch_count);
		for (i=0; i<10; i++)
			fprintf(stderr,"K%d:%04d ", i+1, tms->current_k[i]);
		fprintf(stderr,"Out:%06d", this_sample);
		fprintf(stderr,"\n");
#endif
		/* next, force result to 14 bits (since its possible that the addition at the final (k1) stage of the lattice overflowed) */
		while (this_sample > 16383) this_sample -= 32768;
		while (this_sample < -16384) this_sample += 32768;
		if (tms->digital_select == 0) // analog SPK pin output is only 8 bits, with clipping
			buffer[buf_count] = clip_analog(this_sample);
		else // digital I/O pin output is 12 bits
		{
#ifdef ALLOW_4_LSB
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn nnnn
			// N taps:                       ^                 = 0x2000;
			// output: ssss ssss ssss ssss snnn nnnn nnnn nnnN
			buffer[buf_count] = (this_sample<<1)|((this_sample&0x2000)>>13);
#else
			this_sample &= ~0xF;
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn 0000
			// N taps:                       ^^ ^^^            = 0x3E00;
			// output: ssss ssss ssss ssss snnn nnnn nnnN NNNN
			buffer[buf_count] = (this_sample<<1)|((this_sample&0x3E00)>>9);
#endif
		}
		// Update all counts

		tms->subcycle++;
		if ((tms->subcycle == 2) && (tms->PC == 12))
		{
			tms->subcycle = tms->subc_reload;
			tms->PC = 0;
			tms->interp_period++;
			tms->interp_period&=0x7;
		}
		else if (tms->subcycle == 3)
		{
			tms->subcycle = tms->subc_reload;
			tms->PC++;
		}
		/* Circuit 412 in the patent ensures that when INHIBIT is true,
		 * during the period from IP=7 PC=12 T12, to IP=0 PC=12 T12, the pitch
		 * count is forced to 0; since the initial stop happens right before
		 * the switch to IP=0 PC=0 and this code is located after the switch would
		 * happen, we check for ip=0 inhibit=1, which covers that whole range.
		 * The purpose of Circuit 412 is to prevent a spurious click caused by
		 * the voiced source being fed to the filter before all the values have
		 * been updated during ip=0 when interpolation was inhibited.
		 */
		tms->pitch_count++;
		if (tms->pitch_count >= tms->current_pitch) tms->pitch_count = 0;
		if ((tms->interp_period == 0)&&(tms->inhibit==1)) tms->pitch_count = 0;
		tms->pitch_count &= 0x1FF;
		buf_count++;
		size--;
	}

empty:

	while (size > 0)
	{
		tms->subcycle++;
		if ((tms->subcycle == 2) && (tms->PC == 12))
		{
			tms->subcycle = tms->subc_reload;
			tms->PC = 0;
			tms->interp_period++;
			tms->interp_period&=0x7;
		}
		else if (tms->subcycle == 3)
		{
			tms->subcycle = tms->subc_reload;
			tms->PC++;
		}
		buffer[buf_count] = -1;	/* should be just -1; actual chip outputs -1 every idle sample; (cf note in data sheet, p 10, table 4) */
		buf_count++;
		size--;
	}
}

/**********************************************************************************************

	 clip_analog -- clips the 14 bit return value from the lattice filter to its final 10 bit value (-512 to 511), and upshifts/range extends this to 16 bits

***********************************************************************************************/

static INT16 clip_analog(INT16 cliptemp)
{
	/* clipping, just like the patent shows:
	 * the top 10 bits of this result are visible on the digital output IO pin.
	 * next, if the top 3 bits of the 14 bit result are all the same, the lowest of those 3 bits plus the next 7 bits are the signed analog output, otherwise the low bits are all forced to match the inverse of the topmost bit, i.e.:
	 * 1x xxxx xxxx xxxx -> 0b10000000
	 * 11 1bcd efgh xxxx -> 0b1bcdefgh
	 * 00 0bcd efgh xxxx -> 0b0bcdefgh
	 * 0x xxxx xxxx xxxx -> 0b01111111
	 */
#ifdef DEBUG_CLIP
	if ((cliptemp > 2047) || (cliptemp < -2048)) fprintf(stderr,"clipping cliptemp to range; was %d\n", cliptemp);
#endif
	if (cliptemp > 2047) cliptemp = 2047;
	else if (cliptemp < -2048) cliptemp = -2048;
	/* at this point the analog output is tapped */
#ifdef ALLOW_4_LSB
	// input:  ssss snnn nnnn nnnn
	// N taps:       ^^^ ^         = 0x0780
	// output: snnn nnnn nnnn NNNN
	return (cliptemp << 4)|((cliptemp&0x780)>>7); // upshift and range adjust
#else
	cliptemp &= ~0xF;
	// input:  ssss snnn nnnn 0000
	// N taps:       ^^^ ^^^^      = 0x07F0
	// P taps:       ^             = 0x0400
	// output: snnn nnnn NNNN NNNP
	return (cliptemp << 4)|((cliptemp&0x7F0)>>3)|((cliptemp&0x400)>>10); // upshift and range adjust
#endif
}


/**********************************************************************************************

	 matrix_multiply -- does the proper multiply and shift
	 a is the k coefficient and is clamped to 10 bits (9 bits plus a sign)
	 b is the running result and is clamped to 14 bits.
	 output is 14 bits, but note the result LSB bit is always 1.
	 Because the low 4 bits of the result are trimmed off before
	 output, this makes almost no difference in the computation.

**********************************************************************************************/
static INT32 matrix_multiply(INT32 a, INT32 b)
{
	INT32 result;
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	result = ((a*b)>>9)|1;//&(~1);
#ifdef VERBOSE
	if (result>16383) fprintf(stderr,"matrix multiplier overflowed! a: %x, b: %x, result: %x", a, b, result);
	if (result<-16384) fprintf(stderr,"matrix multiplier underflowed! a: %x, b: %x, result: %x", a, b, result);
#endif
	return result;
}

/**********************************************************************************************

	 lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
	 excitation data, and specific values of all the current k constants,  and returns the
	 resulting sample.

***********************************************************************************************/

static INT32 lattice_filter(tms5220_state *tms)
{
	// Lattice filter here
	// Aug/05/07: redone as unrolled loop, for clarity - LN
	/* Originally Copied verbatim from table I in US patent 4,209,804, now updated to be in same order as the actual chip does it, not that it matters.
	  notation equivalencies from table:
	  Yn(i) == tms->u[n-1]
	  Kn = tms->current_k[n-1]
	  bn = tms->x[n-1]
	 */
		tms->u[10] = matrix_multiply(tms->previous_energy, (tms->excitation_data<<6));  //Y(11)
		tms->u[9] = tms->u[10] - matrix_multiply(tms->current_k[9], tms->x[9]);
		tms->u[8] = tms->u[9] - matrix_multiply(tms->current_k[8], tms->x[8]);
		tms->u[7] = tms->u[8] - matrix_multiply(tms->current_k[7], tms->x[7]);
		tms->u[6] = tms->u[7] - matrix_multiply(tms->current_k[6], tms->x[6]);
		tms->u[5] = tms->u[6] - matrix_multiply(tms->current_k[5], tms->x[5]);
		tms->u[4] = tms->u[5] - matrix_multiply(tms->current_k[4], tms->x[4]);
		tms->u[3] = tms->u[4] - matrix_multiply(tms->current_k[3], tms->x[3]);
		tms->u[2] = tms->u[3] - matrix_multiply(tms->current_k[2], tms->x[2]);
		tms->u[1] = tms->u[2] - matrix_multiply(tms->current_k[1], tms->x[1]);
		tms->u[0] = tms->u[1] - matrix_multiply(tms->current_k[0], tms->x[0]);
		tms->x[9] = tms->x[8] + matrix_multiply(tms->current_k[8], tms->u[8]);
		tms->x[8] = tms->x[7] + matrix_multiply(tms->current_k[7], tms->u[7]);
		tms->x[7] = tms->x[6] + matrix_multiply(tms->current_k[6], tms->u[6]);
		tms->x[6] = tms->x[5] + matrix_multiply(tms->current_k[5], tms->u[5]);
		tms->x[5] = tms->x[4] + matrix_multiply(tms->current_k[4], tms->u[4]);
		tms->x[4] = tms->x[3] + matrix_multiply(tms->current_k[3], tms->u[3]);
		tms->x[3] = tms->x[2] + matrix_multiply(tms->current_k[2], tms->u[2]);
		tms->x[2] = tms->x[1] + matrix_multiply(tms->current_k[1], tms->u[1]);
		tms->x[1] = tms->x[0] + matrix_multiply(tms->current_k[0], tms->u[0]);
		tms->x[0] = tms->u[0];
		tms->previous_energy = tms->current_energy;
#ifdef DEBUG_LATTICE
		int i;
		fprintf(stderr,"V:%04d ", tms->u[10]);
		for (i = 9; i >= 0; i--)
		{
			fprintf(stderr,"Y%d:%04d ", i+1, tms->u[i]);
			fprintf(stderr,"b%d:%04d ", i+1, tms->x[i]);
			if ((i % 5) == 0) fprintf(stderr,"\n");
		}
#endif
		return tms->u[0];
}


/**********************************************************************************************

	 process_command -- extract a byte from the FIFO and interpret it as a command

***********************************************************************************************/

static void process_command(tms5220_state *tms, unsigned char cmd)
{
#ifdef DEBUG_COMMAND_DUMP
		fprintf(stderr,"process_command called with parameter %02X\n",cmd);
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
			/* clear out variables before speaking */
			// TODO: similar to the victory case described above, but for VSM speech
			tms->subcycle = tms->subc_reload;
			tms->PC = 0;
			tms->interp_period = reload_table[tms->tms5220c_rate&0x3];
			tms->new_frame_energy_idx = 0;
			tms->new_frame_pitch_idx = 0;
			int i;
			for (i = 0; i < 4; i++)
				tms->new_frame_k_idx[i] = 0;
			for (i = 4; i < 7; i++)
				tms->new_frame_k_idx[i] = 0xF;
			for (i = 7; i < tms->coeff->num_k; i++)
				tms->new_frame_k_idx[i] = 0x7;
			break;

		case 0x60 : /* speak external */
			if (tms->talk_status == 0) /* TALKST must be clear for SPKEXT */
			{
				//SPKEXT going active activates SPKEE which clears the fifo
				tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;
				tms->speak_external = 1;
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
	int indx, i, rep_flag;

	// We actually don't care how many bits are left in the fifo here; the frame subpart will be processed normally, and any bits extracted 'past the end' of the fifo will be read as zeroes; the fifo being emptied will set the /BE latch which will halt speech exactly as if a stop frame had been encountered (instead of whatever partial frame was read); the same exact circuitry is used for both on the real chip, see us patent 4335277 sheet 16, gates 232a (decode stop frame) and 232b (decode /BE plus DDIS (decode disable) which is active during speak external).

	/* if the chip is a tms5220C, and the rate mode is set to that each frame (0x04 bit set)
	has a 2 bit rate preceding it, grab two bits here and store them as the rate; */
	if ((tms->variant == SUBTYPE_TMS5220C) && (tms->tms5220c_rate & 0x04))
	{
		indx = extract_bits(tms, 2);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(indx,2);
		fprintf(stderr," ");
#endif
		tms->interp_period = reload_table[indx];
	}
	else // non-5220C and 5220C in fixed rate mode
	tms->interp_period = reload_table[tms->tms5220c_rate&0x3];

	update_status_and_ints(tms);
	if (!tms->talk_status) goto ranout;

	// attempt to extract the energy index
	tms->new_frame_energy_idx = extract_bits(tms,tms->coeff->energy_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(tms->new_frame_energy_idx,tms->coeff->energy_bits);
	fprintf(stderr," ");
#endif
	update_status_and_ints(tms);
	if (!tms->talk_status) goto ranout;
	// if the energy index is 0 or 15, we're done
	if ((tms->new_frame_energy_idx == 0) || (tms->new_frame_energy_idx == 15))
		return;


	// attempt to extract the repeat flag
	rep_flag = extract_bits(tms,1);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(rep_flag, 1);
	fprintf(stderr," ");
#endif

	// attempt to extract the pitch
	tms->new_frame_pitch_idx = extract_bits(tms,tms->coeff->pitch_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(tms->new_frame_pitch_idx,tms->coeff->pitch_bits);
	fprintf(stderr," ");
#endif
	update_status_and_ints(tms);
	if (!tms->talk_status) goto ranout;
	// if this is a repeat frame, just do nothing, it will reuse the old coefficients
	if (rep_flag)
		return;

	// extract first 4 K coefficients
	for (i = 0; i < 4; i++)
	{
		tms->new_frame_k_idx[i] = extract_bits(tms,tms->coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(tms->new_frame_k_idx[i],tms->coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		update_status_and_ints(tms);
		if (!tms->talk_status) goto ranout;
	}

	// if the pitch index was zero, we only need 4 K's...
	if (tms->new_frame_pitch_idx == 0)
	{
		/* and the rest of the coefficients are zeroed, but that's done in the generator code */
		return;
	}

	// If we got here, we need the remaining 6 K's
	for (i = 4; i < tms->coeff->num_k; i++)
	{
		tms->new_frame_k_idx[i] = extract_bits(tms, tms->coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(tms->new_frame_k_idx[i],tms->coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		update_status_and_ints(tms);
		if (!tms->talk_status) goto ranout;
	}
#ifdef VERBOSE
	if (tms->speak_external)
		logerror("Parsed a frame successfully in FIFO - %d bits remaining\n", (tms->fifo_count*8)-(tms->fifo_bits_taken));
	else
		logerror("Parsed a frame successfully in ROM\n");
#endif
	return;

	ranout:
#ifdef DEBUG_FRAME_ERRORS
	logerror("Ran out of bits on a parse!\n");
#endif
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
	if (!tms->irq_func.isnull() && state != tms->irq_pin)
		tms->irq_func(!state);
	tms->irq_pin = state;
}

/**********************************************************************************************

     update_ready_state -- update the ready line

***********************************************************************************************/

static void update_ready_state(tms5220_state *tms)
{
	int state = tms5220_ready_read(tms);
#ifdef DEBUG_PIN_READS
	logerror("ready pin set to state %d\n", state);
#endif
    if (!tms->readyq_func.isnull() && state != tms->ready_pin)
    	tms->readyq_func(!state);
    tms->ready_pin = state;
}


/**********************************************************************************************

     DEVICE_START( tms5220 ) -- allocate buffers and reset the 5220

***********************************************************************************************/

static DEVICE_START( tms5220 )
{
	static const tms5220_interface dummy = { DEVCB_NULL };
	tms5220_state *tms = get_safe_token(device);

	tms->intf = device->static_config() ? (const tms5220_interface *)device->static_config() : &dummy;
	//tms->table = *device->region();

	tms->device = device;
	tms5220_set_variant(tms, TMS5220_IS_5220);
	tms->clock = device->clock();

	assert_always(tms != NULL, "Error creating TMS5220 chip");

	/* resolve irq and readyq line */
	tms->irq_func.resolve(tms->intf->irq_func, *device);
	tms->readyq_func.resolve(tms->intf->readyq_func, *device);

	/* initialize a stream */
	tms->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock() / 80, tms, tms5220_update);

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

	tms->digital_select = FORCE_DIGITAL; // assume analog output
	/* initialize the FIFO */
	/*memset(tms->fifo, 0, sizeof(tms->fifo));*/
	tms->fifo_head = tms->fifo_tail = tms->fifo_count = tms->fifo_bits_taken = 0;

	/* initialize the chip state */
	/* Note that we do not actually clear IRQ on start-up : IRQ is even raised if tms->buffer_empty or tms->buffer_low are 0 */
	tms->speaking_now = tms->speak_external = tms->talk_status = tms->irq_pin = tms->ready_pin = 0;
	set_interrupt_state(tms, 0);
	update_ready_state(tms);
	tms->buffer_empty = tms->buffer_low = 1;

	tms->RDB_flag = FALSE;

	/* initialize the energy/pitch/k states */
#ifdef PERFECT_INTERPOLATION_HACK
	tms->old_frame_energy_idx = tms->old_frame_pitch_idx = 0;
	memset(tms->old_frame_k_idx, 0, sizeof(tms->old_frame_k_idx));
#endif
	tms->new_frame_energy_idx = tms->current_energy = tms->target_energy = 0;
	tms->new_frame_pitch_idx = tms->current_pitch = tms->target_pitch = 0;
	memset(tms->new_frame_k_idx, 0, sizeof(tms->new_frame_k_idx));
	memset(tms->current_k, 0, sizeof(tms->current_k));
	memset(tms->target_k, 0, sizeof(tms->target_k));

	/* initialize the sample generators */
	tms->inhibit = 1;
	tms->subcycle = tms->tms5220c_rate = tms->pitch_count = tms->PC = 0;
	tms->subc_reload = FORCE_SUBC_RELOAD;
	tms->OLDE = tms->OLDP = 1;
	tms->interp_period = reload_table[tms->tms5220c_rate&0x3];
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
		    tms->stream->update();
		    tms5220_data_write(tms, tms->write_latch);
		    break;
		case 0x01:
			/* Read */
		    /* bring up to date first */
		    tms->stream->update();
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
	update_ready_state(tms);
}

/*
 * /RS line write handler
 */
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
			/* high to low - schedule ready cycle */
#ifdef DEBUG_RS_WS
			logerror("Scheduling ready cycle for /RS...\n");
#endif
			/* upon /RS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
			tms->io_ready = 0;
			update_ready_state(tms);
			/* How long does /READY stay inactive, when /RS is pulled low? I believe its almost always ~16 clocks (25 usec at 800khz as shown on the datasheet) */
			tms->device->machine().scheduler().timer_set(attotime::from_hz(device->clock()/16), FUNC(io_ready_cb), 1, tms); // this should take around 10-16 (closer to ~11?) cycles to complete
		}
	}
}

/*
 * /WS line write handler
 */
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
			/* high to low - schedule ready cycle */
#ifdef DEBUG_RS_WS
			logerror("Scheduling ready cycle for /WS...\n");
#endif
			/* upon /WS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
			tms->io_ready = 0;
			update_ready_state(tms);
			/* Now comes the complicated part: long does /READY stay inactive, when /WS is pulled low? This depends ENTIRELY on the command written, or whether the chip is in speak external mode or not...
            Speak external mode: ~16 cycles
            Command Mode:
            SPK: ? cycles
            SPKEXT: ? cycles
            RDBY: between 60 and 140 cycles
            RB: ? cycles (80?)
            RST: between 60 and 140 cycles
            SET RATE (5220C only): ? cycles (probably ~16)
            */
			// TODO: actually HANDLE the timing differences! currently just assuming always 16 cycles
			tms->device->machine().scheduler().timer_set(attotime::from_hz(device->clock()/16), FUNC(io_ready_cb), 1, tms); // this should take around 10-16 (closer to ~15) cycles to complete for fifo writes, TODO: but actually depends on what command is written if in command mode
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
	    tms->stream->update();
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
	    tms->stream->update();
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
    tms->stream->update();
    return !tms5220_ready_read(tms);
}



/**********************************************************************************************

     tms5220_time_to_ready -- return the time in seconds until the ready line is asserted

***********************************************************************************************/

double tms5220_time_to_ready(device_t *device)
{
	tms5220_state *tms = get_safe_token(device);
	double cycles;

	/* bring up to date first */
	tms->stream->update();
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
    tms->stream->update();
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

void tms5220_set_frequency(device_t *device, int frequency)
{
	tms5220_state *tms = get_safe_token(device);
	tms->stream->set_sample_rate(frequency / 80);
	tms->clock = frequency;
}

const device_type TMS5220C = &device_creator<tms5220c_device>;

tms5220c_device::tms5220c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5220_device(mconfig, TMS5220C, "TMS5220C", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5220c_device::device_start()
{
	DEVICE_START_NAME( tms5220c )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5220c_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMS5220 = &device_creator<tms5220_device>;

tms5220_device::tms5220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS5220, "TMS5220", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tms5220_state);
}
tms5220_device::tms5220_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tms5220_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tms5220_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5220_device::device_start()
{
	DEVICE_START_NAME( tms5220 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms5220_device::device_reset()
{
	DEVICE_RESET_NAME( tms5220 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5220_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMC0285 = &device_creator<tmc0285_device>;

tmc0285_device::tmc0285_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5220_device(mconfig, TMC0285, "TMC0285", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc0285_device::device_start()
{
	DEVICE_START_NAME( tmc0285 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tmc0285_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type TMS5200 = &device_creator<tms5200_device>;

tms5200_device::tms5200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5220_device(mconfig, TMS5200, "TMS5200", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5200_device::device_start()
{
	DEVICE_START_NAME( tms5200 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5200_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}




/******************************************************************************
   New class implementation
******************************************************************************/

#define M_INTERP_SHIFT >> m_coeff->interp_coeff[m_interp_period]

tms52xx_device::tms52xx_device(const machine_config &mconfig, device_type type,  const char *name, const char *tag, const struct tms5100_coeffs* coeffs, const int var, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_variant(var),
	  m_coeff(coeffs)
{
}


tms5220n_device::tms5220n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms52xx_device(mconfig, TMS5220N, "TMS5220N", tag, &tms5220_coeff, TMS5220_IS_5220, owner, clock)
{
}

tms5220cn_device::tms5220cn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms52xx_device(mconfig, TMS5220CN, "TMS5220CN", tag, &tms5220c_coeff, TMS5220_IS_5220C, owner, clock)
{
}

tmc0285n_device::tmc0285n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms52xx_device(mconfig, TMC0285N, "TMC0285N", tag, &tms5200_coeff, TMS5220_IS_TMC0285, owner, clock)
{
}

tms5200n_device::tms5200n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms52xx_device(mconfig, TMS5200N, "TMS5200N", tag, &tms5200_coeff, TMS5220_IS_5200, owner, clock)
{
}

void tms52xx_device::device_start()
{
	const tms52xx_config *conf = reinterpret_cast<const tms52xx_config *>(static_config());
	//m_table = region();

	/* resolve irq and readyq line */
	m_irq_func.resolve(conf->irq_func, *this);
	m_readyq_func.resolve(conf->readyq_func, *this);
	m_read_mem.resolve(conf->read_mem, *this);
	m_load_address.resolve(conf->load_address, *this);
	m_read_and_branch.resolve(conf->read_and_branch, *this);

	/* initialize a stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / 80, this);

	/*if (m_table == NULL)
    {
        assert_always(m_conf->M0_callback != NULL, "Missing _mandatory_ 'M0_callback' function pointer in the TMS5110 interface\n  This function is used by TMS5220 to call for a new single bit\n  needed to generate the speech when in VSM mode\n  Aborting startup...\n");
        m_M0_callback = conf->M0_callback;
        m_set_load_address = conf->load_address;
    }
    else
    {
        m_M0_callback = speech_rom_read_bit;
        m_set_load_address = speech_rom_set_addr;
    }*/

	/* not during reset which is called frm within a write! */
	m_io_ready = true;
	m_true_timing = false;
	m_rs_ws = 0x03; // rs and ws are assumed to be inactive on device startup

	m_ready_timer = timer_alloc(0);

	register_for_save_states();
}

void tms52xx_device::device_reset()
{
	m_digital_select = FORCE_DIGITAL;			// assume analog output
	// initialize the FIFO
	// should we do a memset here to clear the fifo contents?
	m_fifo_head = 0;
	m_fifo_tail = 0;
	m_fifo_count = 0;
	m_fifo_bits_taken = 0;

	// initialize the chip state
	/* Note that we do not actually clear IRQ on start-up: IRQ is even raised
	 * if m_buffer_empty or m_buffer_low are 0 */
	m_speaking_now = false;
	m_speak_external = false;
	m_talk_status = false;
	m_irq_pin = 0;		// CLEAR_LINE
	m_ready_pin = 0;	// CLEAR_LINE

	set_interrupt_state(0);			// CLEAR_LINE
	update_ready_state();
	m_buffer_empty = true;
	m_buffer_low = true;

	m_RDB_flag = false;

	/* initialize the energy/pitch/k states */
#ifdef PERFECT_INTERPOLATION_HACK
	m_old_frame_energy_idx = 0;
	m_old_frame_pitch_idx = 0;
	memset(m_old_frame_k_idx, 0, sizeof(m_old_frame_k_idx));
#endif

	m_new_frame_energy_idx = 0;
	m_current_energy = 0;
	m_target_energy = 0;
	m_new_frame_pitch_idx = 0;
	m_current_pitch = 0;
	m_target_pitch = 0;

	memset(m_new_frame_k_idx, 0, sizeof(m_new_frame_k_idx));
	memset(m_current_k, 0, sizeof(m_current_k));
	memset(m_target_k, 0, sizeof(m_target_k));

	/* initialize the sample generators */
	m_inhibit = true;
	m_subcycle = 0;
	m_tms5220c_rate = 0;
	m_pitch_count = 0;
	m_PC = 0;

	m_subc_reload = FORCE_SUBC_RELOAD;
	m_OLDE = 1;
	m_OLDP = 1;

	m_interp_period = reload_table[m_tms5220c_rate&0x3];
	m_RNG = 0x1FFF;
	memset(m_u, 0, sizeof(m_u));
	memset(m_x, 0, sizeof(m_x));

	m_load_address(0, 0);
	m_schedule_dummy_read = true;
}

/******************************************************************************
     set_interrupt_state -- generate an interrupt
*******************************************************************************/

void tms52xx_device::set_interrupt_state(int state)
{
#ifdef DEBUG_PIN_READS
	logerror("tms52xx: irq pin set to state %d\n", state);
#endif
    if (state != m_irq_pin) m_irq_func(!state);
    m_irq_pin = state;
}

/******************************************************************************
     update_ready_state -- update the ready line
*******************************************************************************/

void tms52xx_device::update_ready_state()
{
	int state = ready_read();
#ifdef DEBUG_PIN_READS
	logerror("tms52xx: ready pin set to state %d\n", state);
#endif
	if (state != m_ready_pin) m_readyq_func(!state);
	m_ready_pin = state;
}

/******************************************************************************
     tms5220_set_frequency -- adjusts the playback frequency
*******************************************************************************/

void tms52xx_device::set_frequency(int frequency)
{
	m_stream->set_sample_rate(frequency / 80);
	m_clock = frequency;
}

/******************************************************************************
     tms5220_update -- update the sound chip so that it is in sync with CPU execution
*******************************************************************************/

void tms52xx_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples>0)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		process(sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
	}
}

/******************************************************************************
    process -- fill the buffer with a specific number of samples
*******************************************************************************/

void tms52xx_device::process(INT16 *buffer, unsigned int size)
{
	int buf_count=0;
	int i, bitout;
	bool zpar;
	INT32 this_sample;

	// The following gotos are probably safe to remove
	// if we're empty and still not speaking, fill with nothingness
	if (!m_speaking_now) goto empty;

	// if speak external is set, but talk status is not (yet) set,
	// wait for buffer low to clear
	if (!m_talk_status && m_speak_external && m_buffer_low) goto empty;

	// loop until the buffer is full or we've stopped speaking
	while ((size > 0) && m_speaking_now)
	{
		/* if it is the appropriate time to update the old energy/pitch idxes,
		 * i.e. when IP=7, PC=12, T=17, subcycle=2, do so. Since IP=7 PC=12 T=17
		 * is JUST BEFORE the transition to IP=0 PC=0 T=0 sybcycle=(0 or 1),
		 * which happens 4 T-cycles later), we change on the latter.
		 */
		if ((m_interp_period == 0) && (m_PC == 0) && (m_subcycle < 2))
		{
			m_OLDE = (m_new_frame_energy_idx == 0);
			m_OLDP = (m_new_frame_pitch_idx == 0);
		}

		/* if we're ready for a new frame to be applied, i.e. when IP=0, PC=12, Sub=1
		 * (In reality, the frame was really loaded incrementally during the
		 * entire IP=0 PC=x time period, but it doesn't affect anything until IP=0 PC=12 happens)
		 */
		if ((m_interp_period == 0) && (m_PC == 12) && (m_subcycle == 1))
		{
			// HACK for regression testing, be sure to comment out before release!
			// m_RNG = 0x1234;
			// end HACK

			// appropriately override the interp count if needed; this will be incremented after the frame parse!
			m_interp_period = reload_table[m_tms5220c_rate & 0x3];

#ifdef PERFECT_INTERPOLATION_HACK
			// remember previous frame energy, pitch, and coefficients
			m_old_frame_energy_idx = m_new_frame_energy_idx;
			m_old_frame_pitch_idx = m_new_frame_pitch_idx;
			for (i = 0; i < m_coeff->num_k; i++)
				m_old_frame_k_idx[i] = m_new_frame_k_idx[i];
#endif

			// if the talk status was clear last frame, halt speech now.
			if (m_talk_status == false)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"tms5220_process: processing frame: talk status = 0 caused by stop frame or buffer empty, halting speech.\n");
#endif
				m_speaking_now = false; // finally halt speech
				goto empty;
			}

			// Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[]
			parse_frame();
#ifdef DEBUG_PARSE_FRAME_DUMP
			fprintf(stderr,"\n");
#endif
			// if the new frame is a stop frame, set an interrupt and set talk status to 0
			if (M_NEW_FRAME_STOP_FLAG == 1)
			{
				m_talk_status = false;
				m_speak_external = false;
				set_interrupt_state(1);
				update_status_and_ints();
			}

			// in all cases where interpolation would be inhibited, set the inhibit flag; otherwise clear it.
			// Interpolation inhibit cases:
			// * Old frame was voiced, new is unvoiced
			// * Old frame was silence/zero energy, new has nonzero energy
			// * Old frame was unvoiced, new is voiced

			if ( ((M_OLD_FRAME_UNVOICED_FLAG == false) && (M_NEW_FRAME_UNVOICED_FLAG == true))
				|| ((M_OLD_FRAME_UNVOICED_FLAG == true) && (M_NEW_FRAME_UNVOICED_FLAG == false))
				|| ((M_OLD_FRAME_SILENCE_FLAG == true) && (M_NEW_FRAME_SILENCE_FLAG == false)) )
				m_inhibit = true;
			else // normal frame, normal interpolation
				m_inhibit = false;

			// load new frame targets from tables, using parsed indices
			m_target_energy = m_coeff->energytable[m_new_frame_energy_idx];
			m_target_pitch = m_coeff->pitchtable[m_new_frame_pitch_idx];
			zpar = M_NEW_FRAME_UNVOICED_FLAG; // find out if parameters k5-k10 should be zeroed
			for (i = 0; i < 4; i++)
				m_target_k[i] = m_coeff->ktable[i][m_new_frame_k_idx[i]];
			for (i = 4; i < m_coeff->num_k; i++)
				m_target_k[i] = (m_coeff->ktable[i][m_new_frame_k_idx[i]] * (1-zpar));

#ifdef DEBUG_GENERATION
			/* Debug info for current parsed frame */
			fprintf(stderr, "OLDE: %d; OLDP: %d; ", m_OLDE, m_OLDP);
			fprintf(stderr,"Processing frame: ");
			if (m_inhibit == 0)
				fprintf(stderr, "Normal Frame\n");
			else
				fprintf(stderr,"Interpolation Inhibited\n");
			fprintf(stderr,"*** current Energy, Pitch and Ks =      %04d,   %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",
				m_current_energy, m_current_pitch,
				m_current_k[0], m_current_k[1],	m_current_k[2], m_current_k[3],
				m_current_k[4], m_current_k[5], m_current_k[6], m_current_k[7],
				m_current_k[8], m_current_k[9]);
			fprintf(stderr,"*** target Energy(idx), Pitch, and Ks = %04d(%x),%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",
				m_target_energy, m_new_frame_energy_idx, m_target_pitch,
				m_target_k[0], m_target_k[1], m_target_k[2], m_target_k[3],
				m_target_k[4], m_target_k[5], m_target_k[6], m_target_k[7],
				m_target_k[8], m_target_k[9]);
#endif

			/* if TS is now 0, ramp the energy down to 0. Is this really correct to hardware? */
			if (!m_talk_status)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"Talk status is 0, forcing target energy to 0\n");
#endif
				m_target_energy = 0;
			}
		}
		else // Not a new frame, just interpolate the existing frame.
		{
			bool inhibit_state = (m_inhibit && (m_interp_period != 0)); // disable inhibit when reaching the last interp period, but don't overwrite the tms->inhibit value
#ifdef PERFECT_INTERPOLATION_HACK
			int samples_per_frame = (m_subc_reload!=0)? 175:266;		// either (13 A cycles + 12 B cycles) * 7 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 7 interps for SPKSLOW
			//int samples_per_frame = (m_subc_reload!=0)?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
			int current_sample = (m_subcycle - m_subc_reload)+(m_PC*(3-m_subc_reload))+((m_subc_reload?25:38)*((m_interp_period-1)&7));

			zpar = M_OLD_FRAME_UNVOICED_FLAG;
			//fprintf(stderr, "CS: %03d", current_sample);
			// reset the current energy, pitch, etc to what it was at frame start
			m_current_energy = m_coeff->energytable[m_old_frame_energy_idx];
			m_current_pitch = m_coeff->pitchtable[m_old_frame_pitch_idx];
			for (i = 0; i < 4; i++)
				m_current_k[i] = m_coeff->ktable[i][m_old_frame_k_idx[i]];
			for (i = 4; i < m_coeff->num_k; i++)
				m_current_k[i] = (m_coeff->ktable[i][m_old_frame_k_idx[i]] * (1-zpar));
			// now adjust each value to be exactly correct for each of the samples per frame
			if (m_interp_period != 0) // if we're still interpolating...
			{
				if (!inhibit_state)
				{
					m_current_energy += ((m_target_energy - m_current_energy)*current_sample)/samples_per_frame;
					m_current_pitch += ((m_target_pitch - m_current_pitch)*current_sample)/samples_per_frame;
					for (i = 0; i < m_coeff->num_k; i++)
					{
						m_current_k[i] += ((m_target_k[i] - m_current_k[i])*current_sample)/samples_per_frame;
					}
				}
			}
			else // we're done, play this frame for 1/8 frame.
			{
				m_current_energy = m_target_energy;
				m_current_pitch = m_target_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_current_k[i] = m_target_k[i];
			}
#else
			// Updates to parameters only happen on subcycle '2' (B cycle) of PCs.
			if (m_subcycle == 2)
			{
				if (!inhibit_state)
				{
					switch(m_PC)
					{
					case 0: // PC = 0, B cycle, write updated energy
						m_current_energy += ((m_target_energy - m_current_energy) M_INTERP_SHIFT);
						break;
					case 1: /* PC = 1, B cycle, write updated pitch */
						m_current_pitch += ((m_target_pitch - m_current_pitch) M_INTERP_SHIFT);
						break;
					case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
						/* PC = 2 through 11, B cycle, write updated K1 through K10 */
						m_current_k[m_PC-2] += ((m_target_k[m_PC-2] - m_current_k[m_PC-2]) M_INTERP_SHIFT);
						break;
					case 12: /* PC = 12, do nothing */
						break;
					}
				}
			}
#endif
		}

		// calculate the output
		if (M_OLD_FRAME_UNVOICED_FLAG == true)
		{
			// generate unvoiced samples here
#ifndef UNVOICED_HACK
			if (m_RNG & 1)
				m_excitation_data = ~0x3F; // according to the patent it is (either + or -) half of the maximum value in the chirp table, so either 01000000(0x40) or 11000000(0xC0)
			else
				m_excitation_data = 0x40;
#else // hack to tweak unvoiced strength, doesn't match patent
			if (m_RNG & 1)
				m_excitation_data = 0;
			else
				m_excitation_data = 0x40;
#endif
		}
		else // (M_OLD_FRAME_UNVOICED_FLAG == false)
		{
			// generate voiced samples here
			// US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			// function has a chirp/peak and then a long chain of zeroes.
			// The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
			// and if the address reaches that point the ADDRESS incrementer is
			// disabled, forcing all samples beyond 51d to be == 51d
			// (address 51d holds zeroes, which may or may not be inverted to -1)

			if (m_pitch_count >= 51)
				m_excitation_data = m_coeff->chirptable[51];
			else // tms->pitch_count < 51
				m_excitation_data = m_coeff->chirptable[m_pitch_count];
		}

		// Update LFSR *20* times every sample (once per T cycle), like patent shows
		for (i=0; i<20; i++)
		{
			bitout = ((m_RNG >> 12) & 1) ^
					((m_RNG >>  3) & 1) ^
					((m_RNG >>  2) & 1) ^
					((m_RNG >>  0) & 1);
			m_RNG <<= 1;
			m_RNG |= bitout;
		}

		this_sample = lattice_filter(); // execute lattice filter
#ifdef DEBUG_GENERATION_VERBOSE
		//fprintf(stderr,"C:%01d; ",tms->subcycle);
		fprintf(stderr,"IP:%01d PC:%02d X:%04d E:%03d P:%03d Pc:%03d ",m_interp_period, m_PC,
			m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
		//fprintf(stderr,"X:%04d E:%03d P:%03d Pc:%03d ", m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
		for (i=0; i<10; i++)
			fprintf(stderr,"K%d:%04d ", i+1, m_current_k[i]);
		fprintf(stderr,"Out:%06d", this_sample);
		fprintf(stderr,"\n");
#endif
		// next, force result to 14 bits (since its possible that the addition at the final (k1) stage of the lattice overflowed)
		while (this_sample > 16383) this_sample -= 32768;
		while (this_sample < -16384) this_sample += 32768;
		if (!m_digital_select) // analog SPK pin output is only 8 bits, with clipping
			buffer[buf_count] = clip_analog(this_sample);
		else // digital I/O pin output is 12 bits
		{
#ifdef ALLOW_4_LSB
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn nnnn
			// N taps:                       ^                 = 0x2000;
			// output: ssss ssss ssss ssss snnn nnnn nnnn nnnN
			buffer[buf_count] = (this_sample<<1)|((this_sample&0x2000)>>13);
#else
			this_sample &= ~0xF;
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn 0000
			// N taps:                       ^^ ^^^            = 0x3E00;
			// output: ssss ssss ssss ssss snnn nnnn nnnN NNNN
			buffer[buf_count] = (this_sample<<1)|((this_sample&0x3E00)>>9);
#endif
		}

		// Update all counts
		m_subcycle++;
		if ((m_subcycle == 2) && (m_PC == 12))
		{
			m_subcycle = m_subc_reload;
			m_PC = 0;
			m_interp_period++;
			m_interp_period &= 0x7;
		}
		else if (m_subcycle == 3)
		{
			m_subcycle = m_subc_reload;
			m_PC++;
		}
		/* Circuit 412 in the patent ensures that when INHIBIT is true,
		 * during the period from IP=7 PC=12 T12, to IP=0 PC=12 T12, the pitch
		 * count is forced to 0; since the initial stop happens right before
		 * the switch to IP=0 PC=0 and this code is located after the switch would
		 * happen, we check for ip=0 inhibit=1, which covers that whole range.
		 * The purpose of Circuit 412 is to prevent a spurious click caused by
		 * the voiced source being fed to the filter before all the values have
		 * been updated during ip=0 when interpolation was inhibited.
		 */
		m_pitch_count++;
		if (m_pitch_count >= m_current_pitch) m_pitch_count = 0;
		if ((m_interp_period == 0) && m_inhibit) m_pitch_count = 0;
		m_pitch_count &= 0x1FF;
		buf_count++;
		size--;
	}

empty:

	while (size > 0)
	{
		m_subcycle++;
		if ((m_subcycle == 2) && (m_PC == 12))
		{
			m_subcycle = m_subc_reload;
			m_PC = 0;
			m_interp_period++;
			m_interp_period &= 0x7;
		}
		else if (m_subcycle == 3)
		{
			m_subcycle = m_subc_reload;
			m_PC++;
		}
		buffer[buf_count] = -1;	// should be just -1; actual chip outputs -1 every idle sample; (cf note in data sheet, p 10, table 4)
		buf_count++;
		size--;
	}
}

/******************************************************************************
    lattice_filter -- executes one 'full run' of the lattice filter on a
    specific byte of excitation data, and specific values of all the current k
    constants,  and returns the resulting sample.
******************************************************************************/

INT32 tms52xx_device::lattice_filter()
{
	/* Lattice filter here */
	/* Aug/05/07: redone as unrolled loop, for clarity - LN
	 * Originally Copied verbatim from table I in US patent 4,209,804, now updated
	 * to be in same order as the actual chip does it, not that it matters.
	 * notation equivalencies from table:
	 *      Yn(i) == m_u[n-1]
	 *      Kn = m_current_k[n-1]
	 *      bn = m_x[n-1]
	 */

	m_u[10] = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
	m_u[9] = m_u[10] - matrix_multiply(m_current_k[9], m_x[9]);
	m_u[8] = m_u[9] - matrix_multiply(m_current_k[8], m_x[8]);
	m_u[7] = m_u[8] - matrix_multiply(m_current_k[7], m_x[7]);
	m_u[6] = m_u[7] - matrix_multiply(m_current_k[6], m_x[6]);
	m_u[5] = m_u[6] - matrix_multiply(m_current_k[5], m_x[5]);
	m_u[4] = m_u[5] - matrix_multiply(m_current_k[4], m_x[4]);
	m_u[3] = m_u[4] - matrix_multiply(m_current_k[3], m_x[3]);
	m_u[2] = m_u[3] - matrix_multiply(m_current_k[2], m_x[2]);
	m_u[1] = m_u[2] - matrix_multiply(m_current_k[1], m_x[1]);
	m_u[0] = m_u[1] - matrix_multiply(m_current_k[0], m_x[0]);
	m_x[9] = m_x[8] + matrix_multiply(m_current_k[8], m_u[8]);
	m_x[8] = m_x[7] + matrix_multiply(m_current_k[7], m_u[7]);
	m_x[7] = m_x[6] + matrix_multiply(m_current_k[6], m_u[6]);
	m_x[6] = m_x[5] + matrix_multiply(m_current_k[5], m_u[5]);
	m_x[5] = m_x[4] + matrix_multiply(m_current_k[4], m_u[4]);
	m_x[4] = m_x[3] + matrix_multiply(m_current_k[3], m_u[3]);
	m_x[3] = m_x[2] + matrix_multiply(m_current_k[2], m_u[2]);
	m_x[2] = m_x[1] + matrix_multiply(m_current_k[1], m_u[1]);
	m_x[1] = m_x[0] + matrix_multiply(m_current_k[0], m_u[0]);
	m_x[0] = m_u[0];
	m_previous_energy = m_current_energy;
#ifdef DEBUG_LATTICE

	int i;
	fprintf(stderr,"V:%04d ", m_u[10]);
	for (i = 9; i >= 0; i--)
	{
		fprintf(stderr,"Y%d:%04d ", i+1, m_u[i]);
		fprintf(stderr,"b%d:%04d ", i+1, m_x[i]);
		if ((i % 5) == 0) fprintf(stderr,"\n");
	}
#endif
	return m_u[0];
}

/**********************************************************************************************
    data_write -- handle a write to the TMS5220
***********************************************************************************************/

void tms52xx_device::data_write(int data)
{
#ifdef DEBUG_DUMP_INPUT_DATA
	fprintf(stdout, "%c",data);
#endif
	if (m_speak_external) // If we're in speak external mode
	{
		/* add this byte to the FIFO */
		if (m_fifo_count < FIFO_SIZE)
		{
			m_fifo[m_fifo_tail] = data;
			m_fifo_tail = (m_fifo_tail + 1) % FIFO_SIZE;
			m_fifo_count++;
#ifdef DEBUG_FIFO
			logerror("tms52xx: data_write: Added byte to FIFO (current count=%2d)\n", m_fifo_count);
#endif
			update_status_and_ints();
			if ((!m_talk_status) && (!m_buffer_low)) // we just unset buffer low with that last write, and talk status *was* zero...
			{
				int i;
#ifdef DEBUG_FIFO
				logerror("tms52xx: data_write triggered talk status to go active!\n");
#endif
				/* ...then we now have enough bytes to start talking; clear out
				 * the new frame parameters (it will become old frame just before the first call to parse_frame())
				 * TODO: the 3 lines below (and others) are needed for victory
				 * to not fail its selftest due to a sample ending too late, may require additional investigation */
				m_subcycle = m_subc_reload;
				m_PC = 0;
				m_interp_period = reload_table[m_tms5220c_rate & 0x3]; // is this correct? should this be always 7 instead, so that the new frame is loaded quickly?
				m_new_frame_energy_idx = 0;
				m_new_frame_pitch_idx = 0;

				for (i = 0; i < 4; i++)
					m_new_frame_k_idx[i] = 0;
				for (i = 4; i < 7; i++)
					m_new_frame_k_idx[i] = 0xF;
				for (i = 7; i < m_coeff->num_k; i++)
					m_new_frame_k_idx[i] = 0x7;
				m_talk_status = m_speaking_now = true;
			}
		}
		else
		{
#ifdef DEBUG_FIFO
			logerror("tms52xx: data_write: Ran out of room in the tms52xx FIFO! this should never happen!\n");
			// at this point, /READY should remain HIGH/inactive until the fifo has at least one byte open in it.
#endif
		}


	}
	else //(! m_speak_external)
		/* R Nabet : we parse commands at once.  It is necessary for such commands as read. */
		process_command(data);
}

/******************************************************************************
    process_command -- extract a byte from the FIFO and interpret it as a command
*******************************************************************************/

void tms52xx_device::process_command(unsigned char cmd)
{
#ifdef DEBUG_COMMAND_DUMP
	fprintf(stderr,"process_command called with parameter %02X\n",cmd);
#endif
	// parse the command
	switch (cmd & 0x70)
	{
	case 0x10 : // read byte
		if (!m_talk_status) // TALKST must be clear for RDBY
		{
			if (m_schedule_dummy_read)
			{
				m_schedule_dummy_read = false;
				(void)m_read_mem(1);
			}
			m_data_register = m_read_mem(8);	// read one byte from speech ROM...
			m_RDB_flag = true;
		}
		break;

	case 0x00:
	case 0x20: // set rate (tms5220c only), otherwise NOP
		if (m_variant == SUBTYPE_TMS5220C)
		{
			m_tms5220c_rate = cmd & 0x0F;
		}
		break;

	case 0x30: // read and branch
		if (!m_talk_status) // TALKST must be clear for RB
		{
#ifdef VERBOSE
			logerror("tms5520: read and branch command received\n");
#endif
			m_RDB_flag = false;
			m_read_and_branch(0, 0);
		}
		break;

	case 0x40 : // load address
		if (!m_talk_status) // TALKST must be clear for LA
		{
			// tms5220 data sheet says that if we load only one 4-bit nibble, it won't work.
			// This code does not care about this.
			m_load_address(0, cmd & 0x0f);
			m_schedule_dummy_read = true;
		}
		break;

	case 0x50:	// speak
		if (m_schedule_dummy_read)
		{
			m_schedule_dummy_read = false;
			(void)m_read_mem(1);
		}
		m_speaking_now = true;
		m_speak_external = false;
		m_talk_status = true;  // start immediately
		// clear out variables before speaking
		// TODO: similar to the victory case described above, but for VSM speech
		m_subcycle = m_subc_reload;
		m_PC = 0;
		m_interp_period = reload_table[m_tms5220c_rate & 0x3];
		m_new_frame_energy_idx = 0;
		m_new_frame_pitch_idx = 0;

		int i;
		for (i = 0; i < 4; i++)
			m_new_frame_k_idx[i] = 0;
		for (i = 4; i < 7; i++)
			m_new_frame_k_idx[i] = 0xF;
		for (i = 7; i < m_coeff->num_k; i++)
			m_new_frame_k_idx[i] = 0x7;
		break;

	case 0x60: // speak external
		if (!m_talk_status) // TALKST must be clear for SPKEXT
		{
			//SPKEXT going active activates SPKEE which clears the fifo
			m_fifo_head = m_fifo_tail = 0;
			m_fifo_count = m_fifo_bits_taken = 0;
			m_speak_external = true;
			m_RDB_flag = false;
		}
		break;

	case 0x70: // reset
		if (m_schedule_dummy_read)
		{
			m_schedule_dummy_read = false;
			(void)m_read_mem(1);
		}
		device_reset();
		break;
	}

	// update the buffer low state
	update_status_and_ints();
}

/******************************************************************************
    parse_frame -- parse a new frame's worth of data; returns 0 if not
    enough bits in buffer
*******************************************************************************/

void tms52xx_device::parse_frame()
{
	int indx, i, rep_flag;

	/* We actually don't care how many bits are left in the fifo here; the
	 * frame subpart will be processed normally, and any bits extracted 'past
	 * the end' of the fifo will be read as zeroes; the fifo being emptied will
	 * set the /BE latch which will halt speech exactly as if a stop frame had
	 * been encountered (instead of whatever partial frame was read); the same
	 * exact circuitry is used for both on the real chip, see us patent 4335277
	 * sheet 16, gates 232a (decode stop frame) and 232b (decode /BE plus DDIS
	 * (decode disable) which is active during speak external). */

	/* if the chip is a tms5220C, and the rate mode is set to that each frame (0x04 bit set)
	 * has a 2 bit rate preceding it, grab two bits here and store them as the rate; */
	if ((m_variant == SUBTYPE_TMS5220C) && (m_tms5220c_rate & 0x04))
	{
		indx = extract_bits(2);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(indx,2);
		fprintf(stderr," ");
#endif
		m_interp_period = reload_table[indx];
	}
	else // non-5220C and 5220C in fixed rate mode
		m_interp_period = reload_table[m_tms5220c_rate & 0x3];

	update_status_and_ints();
	if (!m_talk_status) goto ranout;

	// attempt to extract the energy index
	m_new_frame_energy_idx = extract_bits(m_coeff->energy_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_energy_idx,m_coeff->energy_bits);
	fprintf(stderr," ");
#endif
	update_status_and_ints();
	if (!m_talk_status) goto ranout;
	// if the energy index is 0 or 15, we're done
	if ((m_new_frame_energy_idx == 0) || (m_new_frame_energy_idx == 15))
		return;

	// attempt to extract the repeat flag
	rep_flag = extract_bits(1);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(rep_flag, 1);
	fprintf(stderr," ");
#endif

	// attempt to extract the pitch
	m_new_frame_pitch_idx = extract_bits(m_coeff->pitch_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_pitch_idx, m_coeff->pitch_bits);
	fprintf(stderr," ");
#endif
	update_status_and_ints();
	if (!m_talk_status) goto ranout;
	/* if this is a repeat frame, just do nothing, it will reuse the
	 * old coefficients */
	if (rep_flag) return;

	// extract first 4 K coefficients
	for (i = 0; i < 4; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i], m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		update_status_and_ints();
		if (!m_talk_status) goto ranout;
	}

	// if the pitch index was zero, we only need 4 K's...
	if (m_new_frame_pitch_idx == 0)
	{
		// and the rest of the coefficients are zeroed, but that's done in the generator code
		return;
	}

	// If we got here, we need the remaining 6 K's
	for (i = 4; i < m_coeff->num_k; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i], m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		update_status_and_ints();
		if (!m_talk_status) goto ranout;
	}
#ifdef VERBOSE
	if (m_speak_external)
		logerror("tms52xx: Parsed a frame successfully in FIFO - %d bits remaining\n", (m_fifo_count*8)-(m_fifo_bits_taken));
	else
		logerror("tms52xx: Parsed a frame successfully in ROM\n");
#endif
	return;

	ranout:
#ifdef DEBUG_FRAME_ERRORS
	logerror("tms52xx: Ran out of bits on a parse!\n");
#endif
	return;
}

/**********************************************************************************************

     update_status_and_ints -- check to see if the various flags should be on or off
     Description of flags, and their position in the status register:
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
                eighth data byte of the stack.
        bit D2(bit 5) = BE - Buffer Empty is active (high) when the FIFO buffer has run out of data
                while executing a Speak External command. Buffer Empty is set when the last bit
                of the "Last-In" byte is shifted out to the Synthesis Section. This causes
                Talk Status to be cleared. Speech is terminated at some abnormal point and the
                Speak External command execution is terminated.

***********************************************************************************************/

void tms52xx_device::update_status_and_ints()
{
	// update flags and set ints if needed
	update_ready_state();

	/* BL is set if neither byte 9 nor 8 of the fifo are in use; this
	 * translates to having fifo_count (which ranges from 0 bytes in use to 16
	 * bytes used) being less than or equal to 8. Victory/Victorba depends on this. */
	if (m_fifo_count <= 8)
	{
		// generate an interrupt if necessary; if /BL was inactive and is now active, set int.
		if (!m_buffer_low) set_interrupt_state(1);
		m_buffer_low = true;
	}
	else
		m_buffer_low = false;

	/* BE is set if neither byte 15 nor 14 of the fifo are in use; this
	 * translates to having fifo_count equal to exactly 0 */
	if (m_fifo_count == 0)
	{
		// generate an interrupt if necessary; if /BE was inactive and is now active, set int.
		if (!m_buffer_empty) set_interrupt_state(1);
		m_buffer_empty = true;
	}
	else
		m_buffer_empty = false;

	/* TS is talk status and is set elsewhere in the fifo parser and in
	 * the SPEAK command handler; however, if /BE is true during speak external
	 * mode, it is immediately unset here. */
	if (m_speak_external && m_buffer_empty)
	{
		// generate an interrupt: /TS was active, and is now inactive.
		if (m_talk_status)
		{
			m_talk_status = m_speak_external = false;
			set_interrupt_state(1);
		}
	}
	/* Note that TS being unset will also generate an interrupt when a STOP
	 * frame is encountered; this is handled in the sample generator code and not here */
}

/******************************************************************************
    extract_bits -- extract a specific number of bits from the current input stream (FIFO or VSM)
*******************************************************************************/

int tms52xx_device::extract_bits(int count)
{
	int val = 0;

	if (m_speak_external)
	{
		// extract from FIFO
		while (count--)
		{
			val = (val << 1) | ((m_fifo[m_fifo_head] >> m_fifo_bits_taken) & 1);
			m_fifo_bits_taken++;
			if (m_fifo_bits_taken >= 8)
			{
				m_fifo_count--;
				m_fifo[m_fifo_head] = 0; // zero the newly depleted fifo head byte
				m_fifo_head = (m_fifo_head + 1) % FIFO_SIZE;
				m_fifo_bits_taken = 0;
				update_status_and_ints();
			}
		}
	}
	else
	{
		// extract from VSM (speech ROM)
		val = m_read_mem(count);
	}
	return val;
}

/******************************************************************************
    status_read -- read status or data from the TMS5220
*******************************************************************************/

int tms52xx_device::status_read()
{
	if (m_RDB_flag)
	{	// if last command was read, return data register
		m_RDB_flag = false;
		return m_data_register;
	}
	else
	{	// read status
		// clear the interrupt pin on status read
		set_interrupt_state(0);
#ifdef DEBUG_PIN_READS
		logerror("tms52xx: Status read: TS=%d BL=%d BE=%d\n", m_talk_status, m_buffer_low, m_buffer_empty);
#endif

		int retvalue = 0;
		if (m_talk_status) retvalue |= 0x80;
		if (m_buffer_low) retvalue |= 0x40;
		if (m_buffer_empty) retvalue |= 0x20;
		return retvalue;
	}
}

/******************************************************************************
    ready_read -- returns the ready state of the TMS5220
*******************************************************************************/

inline bool tms52xx_device::ready_read()
{
#ifdef DEBUG_PIN_READS
	logerror("tms52xx: ready_read: ready pin read, io_ready is %d, fifo count is %d\n", m_io_ready, m_fifo_count);
#endif
	return ((m_fifo_count < FIFO_SIZE)||(!m_speak_external)) && m_io_ready;
}

/******************************************************************************
    int_read -- returns the interrupt state of the TMS5220
*******************************************************************************/

inline int tms52xx_device::int_read()
{
#ifdef DEBUG_PIN_READS
	logerror("tms52xx: int_read: irq pin read, state is %d\n", m_irq_pin);
#endif
	return m_irq_pin;
}

/******************************************************************************
    cycles_to_ready -- returns the number of cycles until ready is asserted
    NOTE: this function is deprecated and is known to be VERY inaccurate.
    Use at your own peril!
*******************************************************************************/

int tms52xx_device::cycles_to_ready()
{
	int answer;

	if (ready_read()) answer = 0;
	else
	{
		int val;
		int samples_per_frame = (m_subc_reload!=0)?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
		int current_sample = ((m_PC * (3-m_subc_reload)) + (((m_subc_reload!=0)? 38:25) * m_interp_period));
		answer = samples_per_frame - current_sample + 8;

		// total number of bits available in current byte is (8 - tms->fifo_bits_taken)
		// if more than 4 are available, we need to check the energy
		if (m_fifo_bits_taken < 4)
		{
			// read energy
			val = (m_fifo[m_fifo_head] >> m_fifo_bits_taken) & 0xf;
			if (val == 0)
				// 0 -> silence frame: we will only read 4 bits, and we will
				// therefore need to read another frame before the FIFO is not
				// full any more
				answer += (m_subc_reload!=0)?200:304;
			// 15 -> stop frame, we will only read 4 bits, but the FIFO will
			// we cleared
			//otherwise, we need to parse the repeat flag (1 bit) and the
			// pitch (6 bits), so everything will be OK.
		}
	}
	return answer;
}

/******************************************************************************
    True timing
*******************************************************************************/

void tms52xx_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (param)
	{
		switch (m_rs_ws)
		{
		case 0x02:
			// Write
			// bring up to date first
#ifdef DEBUG_IO_READY
			logerror("tms52xx: Serviced write: %02x\n", tms->write_latch);
			//fprintf(stderr, "Processed write data: %02X\n", tms->write_latch);
#endif
			m_stream->update();
			data_write(m_write_latch);
		    break;
		case 0x01:
			// Read
			// bring up to date first
			m_stream->update();
			m_read_latch = status_read();
			break;
		case 0x03:
			// High Impedance
		case 0x00:
			// illegal
			break;
		}
	}
	m_io_ready = param;
	update_ready_state();
}

/*****************************************************
    /RS line write handler
*****************************************************/

WRITE_LINE_MEMBER( tms52xx_device::rsq_w )
{
	UINT8 new_val;

	m_true_timing = true;
	state &= 0x01;
#ifdef DEBUG_RS_WS
	logerror("tms52xx: /RS written with data: %d\n", state);
#endif
	new_val = (m_rs_ws & 0x01) | (state<<1);
	if (new_val != m_rs_ws)
	{
		m_rs_ws = new_val;
		if (new_val == 0)
		{
			if (m_variant == SUBTYPE_TMS5220C)
				reset();
#ifdef DEBUG_RS_WS
			else
				// illegal
				logerror("tms52xx: illegal line setting /RS=0 and /WS=0\n");
#endif
			return;
		}
		else if (new_val == 3)
		{
			// high impedance
			m_read_latch = 0xff;
			return;
		}
		if (state)
		{
			// low to high
		}
		else
		{
			// high to low - schedule ready cycle
#ifdef DEBUG_RS_WS
			logerror("tms52xx: Scheduling ready cycle for /RS...\n");
#endif
			/* upon /RS being activated, /READY goes inactive after 100 nsec from
			 * data sheet, through 3 asynchronous gates on patent. This is effectively
			 * within one clock, so we immediately set io_ready to 0 and activate the callback. */
			m_io_ready = 0;
			update_ready_state();
			/* How long does /READY stay inactive, when /RS is pulled low?
			 * I believe its almost always ~16 clocks (25 usec at 800khz as shown on the datasheet) */
			m_ready_timer->adjust(attotime::from_hz(clock()/16));
		}
	}
}

/*****************************************************
    /WS line write handler
*****************************************************/

WRITE_LINE_MEMBER( tms52xx_device::wsq_w )
{
	UINT8 new_val;

	m_true_timing = true;
	state &= 0x01;
#ifdef DEBUG_RS_WS
	logerror("tms52xx: /WS written with data: %d\n", state);
#endif
	new_val = (m_rs_ws & 0x02) | (state<<0);
	if (new_val != m_rs_ws)
	{
		m_rs_ws = new_val;
		if (new_val == 0)
		{
			if (m_variant == SUBTYPE_TMS5220C)
				reset();
#ifdef DEBUG_RS_WS
			else
				// illegal
				logerror("tms52xx: illegal line setting /RS=0 and /WS=0\n");
#endif
			return;
		}
		else if ( new_val == 3)
		{
			// high impedance
			m_read_latch = 0xff;
			return;
		}
		if (state)
		{
			// low to high
		}
		else
		{
			// high to low - schedule ready cycle
#ifdef DEBUG_RS_WS
			logerror("tms52xx: Scheduling ready cycle for /WS...\n");
#endif
			/* upon /WS being activated, /READY goes inactive after 100 nsec
			 * from data sheet, through 3 asynchronous gates on patent.
			 * This is effectively within one clock, so we immediately set
			 * io_ready to 0 and activate the callback. */
			m_io_ready = 0;
			update_ready_state();
			/* Now comes the complicated part: long does /READY stay inactive
			 * when /WS is pulled low? This depends ENTIRELY on the command written,
			 * or whether the chip is in speak external mode or not...
			 * Speak external mode: ~16 cycles
			 * Command Mode:
			 * SPK: ? cycles
			 * SPKEXT: ? cycles
			 * RDBY: between 60 and 140 cycles
			 * RB: ? cycles (80?)
			 * RST: between 60 and 140 cycles
			 * SET RATE (5220C only): ? cycles (probably ~16) */

			// TODO: actually HANDLE the timing differences! currently just assuming always 16 cycles
			m_ready_timer->adjust(attotime::from_hz(clock()/16));
		}
	}
}

/*****************************************************************************
    write -- write data to the sound chip
*******************************************************************************/

WRITE8_MEMBER( tms52xx_device::write )
{
#ifdef DEBUG_RS_WS
	logerror("tms52xx: write data %02x\n", data);
#endif
	if (!m_true_timing)
	{
		// bring up to date first
		m_stream->update();
		data_write(data);
	}
	else
	{
		// actually in a write?
#ifdef DEBUG_RS_WS
		if (!(m_rs_ws == 0x02))
			logerror("tms52xx: write data written outside ws, status: %02x!\n", m_rs_ws);
#endif
		m_write_latch = data;
	}
}

/******************************************************************************
    read -- read status or data from the sound chip
*******************************************************************************/

READ8_MEMBER( tms52xx_device::read )
{
	if (!m_true_timing)
	{
		// bring up-to-date first
		m_stream->update();
		return status_read();
	}
	else
	{
		// actually in a read?
		if (m_rs_ws == 0x01)
			return m_read_latch;
#ifdef DEBUG_RS_WS
		else
			logerror("tms52xx: data read outside rs!\n");
#endif
		return 0xff;
	}
}

/*******************************************************************************
    ready -- return the not ready status from the sound chip
*******************************************************************************/

READ_LINE_MEMBER( tms52xx_device::readyq )
{
	// bring up-to-date first
	m_stream->update();
	return !ready_read();
}


/******************************************************************************
    time_to_ready -- return the time in seconds until the
    ready line is asserted
*******************************************************************************/

double tms52xx_device::time_to_ready()
{
	double cycles;

	// bring up-to-date
	m_stream->update();
	cycles = cycles_to_ready();
	return cycles * 80.0 / m_clock;
}

/******************************************************************************
    intq -- return the interrupt status from the sound chip
******************************************************************************/

READ_LINE_MEMBER( tms52xx_device::intq )
{
	// bring up-to-date first
	m_stream->update();
	return !int_read();
}

void tms52xx_device::register_for_save_states()
{
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_fifo_bits_taken));

	save_item(NAME(m_speaking_now));
	save_item(NAME(m_speak_external));
	save_item(NAME(m_talk_status));
	save_item(NAME(m_buffer_low));
	save_item(NAME(m_buffer_empty));
	save_item(NAME(m_irq_pin));
	save_item(NAME(m_ready_pin));

	save_item(NAME(m_OLDE));
	save_item(NAME(m_OLDP));

	save_item(NAME(m_new_frame_energy_idx));
	save_item(NAME(m_new_frame_pitch_idx));
	save_item(NAME(m_new_frame_k_idx));
#ifdef PERFECT_INTERPOLATION_HACK
	save_item(NAME(m_old_frame_energy_idx));
	save_item(NAME(m_old_frame_pitch_idx));
	save_item(NAME(m_old_frame_k_idx));
#endif
	save_item(NAME(m_current_energy));
	save_item(NAME(m_current_pitch));
	save_item(NAME(m_current_k));

	save_item(NAME(m_target_energy));
	save_item(NAME(m_target_pitch));
	save_item(NAME(m_target_k));

	save_item(NAME(m_previous_energy));

	save_item(NAME(m_subcycle));
	save_item(NAME(m_subc_reload));
	save_item(NAME(m_PC));
	save_item(NAME(m_interp_period));
	save_item(NAME(m_inhibit));
	save_item(NAME(m_tms5220c_rate));
	save_item(NAME(m_pitch_count));

	save_item(NAME(m_u));
	save_item(NAME(m_x));

	save_item(NAME(m_RNG));
	save_item(NAME(m_excitation_data));

	save_item(NAME(m_schedule_dummy_read));
	save_item(NAME(m_data_register));
	save_item(NAME(m_RDB_flag));
	save_item(NAME(m_digital_select));

	save_item(NAME(m_io_ready));
}


const device_type TMS5220N = &device_creator<tms5220n_device>;
const device_type TMS5220CN = &device_creator<tms5220cn_device>;
const device_type TMC0285N = &device_creator<tmc0285n_device>;
const device_type TMS5200N = &device_creator<tms5200n_device>;

