/*******************************************************************************************************

  PINBALL
  Inder S.A. of Spain

  All manuals are in Spanish (including the 'English' ones), so some guesswork will be needed.
  The schematics for Brave Team, Canasta are too blurry to read.
  Each game has different hardware.

  Setting up:
  - First time run, the displays will all show zero. Set up the dips. Then exit and restart.
    The game will be working.

  Status:
  - Brave Team: working
  - Others: still to be worked on


********************************************************************************************************/

#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"
#include "sound/ay8910.h"
#include "inder.lh"

class inder_state : public genpin_class
{
public:
	inder_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sn(*this, "sn")
		, m_switches(*this, "SW")
	{ }

	DECLARE_READ8_MEMBER(sw_r);
	DECLARE_WRITE8_MEMBER(sw_w);
	DECLARE_WRITE8_MEMBER(sol_brvteam_w);
	DECLARE_WRITE8_MEMBER(sol_canasta_w);
	DECLARE_WRITE8_MEMBER(sn_w);
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_DRIVER_INIT(inder);
private:
	UINT8 m_row;
	UINT8 m_segment[5];
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	optional_device<sn76489_device> m_sn;
	required_ioport_array<11> m_switches;
};

static ADDRESS_MAP_START( brvteam_map, AS_PROGRAM, 8, inder_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x20ff) AM_WRITE(disp_w)
	AM_RANGE(0x4000, 0x43ff) AM_RAM // pair of 2114
	AM_RANGE(0x4400, 0x44ff) AM_RAM AM_SHARE("nvram") // pair of 5101, battery-backed
	AM_RANGE(0x4800, 0x480a) AM_READWRITE(sw_r,sw_w)
	AM_RANGE(0x4900, 0x4900) AM_WRITE(sol_brvteam_w)
	AM_RANGE(0x4901, 0x4907) AM_WRITE(lamp_w)
	AM_RANGE(0x4b00, 0x4b00) AM_WRITE(sn_w)
	//AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( canasta_map, AS_PROGRAM, 8, inder_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x20ff) AM_WRITE(disp_w)
	AM_RANGE(0x4000, 0x43ff) AM_RAM // pair of 2114
	AM_RANGE(0x4400, 0x44ff) AM_RAM AM_SHARE("nvram") // pair of 5101, battery-backed
	AM_RANGE(0x4800, 0x480a) AM_READWRITE(sw_r,sw_w)
	AM_RANGE(0x4900, 0x4900) AM_WRITE(sol_canasta_w)
	AM_RANGE(0x4901, 0x4907) AM_WRITE(lamp_w)
	AM_RANGE(0x4b00, 0x4b00) AM_DEVWRITE("ay", ay8910_device, address_w)
	AM_RANGE(0x4b01, 0x4b01) AM_DEVREAD("ay", ay8910_device, data_r)
	AM_RANGE(0x4b02, 0x4b02) AM_DEVWRITE("ay", ay8910_device, data_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( inder )
	PORT_START("SW.0")
	PORT_DIPNAME( 0x03, 0x01, "Coin Slot 1") // sw G,H
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C )) // slot 2: 1 moneda 4 partidas  // selection 00 is same as 01
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C )) // slot 2: 1 moneda 5 partidas
	PORT_DIPSETTING(    0x03, "4 moneda 6 partidas") // slot 2: 1 moneda 6 partidas
	PORT_DIPNAME( 0x08, 0x08, "Balls") // sw E
	PORT_DIPSETTING(    0x08, "3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPNAME( 0x30, 0x20, "Points for free game") // sw C,D
	PORT_DIPSETTING(    0x00, "850000")
	PORT_DIPSETTING(    0x10, "800000")
	PORT_DIPSETTING(    0x20, "750000")
	PORT_DIPSETTING(    0x30, "700000")
	PORT_BIT( 0xc4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.1")
	PORT_DIPNAME( 0x03, 0x03, "High Score") //"Handicap"  // sw O,P
	PORT_DIPSETTING(    0x00, "990000")
	PORT_DIPSETTING(    0x01, "950000")
	PORT_DIPSETTING(    0x02, "900000")
	PORT_DIPSETTING(    0x03, "850000")
	PORT_DIPNAME( 0x04, 0x00, "Maximum Credits") // sw N
	PORT_DIPSETTING(    0x04, "10")
	PORT_DIPSETTING(    0x00, "20")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.2") // bank of unused dipswitches
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.5") // Contactos 50-57
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // "Monedero A"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // "Monedero B"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT ) // "Falta"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) // "Pulsador Partidas"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Reset") // "Puesta a cero"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Accounting info") // "Test economico"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Test") // "Test tecnico"

	PORT_START("SW.6") // 60-67
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.7") // 70-77
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.8") // 80-87
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_START("SW.9") // 90-97
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)

	PORT_START("SW.10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( canasta )
	PORT_START("SW.0")
	PORT_DIPNAME( 0x03, 0x00, "Coin Slot 1") // sw G,H
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C )) // slot 2: 1 moneda 4 partidas
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C )) // slot 2: 1 moneda 5 partidas
	PORT_DIPSETTING(    0x02, "4 moneda 6 partidas") // slot 2: 1 moneda 6 partidas
	PORT_DIPSETTING(    0x03, "4 moneda 9 partidas") // slot 2: 1 moneda 9 partidas
	PORT_DIPNAME( 0x08, 0x08, "Balls") // sw E
	PORT_DIPSETTING(    0x08, "3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPNAME( 0x30, 0x20, "Points for 1st free game") // sw C,D
	PORT_DIPSETTING(    0x00, "1900000")
	PORT_DIPSETTING(    0x10, "1800000")
	PORT_DIPSETTING(    0x20, "1700000")
	PORT_DIPSETTING(    0x30, "1500000")
	PORT_DIPNAME( 0xc0, 0xc0, "Points for 2nd free game") // sw A,B
	PORT_DIPSETTING(    0x00, "NA")
	PORT_DIPSETTING(    0x10, "2900000")
	PORT_DIPSETTING(    0x20, "2700000")
	PORT_DIPSETTING(    0x30, "2500000")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.1")
	PORT_DIPNAME( 0x03, 0x03, "High Score") //"Handicap"  // sw O,P
	PORT_DIPSETTING(    0x00, "3500000")
	PORT_DIPSETTING(    0x01, "3000000")
	PORT_DIPSETTING(    0x02, "2400000")
	PORT_DIPSETTING(    0x03, "2000000")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.2") // bank of unused dipswitches
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.5") // Contactos 50-57
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // "Monedero A"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // "Monedero B"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT ) // "Falta"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) // "Pulsador Partidas"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Reset") // "Puesta a cero"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Accounting info") // "Test economico"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Test") // "Test tecnico"

	PORT_START("SW.6") // 60-67
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.7") // 70-77
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.8") // 80-87
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_START("SW.9") // 90-97
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)

	PORT_START("SW.10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

READ8_MEMBER( inder_state::sw_r )
{
	return m_switches[m_row]->read();
}

WRITE8_MEMBER( inder_state::sw_w )
{
	m_row = offset;
}

WRITE8_MEMBER( inder_state::sn_w )
{
	m_sn->write(space, 0, BITSWAP8(data, 0, 1, 2, 3, 4, 5, 6, 7));
}

// "bobinas"
WRITE8_MEMBER( inder_state::sol_brvteam_w )
{
	if ((data & 0xee) && BIT(data, 4)) // solenoid selected & activated
	{
		if BIT(data, 1)
			m_samples->start(0, 7); // left sling near bumpers "canon izq"

		if BIT(data, 2)
			m_samples->start(1, 7); // right sling near bumpers "canon der"

		if BIT(data, 3)
			m_samples->start(0, 5); // outhole

		if BIT(data, 5)
			m_samples->start(2, 0); // left bumper

		if BIT(data, 6)
			m_samples->start(3, 0); // right bumper

		if BIT(data, 7)
			m_samples->start(4, 0); // middle bumper
	}
}

// no slings in this game
WRITE8_MEMBER( inder_state::sol_canasta_w )
{
	if ((data & 0xee) && BIT(data, 4)) // solenoid selected & activated
	{
		if BIT(data, 3)
			m_samples->start(0, 5); // outhole

		if BIT(data, 5)
			m_samples->start(2, 0); // left bumper

		if BIT(data, 6)
			m_samples->start(3, 0); // right bumper

		if BIT(data, 7)
			m_samples->start(4, 0); // middle bumper
	}
}

WRITE8_MEMBER( inder_state::disp_w )
{
	UINT8 i;
	if (offset < 5)
		m_segment[offset] = data;
	else
	{
		offset = (offset >> 3) & 7;
		for (i = 0; i < 5; i++)
			output_set_digit_value(i*10+offset, m_segment[i]);
	}
}


void inder_state::machine_reset()
{
	m_row = 0;
}

DRIVER_INIT_MEMBER( inder_state, inder )
{
}

static MACHINE_CONFIG_START( inder, inder_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz / 2)
	MCFG_CPU_PROGRAM_MAP(brvteam_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(inder_state, irq0_line_hold, 250) // NE556

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_inder)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("snvol")
	MCFG_SOUND_ADD("sn", SN76489, XTAL_8MHz / 2) // jumper choice of 2 or 4 MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "snvol", 2.0)

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	//MCFG_I8255_IN_PORTA_CB(READ8(inder_state, porta_r))
	//MCFG_I8255_OUT_PORTA_CB(WRITE8(inder_state, porta_w))
	//MCFG_I8255_IN_PORTB_CB(READ8(inder_state, portb_r))
	//MCFG_I8255_OUT_PORTB_CB(WRITE8(inder_state, portb_w))
	//MCFG_I8255_IN_PORTC_CB(READ8(inder_state, portc_r))
	//MCFG_I8255_OUT_PORTC_CB(WRITE8(inder_state, portc_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( canasta, inder_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz / 2)
	MCFG_CPU_PROGRAM_MAP(canasta_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(inder_state, irq0_line_hold, 250) // NE556

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_inder)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("ayvol")
	MCFG_SOUND_ADD("ay", AY8910, XTAL_4MHz / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "ayvol", 1.0)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Brave Team (1985)
/-------------------------------------------------------------------*/
ROM_START(brvteam)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("brv-tea.m0", 0x0000, 0x1000, CRC(1fa72160) SHA1(0fa779ce2604599adff1e124d0b161b69094a614))
	ROM_LOAD("brv-tea.m1", 0x1000, 0x1000, CRC(4f02ca47) SHA1(68ec7d48c335a1ddd808feaeccac04a4f63d1a33))
ROM_END

/*-------------------------------------------------------------------
/ Canasta '86' (1986)
/-------------------------------------------------------------------*/
ROM_START(canasta)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("c860.bin", 0x0000, 0x1000, CRC(b1f79e52) SHA1(8e9c616f9be19d056da2f86778539d62c0885bac))
	ROM_LOAD("c861.bin", 0x1000, 0x1000, CRC(25ae3994) SHA1(86dcda3278fbe0e57b8ff4858b955d067af414ce))
ROM_END

/*-------------------------------------------------------------------
/ Lap By Lap (1986)
/-------------------------------------------------------------------*/
ROM_START(lapbylap)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("lblr0.bin", 0x0000, 0x1000, CRC(2970f31a) SHA1(01fb774de19944bb3a19577921f84ab5b6746afb))
	ROM_LOAD("lblr1.bin", 0x1000, 0x1000, CRC(94787c10) SHA1(f2a5b07e57222ee811982eb220c239e34a358d6f))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("lblsr0.bin", 0x0000, 0x2000, CRC(cbaddf02) SHA1(8207eebc414d90328bfd521190d508b88bb870a2))
ROM_END

/*-------------------------------------------------------------------
/ 250 CC (1992)
/-------------------------------------------------------------------*/
ROM_START(ind250cc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-250cc.bin", 0x0000, 0x2000, CRC(753d82ec) SHA1(61950336ba571f9f75f2fc31ccb7beaf4e05dddc))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("a-250cc.bin", 0x0000, 0x2000, CRC(b64bdafb) SHA1(eab6d54d34b44187d454c1999e4bcf455183d5a0))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("b-250cc.bin", 0x00000, 0x10000, CRC(884c31c8) SHA1(23a838f1f0cb4905fa8552579b5452134f0fc9cc))
	ROM_LOAD("c-250cc.bin", 0x10000, 0x10000, CRC(5a1dfa1d) SHA1(4957431d87be0bb6d27910b718f7b7edcd405fff))
	ROM_LOAD("d-250cc.bin", 0x20000, 0x10000, CRC(a0940387) SHA1(0e06483e3e823bf4673d8e0bd120b0a6b802035d))
	ROM_LOAD("e-250cc.bin", 0x30000, 0x10000, CRC(538b3274) SHA1(eb76c41a60199bb94aec4666222e405bbcc33494))
ROM_END

/*-------------------------------------------------------------------
/ Atleta (1991)
/-------------------------------------------------------------------*/
ROM_START(atleta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atleta0.cpu", 0x0000, 0x2000, CRC(5f27240f) SHA1(8b77862fa311d703b3af8a1db17e13b17dca7ec6))
	ROM_LOAD("atleta1.cpu", 0x2000, 0x2000, CRC(12bef582) SHA1(45e1da318141d9228bc91a4e09fff6bf6f194235))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("atletaa.snd", 0x0000, 0x2000, CRC(051c5329) SHA1(339115af4a2e3f1f2c31073cbed1842518d5916e))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("atletab.snd", 0x00000, 0x10000, CRC(7f155828) SHA1(e459c81b2c2e47d4276344d8d6a08c2c6242f941))
	ROM_LOAD("atletac.snd", 0x10000, 0x10000, CRC(20456363) SHA1(b226400dac35dedc039a7e03cb525c6033b24ebc))
	ROM_LOAD("atletad.snd", 0x20000, 0x10000, CRC(6518e3a4) SHA1(6b1d852005dabb76c7c65b87ecc9ee1422f16737))
	ROM_LOAD("atletae.snd", 0x30000, 0x10000, CRC(1ef7b099) SHA1(08400db3e238baf1673a2da604c999db6be30ffe))
ROM_END


/*-------------------------------------------------------------------
/ Clown (1988)
/-------------------------------------------------------------------*/
ROM_START(pinclown)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("clown_a.bin", 0x0000, 0x2000, CRC(b7c3f9ab) SHA1(89ede10d9e108089da501b28f53cd7849f791a00))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("clown_b.bin", 0x0000, 0x2000, CRC(81a66302) SHA1(3d1243ae878747f20e54cd3322c5a54ded45ce21))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("clown_c.bin", 0x00000, 0x10000, CRC(dff89319) SHA1(3745a02c3755d11ea7fb552f7a5df2e8bbee2c29))
	ROM_LOAD("clown_d.bin", 0x10000, 0x10000, CRC(cce4e1dc) SHA1(561c9331d2d110d34cf250cd7b25be16a72a1d79))
	ROM_LOAD("clown_e.bin", 0x20000, 0x10000, CRC(98263526) SHA1(509764e65847637824ba93f7e6ce926501c431ce))
	ROM_LOAD("clown_f.bin", 0x30000, 0x10000, CRC(5f01b531) SHA1(116b1670ef4d5c054bb09dc55aa7d5d3ca047079))
ROM_END

/*-------------------------------------------------------------------
/ Corsario (1989)
/-------------------------------------------------------------------*/
ROM_START(corsario)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-corsar.bin", 0x0000, 0x2000, CRC(800f6895) SHA1(a222e7ea959629202686815646fc917ffc5a646c))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("a-corsar.bin", 0x0000, 0x2000, CRC(e14b7918) SHA1(5a5fc308b0b70fe041b81071ba4820782b6ff988))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("b-corsar.bin", 0x00000, 0x10000, CRC(7f155828) SHA1(e459c81b2c2e47d4276344d8d6a08c2c6242f941))
	ROM_LOAD("c-corsar.bin", 0x10000, 0x10000, CRC(047fd722) SHA1(2385507459f85c68141adc7084cb51dfa02462f6))
	ROM_LOAD("d-corsar.bin", 0x20000, 0x10000, CRC(10d8b448) SHA1(ed1918e6c55eba07dde31b9755c9403e073cad98))
	ROM_LOAD("e-corsar.bin", 0x30000, 0x10000, CRC(918ee349) SHA1(17cded8b5626c91e400d26332a160704f2fd2b55))
ROM_END

/*-------------------------------------------------------------------
/ Mundial 90 (1990)
/-------------------------------------------------------------------*/
ROM_START(mundial)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mundial.cpu", 0x0000, 0x2000, CRC(b615e69b) SHA1(d129eb6f2943af40ddffd0da1e7a711b58f65b3c))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("snd11.bin", 0x0000, 0x2000, CRC(2cebc1a5) SHA1(e0dae2b1ce31ff436b55ceb1ec71d39fc56694da))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("snd24.bin", 0x00000, 0x10000, CRC(603bfc3c) SHA1(8badd9731243270ce5b8003373ed09ec7eac6ca6))
	ROM_LOAD("snd23.bin", 0x10000, 0x10000, CRC(2868ce6f) SHA1(317457763f764be08cbe6a5dd4008ba2257c9d78))
	ROM_LOAD("snd22.bin", 0x20000, 0x10000, CRC(2559f874) SHA1(cbf57f29e394d5dc320e7dcbd2625f6c96412a06))
	ROM_LOAD("snd21.bin", 0x30000, 0x10000, CRC(7a8f7402) SHA1(39666ba2634fe9c720c2c9bcc9ccc73874ed85e7))
ROM_END

/*-------------------------------------------------------------------
/ Moon Light (1987)
/-------------------------------------------------------------------*/
ROM_START(pinmoonl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ci-3.bin", 0x0000, 0x2000, CRC(56b901ae) SHA1(7269d1a100c378b21454f9f80f5bd9fbb736c222))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("ci-11.bin", 0x0000, 0x2000, CRC(a0732fe4) SHA1(54f62cd81bdb7e1924acb67ddbe43eb3d0a4eab0))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("ci-24.bin", 0x00000, 0x10000, CRC(6406bd18) SHA1(ae45ed9e8b1fd278a36a68b780352dbbb6ee781e))
	ROM_LOAD("ci-23.bin", 0x10000, 0x10000, CRC(eac346da) SHA1(7c4c26ae089dda0dcd7300fd1ecabf5a91099c41))
	ROM_LOAD("ci-22.bin", 0x20000, 0x10000, CRC(379740da) SHA1(83ad13ab7f1f37c78397d8e830bd74c5a7aea758))
	ROM_LOAD("ci-21.bin", 0x30000, 0x10000, CRC(0febb4a7) SHA1(e6cc1b26ddfe9cd58da29de2a50a83ce50afe323))
ROM_END

/*-------------------------------------------------------------------
/ Metal Man (1992)
/-------------------------------------------------------------------*/
ROM_START(metalman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_0.bin", 0x00000, 0x02000, CRC(7fe4335b) SHA1(52ef2efa29337eebd8c2c9a8aec864356a6829b6))
	ROM_LOAD("cpu_1.bin", 0x02000, 0x02000, CRC(2cca735e) SHA1(6a76017dfbcac0d57fcec8f07f92d5e04dd3e00b))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("sound_e1.bin", 0x0000, 0x2000, CRC(55e889e8) SHA1(0a240868c1b17762588c0ed9a14f568a6e50f409))

	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD("sound_e2.bin", 0x00000, 0x20000, CRC(5ac61535) SHA1(75b9a805f8639554251192e3777073c29952c78f))

	ROM_REGION(0x10000, "soundcpu2", 0)
	ROM_LOAD("sound_m1.bin", 0x00000, 0x02000, CRC(21a9ee1d) SHA1(d906ac7d6e741f05e81076a5be33fc763f0de9c1))

	ROM_REGION(0x80000, "user2", 0)
	ROM_LOAD("sound_m2.bin", 0x00000, 0x20000, CRC(349df1fe) SHA1(47e7ddbdc398396e40bb5340e5edcb8baf06c255))
	ROM_LOAD("sound_m3.bin", 0x40000, 0x20000, CRC(4d9f5ed2) SHA1(bc6b7c70369c25eddddac5304497f30cee7675d4))
ROM_END

// sn76489
GAME(1985,  brvteam,    0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Brave Team",         GAME_MECHANICAL)

// ay8910
GAME(1986,  canasta,    0,    canasta,  canasta,  inder_state, inder,  ROT0, "Inder", "Canasta '86'",       GAME_MECHANICAL | GAME_NOT_WORKING )

// sound cpu with 2x ay8910
GAME(1986,  lapbylap,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Lap By Lap",         GAME_IS_SKELETON_MECHANICAL)
GAME(1987,  pinmoonl,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Moon Light (Inder)", GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  pinclown,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Clown (Inder)",      GAME_IS_SKELETON_MECHANICAL)
GAME(1989,  corsario,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Corsario",           GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  mundial,    0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Mundial 90",         GAME_IS_SKELETON_MECHANICAL)
GAME(1991,  atleta,     0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Atleta",             GAME_IS_SKELETON_MECHANICAL)
GAME(1992,  ind250cc,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "250 CC",             GAME_IS_SKELETON_MECHANICAL)
GAME(1992,  metalman,   0,    inder,    inder,    inder_state, inder,  ROT0, "Inder", "Metal Man",          GAME_IS_SKELETON_MECHANICAL)
