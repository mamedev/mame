// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

CASIO PV-2000

(preliminary work by anondumper)
Thanks for the loaner (Ianoid)

NOTE:
The PCB has printed names of components, not ICXY, etc
but: "hn613128pc64.bin"

SEE
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_k1.jpg
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_14.jpg
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_15.jpg

Keyboard inputs are partially supported. Keys missing from the input ports:
- GAME - no beep in basic - is this really a key?

Todo:
- Add joystick support
- Cassette support

Also See:
http://www2.odn.ne.jp/~haf09260/Pv2000/EnrPV.htm
For BIOS CRC confirmation
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "imagedev/cassette.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class pv2000_state : public driver_device
{
public:
	pv2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cass(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_keyboard(*this, "IN%u", 0U)
	{ }

	void pv2000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<10> m_keyboard;
	void cass_conf_w(uint8_t data);
	void keys_w(uint8_t data);
	uint8_t keys_hi_r();
	uint8_t keys_lo_r();
	uint8_t keys_mod_r();
	void pv2000_vdp_interrupt(int state);
	uint8_t cass_in();
	void cass_out(uint8_t data);
	bool m_last_state = false;
	uint8_t m_key_pressed = 0;
	uint8_t m_keyb_column = 0;
	uint8_t m_cass_conf = 0;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void pv2000_io_map(address_map &map) ATTR_COLD;
	void pv2000_map(address_map &map) ATTR_COLD;
};


void pv2000_state::cass_conf_w(uint8_t data)
{
	logerror("%s: cass_conf_w %02x\n", machine().describe_context(), data);

	m_cass_conf = data & 0x0f;

	if (m_cass_conf & 0x01)
		m_cass->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	else
		m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}


void pv2000_state::keys_w(uint8_t data)
{
	logerror("%s: keys_w %02x\n", machine().describe_context(), data);

	m_keyb_column = data & 0x0f;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


uint8_t pv2000_state::keys_hi_r()
{
	uint8_t data = 0;

	switch (m_keyb_column)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		data = m_keyboard[m_keyb_column]->read() >> 4;
	}

	return data;
}


uint8_t pv2000_state::keys_lo_r()
{
	uint8_t data = 0;

	logerror("%s: pv2000_keys_r\n", machine().describe_context());

	switch (m_keyb_column)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		data = m_keyboard[m_keyb_column]->read() & 0x0f;
	}

	return 0xf0 | data;
}


uint8_t pv2000_state::keys_mod_r()
{
	return 0xf0 | ioport("MOD")->read();
}

uint8_t pv2000_state::cass_in()
{
	// from what i can tell,
	// 0 = data in
	// 1 = must be high
	// 2 = must be low
	// bits 1 & 2 are checked while reading and writing tapes
	// Press STOP key (F1) to cancel LOAD or SAVE

	return 2 | ((m_cass->input() > +0.03) ? 1 : 0);
}

void pv2000_state::cass_out(uint8_t data)
{
	// it outputs 8-bit values here which are not the bytes in the file
	// result is not readable

	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}


/* Memory Maps */

void pv2000_state::pv2000_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();

	map(0x4000, 0x4001).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));

	map(0x7000, 0x7fff).ram();
	//map(0x8000, 0xbfff) ext ram?
	//map(0xc000, 0xffff)      // mapped by the cartslot
}


void pv2000_state::pv2000_io_map(address_map &map)
{
	map.global_mask(0xff);

	//theres also printer and tape I/O (TODO)
	map(0x00, 0x00).w(FUNC(pv2000_state::cass_conf_w));

	//keyboard/joystick
	map(0x10, 0x10).r(FUNC(pv2000_state::keys_hi_r));
	map(0x20, 0x20).rw(FUNC(pv2000_state::keys_lo_r), FUNC(pv2000_state::keys_w));

	//sn76489a
	map(0x40, 0x40).r(FUNC(pv2000_state::keys_mod_r)).w("sn76489a", FUNC(sn76489a_device::write));

	/* Cassette input. Gets hit a lot after a GLOAD command */
	map(0x60, 0x60).rw(FUNC(pv2000_state::cass_in), FUNC(pv2000_state::cass_out));
}


static INPUT_PORTS_START( pv2000 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hiragana")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Yen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN4_1") /* Game?? DEL / MODE / STOP ??, no beep in basic, START in galaga */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN4_2") /* DEL / MODE / STOP ??, no beep in basic, SELECT in galaga */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Up+Left") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Down+Left") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Down+Right") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Up+Right") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN6_2") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN6_3") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN7_2") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN7_3") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("IN8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Attack 0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_NAME("Attack 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_2") /* Unknown ?, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_3") /* Unknown ?, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mode")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_7") /* Unknown ?, no beep in basic */

	PORT_START("IN9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Stop")

	PORT_START("MOD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("Color")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_NAME("Func")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void pv2000_state::pv2000_vdp_interrupt(int state)
{
	// only if it goes up
	if (state && !m_last_state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	m_last_state = state;

	/* Check if irq triggering from keyboard presses is enabled */
	if (m_keyb_column == 0x0f)
	{
		/* Check if a key is pressed */
		uint8_t key_pressed = m_keyboard[0]->read()
				| m_keyboard[1]->read()
				| m_keyboard[2]->read()
				| m_keyboard[3]->read()
				| m_keyboard[4]->read()
				| m_keyboard[5]->read()
				| m_keyboard[6]->read()
				| m_keyboard[7]->read()
				| m_keyboard[8]->read();

		if (key_pressed && m_key_pressed != key_pressed)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

		m_key_pressed = key_pressed;
	}
}



/* Machine Initialization */

void pv2000_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
}

void pv2000_state::machine_reset()
{
	m_last_state = 0;
	m_key_pressed = 0;
	m_keyb_column = 0;

	m_maincpu->set_input_line_vector(INPUT_LINE_IRQ0, 0xff); // Z80
	memset(&memregion("maincpu")->base()[0x7000], 0xff, 0x1000);    // initialize RAM
}

DEVICE_IMAGE_LOAD_MEMBER(pv2000_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size != 0x2000 && size != 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 8K or 16K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

/* Machine Drivers */
void pv2000_state::pv2000(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(7'159'090)/2); // 3.579545 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &pv2000_state::pv2000_map);
	m_maincpu->set_addrmap(AS_IO, &pv2000_state::pv2000_io_map);

	// video hardware
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(FUNC(pv2000_state::pv2000_vdp_interrupt));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, "sn76489a", XTAL(7'159'090)/2).add_route(ALL_OUTPUTS, "mono", 1.00); /* 3.579545 MHz */

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "pv2000_cart", "bin,rom,col").set_device_load(FUNC(pv2000_state::cart_load));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pv2000");
}



/* ROMs */
ROM_START (pv2000)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hn613128pc64.bin", 0x0000, 0x4000, CRC(8f31f297) SHA1(94b5f54dd7bce321e377fdaaf592acd3870cf621) )
ROM_END

} // anonymous namespace


/* System Drivers */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY   FULLNAME    FLAGS
CONS( 1983, pv2000, 0,      0,      pv2000,  pv2000, pv2000_state, empty_init, "Casio",  "PV-2000",  MACHINE_NOT_WORKING )
