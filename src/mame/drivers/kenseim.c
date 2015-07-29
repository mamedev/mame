// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/* Ken Sei Mogura: Street Fighter II
  aka Street Fighter II Whack-a-mole game */

/*
  this game uses a CPS1 board as the 'Video board'
  the main game is driven by an additional PCB containing a TMPZ84C011 and MB89363B
  moles + hammer are physical non-video parts

  https://www.youtube.com/watch?v=wk71y7OGU-k
  https://www.youtube.com/watch?v=mV00MMyBBXM
  https://www.youtube.com/watch?v=yQpMvRL0FfM

  todo:
  - we have no way of telling the physical order of the moles on the control panel vs. the reads / writes
    so our moles could be in the wrong positions - maybe there are fixed patterns in the video we can use
    to figure it out? * the big mole appears to be worth 2 points so that one we can identify



  Additional 'DRIVE BOARD' PCB

  --------------------------------------------------------------------------------------------------------------|
  |                                                                                                             |
  |                                              CN2 (20 pin)                                                   |
  | KENSEI MOGURA                                                                                               |
  |  9401-TS280                                    62381 62381                                                  |
  | TOGO JAPAN                                                                                                  |
  |                                                                                                         C   |
  |           TC55257BFL                                                                                    N   |
  |                                                                                                         8   |
  |           PROGRAM ROM                                                                                 (8    |
  |            (label KENSEI                                                                               pin) |
  |                   MOGURA                                                                                    |
  |                   Ver1.0)                                                                               C   |
  |                                                                                                         N   |
  |                                                                                                         5   |
  |                                                                                                       (6    |
  |                                                           MB89363B                                     pin) |
  |                    TMPZ84C011-8                                                    TLP521-4                 |
  |         16.00Mhz    (rotated 180                                                                        C   |
  |                       degrees)                                                                          N   |
  |                                                                                                         4   |
  |                                                                                                       (4    |
  |                                                                                                        pin) |
  |                  DIPSW1 DIPSW2                                            TLP521-4                          |
  |                                                                                    LEDS4,3,2,1              |
  |                                                                                                         C   |
  |                                                                                                         N   |
  |    LEDS7,6,5                                                                                            1   |
  |                                              62064 62064 62064                     62064              (12   |
  |    CN9                                                                                                 pin) |
  |    (2*3 pin)  CN6 (2pin)                     CN7 (15 pin)          CN3 (14 pin)                             |
  |                                                                                                             |
  --------------------------------------------------------------------------------------------------------------|

  TMPZ84C011 - Toshiba Z80 + CTC + custom I/O, chip is rated 8MHz
  MB89363B   - Fujitsu 6-port I/O chip, basically two 8255
  TC55257BFL - Toshiba 32KB SRAM
  62381      - Toshiba TD62381F, common LED driver
  62064      - Toshiba TD62064AF, 4ch high current darlington sink driver
  TLP521-4   - Toshiba opto-isolator



Jamma (on CPS board)

(connection)     (unusual use)             (unusual use)    (connection)
--------------------------------|----|----|----------------|----------------|
GND             |GROUND         | 1  | A  | GROUND         | GND            |
GND             |GROUND         | 2  | B  | GROUND         | GND            |
+5              |+5VDC          | 3  | C  | +5VDC          | +5             |
+5              |+5VDC          | 4  | D  | +5VDC          | +5             |
n/c             |-5VDC          | 5  | E  | -5VDC          | n/c            |
+12             |+12VDC         | 6  | F  | +12VDC         | +12            |
n/c             |KEY SLOT       | 7  | H  | KEY SLOT       | n/c            |
ACK   (to CN2)  |COIN COUNTER 1 | 8  | J  | COIN COUNTER 2 | ST4 (to CN2)   |
ST2   (to CN2)  |COIN LOCK 1    | 9  | K  | COIN LOCK 2    | ST3 (to CN2)   |
SPK+            |SPEAKER +      | 10 | L  | SPEAKER -      | SPK-           |
n/c             |N/C            | 11 | M  | N/C            | n/c            |
VR              |VIDEO RED      | 12 | N  | VIDEO GREEN    | VG             |
VB              |VIDEO BLUE     | 13 | O  | VIDEO SYNC     | VSYNC          |
VGnd            |VIDEO GROUND   | 14 | R  | SERVICE SWITCH | D9 (from CN2)  |
n/c             |TEST SWITCH    | 15 | S  | TILT SWITCH    | n/c            |
n/c             |COIN 1         | 16 | T  | COIN 2         | n/c            |
REQ  (from CN2) |P1 START       | 17 | U  | P2 START       | LVm (from CN2) |
D8   (from CN2) |P1 UP          | 18 | V  | P2 UP          | D4 (from CN2)  |
D7   (from CN2) |P1 DOWN        | 19 | W  | P2 DOWN        | D3 (from CN2)  |
D6   (from CN2) |P1 LEFT        | 20 | X  | P2 LEFT        | D2 (from CN2)  |
D5   (from CN2) |P1 RIGHT       | 21 | Y  | P2 RIGHT       | D1 (from CN2)  |
n/c             |P1 B1          | 22 | Z  | P2 B1          | n/c            |
n/c             |P1 B2          | 23 | a  | P2 B2          | n/c            |
n/c             |P1 B3          | 24 | b  | P2 B3          | n/c            |
n/c             |N/C            | 25 | c  | N/C            | n/c            |
n/c             |N/C            | 26 | d  | N/C            | n/c            |
GND             |GROUND         | 27 | e  | GROUND         | GND            |
GND             |GROUND         | 28 | f  | GROUND         | GND            |
----------------|------------------------------------------|----------------|

CN1 - 12 pin connector, various cabinet inputs / outputs

2P Start Lamp | 1
1P Start Lamp | 2
Coin Lock     | 3
Coin Counter  | 4
GND           | 5
GND           | 6
Service Sw    | 7
2P Start Sw   | 8
1P Start Sw   | 9
Coin Sw       | 10
+24V          | 11
+24V          | 12

CN2 - 20 pin connector, goes from DRIVE PCB to the JAMMA on CPS board

ST4 | 1
ST3 | 2
ST2 | 3
ACK | 4
n/c | 5
LVm | 6
REQ | 7
D9  | 8
D8  | 9
D7  | 10
D6  | 11
D5  | 12
D4  | 13
D3  | 14
D2  | 15
D1  | 16
GND | 17
+12 | 18
+5  | 19
GND | 20

*/

// note: I've kept this code out of cps1.c as there is likely to be a substantial amount of game specific code here ones all the extra hardware is emulated

#include "emu.h"
#include "cpu/z80/tmpz84c011.h"
#include "machine/mb89363b.h"
#include "includes/cps1.h"
#include "kenseim.lh"

class kenseim_state : public cps_state
{
public:
	kenseim_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag),
		m_to_68k_cmd_low(0),
		m_to_68k_cmd_d9(0),
		m_to_68k_cmd_req(0),
		m_to_68k_cmd_LVm(0),
		m_from68k_ack(0),
		m_from68k_st4(0),
		m_from68k_st3(0),
		m_from68k_st2(0)

	{
		for (int i = 0; i < 6; i++)
		{
			mole_state_a[i] = 0x00;
			mole_state_b[i] = 0x00;
		}
	}

	void mole_up(int side, int mole)
	{
		if (side == 0)
			mole_state_a[mole] = 80;
		else
			mole_state_b[mole] = 80;
	}

	void mole_down(int side, int mole)
	{
		if (side == 0)
			mole_state_a[mole] = 0x00;
		else
			mole_state_b[mole] = 0x00;
	}

	void update_moles()
	{
		for (int i = 0; i < 6; i++)
		{
			char temp[32];
			sprintf(temp, "molea_%d", i);
			output_set_value(temp, mole_state_a[i]);
		}

		for (int i = 0; i < 6; i++)
		{
			char temp[32];
			sprintf(temp, "moleb_%d", i);
			output_set_value(temp, mole_state_b[i]);
		}
	}


	/* kenseim */
	DECLARE_WRITE16_MEMBER(cps1_kensei_w);
	DECLARE_DRIVER_INIT(kenseim);

	// certain

	DECLARE_WRITE8_MEMBER(mb8936_portc_w); // 20x LEDs



	// uncertain
	DECLARE_WRITE8_MEMBER(cpu_portc_w); // 4 bit out (lamps, coinlock etc.?)

	DECLARE_READ8_MEMBER(cpu_portd_r);  // 4 bit in (comms flags from 68k)

	DECLARE_WRITE8_MEMBER(cpu_portd_w); // 4 bit out (command flags to 68k?)
	DECLARE_WRITE8_MEMBER(cpu_porte_w); // 8 bit out (command to 68k?)

	WRITE8_MEMBER(mb8936_porta_w); // maybe molesa output? (6-bits?)
	WRITE8_MEMBER(mb8936_portb_w); // maybe molesb output? (6-bits?)
	WRITE8_MEMBER(mb8936_portf_w); // maybe strobe output?

	UINT8 m_to_68k_cmd_low;
	UINT8 m_to_68k_cmd_d9;
	UINT8 m_to_68k_cmd_req;
	UINT8 m_to_68k_cmd_LVm;


	int m_from68k_ack;
	int m_from68k_st4;
	int m_from68k_st3;
	int m_from68k_st2;


	DECLARE_CUSTOM_INPUT_MEMBER(kenseim_cmd_1234_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kenseim_cmd_5678_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kenseim_cmd_9_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kenseim_cmd_req_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kenseim_cmd_LVm_r);

	void set_leds(UINT32 ledstates);
	int m_led_latch;
	int m_led_serial_data;
	int m_led_clock;

	int mole_state_a[6];
	int mole_state_b[6];

};


/*******************************
  Misc System Functions
 ******************************/

void kenseim_state::set_leds(UINT32 ledstates)
{
	for (int i=0; i<20; i++)
		output_set_lamp_value(i+1, ((ledstates & (1 << i)) != 0));
}

// could be wrong
WRITE8_MEMBER(kenseim_state::mb8936_portc_w)
{
	// I'm guessing these are the 20 'power meter' LEDs, 10 for each player? (it writes 42 times, with the last write being some terminator?)

//  printf("%s mb8936 write %02x to port C but no handler assigned (serial data?)\n", machine().describe_context(), data);

	if (data & 0x08)
	{
		if (data & 0x02)
		{
			if (data & 0x04)
			{
				// send and reset? maybe?
				//printf("led write reset?\n");
				m_led_latch = 0;
				set_leds(m_led_serial_data);
				m_led_serial_data = 0;
			}
			else if (!(m_led_clock & 0x02))
			{
				//printf("write data bit %d\n", m_led_latch & 1);
				m_led_serial_data = (m_led_serial_data << 1) | (m_led_latch & 1);
			}

		}
		else
		{
			m_led_latch = data & 0x5;
			//printf("set latch %02x\n", m_led_latch);
		}

		m_led_clock = data & 0x02;
	}

}


WRITE8_MEMBER(kenseim_state::mb8936_porta_w) // maybe molesa output? (6-bits?)
{
	//if (data&0xc0) printf("%s mb8936 write %02x to port A (mole output 1?)\n", machine().describe_context(), data);


	for (int i = 0; i < 6; i++)
	{
		int bit = (data >> i) & 1;

		if (bit)
			mole_down(0, i);
		else
			mole_up(0, i);
	}

	update_moles();

}

WRITE8_MEMBER(kenseim_state::mb8936_portb_w) // maybe molesb output? (6-bits?)
{
	//if (data&0xc0) printf("%s mb8936 write %02x to port B (mole output 2?)\n", machine().describe_context(), data);

	for (int i = 0; i < 6; i++)
	{
		int bit = (data >> i) & 1;

		if (bit)
			mole_down(1, i);
		else
			mole_up(1, i);
	}

	update_moles();

}

WRITE8_MEMBER(kenseim_state::mb8936_portf_w)
{
	// typically written when the 'moles' output is, maybe the 2 strobes?
	//printf("%s mb8936 write %02x to port F (strobe?)\n", machine().describe_context(), data);
}


WRITE8_MEMBER(kenseim_state::cpu_portc_w)
{
	// port direction is set to 4-in 4-out
	// d4: coin counter
	// d5: coin lock
	// d6: left start button lamp
	// d7: right start button lamp
	coin_lockout_w(machine(), 0, (data & 0x10) ? 0 : 1); // toggles if you attempt to insert a coin when there are already 15 coins inserted
	coin_counter_w(machine(), 0, (data & 0x20) ? 0 : 1);
	output_set_value("startlamp1", (data & 0x80) ? 0 : 1);
	output_set_value("startlamp2", (data & 0x40) ? 0 : 1);
}





/*******************************
  Comms
 ******************************/

/* 68k side COMMS reads */

CUSTOM_INPUT_MEMBER(kenseim_state::kenseim_cmd_1234_r)
{
	return (m_to_68k_cmd_low & 0x0f) >> 0;
}

CUSTOM_INPUT_MEMBER(kenseim_state::kenseim_cmd_5678_r)
{
	return (m_to_68k_cmd_low & 0xf0) >> 4;
}

CUSTOM_INPUT_MEMBER(kenseim_state::kenseim_cmd_9_r)
{
	return m_to_68k_cmd_d9;
}

CUSTOM_INPUT_MEMBER(kenseim_state::kenseim_cmd_req_r)
{
	return m_to_68k_cmd_req;
}

CUSTOM_INPUT_MEMBER(kenseim_state::kenseim_cmd_LVm_r)
{
	return m_to_68k_cmd_LVm;;
}

/* 68k side COMMS writes */

WRITE16_MEMBER(kenseim_state::cps1_kensei_w)
{
	if (ACCESSING_BITS_8_15)
	{
		// NOTE: remapped from default jamma output pins:
		// coin_counter_w(machine(), 0, data & 0x0100);
		// coin_counter_w(machine(), 1, data & 0x0200);
		// coin_lockout_w(machine(), 0, ~data & 0x0400);
		// coin_lockout_w(machine(), 1, ~data & 0x0800);

		// bit 15 = CPS-A custom reset?

		m_from68k_ack = (data & 0x0100) >> 8;
		m_from68k_st4 = (data & 0x0200) >> 9;
		m_from68k_st2 = (data & 0x0400) >> 10;
		m_from68k_st3 = (data & 0x0800) >> 11;
	}
}


/* Z80 side COMMS reads */

READ8_MEMBER(kenseim_state::cpu_portd_r)
{
	// port direction is set to 4-in 4-out
	// d4: ACK
	// d5: ST2
	// d6: ST3
	// d7: ST4
	return (m_from68k_ack << 4) | (m_from68k_st2 << 5) | (m_from68k_st3 << 6) | (m_from68k_st4 << 7);
}

/* Z80 side COMMS writes */

WRITE8_MEMBER(kenseim_state::cpu_portd_w)
{
	// port direction is set to 4-in 4-out
	// d0: D9
	// d1: REQ
	// d2: LVm
	// d3: N/C
	m_to_68k_cmd_d9 = data >> 0 & 1;
	m_to_68k_cmd_req = data >> 1 & 1;
	m_to_68k_cmd_LVm = data >> 2 & 1;
}

WRITE8_MEMBER(kenseim_state::cpu_porte_w)
{
	// DT1-DT8
	m_to_68k_cmd_low = data;
}







static ADDRESS_MAP_START( kenseim_map, AS_PROGRAM, 8, kenseim_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kenseim_io_map, AS_IO, 8, kenseim_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x27) AM_DEVREADWRITE("mb89363b", mb89363b_device, read, write)
ADDRESS_MAP_END



static const z80_daisy_config daisy_chain_gamecpu[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ NULL }
};

static MACHINE_CONFIG_DERIVED_CLASS( kenseim, cps1_12MHz, kenseim_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("gamecpu", TMPZ84C011, XTAL_16MHz/2) // tmpz84c011-8
	MCFG_CPU_CONFIG(daisy_chain_gamecpu)
	MCFG_CPU_PROGRAM_MAP(kenseim_map)
	MCFG_CPU_IO_MAP(kenseim_io_map)
	MCFG_TMPZ84C011_PORTC_WRITE_CB(WRITE8(kenseim_state, cpu_portc_w))
	MCFG_TMPZ84C011_PORTD_WRITE_CB(WRITE8(kenseim_state, cpu_portd_w))
	MCFG_TMPZ84C011_PORTE_WRITE_CB(WRITE8(kenseim_state, cpu_porte_w))
	MCFG_TMPZ84C011_PORTA_READ_CB(IOPORT("DSW1"))
	MCFG_TMPZ84C011_PORTB_READ_CB(IOPORT("DSW2"))
	MCFG_TMPZ84C011_PORTC_READ_CB(IOPORT("CAB-IN"))
	MCFG_TMPZ84C011_PORTD_READ_CB(READ8(kenseim_state, cpu_portd_r))

	MCFG_MB89363B_ADD("mb89363b")
	// a,b,c always $80: all ports set as output
	// d,e,f always $92: port D and E as input, port F as output
	MCFG_MB89363B_OUT_PORTA_CB(WRITE8(kenseim_state, mb8936_porta_w))
	MCFG_MB89363B_OUT_PORTB_CB(WRITE8(kenseim_state, mb8936_portb_w))
	MCFG_MB89363B_OUT_PORTC_CB(WRITE8(kenseim_state, mb8936_portc_w))
	MCFG_MB89363B_IN_PORTD_CB(IOPORT("MOLEA"))
	MCFG_MB89363B_IN_PORTE_CB(IOPORT("MOLEB"))
	MCFG_MB89363B_OUT_PORTF_CB(WRITE8(kenseim_state, mb8936_portf_w))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")
MACHINE_CONFIG_END

static INPUT_PORTS_START( kenseim )
	// the regular CPS1 input ports are used for comms with the extra board
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED /*IPT_COIN1*/ ) // n/c
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED /*IPT_COIN2*/ ) // n/c
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, kenseim_state, kenseim_cmd_9_r, NULL) //   PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) // D9
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // n/c?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, kenseim_state, kenseim_cmd_req_r, NULL) // PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) // REQ
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, kenseim_state, kenseim_cmd_LVm_r, NULL) // PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) // LVm
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // PORT_SERVICE( 0x40, IP_ACTIVE_LOW ) n/c
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // n/c?

	PORT_START("IN1")
//  PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) // D5
//  PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) // D6
//  PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) // D7
//  PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) // D8
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, kenseim_state, kenseim_cmd_5678_r, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED/*IPT_BUTTON1*/ ) /*PORT_PLAYER(1)*/ // n/c
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED/*IPT_BUTTON2*/ ) /*PORT_PLAYER(1)*/ // n/c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED/*IPT_BUTTON3*/ ) /*PORT_PLAYER(1)*/ // n/c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // n/c?

//  PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) // D1
//  PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) // D2
//  PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) // D3
//  PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) // D4
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, kenseim_state, kenseim_cmd_1234_r, NULL)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED /*IPT_BUTTON1*/ ) /*PORT_PLAYER(2)*/ // n/c
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED /*IPT_BUTTON2*/ ) /*PORT_PLAYER(2)*/ // n/c
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED /*IPT_BUTTON3*/ ) /*PORT_PLAYER(2)*/ // n/c
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // n/c?

	// most of the regular CPS1 dips are unused
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(A):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(A):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(A):3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "CPSA SW(A):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "CPSA SW(A):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "CPSA SW(A):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(A):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(B):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(B):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(B):3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "CPSA SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "CPSA SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "CPSA SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(C):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(C):3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("CPSA SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("CPSA SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("CPSA SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(C):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(C):8" )

	// the extra board has 2 dip banks used for most game options
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )                   PORT_DIPLOCATION("DRV SW(1):1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Head Appear Once Ratio" )             PORT_DIPLOCATION("DRV SW(1):3,4")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x30, "Strength of Computer" )               PORT_DIPLOCATION("DRV SW(1):5,6")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPNAME( 0x40, 0x40, "Game Time" )                          PORT_DIPLOCATION("DRV SW(1):7")
	PORT_DIPSETTING(    0x00, "Long (59 seconds)" )
	PORT_DIPSETTING(    0x40, "Short (49 seconds)" )
	PORT_DIPNAME( 0x80, 0x80, "Winner of 2 Player faces Vega" )      PORT_DIPLOCATION("DRV SW(1):8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1 (1-bit)" )                           PORT_DIPLOCATION("DRV SW(2):1")  // manual lists unused, but see code at  0x0E9E
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPNAME( 0x06, 0x06, "Unknown 2 (2-bit)" )                           PORT_DIPLOCATION("DRV SW(2):2,3")  // ^^
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 3 (2-bit)" )                           PORT_DIPLOCATION("DRV SW(2):4,5")  // ^^
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPUNUSED( 0x20, 0x20 )                                      PORT_DIPLOCATION("DRV SW(2):6") // appears unused
	PORT_DIPUNUSED( 0x40, 0x40 )                                      PORT_DIPLOCATION("DRV SW(2):7") // apeears unused
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "DRV SW(2):8" )

	PORT_START("CAB-IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Ryu Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Chun-Li Start")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("MOLEA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_W) // big mole (2pts)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MOLEB")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD) // big mole (2pts)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? (see disasm, it gets checked but seems to have no noticeable effect in-game)
INPUT_PORTS_END

/* B-Board 91634B-2 */
ROM_START( kenseim )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "knm_23.8f", 0x000000, 0x80000, CRC(f8368900) SHA1(07a8e9fffcf7be6cb154b60a0559211bc7127c5d) )
	ROM_LOAD16_WORD_SWAP( "knm_21.6f", 0x100000, 0x80000, CRC(a8025e91) SHA1(24cd3f34ae96947a1101e5f5cb6cf0d1c1d66dc0) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "knm_01.3a",  0x000000, 0x80000, CRC(923f0c0c) SHA1(2569543ba33900d1e9c7c3981c8fe1cb40743546) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_02.4a",  0x000002, 0x80000, CRC(fa694f67) SHA1(b1ffbeaba71619e9b52f1f50abc7dafe2f3332b1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_03.5a",  0x000004, 0x80000, CRC(af7af02c) SHA1(ce2e0c696b50e4806f25fc69bf4455048c9fa396) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_04.6a",  0x000006, 0x80000, CRC(607a9af4) SHA1(78862e37c1fa727d9e36099e87ee17dfa9d2498f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_05.7a",  0x200000, 0x80000, CRC(d877eee9) SHA1(d63e123fa6c1f9927cec3cf93474f31729348fd5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_06.8a",  0x200002, 0x80000, CRC(8821a281) SHA1(216305421783baa20994eec33e26537f69f34fcb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_07.9a",  0x200004, 0x80000, CRC(00306d09) SHA1(581c4ba6f9eb3050d6bf989016532457314441e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_08.10a", 0x200006, 0x80000, CRC(4a329d16) SHA1(60d66cec8c226ef49890d8b2cd82d798dfefa049) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_10.3c",  0x400000, 0x80000, CRC(ca93a942) SHA1(1f293617e6f202054690035ebe6b6e45ffe68cc9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_11.4c",  0x400002, 0x80000, CRC(a91f3091) SHA1(7cddcd30aa6a561ce297b877611ffabfac10be28) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_12.5c",  0x400004, 0x80000, CRC(5da8303a) SHA1(de30149e323f7892bb9967a98a0d3cd9c261dc69) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_13.6c",  0x400006, 0x80000, CRC(889bb671) SHA1(c7952ed801343e79c06be8ed765a293e7322307b) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "knm_09.12a",  0x00000, 0x08000, CRC(15394dd7) SHA1(d96413cc8fa6cd3cfdafb2ab6305e41cfd2b8874) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "knm_18.11c",  0x00000, 0x20000, CRC(9e3e4773) SHA1(6e750a9610fabc4bf4964b5a754414d612d43dec) )
	ROM_LOAD( "knm_19.12c",  0x20000, 0x20000, CRC(d6c4047f) SHA1(1259a3cbfc14c348ce4bd87b5de5e97ad252f7fb) )

	ROM_REGION( 0x0200, "aboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "knm10b.1a",    0x0000, 0x0117, CRC(e40131d4) SHA1(47e9f67ecacdf1d946838815dfe7396c9c698f04) )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )

	ROM_REGION( 0x08000, "gamecpu", 0 )
	ROM_LOAD( "kensei_mogura_ver1.0.u2", 0x00000, 0x08000, CRC(725cfcfc) SHA1(5a4c6e6efe2ddb38bec3218e55a746ea0146209f) )
ROM_END

DRIVER_INIT_MEMBER(kenseim_state,kenseim)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800030, 0x800037, write16_delegate(FUNC(kenseim_state::cps1_kensei_w),this));

	DRIVER_INIT_CALL(cps1);

	m_led_serial_data = 0;
	m_led_clock = 0;
	m_led_latch = 0;
}


// 1994.04.18 is from extra PCB rom, Siguma or Sigma? (Siguma is in the ROM)
// the CPS1 board roms contain "M O G U R A   9 2 0 9 2 4" strings suggesting that part of the code was developed earlier
GAMEL( 1994, kenseim,       0,        kenseim, kenseim,      kenseim_state,   kenseim,     ROT0,   "Capcom / Togo / Sigma", "Ken Sei Mogura: Street Fighter II (Japan 940418, Ver 1.00)", MACHINE_CLICKABLE_ARTWORK, layout_kenseim )
