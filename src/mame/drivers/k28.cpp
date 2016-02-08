// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  K28

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/tms6100.h"
#include "sound/votrax.h"

#include "k28.lh"


class k28_state : public driver_device
{
public:
	k28_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms6100(*this, "tms6100"),
		m_speech(*this, "speech"),
		m_inp_matrix(*this, "IN")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<tms6100_device> m_tms6100;
	required_device<votrax_sc01_device> m_speech;
	required_ioport_array<7> m_inp_matrix;

	UINT8 m_inp_mux;
	UINT8 m_phoneme;
	int m_speech_strobe;

	DECLARE_WRITE8_MEMBER(mcu_p0_w);
	DECLARE_READ8_MEMBER(mcu_p1_r);
	DECLARE_READ8_MEMBER(mcu_p2_r);
	DECLARE_WRITE8_MEMBER(mcu_p2_w);
	DECLARE_WRITE8_MEMBER(mcu_prog_w);
	DECLARE_READ8_MEMBER(mcu_t1_r);

protected:
	virtual void machine_start() override;
};

void k28_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_speech_strobe = 0;
	m_phoneme = 0x3f;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_speech_strobe));
	save_item(NAME(m_phoneme));
}


/***************************************************************************

  I/O, Address Map(s)

***************************************************************************/

WRITE8_MEMBER(k28_state::mcu_p0_w)
{
	// d0,d1: phoneme high bits
	// d0-d2: input mux high bits
	m_inp_mux = (m_inp_mux & 0xf) | (~data << 4 & 0x70);
	m_phoneme = (m_phoneme & 0xf) | (data << 4 & 0x30);
	
	// d3: SC-01 strobe, latch phoneme on rising edge
	if (data & 8 && m_speech_strobe == 0)
		m_speech->write(space, 0, m_phoneme);
	m_speech_strobe = data & 8;
	
	//printf("%d",data>>4&1);
	
	// d4: VSM chip enable
	// d6: VSM M0
	// d7: VSM M1
	m_tms6100->cs_w(~data >> 4 & 1);
	m_tms6100->m0_w(data >> 6 & 1);
	m_tms6100->m1_w(data >> 7 & 1);
	m_tms6100->clk_w(1);
	m_tms6100->clk_w(0);
}

READ8_MEMBER(k28_state::mcu_p1_r)
{
	UINT8 data = 0;
	
	// multiplexed inputs (active low)
	for (int i = 0; i < 7; i++)
		if (m_inp_mux >> i & 1)
			data |= m_inp_matrix[i]->read();
	
	return data ^ 0xff;
}

READ8_MEMBER(k28_state::mcu_p2_r)
{
	// d3: VSM data
	return (m_tms6100->data_line_r()) ? 8 : 0;
}

WRITE8_MEMBER(k28_state::mcu_p2_w)
{
	// d0-d3: VSM data, input mux and SC-01 phoneme lower nibble
	m_tms6100->add_w(space, 0, data);
	m_inp_mux = (m_inp_mux & ~0xf) | (~data & 0xf);
	m_phoneme = (m_phoneme & ~0xf) | (data & 0xf);
}

WRITE8_MEMBER(k28_state::mcu_prog_w)
{
	// 8021 PROG: MM5445 CLK pin
}

READ8_MEMBER(k28_state::mcu_t1_r)
{
	printf("1");
	
	// 8021 T1: SC-01 A/R pin
	return m_speech->request();
}


static ADDRESS_MAP_START( k28_mcu_map, AS_IO, 8, k28_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_WRITE(mcu_p0_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(mcu_p1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(mcu_p2_r, mcu_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_WRITE(mcu_prog_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(mcu_t1_r)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( k28 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( k28, k28_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8021, XTAL_3_579545MHz)
	MCFG_CPU_IO_MAP(k28_mcu_map)

	MCFG_DEVICE_ADD("tms6100", TMS6100, XTAL_3_579545MHz)
	MCFG_DEFAULT_LAYOUT(layout_k28)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DEVICE_ADD("speech", VOTRAX_SC01, 760000) // measured 760kHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/***************************************************************************

  ROM Defs, Game driver(s)

***************************************************************************/

ROM_START( k28 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "k28_8021.bin", 0x0000, 0x0400, CRC(15536d20) SHA1(fac98ce652340ffb2d00952697c3a9ce75393fa4) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff? = space reserved for cartridge
	ROM_LOAD( "cm62050.vsm", 0x0000, 0x4000, CRC(6afb8645) SHA1(e22435568ed11c6516a3b4008131f99cd4e47aa9) )
	ROM_LOAD( "cm62051.vsm", 0x4000, 0x4000, CRC(0fa61baa) SHA1(831be669423ba60c7f85a896b4b09a1295478bd9) )
ROM_END



COMP( 1981, k28, 0, 0, k28, k28, driver_device, 0, "Tiger Electronics", "K28: Talking Learning Computer (model 7-230)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
