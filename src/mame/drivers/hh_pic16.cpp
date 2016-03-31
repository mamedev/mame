// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  GI PIC16xx-driven dedicated handhelds or other simple devices.

  known chips:

  serial  device  etc.
-----------------------------------------------------------
 @036     1655A   1979, Ideal Maniac
 *061     1655A   1980, Lakeside Le Boom (have dump)
 *094     1655A   1980, GAF Melody Madness (have dump)
 *110     1650A   1979, Tiger Rocket Pinball
 *192     1650    19??, (a phone dialer, have dump)
 *255     1655    19??, (a talking clock, have dump)

  (* denotes not yet emulated by MAME, @ denotes it's in this driver)


  TODO:
  - it doesn't work, emu/cpu/pic16c5x needs some love (dev note: hack to make it
    playable: remove the TRIS masks)

***************************************************************************/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/speaker.h"

#include "maniac.lh"


class hh_pic16_state : public driver_device
{
public:
	hh_pic16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<2> m_inp_matrix; // max 2
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT8 m_b;                          // MCU port B data
	UINT8 m_c;                          // MCU port C data

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	UINT32 m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	UINT16 m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	UINT32 m_display_cache[0x20];       // (internal use)
	UINT8 m_display_decay[0x20][0x20];  // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_pic16_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_b = 0;
	m_c = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_b));
	save_item(NAME(m_c));
}

void hh_pic16_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_pic16_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			UINT32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_display_segmask[y] != 0)
				output().set_digit_value(y, active_state[y] & m_display_segmask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x <= m_display_maxx; x++)
			{
				int state = active_state[y] >> x & 1;
				char buf1[0x10]; // lampyx
				char buf2[0x10]; // y.x

				if (x == m_display_maxx)
				{
					// always-on if selected
					sprintf(buf1, "lamp%da", y);
					sprintf(buf2, "%d.a", y);
				}
				else
				{
					sprintf(buf1, "lamp%d", y * mul + x);
					sprintf(buf2, "%d.%d", y, x);
				}
				output().set_value(buf1, state);
				output().set_value(buf2, state);
			}
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_pic16_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_pic16_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_pic16_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Ideal Maniac, by Ralph Baer
  * PIC1655A-036


***************************************************************************/

class maniac_state : public hh_pic16_state
{
public:
	maniac_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_pic16_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(output_w);
};

// handlers

WRITE8_MEMBER(maniac_state::output_w)
{
	// B,C: outputs
	offset -= PIC16C5x_PORTB;
	if (offset)
		m_c = data;
	else
		m_b = data;

	// d7: speaker out
	m_speaker->level_w((m_b >> 7 & 1) | (m_c >> 6 & 2));

	// d0-d6: 7seg
	m_display_segmask[offset] = 0x7f;
	m_display_state[offset] = ~data & 0x7f;

	set_display_size(7, 2);
	display_update();
}


// config

static INPUT_PORTS_START( maniac )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // bottom-right
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // upper-right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // bottom-left
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // upper-left
INPUT_PORTS_END

static const INT16 maniac_speaker_levels[] = { 0, 0x7fff, -0x8000, 0 };

static MACHINE_CONFIG_START( maniac, maniac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC16C55, 850000) // RC osc. R=13.4K, C=470pf, but unknown RC curve - measured 800-890kHz
	MCFG_PIC16C5x_READ_A_CB(IOPORT("IN.0"))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(maniac_state, output_w))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(maniac_state, output_w))
	MCFG_PIC16C5x_SET_CONFIG(0) // ?

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_pic16_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_maniac)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, maniac_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( maniac )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "pic1655a-036", 0x0000, 0x0400, CRC(a96f7011) SHA1(e97ae44d3c1e74c7e1024bb0bdab03eecdc9f827) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE INPUT   INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, maniac,    0,        0, maniac,  maniac, driver_device, 0, "Ideal", "Maniac", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
