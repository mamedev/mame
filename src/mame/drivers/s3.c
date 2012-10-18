/***********************************************************************************

    Pinball
    Williams System 3

    Typical of Williams hardware: Motorola 8-bit CPUs, and lots of PIAs.

    Sound doesn't work because the diagram doesn't show where the sound data
    comes from. (variable m_sound_data needs to be written to).

************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
//#include "s3.lh"


class s3_state : public genpin_class
{
public:
	s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_dac(*this, "dac"),
	m_pia0(*this, "pia0"),
	m_pia1(*this, "pia1"),
	m_pia2(*this, "pia2"),
	m_pia3(*this, "pia3"),
	m_pia4(*this, "pia4")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_READ_LINE_MEMBER(cb1_r);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	bool m_cb1;
};

static ADDRESS_MAP_START( s3_main_map, AS_PROGRAM, 8, s3_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia0", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia1", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia2", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia3", pia6821_device, read, write) // inputs
	AM_RANGE(0x6000, 0x67ff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s3_audio_map, AS_PROGRAM, 8, s3_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pia4", pia6821_device, read, write) // sounds
	AM_RANGE(0x0800, 0x0fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s3 )
INPUT_PORTS_END

void s3_state::machine_reset()
{
	m_t_c = 0;
}

static const pia6821_interface pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

static const pia6821_interface pia1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

static const pia6821_interface pia2_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

static const pia6821_interface pia3_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ_LINE_MEMBER( s3_state::cb1_r )
{
	return m_cb1;
}

READ8_MEMBER( s3_state::dac_r )
{
	m_cb1 = (m_sound_data < 255);
	// add code to activate cb1
	return m_sound_data;	
}

WRITE8_MEMBER( s3_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

static const pia6821_interface pia4_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_DRIVER_LINE_MEMBER(s3_state, cb1_r),		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s3_state, dac_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)		/* IRQB */
};

TIMER_DEVICE_CALLBACK_MEMBER( s3_state::irq)
{
	if (m_t_c > 0x10)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

static MACHINE_CONFIG_START( s3, s3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 3580000)
	MCFG_CPU_PROGRAM_MAP(s3_main_map)
	MCFG_CPU_ADD("audiocpu", M6802, 3580000)
	MCFG_CPU_PROGRAM_MAP(s3_audio_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s3_state, irq, attotime::from_hz(1000))

	/* Video */
	//MCFG_DEFAULT_LAYOUT(layout_s3)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_PIA6821_ADD("pia0", pia0_intf)
	MCFG_PIA6821_ADD("pia1", pia1_intf)
	MCFG_PIA6821_ADD("pia2", pia2_intf)
	MCFG_PIA6821_ADD("pia3", pia3_intf)
	MCFG_PIA6821_ADD("pia4", pia4_intf)
MACHINE_CONFIG_END

/*-------------------------------------
/ Contact - Sys.3 (Game #482)
/-------------------------------------*/
ROM_START(cntct_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(35359b60) SHA1(ab4c3328d93bdb4c952090b327c91b0ded36152c))
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x0800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*-------------------------------------
/ Disco Fever - Sys.3 (Game #483)
/-------------------------------------*/
ROM_START(disco_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(831d8adb) SHA1(99a9c3d5c8cbcdf3bb9c210ad9d05c34905b272e))
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x0800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*----------------------------
/ Hot Tip - Sys.3 (Game #477) - No Sound board
/----------------------------*/
ROM_START(httip_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b1d4fd9b) SHA1(e55ecf1328a55979c4cf8f3fb4e6761747e0abc4))
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*---------------------------------
/ Lucky Seven - Sys.3 (Game #480) - No Sound board
/---------------------------------*/
ROM_START(lucky_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(7cfbd4c7) SHA1(825e2245fd1615e932973f5e2b5ed5f2da9309e7))
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716", 0x7800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------
/ World Cup Soccer - Sys.3 (Game #481)
/-------------------------------------*/
ROM_START(wldcp_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(c8071956) SHA1(0452aaf2ec1bcc5717fe52a6c541d79402bebb17))
	ROM_LOAD("white1.716", 0x7000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2wc.716", 0x7800, 0x0800, CRC(618d15b5) SHA1(527387893eeb2cd4aa563a4cfb1948a15d2ed741))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x0800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END


GAME( 1977, httip_l1, 0, s3, s3, driver_device, 0, ROT0, "Williams", "Hot Tip (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, lucky_l1, 0, s3, s3, driver_device, 0, ROT0, "Williams", "Lucky Seven (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, wldcp_l1, 0, s3, s3, driver_device, 0, ROT0, "Williams", "World Cup Soccer (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, cntct_l1, 0, s3, s3, driver_device, 0, ROT0, "Williams", "Contact (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, disco_l1, 0, s3, s3, driver_device, 0, ROT0, "Williams", "Disco Fever (L-1)", GAME_IS_SKELETON_MECHANICAL)
