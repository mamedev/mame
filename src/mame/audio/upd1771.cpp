// license:BSD-3-Clause
// copyright-holders:David Viens
/**********************************************************************

    NEC uPD1771-017 as used in the Epoch Super Cassette Vision (SCV)

    Made using recording/analysis on a Yeno (PAL Super Cassete Vision)
    by plgDavid

    Full markings on my 2 specimens are
    "NEC JAPAN 8431K9 D1771C 017" (31st week of 1984, mask rom #017)

    I've since (October 2012) got a Grandstand Firefox F-7 Handheld game
    (AKA Epoch GalagaX6/Epoch Astro Thunder 7/Tandy Astro Thunder),
    (http://www.handheldmuseum.com/Grandstand/Firefox.htm)
    which includes a
    "NEC JAPAN 8319K9 D1771C 011" (19th week of 1983, mask rom #011)
    Thanks to user 'Blanka' from Dragonslairfans for the nice catch!
    (http://www.dragonslairfans.com/smfor/index.php?topic=3061.0)

    Since the chip generates tones using ROM wavetables,
    it is perfectly possible to generate other sounds with different rom code and data.

    Most upd17XXX devices are typically 4bit NEC MCUs, however based on information
    in "Electronic Speech Synthesis" by Geoff Bristow (ISBN 0-07-007912-9, pages 148-152)
    the upd1770/1771 is not one of these 4-bit ones.

    The uPD1770/uPD1771 SSM is a 16-bit-wide rom/ram mcu with 8kb (4kw) of rom code,
    64 bytes of ram (16x16bit words addressable as 16 or 2x8 bits each, the
    remaining 32 bytes acting as an 8-level stack), 182 instructions, a complex
    noise and tone internal interrupt system, external interrupts,
    and two 8-bit ports with multiple modes allowing for chips to operate as master
    or slave devices.
    SSM stands for "Sound Synthesis Microcomputer".

    People who I *THINK* worked on the uPD1771 and what part I think they worked on:
    Toshio Oura - Project Lead(?), VSRSSS/TSRSSS speech synthesis engine (on upd1776C), master/slave i/o controls, author of bristow article and primary author of the IEEE article
    Hatsuhide Igarashi - Clock oscillator and pad layout, coauthor on the IEEE article, other IEEE stuff
    Tomoaki Isozaki - ? (senior NEC engineer?), coauthor on the IEEE article
    Sachiyuki Toufuku - ?, coauthor on the IEEE article
    Tojiro Mukawa - IGFETs and the DAC
    M. Sakai ? - digital filtering for VSRSSS? (IEEE 4131979, 1169295)
    M. Endo ? - digital design system or speech synthesis? (IEEE 4069656, another? person: IEEE 150330, 225838)
    H. Aoyama ? - logic design system used to assemble/lay out the chip? (IEEE 1585393)
    I. Fujitaka ? (no IEEE)
    Eiji Sugimoto - cpu design? 1156033 1155824
    F. Tsukuda ? (no IEEE)
    N. Miyake ? switched capacitor stuff? (IEEE nnnnnn)


    The uPD1771 internal workings are described to some extent by the Bristow book
    and the IEEE article "A Single-Chip Sound Synthesis Microcomputer" which complements the book
    and are covered by at least four US patents:
    4184152 - on IGFET-based DAC stuff
    4488061 - on the IGFET-based drive circuit part of the DAC.
    4408094 - covers the 3 pin DAC with the volume control/vref pin. Not all that interesting,
              except it might describe to some extent how the (9->5bit?) PWM works in the text.
    4470113 - covers the multiplexed PB0/1/2/3 pins and their use as /CS /WR /RD and ALE
              note as I have marked the pins below I assume the final pins connected
              to /CS /WR /RD and /ALE are PB7,6,5,4 but this is just a guess of mine:
              The actual order may well match the patent.
    4577343 - covers the VSRSSS implementation as discussed in the Bristow book.
              This patent has an internal diagram of the workings of the chips and
              a limited description of how many registers etc it has.
    4805508 - on the operation of the tone divider register and correction for accurate period when
              the tone interrupt frequency is not perfectly divisible from the clock.
    These next two may not be specific to the 1771 or even related at all!
    4321562 - on a self-adjusting circuit for internal coupling to the clock crystal inputs.
              This may be a generic NEC invention and probably isn't limited to the upd1771.
    4656491 - on a new method of distributing resistors and transistors on anti-ESD pin buffers
              This may be a generic NEC invention and probably isn't limited to the upd1771.


    Based on the 4577343 patent mostly, plus the bristow and IEEE article:
    * these are the registers:
    8bits:
     AH, AL (forming the 16-bit A' accumulator),
     B, C (a pair of general purpose registers),
    4bits (may be technically part of ALU):
     H -> points to one of the 16 words of ram
    1bit:
     L -> selector of left or right half of the ram word
    ?bits:
     D (having to do with the DAC)
     N (3 bits? having to do with the pseudorandom noise interrupt, namely setting the clock divider ratio for the PRNG clock vs cpu clock)
     MODE (5 or more bits? enabling/disabling/acking the noise interrupt, and the tone interrupts (there are four!))
     SP (the stack pointer, probably 5 bits, points to the stack ram; may encompass H and L as above!)
     FLO: unsure. quite possibly 'flag overflow' used for branching. there likely exists other flags as well...
     ODF: 'output data flag?', selects which half of a selected ram word is output to the dac not really sure of this?


    Mask roms known:
    uPD1776C: mentioned in the bristow book, implements VSRSSS speech concatenation
              (see US Patent 4577343 which is a patent on this VSRSSS implementation)
    uPD1771C-006: used in NEC APC for sound as the "MPU"
            -011: used on Firefox F-4/Astro Thunder handheld
            -015: unknown, known to exist from part scalper sites only.
            -017: used on Epoch Super Cassete Vision for sound; This audio driver HLEs that part only.

     Used pinout in the SCV:

     NC           1        28        NC
     NC           2        27        NC
     NC           3        26        ACK
    !WR           4        25        D7
    !CS           5        24        D6
     RESET        6        23        D5
     NC           7        22        D4
     VCC          8        21        D3
     6Mhz XIN     9        20        D2
     6Mhz XOUT   10        19        D1
     AUDOUT      11        18        D0
     NC(recheck!)12        17        GND
     AUDOUT(inv) 13        16        VCC
     GND         14        15        ? tied to pin 16 (VCC) through a resistor (pullup?)

     Pinout based on guesses and information in "Electronic Speech Synthesis" by Geoff Bristow
     (ISBN 0-07-007912-9, pages 148-152), and the data on page 233 of the Nec APC technical manual at
     http://bitsavers.trailing-edge.com/pdf/nec/APC/819-000100-1003_APC_System_Reference_Guide_Apr83.pdf
     I/O pin purpose based on testing by kevtris.
     [x] is unsure:
     PB3        <>  1        28 <>     PB2
     PB4(/ALE)  <>  2        27 <>     PB1
     PB5(/RD)   <>  3        26 <>     PB0
     PB6(/WR)   <>  4        25 <>     D7(PA7)
     PB7(/CS)   <>  5        24 <>     D6(PA6)
     /RESET     ->  6        23 <>     D5(PA5)
     [/TSTOUT?] <-  7        22 <>     D4(PA4)
     VCC        --  8        21 <>     D3(PA3)
     XI(CLK)    ->  9        20 <>     D2(PA2)
     XO         <- 10        19 <>     D1(PA1)
     D/A OUT +  <- 11        18 <>     D0(PA0)
     D/A POWER  -- 12        17 <-     CH2
     D/A OUT -  <- 13        16 <>     /EXTINT and [/TSTOUT2?] (test out is related to pin 15 state)
     GND        -- 14        15 <-     CH1 tied to pin 16 (VCC) through a resistor, on APC to VCC thru a 12k resistor and thru a 10uf cap to gnd

    CH1 and CH2 are mode selects, purpose based on testing by kevtris:
    CH1 CH2
    H   L   - 'master' mode, pb4-pb7 are i/o? /EXTINT is an input
    L   L   - 'slave' mode where pb4-pb7 are /ale /rd /wr /cs? /EXTINT may be an output in this mode?
    X   H   - test mode: the 512 byte 16-bit wide ROM is output sequentially on pins pa7-pa0 and pb7-pb0 for the high and low bytes of each word respectively
    D/A POWER is the FET source for the D/A OUT+ and D/A OUT- push-pull dac drains; it should be tied to VCC or thereabouts

    In the SCV (info from plgDavid):
    pin  5 is tied to the !SCPU pin on the Epoch TV chip pin 29 (0x3600 writes)
    pin  6 is tied to the   PC3 pin of the upD7801 CPU
    pin 26 is tied to the  INT1 pin of the upD7801 (CPU pin 12),

    1,2,3,28,27 don't generate any digital signals
    6 seems to be lowered 2.5 ms before an audio write
    7  is always low.
    12 is always high

    (NOTE: the photomicrograph in the bristow book makes it fairly clear due to
    pad thicknessess that the real VCC is pin 8 and the real GND is pin 14.
    The function of pin 7 is unknown.

    Pins 11 and 13 go to a special circuit, which according to kevtris's analysis
    of my schematics, consist of a balanced output (not unlike XLR cables),
    which are then combined together then sent to the RF box.
    (The bristow book explains that there are two DAC pins and one DAC
    VREF/volume pin. The dac+ and dac- are pins 11 and 13, and based on the
    photomicrograph it looks like dac vref is probably pin 12)

    HLE:
    All writes are made through address 0x3600 on the upD7801
    Instead of using register=value, this chip require sending multiple
    bytes for each command, one after the other.

**********************************************************************/

#include "emu.h"
#include "audio/upd1771.h"


#define LOG 0

/*
  Each of the 8 waveforms have been extracted from the uPD1771c-017 internal
  ROM, from offset 0x1fd (start of first waveform) to offset 0x2fc (end of
  last waveform).
  (note: given test mode dumping offset non-clarity it may be 0x200-0x2ff)
  The waveforms are stored in an 8-bit sign-magnitude format, so if in ROM the
  upper bit is 0x80, invert the lower 7 bits to get the 2's complement result
  seen here.
  Note that only the last 4 waveforms appear to have been intended for use as
  waveforms; the first four look as if they're playing back a piece of code as
  wave data.
*/
const signed char WAVEFORMS[8][32]={
{   0,   0,-123,-123, -61, -23, 125, 107,  94,  83,-128,-128,-128,   0,   0,   0,   1,   1,   1,   1,   0,   0,   0,-128,-128,-128,   0,   0,   0,   0,   0,   0},
{  37,  16,  32, -21,  32,  52,   4,   4,  33,  18,  60,  56,   0,   8,   5,  16,  65,  19,  69,  16,  -2,  19,  37,  16,  97,  19,   0,  87, 127,  -3,   1,   2},
{   0,   8,   1,  52,   4,   0,   0,  77,  81,-109,  47,  97, -83,-109,  38,  97,   0,  52,   4,   0,   1,   4,   1,  22,   2, -46,  33,  97,   0,   8, -85, -99},
{  47,  97,  40,  97,  -3,  25,  64,  17,   0,  52,  12,   5,  12,   5,  12,   5,  12,   5,  12,   5,   8,   4,-114,  19,   0,  52,-122,  21,   2,   5,   0,   8},
{ -52, -96,-118,-128,-111, -74, -37,  -5,  31,  62,  89, 112, 127, 125, 115,  93,  57,  23,   0, -16,  -8,  15,  37,  54,  65,  70,  62,  54,  43,  31,  19,   0},
{ -81,-128, -61,  13,  65,  93, 127,  47,  41,  44,  52,  55,  56,  58,  58,  34,   0,  68,  76,  72,  61, 108,  55,  29,  32,  39,  43,  49,  50,  51,  51,   0},
{ -21, -45, -67, -88,-105,-114,-122,-128,-123,-116,-103, -87, -70, -53, -28,  -9,  22,  46,  67,  86, 102, 114, 123, 125, 127, 117, 104,  91,  72,  51,  28,   0},
{ -78,-118,-128,-102, -54,  -3,  40,  65,  84,  88,  84,  80,  82,  88,  94, 103, 110, 119, 122, 125, 122, 122, 121, 123, 125, 126, 127, 127, 125, 118,  82,   0}
};


#define NOISE_SIZE 255


static unsigned char noise_tbl[]=
{
	0x1c,0x86,0x8a,0x8f,0x98,0xa1,0xad,0xbe,0xd9,0x8a,0x66,0x4d,0x40,0x33,0x2b,0x23,
	0x1e,0x8a,0x90,0x97,0xa4,0xae,0xb8,0xd6,0xec,0xe9,0x69,0x4a,0x3e,0x34,0x2d,0x27,
	0x24,0x24,0x89,0x8e,0x93,0x9c,0xa5,0xb0,0xc1,0xdd,0x40,0x36,0x30,0x29,0x27,0x24,
	0x8b,0x90,0x96,0x9e,0xa7,0xb3,0xc4,0xe1,0x25,0x21,0x8a,0x8f,0x93,0x9d,0xa5,0xb2,
	0xc2,0xdd,0xdd,0x98,0xa2,0xaf,0xbf,0xd8,0xfd,0x65,0x4a,0x3c,0x31,0x2b,0x24,0x22,
	0x1e,0x87,0x8c,0x91,0x9a,0xa3,0xaf,0xc0,0xdb,0xbe,0xd9,0x8c,0x66,0x4d,0x40,0x34,
	0x2c,0x24,0x1f,0x88,0x90,0x9a,0xa4,0xb2,0xc2,0xda,0xff,0x67,0x4d,0x3d,0x34,0x2d,
	0x26,0x24,0x20,0x89,0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xde,0xc1,0xda,0xff,0x67,0x4d,
	0x3d,0x33,0x2d,0x26,0x24,0x20,0x89,0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xdd,0xa3,0xb0,
	0xc0,0xd9,0xfe,0x66,0x4b,0x3c,0x32,0x2b,0x24,0x23,0x1e,0x88,0x8d,0x92,0x9b,0xa4,
	0xb0,0xc1,0xdc,0xad,0xbe,0xda,0x22,0x20,0x1c,0x85,0x8a,0x8f,0x98,0xa1,0xad,0xbe,
	0xda,0x20,0x1b,0x85,0x8d,0x97,0xa1,0xaf,0xbf,0xd8,0xfd,0x64,0x49,0x3a,0x30,0x2a,
	0x23,0x21,0x1d,0x86,0x8b,0x91,0x9a,0xa2,0xae,0xc0,0xdb,0x33,0x2b,0x24,0x1f,0x88,
	0x90,0x9a,0xa4,0xb2,0xc2,0xda,0xff,0x67,0x4c,0x3e,0x33,0x2d,0x25,0x24,0x1f,0x89,
	0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xde,0x85,0x8e,0x98,0xa2,0xb0,0xc0,0xd9,0xfe,0x64,
	0x4b,0x3b,0x31,0x2a,0x23,0x22,0x1e,0x88,0x8c,0x91,0x9b,0xa3,0xaf,0xc1,0xdc,0xdc
};



#define STATE_SILENCE 0
#define STATE_NOISE   1
#define STATE_TONE    2
#define STATE_ADPCM   3


const device_type UPD1771C = &device_creator<upd1771c_device>;


upd1771c_device::upd1771c_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, UPD1771C, "NEC uPD1771C 017", tag, owner, clock, "upd1771c", __FILE__),
					device_sound_interface(mconfig, *this),
					m_ack_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd1771c_device::device_start()
{
	/* resolve callbacks */
	m_ack_handler.resolve_safe();

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(upd1771c_device::ack_callback),this));

	m_channel = machine().sound().stream_alloc(*this, 0, 1, clock() / 4);

	save_item(NAME(m_packet));
	save_item(NAME(m_index));
	save_item(NAME(m_expected_bytes));
	save_item(NAME(m_state));
	save_item(NAME(m_pc3));
	save_item(NAME(m_t_timbre));
	save_item(NAME(m_t_offset));
	save_item(NAME(m_t_period));
	save_item(NAME(m_t_volume));
	save_item(NAME(m_t_tpos));
	save_item(NAME(m_t_ppos));
	save_item(NAME(m_nw_timbre));
	save_item(NAME(m_nw_volume));
	save_item(NAME(m_nw_period));
	save_item(NAME(m_nw_tpos));
	save_item(NAME(m_nw_ppos));
	save_item(NAME(m_n_value));
	save_item(NAME(m_n_volume));
	save_item(NAME(m_n_period));
	save_item(NAME(m_n_ppos));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd1771c_device::device_reset()
{
	m_index = 0;
	m_expected_bytes = 0;
	m_pc3 = 0;
	m_t_tpos = 0;
	m_t_ppos = 0;
	m_state = 0;
	m_nw_tpos = 0;
	memset(m_n_value, 0x00, sizeof(m_n_value));
	memset(m_n_ppos, 0x00, sizeof(m_n_ppos));
}



READ8_MEMBER( upd1771c_device::read )
{
	return 0x80; // TODO
}

/*
*************TONE*****************
Tone consists of a wavetable playback mechanism.
Each wavetable is a looping period of 32 samples but can be played with an offset from any point in the table
effectively shrinking the sample loop, thus allowing different pitch "macros ranges" to be played.
This method is rather crude because the spectrum of the sound get heavily altered...
unless that was the intent.

Tone Write (4 bytes):

Byte0: 0x02

Byte1: 0bTTTOOOOO
  MSB 3 bits of Timbre (8 wavetables)
  LSB 5 bits offset in the table.

Byte2: 0bPPPPPPPP
  8bits of clock divider/period
  Anything under <= 0x20 give the same value

Byte3: 0b???VVVVV
   MSB 3 bits unknown
   LSB 5 bits of "Volume"

   Note: volume is not a volume in a normal sense but some kind
   of bit cropping/rounding.
*/


/*
*************NOISE*****************
Noise consists on 4 different components
A weird Wavetable LFSR (for lack of a better term),
and three independent (but equal) low frequency
50/50 pulse wavs.

The 4 components are mixed in a mysterious way,
a weird ORing with volume having a huge effect.

Byte0: 0x01

Byte1: 0bTTTOOOOO
  MSB 3 bits of LFSR Timbre (8 wavetables)
  LSB 5 bits ?????????

Byte2: 0bPPPPPPPP
  8bits of clock divider/period

Byte3: 0b???VVVVV
   MSB 3 bits unknown
   LSB 5 bits of "Volume"


Byte4: 0bPPPPPPPP  Low Freq0 period(if not 0 this periodically resets the  Wavetable LFSR)
Byte5: 0bPPPPPPPP  Low Freq1 period(if not 0 this periodically resets the  Wavetable LFSR)
Byte6: 0bPPPPPPPP  Low Freq2 period(if not 0 this periodically resets the  Wavetable LFSR)

Byte7: 0b???VVVVV  Low Freq0 volume
Byte8: 0b???VVVVV  Low Freq1 volume
Byte9: 0b???VVVVV  Low Freq2 volume
*/

WRITE8_MEMBER( upd1771c_device::write )
{
	//if (LOG)
	//  logerror( "upd1771_w: received byte 0x%02x\n", data );

	m_ack_handler(0);

	if (m_index < MAX_PACKET_SIZE)
		m_packet[m_index++] = data;
	else
	{
		logerror("upd1771_w: received byte 0x%02x overload!\n", data);
		return;
	}

	switch (m_packet[0])
	{
		case 0:
			m_state = STATE_SILENCE;
			m_index = 0;
			//logerror( "upd1771_w: ----------------silence  state reset\n");
			break;

		case 1:
			if (m_index == 10)
			{
				m_state = STATE_NOISE;
				m_index = 0;

				m_nw_timbre = (m_packet[1] & 0xe0) >> 5;
				m_nw_period = ((UINT32)m_packet[2] + 1) << 7;
				m_nw_volume = m_packet[3] & 0x1f;

				//very long clocked periods.. used for engine drones
				m_n_period[0] = (((UINT32)m_packet[4]) + 1) << 7;
				m_n_period[1] = (((UINT32)m_packet[5]) + 1) << 7;
				m_n_period[2] = (((UINT32)m_packet[6]) + 1) << 7;

				m_n_volume[0] = m_packet[7] & 0x1f;
				m_n_volume[1] = m_packet[8] & 0x1f;
				m_n_volume[2] = m_packet[9] & 0x1f;

				//logerror( "upd1771_w: ----------------noise state reset\n");
			}
			else
				m_timer->adjust(attotime::from_ticks(512, clock()));
			break;

		case 2:
			if (m_index == 4)
			{
				//logerror( "upd1771_w: ----------------tone  state reset\n");
				m_t_timbre = (m_packet[1] & 0xe0) >> 5;
				m_t_offset = (m_packet[1] & 0x1f);
				m_t_period = m_packet[2];
				//smaller periods don't all equal to 0x20
				if (m_t_period < 0x20)
					m_t_period = 0x20;

				m_t_volume =  m_packet[3] & 0x1f;
				m_state = STATE_TONE;
				m_index = 0;
			}
			else
				m_timer->adjust(attotime::from_ticks(512, clock()));
			break;

		case 0x1f:
			//6Khz(ish) DIGI playback

			//end capture
			if (m_index >= 2 && m_packet[m_index - 2] == 0xfe && m_packet[m_index - 1] == 0x00)
			{
				//TODO play capture!
				m_index = 0;
				m_packet[0] = 0;
				m_state = STATE_ADPCM;
			}
			else
				m_timer->adjust(attotime::from_ticks(512, clock()));
			break;

		//garbage: wipe stack
		default:
			m_state = STATE_SILENCE;
			m_index = 0;
			break;
	}
}


WRITE_LINE_MEMBER( upd1771c_device::pcm_write )
{
	//RESET upon HIGH
	if (state != m_pc3)
	{
		logerror("upd1771_pc3 change!: state = %d\n", state);
		m_index = 0;
		m_packet[0] = 0;
	}

	m_pc3 = state;
}


TIMER_CALLBACK_MEMBER( upd1771c_device::ack_callback )
{
	m_ack_handler(1);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd1771c_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	switch (m_state)
	{
		case STATE_TONE:
			//logerror( "upd1771_STATE_TONE samps:%d %d %d %d %d %d\n",(int)samples,
			//    (int)m_t_timbre,(int)m_t_offset,(int)m_t_volume,(int)m_t_period,(int)m_t_tpos);

			while (--samples >= 0)
			{
				*buffer++ = (WAVEFORMS[m_t_timbre][m_t_tpos]) * m_t_volume * 2;

				m_t_ppos++;
				if (m_t_ppos >= m_t_period)
				{
					m_t_tpos++;
					if (m_t_tpos == 32)
						m_t_tpos = m_t_offset;

					m_t_ppos = 0;
				}
			}
			break;

		case STATE_NOISE:
			while (--samples >= 0)
			{
				*buffer = 0;

				//"wavetable-LFSR" component
				int wlfsr_val = ((int)noise_tbl[m_nw_tpos]) - 127;//data too wide

				m_nw_ppos++;
				if (m_nw_ppos >= m_nw_period)
				{
					m_nw_tpos++;
					if (m_nw_tpos == NOISE_SIZE)
						m_nw_tpos = 0;
					m_nw_ppos = 0;
				}

				//mix in each of the noise's 3 pulse components
				char res[3];
				for (int i = 0; i < 3; ++i)
				{
					res[i] = m_n_value[i] * 127;
					m_n_ppos[i]++;
					if (m_n_ppos[i] >= m_n_period[i])
					{
						m_n_ppos[i] = 0;
						m_n_value[i] = !m_n_value[i];
					}
				}
				//not quite, but close.
				*buffer+= (
							(wlfsr_val * m_nw_volume) |
							(res[0] * m_n_volume[0]) |
							(res[1] * m_n_volume[1]) |
							(res[2] * m_n_volume[2])
							) ;

				buffer++;
			}
			break;

		default:
			//fill buffer with silence
			while (--samples >= 0)
			{
				*buffer++ = 0;
			}
			break;
	}
}
