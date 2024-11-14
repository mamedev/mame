// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MC 1000

    12/05/2009 Skeleton driver.

    http://ensjo.wikispaces.com/MC-1000+on+JEMU
    http://ensjo.blogspot.com/2006/11/color-artifacting-no-mc-1000.html

****************************************************************************/

/*

    TODO:

    - xtal frequency?
    - Z80 wait at 0x0000-0x1fff when !hsync & !vsync
    - 80-column card (MC6845) character generator ROM
    - Charlemagne / GEM-1000 / Junior Computer ROMs

*/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "video/mc6847.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define SCREEN_TAG      "screen"
#define Z80_TAG         "u13"
#define AY8910_TAG      "u21"
#define MC6845_TAG      "mc6845"
#define MC6847_TAG      "u19"
#define CENTRONICS_TAG  "centronics"

#define MC1000_MC6845_VIDEORAM_SIZE     0x800
#define MC1000_MC6847_VIDEORAM_SIZE     0x1800

class mc1000_state : public driver_device
{
public:
	mc1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_vdg(*this, MC6847_TAG),
		m_crtc(*this, MC6845_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_mc6845_video_ram(*this, "mc6845_vram"),
		m_mc6847_video_ram(*this, "mc6847_vram"),
		m_y(*this, "Y%u", 0),
		m_joy(*this, "JOY%u", 0),
		m_modifiers(*this, "MODIFIERS"),
		m_joykeymap(*this, "JOYKEYMAP%u", 0),
		m_banks(*this, "bank%u", 1U)
	{ }

	void mc1000(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	optional_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_shared_ptr<uint8_t> m_mc6845_video_ram;
	required_shared_ptr<uint8_t> m_mc6847_video_ram;
	required_ioport_array<8> m_y;
	required_ioport_array<2> m_joy;
	required_ioport m_modifiers;
	required_ioport_array<2> m_joykeymap;
	required_memory_bank_array<5> m_banks;

	std::unique_ptr<uint8_t[]> m_banked_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t printer_r();
	void printer_w(uint8_t data);
	void mc6845_ctrl_w(uint8_t data);
	void mc6847_attr_w(uint8_t data);
	void fs_w(int state);
	void hs_w(int state);
	uint8_t videoram_r(offs_t offset);
	void keylatch_w(uint8_t data);
	uint8_t keydata_r();
	uint8_t rom_banking_r(offs_t offset);

	void bankswitch();

	/* cpu state */
	int m_ne555_int;

	/* memory state */
	int m_rom0000;
	uint8_t m_mc6845_bank;
	uint8_t m_mc6847_bank;

	/* keyboard state */
	int m_keylatch;

	/* video state */
	int m_hsync;
	int m_vsync;
	uint8_t m_mc6847_attr;

	void write_centronics_busy(int state);
	int m_centronics_busy;

	void init_mc1000();
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tick);
	void mc1000_banking_mem(address_map &map) ATTR_COLD;
	void mc1000_io(address_map &map) ATTR_COLD;
	void mc1000_mem(address_map &map) ATTR_COLD;
};

/* Memory Banking */

void mc1000_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* MC6845 video RAM */
	m_banks[1]->set_entry(m_mc6845_bank);

	/* extended RAM */
	if (m_ram->size() > 16*1024)
	{
		program.install_readwrite_bank(0x4000, 0x7fff, m_banks[2]);
	}
	else
	{
		program.unmap_readwrite(0x4000, 0x7fff);
	}

	/* MC6847 video RAM */
	if (m_mc6847_bank)
	{
		if (m_ram->size() > 16*1024)
		{
			program.install_readwrite_bank(0x8000, 0x97ff, m_banks[3]);
		}
		else
		{
			program.unmap_readwrite(0x8000, 0x97ff);
		}
	}
	else
	{
		program.install_readwrite_bank(0x8000, 0x97ff, m_banks[3]);
	}

	m_banks[3]->set_entry(m_mc6847_bank);

	/* extended RAM */
	if (m_ram->size() > 16*1024)
	{
		program.install_readwrite_bank(0x9800, 0xbfff, m_banks[4]);
	}
	else
	{
		program.unmap_readwrite(0x9800, 0xbfff);
	}
}

/* Read/Write Handlers */

void mc1000_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

uint8_t mc1000_state::printer_r()
{
	return m_centronics_busy;
}

void mc1000_state::printer_w(uint8_t data)
{
	m_centronics->write_strobe(BIT(data, 0));
}

void mc1000_state::mc6845_ctrl_w(uint8_t data)
{
	m_mc6845_bank = BIT(data, 0);

	bankswitch();
}

void mc1000_state::mc6847_attr_w(uint8_t data)
{
	/*

	    bit     description

	    0       enable CPU video RAM access
	    1       CSS
	    2       GM0
	    3       GM1
	    4       GM2
	    5       _INT/EXT
	    6       _A/S
	    7       _A/G

	*/

	m_mc6847_bank = BIT(data, 0);
	m_vdg->css_w(BIT(data, 1));
	m_vdg->gm0_w(BIT(data, 2));
	m_vdg->gm1_w(BIT(data, 3));
	m_vdg->gm2_w(BIT(data, 4));
	m_vdg->intext_w(BIT(data, 5));
	m_vdg->as_w(BIT(data, 6));
	m_vdg->ag_w(BIT(data, 7));

	bankswitch();
}

/* Memory Maps */

void mc1000_state::mc1000_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1");
	map(0x2000, 0x27ff).bankrw("bank2").share("mc6845_vram");
	map(0x2800, 0x3fff).ram().share("ram2800");
	map(0x4000, 0x7fff).bankrw("bank3");
	map(0x8000, 0x97ff).bankrw("bank4").share("mc6847_vram");
	map(0x9800, 0xbfff).bankrw("bank5");
	map(0xc000, 0xffff).rom().region(Z80_TAG, 0);
}

void mc1000_state::mc1000_banking_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1");
	map(0x2000, 0x27ff).bankrw("bank2").share("mc6845_vram");
	map(0x2800, 0x3fff).ram().share("ram2800");
	map(0x4000, 0x7fff).bankrw("bank3");
	map(0x8000, 0x97ff).bankrw("bank4").share("mc6847_vram");
	map(0x9800, 0xbfff).bankrw("bank5");
	map(0xc000, 0xffff).r(FUNC(mc1000_state::rom_banking_r));
}

void mc1000_state::mc1000_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x04, 0x04).rw(FUNC(mc1000_state::printer_r), FUNC(mc1000_state::printer_w));
	map(0x05, 0x05).w("cent_data_out", FUNC(output_latch_device::write));
//  map(0x10, 0x10).w(m_crtc, FUNC(mc6845_device::address_w));
//  map(0x11, 0x11).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x12, 0x12).w(FUNC(mc1000_state::mc6845_ctrl_w));
	map(0x20, 0x20).w(AY8910_TAG, FUNC(ay8910_device::address_w));
	map(0x40, 0x40).r(AY8910_TAG, FUNC(ay8910_device::data_r));
	map(0x60, 0x60).w(AY8910_TAG, FUNC(ay8910_device::data_w));
	map(0x80, 0x80).w(FUNC(mc1000_state::mc6847_attr_w));
}

/* Input Ports */

static INPUT_PORTS_START( mc1000 )
	PORT_START("JOY0") /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    /* = 'I' */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  /* = 'Q' */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  /* = 'Y' */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) /* = '1' */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )        /* = '9' */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY1") /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)        /* = '@' */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)    /* = 'H' */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)  /* = 'P' */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)  /* = 'X' */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) /* = '0' */
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUBOUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MODIFIERS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("JOYKEYMAP0")
	PORT_CONFNAME( 0x01, 0x00, "JOYSTICK A (P1) keyboard mapping" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("JOYKEYMAP1")
	PORT_CONFNAME( 0x01, 0x00, "JOYSTICK B (P2) keyboard mapping" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

/* Video */

void mc1000_state::fs_w(int state)
{
	m_vsync = state;
}

void mc1000_state::hs_w(int state)
{
	m_hsync = state;
}

uint8_t mc1000_state::videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	m_vdg->inv_w(BIT(m_mc6847_video_ram[offset], 7));

	return m_mc6847_video_ram[offset];
}

/* AY-3-8910 Interface */

void mc1000_state::keylatch_w(uint8_t data)
{
	m_keylatch = data;

	m_cassette->output(BIT(data, 7) ? -1.0 : +1.0);
}

uint8_t mc1000_state::keydata_r()
{
	uint8_t data = 0xff;

	if (!BIT(m_keylatch, 0))
	{
		data &= m_y[0]->read();
		if (m_joykeymap[1]->read()) data &= m_joy[1]->read();
	}
	if (!BIT(m_keylatch, 1))
	{
		data &= m_y[1]->read();
		if (m_joykeymap[0]->read()) data &= m_joy[0]->read();
	}
	if (!BIT(m_keylatch, 2)) data &= m_y[2]->read();
	if (!BIT(m_keylatch, 3)) data &= m_y[3]->read();
	if (!BIT(m_keylatch, 4)) data &= m_y[4]->read();
	if (!BIT(m_keylatch, 5)) data &= m_y[5]->read();
	if (!BIT(m_keylatch, 6)) data &= m_y[6]->read();
	if (!BIT(m_keylatch, 7)) data &= m_y[7]->read();

	data = (m_modifiers->read() & 0xc0) | (data & 0x3f);

	if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
	{
		if (m_cassette->input() >= +0.0) data &= 0x7f;
	}

	return data;
}


uint8_t mc1000_state::rom_banking_r(offs_t offset)
{
	m_banks[0]->set_entry(0);
	m_rom0000 = 0;
	return m_rom->base()[offset];
}

/* Machine Initialization */

void mc1000_state::machine_start()
{
	/* setup memory banking */
	m_banked_ram = make_unique_clear<uint8_t[]>(0xc000);

	m_banks[0]->configure_entry(0, m_banked_ram.get());
	m_banks[0]->configure_entry(1, m_rom->base());
	m_banks[0]->set_entry(1);

	m_rom0000 = 1;
	m_mc6845_bank = 0;
	m_mc6847_bank = 0;

	m_banks[1]->configure_entry(0, m_banked_ram.get() + 0x2000);
	m_banks[1]->configure_entry(1, m_mc6845_video_ram);
	m_banks[1]->set_entry(0);

	m_banks[2]->configure_entry(0, m_banked_ram.get() + 0x4000);
	m_banks[2]->set_entry(0);

	m_banks[3]->configure_entry(0, m_mc6847_video_ram);
	m_banks[3]->configure_entry(1, m_banked_ram.get() + 0x8000);
	m_banks[3]->set_entry(0);

	m_banks[4]->configure_entry(0, m_banked_ram.get() + 0x9800);
	m_banks[4]->set_entry(0);

	bankswitch();

	/* register for state saving */
	save_pointer(NAME(m_banked_ram), 0xc000);
	save_item(NAME(m_rom0000));
	save_item(NAME(m_mc6845_bank));
	save_item(NAME(m_mc6847_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
}

void mc1000_state::machine_reset()
{
	m_banks[0]->set_entry(1);

	m_rom0000 = 1;
}

/* Machine Driver */

/*

 Interrupt generator:
 NE555 chip in astable circuit.

  +---------*---*---o V+
  |         |   |
 +-+        |   |
 | |390K    |   |
 | |R17     |8  |4
 +-+      +-------+
  |      7|       |3
  *-------|       |-------> /INT (Z80)
  |       |       |
  |       |       |
 +-+R16  2| IC 28 |
 | |1K +--|       |
 | |   |  |  555  |
 +-+   |  |       |
  |    | 6|       |5
  *----*--|       |---+
  |       |       |   |
 ---C30   +-------+  ---C29
 ---10n       |1     ---10n
 _|_         _|_     _|_
 ///         ///     ///

 Calculated properties:

 * 99.74489795918367 Duty Cycle Percentage
 * 367.3469387755102 Frequency in Hertz
 * 0.00000693 Seconds Low
 * 0.00270963 Seconds High

 */

#define MC1000_NE555_FREQ       (367) /* Hz */
#define MC1000_NE555_DUTY_CYCLE (99.745) /* % */

TIMER_DEVICE_CALLBACK_MEMBER(mc1000_state::ne555_tick)
{
	// (m_ne555_int not needed anymore and can be done with?)
	m_ne555_int = param;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, param);
}

void mc1000_state::mc1000(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3579545);
	m_maincpu->set_addrmap(AS_PROGRAM, &mc1000_state::mc1000_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &mc1000_state::mc1000_banking_mem);
	m_maincpu->set_addrmap(AS_IO, &mc1000_state::mc1000_io);

	/* timers */
	timer_device &ne555clear(TIMER(config, "ne555clear"));
	ne555clear.configure_periodic(FUNC(mc1000_state::ne555_tick), attotime::from_hz(MC1000_NE555_FREQ));
	ne555clear.config_param(CLEAR_LINE);

	timer_device &ne555assert(TIMER(config, "ne555assert"));
	ne555assert.configure_periodic(FUNC(mc1000_state::ne555_tick), attotime::from_hz(MC1000_NE555_FREQ));
	ne555assert.set_start_delay(attotime::from_hz(MC1000_NE555_FREQ * 100 / MC1000_NE555_DUTY_CYCLE));
	ne555assert.config_param(ASSERT_LINE);

	/* video hardware */
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_vdg, XTAL(3'579'545));
	m_vdg->hsync_wr_callback().set(FUNC(mc1000_state::hs_w));
	m_vdg->fsync_wr_callback().set(FUNC(mc1000_state::fs_w));
	m_vdg->input_callback().set(FUNC(mc1000_state::videoram_r));
	m_vdg->set_screen(SCREEN_TAG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, AY8910_TAG, 3579545/2));
	ay8910.set_flags(AY8910_SINGLE_OUTPUT);
	ay8910.set_resistors_load(RES_K(2.2), 0, 0);
	ay8910.port_b_read_callback().set(FUNC(mc1000_state::keydata_r));
	ay8910.port_a_write_callback().set(FUNC(mc1000_state::keylatch_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("mc1000_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("mc1000_cass");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(mc1000_state::write_centronics_busy));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("48K");
}

/* ROMs */

ROM_START( mc1000 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_LOAD( "mc1000.ic17", 0x0000, 0x2000, CRC(8e78d80d) SHA1(9480270e67a5db2e7de8bc5c8b9e0bb210d4142b) )
	ROM_LOAD( "mc1000.ic12", 0x2000, 0x2000, CRC(750c95f0) SHA1(fd766f5ea4481ef7fd4df92cf7d8397cc2b5a6c4) )
ROM_END

} // Anonymous namespace


/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS */
COMP( 1985, mc1000, 0,      0,      mc1000,  mc1000, mc1000_state, empty_init, "CCE",   "MC-1000", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
