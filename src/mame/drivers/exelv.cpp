// license:GPL-2.0+
// copyright-holders:Raphael Nabet
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

STATUS:
    * EXL 100 cannot be emulated because the ROMs are not dumped
    * EXELTEL stops early in the boot process and displays a red error screen,
      presumably because the I/O processor is not emulated

TODO:
    * dump exeltel tms7042 I/O CPU ROM
    * everything
*/


#include "emu.h"
#include "cpu/tms7000/tms7000.h"
#include "video/tms3556.h"
#include "sound/tms5220.h"
#include "machine/spchrom.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
//#include "imagedev/cassette.h"
#include "softlist.h"

class exelv_state : public driver_device
{
public:
	exelv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_tms3556(*this, "tms3556"),
			m_tms5220c(*this, "tms5220c"),
			m_cart(*this, "cartslot")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<tms3556_device> m_tms3556;
	required_device<tms5220c_device> m_tms5220c;
	optional_device<generic_slot_device> m_cart;

	DECLARE_READ8_MEMBER( mailbox_wx319_r );
	DECLARE_WRITE8_MEMBER( mailbox_wx318_w );
	DECLARE_READ8_MEMBER( tms7020_porta_r );
	DECLARE_WRITE8_MEMBER( tms7020_portb_w );
	DECLARE_READ8_MEMBER( tms7041_porta_r );
	DECLARE_WRITE8_MEMBER( tms7041_portb_w );
	DECLARE_READ8_MEMBER( tms7041_portc_r );
	DECLARE_WRITE8_MEMBER( tms7041_portc_w );
	DECLARE_READ8_MEMBER( tms7041_portd_r );
	DECLARE_WRITE8_MEMBER( tms7041_portd_w );
	DECLARE_READ8_MEMBER( rom_r );

	DECLARE_MACHINE_START(exl100);
	DECLARE_MACHINE_START(exeltel);

	/* tms7020 i/o ports */
	UINT8   m_tms7020_portb;

	/* tms7041 i/o ports */
	UINT8   m_tms7041_portb;
	UINT8   m_tms7041_portc;
	UINT8   m_tms7041_portd;

	/* mailbox data */
	UINT8   m_wx318;    /* data of 74ls374 labeled wx318 */
	UINT8   m_wx319;    /* data of 74sl374 labeled wx319 */

	TIMER_DEVICE_CALLBACK_MEMBER(exelv_hblank_interrupt);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( exelvision_cartridge );
};


TIMER_DEVICE_CALLBACK_MEMBER(exelv_state::exelv_hblank_interrupt)
{
	m_tms3556->interrupt(machine());
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


READ8_MEMBER(exelv_state::mailbox_wx319_r)
{
	logerror("[TMS7220] reading mailbox %d\n", m_wx319);
	return m_wx319;
}


WRITE8_MEMBER(exelv_state::mailbox_wx318_w)
{
	logerror("wx318 write 0x%02x\n", data);
	m_wx318 = data;
}


/*
    TMS7020 PORT A
    A0 - R - TMS7041 port B bit 7 (REV3)
    A1 -
    A2 -
    A3 -
    A4 -
    A5 -
    A6 -
    A7 -
*/
READ8_MEMBER(exelv_state::tms7020_porta_r)
{
	logerror("tms7020_porta_r\n");
	return ( m_tms7041_portb & 0x80 ) ? 0x01 : 0x00;
}


/*
    TMS7020 PORT B
    B0 - W - TMS7041 port A bit 2 (REV2)
    B1 - W - TMS7041 port A bit 4 (REV4)
    B2 -
    B3 -
    B4 -
    B5 -
    B6 -
    B7 -
*/
WRITE8_MEMBER(exelv_state::tms7020_portb_w)
{
	logerror("tms7020_portb_w: data = 0x%02x\n", data);
	m_tms7020_portb = data;
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
READ8_MEMBER(exelv_state::tms7041_porta_r)
{
	UINT8 data = 0x00;
	static UINT8 data_last=0;

	// TMS5220 OK
	data |= m_tms5220c->intq_r() ? 0x08 : 0x00; // A3
	data |= m_tms5220c->readyq_r() ? 0x80 : 0x00; // A7

	// TMS7220
	data |= (m_tms7020_portb & 0x01 ) ? 0x04 : 0x00; // A2
	data |= (m_tms7020_portb & 0x02) ? 0x10 : 0x00; // A4

	// SERIAL PORT

	if (data!=data_last) {
		logerror("tms7041_porta_r %x\n",data);
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
WRITE8_MEMBER(exelv_state::tms7041_portb_w)
{
	logerror("tms7041_portb_w: data = 0x%02x\n", data);

	m_tms5220c->wsq_w((data & 0x01) ? 1 : 0);
	m_tms5220c->rsq_w((data & 0x02) ? 1 : 0);

	logerror("TMS7020 %s int1\n",((data & 0x04) ? "clear" : "assert"));
	m_maincpu->set_input_line(TMS7000_INT1_LINE, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);

	/* Check for low->high transition on B6 */
	if (!(m_tms7041_portb & 0x40) && (data & 0x40))
	{
		logerror("wx319 write 0x%02x\n", m_tms7041_portc);
		m_wx319 = m_tms7041_portc;
	}

	m_tms7041_portb = data;
}


/*
    TMS7041 PORT C - connected to mailbox WX318 and WX319 data bits
*/
READ8_MEMBER(exelv_state::tms7041_portc_r)
{
	UINT8 data = 0xff;
	logerror("tms7041_portc_r\n");

	/* Check if wx318 output is enabled */
	if (!(m_tms7041_portb & 0x20))
		data = m_wx318;

	return data;
}


WRITE8_MEMBER(exelv_state::tms7041_portc_w)
{
	logerror("tms7041_portc_w: data = 0x%02x\n", data);
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
READ8_MEMBER(exelv_state::tms7041_portd_r)
{
	UINT8 data = 0xff;
	data=m_tms5220c->status_r(space, 0, data);
	logerror("tms7041_portd_r\n");
	return data;
}


WRITE8_MEMBER(exelv_state::tms7041_portd_w)
{
	logerror("tms7041_portd_w: data = 0x%02x\n", data);

	m_tms5220c->data_w(space, 0, data);
	m_tms7041_portd = data;
}


/*
    CARTRIDGE ACCESS
*/
READ8_MEMBER(exelv_state::rom_r)
{
	if (m_cart && m_cart->exists())
		return m_cart->read_rom(space, offset + 0x200);

	return 0;
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

static ADDRESS_MAP_START(tms7020_mem, AS_PROGRAM, 8, exelv_state)
	AM_RANGE(0x0080, 0x00ff) AM_NOP
	AM_RANGE(0x0124, 0x0124) AM_DEVREAD("tms3556", tms3556_device, vram_r)
	AM_RANGE(0x0125, 0x0125) AM_DEVREAD("tms3556", tms3556_device, reg_r)
	AM_RANGE(0x0128, 0x0128) AM_DEVREAD("tms3556", tms3556_device, initptr_r)
	AM_RANGE(0x012d, 0x012d) AM_DEVWRITE("tms3556", tms3556_device, reg_w)
	AM_RANGE(0x012e, 0x012e) AM_DEVWRITE("tms3556", tms3556_device, vram_w)

	AM_RANGE(0x0130, 0x0130) AM_READWRITE(mailbox_wx319_r, mailbox_wx318_w)
	AM_RANGE(0x0200, 0x7fff) AM_READ(rom_r)
	AM_RANGE(0x8000, 0xbfff) AM_NOP
	AM_RANGE(0xc000, 0xc7ff) AM_RAM                                     /* CPU RAM */
	AM_RANGE(0xc800, 0xf7ff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7020_port, AS_IO, 8, exelv_state)
	AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(tms7020_porta_r)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(tms7020_portb_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7041_port, AS_IO, 8, exelv_state)
	AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(tms7041_porta_r)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(tms7041_portb_w)
	AM_RANGE(TMS7000_PORTC, TMS7000_PORTC) AM_READWRITE(tms7041_portc_r, tms7041_portc_w)
	AM_RANGE(TMS7000_PORTD, TMS7000_PORTD) AM_READWRITE(tms7041_portd_r, tms7041_portd_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START(tms7040_mem, AS_PROGRAM, 8, exelv_state)
	AM_RANGE(0x0080, 0x00ff) AM_NOP
	AM_RANGE(0x0124, 0x0124) AM_DEVREAD("tms3556", tms3556_device, vram_r)
	AM_RANGE(0x0125, 0x0125) AM_DEVREAD("tms3556", tms3556_device, reg_r)
	AM_RANGE(0x0128, 0x0128) AM_DEVREAD("tms3556", tms3556_device, initptr_r)
	AM_RANGE(0x012d, 0x012d) AM_DEVWRITE("tms3556", tms3556_device, reg_w)
	AM_RANGE(0x012e, 0x012e) AM_DEVWRITE("tms3556", tms3556_device, vram_w)
	AM_RANGE(0x0130, 0x0130) AM_READWRITE(mailbox_wx319_r, mailbox_wx318_w)
	AM_RANGE(0x0200, 0x7fff) AM_ROMBANK("bank1")                                /* system ROM */
	AM_RANGE(0x8000, 0xbfff) AM_NOP
	AM_RANGE(0xc000, 0xc7ff) AM_RAM                                     /* CPU RAM */
	AM_RANGE(0xc800, 0xefff) AM_NOP
ADDRESS_MAP_END


/* keyboard: ??? */
static INPUT_PORTS_START(exelv)

	PORT_START("exelv")

INPUT_PORTS_END


/* Machine Initialization */

MACHINE_START_MEMBER( exelv_state, exl100)
{
	/* register for state saving */
	save_item(NAME(m_tms7020_portb));
	save_item(NAME(m_tms7041_portb));
	save_item(NAME(m_tms7041_portc));
	save_item(NAME(m_tms7041_portd));
	save_item(NAME(m_wx318));
	save_item(NAME(m_wx319));
}

MACHINE_START_MEMBER( exelv_state, exeltel)
{
	UINT8 *rom = memregion("user1")->base() + 0x0200;
	membank("bank1")->configure_entry(0, rom);
	membank("bank1")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_tms7020_portb));
	save_item(NAME(m_tms7041_portb));
	save_item(NAME(m_tms7041_portc));
	save_item(NAME(m_tms7041_portd));
	save_item(NAME(m_wx318));
	save_item(NAME(m_wx319));
}


static MACHINE_CONFIG_START( exl100, exelv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS7020_EXL, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(tms7020_mem)
	MCFG_CPU_IO_MAP(tms7020_port)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", exelv_state, exelv_hblank_interrupt, "screen", 0, 1)
	MCFG_MACHINE_START_OVERRIDE(exelv_state, exl100)

	MCFG_CPU_ADD("tms7041", TMS7041, XTAL_4_9152MHz)
	MCFG_CPU_IO_MAP(tms7041_port)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TMS3556_ADD("tms3556")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_UPDATE_DEVICE("tms3556", tms3556_device, screen_update)
#if TMS3556_DOUBLE_WIDTH
	MCFG_SCREEN_SIZE(TMS3556_TOTAL_WIDTH*2, TMS3556_TOTAL_HEIGHT*2)
	MCFG_SCREEN_VISIBLE_AREA(0, TMS3556_TOTAL_WIDTH*2-1, 0, TMS3556_TOTAL_HEIGHT*2-1)
#else
	MCFG_SCREEN_SIZE(TMS3556_TOTAL_WIDTH, TMS3556_TOTAL_HEIGHT*2)
	MCFG_SCREEN_VISIBLE_AREA(0, TMS3556_TOTAL_WIDTH-1, 0, TMS3556_TOTAL_HEIGHT-1)
#endif
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	// MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)

	/* sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5220c", TMS5220C, 640000)
	// MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "exelvision_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	MCFG_SOFTWARE_LIST_ADD("cart_list", "exl100")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( exeltel, exelv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS7040, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(tms7040_mem)
	MCFG_CPU_IO_MAP(tms7020_port)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", exelv_state, exelv_hblank_interrupt, "screen", 0, 1)
	MCFG_MACHINE_START_OVERRIDE(exelv_state, exeltel)

	MCFG_CPU_ADD("tms7042", TMS7042, XTAL_4_9152MHz)
	MCFG_CPU_IO_MAP(tms7041_port)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TMS3556_ADD("tms3556")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_UPDATE_DEVICE("tms3556", tms3556_device, screen_update)
#if TMS3556_DOUBLE_WIDTH
	MCFG_SCREEN_SIZE(TMS3556_TOTAL_WIDTH*2, TMS3556_TOTAL_HEIGHT*2)
	MCFG_SCREEN_VISIBLE_AREA(0, TMS3556_TOTAL_WIDTH*2-1, 0, TMS3556_TOTAL_HEIGHT*2-1)
#else
	MCFG_SCREEN_SIZE(TMS3556_TOTAL_WIDTH, TMS3556_TOTAL_HEIGHT*2)
	MCFG_SCREEN_VISIBLE_AREA(0, TMS3556_TOTAL_WIDTH-1, 0, TMS3556_TOTAL_HEIGHT-1)
#endif
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)

	/* sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5220c", TMS5220C, 640000)
	MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*
  ROM loading
*/
ROM_START(exl100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("exl100in.bin", 0xf800, 0x0800, CRC(049109a3) SHA1(98a07297dcdacef41c793c197b6496dac1e8e744))      /* TMS7020 ROM, correct */

	ROM_REGION(0x10000, "tms7041", 0)
	ROM_LOAD("exl100_7041.bin", 0xf000, 0x1000, CRC(38f6fc7a) SHA1(b71d545664a974d8ad39bdf600c5b9884c3efab6))           /* TMS7041 internal ROM, correct  */
//  ROM_REGION(0x8000, "vsm", 0)
ROM_END


ROM_START(exeltel)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("exeltel_7040.bin", 0xf000, 0x1000, CRC(2792f02f) SHA1(442a852eb68ef78974733d169084752a131de23d))      /* TMS7040 internal ROM */

	ROM_REGION(0x10000, "tms7042", 0)
	ROM_LOAD("exeltel_7042.bin", 0xf000, 0x1000, BAD_DUMP CRC(a0163507) SHA1(8452849df7eac8a89cf03ee98e2306047c1c4c38))         /* TMS7042 internal ROM, needs redump */

	ROM_REGION(0x10000,"user1",0)
	ROM_SYSTEM_BIOS( 0, "french", "French v1.4" )
	ROMX_LOAD("exeltel14.bin", 0x0000, 0x10000, CRC(52a80dd4) SHA1(2cb4c784fba3aec52770999bb99a9a303269bf89), ROM_BIOS(1))  /* French system ROM v1.4 */
	ROM_SYSTEM_BIOS( 1, "spanish", "Spanish" )
	ROMX_LOAD("amper.bin", 0x0000, 0x10000, CRC(45af256c) SHA1(3bff16542f8ac55b9841084ea38034132459facb), ROM_BIOS(2)) /* Spanish system rom */

	ROM_REGION(0x8000, "vsm", 0)
	ROM_LOAD("cm62312.bin", 0x0000, 0x4000, CRC(93b817de) SHA1(03863087a071b8f22d36a52d18243f1c33e17ff7)) /* system speech ROM */
ROM_END


/*   YEAR   NAME     PARENT      COMPAT  MACHINE     INPUT   INIT    COMPANY         FULLNAME */
COMP(1984,  exl100,  0,          0,      exl100,     exelv, driver_device,  0,       "Exelvision",   "EXL 100",  MACHINE_NOT_WORKING)
COMP(1986,  exeltel, exl100,     0,      exeltel,    exelv, driver_device,  0,       "Exelvision",   "Exeltel",  MACHINE_NOT_WORKING)
