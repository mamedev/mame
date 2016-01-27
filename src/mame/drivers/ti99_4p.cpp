// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG SGCPU (a.k.a. 99/4p) system

    This system is a reimplementation of the old ti99/4a console.  It is known
    both as the 99/4p ("peripheral box", since the system is a card to be
    inserted in the peripheral box, instead of a self contained console), and
    as the SGCPU ("Second Generation CPU", which was originally the name used
    in TI documentation to refer to either (or both) TI99/5 and TI99/8
    projects).

    The SGCPU was designed and built by the SNUG (System 99 Users Group),
    namely by Michael Becker for the hardware part and Harald Glaab for the
    software part.  It has no relationship with TI.

    The card is architectured around a 16-bit bus (vs. an 8-bit bus in every
    other TI99 system).  It includes 64kb of ROM, including a GPL interpreter,
    an internal DSR ROM which contains system-specific code, part of the TI
    extended Basic interpreter, and up to 1Mbyte of RAM.  It still includes a
    16-bit to 8-bit multiplexer in order to support extension cards designed
    for TI99/4a, but it can support 16-bit cards, too.  It does not include
    GROMs, video or sound: instead, it relies on the HSGPL and EVPC cards to
    do the job.

    IMPORTANT: The SGCPU card relies on a properly set up HSGPL flash memory
    card; without, it will immediately lock up. It is impossible to set it up
    from here (a bootstrap problem; you cannot start without the HSGPL).
    The best chance is to start a ti99_4ev with a plugged-in HSGPL
    and go through the setup process there. Copy the nvram files of the hsgpl into this
    driver's nvram subdirectory. The contents will be directly usable for the SGCPU.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900.h"
#include "sound/wave.h"
#include "sound/dac.h"

#include "machine/tms9901.h"
#include "imagedev/cassette.h"

#include "bus/ti99x/videowrp.h"
#include "bus/ti99x/joyport.h"

#include "bus/ti99_peb/peribox.h"

#define TMS9901_TAG "tms9901"
#define SGCPU_TAG "sgcpu"

#define VERBOSE 1
#define LOG logerror

class ti99_4p_state : public driver_device
{
public:
	ti99_4p_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_tms9901(*this, TMS9901_TAG),
		m_sound(*this, TISOUND_TAG),
		m_video(*this, VIDEO_SYSTEM_TAG),
		m_cassette(*this, "cassette"),
		m_peribox(*this, PERIBOX_TAG),
		m_joyport(*this, JOYPORT_TAG)   { }

	DECLARE_WRITE_LINE_MEMBER( console_ready );
	DECLARE_WRITE_LINE_MEMBER( console_ready_dmux );

	DECLARE_WRITE_LINE_MEMBER( extint );
	DECLARE_WRITE_LINE_MEMBER( notconnected );
	DECLARE_READ8_MEMBER( interrupt_level );
	DECLARE_READ16_MEMBER( memread );
	DECLARE_WRITE16_MEMBER( memwrite );

	DECLARE_READ16_MEMBER( samsmem_read );
	DECLARE_WRITE16_MEMBER( samsmem_write );

	DECLARE_WRITE8_MEMBER(external_operation);
	DECLARE_WRITE_LINE_MEMBER( clock_out );

	void    clock_in(int clock);

	// CRU (Communication Register Unit) handling
	DECLARE_READ8_MEMBER( cruread );
	DECLARE_WRITE8_MEMBER( cruwrite );
	DECLARE_READ8_MEMBER( read_by_9901 );
	DECLARE_WRITE_LINE_MEMBER(keyC0);
	DECLARE_WRITE_LINE_MEMBER(keyC1);
	DECLARE_WRITE_LINE_MEMBER(keyC2);
	DECLARE_WRITE_LINE_MEMBER(cs_motor);
	DECLARE_WRITE_LINE_MEMBER(audio_gate);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);
	DECLARE_WRITE8_MEMBER(tms9901_interrupt);
	DECLARE_WRITE_LINE_MEMBER(alphaW);
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(ti99_4p);

	DECLARE_WRITE_LINE_MEMBER(set_tms9901_INT2_from_v9938);

	required_device<tms9900_device>             m_cpu;
	required_device<tms9901_device>             m_tms9901;
	required_device<ti_sound_system_device> m_sound;
	required_device<ti_exp_video_device>        m_video;
	required_device<cassette_image_device>  m_cassette;
	required_device<peribox_device>             m_peribox;
	required_device<joyport_device>             m_joyport;

	// Pointer to ROM0
	UINT16  *m_rom0;

	// Pointer to DSR ROM
	UINT16  *m_dsr;

	// Pointer to ROM6, first bank
	UINT16  *m_rom6a;

	// Pointer to ROM6, second bank
	UINT16  *m_rom6b;

	// AMS RAM (1 Mib)
	std::vector<UINT16> m_ram;

	// Scratch pad ram (1 KiB)
	std::vector<UINT16> m_scratchpad;

	// First joystick. 6 for TI-99/4A
	int     m_firstjoy;

	// READY line
	int     m_ready_line, m_ready_line_dmux;

private:
	DECLARE_READ16_MEMBER( datamux_read );
	DECLARE_WRITE16_MEMBER( datamux_write );
	void    set_keyboard_column(int number, int data);

	int     m_keyboard_column;
	int     m_check_alphalock;

	// True if SGCPU DSR is enabled
	bool m_internal_dsr;

	// True if SGCPU rom6 is enabled
	bool m_internal_rom6;

	// Offset to the ROM6 bank.
	int m_rom6_bank;

	// Wait states
	int m_waitcount;

	// TRUE when mapper is active
	bool m_map_mode;

	// TRUE when mapper registers are accessible
	bool m_access_mapper;

	UINT8   m_lowbyte;
	UINT8   m_highbyte;
	UINT8   m_latch;

	// Mapper registers
	UINT8 m_mapper[16];

	// Latch for 9901 INT2, INT1 lines
	int     m_9901_int;
	void    set_9901_int(int line, line_state state);

	int     m_ready_prev;       // for debugging purposes only

};

static ADDRESS_MAP_START(memmap, AS_PROGRAM, 16, ti99_4p_state)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE( memread, memwrite )
ADDRESS_MAP_END

static ADDRESS_MAP_START(cru_map, AS_IO, 8, ti99_4p_state)
	AM_RANGE(0x0000, 0x003f) AM_DEVREAD(TMS9901_TAG, tms9901_device, read)
	AM_RANGE(0x0000, 0x01ff) AM_READ( cruread )

	AM_RANGE(0x0000, 0x01ff) AM_DEVWRITE(TMS9901_TAG, tms9901_device, write)
	AM_RANGE(0x0000, 0x0fff) AM_WRITE( cruwrite )
ADDRESS_MAP_END

/*
    Input ports, used by machine code for TI keyboard and joystick emulation.

    Since the keyboard microcontroller is not emulated, we use the TI99/4a 48-key keyboard,
    plus two optional joysticks.
*/

static INPUT_PORTS_START(ti99_4p)
	/* 4 ports for keyboard and joystick */
	PORT_START("COL0")  // col 0
		PORT_BIT(0x88, IP_ACTIVE_LOW, IPT_UNUSED)
		/* The original control key is located on the left, but we accept the right control key as well */
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")      PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		/* TI99/4a has a second shift key which maps the same */
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		/* The original function key is located on the right, but we accept the left alt key as well */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN")      PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT")  PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHAR(UCHAR_MAMEKEY(F12))

	PORT_START("COL1")  // col 1
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR('~')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK")  PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR('\'')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("COL2")  // col 2
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('`')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 # ERASE") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(UCHAR_MAMEKEY(F3))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO")  PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(UCHAR_MAMEKEY(F8))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR('?')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("COL3")  // col 3
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('[')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID")   PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("COL4")  // col 4
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(']')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN")  PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("COL5")  // col 5
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR('\\')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR('|')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR(UCHAR_MAMEKEY(F10))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('\"')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('-')

	PORT_START("ALPHA") /* one more port for Alpha line */
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alpha Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE


INPUT_PORTS_END

/*
    Memory access
*/
READ16_MEMBER( ti99_4p_state::memread )
{
	int addroff = offset << 1;
	if (m_rom0 == nullptr) return 0;   // premature access

	UINT16 zone = addroff & 0xe000;
	UINT16 value = 0;

	if (zone==0x0000)
	{
		// ROM0
		value = m_rom0[(addroff & 0x1fff)>>1];
		return value;
	}
	if (zone==0x2000 || zone==0xa000 || zone==0xc000 || zone==0xe000)
	{
		value = samsmem_read(space, offset, mem_mask);
		return value;
	}

	if (zone==0x4000)
	{
		if (m_internal_dsr)
		{
			value = m_dsr[(addroff & 0x1fff)>>1];
			return value;
		}
		else
		{
			if (m_access_mapper && ((addroff & 0xffe0)==0x4000))
			{
				value = m_mapper[offset & 0x000f]<<8;
				return value;
			}
		}
	}

	if (zone==0x6000 && m_internal_rom6)
	{
		if (m_rom6_bank==0)
			value = m_rom6a[(addroff & 0x1fff)>>1];
		else
			value = m_rom6b[(addroff & 0x1fff)>>1];

		return value;
	}

	// Scratch pad RAM and sound
	// speech is in peribox
	// groms are in hsgpl in peribox
	if (zone==0x8000)
	{
		if ((addroff & 0xfff0)==0x8400) // cannot read from sound
		{
			value = 0;
			return value;
		}
		if ((addroff & 0xfc00)==0x8000)
		{
			value = m_scratchpad[(addroff & 0x03ff)>>1];
			return value;
		}
		// Video: 8800, 8802
		if ((addroff & 0xfffd)==0x8800)
		{
			value = m_video->read16(space, offset, mem_mask);
			return value;
		}
	}

	// If we are here, check the peribox via the datamux
	// catch-all for unmapped zones
	value = datamux_read(space, offset, mem_mask);
	return value;
}

WRITE16_MEMBER( ti99_4p_state::memwrite )
{
//  m_cpu->adjust_icount(-4);

	int addroff = offset << 1;
	UINT16 zone = addroff & 0xe000;

	if (zone==0x0000)
	{
		// ROM0
		if (VERBOSE>4) LOG("sgcpu: ignoring ROM write access at %04x\n", addroff);
		return;
	}

	if (zone==0x2000 || zone==0xa000 || zone==0xc000 || zone==0xe000)
	{
		samsmem_write(space, offset, data, mem_mask);
		return;
	}

	if (zone==0x4000)
	{
		if (m_internal_dsr)
		{
			if (VERBOSE>4) LOG("sgcpu: ignoring DSR write access at %04x\n", addroff);
			return;
		}
		else
		{
			if (m_access_mapper && ((addroff & 0xffe0)==0x4000))
			{
				m_mapper[offset & 0x000f] = data;
				return;
			}
		}
	}

	if (zone==0x6000 && m_internal_rom6)
	{
		m_rom6_bank = offset & 0x0001;
		return;
	}

	// Scratch pad RAM and sound
	// speech is in peribox
	// groms are in hsgpl in peribox
	if (zone==0x8000)
	{
		if ((addroff & 0xfff0)==0x8400)     //sound write
		{
			m_sound->write(space, 0, (data >> 8) & 0xff);
			return;
		}
		if ((addroff & 0xfc00)==0x8000)
		{
			m_scratchpad[(addroff & 0x03ff)>>1] = data;
			return;
		}
		// Video: 8C00, 8C02
		if ((addroff & 0xfffd)==0x8c00)
		{
			m_video->write16(space, offset, data, mem_mask);
			return;
		}
	}

	// If we are here, check the peribox via the datamux
	// catch-all for unmapped zones
	datamux_write(space, offset, data, mem_mask);
}

/***************************************************************************
    Internal datamux; similar to TI-99/4A. However, here we have just
    one device, the peripheral box, so it is much simpler.
***************************************************************************/

/*
    The datamux is connected to the clock line in order to operate
    the wait state counter.
*/
void ti99_4p_state::clock_in(int clock)
{
	if (clock==ASSERT_LINE && m_waitcount!=0)
	{
		m_waitcount--;
		if (m_waitcount==0) console_ready_dmux(ASSERT_LINE);
	}
}


READ16_MEMBER( ti99_4p_state::datamux_read )
{
	UINT8 hbyte = 0;
	UINT16 addroff = (offset << 1);

	m_peribox->readz(space, addroff+1, &m_latch, mem_mask);
	m_lowbyte = m_latch;

	m_peribox->readz(space, addroff, &hbyte, mem_mask);
	m_highbyte = hbyte;

	// use the latch and the currently read byte and put it on the 16bit bus
//  printf("read  address = %04x, value = %04x, memmask = %4x\n", addroff,  (hbyte<<8) | sgcpu->latch, mem_mask);

	// Insert four wait states and let CPU enter wait state
	m_waitcount = 6;
	console_ready_dmux(CLEAR_LINE);

	return (hbyte<<8) | m_latch ;
}

/*
    Write access.
    TODO: use the 16-bit expansion in the box for suitable cards
*/
WRITE16_MEMBER( ti99_4p_state::datamux_write )
{
	UINT16 addroff = (offset << 1);
//  printf("write address = %04x, value = %04x, memmask = %4x\n", addroff, data, mem_mask);

	// read more about the datamux in datamux.c

	// Write to the PEB
	m_peribox->write(space, addroff+1, data & 0xff);

	// Write to the PEB
	m_peribox->write(space, addroff, (data>>8) & 0xff);

	// Insert four wait states and let CPU enter wait state
	m_waitcount = 6;
	console_ready_dmux(CLEAR_LINE);
}

/***************************************************************************
   CRU interface
***************************************************************************/

#define MAP_CRU_BASE 0x0f00
#define SAMS_CRU_BASE 0x1e00

/*
    CRU write
*/
WRITE8_MEMBER( ti99_4p_state::cruwrite )
{
	int addroff = offset<<1;

	if ((addroff & 0xff00)==MAP_CRU_BASE)
	{
		if ((addroff & 0x000e)==0)  m_internal_dsr = data;
		if ((addroff & 0x000e)==2)  m_internal_rom6 = data;
		if ((addroff & 0x000e)==4)  m_peribox->senila((data!=0)? ASSERT_LINE : CLEAR_LINE);
		if ((addroff & 0x000e)==6)  m_peribox->senilb((data!=0)? ASSERT_LINE : CLEAR_LINE);
		// TODO: more CRU bits? 8=Fast timing / a=KBENA
		return;
	}
	if ((addroff & 0xff00)==SAMS_CRU_BASE)
	{
		if ((addroff & 0x000e)==0) m_access_mapper = data;
		if ((addroff & 0x000e)==2) m_map_mode = data;
		return;
	}

	// No match - pass to peribox
	m_peribox->cruwrite(space, addroff, data);
}

READ8_MEMBER( ti99_4p_state::cruread )
{
	UINT8 value = 0;
	m_peribox->crureadz(space, offset<<4, &value);
	return value;
}

/***************************************************************************
   AMS Memory implementation
***************************************************************************/

/*
    Memory read. The SAMS card has two address areas: The memory is at locations
    0x2000-0x3fff and 0xa000-0xffff, and the mapper area is at 0x4000-0x401e
    (only even addresses).
*/
READ16_MEMBER( ti99_4p_state::samsmem_read )
{
	UINT32 address = 0;
	int addroff = offset << 1;

	// select memory expansion
	if (m_map_mode)
		address = (m_mapper[(addroff>>12) & 0x000f] << 12) + (addroff & 0x0fff);
	else // transparent mode
		address = addroff;

	return m_ram[address>>1];
}

/*
    Memory write
*/
WRITE16_MEMBER( ti99_4p_state::samsmem_write )
{
	UINT32 address = 0;
	int addroff = offset << 1;

	// select memory expansion
	if (m_map_mode)
		address = (m_mapper[(addroff>>12) & 0x000f] << 12) + (addroff & 0x0fff);
	else // transparent mode
		address = addroff;

	m_ram[address>>1] = data;
}

/***************************************************************************
    Keyboard/tape control
****************************************************************************/
static const char *const column[] = { "COL0", "COL1", "COL2", "COL3", "COL4", "COL5" };

READ8_MEMBER( ti99_4p_state::read_by_9901 )
{
	int answer=0;

	switch (offset & 0x03)
	{
	case TMS9901_CB_INT7:
		// Read pins INT3*-INT7* of TI99's 9901.
		// bit 1: INT1 status
		// bit 2: INT2 status
		// bit 3-7: keyboard status bits 0 to 4
		//
		// |K|K|K|K|K|I2|I1|C|
		//
		if (m_keyboard_column >= m_firstjoy) // joy 1 and 2
		{
			answer = m_joyport->read_port();
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		if (m_check_alphalock)
		{
			answer &= ~(ioport("ALPHA")->read());
		}
		answer = (answer << 3) | m_9901_int;
		break;

	case TMS9901_INT8_INT15:
		// Read pins INT8*-INT15* of TI99's 9901.
		// bit 0-2: keyboard status bits 5 to 7
		// bit 3: tape input mirror
		// bit 5-7: weird, not emulated

		// |1|1|1|1|0|K|K|K|
		if (m_keyboard_column >= m_firstjoy) answer = 0x07;
		else answer = ((ioport(column[m_keyboard_column])->read())>>5) & 0x07;
		answer |= 0xf0;
		break;

	case TMS9901_P0_P7:
		break;

	case TMS9901_P8_P15:
		// Read pins P8-P15 of TI99's 9901.
		// bit 26: high
		// bit 27: tape input
		answer = 4;
		if (m_cassette->input() > 0) answer |= 8;
		break;
	}
	return answer;
}

/*
    WRITE key column select (P2-P4)
*/
void ti99_4p_state::set_keyboard_column(int number, int data)
{
	if (data!=0)    m_keyboard_column |= 1 << number;
	else            m_keyboard_column &= ~(1 << number);

	if (m_keyboard_column >= m_firstjoy)
	{
		m_joyport->write_port(m_keyboard_column - m_firstjoy + 1);
	}
}

WRITE_LINE_MEMBER( ti99_4p_state::keyC0 )
{
	set_keyboard_column(0, state);
}

WRITE_LINE_MEMBER( ti99_4p_state::keyC1 )
{
	set_keyboard_column(1, state);
}

WRITE_LINE_MEMBER( ti99_4p_state::keyC2 )
{
	set_keyboard_column(2, state);
}

/*
    WRITE alpha lock line (P5)
*/
WRITE_LINE_MEMBER( ti99_4p_state::alphaW )
{
	m_check_alphalock = (state==0);
}

/*
    command CS1 (only) tape unit motor (P6)
*/
WRITE_LINE_MEMBER( ti99_4p_state::cs_motor )
{
	m_cassette->change_state((state!=0)? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

/*
    audio gate (P8)

    Set to 1 before using tape: this enables the mixing of tape input sound
    with computer sound.

    We do not really need to emulate this as the tape recorder generates sound
    on its own.
*/
WRITE_LINE_MEMBER( ti99_4p_state::audio_gate )
{
}

/*
    tape output (P9)
*/
WRITE_LINE_MEMBER( ti99_4p_state::cassette_output )
{
	m_cassette->output((state!=0)? +1 : -1);
}

/*
// TMS9901 setup. The callback functions pass a reference to the TMS9901 as device.
const tms9901_interface tms9901_wiring_sgcpu =
{
    TMS9901_INT1 | TMS9901_INT2 | TMS9901_INTC, // only input pins whose state is always known

    // read handler
    DEVCB_DRIVER_MEMBER(ti99_4p_state, read_by_9901),

    {   // write handlers
        DEVCB_NULL,
        DEVCB_NULL,
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, keyC0),
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, keyC1),
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, keyC2),
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, alphaW),
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, cs_motor),
        DEVCB_NULL,
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, audio_gate),
        DEVCB_DRIVER_LINE_MEMBER(ti99_4p_state, cassette_output),
        DEVCB_NULL,
        DEVCB_NULL,
        DEVCB_NULL,
        DEVCB_NULL,
        DEVCB_NULL,
        DEVCB_NULL
    },

    // interrupt handler
    DEVCB_DRIVER_MEMBER(ti99_4p_state, tms9901_interrupt)
};

*/

/***************************************************************************
    Control lines
****************************************************************************/

/*
    We may have lots of devices pulling down this line; so we should use a AND
    gate to do it right. On the other hand, when READY is down, there is just
    no chance to make another device pull down the same line; the CPU just
    won't access any other device in this time.
*/
WRITE_LINE_MEMBER( ti99_4p_state::console_ready )
{
	m_ready_line = state;
	int combined = (m_ready_line == ASSERT_LINE && m_ready_line_dmux == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE;

	if (VERBOSE>6)
	{
		if (m_ready_prev != combined) LOG("ti99_4p: READY level = %d\n", combined);
	}
	m_ready_prev = combined;
	m_cpu->set_ready(combined);
}

/*
    The exception of the above rule. Memory access over the datamux also operates
    the READY line, and the datamux raises READY depending on the clock pulse.
    So we must make sure this does not interfere.
*/
WRITE_LINE_MEMBER( ti99_4p_state::console_ready_dmux )
{
	m_ready_line_dmux = state;
	int combined = (m_ready_line == ASSERT_LINE && m_ready_line_dmux == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE;

	if (VERBOSE>7)
	{
		if (m_ready_prev != combined) LOG("ti99_4p: READY dmux level = %d\n", state);
	}
	m_ready_prev = combined;
	m_cpu->set_ready(combined);
}

void ti99_4p_state::set_9901_int( int line, line_state state)
{
	m_tms9901->set_single_int(line, state);
	// We latch the value for the read operation. Mind the negative logic.
	if (state==CLEAR_LINE) m_9901_int |= (1<<line);
	else m_9901_int &= ~(1<<line);
}

WRITE_LINE_MEMBER( ti99_4p_state::extint )
{
	if (VERBOSE>6) LOG("ti99_4p: EXTINT level = %02x\n", state);
	set_9901_int(1, (line_state)state);
}

WRITE_LINE_MEMBER( ti99_4p_state::notconnected )
{
	if (VERBOSE>6) LOG("ti99_4p: Setting a not connected line ... ignored\n");
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
WRITE_LINE_MEMBER( ti99_4p_state::clock_out )
{
	clock_in(state);
}

WRITE8_MEMBER( ti99_4p_state::tms9901_interrupt )
{
	// offset contains the interrupt level (0-15)
	// However, the TI board just ignores that level and hardwires it to 1
	// See below (interrupt_level)
	m_cpu->set_input_line(INT_9900_INTREQ, data);
}

READ8_MEMBER( ti99_4p_state::interrupt_level )
{
	// On the TI-99 systems these IC lines are not used; the input lines
	// at the CPU are hardwired to level 1.
	return 1;
}

WRITE8_MEMBER( ti99_4p_state::external_operation )
{
	static const char* extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	if (VERBOSE>1) LOG("External operation %s not implemented on the SGCPU board\n", extop[offset]);
}

/*****************************************************************************/

void ti99_4p_state::machine_start()
{
	m_ram.resize(0x80000/2);
	m_scratchpad.resize(0x400/2);

	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);

	m_firstjoy = 6;

	m_ready_line = m_ready_line_dmux = ASSERT_LINE;

	UINT16 *rom = (UINT16*)(memregion("maincpu")->base());
	m_rom0  = rom + 0x2000;
	m_dsr   = rom + 0x6000;
	m_rom6a = rom + 0x3000;
	m_rom6b = rom + 0x7000;
}

/*
    set the state of int2 (called by the v9938)
*/
WRITE_LINE_MEMBER(ti99_4p_state::set_tms9901_INT2_from_v9938)
{
	set_9901_int(2, (line_state)state);
}

/*
    Reset the machine.
*/
MACHINE_RESET_MEMBER(ti99_4p_state,ti99_4p)
{
	set_9901_int(12, CLEAR_LINE);

	m_cpu->set_ready(ASSERT_LINE);
	m_cpu->set_hold(CLEAR_LINE);
	m_9901_int = 0x03; // INT2* and INT1* set to 1, i.e. inactive
}


/*
    Machine description.
*/
static MACHINE_CONFIG_START( ti99_4p_60hz, ti99_4p_state )
	/* basic machine hardware */
	/* TMS9900 CPU @ 3.0 MHz */
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map)
	MCFG_TMS99xx_EXTOP_HANDLER( WRITE8(ti99_4p_state, external_operation) )
	MCFG_TMS99xx_INTLEVEL_HANDLER( READ8(ti99_4p_state, interrupt_level) )
	MCFG_TMS99xx_CLKOUT_HANDLER( WRITELINE(ti99_4p_state, clock_out) )

	/* video hardware */
	MCFG_DEVICE_ADD(VIDEO_SYSTEM_TAG, V9938VIDEO, 0)
	MCFG_V9938_ADD(VDP_TAG, SCREEN_TAG, 0x20000, XTAL_21_4772MHz)  /* typical 9938 clock, not verified */
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(ti99_4p_state, set_tms9901_INT2_from_v9938))
	MCFG_V99X8_SCREEN_ADD_NTSC(SCREEN_TAG, VDP_TAG, XTAL_21_4772MHz)

	// tms9901
	MCFG_DEVICE_ADD(TMS9901_TAG, TMS9901, 3000000)
	MCFG_TMS9901_READBLOCK_HANDLER( READ8(ti99_4p_state, read_by_9901) )
	MCFG_TMS9901_P2_HANDLER( WRITELINE( ti99_4p_state, keyC0) )
	MCFG_TMS9901_P3_HANDLER( WRITELINE( ti99_4p_state, keyC1) )
	MCFG_TMS9901_P4_HANDLER( WRITELINE( ti99_4p_state, keyC2) )
	MCFG_TMS9901_P6_HANDLER( WRITELINE( ti99_4p_state, cs_motor) )
	MCFG_TMS9901_P8_HANDLER( WRITELINE( ti99_4p_state, audio_gate) )
	MCFG_TMS9901_P9_HANDLER( WRITELINE( ti99_4p_state, cassette_output) )
	MCFG_TMS9901_INTLEVEL_HANDLER( WRITE8( ti99_4p_state, tms9901_interrupt) )

	// Peripheral expansion box (SGCPU composition)
	MCFG_DEVICE_ADD( PERIBOX_TAG, PERIBOX_SG, 0)
	MCFG_PERIBOX_INTA_HANDLER( WRITELINE(ti99_4p_state, extint) )
	MCFG_PERIBOX_INTB_HANDLER( WRITELINE(ti99_4p_state, notconnected) )
	MCFG_PERIBOX_READY_HANDLER( WRITELINE(ti99_4p_state, console_ready) )

	// sound hardware
	MCFG_TI_SOUND_94624_ADD( TISOUND_TAG )
	MCFG_TI_SOUND_READY_HANDLER( WRITELINE(ti99_4p_state, console_ready) )

	// Cassette drives
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( "cassette" )

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG )

MACHINE_CONFIG_END


ROM_START(ti99_4p)
	/*CPU memory space*/
	ROM_REGION16_BE(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("sgcpu_hb.bin", 0x0000, 0x8000, CRC(aa100730) SHA1(35e585b2dcd3f2a0005bebb15ede6c5b8c787366) ) /* system ROMs */
	ROM_LOAD16_BYTE("sgcpu_lb.bin", 0x0001, 0x8000, CRC(2a5dc818) SHA1(dec141fe2eea0b930859cbe1ebd715ac29fa8ecb) ) /* system ROMs */
ROM_END

/*    YEAR  NAME      PARENT   COMPAT   MACHINE      INPUT    INIT      COMPANY     FULLNAME */
COMP( 1996, ti99_4p,  0,       0,       ti99_4p_60hz, ti99_4p, driver_device, 0, "System 99 Users Group",       "SGCPU (a.k.a. 99/4P)" , 0 )
