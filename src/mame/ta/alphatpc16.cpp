// license:BSD-3-Clause
// copyright-holders:Carl

/******************************************************************************************
The Alphatronic PC 16 was modeled after the earlier 8 bit Alphatronic PC – on the outside
a typical "breadbox" home computer case with an integrated keyboard and external floppy disk drives.
Coming from a typewriter company, it had ambitions for office use - its high price and mismatched
intended use and design coupled with its release date gave it little success. It was released
in the early phase of DOS computers, when IBM’s standard was not yet set into stone, but would
vanquish all "incompatible" solutions within short time.
The start screen allows the selection
of functions by entering the first (two) letters of the command or using the F-key as shown on the screen.

CPU: 8088@5MHz - RAM: 64KB, can be extended to 128KB - ROM: 64KB (monitor and BASIC)

Keyboard: ALP: the key marked with a Greek alpha character makes the computer/printer combo a
          typewriter, everything is typed "through".
          BRK: BREAK stops a BASIC program or acts like CTRL-C
          GRAPH: allows the entering of semigraphics characters, locks into place like CAPSLK
          numeric and BTX function keys:
          UP, DWN, LFT and RWT are marked with their respective arrows and move the cursor
          DEL is marked (R) for BTX mode and reloads the page
          CLR is marked (i) in BTX mode for “info” and loads the start page
          = is marked with a page that’s being turned and is a "reveal" key, e.g. for quizzes
          <-/ is marked with an envelope, it loads BTX’s messaging service
          the + and – keys aren’t defined yet in BTX mode, they are marked with a page and an arrow each
         * shows a screen with a black/white contrast and turns BTX screen attributes on and off
          / has a telephone receiver and three lines and is the online/offline/dialing key

                                                  [RST] [F1 ] [F2] [F3 ] [F4 ] [F5 ] [F6 ]

[ESC] [1 !] [2 ”] [3 §] [4 $] [5 %] [6 &] [7 /] [8 (][9 )] [0 =] [ß ?] [´ `] [BRK] [GRAPH]   [ 7 ] [ 8 ] [ 9 ] [ * ] [ / ]
[TAB  ] [ Q ] [ W ] [ E ] [ R ] [ T ] [ Z ] [ U ] [ I ] [ O ] [ P ] [ Ü ] [+ *] [CTR] [| ]   [ 4 ] [ 5 ] [ 6 ] [ + ] [ - ]
[CAPSLK]  [ A ] [ S ] [ D ] [ F ] [ G ] [ H ] [ J ] [ K ] [ L ] [ Ö ] [ Ä ] [^ #] [  <-- ]   [ 1 ] [ 2 ] [ 3 ] [ = ] [<-/]
[SHFT] [< >] [ Y ] [ X ] [ C ] [ V ] [ B ] [ N ] [ M ] [, ;] [: .] [- _] [  SHFT  ]  [ALP]   [ 0 ] [UP ] [ . ] [DEL] [CLR]
                   [                      LEER                   ]                           [LFT] [DWN] [RGT] [INS] [HOM]

Graphics options: Standard monitor cassette (Cinch for bw and DIN for SCART/RGB connectors) 40/80x21/25
characters, Full graphics cassette (512x256 pixels, 16 colours, vector graphics, 64K video RAM),
BTX cassette (compatible with the standard monitor cassette but includes a modem for BTX functionality)
Floppy: 1 or 2 5.25” DSDD 40 tracks x 5 sectors x 1024 bytes, external via SCSI, z80+wd1770+sa455
Connectors: joystick, cassette recorder (600/1200 BD) FSK, printer (recommended: TRD 7020 or
GABRIELE 9009 typewriter, V24), floppy, module slot
Options: 2 versions of an autonomous processor PCB (Z8671 based, programmable in TinyBasic
via the PC 16 Terminal, operates independently after programming), connects to the printer port
******************************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "video/ef9345.h"
#include "machine/z80sio.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "sound/beep.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class alphatpc16_state : public driver_device
{
public:
	alphatpc16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_ef9345(*this, "ef9345"),
		m_beep(*this, "beeper"),
		m_wdfdc(*this, "wdfdc"),
		m_ram(*this, RAM_TAG),
		m_z80(*this, "z80"),
		m_flop(*this, "wdfdc:%u", 0),
		m_keys(*this, "KEYS.%u", 0)
	{ }

public:
	void alphatpc16(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void apc16_io(address_map &map) ATTR_COLD;
	void apc16_map(address_map &map) ATTR_COLD;
	void apc16_z80_io(address_map &map) ATTR_COLD;
	void apc16_z80_map(address_map &map) ATTR_COLD;
	void ef9345(address_map &map) ATTR_COLD;

	u8 p1_r();
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	u8 host_scsi_r(offs_t offset);
	void host_scsi_w(offs_t offset, u8 data);
	u8 flop_scsi_r(offs_t offset);
	void flop_scsi_w(offs_t offs_t, u8 data);
	u8 p00_r();
	void p40_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<ef9345_device> m_ef9345;
	required_device<beep_device> m_beep;
	required_device<wd1770_device> m_wdfdc;
	required_device<ram_device> m_ram;
	required_device<cpu_device> m_z80;
	required_device_array<floppy_connector, 4> m_flop;
	required_ioport_array<8> m_keys;

	u8 m_p1 = 0, m_p2 = 0, m_data = 0, m_p40 = 0;
	bool m_bsy = false, m_req = false, m_ack = false, m_cd = false, m_io = false, m_sel = false;
};

void alphatpc16_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
	m_wdfdc->set_floppy(m_flop[0]->get_device());

	m_bsy = false;
	m_req = false;
	m_ack = false;
	m_cd = false;
	m_io = false;
	m_sel = false;
}

void alphatpc16_state::p1_w(u8 data)
{
	if((data == 0xff) && !BIT(m_p2, 7))
		m_p2 &= ~3; //??
	m_p1 = data;
}

u8 alphatpc16_state::p1_r()
{
	return m_p1;
}

void alphatpc16_state::p2_w(u8 data)
{
	m_beep->set_state(!BIT(data, 3));
	m_pic8259->ir0_w(BIT(data, 5));
	m_p2 = data;
}

u8 alphatpc16_state::p2_r()
{
	bool key = false;
	if(m_p1 < 0x80)
		key = BIT(m_keys[m_p1 >> 4]->read(), m_p1 & 0xf);
	return (m_p2 | 0x40) & ~(key ? (m_p1 < 0x40 ? 2 : 1) : 0);
}

u8 alphatpc16_state::p00_r()
{
	u8 ret = 0;
	switch(m_p40 & 0xf0)
	{
		case 0x00:
			ret |= m_flop[0]->get_device()->exists() << 3;
			break;
		case 0x10:
			ret |= m_flop[1]->get_device()->exists() << 3;
			break;
		case 0x20:
			ret |= m_flop[2]->get_device()->exists() << 3;
			break;
		case 0x40:
			ret |= m_flop[3]->get_device()->exists() << 3;
			break;
	}
	return ret;
}

void alphatpc16_state::p40_w(u8 data)
{
	switch(data & 0xf0)
	{
		case 0x00:
			m_wdfdc->set_floppy(m_flop[0]->get_device());
			m_flop[0]->get_device()->ss_w(BIT(data, 2));
			break;
		case 0x10:
			m_wdfdc->set_floppy(m_flop[1]->get_device());
			m_flop[1]->get_device()->ss_w(BIT(data, 2));
			break;
		case 0x20:
			m_wdfdc->set_floppy(m_flop[2]->get_device());
			m_flop[2]->get_device()->ss_w(BIT(data, 2));
			break;
		case 0x40:
			m_wdfdc->set_floppy(m_flop[3]->get_device());
			m_flop[3]->get_device()->ss_w(BIT(data, 2));
			break;
	}
	m_p40 = data;
}

// this scsi emulation is an unrealistic mess
void alphatpc16_state::host_scsi_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			m_ack = true;
			m_req = false;
			m_sel = false;
			m_data = data;
			m_z80->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			m_z80->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
			break;
		case 2:
			m_ack = false;
			m_sel = true;
			m_data = data;
			m_z80->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			m_z80->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
			break;
		case 3: //rst ?
			if(data)
				m_z80->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			else
			{
				m_z80->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_bsy = false;
				m_cd = false;
				m_io = false;
			}
			m_z80->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
			break;
	}
	logerror("%s, data %x bsy %d sel %d req %d ack %d cd %d io %d\n", machine().describe_context(), m_data, m_bsy, m_sel, m_req, m_ack, m_cd, m_io);
}

u8 alphatpc16_state::host_scsi_r(offs_t offset)
{
	u8 ret = 0;
	switch(offset)
	{
		case 0:
			if(!m_req && (m_maincpu->state_int(I8086_IP) == 0x2f46))
			{
				m_maincpu->set_state_int(I8086_IP, m_maincpu->state_int(I8086_IP) - 2);
				break;
			}
			m_ack = true;
			m_req = false;
			ret = m_data;
			m_z80->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
			logerror("%s, data %x bsy %d sel %d req %d ack %d cd %d io %d\n", machine().describe_context(), m_data, m_bsy, m_sel, m_req, m_ack, m_cd, m_io);
			break;
		case 1:
			ret = m_req | (m_bsy << 1) | (m_cd << 3) | (m_io << 5); // bit 2 msg?
			break;
	}
	return ret;
}

void alphatpc16_state::flop_scsi_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			if(m_z80->state_int(STATE_GENPC) == 0xcd)
				m_z80->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
			m_req = true;
			m_ack = false;
			m_data = data;
			break;
		case 1:
			m_bsy = !BIT(data, 0);
			m_cd = !BIT(data, 2);
			m_io = !BIT(data, 3);
			break;
	}
	logerror("%s, data %x bsy %d sel %d req %d ack %d cd %d io %d\n", machine().describe_context(), m_data, m_bsy, m_sel, m_req, m_ack, m_cd, m_io);
}

u8 alphatpc16_state::flop_scsi_r(offs_t offset)
{
	u8 ret = 0;
	switch(offset)
	{
		case 0:
			if(m_z80->state_int(STATE_GENPC) == 0xbc)
				m_z80->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
			m_ack = false;
			m_req = false;
			ret = m_data;
			logerror("%s, data %x bsy %d sel %d req %d ack %d cd %d io %d\n", machine().describe_context(), m_data, m_bsy, m_sel, m_req, m_ack, m_cd, m_io);
			break;
		case 1:
			ret = m_bsy | m_sel << 1 | !m_ack << 2;
			m_ack = false;
			break;
	}
	return ret;
}

void alphatpc16_state::apc16_map(address_map &map)
{
	map(0x80020, 0x8002f).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0x82000, 0x82001).rw("i8741", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x84000, 0x84003).rw("z80dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x86000, 0x86001).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x8a000, 0x8a003).rw(FUNC(alphatpc16_state::host_scsi_r), FUNC(alphatpc16_state::host_scsi_w));
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void alphatpc16_state::apc16_io(address_map &map)
{
}

void alphatpc16_state::apc16_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("flop", 0);
	map(0x4000, 0x47ff).ram();
}

void alphatpc16_state::apc16_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(alphatpc16_state::p00_r));
	map(0x20, 0x23).rw(m_wdfdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x40, 0x40).w(FUNC(alphatpc16_state::p40_w));
	map(0x62, 0x63).rw(FUNC(alphatpc16_state::flop_scsi_r), FUNC(alphatpc16_state::flop_scsi_w));
}

// not sure the actual layout of the rows
static INPUT_PORTS_START( alphatpc16 )
	PORT_START("KEYS.0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Unknown 0x7e")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('A')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('B')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('C')
	PORT_START("KEYS.1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('@')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_START("KEYS.2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('{') PORT_CHAR('[')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('|') PORT_CHAR('\\')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('}') PORT_CHAR(']')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("~ ?") PORT_CHAR('~') PORT_CHAR('?')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('`') PORT_CHAR('\'')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PAUSE) PORT_NAME("BREAK")
	PORT_START("KEYS.3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRAPH")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Ü")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("# ^") PORT_CHAR('#') PORT_CHAR('^')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("KEYS.4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Ö")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Unknown 0x7f")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('\r')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("< >") PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_START("KEYS.5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Numpad =") PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CLEAR")
	PORT_START("KEYS.6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Unknown 0x61")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("KEYS.7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Unknown 0x78")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static void atpc16_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD); // sa455
}

void alphatpc16_state::alphatpc16(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 15_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &alphatpc16_state::apc16_map);
	m_maincpu->set_addrmap(AS_IO, &alphatpc16_state::apc16_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic8259, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	z80dart_device &dart(Z80DART(config, "z80dart", 15_MHz_XTAL / 3)); // clock?
	dart.out_int_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));

	Z80(config, m_z80, 8_MHz_XTAL / 2);
	m_z80->set_addrmap(AS_PROGRAM, &alphatpc16_state::apc16_z80_map);
	m_z80->set_addrmap(AS_IO, &alphatpc16_state::apc16_z80_io);
	WD1770(config, m_wdfdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, m_flop[0], atpc16_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	dynamic_cast<device_slot_interface *>(m_flop[0].target())->set_fixed(true);
	FLOPPY_CONNECTOR(config, m_flop[1], atpc16_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_flop[2], atpc16_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_flop[3], atpc16_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	i8741a_device& i8741(I8741A(config, "i8741", 4.608_MHz_XTAL));
	i8741.p1_in_cb().set(FUNC(alphatpc16_state::p1_r));
	i8741.p1_out_cb().set(FUNC(alphatpc16_state::p1_w));
	i8741.p2_in_cb().set(FUNC(alphatpc16_state::p2_r));
	i8741.p2_out_cb().set(FUNC(alphatpc16_state::p2_w));

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 1000).add_route(ALL_OUTPUTS, "mono", 1.00); // Unknown freq

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_ef9345, FUNC(ef9345_device::screen_update));
	screen.set_size(492, 270);
	screen.set_visarea(00, 492-1, 00, 270-1);
	PALETTE(config, "palette").set_entries(8);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_palette_tag("palette");

	TIMER(config, "scanline").configure_scanline(NAME([this](timer_device &t, s32 p){m_ef9345->update_scanline((uint16_t)p);}), screen, 0, 10);

	// these are supported by the bios, they may not have been available on real hardware
	RAM(config, m_ram).set_default_size("64K").set_extra_options("128K,192K,256K,384K,448K,512K");
}

ROM_START( alphatpc16 )
	ROM_REGION(0x10000,"bios", 0)
	ROM_LOAD("cdae03_04_16.bin", 0x0000, 0x8000, CRC(0ff5b549) SHA1(f5612e7864c06da586087645fed97c78e84a5d04))
	ROM_LOAD("cdae02_07_15.bin", 0x8000, 0x8000, CRC(22fd3acb) SHA1(ddab380dd15326ca699d6b4b7f4bf7c1a9d498ea))

	ROM_REGION(0x400, "i8741", 0)
	ROM_LOAD("d8741ad.bin", 0x000, 0x400, CRC(e71d5d9f) SHA1(deda490491d3ee08f47bd23bb29dc92b3806d3f2))

	ROM_REGION(0x4000, "flop", 0)
	ROM_LOAD("cdae04_03_21.bin", 0x0000, 0x4000, CRC(11bc6551) SHA1(28c1f02fdc040035aba249c4ad21de9b5ec95298))

	ROM_REGION( 0x4000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )                // from dcvg5k
ROM_END

} // Anonymous namespace


COMP( 1985, alphatpc16,  0, 0, alphatpc16, alphatpc16, alphatpc16_state, empty_init, "Triumph-Adler", "alphatronic PC-16",  MACHINE_NOT_WORKING)

