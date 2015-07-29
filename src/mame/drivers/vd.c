// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************

    PINBALL
    Videodens

    PinMAME used as reference (unable to find any info at all on the net).

    Nothing in this driver is confirmed except where noted.

***************************************************************************************/


#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "vd.lh"

class vd_state : public driver_device
{
public:
	vd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(dsw_r) { return 0; }
	DECLARE_WRITE8_MEMBER(col_w);
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(sol_w) { };
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_t_c;
	UINT8 segment[5];
};


static ADDRESS_MAP_START( vd_map, AS_PROGRAM, 8, vd_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x62ff) AM_RAM
	AM_RANGE(0x6700, 0x67ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( vd_io, AS_IO, 8, vd_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ_PORT("X0")
	AM_RANGE(0x01,0x01) AM_READ_PORT("X1")
	AM_RANGE(0x02,0x02) AM_READ_PORT("X2")
	AM_RANGE(0x03,0x03) AM_READ_PORT("X3")
	AM_RANGE(0x04,0x04) AM_READ_PORT("X4")
	AM_RANGE(0x05,0x05) AM_READ_PORT("X5")
	AM_RANGE(0x20,0x27) AM_WRITE(lamp_w)
	AM_RANGE(0x28,0x28) AM_WRITE(sol_w)
	AM_RANGE(0x40,0x44) AM_WRITE(disp_w)
	AM_RANGE(0x60,0x60) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x61,0x61) AM_READ_PORT("DSW") //AM_READ(dsw_r)
	AM_RANGE(0x62,0x62) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x80,0x80) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x82,0x82) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0xa0,0xa0) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xc0,0xc0) AM_WRITE(col_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( vd )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "Accounting #1")
	PORT_DIPSETTING(0x0000, DEF_STR(Off))
	PORT_DIPSETTING(0x0001, DEF_STR(On))
	PORT_DIPNAME( 0x0002, 0x0000, "Accounting #2")
	PORT_DIPSETTING(0x0000, DEF_STR(Off))
	PORT_DIPSETTING(0x0002, DEF_STR(On))
	PORT_DIPNAME( 0x0004, 0x0000, "Accounting #3")
	PORT_DIPSETTING(0x0000, DEF_STR(Off))
	PORT_DIPSETTING(0x0004, DEF_STR(On))
	PORT_DIPNAME( 0x0018, 0x0000, "Credits per coin (chute #1/#2)")
	PORT_DIPSETTING(0x0018, "0.5/3" )
	PORT_DIPSETTING(0x0000, "1/5" )
	PORT_DIPSETTING(0x0008, "1/6" )
	PORT_DIPSETTING(0x0010, "2/8" )
	PORT_DIPNAME( 0x0020, 0x0000, "S6")
	PORT_DIPSETTING(0x0000, "0" )
	PORT_DIPSETTING(0x0020, "1" )
	PORT_DIPNAME( 0x0040, 0x0000, "Match feature")
	PORT_DIPSETTING(0x0040, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME( 0x0080, 0x0000, "S8")
	PORT_DIPSETTING(0x0000, "0" )
	PORT_DIPSETTING(0x0080, "1" )

	PORT_START("X0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_TILT)

	PORT_START("X1")
	PORT_START("X2")
	PORT_START("X3")
	PORT_START("X4")
	PORT_START("X5")
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER( vd_state::irq )
{
	if (m_t_c > 40)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	else
		m_t_c++;
}

WRITE8_MEMBER( vd_state::disp_w )
{
	segment[offset] = data;
	if (!offset)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE8_MEMBER( vd_state::col_w )
{
	if (data != 0x3f)
	{
		data &= 7;
		output_set_digit_value(data + 11, segment[0]);
		output_set_digit_value(data + 21, segment[1]);
		output_set_digit_value(data + 31, segment[2]);
		output_set_digit_value(data + 41, segment[3]);
		output_set_digit_value(data + 51, segment[4]);
	}
}

void vd_state::machine_reset()
{
	m_t_c = 0;
}

static MACHINE_CONFIG_START( vd, vd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(vd_map)
	MCFG_CPU_IO_MAP(vd_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", vd_state, irq, attotime::from_hz(484))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33/3)
	MCFG_SOUND_ADD("ay2", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33/3)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_vd)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Break '86 (1986)
/-------------------------------------------------------------------*/
ROM_START(break86)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("break1.cpu", 0x0000, 0x2000, CRC(c187d263) SHA1(1790566799ccc41cd5445936e86f945150e24e8a))
	ROM_LOAD("break2.cpu", 0x2000, 0x2000, CRC(ed8f84ab) SHA1(ff5d7e3c373ca345205e8b92c6ce7b02f36a3d95))
	ROM_LOAD("break3.cpu", 0x4000, 0x2000, CRC(3cdfedc2) SHA1(309fd04c81b8facdf705e6297c0f4d507957ae1f))
ROM_END

/*-------------------------------------------------------------------
/ Papillon (1986)
/-------------------------------------------------------------------*/
ROM_START(papillon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u4.dat", 0x0000, 0x2000, CRC(e57bfcdd) SHA1(d0d5c798552a2436693dfee0e2ebf4b6f465b194))
	ROM_LOAD("u5.dat", 0x2000, 0x2000, CRC(6d2ef02a) SHA1(0b67b2edd85624531630c162ae31af8078be01e3))
	ROM_LOAD("u6.dat", 0x4000, 0x2000, CRC(6b2867b3) SHA1(720fe8a65b447e839b0eb9ea21e0b3cb0e50cf7a))
ROM_END

#if 0
/*-------------------------------------------------------------------
/ Ator (19??)
/-------------------------------------------------------------------*/
ROM_START(ator)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ator.u4", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("ator.u5", 0x2000, 0x2000, NO_DUMP)
	ROM_LOAD("ator.u6", 0x4000, 0x2000, CRC(21aad5c4) SHA1(e78da5d80682710db34cbbfeae5af54241c73371))
ROM_END
#endif

//GAME(19??, ator,     0,    vd,  vd, driver_device, 0,  ROT0,  "Videodens", "Ator", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, break86,  0,    vd,  vd, driver_device, 0,  ROT0,  "Videodens", "Break '86", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, papillon, 0,    vd,  vd, driver_device, 0,  ROT0,  "Videodens", "Papillon", MACHINE_IS_SKELETON_MECHANICAL)
