// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tomy Alien Chase (manufactured in Japan)
  * boards are labeled TN-16
  * NEC uCOM-43 MCU, labeled D553C 258
  * red/green VFD display with color overlay, 2-sided*
  
  *Player one views the VFD from the front (grid+filament side) while the
  opposite player views it from the back side (through the conductive traces),
  basically a mirror-image.
  
  This is a space-themed tabletop VFD electronic game. To start, simply
  press [UP]. Hold a joystick direction to move around.

  NOTE!: MESS external artwork is required to be able to play

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "alnchase.lh" // this is a test layout, external artwork is necessary


class alnchase_state : public driver_device
{
public:
	alnchase_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<2> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;
	
	UINT8 m_input_mux;
	UINT32 m_plate;
	UINT16 m_grid;

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(port_e_w);

	UINT32 m_vfd_state[0x10];
	void update_vfd();

	virtual void machine_start();
};



/***************************************************************************

  Display

***************************************************************************/

void alnchase_state::update_vfd()
{
	for (int i = 0; i < 9; i++)
		if (m_grid & (1 << i) && m_vfd_state[i] != m_plate)
		{
			// on difference, send to output
			for (int j = 0; j < 17; j++)
				output_set_lamp_value(i*100 + j, m_plate >> j & 1);
			
			m_vfd_state[i] = m_plate;
		}
}



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(alnchase_state::input_r)
{
	UINT8 inp = 0;

	// read selected button rows
	for (int i = 0; i < 2; i++)
		if (m_input_mux >> i & 1)
			inp |= m_button_matrix[i]->read();

	return inp;
}

WRITE8_MEMBER(alnchase_state::display_w)
{
	int shift;
	
	if (offset <= NEC_UCOM4_PORTE)
	{
		// C/D/E0: vfd matrix grid
		shift = (offset - NEC_UCOM4_PORTC) * 4;
		m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
		
		// C0(grid 0): input enable PL1
		// D0(grid 4): input enable PL2
		m_input_mux = (m_grid & 1) | (m_grid >> 3 & 2);
	}
	
	if (offset >= NEC_UCOM4_PORTE)
	{
		// E23/F/G/H/I: vfd matrix plate
		shift = (offset - NEC_UCOM4_PORTE) * 4;
		m_plate = ((m_plate << 2 & ~(0xf << shift)) | (data << shift)) >> 2;
	}
	
	update_vfd();
}

WRITE8_MEMBER(alnchase_state::port_e_w)
{
	display_w(space, offset, data);

	// E1: speaker out
	m_speaker->level_w(data >> 1 & 1);
}



/***************************************************************************

  Inputs

***************************************************************************/

/* physical button layout and labels is like this:

    POWER SOUND LEVEL PLAYER
     ON    ON    PRO   TWO        START
      o     o     |     |    
      |     |     |     |       [joystick]
      |     |     o     o
     OFF   OFF   AMA   ONE     GAME 0,1,2,3
    
    1 PLAYER SIDE
    
    other player side only has a joystick
*/

static INPUT_PORTS_START( alnchase )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.1") // D0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // on non-mirrored view, swap P2 left/right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("SW") // port B
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void alnchase_state::machine_start()
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


static MACHINE_CONFIG_START( alnchase, alnchase_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(READ8(alnchase_state, input_r))
	MCFG_UCOM4_READ_B_CB(IOPORT("SW"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(alnchase_state, display_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(alnchase_state, display_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(alnchase_state, port_e_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(alnchase_state, display_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(alnchase_state, display_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(alnchase_state, display_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(alnchase_state, display_w))

	MCFG_DEFAULT_LAYOUT(layout_alnchase)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( alnchase )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-258", 0x0000, 0x0800, CRC(c5284ff5) SHA1(6a20aaacc9748f0e0335958f3cea482e36153704) )
ROM_END


CONS( 1984, alnchase, 0, 0, alnchase, alnchase, driver_device, 0, "Tomy", "Alien Chase", GAME_NOT_WORKING )
