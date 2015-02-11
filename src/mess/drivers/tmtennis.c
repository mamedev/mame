// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tomy Tennis (manufactured in Japan)
  * board labeled TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, labeled D552C 048
  * VFD display NEC FIP11AM15T (FIP=fluorescent indicator panel)
  
  The initial release of this game was in 1979, known as Pro-Tennis,
  it is unknown if the hardware and/or ROM contents differ.
  

  TODO:
  - 2-player mode doesn't work: the guys auto-serve and the left player
    always hits the net, mcu emulation bug?
  - difficulty switch changes mcu freq

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "tmtennis.lh" // this is a test layout, external artwork is necessary

// master clock is from an LC circuit oscillating by default at 360kHz,
// the difficulty switch puts a capacitor across it to slow it down to 260kHz
#define MASTER_CLOCK_PRO1 (260000)
#define MASTER_CLOCK_PRO2 (360000)


class tmtennis_state : public driver_device
{
public:
	tmtennis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<2> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;
	
	UINT8 m_input_mux;
	UINT16 m_plate;
	UINT16 m_grid;

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(port_e_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(grid_w);

	UINT16 m_vfd_state[0x10];
	void update_vfd();

	virtual void machine_start();
};



/***************************************************************************

  Display

***************************************************************************/

void tmtennis_state::update_vfd()
{
	for (int i = 0; i < 12; i++)
		if (m_grid & (1 << i) && m_vfd_state[i] != m_plate)
		{
			// on difference, send to output
			for (int j = 0; j < 12; j++)
				output_set_lamp_value(i*100 + j, m_plate >> j & 1);
			
			m_vfd_state[i] = m_plate;
		}
}



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(tmtennis_state::input_r)
{
	// port A/B: buttons
	UINT8 inp = 0xff;

	// read selected button rows
	for (int i = 0; i < 2; i++)
		if (~m_input_mux >> i & 1)
			inp &= m_button_matrix[i]->read();

	return inp >> (offset*4);
}

WRITE8_MEMBER(tmtennis_state::port_e_w)
{
	// E0/E1: input mux
	// E2: speaker out
	// E3: N/C
	m_input_mux = data & 3;
	m_speaker->level_w(data >> 2 & 1);
}

WRITE8_MEMBER(tmtennis_state::plate_w)
{
	// port C/D/F: vfd matrix plate
	if (offset == NEC_UCOM4_PORTF) offset--;
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	
	update_vfd();
}

WRITE8_MEMBER(tmtennis_state::grid_w)
{
	// port G/H/I: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	
	update_vfd();
}



/***************************************************************************

  Inputs

***************************************************************************/

/* Pro-Tennis physical button layout and labels is like this:

    [SERVE] [1] [2] [3]       [3] [2] [1] [SERVE]
            [4] [5] [6]       [6] [5] [4]

    PRACTICE<--PRO1-->PRO2    1PLAYER<--OFF-->2PLAYER
*/

static INPUT_PORTS_START( tmtennis )
	PORT_START("IN.0") // E0 port A/B (left side)
	PORT_CONFNAME( 0x101, 0x001, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(     0x000, "Practice" )
	PORT_CONFSETTING(     0x001, "Pro 1" )
	PORT_CONFSETTING(     0x101, "Pro 2" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) // P2 serve
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_START("IN.1") // E1 port A/B (right side)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // P1 serve
	PORT_CONFNAME( 0x02, 0x02, "Players" )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tmtennis_state::machine_start()
{
	// zerofill
	memset(m_vfd_state, 0, sizeof(m_vfd_state));
	m_input_mux = 0;
	m_plate = 0;
	m_grid = 0;

	// register for savestates
	save_item(NAME(m_vfd_state));
	save_item(NAME(m_input_mux));
	save_item(NAME(m_plate));
	save_item(NAME(m_grid));
}


static MACHINE_CONFIG_START( tmtennis, tmtennis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D552, MASTER_CLOCK_PRO2)
	MCFG_UCOM4_READ_A_CB(READ8(tmtennis_state, input_r))
	MCFG_UCOM4_READ_B_CB(READ8(tmtennis_state, input_r))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(tmtennis_state, port_e_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(tmtennis_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(tmtennis_state, grid_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(tmtennis_state, grid_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(tmtennis_state, grid_w))

	MCFG_DEFAULT_LAYOUT(layout_tmtennis)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tmtennis )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-048", 0x0000, 0x0400, CRC(78702003) SHA1(4d427d4dbeed901770c682338867f58c7b54eee3) )
ROM_END


CONS( 1980, tmtennis, 0, 0, tmtennis, tmtennis, driver_device, 0, "Tomy", "Tennis (Tomytronic)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
