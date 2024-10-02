// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Basic Master Jr. (MB-6885) (c) 1982? Hitachi

TODO:
- Identify and improve Sound DAC details;
- Keyboard eats inputs if typed relatively fast (verify);
- Add kana mappings to keyboard;
- Timer control (needs SW that bothers with it);
- Downgrade for earlier variants (needs dump first), convert MP-1710 color adapter to expansion
  bus device (bmjr specific);
- Floppy adapter MP-1803, thru expansion bus;
- Border color for MP-1710;
- Printer, MP-1041/MP-1045;
- Hookup SW list;

**************************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class bmjr_state : public driver_device
{
public:
	bmjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_dac(*this, "dac")
		, m_work_ram(*this, "work_ram")
		, m_basic_view(*this, "basic_view")
		, m_printer_view(*this, "printer_view")
		, m_monitor_view(*this, "monitor_view")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "KEY%d", 0U)
	{ }

	void bmjr(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(break_key_pressed);
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
private:
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_5bit_binary_weighted_device> m_dac;
	required_shared_ptr<u8> m_work_ram;
	memory_view m_basic_view;
	memory_view m_printer_view;
	memory_view m_monitor_view;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<16> m_io_keyboard;

	bool m_tape_switch = 0;
	u8 m_screen_reverse = 0U;
	u8 m_key_select = 0U;
	u8 m_nmi_enable = 0U;
	u16 m_casscnt = 0U;
	bool m_cassold = 0, m_cassbit = 0;
	u8 m_bank_mode = 0U;

	u8 bank_mode_r();
	void bank_mode_w(u8 data);
	u8 key_r();
	void key_w(u8 data);
	u8 timer_r();
	u8 tape_r();
	void tape_w(u8 data);
	u8 tape_stop_r();
	u8 tape_start_r();
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);

	std::unique_ptr<u8[]> m_color_ram;
	u8 m_color_mode = 0U;
	u8 m_tile_latch = 0U;
	u8 m_screen_mode = 0U;
	void mp1710_map(address_map &map) ATTR_COLD;
	void screen_mode_w(u8 data);
	void screen_reverse_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void bmjr_state::video_start()
{
	m_color_ram = std::make_unique<u8[]>(0x300);
	save_item(NAME(m_screen_mode));
	save_item(NAME(m_screen_reverse));
	save_item(NAME(m_color_mode));
	save_item(NAME(m_tile_latch));
	save_pointer(NAME(m_color_ram), 0x300);
}

void bmjr_state::video_reset()
{
	m_tile_latch = 0x07;
	m_screen_reverse = 0;
	m_screen_mode = 0;
}

u32 bmjr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 fg_shift = m_screen_reverse ? 4 : 0;
	const u8 bg_shift = m_screen_reverse ? 0 : 4;
	const u16 screen_bank_offset = 0x900 + ((m_screen_mode & 0xf) << 9);

	// TODO: convert to scanline based renderer
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++ )
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x+= 8)
		{
			const u16 tile_offset = (x >> 3) + ((y >> 3) * 32);
			const u16 tile = m_work_ram[0x100 + tile_offset] << 3;
			const u8 gfx_data = BIT(m_screen_mode, 7) ? m_work_ram[screen_bank_offset + (x >> 3) + (y * 32)] : m_p_chargen[tile | (y & 7)];
			const u8 attr = BIT(m_color_mode, 0) ? m_color_ram[tile_offset] : 0x07;
			const u8 fg_color = (attr >> fg_shift) & 7;
			const u8 bg_color = (attr >> bg_shift) & 7;

			for (int xi = 0; xi < 8; xi++)
			{
				const u8 pen = BIT(gfx_data, 7 - xi) ? fg_color : bg_color;
				bitmap.pix(y, x + xi) = pen;
			}
		}
	}

	return 0;
}

u8 bmjr_state::key_r()
{
	return (m_io_keyboard[m_key_select]->read() & 0xf) | ioport("KEYMOD")->read();
}

void bmjr_state::key_w(u8 data)
{
	m_key_select = data & 0xf;

	m_nmi_enable = BIT(data, 7);
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

/*
 * x--- ---- checked in irq routine
 * 1--- ---- routes to 1 second timer at $000c (TIME 3)
 * 0--- ---- routes to $000a-$000b (TIME 1/TIME 2), likely enabled thru $efd0 bit 4 high
 */
u8 bmjr_state::timer_r()
{
	return 0xff;
}

TIMER_DEVICE_CALLBACK_MEMBER( bmjr_state::kansas_r )
{
	/* cassette - turn pulses into a bit */
	bool cass_ws = (m_cassette->input() > +0.04) ? 1 : 0;
	m_casscnt++;

	if (cass_ws != m_cassold)
	{
		m_cassold = cass_ws;
		m_cassbit = (m_casscnt < 12) ? 1 : 0;
		m_casscnt = 0;
	}
	else if (m_casscnt > 32)
	{
		m_casscnt = 32;
		m_cassbit = 0;
	}
}

u8 bmjr_state::tape_r()
{
	//m_cassette->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);

	return m_cassbit ? 0xff : 0x00;
}

void bmjr_state::tape_w(u8 data)
{
	if(!m_tape_switch)
	{
		m_dac->write((data >> 1) & 0x1f);
	}
	else
	{
		//m_cassette->change_state(CASSETTE_RECORD,CASSETTE_MASK_UISTATE);
		m_cassette->output(BIT(data, 0) ? -1.0 : +1.0);
	}
}

u8 bmjr_state::tape_stop_r()
{
	m_tape_switch = 0;
	//m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
	m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
	return 0x01;
}

u8 bmjr_state::tape_start_r()
{
	m_tape_switch = 1;
	m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	return 0x01;
}

u8 bmjr_state::bank_mode_r()
{
	return m_bank_mode;
}

/*
 * ---x ---- timer enable?
 * ---- -x-- maps $f000-$ffff to work RAM
 * ---- --x- maps $e000-$edff to work RAM
 * ---- ---x maps $b000-$dfff to work RAM
 */
void bmjr_state::bank_mode_w(u8 data)
{
	logerror("bank_mode_w %02x\n", data);
	m_bank_mode = data;
	if (BIT(data, 0))
		m_basic_view.disable();
	else
		m_basic_view.select(0);

	if (BIT(data, 1))
		m_printer_view.disable();
	else
		m_printer_view.select(0);

	if (BIT(data, 2))
		m_monitor_view.disable();
	else
		m_monitor_view.select(0);
}

void bmjr_state::screen_reverse_w(u8 data)
{
	m_screen_reverse = BIT(data, 7);
}

/*
 * x--- ---- enable bitmap mode
 * -?-- ---- <unknown, looks set in tandem with bit 7>
 * ---- xxxx bank base for bitmap mode
 */
void bmjr_state::screen_mode_w(u8 data)
{
	m_screen_mode = data;
}

void bmjr_state::mp1710_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) { return m_tile_latch; }),
		NAME([this] (offs_t offset, u8 data) { m_tile_latch = data; })
	);
	//map(0x01, 0x01) border color?
	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) { return m_color_mode; }),
		NAME([this] (offs_t offset, u8 data) { m_color_mode = data; })
	);
}

void bmjr_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("work_ram");
	// overlay for MP-1710 tile latches
	map(0x0100, 0x03ff).lrw8(
		NAME([this] (offs_t offset) { return m_work_ram[offset + 0x100]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_work_ram[offset + 0x100] = data;
			m_color_ram[offset] = m_tile_latch;
		})
	);
	map(0xb000, 0xdfff).view(m_basic_view);
	m_basic_view[0](0xb000, 0xdfff).rom().region("basic", 0);
	map(0xe000, 0xefff).view(m_printer_view);
	m_printer_view[0](0xe000, 0xe7ff).rom().region("printer", 0);
	// 0xe800-0xedff expansion I/O
//  map(0xe800, 0xe803) 6820 or 6821 PIA
	m_printer_view[0](0xe890, 0xe89f).m(*this, FUNC(bmjr_state::mp1710_map));
	// 0xee00-0xefff system I/O (ignored by printer view enabled)
	map(0xee00, 0xee00).r(FUNC(bmjr_state::tape_stop_r)); //R stop tape
	map(0xee20, 0xee20).r(FUNC(bmjr_state::tape_start_r)); //R start tape
	map(0xee40, 0xee40).w(FUNC(bmjr_state::screen_reverse_w)); //W Picture reverse
	map(0xee80, 0xee80).rw(FUNC(bmjr_state::tape_r), FUNC(bmjr_state::tape_w));//RW tape input / output
	map(0xeec0, 0xeec0).rw(FUNC(bmjr_state::key_r), FUNC(bmjr_state::key_w));//RW keyboard
	map(0xef00, 0xef00).r(FUNC(bmjr_state::timer_r));
	map(0xef80, 0xef80).portr("BREAK");
	map(0xefd0, 0xefd0).rw(FUNC(bmjr_state::bank_mode_r), FUNC(bmjr_state::bank_mode_w));
	map(0xefe0, 0xefe0).w(FUNC(bmjr_state::screen_mode_w));
	map(0xf000, 0xffff).view(m_monitor_view);
	m_monitor_view[0](0xf000, 0xffff).rom().region("monitor", 0);
}

INPUT_CHANGED_MEMBER(bmjr_state::break_key_pressed)
{
	if (newval && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static INPUT_PORTS_START( bmjr )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("KEY10")
	// NOTE: works on kana / shift only
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_") PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"@ \u2191") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("KEY11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"[ \u2193") PORT_CODE(KEYCODE_OPENBRACE) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('[')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"^ \u2192") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('^')

	PORT_START("KEY12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"¥ \u2190") PORT_CODE(KEYCODE_4_PAD)

	PORT_START("KEY13")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY14")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY15")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYMOD") /* Note: you should press Normal to return from a Kana state and vice-versa */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( Normal )) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Kana Shift") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Kana") PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("BREAK")
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // TODO: read by timer irq service
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CHANGED_MEMBER(DEVICE_SELF, bmjr_state, break_key_pressed, 0)
INPUT_PORTS_END

static const gfx_layout bmjr_charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8*8
};

static GFXDECODE_START( gfx_bmjr )
	GFXDECODE_ENTRY( "chargen", 0x0000, bmjr_charlayout, 0, 4 )
GFXDECODE_END

void bmjr_state::machine_start()
{
	save_item(NAME(m_tape_switch));
	save_item(NAME(m_key_select));
	save_item(NAME(m_casscnt));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_bank_mode));
}

void bmjr_state::machine_reset()
{
	//m_beep->set_state(0);
	m_tape_switch = 0;
	m_key_select = 0;
	m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
	m_bank_mode = 0;
	m_basic_view.select(0);
	m_printer_view.select(0);
	m_monitor_view.select(0);
	m_maincpu->reset();
}

void bmjr_state::bmjr(machine_config &config)
{
	// 750khz gets the cassette sound close to a normal kansas city 300 baud
	M6800(config, m_maincpu, 754'560); // TODO: HD46800, derive from actual clock / divider
	m_maincpu->set_addrmap(AS_PROGRAM, &bmjr_state::main_map);
	// NOTE: checked by using TIME commands, which implies a separate thread than the actual timer control
	m_maincpu->set_vblank_int("screen", FUNC(bmjr_state::irq0_line_hold));

	// TRQ237/TRQ359
	CASSETTE(config, m_cassette);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("bmjr_cass");

	TIMER(config, "kansas_r").configure_periodic(FUNC(bmjr_state::kansas_r), attotime::from_hz(40000));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(320, 262);
	screen.set_visarea(0, 256 - 1, 0, 192 - 1);
	screen.set_screen_update(FUNC(bmjr_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::BRG_3BIT);
	GFXDECODE(config, "gfxdecode", "palette", gfx_bmjr);

	SPEAKER(config, "mono").front_center();
	// TODO: unknown DAC type, likely connected to discrete circuitry.
	DAC_5BIT_BINARY_WEIGHTED(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.25);

	SOFTWARE_LIST(config, "cass_list").set_original("bmjr_cass");
}

/* ROM definition */
ROM_START( bmjr )
	ROM_REGION( 0x3000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "bas.rom", 0x0000, 0x3000, BAD_DUMP CRC(2318e04e) SHA1(cdb3535663090f5bcaba20b1dbf1f34724ef6a5f)) // needs splitting in three halves

	ROM_REGION( 0x1000, "monitor", ROMREGION_ERASEFF )
	ROM_LOAD( "mon.rom", 0x0000, 0x1000, CRC(776cfa3a) SHA1(be747bc40fdca66b040e0f792b05fcd43a1565ce))

	ROM_REGION( 0x800, "printer", ROMREGION_ERASEFF )
	ROM_LOAD( "prt.rom", 0x000, 0x800, CRC(b9aea867) SHA1(b8dd5348790d76961b6bdef41cfea371fdbcd93d))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(258c6fd7) SHA1(d7c7dd57d6fc3b3d44f14c32182717a48e24587f)) // taken from a JP emulator
ROM_END

} // anonymous namespace


// 1979 Basic Master MB-6880 (retroactively Level 1)
// 1979 Basic Master Level 2 MB-6880L2
// 1980 Basic Master Level 2 II MB-6881
COMP( 1981, bmjr, 0,      0,      bmjr,    bmjr,  bmjr_state, empty_init, "Hitachi", "Basic Master Jr. (MB-6885)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
