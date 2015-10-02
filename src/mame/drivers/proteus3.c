// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

    Proteus III computer.

    2015-10-02 Skeleton [Robbbert]

    Chips:
    6800 @ 894kHz
    6850 (TTY interface)
    6850 (Cassette interface)
    6820 (PIA for Keyboard and video
    6844 DMA
    MC14411 baud rate generator
    CRT96364 CRTC

    To Do:
    - Everything

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"


class proteus3_state : public driver_device
{
public:
	proteus3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
	{ }

//	DECLARE_READ_LINE_MEMBER( proteus3_cb1_r );
	DECLARE_READ8_MEMBER( proteus3_keyboard_r );
//	DECLARE_WRITE_LINE_MEMBER( proteus3_cb2_w );
//	DECLARE_WRITE8_MEMBER( proteus3_digit_w );
private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	//required_device<acia6850_device> m_acia;
};




/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(proteus3_mem, AS_PROGRAM, 8, proteus3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8004, 0x8007) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_DEVREADWRITE("acia1", acia6850_device, status_r, control_w)
	AM_RANGE(0x8009, 0x8009) AM_DEVREADWRITE("acia1", acia6850_device, data_r, data_w)
	AM_RANGE(0x8010, 0x8010) AM_DEVREADWRITE("acia2", acia6850_device, status_r, control_w)
	AM_RANGE(0x8011, 0x8011) AM_DEVREADWRITE("acia2", acia6850_device, data_r, data_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(proteus3)
INPUT_PORTS_END

void proteus3_state::machine_reset()
{
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( proteus3, proteus3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_3_579545MHz)  /* Divided by 4 internally */
	MCFG_CPU_PROGRAM_MAP(proteus3_mem)


	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	//MCFG_PIA_READPB_HANDLER(READ8(proteus3_state, proteus3_keyboard_r))
	//MCFG_PIA_READCA1_HANDLER(READLINE(proteus3_state, proteus3_distance_r))
	//MCFG_PIA_READCB1_HANDLER(READLINE(proteus3_state, proteus3_cb1_r))
	//MCFG_PIA_READCA2_HANDLER(READLINE(proteus3_state, proteus3_fuel_sensor_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(proteus3_state, proteus3_segment_w))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(proteus3_state, proteus3_digit_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(proteus3_state, proteus3_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD ("acia1", ACIA6850, 0)
	MCFG_DEVICE_ADD ("acia2", ACIA6850, 0)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(proteus3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "proteus3_basic8k.m0", 0xe000, 0x2000, CRC(7d9111c2) SHA1(3c032c9c7f87d22a1a9819b3b812be84404d2ad2) )
	ROM_RELOAD( 0xc000, 0x2000 )

	ROM_REGION(0x400, "chargen", 0)
	ROM_LOAD( "proteus3_font.m25",   0x0000, 0x0400, CRC(6a3a30a5) SHA1(ab39bf09722928483e497b87ac2dbd870828893b) )
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     CLASS          INIT      COMPANY                     FULLNAME        FLAGS */
COMP( 1978, proteus3,   0,          0,      proteus3,   proteus3, driver_device,   0,     "Proteus International", "Proteus III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
