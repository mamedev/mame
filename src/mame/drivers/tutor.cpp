// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    Experimental Tomy Tutor driver

    This computer is known as Tomy Tutor in US, and as Grandstand Tutor in UK.
    It was initially released in Japan in 1982 or 1983 under the name of Pyuuta
    (Pi-yu-u-ta, with a Kanji for the "ta").  The Japanese versions are
    different from the English-language versions, as they have different ROMs
    with Japanese messages and support for the katakana syllabus.  There are at
    least 4 versions:
    * original Pyuuta (1982 or 1983) with title screens in Japanese but no
      Basic
    * Pyuuta Jr. (1983?) which is a console with a simplified keyboard
    * Tomy/Grandstand Tutor (circa October 1983?) with title screens in English
      and integrated Basic
    * Pyuuta Mk. 2 (1984?) with a better-looking keyboard and integrated Basic

    The Tomy Tutor features a TMS9995 CPU @10.7MHz (which includes a
    timer/counter and 256 bytes of 16-bit RAM), 48kb of ROM (32kb on early
    models that did not have the BASIC interpreter), a tms9918a/9929a VDP (or
    equivalent?) with 16kb of VRAM, and a sn76489an sound generator.
    There is a tape interface, a 56-key keyboard, an interface for two
    joysticks, a cartridge port and an extension port.  The OS design does not
    seem to be particularly expandable (I don't see any hook for additional
    DSRs), but there were prototypes for a parallel port (emulated)
    and a speech synthesizer unit (not emulated).


    The Tutor appears to be related to Texas Instruments' TI99 series.

    The general architecture is relatively close to the ti99/4(a): arguably,
    the Tutor does not include any GROM (it uses regular CPU ROMs for GPL
    code), and its memory map is quite different due to the fact it was
    designed with a tms9995 in mind (vs. a tms9985 for ti99/4), but, apart from
    that, it has a similar architecture with only 256 bytes of CPU RAM and 16kb
    of VRAM.

    While the OS is not derived directly from the TI99/4(a) OS, there are
    disturbing similarities: the Japanese title screen is virtually identical
    to the TI-99 title screen.  Moreover, the Tutor BASIC seems to be be
    derived from TI Extended BASIC, as both BASIC uses similar tokens and
    syntax, and are partially written in GPL (there is therefore a GPL
    interpreter in Tutor ROMs, although the Tutor GPL is incompatible with TI
    GPL, does not seem to be used by any program other than Tutor Basic, and it
    seems unlikely that the original Pyuuta had this GPL interpreter in ROMs).

    It appears that TI has sold the licence of the TI BASIC to Tomy, probably
    after it terminated its TI99 series.  It is not impossible that the entire
    Tutor concept is derived from a TI project under licence: this machine
    looks like a crossbreed of the TI99/2 and the TI99/4 (or /4a, /4b, /5), and
    it could either have been an early version of the TI99/2 project with a
    tms9918a/99289a VDP instead of the DMA video controller, or a "TI99/3" that
    would have closed the gap between the extremely low-end TI99/2 and the
    relatively mid-range TI99/5.


    Raphael Nabet, 2003


TODO :
    * debug the tape interface (Saved tapes sound OK, both Verify and Load
      recognize the tape as a Tomy tape, but the data seems to be corrupted and
      we end with a read error.)
    * guess which device is located at the >e600 base
    * find info about other Tutor variants


    Interrupts:

    Interrupt levels 1 (external interrupt 1) and 2 (error interrupt) do not
    seem to be used: triggering these seems to cause a soft reset.  XOPs are
    not used at all: the ROM area where these vectors should be defined is used
    by a ROM branch table.


Memory Map found at http://www.floodgap.com/retrobits/tomy/mmap.html

                   *** $0000-$7FFF is the 32K BIOS ***
 it is also possible to replace the BIOS with an external ROM (see $E000 range)

0000
                                reset vector (level 0)
                                0000-0001: WP   0002-0003: PC (%)
0004
                                level 1 and 2 IRQ vectors (%)
                                idem. These just seem to reset the system.
000C
                                additional vectors
0040
                                XOP vectors (%)
                The BIOS doesn't seem to use these for XOPs.
                Instead, this is a branch table.
0080
                BIOS code
                (CRU only: $1EE0-FE: 9995 flag register;
                    $1FDA: MID flag)
4000
                GBASIC
                (on the American v2.3 firmware, the GPL
                    interpreter and VDP RAMLUT co-exist
                    with GBASIC in this range)

              *** $8000-$BFFF is the 16K option ROM area ***

8000
                BASIC (Tutor only) and/or cartridge ROM
                    (controlled by $E100)
                To be recognized, a cartridge must have a
                $55, $66 or $aa header sequence.

                         *** end ROM ***
C000
                unmapped (possible use in 24K cartridges)
E000
                I/O range
                ---------
                9918A VDP data/register ports: $E000, E002
                "MMU" banking controls: $E100-E1FF
                    $E100 write: enable cartridge, disable
                        BIOS at $0000 (???) -- magic
                        required at $E110 for this
                    $E108 write: enable BASIC ROM, disable
                        cartridge at $8000
                    $E10C write: enable cartridge, disable
                        BASIC ROM at $8000
                    $E110 must be $42 to enable $E100
                        and to replace the BIOS with
                        an installed cartridge ROM.
                        BLWP assumed at $0000 (??).
                SN76489AN sound data port: $E200
                Device handshaking: $E600
                    Unknown purpose, disk drive maybe?
                Printer handshaking: $E800
                This is a standard Centronics port.
                    $E810 write: parallel data bus
                    $E820 read: parallel port busy
                    $E840 write: port handshake output
                Keyboard lines: $EC00-$EC7E (*CRU*)
                    (CRU physical address $7600-$763F)
                Cassette lines: $ED00-$EEFF
                    $ED00 (*CRU*): input level
                        (physical address $7680)
                    $EE00 write: tape output zero
                    $EE20 write: tape output one
                    $EE40 write: tape IRQ on
                    $EE60 write: tape IRQ off
                    $EE80, A0, C0, E0: ???

F000
                                TMS9995 RAM (*)
F0FC
                                ??
FFFA
                                decrementer (*)
FFFC
                                NMI vector (*)
FFFF



PYUUTAJR
********

This is a handheld unit with 12 'chiclet' buttons. The keyboard has E800, EA00, EC00, EE00 scanned,
although 2 of these rows have no keys. TUTOR carts will run, however since most of them ask for
A=AMA, P=PRO, these keys don't exist, and so the games cannot be played.

*********************************************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/wave.h"
#include "video/tms9928a.h"
#include "imagedev/cassette.h"
#include "sound/sn76496.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class tutor_state : public driver_device
{
public:
	tutor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_cass(*this, "cassette"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank1_switching(0)
	{
	}

	required_device<tms9995_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	optional_device<cassette_image_device> m_cass;
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	memory_region *m_cart_rom;

	int m_bank1_switching;
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(tutor_mapper_r);
	DECLARE_WRITE8_MEMBER(tutor_mapper_w);
	DECLARE_READ8_MEMBER(tutor_cassette_r);
	DECLARE_WRITE8_MEMBER(tutor_cassette_w);
	DECLARE_READ8_MEMBER(tutor_printer_r);
	DECLARE_WRITE8_MEMBER(tutor_printer_w);

	DECLARE_READ8_MEMBER(tutor_highmem_r);
	int m_tape_interrupt_enable;
	emu_timer *m_tape_interrupt_timer;
	virtual void machine_start();
	virtual void machine_reset();
	TIMER_CALLBACK_MEMBER(tape_interrupt_handler);

	int m_centronics_busy;
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
};


void tutor_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	m_tape_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tutor_state::tape_interrupt_handler),this));

	m_bank1->configure_entry(0, memregion("maincpu")->base() + 0x4000);
	m_bank1->set_entry(0);
	m_bank2->configure_entry(0, memregion("maincpu")->base() + 0x8000);
	m_bank2->set_entry(0);

	if (m_cart_rom)
	{
		if (m_cart_rom->bytes() > 0x4000)
		{
			m_bank1_switching = 1;
			m_bank1->configure_entry(1, m_cart_rom->base());
			m_bank1->set_entry(1);
			m_bank2->configure_entry(1, m_cart_rom->base() + 0x4000);
			m_bank2->set_entry(1);
		}
		else
		{
			m_bank2->configure_entry(1, m_cart_rom->base());
			m_bank2->set_entry(1);
		}
	}
}

void tutor_state::machine_reset()
{
	m_tape_interrupt_enable = 0;
	m_centronics_busy = 0;

	// Enable auto wait states by lowering READY during reset
	m_maincpu->set_ready(CLEAR_LINE);
}

/*
    Keyboard:

    Keyboard ports are located at CRU logical address >ec00->ec7e (CRU physical
    address >7600->763f).  There is one bit per key (bit >00 for keycode >00,
    bit >01 for keycode >01, etc.), each bit is set to one when the key is
    down.

    Joystick:

    Joystick ports seem to overlap keyboard ports, i.e. some CRU bits are
    mapped to both a keyboard key and a joystick switch.
*/

READ8_MEMBER( tutor_state::key_r )
{
	char port[12];
	UINT8 value;

	snprintf(port, ARRAY_LENGTH(port), "LINE%d", offset);
	value = ioport(port)->read();

	/* hack for ports overlapping with joystick */
	if (offset == 4 || offset == 5)
	{
		snprintf(port, ARRAY_LENGTH(port), "LINE%d_alt", offset);
		value |= ioport(port)->read();
	}

	return value;
}


/*
    Cartridge mapping:

    Cartridges share the >8000 address base with BASIC.  A write to @>e10c
    disables the BASIC ROM and enable the cartridge.  A write to @>e108
    disables the cartridge and enables the BASIC ROM.

    In order to be recognized by the system ROM, a cartridge should start with
    >55, >66 or >aa.  This may may correspond to three different ROM header
    versions (I am not sure).

    Cartridge may also define a boot ROM at base >0000 (see below).
*/

READ8_MEMBER( tutor_state::tutor_mapper_r )
{
	int reply;

	switch (offset)
	{
	case 0x10:
		/* return 0x42 if we have a cartridge with an alternate boot ROM */
		reply = 0;
		break;

	default:
		logerror("unknown port in %s %d\n", __FILE__, __LINE__);
		reply = 0;
		break;
	}

	return reply;
}

WRITE8_MEMBER( tutor_state::tutor_mapper_w )
{
	switch (offset)
	{
	case 0x00:
		/* disable system ROM, enable alternate boot ROM in cartridge */
		break;

	case 0x08:
		/* disable cartridge ROM, enable BASIC ROM at base >8000 */
		m_bank1->set_entry(0);
		m_bank2->set_entry(0);
		break;

	case 0x0c:
		/* enable cartridge ROM, disable BASIC ROM at base >8000 */
		if (m_cart_rom)
		{
			if (m_bank1_switching)
				m_bank1->set_entry(1);
			m_bank2->set_entry(1);
		}
		break;

	default:
		if (!(offset & 1))
			logerror("unknown port in %s %d\n", __FILE__, __LINE__);
		break;
	}
}

/*
    This is only called from the debugger; the on-chip memory is handled
    within the CPU itself.
*/
READ8_MEMBER( tutor_state::tutor_highmem_r )
{
	if (m_maincpu->is_onchip(offset | 0xf000)) return m_maincpu->debug_read_onchip_memory(offset&0xff);
	return 0;
}

/*
    Cassette interface:

    The cassette interface uses several ports in the >e000 range.

    Writing to *CPU* address @>ee00 will set the tape output to 0.  Writing to
    *CPU* address @>ee20 will set the tape output to 1.

    Tape input level can be read from *CRU* logical address >ed00 (CRU physical
    address >7680).

    Writing to @>ee40 enables tape interrupts; writing to @>ee60 disables tape
    interrupts.  Tape interrupts are level-4 interrupt that occur when the tape
    input level is high(?).

    There are other output ports: @>ee80, @>eea0, @>eec0 & @>eee0.  I don't
    know their exact meaning.
*/

TIMER_CALLBACK_MEMBER(tutor_state::tape_interrupt_handler)
{
	//assert(m_tape_interrupt_enable);
	m_maincpu->set_input_line(INT_9995_INT4, (m_cass->input() > 0.0) ? ASSERT_LINE : CLEAR_LINE);
}

/* CRU handler */
READ8_MEMBER( tutor_state::tutor_cassette_r )
{
	return (m_cass->input() > 0.0) ? 1 : 0;
}

/* memory handler */
WRITE8_MEMBER( tutor_state::tutor_cassette_w )
{
	if (offset & /*0x1f*/0x1e)
		logerror("unknown port in %s %d\n", __FILE__, __LINE__);

	if ((offset & 0x1f) == 0)
	{
		data = BIT(offset, 5);

		switch ((offset >> 6) & 3)
		{
		case 0:
			/* data out */
			m_cass->output((data) ? +1.0 : -1.0);
			break;
		case 1:
			/* interrupt control??? */
			//logerror("ignoring write of %d to cassette port 1\n", data);
			if (m_tape_interrupt_enable != data)
			{
				m_tape_interrupt_enable = data;
				if (!m_tape_interrupt_enable)
					m_tape_interrupt_timer->adjust(/*attotime::from_hz(44100)*/attotime::zero, 0, attotime::from_hz(44100));
				else
				{
					m_tape_interrupt_timer->adjust(attotime::never);
					m_maincpu->set_input_line(INT_9995_INT4, CLEAR_LINE);
				}
			}
			break;
		case 2:
			/* ??? */
			logerror("ignoring write of %d to cassette port 2\n", data);
			break;
		case 3:
			/* ??? */
			logerror("ignoring write of %d to cassette port 3\n", data);
			break;
		}
	}
}

WRITE_LINE_MEMBER( tutor_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

/* memory handlers */
READ8_MEMBER( tutor_state::tutor_printer_r )
{
	int reply;

	switch (offset)
	{
	case 0x20:
		/* busy */
		reply = m_centronics_busy ? 0x00 : 0xff;
		break;

	default:
		if (! (offset & 1))
			logerror("unknown port in %s %d\n", __FILE__, __LINE__);
		reply = 0;
		break;
	}

	return reply;
}

WRITE8_MEMBER( tutor_state::tutor_printer_w )
{
	switch (offset)
	{
	case 0x10:
		/* data */
		m_cent_data_out->write(space, 0, data);
		break;

	case 0x40:
		/* strobe */
		m_centronics->write_strobe(BIT(data, 7));
		break;

	default:
		if (! (offset & 1))
			logerror("unknown port in %s %d\n", __FILE__, __LINE__);
		break;
	}
}

/*
    Memory map summary:

    @>0000-@>7fff: system ROM (can be paged out, see above).
    @>8000-@>bfff: basic ROM (can be paged out, see above).
    @>c000-@>dfff: free for future expansion? Used by 24kb cartridges?

    @>e000(r/w): VDP data
    @>e002(r/w): VDP register

    @>e100(w): enable cart and disable system ROM at base >0000??? (the system will only link to such a ROM if @>e110 is >42???)
    @>e108(w): disable cart and enable BASIC ROM at base >8000?
    @>e10c(w): enable cart and disable BASIC ROM at base >8000?
    @>e110(r): cartridges should return >42 if they have a ROM at base >0000 and they want the Tutor to boot from this ROM (with a blwp@>0000)???

    @>e200(w): sound write

    @>e600(r): handshake in from whatever device???
    @>e610(w): ???
    @>e620(w): ???
    @>e680(w): handshake out to this device???

    @>e810(w): parallel port data bus
    @>e820(r): parallel port busy input
    @>e840(w): parallel port strobe output

    @>ee00-@>eee0(w): tape interface (see above)

    @>f000-@>f0fb: tms9995 internal RAM 1
    @>fffa-@>fffb: tms9995 internal decrementer
    @>f000-@>f0fb: tms9995 internal RAM 2
*/

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER( tutor_state::test_w )
{
	switch (offset)
	{
	default:
		logerror("unmapped write %d %d\n", offset, data);
		break;
	}
}
#endif

static ADDRESS_MAP_START(tutor_memmap, AS_PROGRAM, 8, tutor_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1") AM_WRITENOP
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2") AM_WRITENOP
	AM_RANGE(0xc000, 0xdfff) AM_NOP /*free for expansion, or cartridge ROM?*/

	AM_RANGE(0xe000, 0xe000) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)    /*VDP data*/
	AM_RANGE(0xe002, 0xe002) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)/*VDP status*/
	AM_RANGE(0xe100, 0xe1ff) AM_READWRITE(tutor_mapper_r, tutor_mapper_w)   /*cartridge mapper*/
	AM_RANGE(0xe200, 0xe200) AM_DEVWRITE("sn76489a", sn76489a_device, write)    /*sound chip*/
	AM_RANGE(0xe800, 0xe8ff) AM_READWRITE(tutor_printer_r, tutor_printer_w) /*printer*/
	AM_RANGE(0xee00, 0xeeff) AM_READNOP AM_WRITE( tutor_cassette_w)     /*cassette interface*/

	AM_RANGE(0xf000, 0xffff) AM_READ(tutor_highmem_r) AM_WRITENOP /*free for expansion (and internal processor RAM)*/
ADDRESS_MAP_END

static ADDRESS_MAP_START(pyuutajr_mem, AS_PROGRAM, 8, tutor_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1") AM_WRITENOP
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2") AM_WRITENOP
	AM_RANGE(0xc000, 0xdfff) AM_NOP /*free for expansion, or cartridge ROM?*/

	AM_RANGE(0xe000, 0xe000) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)    /*VDP data*/
	AM_RANGE(0xe002, 0xe002) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)/*VDP status*/
	AM_RANGE(0xe100, 0xe1ff) AM_READWRITE(tutor_mapper_r, tutor_mapper_w)   /*cartridge mapper*/
	AM_RANGE(0xe200, 0xe200) AM_DEVWRITE("sn76489a", sn76489a_device, write)    /*sound chip*/
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("LINE0")
	AM_RANGE(0xea00, 0xea00) AM_READ_PORT("LINE1")
	AM_RANGE(0xec00, 0xec00) AM_READ_PORT("LINE2")
	AM_RANGE(0xee00, 0xee00) AM_READ_PORT("LINE3")

	AM_RANGE(0xf000, 0xffff) AM_READ(tutor_highmem_r) AM_WRITENOP /*free for expansion (and internal processor RAM)*/
ADDRESS_MAP_END

/*
    CRU map summary:

    >1ee0->1efe: tms9995 flag register
    >1fda: tms9995 MID flag

    >ec00->ec7e(r): keyboard interface
    >ed00(r): tape input
*/

static ADDRESS_MAP_START(tutor_io, AS_IO, 8, tutor_state)
	AM_RANGE(0xec0, 0xec7) AM_READ(key_r)               /*keyboard interface*/
	AM_RANGE(0xed0, 0xed0) AM_READ(tutor_cassette_r)        /*cassette interface*/
ADDRESS_MAP_END

/* tutor keyboard: 56 keys

2008-05 FP:
Small note about natural keyboard support: currently,
- "MON" is mapped to 'F1'
- "MOD" is mapped to 'F2'
- A, S, D, F, G, R seem to have problems in natural mode
*/

static INPUT_PORTS_START(tutor)
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)               PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)               PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)               PORT_CHAR('Q')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)               PORT_CHAR('W')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)               PORT_CHAR('A')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)               PORT_CHAR('S')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)               PORT_CHAR('Z')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)               PORT_CHAR('X')

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)               PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)               PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)               PORT_CHAR('E')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)               PORT_CHAR('R')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)               PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)               PORT_CHAR('F')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)               PORT_CHAR('C')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)               PORT_CHAR('V')

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)               PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)               PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)               PORT_CHAR('T')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)               PORT_CHAR('Y')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)               PORT_CHAR('G')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)               PORT_CHAR('H')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)               PORT_CHAR('B')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)               PORT_CHAR('N')

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)               PORT_CHAR('7') PORT_CHAR('\'')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)               PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)               PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)               PORT_CHAR('U')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)               PORT_CHAR('I')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)               PORT_CHAR('J')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)               PORT_CHAR('K')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)               PORT_CHAR('M')

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)               PORT_CHAR('0') PORT_CHAR('=')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-  b") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)               PORT_CHAR('O')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)               PORT_CHAR('P')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)               PORT_CHAR('L')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)           PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)           PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)            PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("LINE4_alt")
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_CODE(KEYCODE_6_PAD)

	PORT_START("LINE5")
		/* Unused? */
		PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)

		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("o  ^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)       PORT_CHAR('_') PORT_CHAR('@')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)           PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)       PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)           PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)      PORT_CHAR(']') PORT_CHAR('}') // this one is 4th line, 4th key after 'M'

	PORT_START("LINE5_alt")
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)

	PORT_START("LINE6")
		/* Unused? */
		PORT_BIT(0x21, IP_ACTIVE_HIGH, IPT_UNUSED)

		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		/* only one shift key located on the left, but we support both for emulation to be friendlier */
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mon") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RT") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mod") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)           PORT_CHAR(' ')

	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

		/* Unused? */
		PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

// Unit only has 12 buttons. LINE0 & 3 are scanned with the others, but have nothing connected?
static INPUT_PORTS_START(pyuutajr)
	PORT_START("LINE0")
		PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PALLET") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MONITOR") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)

	PORT_START("LINE3")
		PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static MACHINE_CONFIG_START( tutor, tutor_state )
	// basic machine hardware
	// TMS9995 CPU @ 10.7 MHz
	// No lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, XTAL_10_738635MHz, tutor_memmap, tutor_io)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn76489a", SN76489A, 3579545)   /* 3.579545 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(tutor_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_CASSETTE_ADD( "cassette" )

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "tutor_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","tutor")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pyuutajr, tutor )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pyuutajr_mem)
	//MCFG_DEVICE_REMOVE("centronics")
	//MCFG_DEVICE_REMOVE("cassette")
MACHINE_CONFIG_END

/*
  ROM loading
*/

ROM_START(tutor)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tutor1.bin", 0x0000, 0x8000, CRC(702c38ba) SHA1(ce60607c3038895e31915d41bb5cf71cb8522d7a))      /* system ROM */
	ROM_LOAD("tutor2.bin", 0x8000, 0x4000, CRC(05f228f5) SHA1(46a14a45f6f9e2c30663a2b87ce60c42768a78d0))      /* BASIC ROM */
ROM_END


ROM_START(pyuuta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomy29.7", 0x0000, 0x8000, CRC(7553bb6a) SHA1(fa41c45cb6d3daf7435f2a82f77dfa286003255e))      /* system ROM */
ROM_END

ROM_START(pyuutajr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(2ca37e62) SHA1(eebdc5c37d3b532edd5e5ca65eb785269ebd1ac0))      /* system ROM */
ROM_END

/*   YEAR    NAME      PARENT      COMPAT  MACHINE     INPUT                      INIT    COMPANY     FULLNAME */
COMP(1983?,  tutor,    0,          0,      tutor,      tutor,    driver_device,   0,      "Tomy",   "Tomy Tutor" , 0)
COMP(1982,   pyuuta,   tutor,      0,      tutor,      tutor,    driver_device,   0,      "Tomy",   "Tomy Pyuuta" , 0)
COMP(1983,   pyuutajr, tutor,      0,      pyuutajr,   pyuutajr, driver_device,   0,      "Tomy",   "Tomy Pyuuta Jr." , 0)
