// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Electro-mechanical bubble hockey games:

- Chexx (1983 version) by ICE
  http://www.pinrepair.com/arcade/chexx.htm

- Face-Off, an illegal? copy of Chexx
  http://valker.us/gameroom/SegaFaceOff.htm
  https://casetext.com/case/innovative-concepts-in-ent-v-entertainment-enter

(Some sources indicate these may have been copied from a earlier Sega game called Face-Off)

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/digitalk.h"
#include "machine/6522via.h"
#include "chexx.lh"

#define MAIN_CLOCK XTAL_4MHz

class chexx_state : public driver_device
{
public:
	chexx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_via(*this, "via6522"),
			m_digitalker(*this, "digitalker"),
			m_aysnd(*this, "aysnd")
	{
	}

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<digitalker_device> m_digitalker;
	optional_device<ay8910_device> m_aysnd; // only faceoffh

	// vars
	UINT8  m_port_a, m_port_b;
	UINT8  m_bank;
	UINT32 m_shift;
	UINT8  m_lamp;
	UINT8  m_ay_cmd, m_ay_data;

	// callbacks
	TIMER_DEVICE_CALLBACK_MEMBER(update);

	// handlers
	DECLARE_READ8_MEMBER(via_a_in);
	DECLARE_READ8_MEMBER(via_b_in);

	DECLARE_WRITE8_MEMBER(via_a_out);
	DECLARE_WRITE8_MEMBER(via_b_out);

	DECLARE_WRITE_LINE_MEMBER(via_ca2_out);
	DECLARE_WRITE_LINE_MEMBER(via_cb1_out);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_out);
	DECLARE_WRITE_LINE_MEMBER(via_irq_out);

	DECLARE_READ8_MEMBER(input_r);

	DECLARE_WRITE8_MEMBER(ay_w);
	DECLARE_WRITE8_MEMBER(lamp_w);

	// digitalker
	void digitalker_set_bank(UINT8 bank);

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();
};


// VIA

READ8_MEMBER(chexx_state::via_a_in)
{
	UINT8 ret = 0;
	logerror("%s: VIA read A: %02X\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(chexx_state::via_b_in)
{
	UINT8 ret = 0;
	logerror("%s: VIA read B: %02X\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(chexx_state::via_a_out)
{
	m_port_a = data;    // multiplexer

	m_digitalker->digitalker_data_w(space, 0, data, 0);

//  logerror("%s: VIA write A = %02X\n", machine().describe_context(), data);
}
WRITE8_MEMBER(chexx_state::via_b_out)
{
	m_port_b = data;

	digitalker_set_bank(data & 3);
	m_digitalker->set_output_gain(0, BIT(data,2) ? 1.0f : 0.0f); // bit 2 controls the Digitalker output
	coin_counter_w(machine(), 0, BIT(~data,3));
	// bit 4 is EJECT
	// bit 7 is related to speaker out

//  logerror("%s: VIA write B = %02X\n", machine().describe_context(), data);
}

WRITE_LINE_MEMBER(chexx_state::via_ca2_out)
{
	m_digitalker->digitalker_0_cms_w(CLEAR_LINE);
	m_digitalker->digitalker_0_cs_w(CLEAR_LINE);
	m_digitalker->digitalker_0_wr_w(state ? ASSERT_LINE : CLEAR_LINE);

//  logerror("%s: VIA write CA2 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(chexx_state::via_cb1_out)
{
//  logerror("%s: VIA write CB1 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(chexx_state::via_cb2_out)
{
	m_shift = ((m_shift << 1) & 0xffffff) | state;

	// 7segs (score)
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511

	output_set_digit_value(0, patterns[(m_shift >> (16+4)) & 0xf]);
	output_set_digit_value(1, patterns[(m_shift >> (16+0)) & 0xf]);

	output_set_digit_value(2, patterns[(m_shift >>  (8+4)) & 0xf]);
	output_set_digit_value(3, patterns[(m_shift >>  (8+0)) & 0xf]);

	// Leds (period being played)
	output_set_led_value(0, BIT(m_shift,2));
	output_set_led_value(1, BIT(m_shift,1));
	output_set_led_value(2, BIT(m_shift,0));

//  logerror("%s: VIA write CB2 = %02X\n", machine().describe_context(), state);
}
WRITE_LINE_MEMBER(chexx_state::via_irq_out)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
//  logerror("%s: VIA write IRQ = %02X\n", machine().describe_context(), state);
}

READ8_MEMBER(chexx_state::input_r)
{
	UINT8 ret = ioport("DSW")->read();          // bits 0-3
	UINT8 inp = ioport("INPUT")->read();        // bit 7 (multiplexed)

	for (int i = 0; i < 8; ++i)
		if ( ((~m_port_a) & (1 << i)) && ((~inp) & (1 << i)) )
			ret &= 0x7f;

	return ret;
}

// Chexx Memory Map

static ADDRESS_MAP_START( chexx83_map, AS_PROGRAM, 8, chexx_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM AM_MIRROR(0x100) // 6810 - 128 x 8 static RAM
	AM_RANGE(0x4000, 0x400f) AM_DEVREADWRITE("via6522", via6522_device, read, write)
	AM_RANGE(0x8000, 0x8000) AM_READ(input_r)
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

// Face-Off Memory Map

WRITE8_MEMBER(chexx_state::lamp_w)
{
	m_lamp = data;
	output_set_lamp_value(0, BIT(m_lamp,0));
	output_set_lamp_value(1, BIT(m_lamp,1));
}

WRITE8_MEMBER(chexx_state::ay_w)
{
	if (offset)
	{
		m_ay_data = data;
		return;
	}

	if (m_ay_cmd == 0x00 && data == 0x03)
	{
		m_aysnd->address_w(space, offset, m_ay_data, mem_mask);
//      logerror("%s: AY addr = %02X\n", machine().describe_context(), m_ay_data);
	}
	else if (m_ay_cmd == 0x00 && data == 0x02)
	{
		m_aysnd->data_w(space, offset, m_ay_data, mem_mask);
//      logerror("%s: AY data = %02X\n", machine().describe_context(), m_ay_data);
	}
	m_ay_cmd = data;
}

static ADDRESS_MAP_START( faceoffh_map, AS_PROGRAM, 8, chexx_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM AM_MIRROR(0x100) // M58725P - 2KB
	AM_RANGE(0x4000, 0x400f) AM_DEVREADWRITE("via6522", via6522_device, read, write)
	AM_RANGE(0x8000, 0x8000) AM_READ(input_r)
	AM_RANGE(0xa000, 0xa001) AM_WRITE(ay_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(lamp_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

// Inputs

static INPUT_PORTS_START( chexx83 )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(1) // play anthem
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(1) // play anthem

	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1  ) PORT_NAME("P1 Goal Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2  ) PORT_NAME("P2 Goal Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3  ) PORT_NAME("Puck Near Goal Sensors") // play "ohh" sample
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Boo Button")  // stop anthem, play "boo" sample, eject puck
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Boo Button")  // stop anthem, play "boo" sample, eject puck
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Puck Eject Ready Sensor")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Game Duration (mins)" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "2" ) // 40
	PORT_DIPSETTING(    0x04, "3" ) // 60
	PORT_DIPSETTING(    0x08, "4" ) // 80
	PORT_DIPSETTING(    0x0c, "5" ) // 100
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) // multiplexed inputs
INPUT_PORTS_END

// Machine

void chexx_state::machine_start()
{
}

void chexx_state::digitalker_set_bank(UINT8 bank)
{
	if (m_bank != bank)
	{
		UINT8 *src = memregion("samples")->base();
		UINT8 *dst = memregion("digitalker")->base();

		memcpy(dst, src + bank * 0x4000, 0x4000);

		m_bank = bank;
	}
}

void chexx_state::machine_reset()
{
	m_bank = -1;
	digitalker_set_bank(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(chexx_state::update)
{
	// NMI on coin-in
	UINT8 coin = (~ioport("COIN")->read()) & 0x03;
	m_maincpu->set_input_line(INPUT_LINE_NMI, coin ? ASSERT_LINE : CLEAR_LINE);

	// VIA CA1 connected to Digitalker INTR line
	m_via->write_ca1(m_digitalker->digitalker_0_intr_r());

#if 0
	// Play the digitalker samples (it's not hooked up correctly yet)
	static UINT8 sample = 0, bank = 0;

	if (machine().input().code_pressed_once(KEYCODE_Q))
		--bank;
	if (machine().input().code_pressed_once(KEYCODE_W))
		++bank;
	bank %= 3;
	digitalker_set_bank(bank);

	if (machine().input().code_pressed_once(KEYCODE_A))
		--sample;
	if (machine().input().code_pressed_once(KEYCODE_S))
		++sample;

	if (machine().input().code_pressed_once(KEYCODE_Z))
	{
		m_digitalker->digitalker_0_cms_w(CLEAR_LINE);
		m_digitalker->digitalker_0_cs_w(CLEAR_LINE);

		address_space &space = m_maincpu->space(AS_PROGRAM);
		m_digitalker->digitalker_data_w(space, 0, sample, 0);

		m_digitalker->digitalker_0_wr_w(ASSERT_LINE);
		m_digitalker->digitalker_0_wr_w(CLEAR_LINE);
		m_digitalker->digitalker_0_wr_w(ASSERT_LINE);
	}
#endif
}

static MACHINE_CONFIG_START( chexx83, chexx_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M6502, MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(chexx83_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("update", chexx_state, update, attotime::from_hz(60))

	// via
	MCFG_DEVICE_ADD("via6522", VIA6522, MAIN_CLOCK/4)

	MCFG_VIA6522_READPA_HANDLER(READ8(chexx_state, via_a_in))
	MCFG_VIA6522_READPB_HANDLER(READ8(chexx_state, via_b_in))

	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(chexx_state, via_a_out))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(chexx_state, via_b_out))

	MCFG_VIA6522_CA2_HANDLER(WRITELINE(chexx_state, via_ca2_out))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(chexx_state, via_cb1_out))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(chexx_state, via_cb2_out))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(chexx_state, via_irq_out))

	// Layout
	MCFG_DEFAULT_LAYOUT(layout_chexx)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DIGITALKER_ADD("digitalker", MAIN_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.16)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( faceoffh, chexx83 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(faceoffh_map)

	MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

// ROMs

/***************************************************************************

Chexx Hockey (1983 version 1.1)

The "long and skinny" Moog CPU board used a 6502 for the processor,
a 6522 for the PIA, a 6810 static RAM, eight 52164 64k bit sound ROM chips,
a 40 pin 54104 sound chip, and a single 2716 CPU EPROM

***************************************************************************/

ROM_START( chexx83 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "chexx83.u4", 0x0000, 0x0800, CRC(a34abac1) SHA1(75a31670eb6d1b62ba984f0bac7c6e6067f6ae87) )

	ROM_REGION( 0x4000, "digitalker", ROMREGION_ERASE00 )
	// bank switched (from samples region)

	ROM_REGION( 0x10000, "samples", ROMREGION_ERASE00 )
	ROM_LOAD( "chexx83.u12", 0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u13", 0x2000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u14", 0x4000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u15", 0x6000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u16", 0x8000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u17", 0xa000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u18", 0xc000, 0x2000, NO_DUMP )
	ROM_LOAD( "chexx83.u19", 0xe000, 0x2000, NO_DUMP )
ROM_END

/***************************************************************************

Face-Off PCB?

Entertainment Enterprises Ltd. 1983 (sticker)
Serial No. 025402 (sticker)
MADE IN JAPAN (etched)

CPU:     R6502P
RAM:     M58725P (2KB)
I/O:     R6522P (VIA)
Samples: Digitalker (MM54104)
Music:   AY-3-8910
Misc:    XTAL 4MHz, DSW4, 42-pin connector

***************************************************************************/

ROM_START( faceoffh )
	ROM_REGION( 0x1000, "maincpu", 0 )
	// "Copyright (c) 1983  SoftLogic JAPAN"
	ROM_LOAD( "1.5d", 0x0000, 0x1000, CRC(6ab050be) SHA1(ebecae855e22e9c3c46bdee51f84fd5352bf191a) )

	ROM_REGION( 0x4000, "digitalker", ROMREGION_ERASE00 )
	// bank switched (from samples region)

	ROM_REGION( 0x10000, "samples", 0 )
	ROM_LOAD( "9.2a", 0x0000, 0x2000, CRC(059b3725) SHA1(5837bee1ef34ce19a3101b851ca55029776e4b3e) )    // digitalker header
	ROM_LOAD( "8.2b", 0x2000, 0x2000, CRC(679da4e1) SHA1(01a5b9dd132c1b0de97c153d7de226f5bf357338) )

	ROM_LOAD( "7.2c", 0x4000, 0x2000, CRC(f8461b33) SHA1(717a8842e0ce9ba94dd59504a324bede4844e389) )    // digitalker header
	ROM_LOAD( "6.2d", 0x6000, 0x2000, CRC(156c91e0) SHA1(6017d4b5609b214a6e66dcd76493a7d1442c04d4) )

	ROM_LOAD( "5.3a", 0x8000, 0x2000, CRC(19904604) SHA1(633c211a9a822cdf597a6f3c221ae9c8d6482e82) )    // digitalker header
	ROM_LOAD( "4.3b", 0xa000, 0x2000, CRC(c3386d51) SHA1(7882e88db55ba914be81075e4b2d76e246c34d3b) )

	ROM_FILL(         0xc000, 0x2000, 0xff ) // unpopulated
	ROM_FILL(         0xe000, 0x2000, 0xff ) // unpopulated
ROM_END

GAME( 1983, chexx83,  0,       chexx83,  chexx83, driver_device, 0, ROT0, "ICE",                                                 "Chexx (EM Bubble Hockey, 1983 1.1)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME( 1983, faceoffh, chexx83, faceoffh, chexx83, driver_device, 0, ROT0, "SoftLogic (Entertainment Enterprises, Ltd. license)", "Face-Off (EM Bubble Hockey)",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
