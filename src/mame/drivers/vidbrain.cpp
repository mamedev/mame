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
    - use machine/f3853.h
    - joystick scan timer 555
    - expander 1 (F3870 CPU, cassette, RS-232)
    - expander 2 (modem)

*/

#include "emu.h"
#include "includes/vidbrain.h"

#include "machine/rescap.h"
#include "sound/volt_reg.h"
#include "softlist.h"
#include "speaker.h"

#include "vidbrain.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG         1



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  keyboard_w - keyboard column write
//-------------------------------------------------

WRITE8_MEMBER( vidbrain_state::keyboard_w )
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

	if (LOG) logerror("Keyboard %02x\n", data);

	m_keylatch = data;
}


//-------------------------------------------------
//  keyboard_r - keyboard row read
//-------------------------------------------------

READ8_MEMBER( vidbrain_state::keyboard_r )
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

WRITE8_MEMBER( vidbrain_state::sound_w )
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

	if (LOG) logerror("Sound %02x\n", data);

	// sound clock
	int sound_clk = BIT(data, 4);

	if (!m_sound_clk && sound_clk)
	{
		m_dac->write(m_keylatch & 3);
	}

	m_sound_clk = sound_clk;

	// joystick enable
	m_joy_enable = BIT(data, 7);
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
	map(0x0c, 0x0f).rw(F3853_TAG, FUNC(f3853_device::read), FUNC(f3853_device::write));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( trigger_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( vidbrain_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  INPUT_PORTS( vidbrain )
//-------------------------------------------------

static INPUT_PORTS_START( vidbrain )
	PORT_START("IO00")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P \xC2\xA2") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR(0x00a2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; \xC2\xB6") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(0x00b6)

	PORT_START("IO01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('\'') PORT_CHAR('*')

	PORT_START("IO02")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y \xC3\xB7") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR(0x00f7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("IO03")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T \xC3\x97") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR(0x00d7)
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPECIAL ALARM") PORT_CODE(KEYCODE_F4)

	PORT_START("IO06")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEXT CLOCK") PORT_CODE(KEYCODE_F3)

	PORT_START("IO07")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PREVIOUS COLOR") PORT_CODE(KEYCODE_F2)

	PORT_START("UV201-31")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('.')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK TEXT") PORT_CODE(KEYCODE_F1)

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

//-------------------------------------------------
//  UV201_INTERFACE( uv_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vidbrain_state::ext_int_w )
{
	if (state)
	{
		m_smi->ext_int_w(1);
	}
}

WRITE_LINE_MEMBER( vidbrain_state::hblank_w )
{
	if (state && m_joy_enable && !m_timer_ne555->enabled())
	{
		uint8_t joydata = 0;

		if (!BIT(m_keylatch, 0)) joydata = m_joy1_x->read();
		if (!BIT(m_keylatch, 1)) joydata = m_joy1_y->read();
		if (!BIT(m_keylatch, 2)) joydata = m_joy2_x->read();
		if (!BIT(m_keylatch, 3)) joydata = m_joy2_y->read();
		if (!BIT(m_keylatch, 4)) joydata = m_joy3_x->read();
		if (!BIT(m_keylatch, 5)) joydata = m_joy3_y->read();
		if (!BIT(m_keylatch, 6)) joydata = m_joy4_x->read();
		if (!BIT(m_keylatch, 7)) joydata = m_joy4_y->read();

		// NE555 in monostable mode
		// R = 3K9 + 100K linear pot
		// C = 0.003uF
		// t = 1.1 * R * C
		double t = 1.1 * (RES_K(3.9) + RES_K(joydata)) * 3;

		timer_set(attotime::from_nsec(t), TIMER_JOYSTICK);
	}
}

READ8_MEMBER(vidbrain_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void vidbrain_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_uv->ext_int_w(0);
	m_smi->ext_int_w(0);
}


//-------------------------------------------------
//  MACHINE_START( vidbrain )
//-------------------------------------------------

void vidbrain_state::machine_start()
{
	// allocate timers
	m_timer_ne555 = timer_alloc(TIMER_JOYSTICK);

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
	m_maincpu->set_irq_acknowledge_callback(F3853_TAG, FUNC(f3853_device::int_acknowledge));

	// video hardware
	config.set_default_layout(layout_vidbrain);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(UV201_TAG, FUNC(uv201_device::screen_update));
	screen.set_raw(3636363, 232, 18, 232, 262, 21, 262);
	UV201(config, m_uv, 3636363);
	m_uv->set_screen(SCREEN_TAG);
	m_uv->ext_int_wr_callback().set(FUNC(vidbrain_state::ext_int_w));
	m_uv->hblank_wr_callback().set(FUNC(vidbrain_state::hblank_w));
	m_uv->db_rd_callback().set(FUNC(vidbrain_state::memory_read_byte));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.167); // 74ls74.u16 + 120k + 56k
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

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
	ROM_LOAD( "uvres 1n.d67", 0x000, 0x800, CRC(065fe7c2) SHA1(9776f9b18cd4d7142e58eff45ac5ee4bc1fa5a2a) )

	ROM_REGION( 0x800, "res2", 0 )
	ROM_LOAD( "resn2.e5", 0x000, 0x800, CRC(1d85d7be) SHA1(26c5a25d1289dedf107fa43aa8dfc14692fd9ee6) )

	ROM_REGION( 0x800, F3870_TAG, 0 )
	ROM_LOAD( "expander1.bin", 0x0000, 0x0800, CRC(dac31abc) SHA1(e1ac7a9d654c2a70979effc744d98f21d13b4e05) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                        FULLNAME                     FLAGS
COMP( 1977, vidbrain, 0,      0,      vidbrain, vidbrain, vidbrain_state, empty_init, "VideoBrain Computer Company", "VideoBrain FamilyComputer", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
