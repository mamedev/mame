// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at September 2000

chess champion mk II

MOS MPS 6504 2179
MOS MPS 6530 024 1879
 layout of 6530 dumped with my adapter
 0x1300-0x133f io
 0x1380-0x13bf ram
 0x1400-0x17ff rom

2x2111 ram (256x4?)
MOS MPS 6332 005 2179
74145 bcd to decimal encoder (10 low active select lines)
(7400)

4x 7 segment led display (each with dot)
4 single leds
21 keys


83, 84 contains display variables


port a
   0..7 led output
   0..6 keyboard input

port b
   0..5 outputs
   0 speaker out
   6 as chipselect used!?
   7 interrupt out?

   c4, c5, keyboard polling
   c0, c1, c2, c3 led output

Usage:

   under the black keys are operations to be added as first sign
   black and white box are only changing the player

   for the computer to start as white
    switch to black (h enter)
    swap players (g enter)

******************************************************************************/

#include "emu.h"
#include "machine/mos6530.h"
#include "cpu/m6502/m6504.h"
#include "sound/speaker.h"
#include "mk2.lh"


class mk2_state : public driver_device
{
public:
	mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_speaker(*this, "speaker"),
	m_miot(*this, "miot")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<mos6530_device> m_miot;
	DECLARE_READ8_MEMBER(mk2_read_a);
	DECLARE_WRITE8_MEMBER(mk2_write_a);
	DECLARE_READ8_MEMBER(mk2_read_b);
	DECLARE_WRITE8_MEMBER(mk2_write_b);
	UINT8 m_led[5];
	virtual void machine_start() override;
	TIMER_DEVICE_CALLBACK_MEMBER(update_leds);
};


// only lower 12 address bits on bus!
static ADDRESS_MAP_START(mk2_mem , AS_PROGRAM, 8, mk2_state)
	AM_RANGE( 0x0000, 0x01ff) AM_RAM // 2 2111, should be mirrored
	AM_RANGE( 0x0b00, 0x0b0f) AM_DEVREADWRITE("miot", mos6530_device, read, write)
	AM_RANGE( 0x0b80, 0x0bbf) AM_RAM // rriot ram
	AM_RANGE( 0x0c00, 0x0fff) AM_ROM // rriot rom
	AM_RANGE( 0x1000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( mk2 )
	PORT_START("EXTRA")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NEW GAME") PORT_CODE(KEYCODE_F3) // seems to be direct wired to reset
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_START("BLACK")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black A    Black") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black B    Field") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black C    Time?") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black D    Time?") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black E    Time off?") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black F    LEVEL") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black G    Swap") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Black H    White") PORT_CODE(KEYCODE_H)
	PORT_START("WHITE")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("White 8") PORT_CODE(KEYCODE_8)
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(mk2_state::update_leds)
{
	int i;

	for (i=0; i<4; i++)
		output().set_digit_value(i, m_led[i]);

	output().set_led_value(0, BIT(m_led[4], 3));
	output().set_led_value(1, BIT(m_led[4], 5));
	output().set_led_value(2, BIT(m_led[4], 4));
	output().set_led_value(3, BIT(m_led[4], 4) ? 0 : 1);

	m_led[0]= m_led[1]= m_led[2]= m_led[3]= m_led[4]= 0;
}

void mk2_state::machine_start()
{
}

READ8_MEMBER( mk2_state::mk2_read_a )
{
	int data=0xff;
	int help=ioport("BLACK")->read() | ioport("WHITE")->read(); // looks like white and black keys are the same!

	switch (m_miot->portb_out_get()&0x7)
	{
	case 4:
		if (BIT(help, 5)) data&=~0x1; //F
		if (BIT(help, 4)) data&=~0x2; //E
		if (BIT(help, 3)) data&=~0x4; //D
		if (BIT(help, 2)) data&=~0x8; // C
		if (BIT(help, 1)) data&=~0x10; // B
		if (BIT(help, 0)) data&=~0x20; // A
		break;
	case 5:
		if (BIT(ioport("EXTRA")->read(), 2)) data&=~0x8; // Enter
		if (BIT(ioport("EXTRA")->read(), 1)) data&=~0x10; // Clear
		if (BIT(help, 7)) data&=~0x20; // H
		if (BIT(help, 6)) data&=~0x40; // G
		break;
	}
	return data;
}

WRITE8_MEMBER( mk2_state::mk2_write_a )
{
	UINT8 temp = m_miot->portb_out_get();

	m_led[temp & 3] |= data;
}


READ8_MEMBER( mk2_state::mk2_read_b )
{
	return 0xff&~0x40; // chip select mapped to pb6
}


WRITE8_MEMBER( mk2_state::mk2_write_b )
{
	if ((data & 0x06) == 0x06)
			m_speaker->level_w(BIT(data, 0));

	m_led[4]|=data;

	m_maincpu->set_input_line(M6502_IRQ_LINE, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE );
}


static MACHINE_CONFIG_START( mk2, mk2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6504, 1000000)
	MCFG_CPU_PROGRAM_MAP(mk2_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mk2)

	MCFG_DEVICE_ADD("miot", MOS6530, 1000000)
	MCFG_MOS6530_IN_PA_CB(READ8(mk2_state, mk2_read_a))
	MCFG_MOS6530_OUT_PA_CB(WRITE8(mk2_state, mk2_write_a))
	MCFG_MOS6530_IN_PB_CB(READ8(mk2_state, mk2_read_b))
	MCFG_MOS6530_OUT_PB_CB(WRITE8(mk2_state, mk2_write_b))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("led_timer", mk2_state, update_leds, attotime::from_hz(60))
MACHINE_CONFIG_END


ROM_START(ccmk2)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("024_1879", 0x0c00, 0x0400, CRC(4f28c443) SHA1(e33f8b7f38e54d7a6e0f0763f2328cc12cb0eade))
	ROM_LOAD("005_2179", 0x1000, 0x1000, CRC(6f10991b) SHA1(90cdc5a15d9ad813ad20410f21081c6e3e481812)) // chess mate 7.5
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/


/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT  CLASS            INIT    COMPANY               FULLNAME */
CONS( 1979, ccmk2,    0,      0,      mk2,    mk2, driver_device,    0, "Quelle International", "Chess Champion MK II", 0)
// second design sold (same computer/program?)
