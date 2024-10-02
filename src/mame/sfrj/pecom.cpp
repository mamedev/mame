// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Pecom driver by Miodrag Milanovic

2008-11-08 Preliminary driver.

- All commands to be in UPPERCASE.
- Change background colour: SCR n
- Enter monitor: PROB     (B to exit)
- If Capslock is engaged, then Shift doesn't work.
- Control hangs the machine while it is pressed. It doesn't work in the
  expected way.
- Don't touch the Shift key while loading a tape because it will corrupt
  the data.
- The screen will flash in a crazy epileptic fashion while loading a tape.
  Beware!

TODO:
- Cassette: can load its own recordings, but not those from software list
  (software-list tapes are slower & wobbly)
- Both machines currently have 32k ram.
- Autorepeat seems a bit fast

****************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "sound/cdp1869.h"

#include "softlist_dev.h"
#include "speaker.h"

namespace {

class pecom_state : public driver_device
{
public:
	pecom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cdp1869(*this, "cdp1869")
		, m_cassette(*this, "cassette")
		, m_bank1(*this, "bank1")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_io_cnt(*this, "CNT")
		, m_io_keyboard(*this, "LINE%d", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(ef_w);
	void pecom64(machine_config &config);

private:
	void bank_w(uint8_t data);
	uint8_t keyboard_r();
	void cdp1869_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(reset_tick);
	int clear_r();
	int ef2_r();
	void q_w(int state);
	void sc_w(uint8_t data);
	void prd_w(int state);
	CDP1869_CHAR_RAM_READ_MEMBER(char_ram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(char_ram_w);
	CDP1869_PCB_READ_MEMBER(pcb_r);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void cdp1869_page_ram(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_charram;           /* character generator ROM */
	bool m_reset = false;                /* CPU mode */
	bool m_dma = false;              /* memory refresh DMA */

	/* timers */
	emu_timer *m_reset_timer = nullptr;   /* power on reset timer */

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_cdp1869;
	required_device<cassette_image_device> m_cassette;
	required_memory_bank m_bank1;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_ioport m_io_cnt;
	required_ioport_array<26> m_io_keyboard;
};

TIMER_CALLBACK_MEMBER(pecom_state::reset_tick)
{
	m_reset = true;
}

void pecom_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bank1->set_entry(1);

	space.unmap_write(0xf000, 0xffff);
	space.install_read_bank (0xf000, 0xf7ff, m_bank3);
	space.install_read_bank (0xf800, 0xffff, m_bank4);
	m_bank3->set_base(m_rom + 0x7000);
	m_bank4->set_base(m_rom + 0x7800);

	m_reset = false;
	m_dma = false;
	m_reset_timer->adjust(attotime::from_msec(5));
}

void pecom_state::bank_w(uint8_t data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bank1->set_entry(0);

	if (data==2)
	{
		space.install_read_handler (0xf000, 0xf7ff, read8sm_delegate(m_cdp1869, FUNC(cdp1869_device::char_ram_r)));
		space.install_write_handler(0xf000, 0xf7ff, write8sm_delegate(m_cdp1869, FUNC(cdp1869_device::char_ram_w)));
		space.install_read_handler (0xf800, 0xffff, read8sm_delegate(m_cdp1869, FUNC(cdp1869_device::page_ram_r)));
		space.install_write_handler(0xf800, 0xffff, write8sm_delegate(m_cdp1869, FUNC(cdp1869_device::page_ram_w)));
	}
	else
	{
		space.unmap_write(0xf000, 0xffff);
		space.install_read_bank (0xf000, 0xf7ff, m_bank3);
		space.install_read_bank (0xf800, 0xffff, m_bank4);
		m_bank3->set_base(m_rom + 0x7000);
		m_bank4->set_base(m_rom + 0x7800);
	}
}

uint8_t pecom_state::keyboard_r()
{
	/*
	   INP command BUS -> M(R(X)) BUS -> D
	   so on each input, address is also set, 8 lower bits are used as input for keyboard
	   Address is available on address bus during reading of value from port, and that is
	   used to determine keyboard line reading
	*/
	uint16_t addr = m_maincpu->state_int(cosmac_device::COSMAC_R0 + m_maincpu->state_int(cosmac_device::COSMAC_X));
	/* just in case someone is reading non existing ports */
	if (addr<0x7cca || addr>0x7ce3) return 0;
	return m_io_keyboard[addr - 0x7cca]->read();
}

/* CDP1802 Interface */

int pecom_state::clear_r()
{
	return m_reset;
}

int pecom_state::ef2_r()
{
	bool shift = BIT(m_io_cnt->read(), 1);
	double cas = false;//m_cassette->input();

	return (cas < -0.02) | shift; // touching shift kills cassette load
}

void pecom_state::q_w(int state)
{
	m_cassette->output(state ? -1.0 : +1.0);
}

void pecom_state::sc_w(uint8_t data)
{
	switch (data)
	{
		case COSMAC_STATE_CODE_S2_DMA:
			// DMA acknowledge clears the DMAOUT request
			m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, CLEAR_LINE);
			break;

		default:
			break;
	}
}

void pecom_state::cdp1869_w(offs_t offset, uint8_t data)
{
	uint16_t ma = m_maincpu->get_memory_address();

	switch (offset + 3)
	{
	case 3:
		m_cdp1869->out3_w(data);
		break;

	case 4:
		m_cdp1869->out4_w(ma);
		break;

	case 5:
		m_cdp1869->out5_w(ma);
		break;

	case 6:
		m_cdp1869->out6_w(ma);
		break;

	case 7:
		m_cdp1869->out7_w(ma);
		break;
	}
}

CDP1869_CHAR_RAM_READ_MEMBER(pecom_state::char_ram_r )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	return m_charram[charaddr];
}

CDP1869_CHAR_RAM_WRITE_MEMBER(pecom_state::char_ram_w )
{
	uint8_t column = pmd & 0x7f;
	uint16_t charaddr = (column << 4) | cma;

	m_charram[charaddr] = data;
}

CDP1869_PCB_READ_MEMBER(pecom_state::pcb_r )
{
	return BIT(pmd, 7);
}

void pecom_state::prd_w(int state)
{
	// every other PRD triggers a DMAOUT request
	if (m_dma)
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAOUT, HOLD_LINE);

	m_dma = !m_dma;
}

void pecom_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);

	/* allocate memory */
	m_charram = std::make_unique<uint8_t[]>(0x0800);

	/* register for state saving */
	save_item(NAME(m_reset));
	save_item(NAME(m_dma));
	save_pointer(NAME(m_charram), 0x0800);
	m_reset_timer = timer_alloc(FUNC(pecom_state::reset_tick), this);
}

/* Address maps */
void pecom_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x8000, 0xefff).rom().region("maincpu",0);
	map(0xf000, 0xf7ff).bankrw("bank3"); // CDP1869 / ROM
	map(0xf800, 0xffff).bankrw("bank4"); // CDP1869 / ROM
}

void pecom_state::io_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(pecom_state::bank_w));
	map(0x03, 0x03).r(FUNC(pecom_state::keyboard_r));
	map(0x03, 0x07).w(FUNC(pecom_state::cdp1869_w));
}

void pecom_state::cdp1869_page_ram(address_map &map)
{
	map(0x000, 0x3ff).mirror(0x400).ram();
}

/* Input ports */
/* Pecom 64 keyboard layout is as follows

    1!     2"     3#     4$     5%     6&     7'     8(     9)     0     BREAK

   DEL  Q      W      E      R      T      Y      U      I      O      P   ESC

    CAPS    A      S      D      F      G      H      J      K      L   RETURN

  CTRL  ,<     Z      X      C      V      B      N      M      :*     /?   LF

     SHIFT  .>    Down   Left       SPACEBAR       Right    Up      ;+    =-

Being keys distributed on four lines, it makes a bit difficult to accurately remap them
on modern keyboards. Hence, we move by default Up/Down/Left/Right to Cursor Keys and
use Left/Right Ctrl/Alt keys for the remaining keys. Due to the unnatural emulated keyboard
mappings, this is another situation where natural keyboard comes very handy!          */

INPUT_CHANGED_MEMBER(pecom_state::ef_w)
{
	m_maincpu->set_input_line((int)param, newval);
}

static INPUT_PORTS_START( pecom )
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_COLON) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // Actually this is again / ? - same key connected as on SLASH

	PORT_START("LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR('=') PORT_CHAR('-')

	PORT_START("LINE9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("LINE11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("LINE12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')

	PORT_START("LINE13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("LINE15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("LINE16")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("LINE17")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')

	PORT_START("LINE19")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("LINE20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')

	PORT_START("LINE21")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE22")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')

	PORT_START("LINE23")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Š  š  \u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(U'Š') PORT_CHAR(U'š')

	PORT_START("LINE24")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Đ  đ  \u2190") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(U'Đ') PORT_CHAR(U'đ')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Ć  ć  \u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(U'Ć') PORT_CHAR(U'ć')

	PORT_START("LINE25")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Č  č  \u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(U'Č') PORT_CHAR(U'č')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("CNT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_MINUS) PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF4)
INPUT_PORTS_END

/* Machine driver */
void pecom_state::pecom64(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, cdp1869_device::DOT_CLK_PAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pecom_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pecom_state::io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(pecom_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(pecom_state::ef2_r));
	m_maincpu->q_cb().set(FUNC(pecom_state::q_w));
	m_maincpu->sc_cb().set(FUNC(pecom_state::sc_w));

	SPEAKER(config, "mono").front_center();

	CDP1869(config, m_cdp1869, cdp1869_device::DOT_CLK_PAL, &pecom_state::cdp1869_page_ram);
	m_cdp1869->add_pal_screen(config, "screen", cdp1869_device::DOT_CLK_PAL);
	m_cdp1869->set_color_clock(cdp1869_device::COLOR_CLK_PAL);
	m_cdp1869->set_pcb_read_callback(FUNC(pecom_state::pcb_r));
	m_cdp1869->set_char_ram_read_callback(FUNC(pecom_state::char_ram_r));
	m_cdp1869->set_char_ram_write_callback(FUNC(pecom_state::char_ram_w));
	m_cdp1869->pal_ntsc_callback().set_constant(1);
	m_cdp1869->prd_callback().set(FUNC(pecom_state::prd_w));
	m_cdp1869->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("pecom_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("pecom_cass");
}

/* ROM definition */
ROM_START( pecom32 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "090786.bin", 0x0000, 0x4000, CRC(b3b1ea23) SHA1(de69f22568161ced801973345fa39d6d207b9e8c) )
ROM_END

ROM_START( pecom64 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "ver4", "version 4")
	ROMX_LOAD( "rom_1_g_24.02.88_l.bin", 0x0000, 0x4000, CRC(9a433b47) SHA1(dadb8c399e0a25a2693e10e42a2d7fc2ea9ad427), ROM_BIOS(0) )
	ROMX_LOAD( "rom_2_g_24.02.88_d.bin", 0x4000, 0x4000, CRC(2116cadc) SHA1(03f11055cd221d438a40a41874af8fba0fa116d9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ver1", "version 1")
	ROMX_LOAD( "170887-rom1.bin", 0x0000, 0x4000, CRC(43710fb4) SHA1(f84f75061c9ac3e34af93141ecabd3c955881aa2), ROM_BIOS(1) )
	ROMX_LOAD( "170887-rom2.bin", 0x4000, 0x4000, CRC(d0d34f08) SHA1(7baab17d1e68771b8dcef97d0fffc655beabef28), ROM_BIOS(1) )
ROM_END

} // Anonymous namespace

/* Driver */

/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME     FLAGS */
COMP( 1986, pecom32, 0,       0,      pecom64, pecom, pecom_state, empty_init, "Ei Nis (Elektronska Industrija Nis)", "Pecom 32", MACHINE_SUPPORTS_SAVE )
COMP( 1987, pecom64, pecom32, 0,      pecom64, pecom, pecom_state, empty_init, "Ei Nis (Elektronska Industrija Nis)", "Pecom 64", MACHINE_SUPPORTS_SAVE )
