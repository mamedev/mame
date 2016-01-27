// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  DECtalk DTC-01 Driver
*  Copyright (C) 2009-2013 Jonathan Gevaryahu AKA Lord Nightmare
*  with major help (dumping, tech questions answered, etc)
*  from Kevin 'kevtris' Horton, without whom this driver would
*  have been impossible.
*  Special thanks to Al Kossow for archiving the DTC-01 schematic at bitsavers,
*  http://bitsavers.org/pdf/dec/dectalk/MP-01820_DTC01_EngrDrws_Nov83.pdf
*  which has been invaluable for work on this driver.
*  Special thanks to leeeeee for helping figure out what the led selftest codes actually mean
*
*
*  This driver dedicated in memory of Dennis Klatt and Jonathan Allen, without whose
*  original work MITalk and hence KlattTalk and DECtalk would never have existed,
*  in memory of Martin Minow, who wrote much of the DECtalk DTC-01 code, and
*  in memory of Tony Vitale, who was one of the architects of the DECtalk project.
*
*  Staff behind DECtalk itself: (mostly from http://amhistory.si.edu/archives/speechsynthesis/ss_dec.htm ):
*     John C. Broihier
*     Edward A. Bruckert (who followed the DECtalk IP from DEC->Compaq->HP->Force Computers->Fonix inc from where he retired in 2014)
*     Dave Conroy (dtc-03 letter to sound rules; logic design, worked on dtc-01 and dtc-03, see correspondence below)
*     Michael J. Crowley
*     Dennis H. Klatt [1938 - Dec 30, 1988] (worked on klsyn and other parts of MITalk, wrote KLATTtalk which was exclusively licensed to DEC in 1982, worked on dtc-01 and dtc-03)
*     Martin A. Minow [Nov 6, 1940 - Dec 21, 2000] (dtc-01 conversion of hunnicutt's letter to sound rules, phonetics and general programming)
*     Walter Tetschner (DECtalk project director; as of 2013 publishes ASRnews: http://www.asrnews.com/)
*     Anthony J. Vitale [Apr 30, 1945 - Aug 5, 2002] (DECtalk project architect; dtc-03 letter to sound rules; worked on dtc-01 and dtc-03)
*
*  Staff behind MITalk: (also see http://amhistory.si.edu/archives/speechsynthesis/ss_mit.htm )
*     Dennis H. Klatt (see above)
*     Jonathan Allen [Jun 4, 1934 - Apr 24, 2000] (speech synthesis theory and development)
*     M. Sharon 'Sheri' Hunnicutt (worked on letter to phoneme rules, which were licensed non-exclusively to DEC in 1982; still works as a docent at KTH in Sweden as of 2013)
*
*  TODO:
*  * DUART:
*      * The duart self tests are EXTENSIVE and make an excellent check of many of the duart internal bits.
*        To enable the self tests rather than bypassing them, under dipswitches, set 'Skip Self Test(IP4)' to 'Open (VCC)'
*        and under system configuration set 'Hack to prevent hang when skip self test is shorted' to 'Off'
*    * <DONE> DUART needs to be reset on reset line activation. as is it works ok, but it should be done anyway.
*    * DUART needs its i/o pins connected as well:
*    * pins IP0, IP2, and IP3 are connected to the primary serial port:
*      * IP0 is CTS
*      * IP2 is DSR
*      * IP3 is RLS (received line signal, this pin is rarely used on rs232)
*    * <DONE> pins IP4, IP5 and IP6 are on jumpers on the DUART, tied high normally but jumperable low, should be handled as dipswitches:
*      * IP4 low: skip hardware self tests
*      * IP5 low: unknown
*      * IP6 low: unknown
*    * pins OP0 and OP2 are connected to the primary serial port:
*      * OP0 is RTS
*      * OP2 is DTR
*  * <DONE> Figure out why the v1.8 dectalk firmware clips/screeches like hell (it requires the older dsp code to work properly)
*  * <DONE> Figure out why the older -165/-166 and newer -409/-410 tms32010 dsp firmwares don't produce any sound, while the middle -204/-205 one does (fifo implementations were busted)
*  * <DONE> Actually store the X2212 nvram's eeprom data to disk rather than throwing it out on exit
*    * Get setup mode with the serial BREAK int working enough to actually properly save the default nvram back to the chip in emulation, and get rid of the (currently unused) nvram default image in the rom definitions
*  * emulate/simulate the MT8860 DTMF decoder and MT8865 DTMF filter as a 16-key input device? or hook it to some simple fft code? Francois Jalbert's fftmorse code ran full speed on a 12mhz 80286, maybe use that?
*    Sarayan suggested this can be done in one of two ways:
*    1. Standalone 'canned' DTMF detector and discriminator code (francois' and peter jennings' 286 code, or modern fft code)
*    2. Emulate the MT8865 as a set of two 7th order bandpass filters with 2
*       outputs for the low and high band (as in real life as documented on the
*       datasheet) and emulate the MT8860 exactly as in real life as well, as a
*       dual-input pulse-width measurement device to distinguish the 4 low and
*       high band DTMF tones as well as their combined presence.
*       The latter is clearly more accurate but likely slower.
*  * figure out how to plumb diserial/rs232 to have an external application send data to the two serial ports to be spoken; this shouldn't be too hard at this point.
*
* LED error code list (found by experimentation and help from leeeeee):
*    Startup Self tests:
*    FF 00 - M68k address register check fail or data register check fail (test code at $21E)
*     (for some data register failures the processor just spins forever and no led code is generated)
*    FF 01 - ROM check fail @ 0x00000, rom at E8 or E22 (test code at $278)
*    FF 02 - ROM check fail @ 0x08000, rom at E7 or E21 "
*    FF 03 - ROM check fail @ 0x10000, rom at E6 or E20 "
*    FF 04 - ROM check fail @ 0x18000, rom at E5 or E19 "
*    FF 05 - ROM check fail @ 0x20000, rom at E4 or E18 "
*    FF 06 - ROM check fail @ 0x28000, rom at E3 or E17 "
*    FF 07 - ROM check fail @ 0x30000, rom at E2 or E16 "
*    FF 08 - ROM check fail @ 0x38000, rom at E1 or E15 "
*    FF 0F - ROM check fail at multiple addresses
*    FE 01 - RAM check fail @ 0x80000-0x83fff, ram at E36 or E49 (test code at $338)
*    FE 02 - RAM check fail @ 0x84000-0x87fff, ram at E35 or E48 "
*    FE 03 - RAM check fail @ 0x88000-0x8bfff, ram at E34 or E47 "
*    FE 04 - RAM check fail @ 0x8c000-0x8ffff, ram at E33 or E46 "
*    FE 05 - RAM check fail @ 0x90000-0x93fff, ram at E32 or E44 "
*    FE 0F - RAM check fail at multiple addresses
*    FD 00 - DUART test & DUART interrupt test (test code at $046C)
*    FC 00 - This test doesn't exist. Some vestiges of it may remain in code for the FD and FB tests.
*    FB 00 - TMS32010 extensive tests (test code at $051E): test spc interrupt [works] and make dtmf tone to test tlc interrupt [fails in mess, requires dtmf detection on output; this test is actually patented! US 4,552,992]
*    Jump to $102C to skip the self tests
*    During normal operation:
(table taken from http://www3.sympatico.ca/n.rieck/docs/DECtalk_notes.html )
DTC-01 LEDs

   76543210         LEDs
   ||||||||
   |||||||+-------- set if host asserting CTS
   ||||||+--------- set if DECtalk asserting RTS
   |||||+---------- set if host asserting DSR
   ||||+----------- set if host asserting DCD
   |||+------------ set if DECtalk asserting DTR
   +++------------- 3 bit state code

   000              in state 5, first 500 ms; waiting for CD & CTS in first 500 ms when DECtalk is on-line
   001              timing 2 second CD=0 while in state 6 (moving data)
   010              waiting for DSR=0 while disconnecting (part of state 7)
   011              waiting for DSR=1 while connecting (state 3)
   100              delaying for UK modems during disconnect (part of state 7)
   101              waiting for CD and CTS (main part of state 5)
   110              moving data (state 6)
   111              disconnecting (start of state 7)
*    DECtalk dtc-01 hardware and rom version history, from DTC-01 schematic at bitsavers and with additional info from
     http://americanhistory.si.edu/archives/speechsynthesis/ss_dec1.htm
*    August 1983: Hardware version A done
*    unknown date: Version 1.0 roms finalized
*    unknown date: Version 1.1 roms released to fix a bug with insufficient stack space, see ss_dec1 above
*    October 11 1983: Second half of Version 1.8 rom finalized
*    December 05 1983: First half of Version 1.8 rom finalized
*    March 1984: Hardware version B done
       (integrates the output fifo sync error check onto the pcb;
       Version A units are retrofitted when sent in for firmware upgrades)
       (most of the schematics come from this time, and have the version 1.8 roms listed on them)
*    July 02 1984: Second half of Version 2.0 rom finalized
*    July 23 1984: First half of Version 2.0 rom finalized
*    October 1984 (the rest of the schematics come from this time)
*    sometime after 6th week of 1985: dsp roms updated to the -409/-410 set
*
* NVRAM related stuff found by leeeeee (in v2.0):
* $10402 - nvram recall/reset based on byte on stack (0 = recall, 1 = write default to nvram)
* $10f2E - nvram recall
* $10f52 - entry point for nvram check routine
* $11004(entry point)-$11030, $11032-$111B4 - nvram store routine:
*    pass with a blank word on stack and be sure the test and branch at 11016 & 1101a passes (jumps to 1106a)
*    It will then write currently loaded settings to nvram and execute a store, so do it after the defaults are loaded
* $1a7ae - default nvram image, without checksum (0x80 bytes)
*******************************************************************************/
/*the 68k memory map is such:
0x000000-0x007fff: E22 low, E8 high
0x008000-0x00ffff: E21 low, E7 high
0x010000-0x017fff: E20 low, E6 high
0x018000-0x01ffff: E19 low, E5 high
0x020000-0x027fff: E18 low, E4 high
0x028000-0x02ffff: E17 low, E3 high
0x030000-0x037fff: E16 low, E2 high
0x038000-0x03ffff: E15 low, E1 high
mirrrored at 0x040000-0x07ffff
ram/nvram/speech mapping:
0x080000-0x083fff: e36 low, e49 high
0x084000-0x087fff: e35 low, e48 high
0x088000-0x08bfff: e34 low, e47 high
0x08c000-0x08ffff: e33 low, e46 high
0x090000-0x093fff: e32 low, e45 high
0x094000-0x097fff: LED/SW/NVR
0x098000-0x09bfff: 2681 DUART
0x09c000-0x09ffff: DTMF and TMS32010 (TLC, SPC)
mirrored at 0x0a0000-0x0fffff x3
entire space mirrored at 0x100000-0x7fffff
0x800000-0xffffff is open bus?

interrupts:
68k has an interrupt priority decoder attached to it:
TLC is INT level 4
SPC is INT level 5
DUART is INT level 6
*/
/* dtc-03 post by dave conroy from usenet comp.sys.dec on 12/29/2011:
> Wow.  were they better than the DTC01? (OK, I guess they had to be.)

I worked on both of these at DEC (in fact, I think if you look in the
options and modules
list, you will see my initials in the "responsible engineer" column
for the DTC03).

The goals of the DTC03 were lower cost, better letter to sound rules,
and better packaging for large systems (it was pretty inconvenient to
set up 30-40 of
the big DTC01 boxes).

The hardware was quite different. The DTC01 used a Motorola 68000 and
a TI DSP, connected
together by a big bank of (expensive) fifo chips. The DTC03 used then
(then new) Intel 80186 and the same
TI DSP, connected together by DMA (the DTC03 design was done in a way
that used *all* of the
capabilities of the 80186 to reduce the cost). The DTC01 used the
packaging of the VT240, and the DTC03
used the packaging of a family of rack-mounted modems whose part
number escapes me. The
same guy (Rich Ellison) designed both of them. The DTC03 also had an
"option module" system which
was intended to allow non-US systems to be built without needing to
change the common
parts (because it was on an independent module, and could override all
of the telephone control ESC
sequences, it could be taken through the approval process in
isolation), although it was used
to build some semi-custom systems as well.

The code in the DSP and the code that transformed a stream of phonemes
into a stream
of control commands for the DSP was pretty much the same in DTC01/
DTC03, and was based on the work of
Dennis Klatt of MIT (Dennis actually wrote a lot of this code). The
letter to sound system
in DTC01 was the final step in the evolution of a set of letter-to-
sound rules that had been floating around DEC
for a long time; the bulk of the work getting them to work in the
DTC01 was done by
Martin Minow. The letter to sound system in DTC03 was a new design by
myself and Tony Vitale,
an ex-professor of linguistics from Cornell. Most people thought it
worked much
better; in reality, it's big advantage was it made far fewer stupid
stress-placement mistakes because
it did a much better job of understanding prefixes and suffixes.

Dave Conroy
*/
/*
There are 3 things on the pins. Serial I/O. The telephone line. Power.

I believe the power is +5 and +12/-12. The +5 is for the logic,
and the +12/-12 is for the RS232 buffers and all of the analog stuff
in the anti-aliasing
filter, which is built from a pile of op-amps and stuff.

The serial I/O pins go straight to the RS232 buffers and
onward to the UART (an SCN2661, if my memory is correct).

The telephone pins go to the option connectors and the on-board
telephone line interface for the USA/Canada. Audio is available
somewhere on the
option connectors, but a really easy way to get at it is to take the
phone off-hook (send a "dial" command with an empty phone number
string)
and use the telephone pins as a transformer-coupled audio output. This
is what we
used to do in the lab all the time.

dgc (dg(no!spam)cx@mac.com)
*/

// USE_LOOSE_TIMING makes the cpu interleave much lower and boosts it on fifo and flag writes by the 68k and semaphore sets by the dsp
#define USE_LOOSE_TIMING 1
// makes use_loose_timing boost interleave when the outfifo is about to run out. slightly slows things down but may prevent some glitching
#undef USE_LOOSE_TIMING_OUTPUT
// generic logs like led state, and common writes for dsp and spc such as the speech int
#undef VERBOSE
// logs reads and writes to nvram, and nvram store/recall flag messages
#undef NVRAM_LOG
// logs reads and writes to TLC regs
#undef TLC_LOG
// logs reads and writes to SPC regs, 68k side only
#undef SPC_LOG_68K
// logs reads and writes to SPC regs, dsp side only
#undef SPC_LOG_DSP
// logs txa, txb and related serial lines to stderr
#undef SERIAL_TO_STDERR

/* Core includes */
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "machine/mc68681.h"
#include "machine/x2212.h"
#include "sound/dac.h"


class dectalk_state : public driver_device
{
public:
	enum
	{
		TIMER_OUTFIFO_READ
	};

	dectalk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_duart(*this, "duartn68681"),
		m_nvram(*this, "x2212"),
		m_dac(*this, "dac")
	{
	}

	// input fifo, between m68k and tms32010
	UINT16 m_infifo[32]; // technically eight 74LS224 4bit*16stage FIFO chips, arranged as a 32 stage, 16-bit wide fifo
	UINT8 m_infifo_count;
	UINT8 m_infifo_tail_ptr;
	UINT8 m_infifo_head_ptr;
	// output fifo, between tms32010 and 10khz sample latch for dac
	UINT16 m_outfifo[16]; // technically three 74LS224 4bit*16stage FIFO chips, arranged as a 16 stage, 12-bit wide fifo
	UINT8 m_outfifo_count;
	UINT8 m_outfifo_tail_ptr;
	UINT8 m_outfifo_head_ptr;
	UINT8 m_infifo_semaphore; // latch for status of output fifo, d-latch 74ls74 @ E64 'lower half'
	UINT8 m_spc_error_latch; // latch for error status of speech dsp, d-latch 74ls74 @ E64 'upper half'
	UINT8 m_m68k_spcflags_latch; // latch for initializing the speech dsp, d-latch 74ls74 @ E29 'lower half', AND latch for spc irq enable, d-latch 74ls74 @ E29 'upper half'; these are stored in bits 0 and 6 respectively, the rest of the bits stored here MUST be zeroed!
	UINT8 m_m68k_tlcflags_latch; // latch for telephone interface stuff, d-latches 74ls74 @ E93 'upper half' and @ 103 'upper and lower halves'
	UINT8 m_simulate_outfifo_error; // simulate an error on the outfifo, which does something unusual to the dsp latches
	UINT8 m_tlc_tonedetect;
	UINT8 m_tlc_ringdetect;
	UINT8 m_tlc_dtmf; // dtmf holding reg
	UINT8 m_duart_inport; // low 4 bits of duart input
	UINT8 m_duart_outport; // most recent duart output
	UINT8 m_hack_self_test; // temp variable for hack below

	required_device<m68000_base_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<mc68681_device> m_duart;
	required_device<x2212_device> m_nvram;
	required_device<dac_device> m_dac;
	DECLARE_WRITE_LINE_MEMBER(dectalk_duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(dectalk_duart_txa);
	DECLARE_READ8_MEMBER(dectalk_duart_input);
	DECLARE_WRITE8_MEMBER(dectalk_duart_output);
	DECLARE_READ8_MEMBER(nvram_recall);
	DECLARE_WRITE8_MEMBER(led_write);
	DECLARE_WRITE8_MEMBER(nvram_store);
	DECLARE_WRITE16_MEMBER(m68k_infifo_w);
	DECLARE_READ16_MEMBER(m68k_spcflags_r);
	DECLARE_WRITE16_MEMBER(m68k_spcflags_w);
	DECLARE_READ16_MEMBER(m68k_tlcflags_r);
	DECLARE_WRITE16_MEMBER(m68k_tlcflags_w);
	DECLARE_READ16_MEMBER(m68k_tlc_dtmf_r);
	DECLARE_WRITE16_MEMBER(spc_latch_outfifo_error_stats);
	DECLARE_READ16_MEMBER(spc_infifo_data_r);
	DECLARE_WRITE16_MEMBER(spc_outfifo_data_w);
	DECLARE_READ16_MEMBER(spc_semaphore_r);
	DECLARE_DRIVER_INIT(dectalk);
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(outfifo_read_cb);
	void dectalk_outfifo_check ();
	void dectalk_clear_all_fifos(  );
	void dectalk_semaphore_w ( UINT16 data );
	UINT16 dectalk_outfifo_r (  );
	DECLARE_WRITE_LINE_MEMBER(dectalk_reset);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/* 2681 DUART */
WRITE_LINE_MEMBER(dectalk_state::dectalk_duart_irq_handler)
{
	m_maincpu->set_input_line_and_vector(M68K_IRQ_6, state, M68K_INT_ACK_AUTOVECTOR);
	//drvstate->m_maincpu->set_input_line_and_vector(M68K_IRQ_6, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
	//drvstate->m_maincpu->set_input_line_and_vector(M68K_IRQ_6, HOLD_LINE, vector);
}

READ8_MEMBER(dectalk_state::dectalk_duart_input)
{
	UINT8 data = 0;
	data |= m_duart_inport&0xF;
	data |= (ioport("duart_in")->read()&0xF0);
	if ((m_hack_self_test == 1) && (ioport("hacks")->read()&0x01)) data |= 0x10; // hack to prevent hang if selftest disable bit is kept low past the first read; i suppose the proper use of this bit was an incremental switch, or perhaps its expecting an interrupt later from serial in or tone in? added a dipswitch to disable the hack for testing
		m_hack_self_test = 1;
	return data;
}

WRITE8_MEMBER(dectalk_state::dectalk_duart_output)
{
	m_duart_outport = data;
#ifdef SERIAL_TO_STDERR
	fprintf(stderr, "RTS: %01X, DTR: %01X\n", data&1, (data&4)>>2);
#endif
}

WRITE_LINE_MEMBER(dectalk_state::dectalk_duart_txa)
{
	//TODO: this needs to be plumbed so it shows up optionally on a second terminal somehow, or connects to diserial
	// it is the second 'alternate' serial connection on the DTC-01, used for a serial passthru and other stuff.
#ifdef SERIAL_TO_STDERR
	fprintf(stderr, "TXA:%02X ",data);
#endif
}

/* FIFO and TMS32010 stuff */
#define SPC_INITIALIZE state->m_m68k_spcflags_latch&0x1 // speech initialize flag
#define SPC_IRQ_ENABLED ((state->m_m68k_spcflags_latch&0x40)>>6) // irq enable flag

void dectalk_state::dectalk_outfifo_check ()
{
	// check if output fifo is full; if it isn't, set the int on the dsp
	if (m_outfifo_count < 16)
		m_dsp->set_input_line(0, ASSERT_LINE); // TMS32010 INT
	else
		m_dsp->set_input_line(0, CLEAR_LINE); // TMS32010 INT
}

void dectalk_state::dectalk_clear_all_fifos(  )
{
	// clear fifos (TODO: memset would work better here...)
	int i;
	for (i=0; i<16; i++) m_outfifo[i] = 0;
	m_outfifo_count = 0;
	m_outfifo_tail_ptr = m_outfifo_head_ptr = 0;
	for (i=0; i<32; i++) m_infifo[i] = 0;
	m_infifo_count = 0;
	m_infifo_tail_ptr = m_infifo_head_ptr = 0;
	dectalk_outfifo_check();
}

// helper for dsp infifo_semaphore flag to make dealing with interrupts easier
void dectalk_state::dectalk_semaphore_w ( UINT16 data )
{
	m_infifo_semaphore = data&1;
	if ((m_infifo_semaphore == 1) && (m_m68k_spcflags_latch&0x40))
	{
#ifdef VERBOSE
		logerror("speech int fired!\n");
#endif
		m_maincpu->set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	}
	else
	m_maincpu->set_input_line_and_vector(M68K_IRQ_5, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

// read the output fifo and set the interrupt line active on the dsp
UINT16 dectalk_state::dectalk_outfifo_r (  )
{
	UINT16 data = 0xFFFF;
#ifdef USE_LOOSE_TIMING_OUTPUT
	// if outfifo count is less than two, boost the interleave to prevent running the fifo out
	if (m_outfifo_count < 2)
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef VERBOSE
	if (m_outfifo_count == 0) logerror("output fifo is EMPTY! repeating previous sample!\n");
#endif
	data = m_outfifo[m_outfifo_tail_ptr];
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	if (m_outfifo_count > 0)
	{
		m_outfifo_tail_ptr++;
		m_outfifo_count--;
	}
	m_outfifo_tail_ptr&=0xF;
	dectalk_outfifo_check();
	return ((data&0xfff0)^0x8000); // yes this is right, top bit is inverted and bottom 4 are ignored
	//return data; // not right but want to get it working first
}

/* Machine reset and friends: stuff that needs setting up which IS directly affected by reset */
WRITE_LINE_MEMBER(dectalk_state::dectalk_reset)
{
	m_hack_self_test = 0; // hack
	// stuff that is DIRECTLY affected by the RESET line
	machine().device<x2212_device>("x2212")->recall(0);
	machine().device<x2212_device>("x2212")->recall(1);
	machine().device<x2212_device>("x2212")->recall(0); // nvram recall
	m_m68k_spcflags_latch = 1; // initial status is speech reset(d0) active and spc int(d6) disabled
	m_m68k_tlcflags_latch = 0; // initial status is tone detect int(d6) off, answer phone(d8) off, ring detect int(d14) off
	machine().device("duartn68681")->reset(); // reset the DUART
	// stuff that is INDIRECTLY affected by the RESET line
	dectalk_clear_all_fifos(); // speech reset clears the fifos, though we have to do it explicitly here since we're not actually in the m68k_spcflags_w function.
	dectalk_semaphore_w(0); // on the original DECtalk DTC-01 pcb revision, this is a semaphore for the INPUT fifo, later dec hacked on a check for the 3 output fifo chips to see if they're in sync, and set both of these latches if true.
	m_spc_error_latch = 0; // spc error latch is cleared on /reset
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // speech reset forces the CLR line active on the tms32010
	m_tlc_tonedetect = 0; // TODO, needed for selftest pass
	m_tlc_ringdetect = 0; // TODO
	m_tlc_dtmf = 0; // TODO
	m_duart_inport = 0xF;
	m_duart_outport = 0;
}

void dectalk_state::machine_reset()
{
	/* hook the RESET line, which resets a slew of other components */
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(dectalk_state::dectalk_reset),this));
}

/* Begin 68k i/o handlers */

READ8_MEMBER(dectalk_state::nvram_recall)// recall from x2212 nvram chip
{
#ifdef NVRAM_LOG
	fprintf(stderr,"NVRAM RECALL executed: offset %03x\n", offset);
#endif
	m_nvram->recall(0);
	m_nvram->recall(1);
	m_nvram->recall(0);
	return 0xFF;
}

WRITE8_MEMBER(dectalk_state::led_write)
{
	popmessage("LED status: %02X\n", data&0xFF);
#ifdef VERBOSE
	logerror("m68k: LED status: %02X\n", data&0xFF);
#endif
	//popmessage("LED status: %x %x %x %x %x %x %x %x\n", data&0x80, data&0x40, data&0x20, data&0x10, data&0x8, data&0x4, data&0x2, data&0x1);
}

WRITE8_MEMBER(dectalk_state::nvram_store) // store to X2212 NVRAM chip
{
#ifdef NVRAM_LOG
		fprintf(stderr,"NVRAM STORE executed: offset %03x, data written (and ignored) is %02x\n", offset, data);
#endif
	m_nvram->store(0);
	m_nvram->store(1);
	m_nvram->store(0);
}

WRITE16_MEMBER(dectalk_state::m68k_infifo_w)// 68k write to the speech input fifo
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_68K
	logerror("m68k: SPC infifo written with data = %04X, fifo head was: %02X; fifo tail: %02X\n",data, m_infifo_head_ptr, m_infifo_tail_ptr);
#endif
	// if fifo is full (head ptr = tail ptr-1), do not increment the head ptr and do not store the data
	if (m_infifo_count == 32)
	{
#ifdef SPC_LOG_68K
		logerror("infifo was full, write ignored!\n");
#endif
		return;
	}
	m_infifo[m_infifo_head_ptr] = data;
	m_infifo_head_ptr++;
	m_infifo_count++;
	m_infifo_head_ptr&=0x1F;
}

READ16_MEMBER(dectalk_state::m68k_spcflags_r)// 68k read from the speech flags
{
	UINT8 data = 0;
	data |= m_m68k_spcflags_latch; // bits 0 and 6
	data |= m_spc_error_latch<<5; // bit 5
	data |= m_infifo_semaphore<<7; // bit 7
#ifdef SPC_LOG_68K
	logerror("m68k: SPC flags read, returning data = %04X\n",data);
#endif
	return data;
}

WRITE16_MEMBER(dectalk_state::m68k_spcflags_w)// 68k write to the speech flags (only 3 bits do anything)
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_68K
	logerror("m68k: SPC flags written with %04X, only storing %04X\n",data, data&0x41);
#endif
	m_m68k_spcflags_latch = data&0x41; // ONLY store bits 6 and 0!
	// d0: initialize speech flag (reset tms32010 and clear infifo and outfifo if high)
	if ((data&0x1) == 0x1) // bit 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x01: initialize speech: fifos reset, clear error+semaphore latches and dsp reset\n");
#endif
		dectalk_clear_all_fifos();
		m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // speech reset forces the CLR line active on the tms32010
		// clear the two speech side latches
		m_spc_error_latch = 0;
		dectalk_semaphore_w(0);
	}
	else // (data&0x1) == 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x01 = 0: initialize speech off, dsp running\n");
#endif
		m_dsp->set_input_line(INPUT_LINE_RESET, CLEAR_LINE); // speech reset deassert clears the CLR line on the tms32010
	}
	if ((data&0x2) == 0x2) // bit 1 - clear error and semaphore latches
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x02: clear error+semaphore latches\n");
#endif
		// clear the two speech side latches
		m_spc_error_latch = 0;
		dectalk_semaphore_w(0);
	}
	if ((data&0x40) == 0x40) // bit 6 - spc irq enable
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x40: speech int enabled\n");
#endif
		if (m_infifo_semaphore == 1)
		{
#ifdef SPC_LOG_68K
			logerror("    speech int fired!\n");
#endif
			m_maincpu->set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because semaphore was set
		}
	}
	else // data&0x40 == 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x40 = 0: speech int disabled\n");
#endif
		m_maincpu->set_input_line_and_vector(M68K_IRQ_5, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
}

READ16_MEMBER(dectalk_state::m68k_tlcflags_r)// dtmf flags read
{
	UINT16 data = 0;
	data |= m_m68k_tlcflags_latch; // bits 6, 8, 14;
	data |= m_tlc_tonedetect<<7; // bit 7 is tone detect
	data |= m_tlc_ringdetect<<14; // bit 15 is ring detect
#ifdef TLC_LOG
	logerror("m68k: TLC flags read, returning data = %04X\n",data);
#endif
	return data;
}

WRITE16_MEMBER(dectalk_state::m68k_tlcflags_w)// dtmf flags write
{
#ifdef TLC_LOG
	logerror("m68k: TLC flags written with %04X, only storing %04X\n",data, data&0x4140);
#endif
	m_m68k_tlcflags_latch = data&0x4140; // ONLY store bits 6 8 and 14!
	if ((data&0x40) == 0x40) // bit 6: tone detect interrupt enable
	{
#ifdef TLC_LOG
		logerror(" | 0x40: tone detect int enabled\n");
#endif
		if (m_tlc_tonedetect == 1)
		{
#ifdef TLC_LOG
			logerror("    TLC int fired!\n");
#endif
			m_maincpu->set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because tone detect was set
		}
	}
	else // data&0x40 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x40 = 0: tone detect int disabled\n");
#endif
	if (((data&0x4000)!=0x4000) || (m_tlc_ringdetect == 0)) // check to be sure we don't disable int if both ints fired at once
		m_maincpu->set_input_line_and_vector(M68K_IRQ_4, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
	if ((data&0x100) == 0x100) // bit 8: answer phone relay enable
	{
#ifdef TLC_LOG
		logerror(" | 0x100: answer phone relay enabled\n");
#endif
	}
	else // data&0x100 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x100 = 0: answer phone relay disabled\n");
#endif
	}
	if ((data&0x4000) == 0x4000) // bit 14: ring int enable
	{
#ifdef TLC_LOG
		logerror(" | 0x4000: ring detect int enabled\n");
#endif
		if (m_tlc_ringdetect == 1)
		{
#ifdef TLC_LOG
			logerror("    TLC int fired!\n");
#endif
			m_maincpu->set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because tone detect was set
		}
	}
	else // data&0x4000 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x4000 = 0: ring detect int disabled\n");
#endif
	if (((data&0x40)!=0x40) || (m_tlc_tonedetect == 0)) // check to be sure we don't disable int if both ints fired at once
		m_maincpu->set_input_line_and_vector(M68K_IRQ_4, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
}

READ16_MEMBER(dectalk_state::m68k_tlc_dtmf_r)// dtmf chip read
{
#ifdef TLC_LOG
	UINT16 data = 0xFFFF;
	data = m_tlc_dtmf&0xF;
	logerror("m68k: TLC dtmf detector read, returning data = %02X", data);
#endif
	return 0;
}
/* End 68k i/o handlers */

/* Begin tms32010 i/o handlers */
WRITE16_MEMBER(dectalk_state::spc_latch_outfifo_error_stats)// latch 74ls74 @ E64 upper and lower halves with d0 and 1 respectively
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_DSP
	logerror("dsp: set fifo semaphore and set error status = %01X\n",data&1);
#endif
	dectalk_semaphore_w((~m_simulate_outfifo_error)&1); // always set to 1 here, unless outfifo desync-between-the-three-parallel-fifo-chips error occurs.
	m_spc_error_latch = (data&1); // latch the dsp 'soft error' state aka "ERROR DETECTED D5 H" on schematics (different from the outfifo error state above!)
}

READ16_MEMBER(dectalk_state::spc_infifo_data_r)
{
	UINT16 data = 0xFFFF;
	data = m_infifo[m_infifo_tail_ptr];
#ifdef SPC_LOG_DSP
	logerror("dsp: SPC infifo read with data = %04X, fifo head: %02X; fifo tail was: %02X\n",data, m_infifo_head_ptr, m_infifo_tail_ptr);
#endif
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	if (m_infifo_count > 0)
	{
		m_infifo_tail_ptr++;
		m_infifo_count--;
	}
	m_infifo_tail_ptr&=0x1F;
	return data;
}

WRITE16_MEMBER(dectalk_state::spc_outfifo_data_w)
{
	// the low 4 data bits are thrown out on the real unit due to use of a 12 bit dac (and to save use of another 16x4 fifo chip), though technically they're probably valid, and with suitable hacking a dtc-01 could probably output full 16 bit samples at 10khz.
#ifdef SPC_LOG_DSP
	logerror("dsp: SPC outfifo write, data = %04X, fifo head was: %02X; fifo tail: %02X\n", data, m_outfifo_head_ptr, m_outfifo_tail_ptr);
#endif
	m_dsp->set_input_line(0, CLEAR_LINE); //TMS32010 INT (cleared because LDCK inverts the IR line, clearing int on any outfifo write... for a moment at least.)
	// if fifo is full (head ptr = tail ptr-1), do not increment the head ptr and do not store the data
	if (m_outfifo_count == 16)
	{
#ifdef SPC_LOG_DSP
		logerror("outfifo was full, write ignored!\n");
#endif
		return;
	}
	m_outfifo[m_outfifo_head_ptr] = data;
	m_outfifo_head_ptr++;
	m_outfifo_count++;
	m_outfifo_head_ptr&=0xF;
	//dectalk_outfifo_check(); // outfifo check should only be done in the audio 10khz polling function
}

READ16_MEMBER(dectalk_state::spc_semaphore_r)// Return state of d-latch 74ls74 @ E64 'lower half' in d0 which indicates whether infifo is readable
{
#ifdef SPC_LOG_DSP
	//logerror("dsp: read infifo semaphore, returned %d\n", m_infifo_semaphore); // commented due to extreme annoyance factor
	if (!m_infifo_semaphore) logerror("dsp: read infifo semaphore, returned %d\n", m_infifo_semaphore);
#endif
	return m_infifo_semaphore;
}
/* end tms32010 i/o handlers */


/******************************************************************************
 Address Maps
******************************************************************************/
/*
Address maps (x = ignored; * = selects address within this range)
68k address map:
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
0   x   x   x   0   x   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROM
0   x   x   x   1   x   x   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  RAM (first 4 chip pairs)
0   x   x   x   1   x   x   1   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  RAM (last chip pair)
0   x   x   x   1   x   x   1   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x   0       W   Status LED <d7-d0>
0   x   x   x   1   x   x   1   0   1   x   x   x   x   0   *   *   *   *   *   *   *   *   1       RW  NVRAM (read/write volatile ram, does not store to eeprom)
0   x   x   x   1   x   x   1   0   1   x   x   x   x   1   *   *   *   *   *   *   *   *   1       RW  NVRAM (all reads do /recall from eeprom, all writes do /store to eeprom)
0   x   x   x   1   x   x   1   1   0   x   x   x   x   x   x   x   x   x   *   *   *   *   x       RW  DUART (keep in mind that a0 is not connected)
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   0   0   *       RW  SPC flags: fifo writable (readonly, d7), spc irq suppress (readwrite, d6), fifo error status (readonly, d5), 'fifo release'/clear-tms-fifo-error-status-bits (writeonly, d1), speech initialize/clear (readwrite, d0) [see schematic sheet 4]
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   0   1   0?      W   SPC fifo write (clocks fifo)
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   1   0   *       RW  TLC flags: ring detect (readonly, d15), ring detected irq enable (readwrite, d14), answer phone (readwrite, d8), tone detected (readonly, d7), tone detected irq enable (readwrite, d6) [see schematic sheet 6]
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   1   1   *       R   TLC tone chip read, reads on bits d0-d7 only, d4-d7 are tied low; d15-d8 are probably open bus
              |               |               |               |               |
*/

static ADDRESS_MAP_START(m68k_mem, AS_PROGRAM, 16, dectalk_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_MIRROR(0x740000) /* ROM */
	AM_RANGE(0x080000, 0x093fff) AM_RAM AM_MIRROR(0x760000) /* RAM */
	AM_RANGE(0x094000, 0x0943ff) AM_WRITE8(led_write, 0x00FF) AM_MIRROR(0x763C00) /* LED array */
	AM_RANGE(0x094000, 0x0941ff) AM_DEVREADWRITE8("x2212", x2212_device, read, write, 0xFF00) AM_MIRROR(0x763C00) /* Xicor X2212 NVRAM */
	AM_RANGE(0x094200, 0x0943ff) AM_READWRITE8(nvram_recall, nvram_store, 0xFF00) AM_MIRROR(0x763C00) /* Xicor X2212 NVRAM */
	AM_RANGE(0x098000, 0x09801f) AM_DEVREADWRITE8("duartn68681", mc68681_device, read, write, 0xff ) AM_MIRROR(0x763FE0) /* DUART */
	AM_RANGE(0x09C000, 0x09C001) AM_READWRITE(m68k_spcflags_r, m68k_spcflags_w) AM_MIRROR(0x763FF8) /* SPC flags reg */
	AM_RANGE(0x09C002, 0x09C003) AM_WRITE(m68k_infifo_w) AM_MIRROR(0x763FF8) /* SPC fifo reg */
	AM_RANGE(0x09C004, 0x09C005) AM_READWRITE(m68k_tlcflags_r, m68k_tlcflags_w) AM_MIRROR(0x763FF8) /* telephone status flags */
	AM_RANGE(0x09C006, 0x09C007) AM_READ(m68k_tlc_dtmf_r) AM_MIRROR(0x763FF8) /* telephone dtmf read */
ADDRESS_MAP_END

// do we even need this below?
static ADDRESS_MAP_START(m68k_io, AS_IO, 16, dectalk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms32010_mem, AS_PROGRAM, 16, dectalk_state )
	AM_RANGE(0x000, 0x7ff) AM_ROM /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms32010_io, AS_IO, 16, dectalk_state )
	AM_RANGE(0, 0) AM_WRITE(spc_latch_outfifo_error_stats) // *set* the outfifo_status_r semaphore, and also latch the error bit at D0.
	AM_RANGE(1, 1) AM_READWRITE(spc_infifo_data_r, spc_outfifo_data_w) //read from input fifo, write to sound fifo
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(spc_semaphore_r) //read infifo-has-data-in-it fifo readable status
	//AM_RANGE(8, 8) //the newer firmware seems to want something mapped here?
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( dectalk )
PORT_START("duart_in") // IP4, IP5, IP6 bits on duart are dipswitches (really unsoldered holes in the pcb where jumper wires can be soldered or shorted over)
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // these bits are dealt with elsewhere
	PORT_DIPNAME( 0x10, 0x00, "Skip Self Test (IP4)" )
	PORT_DIPSETTING(    0x10, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown (IP5)" )
	PORT_DIPSETTING(    0x20, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown (IP6)" )
	PORT_DIPSETTING(    0x40, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // this pin (IP7) doesn't actually exist as a pin at all, reads as 1

PORT_START("hacks")
	PORT_CONFNAME( 0x01, 0x01, "Hack to prevent hang when skip self test is shorted" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
void dectalk_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_OUTFIFO_READ:
		outfifo_read_cb(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in dectalk_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(dectalk_state::outfifo_read_cb)
{
	UINT16 data;
	data = dectalk_outfifo_r();
#ifdef VERBOSE
	if (data!= 0x8000) logerror("sample output: %04X\n", data);
#endif
	timer_set(attotime::from_hz(10000), TIMER_OUTFIFO_READ);
	m_dac->write_signed16(data);
	// hack for break key, requires hacked up duart core so disabled for now
	// also it doesn't work well, the setup menu is badly corrupt
	/*device_t *duart = machine().device("duartn68681");
	if (machine.input().code_pressed(KEYCODE_F1))
	    duart68681_rx_break(duart, 1, 1);
	else
	    duart68681_rx_break(duart, 1, 0);*/
}

/* Driver init: stuff that needs setting up which isn't directly affected by reset */
DRIVER_INIT_MEMBER(dectalk_state,dectalk)
{
	dectalk_clear_all_fifos();
	m_simulate_outfifo_error = 0;
	timer_set(attotime::from_hz(10000), TIMER_OUTFIFO_READ);
}

static MACHINE_CONFIG_START( dectalk, dectalk_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2) /* E74 20MHz OSC (/2) */
	MCFG_CPU_PROGRAM_MAP(m68k_mem)
	MCFG_CPU_IO_MAP(m68k_io)
	MCFG_MC68681_ADD( "duartn68681", XTAL_3_6864MHz ) /* 2681 duart (not 68681!); Y3 3.6864MHz Xtal */
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(dectalk_state, dectalk_duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(WRITELINE(dectalk_state, dectalk_duart_txa))
	MCFG_MC68681_B_TX_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_MC68681_INPORT_CALLBACK(READ8(dectalk_state, dectalk_duart_input))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(dectalk_state, dectalk_duart_output))

	MCFG_CPU_ADD("dsp", TMS32010, XTAL_20MHz) /* Y1 20MHz xtal */
	MCFG_CPU_PROGRAM_MAP(tms32010_mem)
	MCFG_CPU_IO_MAP(tms32010_io)
#ifdef USE_LOOSE_TIMING
	MCFG_QUANTUM_TIME(attotime::from_hz(100))
#else
	MCFG_QUANTUM_PERFECT_CPU("dsp")
#endif

	MCFG_X2212_ADD("x2212")

	/* video hardware */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0) /* E88 10KHz OSC, handled by timer */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.9)

	/* Y2 is a 3.579545 MHz xtal for the dtmf decoder chip */

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("duartn68681", mc68681_device, rx_b_w))
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( dectalk )
	ROM_REGION16_BE(0x40000,"maincpu", 0)
	// DECtalk DTC-01 firmware v2.0 (first half: 23Jul84 tag; second half: 02Jul84 tag), all roms are 27128 eproms
	// technically the correct rom names are 23-123e5.e8, etc, but the chips they were dumped from were NOT labeled that way
	// labels were SP8510123E5 etc, which means the chips were burned at dec in week 10, 1985
	/* the labels dec uses for most non-factory-programmed eproms, proms and pals is something like SPddddnnnto or WBddddnnto
	 * where:
	 * SP or WB = ?
	 * dddd is a 4 digit datecode with yyww (i.e. 8510 = week 10, 1985)
	 * nnn = the dec internal rom number for that type
	 * t = programmable chip type (e = eprom, otprom or mask rom; a,b,f = proms of various sorts; there are others for plas and pals)
	 * o = size of rom
	 *   for eproms/otproms or mask roms it is: e1 = 0x400, e2 = 0x800, e3 = 0x1000, e4 = 0x2000, e5 = 0x4000, e6 = 0x8000, etc)
	 *   for proms it is: a1 = 82s123(0x20, 8b TS); a2 = 82s129(0x100 4b TS); a9 = 82s131(0x200 4b TS); b1 = 82s135(0x100 8b TS); f1 = 82s137(0x400 4b TS); f4 = 82s191(0x800 8b TS); s0 = MC68HC05; m2 = i8051 or other MCS-51; (there are more)
	 */
	ROM_SYSTEM_BIOS( 0, "v20", "DTC-01 Version 2.0")
	ROMX_LOAD("23-123e5.e8", 0x00000, 0x4000, CRC(03e1eefa) SHA1(e586de03e113683c2534fca1f3f40ba391193044), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510123E5" @ E8
	ROMX_LOAD("23-119e5.e22", 0x00001, 0x4000, CRC(af20411f) SHA1(7954bb56b7591f8954403a22d34de31c7d5441ac), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510119E5" @ E22
	ROMX_LOAD("23-124e5.e7", 0x08000, 0x4000, CRC(9edeafcb) SHA1(7724babf4ae5d77c0b4200f608d599058d04b25c), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510124E5" @ E7
	ROMX_LOAD("23-120e5.e21", 0x08001, 0x4000, CRC(f2a346a6) SHA1(af5e4ea0b3631f7d6f16c22e86a33fa2cb520ee0), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510120E5" @ E21
	ROMX_LOAD("23-125e5.e6", 0x10000, 0x4000, CRC(1c0100d1) SHA1(1b60cd71dfa83408b17e13f683b6bf3198c905cc), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510125E5" @ E6
	ROMX_LOAD("23-121e5.e20", 0x10001, 0x4000, CRC(4cb081bd) SHA1(4ad0b00628a90085cd7c78a354256c39fd14db6c), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510121E5" @ E20
	ROMX_LOAD("23-126e5.e5", 0x18000, 0x4000, CRC(7823dedb) SHA1(e2b2415eec838ddd46094f2fea93fd289dd0caa2), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510126E5" @ E5
	ROMX_LOAD("23-122e5.e19", 0x18001, 0x4000, CRC(b86370e6) SHA1(92ab22a24484ad0d0f5c8a07347105509999f3ee), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510122E5" @ E19
	ROMX_LOAD("23-103e5.e4", 0x20000, 0x4000, CRC(35aac6b9) SHA1(b5aec0bf37a176ff4d66d6a10357715957662ebd), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510103E5" @ E4
	ROMX_LOAD("23-095e5.e18", 0x20001, 0x4000, CRC(2296fe39) SHA1(891f3a3b4ce75ef14001257bc8f1f60463a9a7cb), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510095E5" @ E18
	ROMX_LOAD("23-104e5.e3", 0x28000, 0x4000, CRC(9658b43c) SHA1(4d6808f67cbdd316df23adc8ddf701df57aa854a), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510104E5" @ E3
	ROMX_LOAD("23-096e5.e17", 0x28001, 0x4000, CRC(cf236077) SHA1(496c69e52cfa013173f7b9c500ce544a03ad01f7), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510096E5" @ E17
	ROMX_LOAD("23-105e5.e2", 0x30000, 0x4000, CRC(09cddd28) SHA1(de0c25687bab3ff0c88c98622092e0b58331aa16), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510105E5" @ E2
	ROMX_LOAD("23-097e5.e16", 0x30001, 0x4000, CRC(49434da1) SHA1(c450abae0ccf372d7eb87370b8a8c97a45e164d3), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510097E5" @ E16
	ROMX_LOAD("23-106e5.e1", 0x38000, 0x4000, CRC(a389ab31) SHA1(355348bfc96a04193136cdde3418366e6476c3ca), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510106E5" @ E1
	ROMX_LOAD("23-098e5.e15", 0x38001, 0x4000, CRC(3d8910e7) SHA1(01921e77b46c2d4845023605239c45ffa4a35872), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "SP8510098E5" @ E15

	/* the undumped 2.0 (beta?) version likely has roms:
	23-091e5.e22, 23-092e5.e21, 23-093e5.e20, 23-094e5.e19,
	23-099e5.e8, 23-100e5.e7, 23-101e5.e6, 23-102e5.e5
	and shares the 23-103e5 thru 106e5, and 095e5 thru 098e5 roms with
	the 2.0 version above
	*/

	// DECtalk DTC-01 firmware v1.8 (first half: 05Dec83 tag; second half: 11Oct83 tag), all roms are 27128 eproms
	ROM_SYSTEM_BIOS( 1, "v18", "DTC-01 Version 1.8")
	ROMX_LOAD("23-063e5.e8", 0x00000, 0x4000, CRC(9f5ca045) SHA1(1b1b9c1e092c44329b385fb04001e13422eb8d39), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-059e5.e22", 0x00001, 0x4000, CRC(b299cf64) SHA1(84bbe9ff303ea6ce7b1c0b1ad05421edd18fae49), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-064e5.e7", 0x08000, 0x4000, CRC(e4ff20f4) SHA1(fdd91e4d2ef92608a08b2e78b6108e31ff53a1f9), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-060e5.e21", 0x08001, 0x4000, CRC(213c65ba) SHA1(c95662d0d40499af01cdc23f05936762ab54081a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-065e5.e6", 0x10000, 0x4000, CRC(38ea0c75) SHA1(232b622cef6d69a493db1ed02e5236235c68daba), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-061e5.e20", 0x10001, 0x4000, CRC(44f6fe5c) SHA1(81daa4abae273c7f0aead902b5c3c842f7e7f116), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-066e5.e5", 0x18000, 0x4000, CRC(957aa8cf) SHA1(5f9f916b99867d1adbafd58d411feb630f6e4b6d), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-062e5.e19", 0x18001, 0x4000, CRC(10ab969c) SHA1(46ee22a295b8709b6f829751aca5f92e4f459a9f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-032e5.e4", 0x20000, 0x4000, CRC(0f805e3a) SHA1(1d8008e30a448358224364fd8237dbb08907b219), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-031e5.e18", 0x20001, 0x4000, CRC(846b5b68) SHA1(55c759b3fb927d2dfc9d77e8e080748866bea854), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-034e5.e3", 0x28000, 0x4000, CRC(90700738) SHA1(738337c5b6acd3f30c3c4be2457370d2ce9313f9), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-033e5.e17", 0x28001, 0x4000, CRC(48756a4d) SHA1(5946ccd367d88a484bb1549d0cc990b9b7d88f0c), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-036e5.e2", 0x30000, 0x4000, CRC(5c2d4f73) SHA1(30f95e5383c4f71bc700346e2d49e8ad70b94c8c), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-035e5.e16", 0x30001, 0x4000, CRC(80116443) SHA1(7b3b68e61b421dedaad88b5600c739943a316c9e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-038e5.e1", 0x38000, 0x4000, CRC(c840493f) SHA1(abd6af442690e981a9089f19febffc8f3fb52717), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("23-037e5.e15", 0x38001, 0x4000, CRC(d62ab309) SHA1(a743a23625feadf6e46ef889e2bb04af88589992), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION(0x2000,"dsp", 0)
	// NEWER/final? firmware 'later 2.0'; this firmware DOES WORK.
	// this firmware seems to have some leftover test garbage mapped into its space, which is not present on the dtc-01 board
	// it writes 0x0000 to 0x90 on start
	// it writes a sequence of values to 0xFF down to 0xE9
	// it wants something readable mapped at 0x08 or else it waits for an interrupt
	// Is this the same firmware as on the tms320P15 on the dtc-07 or a backported variant of such?
	ROM_LOAD16_BYTE("23-410f4.e70", 0x000, 0x800, CRC(121e2ec3) SHA1(3fabe018d0e0b478093951cb20501853358faa18))
	ROM_LOAD16_BYTE("23-409f4.e69", 0x001, 0x800, CRC(61f67c79) SHA1(9a13426c92f879f2953f180f805990a91c37ac43))
	// DECtalk DTC-01 'klsyn' tms32010 firmware 'earlier 2.0', both proms are 82s191 equivalent; this firmware DOES WORK.
	ROM_LOAD16_BYTE("23-205f4.e70", 0x000, 0x800, CRC(ed76a3ad) SHA1(3136bae243ef48721e21c66fde70dab5fc3c21d0)) // Label: "LM8506205F4 // M1-76161-5" @ E70
	ROM_LOAD16_BYTE("23-204f4.e69", 0x001, 0x800, CRC(79bb54ff) SHA1(9409f90f7a397b041e4440341f2d7934cb479285)) // Label: "LM8504204F4 // 78S191" @ E69
	// older dsp firmware from dectalk firmware 1.8; this firmware DOES WORK, and even works with 2.0 dectalk firmware! its a bit quieter than the others, though.
	ROM_LOAD16_BYTE("23-166f4.e70", 0x000, 0x800, CRC(2d036ffc) SHA1(e8c25ca092dde2dc0aec73921af806026bdfbbc3)) // HM1-76161-5
	ROM_LOAD16_BYTE("23-165f4.e69", 0x001, 0x800, CRC(a3019ca4) SHA1(249f269c38f7f44edb6d025bcc867c8ca0de3e9c)) // HM1-76161-5

	// TODO: load this as default if the nvram file is missing, OR get the setup page working enough that it can be saved properly to the chip from an NVR FAULT state!
	// NOTE: this nvram image is ONLY VALID for v2.0; v1.8 expects a different image.
	ROM_REGION(0x100,"nvram", 0) // default nvram image is at 0x1A7AE in main rom, read lsn first so 0x0005 in rom becomes 05 00 00 00 etc for all words of main rom
	ROM_FILL(0x00, 0xff, 0x00) // blank it first;
	ROM_FILL(0x00, 0x01, 0x05)
	ROM_FILL(0x04, 0x01, 0x00)
	ROM_FILL(0x08, 0x01, 0x06)
	ROM_FILL(0x0C, 0x01, 0x01)
	ROM_FILL(0x10, 0x01, 0x06)
	ROM_FILL(0x14, 0x01, 0x0B)
	ROM_FILL(0x18, 0x01, 0x02)
	ROM_FILL(0x1C, 0x01, 0x02)
	ROM_FILL(0x20, 0x01, 0x01)
	ROM_FILL(0x24, 0x01, 0x01)
	ROM_FILL(0x28, 0x01, 0x00)
	ROM_FILL(0x2C, 0x01, 0x01)
	ROM_FILL(0xFC, 0x01, 0x0D) // checksum, calculated some weird way which I haven't figured out yet
	ROM_FILL(0xFD, 0x01, 0x02) // "
	ROM_FILL(0xFE, 0x01, 0x05) // "
	ROM_FILL(0xFF, 0x01, 0x0B) // "

ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT      COMPANY     FULLNAME            FLAGS */
COMP( 1984, dectalk,    0,      0,      dectalk,    dectalk, dectalk_state, dectalk,  "Digital Equipment Corporation",      "DECtalk DTC-01",   MACHINE_NOT_WORKING )
