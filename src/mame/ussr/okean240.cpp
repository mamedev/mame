// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************************

Okeah-240 (Ocean-240).

2011-12-28 Skeleton driver.

Info from EMU80:

intctl : K580wn59
  irq[0]=kbd.irq2
  irq[1]=kbd.irq
  irq[4]=tim.out[0]

ppa40 : K580ww55
  portA=kbd.pressed.mask
  portB[2]=cas.playback
  portB[5]=kbd.key[58]
  portB[6]=kbd.key[59]
  portC[0-3]=kbd.pressed.row
  portC[4]=kbd.ack

ppaC0 : K580ww55
  portA=vid.scroll.y
  portB[0-3]=mem.frame[0].page
  portB[1-3]=mem.frame[1].page
  portB[4-5]=mm.page
  portC=vid.scroll.x

If you leave out the CPM80 roms, the machine will start up in a machine-language monitor.

Okean240: MONITOR 240/7  known commands:
A ?
B ?
D dump
F fill
G go
L load (via uart)
M move
R ?
S substitute
W ?
X registers

Okean240a: Turbo MONITOR by Alex Z. There are several emulation bugs noticed. Known commands:
H hex arithmetic
J Jamp (set address?)
M move
P ? (locks up)
R read
W write
tab switches between the hex and ascii sides
^C refresh display

NOTE ABOUT THE TEST ROM (okean240t):
- You need to press a key every so often, or hold down Insert.


ToDo:
- Find out if any unconnected keyboard entries are real keys
- Arrow keys (used in Turbo Monitor)
- Colours?
- Sound? (perhaps port E4 bit 3)
- Floppy disks and devices
- Cassette?
- Add memory banking (perhaps port C1)
- 80 column mode (used in Turbo Monitor)

Keyboard:
- okean240  - external ascii keyboard
- okean240a - internal keyboard
- okean240t - serial keyboard & screen

**********************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/timer.h"

#include "screen.h"
#include "emupal.h"


namespace {

class okean240_state : public driver_device
{
public:
	okean240_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_io_modifiers(*this, "MODIFIERS")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_ppikbd(*this, "ppikbd")
		, m_pic(*this, "pic")
	{ }

	void okean240a(machine_config &config);
	void okean240t(machine_config &config);
	void okean240(machine_config &config);

private:
	u8 okean240_port40_r();
	u8 okean240_port41_r();
	void okean240_port42_w(u8 data);
	u8 okean240a_port40_r();
	u8 okean240a_port41_r();
	u8 okean240a_port42_r();
	void okean240_porte2_w(u8 data);
	void kbd_put(u8 data);
	u32 screen_update_okean240(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void okean240_io(address_map &map) ATTR_COLD;
	void okean240_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);
	u8 m_term_data = 0U;
	u8 m_j = 0U;
	u8 m_scroll = 0U;
	u8 m_tog = 0U;
	bool m_key_pressed = false;
	u8 m_kbd_row = 0U;
	required_shared_ptr<u8> m_p_videoram;
	optional_ioport_array<11> m_io_keyboard;
	optional_ioport m_io_modifiers;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<i8080_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<i8255_device> m_ppikbd;
	required_device<pic8259_device> m_pic;
};


// okean240/t: process ascii keyboard
void okean240_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_pic->ir1_w(1);
}

// okean240/t: port 40 (get ascii key)
u8 okean240_state::okean240_port40_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	m_pic->ir1_w(0);
	return ret;
}

// okean240t: port 41 bit 1 (test rom status bit) @E0DB, E0E2
u8 okean240_state::okean240_port41_r()
{
	m_tog ^= 2;
	return m_tog;
}

// okean240a keyboard routines
TIMER_DEVICE_CALLBACK_MEMBER( okean240_state::timer_k )
{
	if (m_key_pressed)
		return;

	for (u8 i = 0; i < 11; i++)
	{
		if (m_io_keyboard[i]->read())
		{
			m_key_pressed = true;
			m_kbd_row = i;
			m_pic->ir1_w(1);
			return;
		}
	}
}


// port 40 (get a single key press)
u8 okean240_state::okean240a_port40_r()
{
	u8 j = m_io_keyboard[m_kbd_row]->read();
	if (j)
	{
		if (j==m_j) return 0;
		m_j=j;
		return j;
	}
	m_j=0;
	return 0;
}

// port 41 bits 6&7 (modifier keys)
u8 okean240_state::okean240a_port41_r()
{
	return m_io_modifiers->read();
}

// port 42 (get a row)
u8 okean240_state::okean240a_port42_r()
{
	return m_kbd_row;
}

// This is a keyboard acknowledge pulse, it goes high then
// straightaway low, if reading port 40 indicates a key is pressed.
// okean240: data bit 7
// okean240a: data bit 4
void okean240_state::okean240_port42_w(u8 data)
{
	m_pic->ir1_w(0);
	m_key_pressed = false;
}

void okean240_state::okean240_porte2_w(u8 data)
{
	m_pic->ir4_w(!BIT(data, 3));
}


void okean240_state::okean240_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram().share("mainram");
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}

void okean240_state::okean240_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x25).nopw();
	map(0x40, 0x43).rw(m_ppikbd, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60, 0x63).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x80, 0x81).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xa0, 0xa1).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xc0, 0xc3).rw("ppic", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe0, 0xe3).rw("ppie", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// map(0x00, 0x1f)=ppa00.data
	// map(0x20, 0x23)=dsk.data
	// map(0x24, 0x24)=dsk.wait
	// map(0x25, 0x25)=dskctl.data
	// map(0x40, 0x5f)=ppa40.data
	// map(0x60, 0x7f)=tim.data
	// map(0x80, 0x81)=intctl.data
	// map(0xa0, 0xa1)=comport.data
	// map(0xc0, 0xdf)=ppaC0.data
	// map(0xe0, 0xff)=ppaE0.data
}


/* Input ports */
static INPUT_PORTS_START( okean240 )
INPUT_PORTS_END


static INPUT_PORTS_START( okean240a )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // comma
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) // minus
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("numdel") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("numenter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)        PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)        PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)        PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num3") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //9E
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)        PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)        PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num6") PORT_CODE(KEYCODE_6_PAD)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //81
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)        PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)        PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)        PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) // plus
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //7F
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) //03

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //86
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)        PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)        PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)        PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //99
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) //8B

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)        PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)        PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)        PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) //84
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //92 prints # and line feed
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)        PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)        PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)        PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) //98
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //85 cr and line feed
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_END)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //83 cancel input
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)        PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)        PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)        PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)        PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)        PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) //93
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)        PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)        PORT_CHAR('0')

	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)        PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)        PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)        PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9)      PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("MODIFIERS")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


void okean240_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_j));
	save_item(NAME(m_scroll));
	save_item(NAME(m_tog));
	save_item(NAME(m_key_pressed));
	save_item(NAME(m_kbd_row));
}


void okean240_state::machine_reset()
{
	m_term_data = 0;
	m_j = 0;
	m_scroll = 0;
	m_key_pressed = false;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom+0x2000);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xe000, 0xe7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

u32 okean240_state::screen_update_okean240(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 y = 0; y < 256; y++)
	{
		u8 const ma = y + m_scroll; // ma must be 8-bit
		u16 *p = &bitmap.pix(y);

		for (u16 x = 0; x < 0x4000; x+=0x200)
		{
			u8 const gfx = m_p_videoram[x|ma] | m_p_videoram[x|ma|0x100];

			/* Display a scanline of a character */
			*p++ = BIT(gfx, 0);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 7);
		}
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout okean240_charlayout =
{
	8, 7,                   /* 8 x 7 characters */
	160,                    /* 160 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8 },
	8*7                 /* every char takes 7 bytes */
};

static GFXDECODE_START( gfx_okean240 )
	GFXDECODE_ENTRY( "maincpu", 0x2c08, okean240_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_okean240a )
	GFXDECODE_ENTRY( "maincpu", 0x2f63, okean240_charlayout, 0, 1 )
GFXDECODE_END


void okean240_state::okean240t(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(12'000'000) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &okean240_state::okean240_mem);
	m_maincpu->set_addrmap(AS_IO, &okean240_state::okean240_io);
	m_maincpu->in_inta_func().set("pic", FUNC(pic8259_device::acknowledge));

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));

	I8255(config, m_ppikbd);
	m_ppikbd->in_pa_callback().set(FUNC(okean240_state::okean240_port40_r));
	m_ppikbd->in_pb_callback().set(FUNC(okean240_state::okean240_port41_r));
	m_ppikbd->out_pc_callback().set(FUNC(okean240_state::okean240_port42_w));

	i8255_device &ppic(I8255(config, "ppic"));
	ppic.out_pa_callback().set([this] (u8 data) { m_scroll = data; });

	i8255_device &ppie(I8255(config, "ppie"));
	ppie.out_pc_callback().set(FUNC(okean240_state::okean240_porte2_w));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<1>(3072000); // artificial rate
	pit.out_handler<1>().set("uart", FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append("uart", FUNC(i8251_device::write_rxc));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	/* video hardware */
	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_RASTER));
	screen1.set_refresh_hz(50);
	screen1.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen1.set_size(256, 256);
	screen1.set_visarea(0, 255, 0, 255);
	screen1.set_screen_update(FUNC(okean240_state::screen_update_okean240));
	screen1.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

void okean240_state::okean240a(machine_config &config)
{
	okean240t(config);
	GFXDECODE(config, "gfxdecode", "palette", gfx_okean240a);
	subdevice<rs232_port_device>("rs232")->set_default_option(nullptr); // not used for keyboard

	m_ppikbd->in_pa_callback().set(FUNC(okean240_state::okean240a_port40_r));
	m_ppikbd->in_pb_callback().set(FUNC(okean240_state::okean240a_port41_r));
	m_ppikbd->in_pc_callback().set(FUNC(okean240_state::okean240a_port42_r));

	subdevice<pit8253_device>("pit")->set_clk<1>(1536000); // artificial rate
	TIMER(config, "timer_k").configure_periodic(FUNC(okean240_state::timer_k), attotime::from_hz(300)); // keyb scan
}

void okean240_state::okean240(machine_config &config)
{
	okean240t(config);
	GFXDECODE(config, "gfxdecode", "palette", gfx_okean240);
	subdevice<rs232_port_device>("rs232")->set_default_option(nullptr); // not used for keyboard
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(okean240_state::kbd_put));
}

/* ROM definition */
ROM_START( okean240 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "monitor.bin", 0x2000, 0x2000, CRC(587799bc) SHA1(1f677afa96722ca4ed2643eaca243548845fc854) )
	ROM_LOAD( "cpm80.bin",   0x0000, 0x2000, CRC(7378e4f9) SHA1(c3c06c6f2e953546452ca6f82140a79d0e4953b4) )
ROM_END

ROM_START( okean240a )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "fddmonitor.bin", 0x2000, 0x2000, CRC(bcac5ca0) SHA1(602ab824704d3d5d07b3787f6227ff903c33c9d5) )
	ROM_LOAD( "fddcpm80.bin",   0x0000, 0x2000, CRC(b89a7e16) SHA1(b8f937c04f430be18e48f296ed3ef37194171204) )
ROM_END

ROM_START( okean240t )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "test.bin",    0x2000, 0x0800, CRC(e9e2b7b9) SHA1(e4e0b6984a2514b6ba3e97500d487ea1a68b7577) )
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS           INIT           COMPANY      FULLNAME              FLAGS
COMP( 1986, okean240,  0,        0,      okean240,  okean240,  okean240_state, empty_init, "<unknown>", "Okeah-240",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1986, okean240a, okean240, 0,      okean240a, okean240a, okean240_state, empty_init, "<unknown>", "Ocean-240 with FDD", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1986, okean240t, okean240, 0,      okean240t, okean240,  okean240_state, empty_init, "<unknown>", "Ocean-240 Test ROM", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
