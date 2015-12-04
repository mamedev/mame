// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    tk2000.c - Microdigital TK2000

    Driver by R. Belmont

    This system is only vaguely Apple II compatible.
    The keyboard works entirely differently, which is a big deal.

    $C05A - banks RAM from c100-ffff
    $C05B - banks ROM from c100-ffff

************************************************************************/

#include "emu.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/ap2_dsk.h"
#include "cpu/m6502/m6502.h"
#include "video/apple2.h"

#define A2_CPU_TAG "maincpu"
#define A2_BUS_TAG "a2bus"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define A2_VIDEO_TAG "a2video"

class tk2000_state : public driver_device
{
public:
	tk2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_ram(*this, RAM_TAG),
		m_video(*this, A2_VIDEO_TAG),
		m_row0(*this, "ROW0"),
		m_row1(*this, "ROW1"),
		m_row2(*this, "ROW2"),
		m_row3(*this, "ROW3"),
		m_row4(*this, "ROW4"),
		m_row5(*this, "ROW5"),
		m_row6(*this, "ROW6"),
		m_row7(*this, "ROW7"),
		m_kbspecial(*this, "keyb_special"),
		m_sysconfig(*this, "a2_config"),
		m_speaker(*this, A2_SPEAKER_TAG),
		m_cassette(*this, A2_CASSETTE_TAG),
		m_upperbank(*this, A2_UPPERBANK_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<a2_video_device> m_video;
	required_ioport m_row0, m_row1, m_row2, m_row3, m_row4, m_row5, m_row6, m_row7;
	required_ioport m_kbspecial;
	required_ioport m_sysconfig;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_upperbank;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_PALETTE_INIT(tk2000);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(c000_r);
	DECLARE_WRITE8_MEMBER(c000_w);
	DECLARE_READ8_MEMBER(c080_r);
	DECLARE_WRITE8_MEMBER(c080_w);
	DECLARE_READ8_MEMBER(c100_r);
	DECLARE_WRITE8_MEMBER(c100_w);

private:
	int m_speaker_state;
	int m_cassette_state;

	UINT8 m_strobe;

	bool m_page2;

	UINT8 *m_ram_ptr;
	int m_ram_size;

	void do_io(address_space &space, int offset);
	UINT8 read_floatingbus();
};

/***************************************************************************
    START/RESET
***************************************************************************/

void tk2000_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_cassette_state = 0;
	m_cassette->output(-1.0f);
	m_upperbank->set_bank(0);

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_strobe));
	save_item(NAME(m_page2));

	// setup video pointers
	m_video->m_ram_ptr = m_ram_ptr;
	m_video->m_aux_ptr = m_ram_ptr;
	m_video->m_char_ptr = nullptr;
	m_video->m_char_size = 0;
}

void tk2000_state::machine_reset()
{
	m_page2 = false;
	m_strobe = 0;
}

/***************************************************************************
    VIDEO
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(tk2000_state::apple2_interrupt)
{
	int scanline = param;

	if((scanline % 8) == 0)
		machine().first_screen()->update_partial(machine().first_screen()->vpos());

	// update the video system's shadow copy of the system config at the end of the frame
	if (scanline == 192)
	{
		m_video->m_sysconfig = m_sysconfig->read();
	}
}

PALETTE_INIT_MEMBER(tk2000_state, tk2000)
{
	m_video->palette_init_apple2(palette);
}

UINT32 tk2000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_video->hgr_update(screen, bitmap, cliprect, 0, 191);
	return 0;
}

/***************************************************************************
    I/O
***************************************************************************/
// most softswitches don't care about read vs write, so handle them here
void tk2000_state::do_io(address_space &space, int offset)
{
	if(space.debugger_access())
	{
		return;
	}

	switch (offset)
	{
		case 0x20:
			m_cassette_state ^= 1;
			m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
			break;

		case 0x30:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x50:  // monochrome
			break;

		case 0x51:  // color
			break;

		case 0x54:  // set page 1
			m_page2 = false;
			m_video->m_page2 = false;
			break;

		case 0x55:  // set page 2
			m_page2 = true;
			m_video->m_page2 = true;
			break;

		case 0x5a:  // ROM
			m_upperbank->set_bank(0);
			break;

		case 0x5b:  // RAM
			m_upperbank->set_bank(1);
			break;

		case 0x5e:
			break;

		default:
			printf("do_io: unk access @ $C0%02X\n", offset & 0xff);
			break;
	}
}

READ8_MEMBER(tk2000_state::c000_r)
{
	switch (offset)
	{
		case 0x00:
			return 0;

		case 0x10:  // keyboard strobe
			return m_strobe;

		case 0x60: // cassette in
		case 0x68:
			return m_cassette->input() > 0.0 ? 0x80 : 0;

		default:
			do_io(space, offset);
			break;
	}

	return read_floatingbus();
}

WRITE8_MEMBER(tk2000_state::c000_w)
{
	switch (offset)
	{
		case 0x00:  // write row mask for keyboard scan
			switch (data)
			{
				case 0:
					break;

				case 0x01: m_strobe = m_row0->read(); break;
				case 0x02: m_strobe = m_row1->read(); break;
				case 0x04: m_strobe = m_row2->read(); break;
				case 0x08: m_strobe = m_row3->read(); break;
				case 0x10: m_strobe = m_row4->read(); break;
				case 0x20: m_strobe = m_row5->read(); break;
				case 0x40: m_strobe = m_row6->read(); break;
				case 0x80: m_strobe = m_row7->read(); break;
			}
			break;

		case 0x5f:
			m_strobe = m_kbspecial->read();
			break;

		default:
			do_io(space, offset);
			break;
	}
}

READ8_MEMBER(tk2000_state::c080_r)
{
	return read_floatingbus();
}

WRITE8_MEMBER(tk2000_state::c080_w)
{
}

READ8_MEMBER(tk2000_state::c100_r)
{
	return m_ram_ptr[offset + 0xc100];
}

WRITE8_MEMBER(tk2000_state::c100_w)
{
	m_ram_ptr[offset + 0xc100] = data;
}

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
UINT8 tk2000_state::read_floatingbus()
{
	enum
	{
		// scanner types
		kScannerNone = 0, kScannerApple2, kScannerApple2e,

		// scanner constants
		kHBurstClock      =    53, // clock when Color Burst starts
		kHBurstClocks     =     4, // clocks per Color Burst duration
		kHClock0State     =  0x18, // H[543210] = 011000
		kHClocks          =    65, // clocks per horizontal scan (including HBL)
		kHPEClock         =    40, // clock when HPE (horizontal preset enable) goes low
		kHPresetClock     =    41, // clock when H state presets
		kHSyncClock       =    49, // clock when HSync starts
		kHSyncClocks      =     4, // clocks per HSync duration
		kNTSCScanLines    =   262, // total scan lines including VBL (NTSC)
		kNTSCVSyncLine    =   224, // line when VSync starts (NTSC)
		kPALScanLines     =   312, // total scan lines including VBL (PAL)
		kPALVSyncLine     =   264, // line when VSync starts (PAL)
		kVLine0State      = 0x100, // V[543210CBA] = 100000000
		kVPresetLine      =   256, // line when V state presets
		kVSyncLines       =     4, // lines per VSync duration
		kClocksPerVSync   = kHClocks * kNTSCScanLines // FIX: NTSC only?
	};

	// vars
	//
	int i, Hires, Mixed, Page2, _80Store, ScanLines, /* VSyncLine, ScanCycles,*/
		h_clock, h_state, h_0, h_1, h_2, h_3, h_4, h_5,
		v_line, v_state, v_A, v_B, v_C, v_0, v_1, v_2, v_3, v_4, /* v_5, */
		_hires, addend0, addend1, addend2, sum, address;

	// video scanner data
	//
	i = m_maincpu->total_cycles() % kClocksPerVSync; // cycles into this VSync

	// machine state switches
	//
	Hires    = 1; //m_video->m_hires ? 1 : 0;
	Mixed    = 0; //m_video->m_mix ? 1 : 0;
	Page2    = m_page2 ? 1 : 0;
	_80Store = 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	//
	h_clock = (i + kHPEClock) % kHClocks; // which horizontal scanning clock
	h_state = kHClock0State + h_clock; // H state bits
	if (h_clock >= kHPresetClock) // check for horizontal preset
	{
		h_state -= 1; // correct for state preset (two 0 states)
	}
	h_0 = (h_state >> 0) & 1; // get horizontal state bits
	h_1 = (h_state >> 1) & 1;
	h_2 = (h_state >> 2) & 1;
	h_3 = (h_state >> 3) & 1;
	h_4 = (h_state >> 4) & 1;
	h_5 = (h_state >> 5) & 1;

	// calculate vertical scanning state
	//
	v_line  = i / kHClocks; // which vertical scanning line
	v_state = kVLine0State + v_line; // V state bits
	if ((v_line >= kVPresetLine)) // check for previous vertical state preset
	{
		v_state -= ScanLines; // compensate for preset
	}
	v_A = (v_state >> 0) & 1; // get vertical state bits
	v_B = (v_state >> 1) & 1;
	v_C = (v_state >> 2) & 1;
	v_0 = (v_state >> 3) & 1;
	v_1 = (v_state >> 4) & 1;
	v_2 = (v_state >> 5) & 1;
	v_3 = (v_state >> 6) & 1;
	v_4 = (v_state >> 7) & 1;
	//v_5 = (v_state >> 8) & 1;

	// calculate scanning memory address
	//
	_hires = Hires;
	if (Hires && Mixed && (v_4 & v_2))
	{
		_hires = 0; // (address is in text memory)
	}

	addend0 = 0x68; // 1            1            0            1
	addend1 =              (h_5 << 5) | (h_4 << 4) | (h_3 << 3);
	addend2 = (v_4 << 6) | (v_3 << 5) | (v_4 << 4) | (v_3 << 3);
	sum     = (addend0 + addend1 + addend2) & (0x0F << 3);

	address = 0;
	address |= h_0 << 0; // a0
	address |= h_1 << 1; // a1
	address |= h_2 << 2; // a2
	address |= sum;      // a3 - aa6
	address |= v_0 << 7; // a7
	address |= v_1 << 8; // a8
	address |= v_2 << 9; // a9
	address |= ((_hires) ? v_A : (1 ^ (Page2 & (1 ^ _80Store)))) << 10; // a10
	address |= ((_hires) ? v_B : (Page2 & (1 ^ _80Store))) << 11; // a11
	if (_hires) // hires?
	{
		// Y: insert hires only address bits
		//
		address |= v_C << 12; // a12
		address |= (1 ^ (Page2 & (1 ^ _80Store))) << 13; // a13
		address |= (Page2 & (1 ^ _80Store)) << 14; // a14
	}
	else
	{
		// N: text, so no higher address bits unless Apple ][, not Apple //e
		//
		if ((1) && // Apple ][? // FIX: check for Apple ][? (FB is most useful in old games)
			(kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	return m_ram_ptr[address % m_ram_size]; // FIX: this seems to work, but is it right!?
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

READ8_MEMBER(tk2000_state::ram_r)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

WRITE8_MEMBER(tk2000_state::ram_w)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

static ADDRESS_MAP_START( apple2_map, AS_PROGRAM, 8, tk2000_state )
	AM_RANGE(0x0000, 0xbfff) AM_READWRITE(ram_r, ram_w)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_r, c000_w)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inhbank_map, AS_PROGRAM, 8, tk2000_state )
	AM_RANGE(0x0000, 0x3eff) AM_ROM AM_REGION("maincpu", 0x100)
	AM_RANGE(0x4000, 0x7eff) AM_READWRITE(c100_r, c100_w)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*
    TK2000 matrix:

          0  1 2 3 4 5 6 7
       0SHIF B V C X Z
       1     G F D S A
       2 SPC T R E W Q
       3 LFT 5 4 3 2 1
       4 RGT 6 7 8 9 0
       5 DWN Y U I O P
       6 UP  H J K L :
       7 RTN N M , . ?

       write row mask 1/2/4/8/10/20/40/80 to $C000
       read column at $C010

       If $C05F is written, the Ctrl key is read in bit 0 of $C010 immediately afterwards.
*/
static INPUT_PORTS_START( tk2000 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")   PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("a2_config")
	PORT_CONFNAME(0x03, 0x00, "Composite monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x01, "B&W")
	PORT_CONFSETTING(0x02, "Green")
	PORT_CONFSETTING(0x03, "Amber")
INPUT_PORTS_END

static MACHINE_CONFIG_START( tk2000, tk2000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(A2_CPU_TAG, M6502, 1021800)     /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", tk2000_state, apple2_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD(A2_VIDEO_TAG, APPLE2_VIDEO, XTAL_14_31818MHz)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(280*2, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, (280*2)-1,0,192-1)
	MCFG_SCREEN_UPDATE_DRIVER(tk2000_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(tk2000_state, tk2000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(A2_SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* /INH banking */
	MCFG_DEVICE_ADD(A2_UPPERBANK_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(inhbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	MCFG_CASSETTE_ADD(A2_CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START(tk2000)
	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "tk2000.rom",   0x000000, 0x004000, CRC(dfdbacc3) SHA1(bb37844c31616046630868a4399ee3d55d6df277) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     INIT      COMPANY            FULLNAME */
COMP( 1984, tk2000,   0,        0,        tk2000,      tk2000,  driver_device,   0,        "Microdigital",    "TK2000", MACHINE_NOT_WORKING )
