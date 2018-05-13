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


//-------------------------------------------------
//  interrupt_check - check interrupts
//-------------------------------------------------

void vidbrain_state::interrupt_check()
{
	int interrupt = CLEAR_LINE;

	switch (m_int_enable)
	{
	case 1:
		if (m_ext_int_latch) interrupt = ASSERT_LINE;
		break;

	case 3:
		if (m_timer_int_latch) interrupt = ASSERT_LINE;
		break;
	}

	m_maincpu->set_input_line(F8_INPUT_LINE_INT_REQ, interrupt);
}


//-------------------------------------------------
//  f3853_w - F3853 SMI write
//-------------------------------------------------

WRITE8_MEMBER( vidbrain_state::f3853_w )
{
	switch (offset)
	{
	case 0:
		// interrupt vector address high
		m_vector = (data << 8) | (m_vector & 0xff);
		logerror("%s: F3853 Interrupt Vector %04x\n", machine().describe_context(), m_vector);
		break;

	case 1:
		// interrupt vector address low
		m_vector = (m_vector & 0xff00) | data;
		logerror("%s: F3853 Interrupt Vector %04x\n", machine().describe_context(), m_vector);
		break;

	case 2:
		// interrupt control
		m_int_enable = data & 0x03;
		logerror("%s: F3853 Interrupt Control %u\n", machine().describe_context(), m_int_enable);
		interrupt_check();

		if (m_int_enable == 0x03) logerror("F3853 Timer not supported!\n");
		break;

	case 3:
		// timer 8-bit polynomial counter
		logerror("%s: F3853 Timer not supported!\n", machine().describe_context());
	}
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
	map(0x00, 0x00).w(this, FUNC(vidbrain_state::keyboard_w));
	map(0x01, 0x01).rw(this, FUNC(vidbrain_state::keyboard_r), FUNC(vidbrain_state::sound_w));
	map(0x0c, 0x0f).w(this, FUNC(vidbrain_state::f3853_w));
//  AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE(F3853_TAG, f3853_device, read, write)
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
//  f3853 interrupt request callback
//-------------------------------------------------

F3853_INTERRUPT_REQ_CB(vidbrain_state::f3853_int_req_w)
{
	m_vector = addr;
	m_maincpu->set_input_line(F8_INPUT_LINE_INT_REQ, level);
}

//-------------------------------------------------
//  UV201_INTERFACE( uv_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vidbrain_state::ext_int_w )
{
	if (state)
	{
		m_ext_int_latch = state;
		interrupt_check();
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
//      IRQ_CALLBACK_MEMBER(vidbrain_int_ack)
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(vidbrain_state::vidbrain_int_ack)
{
	uint16_t vector = m_vector;

	switch (m_int_enable)
	{
	case 1:
		vector |= 0x80;
		m_ext_int_latch = 0;
		break;

	case 3:
		vector &= ~0x80;
		m_timer_int_latch = 0;
		break;
	}

	interrupt_check();

	return vector;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void vidbrain_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_uv->ext_int_w(0);

	m_ext_int_latch = 1;
	interrupt_check();
}


//-------------------------------------------------
//  MACHINE_START( vidbrain )
//-------------------------------------------------

void vidbrain_state::machine_start()
{
	// allocate timers
	m_timer_ne555 = timer_alloc(TIMER_JOYSTICK);

	// register for state saving
	save_item(NAME(m_vector));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_ext_int_latch));
	save_item(NAME(m_timer_int_latch));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_joy_enable));
	save_item(NAME(m_sound_clk));
}


void vidbrain_state::machine_reset()
{
	m_int_enable = 0;
	m_ext_int_latch = 0;
	m_timer_int_latch = 0;
}


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( vidbrain )
//-------------------------------------------------

MACHINE_CONFIG_START(vidbrain_state::vidbrain)
	// basic machine hardware
	MCFG_DEVICE_ADD(F3850_TAG, F8, XTAL(4'000'000)/2)
	MCFG_DEVICE_PROGRAM_MAP(vidbrain_mem)
	MCFG_DEVICE_IO_MAP(vidbrain_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(vidbrain_state,vidbrain_int_ack)

	// video hardware
	MCFG_DEFAULT_LAYOUT(layout_vidbrain)

	MCFG_UV201_ADD(UV201_TAG, SCREEN_TAG, 3636363, uv_intf)
	MCFG_UV201_EXT_INT_CALLBACK(WRITELINE(*this, vidbrain_state, ext_int_w))
	MCFG_UV201_HBLANK_CALLBACK(WRITELINE(*this, vidbrain_state, hblank_w))
	MCFG_UV201_DB_CALLBACK(READ8(*this, vidbrain_state, memory_read_byte))

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MCFG_DEVICE_ADD("dac", DAC_2BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.167) // 74ls74.u16 + 120k + 56k
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	// devices
	MCFG_DEVICE_ADD(F3853_TAG, F3853, XTAL(4'000'000)/2)
	MCFG_F3853_EXT_INPUT_CB(vidbrain_state, f3853_int_req_w)

	// cartridge
	MCFG_VIDEOBRAIN_EXPANSION_SLOT_ADD(VIDEOBRAIN_EXPANSION_SLOT_TAG, vidbrain_expansion_cards, nullptr)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "vidbrain")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
MACHINE_CONFIG_END



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
