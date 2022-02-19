// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************

Argo

2011-03-16 Skeleton driver.

There are no manuals, diagrams, or anything else available afaik.
The entire driver is guesswork, however it appears to be related to UNIOR.

The monitor will only allow certain characters to be typed, thus the
modifier keys appear to do nothing. There is no need to use the enter
key; using spacebar and the correct parameters is enough.

Commands: same as UNIOR

ToDo:
- Add remaining devices, most likely 8255.
- There is no obvious evidence of sound.
- Need dump of the correct chargen.
- Find out if "m_eram" is supposed to exist, if so, what is it for?
- What character should show in the top left corner?
- our dma doesn't emulate the update_flag, so had to use a hack for the L command
- cursor is one character further to the right than it should be, used another hack

****************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
//#include "sound/spkrdev.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class argo_state : public driver_device
{
public:
	argo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_p_chargen(*this, "chargen")
		, m_uart(*this, "uart")
		, m_pit(*this, "pit")
		, m_dma(*this, "dma")
		, m_crtc(*this, "crtc")
		, m_cass(*this, "cassette")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "X%d", 0U)
	{ }

	void argo(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	void argo_videoram_w(offs_t offset, u8 data);
	u8 argo_io_r(offs_t offset);
	void argo_io_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(z0_w);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	void argo_palette(palette_device &palette) const;
	u8 dma_r(offs_t offset);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_device<i8251_device> m_uart;
	required_device<pit8253_device> m_pit;
	required_device<i8257_device> m_dma;
	required_device<i8275_device> m_crtc;
	required_device<cassette_image_device> m_cass;
	required_device<palette_device> m_palette;
	required_ioport_array<11> m_io_keyboard;
	u8 m_framecnt = 0U;
	bool m_ram_ctrl = 0;
	u8 m_scroll_ctrl = 0U;
	bool m_txe = 0, m_txd = 0, m_rts = 0, m_casspol = 0;
	u8 m_cass_data[4]{};
	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_eram;
};

// write to videoram if following 'out b9,61' otherwise write to the unknown 'extra' ram
void argo_state::argo_videoram_w(offs_t offset, u8 data)
{
	if (m_ram_ctrl)
		m_vram[offset] = data;
	else
		m_eram[offset] = data;
}

u8 argo_state::argo_io_r(offs_t offset)
{
	u8 low_io = offset;

	switch (low_io)
	{
	case 0xA1: // keyboard
		offset >>= 8;
		if (offset < 11)
			return m_io_keyboard[offset]->read();
		else
			return 0xff;

	case 0xA0:
	case 0xA2:
	case 0xA4:
	case 0xA6:
		offset >>= 1;
		return m_pit->read(offset&3);

	case 0xC0:
	case 0xC4:
		offset >>= 2;
		return m_crtc->read(offset&1);

	case 0xE8: // wants bit 4 low then high
		{
			u8 data = m_dma->read(8);
			data |= (m_framecnt << 4);  // hack because dma update_flag is not emulated
			return data;
		}

	default:
		logerror("%s: In %X\n",machine().describe_context(),low_io);
		return 0xff;
	}
}

void argo_state::argo_io_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0xA0:
	case 0xA2:
	case 0xA4:
	case 0xA6:
		offset >>= 1;
		m_pit->write(offset&3, data);
		break;

	case 0xA1: // prep scroll step 1
		m_scroll_ctrl = (data == 0x61);
		break;

	case 0xA9: // prep scroll step 2
		if ((m_scroll_ctrl == 1) && (data == 0x61))
			m_scroll_ctrl++;
		break;

	case 0xB9: // switch between videoram and extraram
		m_ram_ctrl = (data == 0x61);
		break;

	case 0xC0:
	case 0xC4:
		offset >>= 2;
		m_crtc->write(offset&1, data);
		break;

	case 0xE8: // hardware scroll
		if ((m_scroll_ctrl == 2) & (data == 0xe3))
		{
			memmove(m_vram.get(), m_vram.get()+80, 24*80);
			m_scroll_ctrl = 0;
		}
		m_dma->write(8, data);
		break;

	default:
		logerror("%s: Out %X,%X\n",machine().describe_context(),offset,data);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( argo_state::kansas_r )
{
	if (m_rts)
	{
		m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
		m_casspol = 1;
		return;
	}

	m_cass_data[1]++;
	m_cass_data[2]++;

	u8 cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		if (m_cass_data[1] > 13)
			m_casspol ^= 1;
		m_cass_data[1] = 0;
		m_cass_data[2] = 0;
		m_uart->write_rxd(m_casspol);
	}
	if ((m_cass_data[2] & 7)==2)
	{
		m_cass_data[3]++;
		m_uart->write_rxc(BIT(m_cass_data[3], 0));
	}
}

WRITE_LINE_MEMBER(argo_state::z0_w)
{
	// write - incoming 2400Hz
	m_uart->write_txc(state);
	if (!m_txe)
	{
		m_cass->output((m_txd ^ state) ? -1.0 : 1.0);
	}

	// read - incoming 2514Hz
}

u8 argo_state::dma_r(offs_t offset)
{
	if (offset < 0xf800)
		return m_maincpu->space(AS_PROGRAM).read_byte(offset);
	else
		return m_vram[offset & 0x7ff];
}

WRITE_LINE_MEMBER( argo_state::hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dma->hlda_w(state);
}

I8275_DRAW_CHARACTER_MEMBER(argo_state::display_pixels)
{
	if ((x == 0) && (y == 0))
		m_framecnt++;

	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 gfx = m_p_chargen[(linecount & 15) | (charcode << 4)];

	if (vsp)
		gfx = 0;

	if (lten)
	{
		gfx = 0xff;
		if (x > 6)
			x-=6; // hack to fix cursor position
	}

	if (rvv)
		gfx ^= 0xff;

	for(u8 i=0;i<7;i++)
		bitmap.pix(y, x + i) = palette[BIT(gfx, 6-i) ? (hlgt ? 2 : 1) : 0];
}

static constexpr rgb_t argo_pens[3] =
{
	{ 0x00, 0x00, 0x00 }, // black
	{ 0xa0, 0xa0, 0xa0 }, // white
	{ 0xff, 0xff, 0xff }  // highlight
};

void argo_state::argo_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, argo_pens);
}


/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 9,                   /* 8 x 9 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_argo )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void argo_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xf7ff).ram().share("mainram");
	map(0xf800, 0xffff).rom().region("maincpu", 0).w(FUNC(argo_state::argo_videoram_w));
}

void argo_state::io_map(address_map &map)
{
	map(0x0000, 0xFFFF).r(FUNC(argo_state::argo_io_r));
	map(0x0000, 0x00ff).mirror(0xff00).w(FUNC(argo_state::argo_io_w));
	map(0x00c1, 0x00c1).mirror(0xff00).rw(m_uart, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x00c3, 0x00c3).mirror(0xff00).rw(m_uart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x00e0, 0x00e7).mirror(0xff00).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
}

/* Input ports */
static INPUT_PORTS_START( argo ) // Keyboard was worked out by trial & error;'F' keys produce symbols
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) // J
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LShift")      PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR('7')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR('9')
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")         PORT_CODE(KEYCODE_ESC)        PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RShift")      PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")          PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR('0')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+")           PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)          PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")          PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR('1')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR('2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR('3')
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) // A
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)          PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)          PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR('4')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR('5')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR('6')
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) // B
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3)          PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)          PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) // K
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) // H
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) // I
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) // C
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4)          PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS")          PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) // G
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) // F
	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5)          PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")       PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*")           PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) // D
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6)          PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")          PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\')
	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) // E
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7)          PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("`")           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')
	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8)          PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\'")          PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')
	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)          PORT_CHAR('L') // if L is the first character, computer hangs
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
INPUT_PORTS_END



void argo_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
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

void argo_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x0800);
	m_eram = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_vram), 0x0800);
	save_pointer(NAME(m_eram), 0x0800);
	save_item(NAME(m_framecnt));
	save_item(NAME(m_ram_ctrl));
	save_item(NAME(m_scroll_ctrl));
	save_item(NAME(m_txe));
	save_item(NAME(m_txd));
	save_item(NAME(m_rts));
	save_item(NAME(m_casspol));
	save_item(NAME(m_cass_data));
}


void argo_state::argo(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3500000); // unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &argo_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &argo_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_argo);
	PALETTE(config, m_palette, FUNC(argo_state::argo_palette), 3);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	//SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.15);
	TIMER(config, "kansas_r").configure_periodic(FUNC(argo_state::kansas_r), attotime::from_hz(38400));

	/* Devices */
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_txd = state; });
	m_uart->txempty_handler().set([this] (bool state) { m_txe = state; });
	m_uart->rts_handler().set([this] (bool state) { m_rts = state; });

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(1689600); // this gives the 2400Hz required by the cassette
	m_pit->out_handler<0>().set(FUNC(argo_state::z0_w));
	m_pit->set_clk<1>(0);
	m_pit->set_clk<2>(0);

	I8257(config, m_dma, XTAL(20'000'000) / 9);  // unknown frequency
	m_dma->out_hrq_cb().set(FUNC(argo_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(argo_state::dma_r));
	m_dma->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));

	i8275_device &crtc(I8275(config, "crtc", XTAL(20'000'000) / 12));  // unknown frequency
	crtc.set_character_width(6);
	crtc.set_display_callback(FUNC(argo_state::display_pixels));
	crtc.drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	crtc.set_screen("screen");
}

/* ROM definition */
ROM_START( argo )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "argo.rom", 0x0000, 0x0800, CRC(4c4c045b) SHA1(be2b97728cc190d4a8bd27262ba9423f252d31a3) )

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT         COMPANY      FULLNAME  FLAGS */
COMP( 1986, argo, unior,  0,      argo,    argo,  argo_state, empty_init, "<unknown>", "Argo",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
