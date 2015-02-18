// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Epoch Dracula (manufactured in Japan)
  * NEC uCOM-43 MCU, labeled D553C 206
  * cyan/red/green VFD display NEC FIP8BM20T
  
  known releases:
  - Japan: Dracula House, yellow case
  - USA: Dracula, red case
  - Other: Dracula, yellow case, published by Hales

  NOTE!: MESS external artwork is required to be able to play


  TODO:
  - display should go off when sound is played, needs decay simulation?

***************************************************************************/

#include "emu.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"

#include "edracula.lh" // this is a test layout, external artwork is necessary


class edracula_state : public driver_device
{
public:
	edracula_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	UINT32 m_plate;
	UINT16 m_grid;

	DECLARE_WRITE8_MEMBER(grid_w);
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE8_MEMBER(port_i_w);

	UINT32 m_vfd_state[0x10];
	void update_vfd();
	
	virtual void machine_start();
};



/***************************************************************************

  Display

***************************************************************************/

void edracula_state::update_vfd()
{
	for (int i = 0; i < 8; i++)
		if (m_grid & (1 << i) && m_vfd_state[i] != m_plate)
		{
			// on difference, send to output
			for (int j = 0; j < 18; j++)
				output_set_lamp_value(i*100 + j, m_plate >> j & 1);
			
			m_vfd_state[i] = m_plate;
		}
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(edracula_state::grid_w)
{
	// port C/D: vfd matrix grid
	int shift = (offset - NEC_UCOM4_PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	
	update_vfd();
}

WRITE8_MEMBER(edracula_state::plate_w)
{
	// port E/F/G/H/I01: vfd matrix plate
	int shift = (offset - NEC_UCOM4_PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	
	update_vfd();
}

WRITE8_MEMBER(edracula_state::port_i_w)
{
	plate_w(space, offset, data & 3);

	// I2: speaker out
	m_speaker->level_w(data >> 2 & 1);
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( edracula )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void edracula_state::machine_start()
{
	// zerofill
	memset(m_vfd_state, 0, sizeof(m_vfd_state));
	m_plate = 0;
	m_grid = 0;

	// register for savestates
	save_item(NAME(m_vfd_state));
	save_item(NAME(m_plate));
	save_item(NAME(m_grid));
}


static MACHINE_CONFIG_START( edracula, edracula_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NEC_D553, XTAL_400kHz)
	MCFG_UCOM4_READ_A_CB(IOPORT("IN0"))
	MCFG_UCOM4_READ_B_CB(IOPORT("IN1"))
	MCFG_UCOM4_WRITE_C_CB(WRITE8(edracula_state, grid_w))
	MCFG_UCOM4_WRITE_D_CB(WRITE8(edracula_state, grid_w))
	MCFG_UCOM4_WRITE_E_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_F_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_G_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_H_CB(WRITE8(edracula_state, plate_w))
	MCFG_UCOM4_WRITE_I_CB(WRITE8(edracula_state, port_i_w))

	MCFG_DEFAULT_LAYOUT(layout_edracula)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )
ROM_END


CONS( 1982, edracula, 0, 0, edracula, edracula, driver_device, 0, "Epoch", "Dracula", GAME_SUPPORTS_SAVE )
