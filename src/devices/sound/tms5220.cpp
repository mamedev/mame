// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Aaron Giles, Jonathan Gevaryahu, Raphael Nabet, Couriersud, Michael Zapf
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
     Updated based on the chip decaps done by digshadow

     Much information regarding the lpc encoding used here comes from US patent 4,209,844
     US patent 4,331,836 describes the complete 51xx chip
     US patent 4,335,277 describes the complete 52xx chip
     Special Thanks to Larry Brantingham for answering questions regarding the chip details

   TMS5200/TMS5220/TMS5220C/CD2501E/CD2501ECD/EFO90503:

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

TODO:
    5110:
    * implement CS
    * TMS5110_CMD_TEST_TALK is only partially implemented
    5220:
    * Samples repeat over and over in the 'eprom' test mode. Needs investigation.
    * Implement a ready callback for pc interfaces
    - this will be quite a challenge since for it to be really accurate
      the whole emulation has to run in sync (lots of timers) with the
      cpu cores.
    * If a command is still executing, /READY will be kept high until the command has
      finished if the next command is written.
    * tomcat has a 5220 which is not hooked up at all

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
    Every PC full count (i.e. overflow from 12 to 0), IP (aka "Interpolation Period")
    is incremented.
* IP (aka "Interpolation Period") ranges from 0 to 7, and corresponds with the amount
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
    Based on the behavior tested on the CD2501ECD this is assumed to be the same for that chip as well.

Most of the following is based on figure 8c of 4,331,836, which is the
  TMS5100/TMC0280 patent, but the same information applies to the TMS52xx
  as well.

OLDP is a status flag which controls whether unvoiced or voiced excitation is
  being generated. It is latched from "P=0" at IP=7 PC=12 T=16.
  (This means that, during normal operation, between IP=7 PC=12 T16 and
  IP=0 PC=1 T17, OLDP and P=0 are the same)
"P=0" is a status flag which is set if the index value for pitch for the new
  frame being parsed (which will become the new target frame) is zero.
  It is used for determining whether interpolation of the next frame is
  inhibited or not. It is updated at IP=0 PC=1 T17. See next section.
OLDE is a status flag which is only used for determining whether
  interpolation is inhibited or not.
  It is latched from "E=0" at IP=7 PC=12 T=16.
  (This means that, during normal operation, between IP=7 PC=12 T16 and
  IP=0 PC=0 T17, OLDE and E=0 are the same)
"E=0" is a status flag which is set if the index value for energy for the new
  frame being parsed (which will become the new target frame) is zero.
  It is used for determining whether interpolation of the next frame is
  inhibited or not. It is updated at IP=0 PC=0 T17. See next section.

Interpolation is inhibited (i.e. interpolation at IP frames will not happen
  except for IP=0) under the following circumstances:
  "P=0" != "OLDP" ("P=0" = 1, and OLDP = 0; OR "P=0" = 0, and OLDP = 1)
    This means the new frame is unvoiced and the old one was voiced, or vice
    versa.
* TODO the 5100 and 5200 patents are inconsistent about the above. Trace the decaps!
  "OLDE" = 1 and "E=0" = 0
    This means the new frame is not silent, and the old frame was silent.



****Documentation of chip commands:***
    76543210  (these are in logical 7 thru 0 order with MSB (7) first; TI calls
              these bits by the opposite order, D0 thru D7, on the datasheet,
              with D0 being the MSB)

    x0x0xbcc: on 5200/5220: NOP (does nothing)
              on 5220C and CD2501ECD: Select frame length by cc, and b selects
              whether every frame is preceded by 2 bits to select the frame
              length (instead of using the value set by cc); the default (and
              after a reset command) is as if '0x00' was written, i.e. for
              frame length (200 samples) and 0 for whether the preceding 2
              bits are enabled (off)

    x001xxxx: READ BYTE (RDBY)
              Sends eight read bit commands (M0 high M1 low) to VSM and reads
              the resulting bits serially into a temporary register, which
              becomes readable as the next byte read from the tms52xx once
              ready goes active. Note the bit order of the byte read from the
              TMS52xx is BACKWARDS as compared to the actual data order as in
              the rom on the VSM chips; the read byte command of the tms5100
              reads the bits in the 'correct' order. This was IMHO a rather
              silly design decision of TI. (I (LN) asked Larry Brantingham
              about this but he wasn't involved with the TMS52xx chips, just
              the 5100); There's ASCII data in the TI 99/4 speech module VSMs
              which has the bit order reversed on purpose because of this!
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x011xxxx: READ AND BRANCH (RB)
              Sends a read and branch command (M0 high, M1 high) to force VSM
              to set its data pointer to whatever the data is at its current
              pointer location is)
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x100aaaa: LOAD ADDRESS (LA)
              Send a load address command (M0 low M1 high) to VSM with the 4
              'a' bits; Note you need to send four or five of these in
              sequence to actually specify an address to the vsm.
    TALK STATUS must be CLEAR for this command to work; otherwise it is treated as a NOP.

    x101xxxx: SPEAK (SPK)
              Begins speaking, pulling speech data from the current address
              pointer location of the VSM modules.

    x110xxxx: SPEAK EXTERNAL (SPKEXT)
              Clears the FIFO using SPKEE line, then sets TALKD (TALKST
              remains zero) until 8 bytes have been written to the FIFO, at
              which point it begins speaking, pulling data from the 16 byte
              FIFO.
    The patent implies TALK STATUS must be CLEAR for this command to work;
    otherwise it is treated as a NOP, but the decap shows that this is not
    true, and is an error on the patent diagram.

    x111xxxx: RESET (RST)
              Resets the speech synthesis core immediately, and clears the FIFO.


    Other chip differences:
    The 5220C (and CD2501ECD maybe?) are quieter due to a better dac arrangement on die which allows less crossover between bits, based on the decap differences.


***MAME Driver specific notes:***

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

TMS5200 AKA TMC0285 AKA CD2501E: (1980 to 1983)
    Arcade: Zaccaria's 'money money' and 'jack rabbit'; Bally/Midway's
'Discs of Tron' (all environmental cabs and a few upright cabs; the code
exists on all versions for the speech though, and upright cabs can be
upgraded to add it by hacking on a 'Squawk & Talk' pinball speech board
(which is also TMS5200 based) with a few modded components)
    Pinball: All Bally/Midway machines which uses the 'Squawk & Talk' board.
    Home computer: TI 99/4 PHP1500 Speech module (along with two VSM
serial chips); Street Electronics Corp.'s Apple II 'Echo 2' Speech
synthesizer (early cards only)

EFO90503: (1982, EFO Sound-3 board used in a few Playmatic/Cidelsa games)
    Arcade: Clean Octopus
    Pinball: Cerberus, Spain '82

CD2501ECD: (1983)
    Home computer: TI 99/8 (prototypes only)

TMS5220: (mostly on things made between 1981 and 1984-1985)
    Arcade: Bally/Midway's 'NFL Football'; Atari's 'Star Wars',
'Firefox', 'Return of the Jedi', 'Road Runner', 'The Empire Strikes
Back' (all verified with schematics); Venture Line's 'Looping' and 'Sky
Bumper' (need verify for both); Olympia's 'Portraits' (need verify);
Exidy's 'Victory' and 'Victor Banana' (need verify for both)
    Pinball: Several (don't know names offhand, have not checked schematics; likely Zaccaria's 'Farfalla')
    Home computer: Street Electronics Corp.'s Apple II 'Echo 2' Speech
synthesizer (later cards only); Texas Instruments' 'Speak and Learn'
scanner wand unit; HP 27201A Speech Output Module (serial port connection).

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

Street electronics had two later 1988-1990-era ECHO appleII cards which are
TSP50c0x/1x MCU based speech and not tms52xx based (though it is likely
emulating the tms5220 in MCU code). Look for a 16-pin chip at U6 labeled
"ECHO-2 SN" or "ECHO-3 SN".

***********************************************************************************************/

#include "emu.h"
#include "tms5220.h"

/* *****optional defines***** */

/* Hacky improvements which don't match patent: */
/* Interpolation shift logic:
 * One of the following two lines should be used, and the other commented
 * The second line is more accurate mathematically but not accurate to the patent
 */
#define INTERP_SHIFT >> m_coeff->interp_coeff[m_IP]
//define INTERP_SHIFT / (1<<m_coeff->interp_coeff[m_IP])

/* Other hacks */
/* HACK: if defined, outputs the low 4 bits of the lattice filter to the i/o
 * or clip logic, even though the real hardware doesn't do this, partially verified by decap */
#undef ALLOW_4_LSB

/* forces m_TALK active instantly whenever m_SPEN would be activated, causing speech delay to be reduced by up to one frame time */
/* for some reason, this hack makes victory behave better, though it does not match the patent */
#define FAST_START_HACK 1


/* *****configuration of chip connection stuff***** */
/* must be defined; if 0, output the waveform as if it was tapped on the
   speaker pin as usual, if 1, output the waveform as if it was tapped on the
   i/o pin (volume is much lower in the latter case) */
#define FORCE_DIGITAL 0

/* 5220 only; must be defined; if 1, normal speech (one A cycle, one B cycle
   per interpolation step); if 0; speak as if SPKSLOW was used (two A cycles,
   one B cycle per interpolation step) */
#define FORCE_SUBC_RELOAD 1

/* *****debugging defines***** */
/* 5220 only; above dumps the data written to the tms52xx to the error log, useful
   for making logged data dumps for real hardware tests */
#define LOG_DUMP_INPUT_DATA (1U << 1)

// 5220 only; above debugs FIFO stuff: writes, reads and flag updates
#define LOG_FIFO (1U << 2)

// Show parsing events
#define LOG_PARSE (1U << 3)

// dumps each speech frame as binary
#define LOG_PARSE_FRAME_DUMP_BIN (1U << 4)

// dumps each speech frame as hex
#define LOG_PARSE_FRAME_DUMP_HEX (1U << 5)

// dumps info if a frame ran out of data
#define LOG_FRAME_ERRORS (1U << 6)

// dumps all non-speech-data command writes
#define LOG_COMMAND (1U << 7)

// Additional information about command processing
#define LOG_COMMAND_VERBOSE (1U << 8)

// Show NOP command (tends to be noisy)
#define LOG_COMMAND_NOP (1U << 9)

// spams the error log with I/O ready messages whenever the ready or IRQ pin is read
#define LOG_PIN_READS (1U << 10)

// dumps debug information related to the sample generation loop, i.e. whether interpolation is inhibited or not, and what the current and target values for each frame are.
#define LOG_GENERATION (1U << 11)

// dumps MUCH MORE debug information related to the sample generation loop, namely the excitation, energy, pitch, k*, and output values for EVERY SINGLE SAMPLE during a frame.
#define LOG_GENERATION_VERBOSE (1U << 12)

// dumps the lattice filter state data each sample.
#define LOG_LATTICE (1U << 13)

// Show errors in matrix multiplication
#define LOG_MULTIPLY (1U << 14)

// dumps info to the error log whenever the analog clip hardware is (or would be) clipping the signal.
#define LOG_CLIP (1U << 15)

// debugs the io ready callback timer
#define LOG_IO_READY (1U << 16)

// debugs the tms5220_data_r and data_w access methods which actually respect rs and ws
#define LOG_RS_WS (1U << 17)

// Show the byte being written by the CPU to the VSP
#define LOG_DATA_W (1U << 18)

// Show interactions with speech rom
#define LOG_VSM (1U << 19)

// Show state changes
#define LOG_STATE (1U << 20)

//#define VERBOSE (LOG_FIFO | LOG_PARSE | LOG_FRAME_ERRORS | LOG_COMMAND | LOG_GENERATION | LOG_DATA_W | LOG_STATE)
#include "logmacro.h"

#define MAX_SAMPLE_CHUNK    512

/* 6+4 Variants, from tms5110r.inc */

#define TMS5220_IS_TMC0281  (1)
#define TMS5220_IS_TMC0281D (2)
#define TMS5220_IS_CD2801   (3)
#define TMS5220_IS_CD2802   (4)
#define TMS5220_IS_TMS5110A (5)
#define TMS5220_IS_M58817   (6)
#define TMS5220_IS_5220C    (7)
#define TMS5220_IS_5200     (8)
#define TMS5220_IS_5220     (9)
#define TMS5220_IS_CD2501ECD (10)

#define TMS5220_IS_CD2501E  TMS5220_IS_5200

// 52xx: decide whether we have rate control or not
#define TMS5220_HAS_RATE_CONTROL ((m_variant == TMS5220_IS_5220C) || (m_variant == TMS5220_IS_CD2501ECD))

// All: decide whether we are a 51xx or a 52xx
#define TMS5220_IS_52xx ((m_variant == TMS5220_IS_5220C) || (m_variant == TMS5220_IS_5200) || (m_variant == TMS5220_IS_5220) || (m_variant == TMS5220_IS_CD2501ECD))

/* 51xx: States for CTL */
// ctl bus is input to tms51xx
#define CTL_STATE_INPUT               (0)
// ctl bus is outputting a test talk command on CTL1(bit 0)
#define CTL_STATE_TTALK_OUTPUT        (1)
// ctl bus is switching direction, next will be above
#define CTL_STATE_NEXT_TTALK_OUTPUT   (2)
// ctl bus is outputting a read nybble 'output' command on CTL1,2,4,8 (bits 0-3)
#define CTL_STATE_OUTPUT              (3)
// ctl bus is switching direction, next will be above
#define CTL_STATE_NEXT_OUTPUT         (4)

static const uint8_t reload_table[4] = { 0, 2, 4, 6 }; //sample count reload for 5220c and cd2501ecd only; 5200 and 5220 always reload with 0; keep in mind this is loaded on IP=0 PC=12 subcycle=1 so it immediately will increment after one sample, effectively being 1,3,5,7 as in the comments above.

#define NOCOMMAND 0xff

// Pull in the ROM tables
#include "tms5110r.hxx"


void tms5220_device::register_for_save_states()
{
	// for sanity purposes these variables should be in the same order as in tms5220.h!

	// 5110 specific stuff
	save_item(NAME(m_PDC));
	save_item(NAME(m_CTL_pins));
	save_item(NAME(m_state));

	// new VSM stuff
	save_item(NAME(m_address));
	save_item(NAME(m_next_is_address));
	save_item(NAME(m_schedule_dummy_read));
	save_item(NAME(m_addr_bit));
	save_item(NAME(m_CTL_buffer));

	// old VSM stuff
	save_item(NAME(m_read_byte_register));
	save_item(NAME(m_RDB_flag));

	// FIFO
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_fifo_bits_taken));

	// global status bits (booleans)
	save_item(NAME(m_previous_talk_status));
	save_item(NAME(m_SPEN));
	save_item(NAME(m_DDIS));
	save_item(NAME(m_TALK));
	save_item(NAME(m_TALKD));
	save_item(NAME(m_buffer_low));
	save_item(NAME(m_buffer_empty));
	save_item(NAME(m_irq_pin));
	save_item(NAME(m_ready_pin));

	save_item(NAME(m_command_register));
	save_item(NAME(m_data_latched));

	// current and previous frames
	save_item(NAME(m_OLDE));
	save_item(NAME(m_OLDP));

	save_item(NAME(m_new_frame_energy_idx));
	save_item(NAME(m_new_frame_pitch_idx));
	save_item(NAME(m_new_frame_k_idx));
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
	save_item(NAME(m_old_frame_energy_idx));
	save_item(NAME(m_old_frame_pitch_idx));
	save_item(NAME(m_old_frame_k_idx));
	save_item(NAME(m_old_zpar));
	save_item(NAME(m_old_uv_zpar));
#endif
	save_item(NAME(m_current_energy));
	save_item(NAME(m_current_pitch));
	save_item(NAME(m_current_k));

	save_item(NAME(m_previous_energy));

	save_item(NAME(m_subcycle));
	save_item(NAME(m_subc_reload));
	save_item(NAME(m_PC));
	save_item(NAME(m_IP));
	save_item(NAME(m_inhibit));
	save_item(NAME(m_uv_zpar));
	save_item(NAME(m_zpar));
	save_item(NAME(m_pitch_zero));
	save_item(NAME(m_c_variant_rate));
	save_item(NAME(m_pitch_count));

	save_item(NAME(m_u));
	save_item(NAME(m_x));

	save_item(NAME(m_RNG));
	save_item(NAME(m_excitation_data));

	save_item(NAME(m_digital_select));

	save_item(NAME(m_io_ready));

	// "proper" rs+ws emulation
	save_item(NAME(m_true_timing));

	save_item(NAME(m_rs_ws));
	save_item(NAME(m_read_latch));
	save_item(NAME(m_write_latch));
}


/**********************************************************************************************

      printbits helper function: takes a long int input and prints the resulting bits to stderr

***********************************************************************************************/

void tms5220_device::printbits(long data, int num)
{
	for (int i = num - 1; i >= 0; i--)
		LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN, "%0ld", (data>>i)&1);

	switch ((num - 1) & 0xfc)
	{
	case 0:
		LOGMASKED(LOG_PARSE_FRAME_DUMP_HEX, "%0lx", data);
		break;
	case 4:
		LOGMASKED(LOG_PARSE_FRAME_DUMP_HEX, "%02lx", data);
		break;
	case 8:
		LOGMASKED(LOG_PARSE_FRAME_DUMP_HEX, "%03lx", data);
		break;
	case 12:
		LOGMASKED(LOG_PARSE_FRAME_DUMP_HEX, "%04lx", data);
		break;
	default:
		LOGMASKED(LOG_PARSE_FRAME_DUMP_HEX, "%04lx", data);
		break;
	}
}

/**********************************************************************************************

    tms5220_device::vsm_write -- wrap a write sent to the VSM

***********************************************************************************************/
void tms5220_device::vsm_write(uint8_t rc, uint8_t m0, uint8_t m1, uint8_t addr)
{
	m_m0_cb(m0);
	m_m1_cb(m1);
	m_addr_cb(offs_t(0), addr);
	if (!m_romclk_cb.isunset())
	{
		//printf("rc %d\n", rc);
		m_romclk_cb(rc);
	}
}

/**********************************************************************************************

    tms5220_device::vsm_write_addr -- wrap a 'load address' set of writes sent to the VSM

***********************************************************************************************/
void tms5220_device::vsm_write_addr(uint8_t addr)
{
	vsm_write(1, 0, 1, addr); // romclk 1, m0 0, m1 1, addr bus nybble = xxxx
	vsm_write(0, 0, 1, addr); // romclk 0, m0 0, m1 1, addr bus nybble = xxxx
	vsm_write(1, 0, 0, addr); // romclk 1, m0 0, m1 0, addr bus nybble = xxxx
	vsm_write(0, 0, 0, addr); // romclk 0, m0 0, m1 0, addr bus nybble = xxxx
}

/**********************************************************************************************

    tms5220_device::vsm_read -- wrap a 'read bit' set of writes sent to the VSM

***********************************************************************************************/
uint8_t tms5220_device::vsm_read()
{
	vsm_write(1, 1, 0, 0); // romclk 1, m0 1, m1 0, addr bus nybble = 0/open bus
	vsm_write(0, 1, 0, 0); // romclk 0, m0 1, m1 0, addr bus nybble = 0/open bus
	vsm_write(1, 0, 0, 0); // romclk 1, m0 0, m1 0, addr bus nybble = 0/open bus
	vsm_write(0, 0, 0, 0); // romclk 0, m0 0, m1 0, addr bus nybble = 0/open bus

	uint8_t val = 0;

	if (!m_data_cb.isunset())
		val = m_data_cb();
	else
		LOGMASKED(LOG_VSM, "Warning: No speech memory attached, returning 0.\n");

	return val;
}

/**********************************************************************************************

    tms5220_device::vsm_read_and_branch -- wrap a 'read-and-branch' set of writes sent to the VSM

***********************************************************************************************/
void tms5220_device::vsm_read_and_branch()
{
	vsm_write(0, 1, 1, 0); // see tms5110.cpp
	vsm_write(1, 1, 1, 0); // romclk 1, m0 1, m1 1, addr bus nybble = 0/open bus
	vsm_write(0, 1, 1, 0); // romclk 0, m0 1, m1 1, addr bus nybble = 0/open bus
	vsm_write(0, 0, 0, 0); // see tms5110.cpp
	vsm_write(1, 0, 0, 0); // romclk 1, m0 0, m1 0, addr bus nybble = 0/open bus
	vsm_write(0, 0, 0, 0); // romclk 0, m0 0, m1 0, addr bus nybble = 0/open bus
}

/**********************************************************************************************

     tms5220_device::data_write -- handle a write to the TMS5220

***********************************************************************************************/

void tms5220_device::data_write(int data)
{
	bool old_buffer_low = m_buffer_low;
	LOGMASKED(LOG_DUMP_INPUT_DATA, "%c", data);

	if (m_DDIS) // If we're in speak external mode
	{
		// add this byte to the FIFO
		if (m_fifo_count < FIFO_SIZE)
		{
			m_fifo[m_fifo_tail] = data;
			m_fifo_tail = (m_fifo_tail + 1) % FIFO_SIZE;
			m_fifo_count++;
			LOGMASKED(LOG_FIFO, "data_write: Added %02X byte to FIFO (current count=%2d)\n", data, m_fifo_count);
			update_fifo_status_and_ints();

			// if we just unset buffer low with that last write, and SPEN *was* zero (see circuit 251, sheet 12)
			if ((!m_SPEN) && (old_buffer_low && (!m_buffer_low))) // MUST HAVE EDGE DETECT
			{
				LOGMASKED(LOG_FIFO, "data_write triggered SPEN to go active!\n");
				// ...then we now have enough bytes to start talking; set zpar and clear out the new frame parameters (it will become old frame just before the first call to parse_frame() )
				m_zpar = true;
				m_uv_zpar = true; // zero k4-k10 as well
				m_OLDE = true; // 'silence/zpar' frames are zero energy
				m_OLDP = true; // 'silence/zpar' frames are zero pitch
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
				m_old_zpar = true; // zero all the old parameters
				m_old_uv_zpar = true; // zero old k4-k10 as well
#endif
				m_SPEN = true;
#ifdef FAST_START_HACK
				m_TALK = true;
#endif
				m_new_frame_energy_idx = 0;
				m_new_frame_pitch_idx = 0;
				for (int i = 0; i < 4; i++)
					m_new_frame_k_idx[i] = 0;
				for (int i = 4; i < 7; i++)
					m_new_frame_k_idx[i] = 0xF;
				for (int i = 7; i < m_coeff->num_k; i++)
					m_new_frame_k_idx[i] = 0x7;

			}
		}
		else
		{
			LOGMASKED(LOG_FIFO, "data_write: Ran out of room in the tms52xx FIFO! this should never happen!\n");
			// at this point, /READY should remain HIGH/inactive until the FIFO has at least one byte open in it.
		}
		m_data_latched = false;
	}
	else //(! m_DDIS)
		// R Nabet : we parse commands at once.  It is necessary for such commands as read.
		process_command(data);

	if (!m_data_latched)
		m_io_ready = true;
}

/**********************************************************************************************

     update_fifo_status_and_ints -- check to see if the various flags should be on or off
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

void tms5220_device::update_fifo_status_and_ints()
{
	/* update 52xx FIFO flags and set ints if needed */
	if (!TMS5220_IS_52xx) return; // bail out if not a 52xx chip
	update_ready_state();

	/* BL is set if neither byte 9 nor 8 of the FIFO are in use; this
	translates to having fifo_count (which ranges from 0 bytes in use to 16
	bytes used) being less than or equal to 8. Victory/Victorba depends on this. */
	if (m_fifo_count <= 8)
	{
		// generate an interrupt if necessary; if /BL was inactive and is now active, set int.
		if (!m_buffer_low)
		{
			m_buffer_low = true;
			set_interrupt_state(1);
		}
	}
	else
		m_buffer_low = false;

	/* BE is set if neither byte 15 nor 14 of the FIFO are in use; this
	translates to having fifo_count equal to exactly 0
	*/
	if (m_fifo_count == 0)
	{
		// generate an interrupt if necessary; if /BE was inactive and is now active, set int.
		if (!m_buffer_empty)
		{
			m_buffer_empty = true;
			set_interrupt_state(1);
		}
		if (m_DDIS)
			m_TALK = m_SPEN = false; // /BE being active clears the TALK status via TCON, which in turn clears SPEN, but ONLY if m_DDIS is set! See patent page 16, gate 232b
	}
	else
		m_buffer_empty = false;

	// generate an interrupt if /TS was active, and is now inactive.
	// also, in this case, regardless if DDIS was set, unset it.
	if (m_previous_talk_status && !talk_status())
	{
		LOGMASKED(LOG_STATE, "Talk status 1 -> 0, unsetting DDIS and firing an interrupt.\n");
		set_interrupt_state(1);
		m_DDIS = false;

		m_previous_talk_status = false;

		// Is there a command stuck in the command register due to speech output?
		// Then resume it.
		if (m_command_register != NOCOMMAND)
		{
			LOGMASKED(LOG_STATE, "Resume command execution: %02X\n", m_command_register);
			process_command(m_command_register);

			// Is there another data transfer pending? Resume it as well.
			if (m_data_latched)
			{
				LOGMASKED(LOG_STATE, "Pending byte write: %02X\n", m_write_latch);
				m_timer_io_ready->adjust(clocks_to_attotime(16), 1);
			}
			else
				m_io_ready = true;

			update_ready_state();
		}
	}
	m_previous_talk_status = talk_status();

	// update continuously when RSQ is held active low
	if (m_rs_ws == 0x01 && !m_RDB_flag)
		m_read_latch = status_read(false);
}

/**********************************************************************************************

     read_bits -- read a specific number of bits from the current input stream (FIFO or VSM)

***********************************************************************************************/

int tms5220_device::read_bits(int count)
{
	int val = 0;

	if (m_DDIS)
	{
		// extract from FIFO
		while (count--)
		{
			val = (val << 1) | ((m_fifo[m_fifo_head] >> m_fifo_bits_taken) & 1);
			m_fifo_bits_taken++;
			if (m_fifo_bits_taken >= 8)
			{
				m_fifo_count--;
				m_fifo[m_fifo_head] = 0; // zero the newly depleted FIFO head byte
				m_fifo_head = (m_fifo_head + 1) % FIFO_SIZE;
				m_fifo_bits_taken = 0;
				update_fifo_status_and_ints();
			}
		}
	}
	else
	{
		if (!m_m0_cb.isunset())
		{
			while (count--)
			{
				val = (val << 1) | vsm_read();
				LOGMASKED(LOG_VSM, "bit read: %d\n", val&1);
			}
		}
		else
		{
			// assume the input floats high if nothing is connected, so a
			// spurious speak vsm command will eventually return a 0xF (STOP)
			// frame which will halt speech
			val = (1<<count)-1;
		}
	}
	return val;
}

void tms5220_device::perform_dummy_read()
{
	m_schedule_dummy_read = false;
	if (!m_m0_cb.isunset())
	{
		LOGMASKED(LOG_VSM, "TMS52xx performing dummy read\n");
		vsm_write_addr(0);
		vsm_read();
	}
}

/**********************************************************************************************

     tms5220_status_read -- read status or data from the TMS5220; if the bool is 1, clear interrupt
     state.

***********************************************************************************************/

uint8_t tms5220_device::status_read(bool clear_int)
{
	if (m_RDB_flag)
	{   /* if last command was read, return data register */
		m_RDB_flag = false;
		return(m_read_byte_register);
	}
	else
	{   /* read status */
		/* clear the interrupt pin on status read */
		if (clear_int)
			set_interrupt_state(0);
		LOGMASKED(LOG_PIN_READS, "Status read: TS=%d BL=%d BE=%d\n", talk_status(), m_buffer_low, m_buffer_empty);
		return (talk_status() << 7) | (m_buffer_low << 6) | (m_buffer_empty << 5);// | (m_write_latch & 0x1f); // low 5 bits are open bus, so use the m_write_latch value.
	}
}


/**********************************************************************************************

     tms5220_ready_read -- returns the ready state of the TMS5220

***********************************************************************************************/

bool tms5220_device::ready_read()
{
	LOGMASKED(LOG_PIN_READS, "ready_read: ready pin read, io_ready is %d, FIFO count is %d, DDIS(speak external) is %d\n", m_io_ready, m_fifo_count, m_DDIS);
	/* if m_true_timing is NOT set (we're in 'hacky instant write mode'), the
	   m_timer_io_ready timer doesn't run and will never de-assert m_io_ready
	   if the FIFO is full, so we need to explicitly check for FIFO full here
	   and return the proper value.

	   SEVERE CAVEAT: This makes the assumption that the ready_read was after
	   wsq was 'virtually asserted', so if the FIFO has no room in it ready
	   will always return inactive, even if no write happened! i.e., after a
	   read command when the FIFO was exactly filled, but no write attempted
	   to overfill it. This behavior is inaccurate to hardware and may cause
	   issues! You have been warned!
	*/
	if (!m_true_timing)
		return ((m_fifo_count < FIFO_SIZE)||(!m_DDIS)) && m_io_ready;
	else
		return m_io_ready;
}


/**********************************************************************************************

     tms5220_int_read -- returns the interrupt state of the TMS5220

***********************************************************************************************/

bool tms5220_device::int_read()
{
	LOGMASKED(LOG_PIN_READS, "int_read: irq pin read, state is %d\n", m_irq_pin);

	return m_irq_pin;
}


/**********************************************************************************************

     tms5220_process -- fill the buffer with a specific number of samples

***********************************************************************************************/

void tms5220_device::process(int16_t *buffer, unsigned int size)
{
	int buf_count = 0;
	int i, bitout;
	int32_t this_sample;

	LOGMASKED(LOG_GENERATION, "process called with size of %d; IP=%d, PC=%d, subcycle=%d, m_SPEN=%d, m_TALK=%d, m_TALKD=%d\n", size, m_IP, m_PC, m_subcycle, m_SPEN, m_TALK, m_TALKD);

	/* loop until the buffer is full or we've stopped speaking */
	while (size > 0)
	{
		if(m_TALKD) // speaking
		{
			/* if we're ready for a new frame to be applied, i.e. when IP=0, PC=12, Sub=1
			* (In reality, the frame was really loaded incrementally during the entire IP=0
			* PC=x time period, but it doesn't affect anything until IP=0 PC=12 happens)
			*/
			if ((m_IP == 0) && (m_PC == 12) && (m_subcycle == 1))
			{
				// HACK for regression testing, be sure to comment out before release!
				//m_RNG = 0x1234;
				// end HACK

				/* appropriately override the interp count if needed; this will be incremented after the frame parse! */
				m_IP = reload_table[m_c_variant_rate&0x3];

#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
				/* remember previous frame energy, pitch, and coefficients */
				m_old_frame_energy_idx = m_new_frame_energy_idx;
				m_old_frame_pitch_idx = m_new_frame_pitch_idx;
				for (i = 0; i < m_coeff->num_k; i++)
					m_old_frame_k_idx[i] = m_new_frame_k_idx[i];
#endif

				/* Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[] */
				parse_frame();

				/* if the new frame is a stop frame, unset both TALK and SPEN (via TCON). TALKD remains active while the energy is ramping to 0. */
				if (new_frame_stop_flag())
				{
					m_TALK = m_SPEN = false;
					update_fifo_status_and_ints(); // probably not necessary...
				}

				/* in all cases where interpolation would be inhibited, set the inhibit flag; otherwise clear it.
				 * Interpolation inhibit cases:
				 * Old frame was voiced, new is unvoiced
				 * Old frame was silence/zero energy, new has non-zero energy
				 * Old frame was unvoiced, new is voiced
				 * Old frame was unvoiced, new frame is silence/zero energy (non-existent on tms51xx rev D and F (present and working on tms52xx, present but buggy on tms51xx rev A and B))
				 */
				if ( (!old_frame_unvoiced_flag() && new_frame_unvoiced_flag())
					|| (old_frame_unvoiced_flag() && !new_frame_unvoiced_flag())
					|| (old_frame_silence_flag() && !new_frame_silence_flag())
					//|| (m_inhibit && old_frame_unvoiced_flag() && new_frame_silence_flag()) ) //TMS51xx INTERP BUG1
					|| (old_frame_unvoiced_flag() && new_frame_silence_flag()) )
					m_inhibit = true;
				else // normal frame, normal interpolation
					m_inhibit = false;

				/* Debug info for current parsed frame */
				LOGMASKED(LOG_GENERATION, "OLDE: %d; NEWE: %d; OLDP: %d; NEWP: %d\n", old_frame_silence_flag(), new_frame_silence_flag(), old_frame_unvoiced_flag(), new_frame_unvoiced_flag());
				LOGMASKED(LOG_GENERATION, "Processing new frame: %s\n", !m_inhibit ? "Normal Frame" : "Interpolation Inhibited");
				LOGMASKED(LOG_GENERATION, "*** current Energy, Pitch and Ks =      %04d,   %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",m_current_energy, m_current_pitch, m_current_k[0], m_current_k[1], m_current_k[2], m_current_k[3], m_current_k[4], m_current_k[5], m_current_k[6], m_current_k[7], m_current_k[8], m_current_k[9]);
				LOGMASKED(LOG_GENERATION, "*** target Energy(idx), Pitch, and Ks = %04d(%x),%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",
					(m_coeff->energytable[m_new_frame_energy_idx] * (1-m_zpar)),
					m_new_frame_energy_idx,
					(m_coeff->pitchtable[m_new_frame_pitch_idx] * (1-m_zpar)),
					(m_coeff->ktable[0][m_new_frame_k_idx[0]] * (1-m_zpar)),
					(m_coeff->ktable[1][m_new_frame_k_idx[1]] * (1-m_zpar)),
					(m_coeff->ktable[2][m_new_frame_k_idx[2]] * (1-m_zpar)),
					(m_coeff->ktable[3][m_new_frame_k_idx[3]] * (1-m_zpar)),
					(m_coeff->ktable[4][m_new_frame_k_idx[4]] * (1-m_uv_zpar)),
					(m_coeff->ktable[5][m_new_frame_k_idx[5]] * (1-m_uv_zpar)),
					(m_coeff->ktable[6][m_new_frame_k_idx[6]] * (1-m_uv_zpar)),
					(m_coeff->ktable[7][m_new_frame_k_idx[7]] * (1-m_uv_zpar)),
					(m_coeff->ktable[8][m_new_frame_k_idx[8]] * (1-m_uv_zpar)),
					(m_coeff->ktable[9][m_new_frame_k_idx[9]] * (1-m_uv_zpar)) );
			}
			else // Not a new frame, just interpolate the existing frame.
			{
				bool inhibit_state = (m_inhibit && (m_IP != 0)); // disable inhibit when reaching the last interp period, but don't overwrite the m_inhibit value
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
				int samples_per_frame = m_subc_reload?175:266; // either (13 A cycles + 12 B cycles) * 7 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 7 interps for SPKSLOW
				//int samples_per_frame = m_subc_reload?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
				int current_sample = (m_subcycle - m_subc_reload)+(m_PC*(3-m_subc_reload))+((m_subc_reload?25:38)*((m_IP-1)&7));
				//logerror( "CS: %03d", current_sample);
				// reset the current energy, pitch, etc to what it was at frame start
				m_current_energy = (m_coeff->energytable[m_old_frame_energy_idx] * (1-m_old_zpar));
				m_current_pitch = (m_coeff->pitchtable[m_old_frame_pitch_idx] * (1-m_old_zpar));
				for (i = 0; i < m_coeff->num_k; i++)
					m_current_k[i] = (m_coeff->ktable[i][m_old_frame_k_idx[i]] * (1-((i<4)?m_old_zpar:m_old_uv_zpar)));
				// now adjust each value to be exactly correct for each of the samples per frame
				if (m_IP != 0) // if we're still interpolating...
				{
					m_current_energy = (m_current_energy + (((m_coeff->energytable[m_new_frame_energy_idx] - m_current_energy)*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-m_zpar);
					m_current_pitch = (m_current_pitch + (((m_coeff->pitchtable[m_new_frame_pitch_idx] - m_current_pitch)*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-m_zpar);
					for (i = 0; i < m_coeff->num_k; i++)
						m_current_k[i] = (m_current_k[i] + (((m_coeff->ktable[i][m_new_frame_k_idx[i]] - m_current_k[i])*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-((i<4)?m_zpar:m_uv_zpar));
				}
				else // we're done, play this frame for 1/8 frame.
				{
					if (m_subcycle == 2) m_pitch_zero = false; // this reset happens around the second subcycle during IP=0
					m_current_energy = (m_coeff->energytable[m_new_frame_energy_idx] * (1-m_zpar));
					m_current_pitch = (m_coeff->pitchtable[m_new_frame_pitch_idx] * (1-m_zpar));
					for (i = 0; i < m_coeff->num_k; i++)
						m_current_k[i] = (m_coeff->ktable[i][m_new_frame_k_idx[i]] * (1-((i<4)?m_zpar:m_uv_zpar)));
				}
#else
				//Updates to parameters only happen on subcycle '2' (B cycle) of PCs.
				if (m_subcycle == 2)
				{
					switch(m_PC)
					{
						case 0: /* PC = 0, B cycle, write updated energy */
						if (m_IP==0) m_pitch_zero = 0; // this reset happens around the second subcycle during IP=0
						m_current_energy = (m_current_energy + (((m_coeff->energytable[m_new_frame_energy_idx] - m_current_energy)*(1-inhibit_state)) INTERP_SHIFT))*(1-m_zpar);
						break;
						case 1: /* PC = 1, B cycle, write updated pitch */
						m_current_pitch = (m_current_pitch + (((m_coeff->pitchtable[m_new_frame_pitch_idx] - m_current_pitch)*(1-inhibit_state)) INTERP_SHIFT))*(1-m_zpar);
						break;
						case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
						/* PC = 2 through 11, B cycle, write updated K1 through K10 */
						m_current_k[m_PC-2] = (m_current_k[m_PC-2] + (((m_coeff->ktable[m_PC-2][m_new_frame_k_idx[m_PC-2]] - m_current_k[m_PC-2])*(1-inhibit_state)) INTERP_SHIFT))*(1-(((m_PC-2)<4)?m_zpar:m_uv_zpar));
						break;
						case 12: /* PC = 12 */
						/* we should NEVER reach this point, PC=12 doesn't have a subcycle 2 */
						break;
					}
				}
#endif
			}

			// calculate the output
			if (old_frame_unvoiced_flag())
			{
				// generate unvoiced samples here
				if (m_RNG & 1)
					m_excitation_data = ~0x3F; /* according to the patent it is (either + or -) half of the maximum value in the chirp table, so either 01000000(0x40) or 11000000(0xC0)*/
				else
					m_excitation_data = 0x40;
			}
			else /* (!old_frame_unvoiced_flag()) */
			{
				// generate voiced samples here
				/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
				 * function has a chirp/peak and then a long chain of zeroes.
				 * The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
				 * and if the address reaches that point the ADDRESS incrementer is
				 * disabled, forcing all samples beyond 51d to be == 51d
				 */
				if (m_pitch_count >= 51)
					m_excitation_data = (int8_t)m_coeff->chirptable[51];
				else /*m_pitch_count < 51*/
					m_excitation_data = (int8_t)m_coeff->chirptable[m_pitch_count];
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
			this_sample = lattice_filter(); /* execute lattice filter */

			//LOGMASKED(LOG_GENERATION_VERBOSE, "C:%01d; ",m_subcycle);
			LOGMASKED(LOG_GENERATION_VERBOSE, "IP:%01d PC:%02d X:%04d E:%03d P:%03d Pc:%03d ",m_IP, m_PC, m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
			//LOGMASKED(LOG_GENERATION_VERBOSE, "X:%04d E:%03d P:%03d Pc:%03d ", m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
			for (i=0; i<10; i++)
				LOGMASKED(LOG_GENERATION_VERBOSE, "K%d:%04d ", i+1, m_current_k[i]);
			LOGMASKED(LOG_GENERATION_VERBOSE, "Out:%06d ", this_sample);
//#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
//          LOGMASKED(LOG_GENERATION_VERBOSE, "%d%d%d%d",m_old_zpar,m_zpar,m_old_uv_zpar,m_uv_zpar);
//#else
//          LOGMASKED(LOG_GENERATION_VERBOSE, "x%dx%d",m_zpar,m_uv_zpar);
//#endif
			LOGMASKED(LOG_GENERATION_VERBOSE, "\n");

			/* next, force result to 14 bits (since its possible that the addition at the final (k1) stage of the lattice overflowed) */
			while (this_sample > 16383) this_sample -= 32768;
			while (this_sample < -16384) this_sample += 32768;
			if (m_digital_select == 0) // analog SPK pin output is only 8 bits, with clipping
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
			if ((m_subcycle == 2) && (m_PC == 12)) // RESETF3
			{
				/* Circuit 412 in the patent acts a reset, resetting the pitch counter to 0
				 * if INHIBIT was true during the most recent frame transition.
				 * The exact time this occurs is betwen IP=7, PC=12 sub=0, T=t12
				 * and m_IP = 0, PC=0 sub=0, T=t12, a period of exactly 20 cycles,
				 * which overlaps the time OLDE and OLDP are updated at IP=7 PC=12 T17
				 * (and hence INHIBIT itself 2 t-cycles later).
				 * According to testing the pitch zeroing lasts approximately 2 samples.
				 * We set the zeroing latch here, and unset it on PC=1 in the generator.
				 */
				if ((m_IP == 7) && m_inhibit) m_pitch_zero = true;
				if (m_IP == 7) // RESETL4
				{
					// Latch OLDE and OLDP
					//if (old_frame_silence_flag()) m_uv_zpar = false; // TMS51xx INTERP BUG2
					m_OLDE = new_frame_silence_flag(); // old_frame_silence_flag()
					m_OLDP = new_frame_unvoiced_flag(); // old_frame_unvoiced_flag()
					/* if TALK was clear last frame, halt speech now, since TALKD (latched from TALK on new frame) just went inactive. */

					LOGMASKED(LOG_GENERATION, "RESETL4, about to update status: IP=%d, PC=%d, subcycle=%d, m_SPEN=%d, m_TALK=%d, m_TALKD=%d\n", m_IP, m_PC, m_subcycle, m_SPEN, m_TALK, m_TALKD);
					if ((!m_TALK) && (!m_SPEN))
						LOGMASKED(LOG_GENERATION, "tms5220_process: processing frame: TALKD = 0 caused by stop frame or buffer empty, halting speech.\n");

					m_TALKD = m_TALK; // TALKD is latched from TALK
					update_fifo_status_and_ints(); // to trigger an interrupt if talk_status has changed
					if ((!m_TALK) && m_SPEN) m_TALK = true; // TALK is only activated if it wasn't already active, if m_SPEN is active, and if we're in RESETL4 (which we are).

					LOGMASKED(LOG_GENERATION, "RESETL4, status updated: IP=%d, PC=%d, subcycle=%d, m_SPEN=%d, m_TALK=%d, m_TALKD=%d\n", m_IP, m_PC, m_subcycle, m_SPEN, m_TALK, m_TALKD);
				}
				m_subcycle = m_subc_reload;
				m_PC = 0;
				m_IP++;
				m_IP &= 0x7;
			}
			else if (m_subcycle == 3)
			{
				m_subcycle = m_subc_reload;
				m_PC++;
			}
			m_pitch_count++;
			if ((m_pitch_count >= m_current_pitch) || m_pitch_zero) m_pitch_count = 0;
			m_pitch_count &= 0x1FF;
		}
		else // m_TALKD == 0
		{
			m_subcycle++;
			if ((m_subcycle == 2) && (m_PC == 12)) // RESETF3
			{
				if (m_IP == 7) // RESETL4
				{
					m_TALKD = m_TALK; // TALKD is latched from TALK
					update_fifo_status_and_ints(); // probably not necessary
					if ((!m_TALK) && m_SPEN) m_TALK = true; // TALK is only activated if it wasn't already active, if m_SPEN is active, and if we're in RESETL4 (which we are).
				}
				m_subcycle = m_subc_reload;
				m_PC = 0;
				m_IP++;
				m_IP&=0x7;
			}
			else if (m_subcycle == 3)
			{
				m_subcycle = m_subc_reload;
				m_PC++;
			}
			buffer[buf_count] = -1; /* should be just -1; actual chip outputs -1 every idle sample; (cf note in data sheet, p 10, table 4) */
		}
	buf_count++;
	size--;
	}
}

/**********************************************************************************************

     clip_analog -- clips the 14 bit return value from the lattice filter to its final 10 bit value (-512 to 511), and upshifts/range extends this to 16 bits

***********************************************************************************************/

int16_t tms5220_device::clip_analog(int16_t cliptemp) const
{
	/* clipping, just like the patent shows:
	 * the top 10 bits of this result are visible on the digital output IO pin.
	 * next, if the top 3 bits of the 14 bit result are all the same, the
	 * lowest of those 3 bits plus the next 7 bits are the signed analog
	 * output, otherwise the low bits are all forced to match the inverse of
	 * the topmost bit, i.e.:
	 * 1x xxxx xxxx xxxx -> 0b10000000
	 * 11 1bcd efgh xxxx -> 0b1bcdefgh
	 * 00 0bcd efgh xxxx -> 0b0bcdefgh
	 * 0x xxxx xxxx xxxx -> 0b01111111
	 */
	if ((cliptemp > 2047) || (cliptemp < -2048))
		LOGMASKED(LOG_CLIP, "clipping cliptemp to range; was %d\n", cliptemp);
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
int32_t tms5220_device::matrix_multiply(int32_t a, int32_t b) const
{
	int32_t result;
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	result = ((a*b)>>9); /** TODO: this isn't technically right to the chip, which truncates the lowest result bit, but it causes glitches otherwise. **/
	if (result>16383) LOGMASKED(LOG_MULTIPLY, "matrix multiplier overflowed! a: %x, b: %x, result: %x", a, b, result);
	if (result<-16384) LOGMASKED(LOG_MULTIPLY, "matrix multiplier underflowed! a: %x, b: %x, result: %x", a, b, result);
	return result;
}

/**********************************************************************************************

     lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
     excitation data, and specific values of all the current k constants,  and returns the
     resulting sample.

***********************************************************************************************/

int32_t tms5220_device::lattice_filter()
{
	// Lattice filter here
	// Aug/05/07: redone as unrolled loop, for clarity - LN
	/* Originally Copied verbatim from table I in US patent 4,209,804, now
	  updated to be in same order as the actual chip does it, not that it matters.

	  notation equivalencies from table:
	  Yn(i) == m_u[n-1]
	  Kn = m_current_k[n-1]
	  bn = m_x[n-1]
	 */
	/*
	    int ep = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
	     m_u[10] = ep;
	    for (int i = 0; i < 10; i++)
	    {
	        int ii = 10-i; // for m = 10, this would be 11 - i, and since i is from 1 to 10, then ii ranges from 10 to 1
	        // int jj = ii+1; // this variable, even on the fortran version, is
	        // never used. It probably was intended to be used on the two lines
	        // below the next one to save some redundant additions on each.
	        ep = ep - (((m_current_k[ii-1] * m_x[ii-1])>>9)|1); // subtract reflection from lower stage 'top of lattice'
	         m_u[ii-1] = ep;
	        m_x[ii] = m_x[ii-1] + (((m_current_k[ii-1] * ep)>>9)|1); // add reflection from upper stage 'bottom of lattice'
	    }
	m_x[0] = ep; // feed the last section of the top of the lattice directly to the bottom of the lattice
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
		int32_t err = m_x[9] + matrix_multiply(m_current_k[9], m_u[9]); //x_10, real chip doesn't use or calculate this
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

		LOGMASKED(LOG_LATTICE, "V:%04d ", m_u[10]);
		for (int i = 9; i >= 0; i--)
		{
			LOGMASKED(LOG_LATTICE, "Y%d:%04d ", i+1, m_u[i]);
		}
		LOGMASKED(LOG_LATTICE, "\n");
		LOGMASKED(LOG_LATTICE, "E:%04d ", err);
		for (int i = 9; i >= 0; i--)
		{
			LOGMASKED(LOG_LATTICE, "b%d:%04d ", i+1, m_x[i]);
		}
		LOGMASKED(LOG_LATTICE, "\n");

		return m_u[0];
}


/**********************************************************************************************

     process_command -- decode byte and run the command

     During SPEAK, the address counter is locked against modification by
     LOAD_ADDRESS. In that time, a LOAD_ADDRESS command does not terminate but
     remains in the command register.
     When another command is fed into the speech processor, the READY line is
     lowered until the command register is cleared, then the new command is
     loaded into the command register.

     Note that the running SPEAK command does not block the command register;
     it is a command that attempts to change the address during the SPEAK
     process. [mz]

***********************************************************************************************/

void tms5220_device::process_command(uint8_t cmd)
{
	m_command_register = cmd;

	/* parse the command */
	switch (cmd & 0x70)
	{
	case 0x10 : /* read byte */
		LOGMASKED(LOG_COMMAND, "Read Byte command received (%02X)\n", cmd);
		if (!talk_status()) /* TALKST must be clear for RDBY */
		{
			if (m_schedule_dummy_read)
			{
				m_schedule_dummy_read = false;
				perform_dummy_read();
			}

			m_read_byte_register = read_bits(8);
			m_RDB_flag = true;
			m_command_register = NOCOMMAND;
		}
		else
			LOGMASKED(LOG_COMMAND_VERBOSE, "Read Byte command received during TALK state, suspended.\n");
		break;

	case 0x00:
	case 0x20: /* set rate (tms5220c and cd2501ecd only), otherwise NOP */
		if (TMS5220_HAS_RATE_CONTROL)
		{
			LOGMASKED(LOG_COMMAND_NOP, "Set Rate (or NOP) command received (%02X)\n", cmd);
			m_c_variant_rate = cmd&0x0F;
		}
		else
			LOGMASKED(LOG_COMMAND_NOP, "NOP command received (%02X)\n", cmd);

		m_command_register = NOCOMMAND;
		break;

	case 0x30 : /* read and branch */
		if (!talk_status()) /* TALKST must be clear for RB */
		{
			LOGMASKED(LOG_COMMAND, "Read and Branch command received (%02X)\n", cmd);
			m_RDB_flag = false;

			if (m_schedule_dummy_read)
			{
				m_schedule_dummy_read = false;
				perform_dummy_read();
			}

			if (!m_m0_cb.isunset())
				vsm_read_and_branch();
			else
				LOGMASKED(LOG_VSM, "WARNING: Read-and-branch: No memory attached to VSP\n");

			m_command_register = NOCOMMAND;
		}
		break;

	case 0x40 : /* load address */
		if (!talk_status()) /* TALKST must be clear for LA */
		{
			LOGMASKED(LOG_COMMAND, "Load Address command received (%02X)\n", cmd);

			// tms5220 data sheet says that if we load only one 4-bit nibble,
			// it won't work. This code does not care about this.
			if (!m_m0_cb.isunset())
				vsm_write_addr(cmd & 0x0f);

			m_schedule_dummy_read = true;
			m_command_register = NOCOMMAND;
		}
		else
		{
			// The Load Address command is not ignored during speech output,
			// as tests show. In fact, it is normally executed when the speech
			// terminates.
			LOGMASKED(LOG_COMMAND_VERBOSE, "Load Address command received during TALK state, suspended.\n");
		}
		break;

	case 0x50 : /* speak */
		LOGMASKED(LOG_COMMAND, "Speak (VSM) command received (%02X)\n", cmd);
		if (m_schedule_dummy_read)
		{
			m_schedule_dummy_read = false;
			perform_dummy_read();
		}

		m_SPEN = 1;
#ifdef FAST_START_HACK
		m_TALK = 1;
#endif
		m_DDIS = false; // speak using VSM
		m_zpar = true; // zero all the parameters
		m_uv_zpar = true; // zero k4-k10 as well
		m_OLDE = true; // 'silence/zpar' frames are zero energy
		m_OLDP = true; // 'silence/zpar' frames are zero pitch
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
		m_old_zpar = true; // zero all the old parameters
		m_old_uv_zpar = true; // zero old k4-k10 as well
#endif
		// following is semi-hack but matches idle state observed on chip
		m_new_frame_energy_idx = 0;
		m_new_frame_pitch_idx = 0;
		for (int i = 0; i < 4; i++)
			m_new_frame_k_idx[i] = 0;
		for (int i = 4; i < 7; i++)
			m_new_frame_k_idx[i] = 0xF;
		for (int i = 7; i < m_coeff->num_k; i++)
			m_new_frame_k_idx[i] = 0x7;

		m_command_register = NOCOMMAND;
		break;

	case 0x60 : /* speak external */
		LOGMASKED(LOG_COMMAND, "Speak External command received (%02X)\n", cmd);

		// SPKEXT going active asserts /SPKEE for 2 clocks, which clears the FIFO and its counters
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
		m_fifo_head = m_fifo_tail = m_fifo_count = m_fifo_bits_taken = 0;
		// SPEN is enabled when the FIFO passes half full (falling edge of BL signal)
		m_DDIS = true; // speak using FIFO
		m_zpar = true; // zero all the parameters
		m_uv_zpar = true; // zero k4-k10 as well
		m_OLDE = true; // 'silence/zpar' frames are zero energy
		m_OLDP = true; // 'silence/zpar' frames are zero pitch
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
		m_old_zpar = true; // zero all the old parameters
		m_old_uv_zpar = true; // zero old k4-k10 as well
#endif
		// following is semi-hack but matches idle state observed on chip
		m_new_frame_energy_idx = 0;
		m_new_frame_pitch_idx = 0;
		for (int i = 0; i < 4; i++)
			m_new_frame_k_idx[i] = 0;
		for (int i = 4; i < 7; i++)
			m_new_frame_k_idx[i] = 0xF;
		for (int i = 7; i < m_coeff->num_k; i++)
			m_new_frame_k_idx[i] = 0x7;
		m_RDB_flag = false;

		m_command_register = NOCOMMAND;
		break;

	case 0x70 : /* reset */
		LOGMASKED(LOG_COMMAND, "Reset command received (%02X)\n", cmd);
		if (m_schedule_dummy_read)
		{
			m_schedule_dummy_read = false;
			perform_dummy_read();
		}

		reset();
		m_command_register = NOCOMMAND;
		break;
	}

	/* update the buffer low state */
	update_fifo_status_and_ints();
}

/******************************************************************************************

     parse_frame -- parse a new frame's worth of data; returns 0 if not enough bits in buffer

******************************************************************************************/

void tms5220_device::parse_frame()
{
	int i, rep_flag;
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
	m_old_uv_zpar = m_uv_zpar;
	m_old_zpar = m_zpar;
#endif
	/* Since we're parsing a frame, we must be talking, so clear zpar here.
	Also, before we started parsing a frame, the P=0 and E=0 latches were both
	reset by RESETL4, so clear m_uv_zpar here.
	*/
	m_uv_zpar = m_zpar = 0;

	/* We actually don't care how many bits are left in the FIFO here;
	the frame subpart will be processed normally, and any bits extracted
	'past the end' of the FIFO will be read as zeroes; the FIFO being emptied
	will set the /BE latch which will halt speech exactly as if a stop frame
	had been encountered (instead of whatever partial frame was read).
	The same exact circuitry is used for both functions on the real chip, see
	us patent 4335277 sheet 16, gates 232a (decode stop frame) and 232b
	(decode /BE plus DDIS (decode disable) which is active during speak external).
	*/

	/* if the chip is a tms5220C, and the rate mode is set to that each frame (0x04 bit set)
	has a 2 bit rate preceding it, grab two bits here and store them as the rate; */
	if ((TMS5220_HAS_RATE_CONTROL) && (m_c_variant_rate & 0x04))
	{
		i = read_bits(2);
		printbits(i, 2);
		LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");
		m_IP = reload_table[i];
	}
	else // non-5220C and 5220C in fixed rate mode
	m_IP = reload_table[m_c_variant_rate&0x3];

	update_fifo_status_and_ints();
	if (m_DDIS && m_buffer_empty) goto ranout;

	// attempt to extract the energy index
	m_new_frame_energy_idx = read_bits(m_coeff->energy_bits);
	printbits(m_new_frame_energy_idx, m_coeff->energy_bits);
	LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");
	update_fifo_status_and_ints();
	if (m_DDIS && m_buffer_empty) goto ranout;
	// if the energy index is 0 or 15, we're done
	if ((m_new_frame_energy_idx == 0) || (m_new_frame_energy_idx == 15))
		return;


	// attempt to extract the repeat flag
	rep_flag = read_bits(1);
	printbits(rep_flag, 1);
	LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");

	// attempt to extract the pitch
	m_new_frame_pitch_idx = read_bits(m_coeff->pitch_bits);
	printbits(m_new_frame_pitch_idx, m_coeff->pitch_bits);
	LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");
	// if the new frame is unvoiced, be sure to zero out the k5-k10 parameters
	m_uv_zpar = new_frame_unvoiced_flag();
	update_fifo_status_and_ints();
	if (m_DDIS && m_buffer_empty) goto ranout;
	// if this is a repeat frame, just do nothing, it will reuse the old coefficients
	if (rep_flag)
		return;

	// extract first 4 K coefficients
	for (i = 0; i < 4; i++)
	{
		m_new_frame_k_idx[i] = read_bits(m_coeff->kbits[i]);
		printbits(m_new_frame_k_idx[i], m_coeff->kbits[i]);
		LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");
		update_fifo_status_and_ints();
		if (m_DDIS && m_buffer_empty) goto ranout;
	}

	// if the pitch index was zero, we only need 4 K's...
	if (m_new_frame_pitch_idx == 0)
	{
		/* and the rest of the coefficients are zeroed, but that's done in the generator code */
		return;
	}

	// If we got here, we need the remaining 6 K's
	for (i = 4; i < m_coeff->num_k; i++)
	{
		m_new_frame_k_idx[i] = read_bits(m_coeff->kbits[i]);
		printbits(m_new_frame_k_idx[i], m_coeff->kbits[i]);
		LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, " ");
		update_fifo_status_and_ints();
		if (m_DDIS && m_buffer_empty) goto ranout;
	}
	LOGMASKED(LOG_PARSE_FRAME_DUMP_BIN | LOG_PARSE_FRAME_DUMP_HEX, "\n");

	if (m_DDIS)
		LOGMASKED(LOG_PARSE, "Parsed a frame successfully in FIFO - %d bits remaining\n", (m_fifo_count*8)-(m_fifo_bits_taken));
	else
		LOGMASKED(LOG_PARSE, "Parsed a frame successfully in ROM\n");
	return;

	ranout:
	LOGMASKED(LOG_FRAME_ERRORS, "Ran out of bits on a parse!\n");
	return;
}

/**********************************************************************************************

     set_interrupt_state -- generate an interrupt

***********************************************************************************************/

void tms5220_device::set_interrupt_state(int state)
{
	if (!TMS5220_IS_52xx) return; // bail out if not a 52xx chip, since there's no int pin

	LOGMASKED(LOG_PIN_READS, "irq pin set to state %d\n", state);

	if (state != m_irq_pin)
	{
		m_irq_pin = state;
		m_irq_handler(!state);
	}
}

/**********************************************************************************************

     update_ready_state -- update the ready line

***********************************************************************************************/

void tms5220_device::update_ready_state()
{
	bool state = m_io_ready;
	if (m_ready_pin != state)
	{
		LOGMASKED(LOG_PIN_READS, "ready pin set to state %d\n", state);
		m_readyq_handler(!state);
		m_ready_pin = state;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5220_device::device_start()
{
	switch (m_variant)
	{
	case TMS5220_IS_TMC0281:
		m_coeff = &T0280B_0281A_coeff;
		break;
	case TMS5220_IS_TMC0281D:
		m_coeff = &T0280D_0281D_coeff;
		break;
	case TMS5220_IS_CD2801:
		m_coeff = &T0280F_2801A_coeff;
		break;
	case TMS5220_IS_M58817:
		m_coeff = &M58817_coeff;
		break;
	case TMS5220_IS_CD2802:
		m_coeff = &T0280F_2802_coeff;
		break;
	case TMS5220_IS_TMS5110A:
		m_coeff = &tms5110a_coeff;
		break;
	case TMS5220_IS_5200:
	case TMS5220_IS_CD2501ECD:
		m_coeff = &T0285_2501E_coeff;
		break;
	case TMS5220_IS_5220C:
	case TMS5220_IS_5220:
		m_coeff = &tms5220_coeff;
		break;
	default:
		fatalerror("Unknown variant in tms5220_set_variant\n");
	}

	/* initialize a stream */
	m_stream = stream_alloc(0, 1, clock() / 80);

	m_timer_io_ready = timer_alloc(FUNC(tms5220_device::set_io_ready), this);

	/* not during reset which is called from within a write! */
	m_io_ready = true;
	m_true_timing = false;
	m_rs_ws = 0x03; // rs and ws are assumed to be inactive on device startup
	m_write_latch = 0; // assume on start that nothing is driving the data bus

	register_for_save_states();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms5220_device::device_reset()
{
	m_digital_select = FORCE_DIGITAL; // assume analog output
	/* initialize the FIFO */
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	m_fifo_head = m_fifo_tail = m_fifo_count = m_fifo_bits_taken = 0;

	/* initialize the chip state */
	/* Note that we do not actually clear IRQ on start-up : IRQ is even raised if m_buffer_empty or m_buffer_low are 0 */
	m_SPEN = m_DDIS = m_TALK = m_TALKD = m_previous_talk_status = m_irq_pin = m_ready_pin = false;
	set_interrupt_state(0);
	update_ready_state();
	m_buffer_empty = m_buffer_low = true;

	m_command_register = NOCOMMAND;
	m_data_latched = false;

	m_RDB_flag = false;

	/* initialize the energy/pitch/k states */
#ifdef TMS5220_PERFECT_INTERPOLATION_HACK
	m_old_frame_energy_idx = m_old_frame_pitch_idx = 0;
	std::fill(std::begin(m_old_frame_k_idx), std::end(m_old_frame_k_idx), 0);
	m_old_zpar = false;
#endif
	m_new_frame_energy_idx = m_current_energy =  m_previous_energy = 0;
	m_new_frame_pitch_idx = m_current_pitch = 0;
	m_zpar = m_uv_zpar = false;
	std::fill(std::begin(m_new_frame_k_idx), std::end(m_new_frame_k_idx), 0);
	std::fill(std::begin(m_current_k), std::end(m_current_k), 0);

	/* initialize the sample generators */
	m_inhibit = true;
	m_subcycle = m_c_variant_rate = m_pitch_count = m_PC = 0;
	m_subc_reload = FORCE_SUBC_RELOAD;
	m_OLDE = m_OLDP = true;
	m_IP = reload_table[m_c_variant_rate&0x3];
	m_RNG = 0x1FFF;
	std::fill(std::begin(m_u), std::end(m_u), 0);
	std::fill(std::begin(m_x), std::end(m_x), 0);

	perform_dummy_read();

	// 5110 specific stuff
	m_PDC = 0;
	m_CTL_pins = 0;
	m_state = 0;
	m_address = 0;
	m_next_is_address = false;
	m_addr_bit = 0;
	m_CTL_buffer = 0;
}

TIMER_CALLBACK_MEMBER(tms5220_device::set_io_ready)
{
	/* bring up to date first */
	m_stream->update();

	LOGMASKED(LOG_IO_READY, "m_timer_io_ready timer fired, param = %02x, m_rs_ws = %02x\n", param, m_rs_ws);
	if (param) // low->high ready state
	{
		switch (m_rs_ws)
		{
		case 0x02:
			/* Write */
			LOGMASKED(LOG_IO_READY, "m_timer_io_ready: Attempting to service write...\n");
			if ((m_fifo_count >= FIFO_SIZE) && m_DDIS) // if FIFO is full and we're in speak external mode
			{
				LOGMASKED(LOG_IO_READY, "m_timer_io_ready: in SPKEXT and FIFO was full! cannot service write now, delaying 16 cycles...\n");
				m_timer_io_ready->adjust(clocks_to_attotime(16), 1);
				break;
			}
			else
			{
				if (m_command_register != NOCOMMAND)
				{
					// The command register is still busy; do not activate /READY
					// and keep the data latch
					LOGMASKED(LOG_IO_READY, "m_timer_io_ready: Command register not ready\n");
				}
				else
				{
					LOGMASKED(LOG_IO_READY, "m_timer_io_ready: Serviced write: %02X\n", m_write_latch);
					m_data_latched = false;
					data_write(m_write_latch);
				}

				break;
			}
		case 0x01:
			/* Read */
			m_read_latch = status_read(true);
			LOGMASKED(LOG_IO_READY, "m_timer_io_ready: Serviced read, returning %02X\n", m_read_latch);
			m_io_ready = true;
			break;
		case 0x03:
			/* High Impedance */
			m_io_ready = true;
			break;
		case 0x00:
			/* illegal */
			m_io_ready = true;
			break;
		}
	}

	update_ready_state();
}

/*
 * /RS line write handler
 */
void tms5220_device::rsq_w(int state)
{
	m_true_timing = true;
	state &= 0x01;
	LOGMASKED(LOG_RS_WS, "/RS written with data: %d\n", state);

	uint8_t new_val = (m_rs_ws & 0x01) | (state<<1);
	if (new_val != m_rs_ws)
	{
		m_rs_ws = new_val;
		if (new_val == 0)
		{
			if (TMS5220_HAS_RATE_CONTROL) // correct for 5220c, probably also correct for cd2501ecd
				reset();
			else
				/* illegal */
				LOGMASKED(LOG_RS_WS, "tms5220_rsq_w: illegal\n");
			return;
		}
		else if (new_val == 3)
		{
			/* high impedance */
			m_read_latch = 0xff;
			return;
		}
		if (state)
		{
			/* low to high */
		}
		else
		{
			/* high to low - schedule ready cycle */
			LOGMASKED(LOG_RS_WS, "Scheduling ready cycle for /RS...\n");
			/* upon /RS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
			m_io_ready = false;
			update_ready_state();
			// The datasheet doesn't give an exact time when /READY should change, but the data is valid 6-11 usec after /RS goes low.
			// It looks like /READY goes high soon after that (although the datasheet graph is not to scale).
			// The value of 13 was measured on a real chip with an oscilloscope, and it fits the datasheet.
			m_timer_io_ready->adjust(attotime::from_usec(13), 1);
		}
	}
}

/*
 * /WS line write handler
 */
void tms5220_device::wsq_w(int state)
{
	m_true_timing = true;
	state &= 0x01;
	LOGMASKED(LOG_RS_WS, "/WS written with data: %d\n", state);

	uint8_t new_val = (m_rs_ws & 0x02) | (state<<0);
	if (new_val != m_rs_ws)
	{
		m_rs_ws = new_val;
		if (new_val == 0)
		{
			if (TMS5220_HAS_RATE_CONTROL) // correct for 5220c, probably also correct for cd2501ecd
				reset();
			else
				/* illegal */
				LOGMASKED(LOG_RS_WS, "tms5220_wsq_w: illegal\n");
			return;
		}
		else if (new_val == 3)
		{
			/* high impedance */
			m_read_latch = 0xff;
			return;
		}
		if (state)
		{
			/* low to high  */
		}
		else
		{
			/* high to low - schedule ready cycle */
			LOGMASKED(LOG_RS_WS, "Scheduling ready cycle for /WS...\n");

			/* upon /WS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
			m_io_ready = false;
			update_ready_state();
			/* Now comes the complicated part: how long does /READY stay inactive, when /WS is pulled low? This depends ENTIRELY on the command written, or whether the chip is in speak external mode or not...
			Speak external mode: ~16 cycles
			Command Mode:
			SPK: ? cycles
			SPKEXT: ? cycles
			RDBY: between 60 and 140 cycles
			RB: ? cycles (80?)
			RST: between 60 and 140 cycles
			SET RATE (5220C and CD2501ECD only): ? cycles (probably ~16)
			*/
			// TODO: actually HANDLE the timing differences! currently just assuming always 16 cycles
			m_timer_io_ready->adjust(clocks_to_attotime(16), 1); // this should take around 10-16 (closer to ~15) cycles to complete for FIFO writes, TODO: but actually depends on what command is written if in command mode
		}
	}
}

/*
 * combined /RS and /WS line write handler;
 * /RS is bit 1, /WS is bit 0
 * Note this is a hack and probably can be removed later, once the 'real' line handlers above defer by at least 4 clock cycles before taking effect
 */
void tms5220_device::combined_rsq_wsq_w(u8 data)
{
	uint8_t falling_edges;
	m_true_timing = true;
	LOGMASKED(LOG_RS_WS, "/RS and /WS written with %d and %d respectively\n", (data&2)>>1, data&1);

	uint8_t new_val = data & 0x03;
	if (new_val != m_rs_ws)
	{
		falling_edges = ((m_rs_ws^new_val)&(~new_val));
		m_rs_ws = new_val;
		switch(new_val)
		{
			case 0:
				if (TMS5220_HAS_RATE_CONTROL) // correct for 5220c, probably also correct for cd2501ecd
					reset();
				else
					/* illegal */
					LOGMASKED(LOG_RS_WS, "tms5220_combined_rsq_wsq_w: illegal\n");
				return;
			case 3:
				/* high impedance */
				m_read_latch = 0xff;
				return;
			case 2: // /WS active, /RS not
				/* check for falling or rising edge */
				if (!(falling_edges&0x01)) return; /* low to high, do nothing */

				/* high to low - schedule ready cycle */
				LOGMASKED(LOG_RS_WS, "Scheduling ready cycle for /WS...\n");

				/* upon /WS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
				m_io_ready = false;
				update_ready_state();

				/* Now comes the complicated part: how long does /READY stay inactive, when /WS is pulled low? This depends ENTIRELY on the command written, or whether the chip is in speak external mode or not...
				Speak external mode: ~16 cycles
				Command Mode:
				SPK: ? cycles
				SPKEXT: ? cycles
				RDBY: between 60 and 140 cycles
				RB: ? cycles (80?)
				RST: between 60 and 140 cycles
				SET RATE (5220C and CD2501ECD only): ? cycles (probably ~16)
				*/
				// TODO: actually HANDLE the timing differences! currently just assuming always 16 cycles
				m_timer_io_ready->adjust(clocks_to_attotime(16), 1); // this should take around 10-16 (closer to ~15) cycles to complete for FIFO writes, TODO: but actually depends on what command is written if in command mode
				return;
			case 1: // /RS active, /WS not
				/* check for falling or rising edge */
				if (!(falling_edges&0x02)) return; /* low to high, do nothing */

				/* high to low - schedule ready cycle */
				LOGMASKED(LOG_RS_WS, "Scheduling ready cycle for /RS...\n");

				/* upon /RS being activated, /READY goes inactive after 100 nsec from data sheet, through 3 asynchronous gates on patent. This is effectively within one clock, so we immediately set io_ready to 0 and activate the callback. */
				m_io_ready = false;
				update_ready_state();

				/* How long does /READY stay inactive, when /RS is pulled low? I believe its almost always ~16 clocks (25 usec at 800khz as shown on the datasheet) */
				m_timer_io_ready->adjust(clocks_to_attotime(16), 1); // this should take around 10-16 (closer to ~11?) cycles to complete
				return;
		}
	}
}


/**********************************************************************************************

     tms5220_data_w -- write data to the sound chip

***********************************************************************************************/

void tms5220_device::data_w(uint8_t data)
{
	LOGMASKED(LOG_DATA_W, "tms5220_write_data: data %02X\n", data);
	/* bring up to date first */
	m_stream->update();
	m_write_latch = data;
	m_data_latched = true;

	if (!m_true_timing) // if we're in the default hacky mode where we don't bother with rsq_w and wsq_w...
		data_write(m_write_latch); // ...force the write through instantly.
	else
	{
		/* actually in a write ? */
		if (!(m_rs_ws == 0x02))
			LOGMASKED(LOG_RS_WS, "tms5220_data_w: data written outside ws, status: %02x!\n", m_rs_ws);
	}
}



/**********************************************************************************************

     tms5220_status_r -- read status or data from the sound chip

***********************************************************************************************/

uint8_t tms5220_device::status_r()
{
	// prevent debugger from changing the internal state
	if (!machine().side_effects_disabled())
		m_stream->update(); /* bring up to date first */

	if (!m_true_timing)
	{
		// prevent debugger from changing the internal state
		if (machine().side_effects_disabled())
			return status_read(false);
		return status_read(true);
	}
	else
	{
		/* actually in a read ? */
		if (m_rs_ws == 0x01)
			return m_read_latch;
		else
			LOGMASKED(LOG_RS_WS, "tms5220_status_r: data read outside rs!\n");
		return 0xff; // m_write_latch; // TODO: return open bus?
	}
}



/**********************************************************************************************

     tms5220_ready_r -- return the not ready status from the sound chip

***********************************************************************************************/

int tms5220_device::readyq_r()
{
	// prevent debugger from changing the internal state
	if (!machine().side_effects_disabled())
		m_stream->update(); /* bring up to date first */
	return !ready_read();
}



/**********************************************************************************************

     tms5220_int_r -- return the int status from the sound chip

***********************************************************************************************/

int tms5220_device::intq_r()
{
	// prevent debugger from changing the internal state
	if (!machine().side_effects_disabled())
		m_stream->update(); /* bring up to date first */
	return !int_read();
}



/**********************************************************************************************

     tms5220_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5220_device::sound_stream_update(sound_stream &stream)
{
	int16_t sample_data[MAX_SAMPLE_CHUNK];

	/* loop while we still have samples to generate */
	for (int sampindex = 0; sampindex < stream.samples(); )
	{
		int length = (stream.samples() > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : stream.samples();

		/* generate the samples and copy to the target buffer */
		process(sample_data, length);
		for (int index = 0; index < length; index++)
			stream.put_int(0, sampindex++, sample_data[index], 32768);
	}
}



/**********************************************************************************************

     tms5220_set_frequency -- adjusts the playback frequency

***********************************************************************************************/

void tms5220_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 80);
}


DEFINE_DEVICE_TYPE(TMS5220C,  tms5220c_device,  "tms5220c",  "TMS5220C")
DEFINE_DEVICE_TYPE(TMS5220,   tms5220_device,   "tms5220",   "TMS5220")
DEFINE_DEVICE_TYPE(CD2501E,   cd2501e_device,   "cd2501e",   "CD2501E")
DEFINE_DEVICE_TYPE(TMS5200,   tms5200_device,   "tms5200",   "TMS5200")
DEFINE_DEVICE_TYPE(CD2501ECD, cd2501ecd_device, "cd2501ecd", "CD2501ECD")


tms5220c_device::tms5220c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms5220_device(mconfig, TMS5220C, tag, owner, clock, TMS5220_IS_5220C)
{
}


tms5220_device::tms5220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms5220_device(mconfig, TMS5220, tag, owner, clock, TMS5220_IS_5220)
{
}

tms5220_device::tms5220_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_variant(variant)
	, m_irq_handler(*this)
	, m_readyq_handler(*this)
	, m_m0_cb(*this)
	, m_m1_cb(*this)
	, m_addr_cb(*this)
	, m_data_cb(*this, 0)
	, m_romclk_cb(*this)
{
}


cd2501e_device::cd2501e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms5220_device(mconfig, CD2501E, tag, owner, clock, TMS5220_IS_CD2501E)
{
}


tms5200_device::tms5200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms5220_device(mconfig, TMS5200, tag, owner, clock, TMS5220_IS_5200)
{
}


cd2501ecd_device::cd2501ecd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms5220_device(mconfig, CD2501ECD, tag, owner, clock, TMS5220_IS_CD2501ECD)
{
}
