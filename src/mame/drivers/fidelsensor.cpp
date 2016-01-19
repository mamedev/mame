// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco
/******************************************************************************

WIP: plan to move to main fidelity chess driver^Z^Z^Z^Z - move magnet board sensor games to this driver




    Fidelity Champion Chess Challenger (model CSC)

    See drivers/fidelz80.cpp for hardware description

    TODO:
    - speech doesn't work
    - make a better artwork

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/s14001a_new.h"

// same layout of Sensory Chess Challenger
//extern const char layout_vsc[];

class csc_state : public driver_device
{
public:
	csc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speech(*this, "speech")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<s14001a_new_device> m_speech;

	virtual void machine_start() override;

	UINT16 input_read(int index);
	DECLARE_WRITE8_MEMBER( pia0_pa_w );
	DECLARE_WRITE8_MEMBER( pia0_pb_w );
	DECLARE_READ8_MEMBER( pia0_pb_r );
	DECLARE_WRITE_LINE_MEMBER( pia0_ca2_w );
	DECLARE_WRITE8_MEMBER( pia1_pa_w );
	DECLARE_WRITE8_MEMBER( pia1_pb_w );
	DECLARE_READ8_MEMBER( pia1_pa_r );
	DECLARE_WRITE_LINE_MEMBER( pia1_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_cb2_w );
	DECLARE_READ_LINE_MEMBER( pia1_ca1_r );
	DECLARE_READ_LINE_MEMBER( pia1_cb1_r );

	UINT8 m_selector;
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
};


UINT16 csc_state::input_read(int index)
{
	static const char *const col_tag[] =
	{
		"IN.0", "IN.1", "IN.2", "IN.3", "IN.4",
		"IN.5", "IN.6", "IN.7", "IN.8"
	};

	return ioport(col_tag[index])->read();
}


WRITE8_MEMBER( csc_state::pia0_pa_w )
{
	UINT8 out_digit = BITSWAP8(data,0,1,5,6,7,2,3,4 );

	switch (m_selector)
	{
	case 0:
		output().set_digit_value(0, out_digit & 0x7f);
		output().set_value("pm_led", BIT(out_digit, 7));
		break;
	case 1:
		output().set_digit_value(1, out_digit & 0x7f);
		break;
	case 2:
		output().set_digit_value(2, out_digit & 0x7f);
		output().set_value("up_dot", BIT(out_digit, 7));
		break;
	case 3:
		output().set_digit_value(3, out_digit & 0x7f);
		output().set_value("low_dot", BIT(out_digit, 7));
		break;
	}

//  m_speech->data_w(space, 0, data & 0x3f);

	// for avoid the digit flashing
	m_selector |= 0x80;
}

WRITE8_MEMBER( csc_state::pia0_pb_w )
{
//  m_speech->start_w(BIT(data, 1));
}

READ8_MEMBER( csc_state::pia0_pb_r )
{
	UINT8 data = 0x04;

	if(m_speech->busy_r())
		data |= 0x08;

	if (m_selector<9)
		if (input_read(m_selector) & 0x100)
			data |= 0x20;

	return data;
}

WRITE_LINE_MEMBER( csc_state::pia0_ca2_w )
{
}

WRITE8_MEMBER( csc_state::pia1_pa_w )
{
	m_selector = (m_selector & 0x0c) | ((data>>6) & 0x03);
}

WRITE8_MEMBER( csc_state::pia1_pb_w )
{
	static const char *const row_tag[] =
	{
		"led_a", "led_b", "led_c", "led_d",
		"led_e", "led_f", "led_g", "led_h"
	};

	if (m_selector < 8)
		for (int i=0; i<8; i++)
			output().set_indexed_value(row_tag[m_selector], i+1, BIT(data, 7-i));
}

READ8_MEMBER( csc_state::pia1_pa_r )
{
	UINT8 data = 0xff;

	if (m_selector<9)
		data = input_read(m_selector);

	return data & 0x3f;
}

WRITE_LINE_MEMBER( csc_state::pia1_ca2_w )
{
	m_selector = (m_selector & 0x07) | (state ? 8 : 0);
}

WRITE_LINE_MEMBER( csc_state::pia1_cb2_w )
{
	m_selector = (m_selector & 0x0b) | (state ? 4 : 0);
}

READ_LINE_MEMBER( csc_state::pia1_ca1_r )
{
	int data = 0x01;

	if (m_selector<9)
		data = BIT(input_read(m_selector), 6);

	return data;
}

READ_LINE_MEMBER( csc_state::pia1_cb1_r )
{
	int data = 0x01;

	if (m_selector<9)
		data = BIT(input_read(m_selector),7);

	return data;
}


TIMER_DEVICE_CALLBACK_MEMBER(csc_state::irq_timer)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
}

/* Address maps */
static ADDRESS_MAP_START(csc_mem, AS_PROGRAM, 8, csc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_RAM AM_MIRROR(0x4000)  //2K RAM
	AM_RANGE( 0x0800, 0x0bff) AM_RAM AM_MIRROR(0x4400)  //1K RAM
	AM_RANGE( 0x1000, 0x1003) AM_DEVREADWRITE("pia0", pia6821_device, read, write) AM_MIRROR(0x47fc)
	AM_RANGE( 0x1800, 0x1803) AM_DEVREADWRITE("pia1", pia6821_device, read, write) AM_MIRROR(0x47fc)
	AM_RANGE( 0x2000, 0x3fff) AM_ROM  AM_MIRROR(0x4000)
	AM_RANGE( 0x8000, 0x9fff) AM_NOP
	AM_RANGE( 0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( csc )
	PORT_START("IN.0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Speak") PORT_CODE(KEYCODE_SPACE)
	PORT_START("IN.1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RV")   PORT_CODE(KEYCODE_V)
	PORT_START("IN.2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TM")   PORT_CODE(KEYCODE_T)
	PORT_START("IN.3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LV")   PORT_CODE(KEYCODE_L)
	PORT_START("IN.4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DM")   PORT_CODE(KEYCODE_M)
	PORT_START("IN.5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("ST")   PORT_CODE(KEYCODE_S)
	PORT_START("IN.6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
	PORT_START("IN.7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
	PORT_START("IN.8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pawn")     PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Rook")     PORT_CODE(KEYCODE_2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Knight")   PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bishop")   PORT_CODE(KEYCODE_4)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Queen")    PORT_CODE(KEYCODE_5)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("King")     PORT_CODE(KEYCODE_6)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL")       PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RE")       PORT_CODE(KEYCODE_R)
		PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_UNUSED) PORT_UNUSED
INPUT_PORTS_END


void csc_state::machine_start()
{
	save_item(NAME(m_selector));
}

/* Machine driver */
static MACHINE_CONFIG_START( csc, csc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3900000/2)
	MCFG_CPU_PROGRAM_MAP(csc_mem)

	//MCFG_DEFAULT_LAYOUT(layout_vsc)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", csc_state, irq_timer, attotime::from_hz(38400/64))

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(csc_state, pia0_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(csc_state, pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(csc_state, pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(csc_state, pia0_ca2_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(csc_state, pia1_pa_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(csc_state, pia1_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(csc_state, pia1_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(csc_state, pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(csc_state, pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(csc_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(csc_state, pia1_cb2_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A_NEW, 25000) // around 25khz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START(csc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341))
	ROM_LOAD("1025a03.bin", 0xa000,  0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c))
	ROM_LOAD("1025a02.bin", 0xc000,  0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0))
	ROM_LOAD("1025a01.bin", 0xe000,  0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b))

	ROM_LOAD("74s474.bin", 0xfe00,  0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("101-32107.bin", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d))
ROM_END

ROM_START( fexcelv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x0000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) )

	ROM_REGION( 0x8000, "speech", 0 )
	ROM_LOAD("101-1081a01.ic2", 0x0000, 0x8000, CRC(c8ae1607) SHA1(6491ce6be60ed77f3dd931c0ca17616f13af943e) )
ROM_END

/* Driver */

/*    YEAR  NAME          PARENT  COMPAT  MACHINE    INPUT       INIT      COMPANY  FULLNAME                     FLAGS */
COMP( 1981, csc,     0,      0,      csc,  csc, driver_device,   0, "Fidelity Electronics", "Champion Chess Challenger (model CSC)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK)

COMP( 1987, fexcelv,     0,      0,      csc,  csc, driver_device,   0, "Fidelity Electronics", "Voice Excellence (model 6092)", MACHINE_NOT_WORKING )
