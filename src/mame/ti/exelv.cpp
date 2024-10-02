// license:GPL-2.0+
// copyright-holders:Raphael Nabet,Robbbert
/*
    Experimental exelvision driver

    Raphael Nabet, 2004

    Exelvision was a French company that designed and sold two computers:
    * EXL 100 (1984)
    * EXELTEL (1986), which is mostly compatible with EXL 100, but has an
      integrated V23b modem and 5 built-in programs.  Two custom variants of
      the EXELTEL were designed for chemist's shops and car dealers: they were
      bundled with application-specific business software, bar code reader,
      etc.
    These computer were mostly sold in France and in Europe (Spain); there was
    an Arabic version, too.

    Exelvision was founded by former TI employees, which is why their designs
    use TI components and have architectural reminiscences of the primitive
    TI-99/4 design (both computers are built around a microcontroller, have
    little CPU RAM and must therefore store program data in VRAM, and feature
    I/R keyboard and joysticks)

Specs:
    * main CPU is a variant of tms7020 (exl100) or tms7040 (exeltel).  AFAIK,
      the only difference compared to a stock tms7020/7040 is the SWAP register
      instruction is replaced by a custom microcoded LVDP instruction that
      reads a byte from the VDP VRAM read port; it seems that the first 6 bytes
      of internal ROM (0xF000-0xF005 on an exeltel) are missing, too.
    * in addition to the internal 128-byte RAM and 2kb (exl100) or 4kb
      (exeltel) ROM, there are 2kb of CPU RAM and 64(?)kb (exeltel only?) of
      CPU ROM.
    * I/O is controlled by a tms7041 (exl100) or tms7042 (exeltel) or a variant
      thereof.  Communication with the main CPU is done through some custom
      interface (I think), details are still to be worked out.
    * video: tms3556 VDP with 32kb of VRAM (expandable to 64kb), attached to
      the main CPU.
    * sound: tms5220 speech synthesizer with speech ROM, attached to the I/O
      CPU
    * keyboard and joystick: an I/R interface controlled by the I/O CPU enables
      to use a keyboard and two joysticks
    * mass storage: tape interface controlled by the main CPU

More info about the keyboard:
- The EXL100 has an infrared keyboard and 2 infrared joysticks. Each uses a MC14497 chip, which scans the
   inputs and drives the infrared transmitter. We only emulate the keyboard, and it only works in Exel Basic.
- The receiver uses a TEA1009 amplifier feeding a pair of NE567 PLL lock detectors. The first one detects
   28437Hz and is for the keyboard and one joystick. When lock achieved, it causes INT1 on the subcpu. The
   other NE567 detects 20000Hz, is for the other joystick, and causes INT3 on the subcpu.
- Although a range of 8 metres was claimed, users found it was very critical in regard to correct positioning.
- You don't hold a modifier key; you hit and release, then hit the key being modified. This also breaks
   the natural keyboard emulation. Exel Basic will indicate if a modifier key is active.

STATUS:
* EXL 100
  - Most games do something, see table below
* EXELTEL can get to the inbuilt "cart" but stops with a black screen,
    presumably because the I/O processor is not emulated

STATUS OF SOFTWARE:

SWList name      Status
---------------------------------------------------------------------------
exelbas          works
exelbasp         Cyan screen, hangs at start
exelmax          options 1-4 work, 5-7 do nothing
exeldrum         can get to the menu, which seems useless
exelogo          can type into it but the usual commands get error
exeltext         works but weird
exlpaint         works
exlmodem         it might work, need instructions
capmenkr         works, video corruptions
guppy            works
imagix           video corruptions, can't proceed
pindo            video corruptions, can select a game, how to play?
quizzy           works, video corruptions
tennis           the demo works, didn't try playing
virus            works, 2nd screen is corrupt, press 1 there.
wizord           works


Using the cassette:
- You must be in Exel Basic. There's no motor control.
- To save: SAVE"1"  hit enter, put player in Record, hit Esc, it saves.
- To load: LOAD"1"  hit enter, hit esc, put player in Play, it loads.
- The cassette output is connected to the audio section, so games could use it as a 1-bit dac.

TODO:
    * dump exeltel tms7042 I/O CPU ROM
    * exeltel: everything
    * The joysticks. No schematics of them have been found.
    * Keyboard layout is preliminary.
    * Keyboard response is terrible, but this might be normal.
    * Add support for cassette k7 format - there's heaps of software for it.
*/


#include "emu.h"

#include "cpu/tms7000/tms7000.h"
#include "imagedev/cassette.h"
#include "machine/spchrom.h"
#include "machine/timer.h"
#include "sound/tms5220.h"
#include "sound/spkrdev.h"
#include "video/tms3556.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class exelv_state : public driver_device
{
public:
	exelv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_tms3556(*this, "tms3556")
		, m_tms5220c(*this, "tms5220c")
		, m_cart(*this, "cartslot")
		, m_cass(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_timer_k(*this, "timer_k")
	{ }

	void exeltel(machine_config &config);
	void exl100(machine_config &config);

private:
	required_device<tms7000_device> m_maincpu;
	optional_device<tms7041_device> m_subcpu;
	required_device<tms3556_device> m_tms3556;
	required_device<tms5220c_device> m_tms5220c;
	optional_device<generic_slot_device> m_cart;
	optional_device<cassette_image_device> m_cass;
	optional_device<speaker_sound_device> m_speaker;
	required_ioport_array<8> m_io_keyboard;
	optional_device<timer_device> m_timer_k;

	uint8_t mailbox_wx319_r();
	void mailbox_wx318_w(uint8_t data);
	uint8_t tms7020_porta_r();
	void tms7020_portb_w(uint8_t data);
	uint8_t tms7041_porta_r();
	void tms7041_portb_w(uint8_t data);
	uint8_t tms7041_portc_r();
	void tms7041_portc_w(uint8_t data);
	uint8_t tms7041_portd_r();
	void tms7041_portd_w(uint8_t data);
	uint8_t rom_r(offs_t offset);

	DECLARE_MACHINE_START(exl100);
	DECLARE_MACHINE_START(exeltel);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);
	void machine_reset() override ATTR_COLD;
	void machine_common();

	/* tms7020 i/o ports */
	uint8_t   m_tms7020_portb = 0;

	/* tms7041 i/o ports */
	uint8_t   m_tms7041_portb = 0;
	uint8_t   m_tms7041_portc = 0;
	uint8_t   m_tms7041_portd = 0;
	uint32_t  m_rom_size = 0;

	/* mailbox data */
	uint8_t   m_wx318 = 0;    /* data of 74ls374 labeled wx318 */
	uint8_t   m_wx319 = 0;    /* data of 74sl374 labeled wx319 */

	TIMER_DEVICE_CALLBACK_MEMBER(exelv_hblank_interrupt);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( exelvision_cartridge );
	void tms7020_mem(address_map &map) ATTR_COLD;
	void tms7040_mem(address_map &map) ATTR_COLD;

	// variables for the keyboard
	u8 k_channels[3] = { 0xff, 0xff, 0x3e }; // [0] = key down, [1] = key being sent; [2] = ch62
	u8 k_ch_byte = 0; // 'k_channels' index; 0 = idly scanning the keyboard; 1 = sending a key; 2 = sending ch62
	u8 k_ch_bit = 0; // bit# in the byte being sent; 0 = AGC, 1 = start bit, 2-7 = bits 0-5 of data, 8 = end of byte
	bool k_bit_bit = 0; // the value of the bit of data being processed
	bool k_bit_num = 0; // each bit gets 2 transmissions, this variable shows which half we are working on;
	// if k_bit_bit is high then do interrupt then none; low has none then interrupt
};


TIMER_DEVICE_CALLBACK_MEMBER(exelv_state::exelv_hblank_interrupt)
{
	m_tms3556->interrupt();
}


/*
    I/O CPU protocol (WIP):

    I do not have a dump of the I/O CPU ROMs.  The I/O CPU CRC command should
    enable to dump them, but don't take my word for it.

    * port B bit >01 is asserted on reset and after a byte is sent to the I/O
      CPU.
    * port B bit >02 is asserted after a byte is read from the I/O CPU.  When
      the I/O  CPU sees this line asserted, it asserts port A bit >01.
    * port A bit >01 is asserted after a byte is sent to CPU (condition
      cleared when port B bit >01 is cleared after being asserted) and when
      port B bit >02 is asserted.
    * I/O CPU pulses the main CPU INT1 line when ready to send data; data can
      be read by the main CPU on the mailbox port (P48).  The data is a
      function code optionally followed by several bytes of data.  Function
      codes are:
        >00: unused
        >01: joystick 0 receive
        >02: joystick 1 receive
        >03: speech buffer start
        >04: speech buffer end
        >05: serial
        >06: unused
        >07: introduction screen (logo) (EXL 100 only?) or character
          definitions
            data byte #1: data length - 1 MSB
            data byte #2: data length - 1 LSB
            data bytes #3 through (data length + 3): graphic data
        >08: I/O cpu initialized
        >09: I/O cpu serial interface ready
        >0a: I/O cpu serial interface not ready
        >0b: screen switched off
        >0c: speech buffer start (EXELTEL only?)
        >0d: speech ROM or I/O cpu CRC check (EXELTEL only?)
            data byte #1: expected CRC MSB
            data byte #2: expected CRC LSB
            data byte #3: data length - 1 MSB
            data byte #4: data length - 1 LSB
            data bytes #5 through (data length + 5): data on which effective
                CRC is computed
        >0e: mailbox test, country code read (EXELTEL only?)
        >0f: speech ROM read (data repeat) (EXELTEL only?)
    * The main CPU sends data to the I/O CPU through the mailbox port (P48).
      The data byte is a function code; some function codes ask for extra data
      bytes, which are sent through the mailbox port as well.  Function codes
      are:
        >00: I/O CPU reset
        >01: NOP (EXELTEL only?)
        >02: read joystick 0 current value
        >03: read joystick 1 current value
        >04: test serial interface availability
        >05: transmit a byte to serial interface
        >06: initialization of serial interface
        >07: read contents of speech ROM (EXELTEL only?)
        >08: reset speech synthesizer
        >09: start speech synthesizer
        >0a: synthesizer data
        >0b: standard generator request
        >0c: I/O CPU CRC (EXELTEL only?)
        >0d: send exelvision logo (EXL 100 only), start speech ROM sound (EXELTEL only?)
        >0e: data for speech on ROM (EXELTEL only?)
        >0f: do not decode joystick 0 keys (EXELTEL only?)
        >10: do not decode joystick 1 keys (EXELTEL only?)
        >11: decode joystick 0 keys (EXELTEL only?)
        >12: decode joystick 1 keys (EXELTEL only?)
        >13: mailbox test: echo sent data (EXELTEL only?)
        >14: enter sleep mode (EXELTEL only?)
        >15: read country code in speech ROM (EXELTEL only?)
        >16: position I/O CPU DSR without initialization (EXELTEL only?)
        >17: handle speech ROM sound with address (EXELTEL only?)
        other values: I/O CPU reset?
*/


uint8_t exelv_state::mailbox_wx319_r()
{
	LOG("[TMS7220] reading mailbox %d\n", m_wx319);
	return m_wx319;
}


void exelv_state::mailbox_wx318_w(uint8_t data)
{
	LOG("wx318 write 0x%02x\n", data);
	m_wx318 = data;
}


/*
    TMS7020 PORT A
    A0 - R - TMS7041 port B bit 7 (REV3)
    A1 -
    A2 -
    A3 -
    A4 - R - cass in
    A5 -
    A6 -
    A7 -
*/
uint8_t exelv_state::tms7020_porta_r()
{
	LOG("tms7020_porta_r\n");
	u8 data = ( m_tms7041_portb & 0x80 ) ? 0x01 : 0x00;
	if (m_cass)
	{
		double level = (m_cass->input());
		if (level < 0.02)
			data |= 0x10;
	}
	return data;
}


/*
    TMS7020 PORT B
    B0 - W - TMS7041 port A bit 2 (REV2)
    B1 - W - TMS7041 port A bit 4 (REV4)
    B2 -
    B3 - W - cass out
    B4 -
    B5 -
    B6 -
    B7 -
*/
void exelv_state::tms7020_portb_w(uint8_t data)
{
	LOG("tms7020_portb_w: data = 0x%02x\n", data);
	m_tms7020_portb = data;
	if (m_cass)
	{
		m_cass->output(BIT(data, 3) ? -1.0 : +1.0);
		m_speaker->level_w(BIT(data, 3) ? -1.0 : +1.0);
	}
}


/*
    TMS7041 PORT A
    A0 - X1 NDSR A8
    A1 - X1 MDTR A9
    A2 - R - TMS7020 port B bit 0 (REV2)
    A3 - TMS5220 IRQ
    A4 - R - TMS7020 port B bit 1 (REV4)
    A5 - X1 RXD A4 (version b) / WX301-14 (version a)
    A6 - X1 SCLK A9
    A7 - TMS5220 RDY
*/
uint8_t exelv_state::tms7041_porta_r()
{
	uint8_t data = 0x00;
	static uint8_t data_last=0;

	// TMS5220 OK
	data |= m_tms5220c->intq_r() ? 0x08 : 0x00; // A3
	data |= m_tms5220c->readyq_r() ? 0x80 : 0x00; // A7

	// TMS7220
	data |= (m_tms7020_portb & 0x01 ) ? 0x04 : 0x00; // A2
	data |= (m_tms7020_portb & 0x02) ? 0x10 : 0x00; // A4

	// SERIAL PORT

	if (data!=data_last) {
		LOG("tms7041_porta_r %x\n",data);
	}
	data_last=data;

	return data;
}


/*
    TMS7041 PORT B
    B0 - W - TMS5220 W
    B1 - W - TMS5220 R
    B2 - W - TMS7020 pin 13 / IRQ1
    B3 - X1 TXD A5 (version b) / WX301-8 (version a)
    B4 - TP3
    B5 - REV5 WX318-1
    B6 - W - REV6 WX319-11
    B7 - W - TMS7020 port A bit 0 (REV3)
*/
void exelv_state::tms7041_portb_w(uint8_t data)
{
	LOG("tms7041_portb_w: data = 0x%02x\n", data);

	// optional code; tms5220 device works in different ways depending on if this exists or not
	m_tms5220c->combined_rsq_wsq_w(data & 3);

	LOG("TMS7020 %s int1\n",((data & 0x04) ? "clear" : "assert"));

	/* Check for high->low transition on B2 */
	// Using hold_line because the pulse is too short and can be missed by the other cpu
	if ((BIT(m_tms7041_portb, 2)) && !BIT(data, 2))
		m_maincpu->set_input_line(TMS7000_INT1_LINE, HOLD_LINE);

	/* Check for low->high transition on B6 */
	if ((!BIT(m_tms7041_portb, 6)) && BIT(data, 6))
	{
		LOG("wx319 write 0x%02x\n", m_tms7041_portc);
		m_wx319 = m_tms7041_portc;
	}

	m_tms7041_portb = data;
}


/*
    TMS7041 PORT C - connected to mailbox WX318 and WX319 data bits
*/
uint8_t exelv_state::tms7041_portc_r()
{
	uint8_t data = 0xff;
	LOG("tms7041_portc_r\n");

	/* Check if wx318 output is enabled */
	if (!(m_tms7041_portb & 0x20))
		data = m_wx318;

	return data;
}


void exelv_state::tms7041_portc_w(uint8_t data)
{
	LOG("tms7041_portc_w: data = 0x%02x\n", data);
	m_tms7041_portc = data;
}


/*
    TMS7041 PORT D
    D0 - TMS5220 D7
    D1 - TMS5220 D6
    D2 - TMS5220 D5
    D3 - TMS5220 D4
    D4 - TMS5220 D3
    D5 - TMS5220 D2
    D6 - TMS5220 D1
    D7 - TMS5220 D0
*/
uint8_t exelv_state::tms7041_portd_r()
{
	uint8_t data = m_tms5220c->status_r();
	LOG("tms7041_portd_r: data = 0x%02x\n", data);
	return data;
}


void exelv_state::tms7041_portd_w(uint8_t data)
{
	LOG("tms7041_portd_w: data = 0x%02x\n", data);

	m_tms5220c->data_w(data);
	m_tms7041_portd = data;
}


/*
    CARTRIDGE ACCESS
*/
uint8_t exelv_state::rom_r(offs_t offset)
{
	if (m_rom_size && m_cart && m_cart->exists())
	{
		if (m_rom_size == 0x7e00)
			return m_cart->read_rom(offset);
		else
			return m_cart->read_rom(offset + 0x200);
	}

	return 0;
}

// INFRARED KEYBOARD
// Note: usec times are from the datasheet and should not be altered, but msec times are just guesswork.
TIMER_DEVICE_CALLBACK_MEMBER(exelv_state::timer_k)
{
	// when a key pressed, a channel 0-61 is sent and repeated every 90ms. When released, send channel 62, then silence.

	if (k_ch_byte < 2)
	{
		k_channels[0] = 0xff;
		for (u8 row = 0; row < 8; row++)
		{
			u8 colin = m_io_keyboard[row]->read();
			if (colin)
			{
				for (u8 j = 0; j < 8; j++)
				{
					if (BIT(colin, j))
					{
						k_channels[0] = row*8+j; // key pressed
						if (k_ch_byte == 0)
						{
							// can accept it for processing
							if (k_channels[1] == 0xff)
								k_channels[1] = k_channels[0];
							// init pointers
							k_ch_bit = 0;
							k_bit_num = 0;
							k_ch_byte = 1;
						}
					}
				}
			}
		}
	}

	// Idling; nothing pressed, nothing to do
	if (k_ch_byte == 0)
	{
		m_timer_k->adjust(attotime::from_msec(25));
		return;
	}

	// AGC bit - a single 540us pulse followed by a large gap
	if (k_ch_bit == 0)
	{
		if (!k_bit_num)
		{
			m_subcpu->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
			k_bit_num = 1;
			m_timer_k->adjust(attotime::from_usec(540));
		}
		else
		{
			m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
			k_bit_num = 0;
			k_ch_bit = 1;
			m_timer_k->adjust(attotime::from_usec(2840));
		}
		return;
	}

	// start bit - a hardcoded '1' - so send a 540us pulse followed by 590us of silence
	if (k_ch_bit == 1)
	{
		if (!k_bit_num)
		{
			m_subcpu->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
			k_bit_num = 1;
			m_timer_k->adjust(attotime::from_usec(540));
		}
		else
		{
			m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
			k_bit_num = 0;
			k_ch_bit = 2;
			m_timer_k->adjust(attotime::from_usec(590));
		}
		return;
	}

	// stop bit - not really a 'bit' - need to turn off any interrupt,
	//  and prepare for either 90msec inter-byte gap, or ch62 terminating byte.
	//  If we are already finishing up the terminating byte, then set to idle.
	if (k_ch_bit == 8)
	{
		m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
		k_ch_bit = 0;
		// just finished sending key
		if (k_ch_byte == 1)
		{
			// key still down, send again in 90ms
			if (k_channels[0] < 0xff)
			{
				m_timer_k->adjust(attotime::from_usec(90000));
			}
			else
			// key was released, send ch62
			{
				k_ch_byte = 2;
				m_timer_k->adjust(attotime::from_msec(3)); // signal end channel
			}
		}
		else
		// just finished sending ch62
		if (k_ch_byte == 2)
		{
			// clean up and go back to looking at kbd
			k_channels[1] = 0xff;
			k_ch_byte = 0;
			m_timer_k->adjust(attotime::from_msec(20));
		}
		return;
	}

	// data bits, LSB first
	// 1st half of a bit
	if (!k_bit_num)
	{
		k_bit_bit = BIT(k_channels[k_ch_byte], k_ch_bit-2);

		if (k_bit_bit)
			m_subcpu->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
		else
			m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);

		k_bit_num = 1;
		m_timer_k->adjust(attotime::from_usec(590));
		return;
	}

	// 2nd half of a bit
	if (!k_bit_bit)
		m_subcpu->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
	else
		m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);

	k_bit_num = 0;
	k_ch_bit++; // next bit
	m_timer_k->adjust(attotime::from_usec(540));
}


/*
    Main CPU memory map summary:

    @>0000-@>007f: tms7020/tms7040 internal RAM
    @>0080-@>00ff: reserved
    @>0100-@>010b: tms7020/tms7040 internal I/O ports
        @>104 (P4): port A
        @>106 (P6): port B
            bit >04: page select bit 0 (LSBit)
    @>010c-@>01ff: external I/O ports?
        @>012d (P45): tms3556 control write port???
        @>012e (P46): tms3556 VRAM write port???
        @>0130 (P48): I/O CPU communication port R/W ("mailbox")
        @>0138 (P56): read sets page select bit 1, write clears it???
        @>0139 (P57): read sets page select bit 2 (MSBit), write clears it???
        @>0140 (P64)
            bit >40: enable page select bit 1 and 2 (MSBits)
    @>0200-@>7fff: system ROM? (two pages?) + cartridge ROMs? (one or two pages?)
    @>8000-@>bfff: free for expansion?
    @>c000-@>c7ff: CPU RAM?
    @>c800-@>efff: free for expansion?
    @>f000-@>f7ff: tms7040 internal ROM
    @>f800-@>ffff: tms7020/tms7040 internal ROM
*/

void exelv_state::tms7020_mem(address_map &map)
{
	map(0x0080, 0x00ff).noprw();
	map(0x0124, 0x0124).r(m_tms3556, FUNC(tms3556_device::vram_r));
	map(0x0125, 0x0125).r(m_tms3556, FUNC(tms3556_device::reg_r));
	map(0x0128, 0x0128).r(m_tms3556, FUNC(tms3556_device::initptr_r));
	map(0x012d, 0x012d).nopr().w(m_tms3556, FUNC(tms3556_device::reg_w));
	map(0x012e, 0x012e).nopr().w(m_tms3556, FUNC(tms3556_device::vram_w));

	map(0x0130, 0x0130).rw(FUNC(exelv_state::mailbox_wx319_r), FUNC(exelv_state::mailbox_wx318_w));
	map(0x0200, 0x7fff).r(FUNC(exelv_state::rom_r));
	map(0x8000, 0xbfff).noprw();
	map(0xc000, 0xc7ff).ram();                                     /* CPU RAM */
	map(0xc800, 0xf7ff).noprw();
}


void exelv_state::tms7040_mem(address_map &map)
{
	map(0x0080, 0x00ff).noprw();
	map(0x0124, 0x0124).r(m_tms3556, FUNC(tms3556_device::vram_r));
	map(0x0125, 0x0125).r(m_tms3556, FUNC(tms3556_device::reg_r));
	map(0x0128, 0x0128).r(m_tms3556, FUNC(tms3556_device::initptr_r));
	map(0x012d, 0x012d).nopr().w(m_tms3556, FUNC(tms3556_device::reg_w));
	map(0x012e, 0x012e).nopr().w(m_tms3556, FUNC(tms3556_device::vram_w));
	map(0x0130, 0x0130).rw(FUNC(exelv_state::mailbox_wx319_r), FUNC(exelv_state::mailbox_wx318_w));
	map(0x0200, 0x7fff).bankr("bank1");                                /* system ROM */
	map(0x8000, 0xbfff).noprw();
	map(0xc000, 0xc7ff).ram();                                     /* CPU RAM */
	map(0xc800, 0xefff).noprw();
}

static INPUT_PORTS_START(exelv)
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_NAME(UTF8_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME(UTF8_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME(UTF8_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME(UTF8_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("_  -") PORT_CHAR('_')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("CTL")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("LOCK")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/  :") PORT_CHAR('/')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("?  ,") PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_NAME("FCT") // function key

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("\xe2\x86\x96") PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("+  =") PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_NAME("1  &") PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("6  $") PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("8  !") PORT_CHAR('8')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME(u8"0  à") PORT_CHAR('0')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("\xe2\x8c\xab") PORT_CHAR(127)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(".  ;") PORT_CHAR('.')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME(u8"2  é") PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("3  \"") PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_NAME(u8"9  ç") PORT_CHAR('9')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("#  )") PORT_CHAR('#')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("SHIFT")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("<  [") PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME(">  ]") PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("~  ^") PORT_CHAR('~')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("4  '") PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME(u8"7  è") PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("*  \\") PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("5  (") PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("%  @") PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* Machine Initialization */

void exelv_state::machine_common()
{
	/* register for state saving */
	save_item(NAME(m_tms7020_portb));
	save_item(NAME(m_tms7041_portb));
	save_item(NAME(m_tms7041_portc));
	save_item(NAME(m_tms7041_portd));
	save_item(NAME(m_wx318));
	save_item(NAME(m_wx319));
	save_item(NAME(k_channels));
	save_item(NAME(k_ch_byte));
	save_item(NAME(k_ch_bit));
	save_item(NAME(k_bit_bit));
	save_item(NAME(k_bit_num));
}

MACHINE_START_MEMBER( exelv_state, exl100)
{
	machine_common();
	save_item(NAME(m_rom_size));

	m_rom_size = 0;
	if (m_cart && m_cart->exists())
		m_rom_size = m_cart->get_rom_size();
}

MACHINE_START_MEMBER( exelv_state, exeltel)
{
	machine_common();

	uint8_t *rom = memregion("user1")->base() + 0x0200;
	membank("bank1")->configure_entry(0, rom);
	membank("bank1")->set_entry(0);
}

void exelv_state::machine_reset()
{
	k_channels[0] = 0xff;
	k_channels[1] = 0xff;
	k_ch_byte = 0;
	k_ch_bit = 0;
	k_bit_bit = 0;
	k_bit_num = 0;

	if (m_timer_k)
		m_timer_k->adjust(attotime::from_seconds(2));

	if (m_subcpu)
	{
		m_subcpu->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
		m_subcpu->set_input_line(TMS7000_INT3_LINE, CLEAR_LINE);
	}
}


void exelv_state::exl100(machine_config &config)
{
	/* basic machine hardware */
	TMS7020_EXL(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &exelv_state::tms7020_mem);
	m_maincpu->in_porta().set(FUNC(exelv_state::tms7020_porta_r));
	m_maincpu->out_portb().set(FUNC(exelv_state::tms7020_portb_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(exelv_state::exelv_hblank_interrupt), "screen", 0, 1);
	MCFG_MACHINE_START_OVERRIDE(exelv_state, exl100)

	TMS7041(config, m_subcpu, 4.9152_MHz_XTAL);
	m_subcpu->in_porta().set(FUNC(exelv_state::tms7041_porta_r));
	m_subcpu->out_portb().set(FUNC(exelv_state::tms7041_portb_w));
	m_subcpu->in_portc().set(FUNC(exelv_state::tms7041_portc_r));
	m_subcpu->out_portc().set(FUNC(exelv_state::tms7041_portc_w));
	m_subcpu->in_portd().set(FUNC(exelv_state::tms7041_portd_r));
	m_subcpu->out_portd().set(FUNC(exelv_state::tms7041_portd_w));

	config.set_perfect_quantum(m_maincpu);

	TMS3556(config, m_tms3556, 18_MHz_XTAL);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_screen_update("tms3556", FUNC(tms3556_device::screen_update));
#if TMS3556_DOUBLE_WIDTH
	screen.set_size(tms3556_device::TOTAL_WIDTH*2, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH*2-1, 0, tms3556_device::TOTAL_HEIGHT*2-1);
#else
	screen.set_size(tms3556_device::TOTAL_WIDTH, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH-1, 0, tms3556_device::TOTAL_HEIGHT-1);
#endif
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	TIMER(config, m_timer_k).configure_generic(FUNC(exelv_state::timer_k));

	//SPEECHROM(config, "vsm", 0);

	/* sound */
	SPEAKER(config, "mono").front_center();
	// The cassette output is connected into the audio circuit
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.30);

	TMS5220C(config, m_tms5220c, 640000);
	// m_tms5220c->set_speechrom_tag("vsm");
	m_tms5220c->add_route(ALL_OUTPUTS, "mono", 1.00);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "exelvision_cart", "bin,rom");

	CASSETTE(config, m_cass, 0);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	SOFTWARE_LIST(config, "cart_list").set_original("exl100");
}

void exelv_state::exeltel(machine_config &config)
{
	/* basic machine hardware */
	TMS7040(config, m_maincpu, 9.8304_MHz_XTAL);
	m_maincpu->set_divide_by_4();
	m_maincpu->set_addrmap(AS_PROGRAM, &exelv_state::tms7040_mem);
	m_maincpu->in_porta().set(FUNC(exelv_state::tms7020_porta_r));
	m_maincpu->out_portb().set(FUNC(exelv_state::tms7020_portb_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(exelv_state::exelv_hblank_interrupt), "screen", 0, 1);
	MCFG_MACHINE_START_OVERRIDE(exelv_state, exeltel)

	tms7042_device &subcpu(TMS7042(config, "tms7042", 9.8304_MHz_XTAL));
	subcpu.set_divide_by_4();
	subcpu.in_porta().set(FUNC(exelv_state::tms7041_porta_r));
	subcpu.out_portb().set(FUNC(exelv_state::tms7041_portb_w));
	subcpu.in_portc().set(FUNC(exelv_state::tms7041_portc_r));
	subcpu.out_portc().set(FUNC(exelv_state::tms7041_portc_w));
	subcpu.in_portd().set(FUNC(exelv_state::tms7041_portd_r));
	subcpu.out_portd().set(FUNC(exelv_state::tms7041_portd_w));

	config.set_perfect_quantum(m_maincpu);

	TMS3556(config, m_tms3556, 18_MHz_XTAL);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_screen_update("tms3556", FUNC(tms3556_device::screen_update));
#if TMS3556_DOUBLE_WIDTH
	screen.set_size(tms3556_device::TOTAL_WIDTH*2, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH*2-1, 0, tms3556_device::TOTAL_HEIGHT*2-1);
#else
	screen.set_size(tms3556_device::TOTAL_WIDTH, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH-1, 0, tms3556_device::TOTAL_HEIGHT-1);
#endif
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	SPEECHROM(config, "vsm", 0);

	/* sound */
	SPEAKER(config, "mono").front_center();
	TMS5220C(config, m_tms5220c, 9.8304_MHz_XTAL / 15); // unknown divider for "VSPCLK" (generated by TAHC06 gate array)
	m_tms5220c->set_speechrom_tag("vsm");
	m_tms5220c->add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*
  ROM loading
*/
ROM_START(exl100)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("exl100in.bin", 0x000, 0x800, CRC(049109a3) SHA1(98a07297dcdacef41c793c197b6496dac1e8e744))      /* TMS7020 ROM, correct */

	ROM_REGION(0x1000, "subcpu", 0)
	ROM_LOAD("exl100_7041.bin", 0x0000, 0x1000, CRC(38f6fc7a) SHA1(b71d545664a974d8ad39bdf600c5b9884c3efab6))           /* TMS7041 internal ROM, correct  */
//  ROM_REGION(0x8000, "vsm", 0)
ROM_END


ROM_START(exeltel)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("exeltel_7040.bin", 0x0000, 0x1000, CRC(2792f02f) SHA1(442a852eb68ef78974733d169084752a131de23d))      /* TMS7040 internal ROM */

	ROM_REGION(0x1000, "tms7042", 0)
	ROM_LOAD("exeltel_7042.bin", 0x0000, 0x1000, BAD_DUMP CRC(a0163507) SHA1(8452849df7eac8a89cf03ee98e2306047c1c4c38))         /* TMS7042 internal ROM, needs redump */

	ROM_REGION(0x10000,"user1",0)
	ROM_SYSTEM_BIOS( 0, "french", "French v1.4" )
	ROMX_LOAD("exeltel14.bin", 0x0000, 0x10000, CRC(52a80dd4) SHA1(2cb4c784fba3aec52770999bb99a9a303269bf89), ROM_BIOS(0))  /* French system ROM v1.4 */
	ROM_SYSTEM_BIOS( 1, "spanish", "Spanish" )
	ROMX_LOAD("amper.bin", 0x0000, 0x10000, CRC(45af256c) SHA1(3bff16542f8ac55b9841084ea38034132459facb), ROM_BIOS(1)) /* Spanish system rom */

	ROM_REGION(0x8000, "vsm", 0)
	ROM_LOAD("cm62312.bin", 0x0000, 0x4000, CRC(93b817de) SHA1(03863087a071b8f22d36a52d18243f1c33e17ff7)) /* system speech ROM */
ROM_END

} // anonymous namespace


//   YEAR   NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY       FULLNAME   FLAGS
COMP(1984,  exl100,  0,      0,      exl100,  exelv, exelv_state, empty_init, "Exelvision", "EXL 100", MACHINE_NOT_WORKING)
COMP(1986,  exeltel, exl100, 0,      exeltel, exelv, exelv_state, empty_init, "Exelvision", "Exeltel", MACHINE_NOT_WORKING)
