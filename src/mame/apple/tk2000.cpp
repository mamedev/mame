// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Capdeville
/***************************************************************************

    tk2000.cpp - Microdigital TK2000

    Driver by R. Belmont

    This system is only vaguely Apple II compatible.
    The keyboard works entirely differently, which is a big deal.

    TODO: emulate expansion connector (not wholly Apple II compatible)

    $C05A - banks RAM from c100-ffff
    $C05B - banks ROM from c100-ffff

    Added Multitech MPF-II support ( another not so apple2 compatible )

************************************************************************/

#include "emu.h"

#include "apple2video.h"

#include "cpu/m6502/m6502.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "machine/74259.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define A2_CPU_TAG "maincpu"
#define A2_BUS_TAG "a2bus"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define A2_VIDEO_TAG "a2video"

class tk2000_state : public driver_device
{
public:
	tk2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_ram(*this, RAM_TAG),
		m_screen(*this, "screen"),
		m_video(*this, A2_VIDEO_TAG),
		m_row(*this, "ROW%u", 0U),
		m_kbspecial(*this, "keyb_special"),
		m_speaker(*this, A2_SPEAKER_TAG),
		m_cassette(*this, A2_CASSETTE_TAG),
		m_upperbank(*this, A2_UPPERBANK_TAG),
		m_softlatch(*this, "softlatch"),
		m_printer(*this, "printer")
	{ }

	void tk2000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_device<a2_video_device_composite> m_video;
	required_ioport_array<8> m_row;
	required_ioport m_kbspecial;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_upperbank;
	required_device<addressable_latch_device> m_softlatch;
	required_device<centronics_device> m_printer;

	int m_speaker_state;
	int m_cassette_state;

	uint8_t m_kbout;

	uint8_t *m_ram_ptr;
	int m_ram_size;
	bool m_printer_busy;
	bool m_ctrl_key;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	void printer_busy_w(int state);
	void kbout_w(uint8_t data);
	uint8_t kbin_r();
	uint8_t casout_r();
	void casout_w(uint8_t data);
	uint8_t snd_r();
	void snd_w(uint8_t data);
	uint8_t switches_r(offs_t offset);
	uint8_t cassette_r();
	void color_w(int state);
	void motor_a_w(int state);
	void motor_b_w(int state);
	void rom_ram_w(int state);
	void ctrl_key_w(int state);
	uint8_t c080_r(offs_t offset);
	void c080_w(offs_t offset, uint8_t data);
	uint8_t c100_r(offs_t offset);
	void c100_w(offs_t offset, uint8_t data);

	void apple2_map(address_map &map) ATTR_COLD;
	void inhbank_map(address_map &map) ATTR_COLD;

	uint8_t read_floatingbus();
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
	m_printer_busy = false;
	m_ctrl_key = false;

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_kbout));
	save_item(NAME(m_printer_busy));
	save_item(NAME(m_ctrl_key));

	// setup video pointers
	m_video->set_ram_pointers(m_ram_ptr, m_ram_ptr);
	m_video->set_char_pointer(nullptr, 0);  // no text modes on this machine
}

void tk2000_state::machine_reset()
{
	m_kbout = 0;
}

/***************************************************************************
    VIDEO
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(tk2000_state::apple2_interrupt)
{
	int scanline = param;

	if((scanline % 8) == 0)
		m_screen->update_partial(m_screen->vpos());
}

/***************************************************************************
    I/O
***************************************************************************/

void tk2000_state::printer_busy_w(int state)
{
	m_printer_busy = state;
}

void tk2000_state::kbout_w(uint8_t data)
{
	// write row mask for keyboard scan
	m_kbout = data;

	m_printer->write_data0(BIT(data, 0));
	m_printer->write_data1(BIT(data, 1));
	m_printer->write_data2(BIT(data, 2));
	m_printer->write_data3(BIT(data, 3));
	m_printer->write_data4(BIT(data, 4));
	m_printer->write_data5(BIT(data, 5));
	m_printer->write_data6(BIT(data, 6));
	m_printer->write_data7(BIT(data, 7));
}

uint8_t tk2000_state::kbin_r()
{
	uint8_t kbin = 0;
	for (int i = 0; i < 8; i++)
		if (BIT(m_kbout, i))
			kbin |= m_row[i]->read();

	if (m_ctrl_key)
		kbin |= m_kbspecial->read();

	return kbin | (m_printer_busy ? 0x40 : 0);
}

uint8_t tk2000_state::casout_r()
{
	if (!machine().side_effects_disabled())
		casout_w(0);
	return read_floatingbus();
}

void tk2000_state::casout_w(uint8_t data)
{
	m_cassette_state ^= 1;
	m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
}

uint8_t tk2000_state::snd_r()
{
	if (!machine().side_effects_disabled())
		snd_w(0);
	return read_floatingbus();
}

void tk2000_state::snd_w(uint8_t data)
{
	m_speaker_state ^= 1;
	m_speaker->level_w(m_speaker_state);
}

uint8_t tk2000_state::switches_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_softlatch->write_bit((offset & 0x0e) >> 1, offset & 0x01);
	return read_floatingbus();
}

uint8_t tk2000_state::cassette_r()
{
	return (m_cassette->input() > 0.0 ? 0x80 : 0) | (read_floatingbus() & 0x7f);
}

void tk2000_state::color_w(int state)
{
	// 0 = color, 1 = black/white
}

void tk2000_state::motor_a_w(int state)
{
}

void tk2000_state::motor_b_w(int state)
{
}

void tk2000_state::rom_ram_w(int state)
{
	// 0 = ROM, 1 = RAM
	m_upperbank->set_bank(state);
}

void tk2000_state::ctrl_key_w(int state)
{
	m_ctrl_key = state;
}

uint8_t tk2000_state::c080_r(offs_t offset)
{
	return read_floatingbus();
}

void tk2000_state::c080_w(offs_t offset, uint8_t data)
{
}

uint8_t tk2000_state::c100_r(offs_t offset)
{
	return m_ram_ptr[offset + 0xc100];
}

void tk2000_state::c100_w(offs_t offset, uint8_t data)
{
	m_ram_ptr[offset + 0xc100] = data;
}

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
uint8_t tk2000_state::read_floatingbus()
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
	Hires    = 1;
	Mixed    = 0;
	Page2 = m_video->get_page2() ? 1 : 0;
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

uint8_t tk2000_state::ram_r(offs_t offset)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

void tk2000_state::ram_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

void tk2000_state::apple2_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(tk2000_state::ram_r), FUNC(tk2000_state::ram_w));
	map(0xc000, 0xc000).w(FUNC(tk2000_state::kbout_w)).nopr();
	map(0xc010, 0xc010).r(FUNC(tk2000_state::kbin_r));
	map(0xc020, 0xc020).rw(FUNC(tk2000_state::casout_r), FUNC(tk2000_state::casout_w));
	map(0xc030, 0xc030).rw(FUNC(tk2000_state::snd_r), FUNC(tk2000_state::snd_w));
	map(0xc050, 0xc05f).r(FUNC(tk2000_state::switches_r)).w(m_softlatch, FUNC(addressable_latch_device::write_a0));
	map(0xc060, 0xc060).mirror(8).r(FUNC(tk2000_state::cassette_r));
	map(0xc080, 0xc0ff).rw(FUNC(tk2000_state::c080_r), FUNC(tk2000_state::c080_w));
	map(0xc100, 0xffff).m(m_upperbank, FUNC(address_map_bank_device::amap8));
}

void tk2000_state::inhbank_map(address_map &map)
{
	map(0x0000, 0x3eff).rom().region("maincpu", 0x100);
	map(0x4000, 0x7eff).rw(FUNC(tk2000_state::c100_r), FUNC(tk2000_state::c100_w));
}

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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")        PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)      PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)      PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)      PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)      PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)      PORT_CHAR('Z')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)      PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)      PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)      PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)      PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)      PORT_CHAR('A')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)      PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)      PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)      PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)      PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)      PORT_CHAR('Q')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)      PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR('*')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)      PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)      PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)      PORT_CHAR('I') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)      PORT_CHAR('O') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)      PORT_CHAR('P') PORT_CHAR('+')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)      PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)      PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)      PORT_CHAR('K') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)      PORT_CHAR('L') PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)  PORT_CHAR(':') PORT_CHAR(';')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)      PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)      PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('?') PORT_CHAR('/')

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")     PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void tk2000_state::tk2000(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1021800);     /* close to actual CPU frequency of 1.020484 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tk2000_state::apple2_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(tk2000_state::apple2_interrupt), "screen", 0, 1);
	config.set_maximum_quantum(attotime::from_hz(60));

	APPLE2_VIDEO_COMPOSITE(config, m_video, XTAL(14'318'181)).set_screen(m_screen);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(280*2, 262);
	m_screen->set_visarea(0, (280*2)-1,0,192-1);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::II, false, false>)));
	m_screen->set_palette(m_video);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* /INH banking */
	ADDRESS_MAP_BANK(config, A2_UPPERBANK_TAG).set_map(&tk2000_state::inhbank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	LS259(config, m_softlatch); // U36
	m_softlatch->q_out_cb<0>().set(FUNC(tk2000_state::color_w));
	m_softlatch->q_out_cb<1>().set(FUNC(tk2000_state::motor_a_w));
	m_softlatch->q_out_cb<2>().set(m_video, FUNC(a2_video_device::scr_w));
	m_softlatch->q_out_cb<3>().set(FUNC(tk2000_state::motor_b_w));
	m_softlatch->q_out_cb<4>().set(m_printer, FUNC(centronics_device::write_strobe));
	m_softlatch->q_out_cb<5>().set(FUNC(tk2000_state::rom_ram_w));
	m_softlatch->q_out_cb<7>().set(FUNC(tk2000_state::ctrl_key_w));

	RAM(config, RAM_TAG).set_default_size("64K");

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	CENTRONICS(config, m_printer, centronics_devices, nullptr);
	m_printer->busy_handler().set(FUNC(tk2000_state::printer_busy_w));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START(tk2000)
	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "tk2000.rom",   0x000000, 0x004000, CRC(dfdbacc3) SHA1(bb37844c31616046630868a4399ee3d55d6df277) )
ROM_END

ROM_START(mpf2)
	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "mpf_ii.rom",   0x000000, 0x004000, CRC(8780189f) SHA1(92378b0db561632b58a9b36a85f8fb00796198bb) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY         FULLNAME */
COMP( 1984, tk2000, 0,      0,      tk2000,  tk2000, tk2000_state, empty_init, "Microdigital", "TK2000 Color Computer", MACHINE_NOT_WORKING )
COMP( 1982, mpf2,   tk2000, 0,      tk2000,  tk2000, tk2000_state, empty_init, "Multitech",    "Microprofessor II",     MACHINE_NOT_WORKING )
