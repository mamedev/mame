// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

  TECNBRAS dot matrix display (70x7 pixels)
  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

    The display is composed of 14 blocks of 5x7 LEDs

    These LEDs are driven by several 74xx chips:
    * one 74138
    * several 74164 and ULN2003 chips

  Changelog:

   2014 JUN 23 [Felipe Sanches]:
   * Initial driver skeleton

================
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "tecnbras.lh"

class tecnbras_state : public driver_device
{
public:
	tecnbras_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(set_x_position_w);
	DECLARE_WRITE8_MEMBER(print_column_w);

//  DECLARE_WRITE8_MEMBER(tecnbras_io_w);
//  DECLARE_READ8_MEMBER(tecnbras_io_r);
	DECLARE_DRIVER_INIT(tecnbras);
private:
	int m_xcoord;
	char m_digit[14][7];
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(i80c31_prg, AS_PROGRAM, 8, tecnbras_state)
	AM_RANGE(0x0000, 0x7FFF) AM_ROM
	AM_RANGE(0x8000, 0xFFFF) AM_RAM
ADDRESS_MAP_END

#define DMD_OFFSET 24 //This is a guess. We should verify the real hardware behaviour
static ADDRESS_MAP_START(i80c31_io, AS_IO, 8, tecnbras_state)
	AM_RANGE(0x0100+DMD_OFFSET, 0x0145+DMD_OFFSET) AM_WRITE(set_x_position_w)
	AM_RANGE(0x06B8, 0x06BC) AM_WRITE(print_column_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_NOP /*buzzer ?*/
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( tecnbras_state, tecnbras )
{
	m_xcoord = 0;
	for (auto & elem : m_digit){
		for (int y=0; y<7; y++){
			elem[y] = 0;
		}
	}
}

WRITE8_MEMBER(tecnbras_state::set_x_position_w)
{
	m_xcoord = offset;
}

WRITE8_MEMBER(tecnbras_state::print_column_w)
{
	int x = m_xcoord + offset;
	for (int i=0; i<7; i++){
		if((x/5) < ARRAY_LENGTH(m_digit)){
			m_digit[x/5][i] &= ~(1 << (x%5));
			m_digit[x/5][i] |= BIT(data, 7-i) ? (1 << (x%5)) : 0;
			output_set_indexed_value("dmd_", (x/5)*7 + i, 0x1F & m_digit[x/5][i]);
		}
	}
}

void tecnbras_state::machine_start()
{
/*
    for (int x=0; x<14; x++){
        for (int y=0; y<7; y++){
            output_set_indexed_value("dmd_", x*7 + y, y);
        }
    }
*/
}

void tecnbras_state::machine_reset()
{
}

static MACHINE_CONFIG_START( tecnbras, tecnbras_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C31, XTAL_12MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(i80c31_prg)
	MCFG_CPU_IO_MAP(i80c31_io)

/* TODO: Add an I2C RTC (Phillips PCF8583P)
   pin 6 (SCL): cpu T0/P3.4 (pin 14)
   pin 5 (SDA): cpu T1/P3.5 (pin 15)
*/

/*
    TODO: Add a speaker
    CPU P1.0 (pin 1)
*/

/*
    TODO: Add a communications port to receive commands from the remote control
*/

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_tecnbras)

MACHINE_CONFIG_END

ROM_START( tecnbras )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tecnbras.u2",  0x0000, 0x8000, CRC(1a1e18fc) SHA1(8907e72f0356a2e2e1097dabac6d6b0b3d717f85) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT     CLASS         INIT    COMPANY  FULLNAME                       FLAGS */
COMP( 200?, tecnbras,   0,      0,      tecnbras,     0,   tecnbras_state, tecnbras, "Tecnbras", "Dot Matrix Display (70x7 pixels)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
