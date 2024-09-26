// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    VideoBrain FamilyComputer

    http://www.atariprotos.com/othersystems/videobrain/videobrain.htm
    http://www.seanriddle.com/vbinfo.html
    http://www.seanriddle.com/videobrain.html
    http://www.google.com/patents/US4232374
    http://www.google.com/patents/US4177462
    http://www.orphanedgames.com/videobrain/
    http://www.datalytixllc.com/videobrain/
    http://blog.kevtris.org/blogfiles/videobrain/videobrain_unwrapped.txt

****************************************************************************/

/*

TODO:
    - wait states (UV201: 2.9us, memory except RES1: 1.65us)
    - interlaced video?
    - pinball background colors
    - Y-zoom starting on odd scanline only 1 line high
    - object height 0 glitch
    - object column 0xff glitch
    - video interrupts
    - R-2R ladder DAC
    - reset on cartridge unload
    - joystick scan timer 555
    - expander 1 (F3870 CPU, cassette, RS-232)
    - expander 2 (modem)

Keyboard:
    - When typing, (A .) key is often misinterpreted as (O #).
    - Shift is a toggle. This prevents Natural Keyboard & Paste from being
      able to shift as needed. The shift state shows as a block on the intro
      screen.

Using the system:
    - At startup, the intro screen gives the choices TEXT, COLOR, CLOCK, ALARM.
    - Press F1 for TEXT - this allows you to test the keyboard, but note that
      the Space key simply advances the cursor - it doesn't erase.
    - Press F2 for COLOR - this shows colour bars, presumably as a hardware test.
      In emulation, the colours are offset to the right and wrap around a bit.
    - Press F3 for CLOCK
    - Press F4 for ALARM

*/

#include "emu.h"

#include "bus/vidbrain/exp.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/dac.h"

#include "softlist_dev.h"
#include "speaker.h"
#include "uv201.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class vidbrain_state : public driver_device
{
public:
	vidbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_smi(*this, "smi"),
		m_uv(*this, "uv"),
		m_dac(*this, "dac"),
		m_exp(*this, "exp"),
		m_io(*this, "IO%02u", 0),
		m_uv201_31(*this, "UV201-31"),
		m_joy_r(*this, "JOY-R"),
		m_joy1_x(*this, "JOY1-X"),
		m_joy1_y(*this, "JOY1-Y"),
		m_joy2_x(*this, "JOY2-X"),
		m_joy2_y(*this, "JOY2-Y"),
		m_joy3_x(*this, "JOY3-X"),
		m_joy3_y(*this, "JOY3-Y"),
		m_joy4_x(*this, "JOY4-X"),
		m_joy4_y(*this, "JOY4-Y")
	{ }

	void vidbrain(machine_config &config);
	void vidbrain_video(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void vidbrain_mem(address_map &map) ATTR_COLD;
	void vidbrain_io(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(joystick_tick);

	void keyboard_w(uint8_t data);
	uint8_t keyboard_r();
	void sound_w(uint8_t data);

	void hblank_w(int state);
	uint8_t memory_read_byte(offs_t offset);

	required_device<cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<uv201_device> m_uv;
	required_device<dac_byte_interface> m_dac;
	required_device<videobrain_expansion_slot_device> m_exp;
	required_ioport_array<8> m_io;
	required_ioport m_uv201_31;
	required_ioport m_joy_r;
	required_ioport m_joy1_x;
	required_ioport m_joy1_y;
	required_ioport m_joy2_x;
	required_ioport m_joy2_y;
	required_ioport m_joy3_x;
	required_ioport m_joy3_y;
	required_ioport m_joy4_x;
	required_ioport m_joy4_y;

	// keyboard state
	uint8_t m_keylatch = 0;
	bool m_joy_enable = false;

	// sound state
	int m_sound_clk = 0;

	// timers
	emu_timer *m_timer_ne555 = nullptr;
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  keyboard_w - keyboard column write
//-------------------------------------------------

void vidbrain_state::keyboard_w(uint8_t data)
{
	/*

	    bit     description

	    0       keyboard column 0, sound data 0
	    1       keyboard column 1, sound data 1
	    2       keyboard column 2
	    3       keyboard column 3
	    4       keyboard column 4
	    5       keyboard column 5
	    6       keyboard column 6
	    7       keyboard column 7

	*/

	LOG("Keyboard %02x\n", data);

	m_keylatch = data;
}


//-------------------------------------------------
//  keyboard_r - keyboard row read
//-------------------------------------------------

uint8_t vidbrain_state::keyboard_r()
{
	/*

	    bit     description

	    0       keyboard row 0, joystick 1 fire
	    1       keyboard row 1, joystick 2 fire
	    2       keyboard row 2, joystick 3 fire
	    3       keyboard row 3, joystick 4 fire
	    4
	    5
	    6
	    7

	*/

	uint8_t data = m_joy_r->read();

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_keylatch, i)) data |= m_io[i]->read();
	}

	if (!m_uv->kbd_r()) data |= m_uv201_31->read();

	return data;
}


//-------------------------------------------------
//  sound_w - sound clock write
//-------------------------------------------------

void vidbrain_state::sound_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       sound clock
	    5       accessory jack pin 5
	    6       accessory jack pin 1
	    7       joystick enable

	*/

	LOG("Sound %02x\n", data);

	// sound clock
	int sound_clk = BIT(data, 4);

	if (!m_sound_clk && sound_clk)
	{
		m_dac->write(m_keylatch & 3);
	}

	m_sound_clk = sound_clk;

	// joystick enable
	m_joy_enable = !BIT(data, 7);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vidbrain_mem )
//-------------------------------------------------

void vidbrain_state::vidbrain_mem(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom().region("res1", 0);
	map(0x0800, 0x08ff).mirror(0x2300).rw(m_uv, FUNC(uv201_device::read), FUNC(uv201_device::write));
	map(0x0c00, 0x0fff).mirror(0x2000).ram();
	map(0x1000, 0x17ff).rw(m_exp, FUNC(videobrain_expansion_slot_device::cs1_r), FUNC(videobrain_expansion_slot_device::cs1_w));
	map(0x1800, 0x1fff).rw(m_exp, FUNC(videobrain_expansion_slot_device::cs2_r), FUNC(videobrain_expansion_slot_device::cs2_w));
	map(0x2000, 0x27ff).rom().region("res2", 0);
	map(0x3000, 0x3fff).rw(m_exp, FUNC(videobrain_expansion_slot_device::unmap_r), FUNC(videobrain_expansion_slot_device::unmap_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( vidbrain_io )
//-------------------------------------------------

void vidbrain_state::vidbrain_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(vidbrain_state::keyboard_w));
	map(0x01, 0x01).rw(FUNC(vidbrain_state::keyboard_r), FUNC(vidbrain_state::sound_w));
	map(0x0c, 0x0f).rw(m_smi, FUNC(f3853_device::read), FUNC(f3853_device::write));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( trigger_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( vidbrain_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  INPUT_PORTS( vidbrain )
//-------------------------------------------------

static INPUT_PORTS_START( vidbrain )
	PORT_START("IO00")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P ¢") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR(0x00a2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"; π") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(0x03c0)

	PORT_START("IO01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('\'') PORT_CHAR('*')

	PORT_START("IO02")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y ÷") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR(0x00f7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("IO03")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T ×") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR(0x00d7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('/')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ERASE RESTART") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("IO04")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('9')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE RUN/STOP") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("IO05")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPECIAL ALARM") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("IO06")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEXT CLOCK") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("IO07")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PREVIOUS COLOR") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("UV201-31")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('.')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK TEXT") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MASTER CONTROL") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, vidbrain_state, trigger_reset, 0)

	PORT_START("JOY1-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(1)

	PORT_START("JOY1-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(1)

	PORT_START("JOY2-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(2)

	PORT_START("JOY2-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(2)

	PORT_START("JOY3-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(3)

	PORT_START("JOY3-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(3)

	PORT_START("JOY4-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(4)

	PORT_START("JOY4-Y")
	PORT_BIT( 0xff, 70, IPT_AD_STICK_Y ) PORT_MINMAX(0, 139) PORT_SENSITIVITY(25) PORT_PLAYER(4)

	PORT_START("JOY-R")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

void vidbrain_state::hblank_w(int state)
{
	if (state && m_joy_enable)
	{
		uint8_t joydata = 0;

		if (BIT(m_keylatch, 0)) joydata |= m_joy1_x->read();
		if (BIT(m_keylatch, 1)) joydata |= m_joy1_y->read();
		if (BIT(m_keylatch, 2)) joydata |= m_joy2_x->read();
		if (BIT(m_keylatch, 3)) joydata |= m_joy2_y->read();
		if (BIT(m_keylatch, 4)) joydata |= m_joy3_x->read();
		if (BIT(m_keylatch, 5)) joydata |= m_joy3_y->read();
		if (BIT(m_keylatch, 6)) joydata |= m_joy4_x->read();
		if (BIT(m_keylatch, 7)) joydata |= m_joy4_y->read();

		// NE555 in monostable mode
		// R = 3K9 + 100K linear pot
		// C = 0.003uF
		// t = 1.1 * R * C
		double t = 1.1 * (RES_K(3.9) + RES_K(joydata)) * 3;

		m_timer_ne555->adjust(attotime::from_nsec(t));
	}
}

uint8_t vidbrain_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(vidbrain_state::joystick_tick)
{
	m_uv->ext_int_w(0);
}


//-------------------------------------------------
//  MACHINE_START( vidbrain )
//-------------------------------------------------

void vidbrain_state::machine_start()
{
	// allocate timers
	m_timer_ne555 = timer_alloc(FUNC(vidbrain_state::joystick_tick), this);

	// register for state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_joy_enable));
	save_item(NAME(m_sound_clk));
}


void vidbrain_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( vidbrain )
//-------------------------------------------------

void vidbrain_state::vidbrain(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, XTAL(4'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vidbrain_state::vidbrain_mem);
	m_maincpu->set_addrmap(AS_IO, &vidbrain_state::vidbrain_io);
	m_maincpu->set_irq_acknowledge_callback(m_smi, FUNC(f3853_device::int_acknowledge));

	// video hardware
	UV201(config, m_uv, 3636363);
	m_uv->set_screen("screen");
	m_uv->ext_int_wr_callback().set(m_smi, FUNC(f3853_device::ext_int_w));
	m_uv->hblank_wr_callback().set(FUNC(vidbrain_state::hblank_w));
	m_uv->db_rd_callback().set(FUNC(vidbrain_state::memory_read_byte));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(m_uv, FUNC(uv201_device::screen_update));
	screen.set_raw(3636363, 232, 18, 232, 262, 21, 262);
	screen.set_physical_aspect(3, 2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // 74ls74.u16 + 120k + 56k

	// devices
	F3853(config, m_smi, XTAL(4'000'000)/2);
	m_smi->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);

	// cartridge
	VIDEOBRAIN_EXPANSION_SLOT(config, m_exp, vidbrain_expansion_cards, nullptr);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("vidbrain");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("1K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vidbrain )
//-------------------------------------------------

ROM_START( vidbrain )
	ROM_REGION( 0x800, "res1", 0 )
	ROM_LOAD( "uvres_1n.d67", 0x000, 0x800, CRC(065fe7c2) SHA1(9776f9b18cd4d7142e58eff45ac5ee4bc1fa5a2a) )

	ROM_REGION( 0x800, "res2", 0 )
	ROM_LOAD( "resn2.e5", 0x000, 0x800, CRC(1d85d7be) SHA1(26c5a25d1289dedf107fa43aa8dfc14692fd9ee6) )

	ROM_REGION( 0x800, "exp1", 0 )
	ROM_LOAD( "expander1.bin", 0x0000, 0x0800, CRC(dac31abc) SHA1(e1ac7a9d654c2a70979effc744d98f21d13b4e05) )
ROM_END

} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                        FULLNAME                     FLAGS
COMP( 1978, vidbrain, 0,      0,      vidbrain, vidbrain, vidbrain_state, empty_init, "VideoBrain Computer Company", "VideoBrain FamilyComputer", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
