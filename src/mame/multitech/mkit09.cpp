// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Multitech Microkit09

2013-12-08 Mostly working driver.


ToDo:
    - Need software to test with

Pasting:
    0-F : as is
    (inc) : ^
    (dec) : V
    M (memory) : -
    G (Go) : X

Test Paste:
    -0000 00^11^22^33^44^55^66^77^88^99^--0000
    Now press up-arrow to confirm the data has been entered.



2015-10-02 Added alternate bios found on a forum. Memory map is different.
               Still to fix keyboard and display. No documentation exists.

2019-10-10 Adjusted mkit09a to display and accept input. However it appears
               to be some other 6809 trainer. Although it "works", the usage
               is largely unknown, as are some of the keys. Cassette status
               also unknown. There may be a device at E400-E407.
               When R (regs) is pressed, you can press 0(CC),1(A),2(B),3(DP),
               4,(IX),5(IY),6(U),7(PC),8(SP).
               Some patches were needed due to bugs or bad dump?

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "video/pwm.h"
#include "speaker.h"

#include "mkit09.lh"


namespace {

class mkit09_state : public driver_device
{
public:
	mkit09_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pia(*this, "pia")
		, m_cass(*this, "cassette")
		, m_maincpu(*this, "maincpu")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void mkit09(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);

protected:
	u8 pa_r();
	u8 pb_r();
	u8 m_digit = 0U;
	u8 m_seg = 0U;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	required_device<pia6821_device> m_pia;
	required_device<cassette_image_device> m_cass;
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_io_keyboard;

private:
	void pa_w(u8 data);
	void pb_w(u8 data);
	void mkit09_mem(address_map &map) ATTR_COLD;
};

class mkit09a_state : public mkit09_state
{
public:
	using mkit09_state::mkit09_state;

	void mkit09a(machine_config &config);

private:
	void pa_w(u8 data);
	void pb_w(u8 data);
	void mkit09a_mem(address_map &map) ATTR_COLD;
};


void mkit09_state::mkit09_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0xa004, 0xa007).mirror(0x1ff8).rw(m_pia, FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xe000, 0xe7ff).mirror(0x1800).rom().region("roms", 0);
}

void mkit09a_state::mkit09a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram();
	map(0xe600, 0xe603).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xee00, 0xefff).ram();
	map(0xf000, 0xffff).rom().region("roms", 0);
}

/* Input ports */
static INPUT_PORTS_START( mkit09 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Inc") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Dec") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BP") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("cnt") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ofs") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_nmi, 0)
INPUT_PORTS_END

// ToDo: work out what the keys marked "??" do.
static INPUT_PORTS_START( mkit09a )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Inc") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Dec") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) // ??
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("cnt") PORT_CODE(KEYCODE_W) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ofs") PORT_CODE(KEYCODE_O) // ?? (same as G?)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BP") PORT_CODE(KEYCODE_Q) // ?? (same as X?)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) // ??
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_nmi, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( mkit09_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( mkit09_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


void mkit09_state::machine_reset()
{
	m_digit = 0;
}

void mkit09_state::machine_start()
{
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}
// read keyboard
u8 mkit09_state::pa_r()
{
	if (m_digit < 4)
		return m_io_keyboard[m_digit]->read();

	return 0xff;
}

// read cassette
u8 mkit09_state::pb_r()
{
	return m_digit | ((m_cass->input() > +0.03) ? 0x80 : 0);
}

// write display segments
void mkit09_state::pa_w(u8 data)
{
	m_seg = bitswap<8>(~data, 7, 0, 5, 6, 4, 2, 1, 3);

	if ((m_digit > 3) && (m_digit < 10))
		m_display->matrix(1 << m_digit, m_seg);
}

void mkit09a_state::pa_w(u8 data)
{
	m_seg = data;

	if ((m_digit > 3) && (m_digit < 10))
		m_display->matrix(1 << (13-m_digit), m_seg);
}

// write cassette, select keyboard row, select a digit
void mkit09_state::pb_w(u8 data)
{
	m_cass->output(BIT(data, 6) ? -1.0 : +1.0);
	m_digit = data & 15;

	if ((m_digit > 3) && (m_digit < 10))
		m_display->matrix(1 << m_digit, m_seg);
}

void mkit09a_state::pb_w(u8 data)
{
	m_cass->output(BIT(data, 6) ? -1.0 : +1.0);
	m_digit = data & 15;

	if ((m_digit > 3) && (m_digit < 10))
		m_display->matrix(1 << (13-m_digit), m_seg);
}


void mkit09_state::mkit09(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mkit09_state::mkit09_mem);

	/* video hardware */
	config.set_default_layout(layout_mkit09);
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3f0, 0xff);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* Devices */
	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(mkit09_state::pa_r));
	m_pia->readpb_handler().set(FUNC(mkit09_state::pb_r));
	m_pia->writepa_handler().set(FUNC(mkit09_state::pa_w));
	m_pia->writepb_handler().set(FUNC(mkit09_state::pb_w));
	m_pia->irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	m_pia->irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void mkit09a_state::mkit09a(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mkit09a_state::mkit09a_mem);

	/* video hardware */
	config.set_default_layout(layout_mkit09);
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3f0, 0xff);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* Devices */
	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(mkit09a_state::pa_r));
	m_pia->readpb_handler().set(FUNC(mkit09a_state::pb_r));
	m_pia->writepa_handler().set(FUNC(mkit09a_state::pa_w));
	m_pia->writepb_handler().set(FUNC(mkit09a_state::pb_w));
	m_pia->cb2_handler().set_nop(); // stop errorlog filling up - is it a keyclick?
	m_pia->irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	m_pia->irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( mkit09 )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "micromon.bin", 0x0000, 0x0800, CRC(c993c7c2) SHA1(2f54a2b423b925798f669f8a6d2cadeb8a82e968) )
ROM_END

ROM_START( mkit09a )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "ukit09like.bin", 0x0000, 0x1000, CRC(2cdb6a84) SHA1(edfc1dfc954bdba80c3df64abf4d7553343c1fae) )
	ROM_FILL(0x1d8,1,0x03) // fix data display
	ROM_FILL(0x99b,1,0x06) // fix start address
	ROM_FILL(0x99c,1,0x63) // ... so that MEM starts at 0000 instead of F908.
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                    FLAGS
COMP( 1983, mkit09,  0,      0,      mkit09,  mkit09,  mkit09_state,  empty_init, "Multitech", "Microkit09",               MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1983, mkit09a, mkit09, 0,      mkit09a, mkit09a, mkit09a_state, empty_init, "Multitech", "Microkit09 (Alt version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
