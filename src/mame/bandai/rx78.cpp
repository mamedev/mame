// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
/************************************************************************************************************

Gundam RX-78 (c) 1983 Bandai

TODO:
- implement printer;
- Implement 2nd cart slot
- Keyboard works in all scenarios, but it is a guess.
- RAM handling can't be right: PCB has 30KB shared RAM (manual also says this in the technical specs),
  but MAME allocates much more
- Find out what port F3 does - read and write - used by many games
- Find out what ports 23 and EF do - used by rengo

Notes:
- BS-BASIC v1.0 notes:
  -- COLOR x doesn't do anything. It sets a memory location with x, but does nothing with it.
  -- COLOR x,y where y sets the background colour. This part works.
  -- When BASIC is first started, it sets the colours but doesn't save the information. So when
     COLOR x is entered, although x has no effect, it also sets the background colour, which not
     having been set, sets the background black.
  -- At the first scroll, the display memory is disrupted in the logo area, probably another
     btanb. After that, scrolling works correctly.
  -- Need a real machine to confirm these problems, but if true, one can only wonder how such
     obvious issues made it out the door.
- To stop a cmt load, press STOP + SHIFT keys (this combination is the BREAK key).

For a list of all known software and devices for the system, please see hash/rx78.xml.

==============================================================================================================
Summary of Monitor commands.
- The monitor is entered at bootup. The prompt is the * character. This is followed by a command
  letter (upper case). Some commands require hex parameters. You must enter all 4 characters of
  these. No spaces allowed except where shown.
- While in BASIC, you may enter the monitor by using the MON command. After you have finished,
  you can return to BASIC by entering the command *J2005.

- Tape commands:
*L Load a tape
*V Verify a tape
*S Save a block of memory to tape. You are asked for a filename (blank is allowed), the start address,
   the end address, and the Jump address (where it should begin execution)

- Memory commands:
*Dnnnn nnnn Displays a hex dump in the address range entered
*Mnnnn      Allows you to examine and modify memory. Enter to skip to next, period (.) to quit.
*Jnnnn      Transfer execution (Jump) to a program in memory at the specified address

- Other:
*R          This is a block transfer load from a mystery parallel device, using ports E0 and E1,
            using handshaking similar to a centronics printer. The incoming file is loaded into
            memory and it appears that the operator is not provided any information of what happened.

==============================================================================================================

Known issues:
- Sekigahara: Possible joystick problem (need to be checked again)
- Need more software to test with.
BTANB:
- ProWrestling: When player 1 jumps at player 2 and misses, he always lands behind player 2.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class rx78_state : public driver_device
{
public:
	rx78_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void init_rx78();
	void rx78(machine_config &config);

private:
	u8 key_r();
	u8 cass_r();
	u8 vram_r(offs_t offset);
	void cass_w(u8 data);
	void vram_w(offs_t offset, u8 data);
	void vram_read_bank_w(u8 data);
	void vram_write_bank_w(u8 data);
	void key_w(u8 data);
	void vdp_reg_w(offs_t offset, u8 data);
	void vdp_bg_reg_w(u8 data);
	void vdp_pri_mask_w(u8 data);
	void portf3_w(u8 data);
	void create_palette(palette_device &palette);
	INTERRUPT_GEN_MEMBER(interrupt);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart_load );
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	void rx78_io(address_map &map);
	void rx78_mem(address_map &map);

	u8 m_vram_read_bank = 0U;
	u8 m_vram_write_bank = 0U;
	u8 m_pal_reg[7]{};
	u8 m_pri_mask = 0U;
	u8 m_key_mux = 0U;
	u8 m_background = 0U;
	bool m_irq_en = true;
	u8 m_irq_slow = 0U;
	u8 m_irq_count = 0U;
	std::unique_ptr<u8[]> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_ioport_array<16> m_io_keyboard;
};


#define MASTER_CLOCK XTAL(28'636'363)


void rx78_state::cass_w(u8 data)
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}

u8 rx78_state::cass_r()
{
	return (m_cass->input() > 0.03) ? 0 : 1;
}


uint32_t rx78_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layers;
	u8 laycol[2];
	const u8 borderx = 32, bordery = 20;

	bitmap.fill(64, cliprect); // set the border

	u16 count = 0x2c0; //first 0x2bf bytes aren't used for bitmap drawing apparently

	for(u8 y=0; y<184; y++)
	{
		for(u8 x=0; x<192; x+=8)
		{
			for (u8 i = 0; i < 8; i++)
			{
				layers = 0;
				for (u8 j = 0; j < 6; j++)
					if (BIT(m_pri_mask, j))
						layers |= (BIT(m_vram[count + j*0x2000], i))<<j;

				laycol[0] = 0;
				laycol[1] = 0;
				for (u8 j = 0; j < 6; j++)
					if (BIT(layers, j))
						laycol[BIT(m_pal_reg[6], j)] |= m_pal_reg[j];

				// This fixes text in Space Enemy
				if (m_pal_reg[6])
					for (u8 j = 0; j < 6; j++)
						if (BIT(layers, j))
							if (!m_pal_reg[j])
								laycol[0] = 0;

				u8 color = laycol[1] ? laycol[1] : (laycol[0] ? laycol[0] : m_background);
				bitmap.pix(y+bordery, x+i+borderx) = color;
			}
			count++;
		}
	}

	return 0;
}


u8 rx78_state::key_r()
{
	if((m_key_mux >= 1) && (m_key_mux <= 15))
		return m_io_keyboard[m_key_mux]->read();

	u8 res = 0;
	for(u8 i=1; i<10; i++)
		res |= m_io_keyboard[i]->read();

	return res;
}

void rx78_state::key_w(u8 data)
{
	// special codes: 10 (disable irq?) used by gundam and exbaseb; 30 (no idea) used by basic
	m_key_mux = data & 15;
	m_irq_en = !BIT(data, 4);
	m_maincpu->set_input_line(0, CLEAR_LINE);    // fixes exbaseb opening screen
}

// guess
void rx78_state::portf3_w(u8 data)
{
	data &= 3;
	if (data == 3)
		m_irq_slow = 64;    // fixes cracer traffic light
	else
	if (data == 2)
		m_irq_slow = 8;     // fixes seki flash rate of sight
	else
		m_irq_slow = 0;
}

u8 rx78_state::vram_r(offs_t offset)
{
	if(m_vram_read_bank == 0 || m_vram_read_bank > 6)
		return 0xff;

	return m_vram[offset + ((m_vram_read_bank - 1) * 0x2000)];
}

void rx78_state::vram_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 6; i++)
		if (BIT(m_vram_write_bank, i))
			m_vram[offset + i * 0x2000] = data;
}

void rx78_state::vram_read_bank_w(u8 data)
{
	m_vram_read_bank = data;
}

void rx78_state::vram_write_bank_w(u8 data)
{
	m_vram_write_bank = data;
}

void rx78_state::vdp_reg_w(offs_t offset, u8 data)
{
	if (offset < 6)
		m_pal_reg[offset] = bitswap<8>(data, 7, 3, 6, 2, 5, 1, 4, 0) & 0x3f;
	else
		m_pal_reg[offset] = data & 0x3f;
}

void rx78_state::vdp_bg_reg_w(u8 data)
{
	u8 r = (data & 0x11) == 0x11 ? 0xff : ((data & 0x11) == 0x01 ? 0x7f : 0);
	u8 g = (data & 0x22) == 0x22 ? 0xff : ((data & 0x22) == 0x02 ? 0x7f : 0);
	u8 b = (data & 0x44) == 0x44 ? 0xff : ((data & 0x44) == 0x04 ? 0x7f : 0);
	m_palette->set_pen_color(64, rgb_t(r,g,b));   // use this as the border colour
	m_background = bitswap<8>(data, 7, 3, 6, 2, 5, 1, 4, 0) & 0x3f;
}

void rx78_state::vdp_pri_mask_w(u8 data)
{
	m_pri_mask = data;
}

void rx78_state::create_palette(palette_device &palette)
{
	constexpr u8 level[] = { 0, 0x7f, 0, 0xff };
	for (u8 i = 0; i < 64; i++)
	{
		u8 r = level[BIT(i, 0, 2)];
		u8 g = level[BIT(i, 2, 2)];
		u8 b = level[BIT(i, 4, 2)];
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
	vdp_bg_reg_w(0);
}


void rx78_state::rx78_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("roms", 0);
	//map(0x2000, 0x5fff)      // mapped by the cartslot
	map(0x6000, 0xafff).ram(); //ext RAM
	map(0xb000, 0xebff).ram();
	map(0xec00, 0xffff).rw(FUNC(rx78_state::vram_r), FUNC(rx78_state::vram_w));
}

void rx78_state::rx78_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  map(0x23, 0x23).nopw(); //used by rengo
//  map(0xe2, 0xe2).noprw(); //printer
//  map(0xe3, 0xe3).nopw(); //printer
//  map(0xef, 0xef).nopw(); //used by rengo
	map(0xf0, 0xf0).rw(FUNC(rx78_state::cass_r), FUNC(rx78_state::cass_w)); //cmt
	map(0xf1, 0xf1).w(FUNC(rx78_state::vram_read_bank_w));
	map(0xf2, 0xf2).w(FUNC(rx78_state::vram_write_bank_w));
	map(0xf3, 0xf3).w(FUNC(rx78_state::portf3_w));
	map(0xf4, 0xf4).rw(FUNC(rx78_state::key_r), FUNC(rx78_state::key_w)); //keyboard
	map(0xf5, 0xfb).w(FUNC(rx78_state::vdp_reg_w)); //vdp
	map(0xfc, 0xfc).w(FUNC(rx78_state::vdp_bg_reg_w)); //vdp
	map(0xfe, 0xfe).w(FUNC(rx78_state::vdp_pri_mask_w));
	map(0xff, 0xff).w("sn1", FUNC(sn76489a_device::write)); //psg
}

/* Input ports */
static INPUT_PORTS_START( rx78 )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("X4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("X6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up Down Arrow") PORT_CODE(KEYCODE_PGUP) PORT_CHAR('^')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right Left Arrow") PORT_CODE(KEYCODE_PGDN)

	PORT_START("X7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLR / HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INST / DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("X8")
	PORT_BIT(0x07,IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_END) PORT_CHAR(0xff) PORT_CHAR(3)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("X9")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) //kana shift?
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0xf8,IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Up Left") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Down Left")  PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Up Right") PORT_PLAYER(1)

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Down Right") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("X13")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Up Left") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("X14")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Down Left")  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Up Right") PORT_PLAYER(2)

	PORT_START("X15")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Down Right") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


void rx78_state::machine_reset()
{
	address_space &prg = m_maincpu->space(AS_PROGRAM);
	if (m_cart->exists())
	{
		u32 size = m_cart->common_get_size("rom");
		if (size > 0x9000)
			size = 0x9000;
		if (size)
			prg.install_read_handler(0x2000, size+0x1FFF, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
	}
}

void rx78_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0xc000);
	save_pointer(NAME(m_vram), 0xc000);
	save_item(NAME(m_vram_read_bank));
	save_item(NAME(m_vram_write_bank));
	save_pointer(NAME(m_pal_reg), 7);
	save_item(NAME(m_pri_mask));
	save_item(NAME(m_key_mux));
	save_item(NAME(m_background));
}

INTERRUPT_GEN_MEMBER(rx78_state::interrupt)
{
	if (m_irq_en)
	{
		m_irq_count++;
		if (m_irq_count > m_irq_slow)
		{
			m_irq_count = 0;
			irq0_line_hold(device);
		}
	}
	else
	// wait for a keypress
	if (key_r())
	{
		irq0_line_hold(device);
	}
}

DEVICE_IMAGE_LOAD_MEMBER( rx78_state::cart_load )
{
	u32 size = m_cart->common_get_size("rom");

	if (size != 0x2000 && size != 0x4000 && size != 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 8K, 16K or 32K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

/* F4 Character Displayer */
static const gfx_layout rx78_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	187,                    /* 187 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_rx78 )
	GFXDECODE_ENTRY( "roms", 0x1a27, rx78_charlayout, 0, 8 )
GFXDECODE_END

void rx78_state::rx78(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/7); // unknown divider
	m_maincpu->set_addrmap(AS_PROGRAM, &rx78_state::rx78_mem);
	m_maincpu->set_addrmap(AS_IO, &rx78_state::rx78_io);
	m_maincpu->set_vblank_int("screen", FUNC(rx78_state::interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
//  screen.set_size(192, 184);
//  screen.set_visarea(0, 192-1, 0, 184-1);
	/* guess: generic NTSC video timing at 256x224, system runs at 192x184, suppose with some border area to compensate */
	screen.set_raw(MASTER_CLOCK/4, 442, 0, 256, 263, 0, 224);
	screen.set_screen_update(FUNC(rx78_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette, FUNC(rx78_state::create_palette), 64+1);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_rx78);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "rx78_cart", "bin,rom").set_device_load(FUNC(rx78_state::cart_load));

	RAM(config, RAM_TAG).set_default_size("32K").set_extra_options("16K");

	SPEAKER(config, "mono").front_center();
	SN76489A(config, "sn1", XTAL(28'636'363)/8).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown divider

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("rx78_cass");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("rx78_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("rx78_cass");
}

/* ROM definition */
ROM_START( rx78 )
	ROM_REGION( 0x2000, "roms", 0 )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(a194ea53) SHA1(ba39e73e6eb7cbb8906fff1f81a98964cd62af0d))
ROM_END

void rx78_state::init_rx78()
{
	u32 ram_size = m_ram->size();
	address_space &prg = m_maincpu->space(AS_PROGRAM);

	if (ram_size == 0x4000)
		prg.unmap_readwrite(0x6000, 0xafff);
}

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY   FULLNAME     FLAGS */
COMP( 1983, rx78, 0,      0,      rx78,    rx78,  rx78_state, init_rx78, "Bandai", "Gundam RX-78", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
