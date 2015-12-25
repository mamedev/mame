// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Wildfire, by Bob and Holly Doyle (prototype), and Garry Kitchen
  * AMI S2150, labeled C10641

  This is an electronic handheld pinball game. It has dozens of small leds
  to create the illusion of a moving ball, and even the flippers are leds.
  A drawing of a pinball table is added as overlay.

  NOTE!: MAME external artwork is required


  TODO:
  - sound emulation could still be improved
  - when the game strobes a led faster, it should appear brighter, for example when
    the ball hits one of the bumpers
  - 7seg decoder is guessed
  - MCU clock is unknown

***************************************************************************/

#include "emu.h"
#include "cpu/amis2000/amis2000.h"
#include "sound/speaker.h"

#include "wildfire.lh" // this is a test layout, use external artwork

// master clock is a single stage RC oscillator: R=?K, C=?pf,
// S2000 default frequency is 850kHz
#define MASTER_CLOCK (850000)


class wildfire_state : public driver_device
{
public:
	wildfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_a12_decay_timer(*this, "a12_decay")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<timer_device> m_a12_decay_timer;

	UINT8 m_d;
	UINT16 m_a;
	UINT8 m_q2;
	UINT8 m_q3;

	UINT16 m_display_state[0x10];
	UINT16 m_display_cache[0x10];
	UINT8 m_display_decay[0x100];

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE16_MEMBER(write_a);
	DECLARE_WRITE_LINE_MEMBER(write_f);

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	bool index_is_7segled(int index);
	void display_update();

	TIMER_DEVICE_CALLBACK_MEMBER(reset_q2);
	void write_a12(int state);
	void sound_update();

	virtual void machine_start() override;
};



/***************************************************************************

  LED Display

***************************************************************************/

// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time, in steps of 1ms
#define DISPLAY_DECAY_TIME 40

inline bool wildfire_state::index_is_7segled(int index)
{
	// first 3 A are 7segleds
	return (index < 3);
}

// lamp translation table: Lzz from patent US4334679 FIG.4 = MAME lampxxy,
// where xx is led column and y is led row, eg. lamp103 is output A10 D3
// (note: 2 mistakes in the patent: the L19 between L12 and L14 should be L13, and L84 should of course be L48)
/*
    L0  = -         L10 = lamp60    L20 = lamp41    L30 = lamp53    L40 = lamp57    L50 = lamp110
    L1  = lamp107   L11 = lamp50    L21 = lamp42    L31 = lamp43    L41 = lamp66    L51 = lamp111
    L2  = lamp106   L12 = lamp61    L22 = lamp52    L32 = lamp54    L42 = lamp76    L52 = lamp112
    L3  = lamp105   L13 = lamp71    L23 = lamp63    L33 = lamp55    L43 = lamp86    L53 = lamp113
    L4  = lamp104   L14 = lamp81    L24 = lamp73    L34 = lamp117   L44 = lamp96    L60 = lamp30
    L5  = lamp103   L15 = lamp92    L25 = lamp115   L35 = lamp75    L45 = lamp67    L61 = lamp30(!)
    L6  = lamp102   L16 = lamp82    L26 = lamp93    L36 = lamp95    L46 = lamp77    L62 = lamp31
    L7  = lamp101   L17 = lamp72    L27 = lamp94    L37 = lamp56    L47 = lamp87    L63 = lamp31(!)
    L8  = lamp80    L18 = lamp114   L28 = lamp84    L38 = lamp65    L48 = lamp97    L70 = lamp33
    L9  = lamp70    L19 = lamp51    L29 = lamp116   L39 = lamp85    L49 = -
*/

void wildfire_state::display_update()
{
	UINT16 active_state[0x10];

	for (int i = 0; i < 0x10; i++)
	{
		// update current state
		m_display_state[i] = (m_a >> i & 1) ? m_d : 0;

		active_state[i] = 0;

		for (int j = 0; j < 0x10; j++)
		{
			int di = j << 4 | i;

			// turn on powered segments
			if (m_display_state[i] >> j & 1)
				m_display_decay[di] = DISPLAY_DECAY_TIME;

			// determine active state
			int ds = (m_display_decay[di] != 0) ? 1 : 0;
			active_state[i] |= (ds << j);
		}
	}

	// on difference, send to output
	for (int i = 0; i < 0x10; i++)
		if (m_display_cache[i] != active_state[i])
		{
			if (index_is_7segled(i))
				output_set_digit_value(i, BITSWAP8(active_state[i],7,0,1,2,3,4,5,6) & 0x7f);

			for (int j = 0; j < 8; j++)
				output_set_lamp_value(i*10 + j, active_state[i] >> j & 1);
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(wildfire_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int i = 0; i < 0x100; i++)
		if (!(m_display_state[i & 0xf] >> (i>>4) & 1) && m_display_decay[i])
			m_display_decay[i]--;

	display_update();
}



/***************************************************************************

  Sound

***************************************************************************/

// Sound output is via a speaker between transistors Q2(from A12) and Q3(from F_out)
// A12 to Q2 has a little electronic circuit going, causing a slight delay.
// (see patent US4334679 FIG.5, the 2 resistors are 10K and the cap is a 4.7uF electrolytic)

// decay time, in steps of 1ms
#define A12_DECAY_TIME 5 /* a complete guess */

void wildfire_state::sound_update()
{
	m_speaker->level_w(m_q2 & m_q3);
}

WRITE_LINE_MEMBER(wildfire_state::write_f)
{
	// F_out pin: speaker out
	m_q3 = (state) ? 1 : 0;
	sound_update();
}

TIMER_DEVICE_CALLBACK_MEMBER(wildfire_state::reset_q2)
{
	m_q2 = 0;
	sound_update();
}

void wildfire_state::write_a12(int state)
{
	if (state)
	{
		m_a12_decay_timer->adjust(attotime::never);
		m_q2 = state;
		sound_update();
	}
	else if (m_a >> 12 & 1)
	{
		// falling edge
		m_a12_decay_timer->adjust(attotime::from_msec(A12_DECAY_TIME));
	}
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(wildfire_state::write_d)
{
	// D0-D7: leds out
	m_d = data;
	display_update();
}

WRITE16_MEMBER(wildfire_state::write_a)
{
	data ^= 0x1fff; // active-low

	// A12: enable speaker
	write_a12(data >> 12 & 1);

	// A0-A2: select 7segleds
	// A3-A11: select other leds
	m_a = data;
	display_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wildfire )
	PORT_START("IN1") // I
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shooter Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void wildfire_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, 0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));

	m_d = 0;
	m_a = 0;
	m_q2 = 0;
	m_q3 = 0;

	// register for savestates
	save_item(NAME(m_display_state));
	save_item(NAME(m_display_cache));
	save_item(NAME(m_display_decay));

	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_q2));
	save_item(NAME(m_q3));
}

// LED segments A-G
enum
{
	lA = 0x40,
	lB = 0x20,
	lC = 0x10,
	lD = 0x08,
	lE = 0x04,
	lF = 0x02,
	lG = 0x01
};

static const UINT8 wildfire_7seg_table[0x10] =
{
	0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, // 0-9 unaltered
	0x77,           // A -> unused?
	lA+lB+lE+lF+lG, // b -> P
	0x4e,           // C -> unused?
	lD+lE+lF,       // d -> L
	0x4f,           // E -> unused?
	lG              // F -> -
};


static MACHINE_CONFIG_START( wildfire, wildfire_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", AMI_S2152, MASTER_CLOCK)
	MCFG_AMI_S2000_7SEG_DECODER(wildfire_7seg_table)
	MCFG_AMI_S2000_READ_I_CB(IOPORT("IN1"))
	MCFG_AMI_S2000_WRITE_D_CB(WRITE8(wildfire_state, write_d))
	MCFG_AMI_S2000_WRITE_A_CB(WRITE16(wildfire_state, write_a))
	MCFG_AMI_S2152_FOUT_CB(WRITELINE(wildfire_state, write_f))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", wildfire_state, display_decay_tick, attotime::from_msec(1))
	MCFG_TIMER_DRIVER_ADD("a12_decay", wildfire_state, reset_q2)

	MCFG_DEFAULT_LAYOUT(layout_wildfire)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wildfire )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "us4341385", 0x0000, 0x0400, CRC(84ac0f1f) SHA1(1e00ddd402acfc2cc267c34eed4b89d863e2144f) ) // from patent US4334679, data should be correct (it included checksums)
	ROM_CONTINUE(          0x0600, 0x0200 )
ROM_END


CONS( 1979, wildfire, 0, 0, wildfire, wildfire, driver_device, 0, "Parker Brothers", "Wildfire (patent)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK ) // note: pretty sure that it matches the commercial release
