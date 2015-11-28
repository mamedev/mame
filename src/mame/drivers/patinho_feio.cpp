// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Patinho Feio
*/

#include "emu.h"
#include "cpu/patinhofeio/patinho_feio.h"

class patinho_feio_state : public driver_device
{
public:
    patinho_feio_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        //,m_maincpu(*this, "maincpu")
    { }

    DECLARE_DRIVER_INIT(patinho_feio);
//    virtual void machine_start();
//    virtual void machine_reset();
//    required_device<patinho_feio_cpu_device> m_maincpu;
};

/*
    driver init function
*/
DRIVER_INIT_MEMBER(patinho_feio_state, patinho_feio)
{
}

static INPUT_PORTS_START( patinho_feio )
//	PORT_START("PANEL")       /* various operator control panel switches */
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
//  PORT_BIT(?, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("control panel key") PORT_CODE(KEYCODE_?)
INPUT_PORTS_END

//Internal 4kbytes of RAM
static ADDRESS_MAP_START(patinho_program_map, AS_PROGRAM, 8, patinho_feio_state)
    AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_START( patinho_feio, patinho_feio_state )
	/* basic machine hardware */
	/* CPU @ approx. 500 kHz (memory cycle time is 2usec) */
	MCFG_CPU_ADD("maincpu", PATINHO_FEIO, 500000)
    MCFG_CPU_PROGRAM_MAP(patinho_program_map)
MACHINE_CONFIG_END

ROM_START(patinho)
	/*CPU memory space*/
	ROM_REGION(0x01000,"maincpu", ROMREGION_ERASEFF)
    /* Load this into th internal RAM... */
    ROM_LOAD("apendice_g__hexam.bin", 0x000, 0x0d5, CRC(c6addc59) SHA1(126bc97247eac45c58708eaac216c2438e9e4af9) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE        INPUT         INIT                              COMPANY                                           FULLNAME */
COMP( 1972, patinho,  0,        0,      patinho_feio,  patinho_feio, patinho_feio_state, patinho_feio, "Escola Politecnica - Universidade de Sao Paulo", "Patinho Feio" , MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
