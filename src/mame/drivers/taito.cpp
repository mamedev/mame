// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

  PINBALL
  Taito of Brasil

  Unable to locate any schematics that are actually useful.

  Used PinMAME and the Rowamet driver as references.

  You need to have a ball in the outhole (hold down X) when starting a game.

  Need schematics to do this properly.

Status of each game:
- shock, obaoba, drakor, meteort, sureshop, cosmic, vortexp, stest, rally:
    Works, with various quality of sounds
- sharkt, snake:
    As above, but outhole can randomly stop working
- lunelle:
    Works, but play can be interrupted by a large flashing '14'
- ladylukt, vegast, titan, gork:
    Works, no sound
- gemini2k, zarza, cavnegro, hawkman, mrblack, sshuttle, fireactd:
    Can insert a coin but cannot start a game
- fireact:
    Cannot insert a coin
- mrblkz80:
    Different hardware, not emulated


ToDO:
- Inputs
- Outputs
- Sound (need a schematic)
- Display flickers ingame
- Votrax makes continual rattling noise and nothing else
- Some games produce sound, but silence or random sounds often occur, or it just
  cuts out for a while.

*****************************************************************************************/

#include "machine/genpin.h"
#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/votrax.h"
#include "sound/dac.h"
#include "taito.lh"

class taito_state : public genpin_class
{
public:
	taito_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "audiocpu")
		, m_pia(*this, "pia")
		, m_votrax(*this, "votrax")
		, m_p_ram(*this, "ram")
	{ }

	DECLARE_DRIVER_INIT(taito);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(pia_pb_r);
	DECLARE_WRITE8_MEMBER(pia_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(votrax_request);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
private:
	UINT8 m_out_offs;
	UINT8 m_sndcmd;
	UINT8 m_votrax_cmd;
	UINT8 m_io[16];
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_device<pia6821_device> m_pia;
	optional_device<votrax_sc01_device> m_votrax;
	required_shared_ptr<UINT8> m_p_ram;
};


static ADDRESS_MAP_START( taito_map, AS_PROGRAM, 8, taito_state )
	AM_RANGE(0x0000, 0x27ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x0080) AM_READ_PORT("X0")
	AM_RANGE(0x2801, 0x2801) AM_MIRROR(0x0080) AM_READ_PORT("X1")
	AM_RANGE(0x2802, 0x2802) AM_MIRROR(0x0080) AM_READ_PORT("X2")
	AM_RANGE(0x2803, 0x2803) AM_MIRROR(0x0080) AM_READ_PORT("X3")
	AM_RANGE(0x2804, 0x2804) AM_MIRROR(0x0080) AM_READ_PORT("X4")
	AM_RANGE(0x2805, 0x2805) AM_MIRROR(0x0080) AM_READ_PORT("X5")
	AM_RANGE(0x2806, 0x2806) AM_MIRROR(0x0080) AM_READ_PORT("X6")
	AM_RANGE(0x2807, 0x2807) AM_MIRROR(0x0080) AM_READ_PORT("X7")
	AM_RANGE(0x2808, 0x2808) AM_MIRROR(0x0080) AM_READ_PORT("X8")
	AM_RANGE(0x28d8, 0x28d8) AM_MIRROR(0x0080) AM_READ_PORT("X0")
	AM_RANGE(0x28d9, 0x28d9) AM_MIRROR(0x0080) AM_READ_PORT("X1")
	AM_RANGE(0x28da, 0x28da) AM_MIRROR(0x0080) AM_READ_PORT("X2")
	AM_RANGE(0x28db, 0x28db) AM_MIRROR(0x0080) AM_READ_PORT("X3")
	AM_RANGE(0x28dc, 0x28dc) AM_MIRROR(0x0080) AM_READ_PORT("X4")
	AM_RANGE(0x28db, 0x28dd) AM_MIRROR(0x0080) AM_READ_PORT("X5")
	AM_RANGE(0x28de, 0x28de) AM_MIRROR(0x0080) AM_READ_PORT("X6")
	AM_RANGE(0x28df, 0x28df) AM_MIRROR(0x0080) AM_READ_PORT("X7")
	AM_RANGE(0x4000, 0x407f) AM_RAM
	AM_RANGE(0x4080, 0x408f) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x4090, 0x409f) AM_READWRITE(io_r,io_w)
	AM_RANGE(0x40a0, 0x40ff) AM_RAM
	AM_RANGE(0x4800, 0x48ff) AM_ROM AM_REGION("roms", 0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( taito_sub_map, AS_PROGRAM, 8, taito_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x0800, 0x1fff) AM_ROM AM_REGION("cpu2", 0x0800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( taito_sub_map2, AS_PROGRAM, 8, taito_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x2000, 0x3fff) AM_ROM AM_REGION("cpu2", 0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( taito_sub_map5, AS_PROGRAM, 8, taito_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_DEVWRITE("aysnd_0", ay8910_device, address_w)
	AM_RANGE(0x1003, 0x1003) AM_DEVWRITE("aysnd_0", ay8910_device, address_w)
	AM_RANGE(0x1007, 0x1007) AM_DEVREAD("aysnd_0", ay8910_device, data_r)
	AM_RANGE(0x100c, 0x100c) AM_DEVWRITE("aysnd_1", ay8910_device, address_w)
	AM_RANGE(0x100a, 0x100a) AM_DEVWRITE("aysnd_0", ay8910_device, data_w)
	AM_RANGE(0x100b, 0x100b) AM_DEVWRITE("aysnd_0", ay8910_device, data_w)
	AM_RANGE(0x100d, 0x100d) AM_DEVREAD("aysnd_1", ay8910_device, data_r)
	AM_RANGE(0x100e, 0x100e) AM_DEVWRITE("aysnd_1", ay8910_device, data_w)
	AM_RANGE(0x2000, 0x7fff) AM_ROM AM_REGION("cpu2", 0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shock_map, AS_PROGRAM, 8, taito_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x1000, 0x100f) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1010, 0x101f) AM_READWRITE(io_r,io_w)
	AM_RANGE(0x1020, 0x10ff) AM_RAM
	AM_RANGE(0x1400, 0x1400) AM_READ_PORT("X0")
	AM_RANGE(0x1401, 0x1401) AM_READ_PORT("X1")
	AM_RANGE(0x1402, 0x1402) AM_READ_PORT("X2")
	AM_RANGE(0x1404, 0x1404) AM_READ_PORT("X4")
	AM_RANGE(0x1405, 0x1405) AM_READ_PORT("X5")
	AM_RANGE(0x1406, 0x1406) AM_READ_PORT("X6")
	AM_RANGE(0x14d8, 0x14d8) AM_READ_PORT("X0")
	AM_RANGE(0x14d9, 0x14d9) AM_READ_PORT("X1")
	AM_RANGE(0x14da, 0x14da) AM_READ_PORT("X2")
	AM_RANGE(0x14db, 0x14db) AM_READ_PORT("X3")
	AM_RANGE(0x14dc, 0x14dc) AM_READ_PORT("X4")
	AM_RANGE(0x14dd, 0x14dd) AM_READ_PORT("X5")
	AM_RANGE(0x1800, 0x1bff) AM_ROM AM_REGION("roms", 0x1800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shock_sub_map, AS_PROGRAM, 8, taito_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("cpu2", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( taito )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT ) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )
INPUT_PORTS_END

READ8_MEMBER( taito_state::io_r )
{
	return m_io[offset];
}

WRITE8_MEMBER( taito_state::io_w )
{
	m_io[offset] = data;

	if (offset == 2)
	{
		UINT8 cmd = (m_io[2]>>4) | (m_io[3] & 0xf0);
		if (cmd != m_sndcmd)
		{
			m_sndcmd = cmd;
			m_pia->cb1_w(data ? 1 : 0);
		}
	}
}

WRITE_LINE_MEMBER( taito_state::pia_cb2_w )
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	m_votrax->write(space, 0, m_votrax_cmd);
}

READ8_MEMBER( taito_state::pia_pb_r )
{
	return ~m_sndcmd;
}

WRITE8_MEMBER( taito_state::pia_pb_w )
{
	m_votrax_cmd = data;
}

WRITE_LINE_MEMBER( taito_state::votrax_request )
{
	m_pia->ca1_w(state ? 0 : 1);
}

void taito_state::machine_reset()
{
}

DRIVER_INIT_MEMBER( taito_state, taito )
{
}

TIMER_DEVICE_CALLBACK_MEMBER( taito_state::timer_a )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // don't know, 7446 assumed
	m_out_offs &= 15;

	UINT8 digit = m_out_offs << 1;
	output().set_digit_value(digit, patterns[m_p_ram[m_out_offs]>>4]);
	output().set_digit_value(++digit, patterns[m_p_ram[m_out_offs++]&15]);
}

static MACHINE_CONFIG_START( taito, taito_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, 19000000/9)
	MCFG_CPU_PROGRAM_MAP(taito_map)

	MCFG_CPU_ADD("audiocpu", M6802, 1000000) // cpu & clock are a guess
	MCFG_CPU_PROGRAM_MAP(taito_sub_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_taito)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_SPEAKER_STANDARD_MONO("dacsnd")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "dacsnd", 0.95)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(taito_state, pia_pa_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_device, write_unsigned8))
	MCFG_PIA_READPB_HANDLER(READ8(taito_state, pia_pb_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(taito_state, pia_pb_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(taito_state, pia_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(taito_state, pia_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, nmi_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_a", taito_state, timer_a, attotime::from_hz(200))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( shock, taito )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(shock_map)
	MCFG_CPU_MODIFY( "audiocpu" )
	MCFG_CPU_PROGRAM_MAP(shock_sub_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( taito2, taito )
	MCFG_CPU_MODIFY( "audiocpu" )
	MCFG_CPU_PROGRAM_MAP(taito_sub_map2)
MACHINE_CONFIG_END

// add vox
static MACHINE_CONFIG_DERIVED( taito4, taito )
	MCFG_SPEAKER_STANDARD_MONO("voxsnd")
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000) // guess
	MCFG_VOTRAX_SC01_REQUEST_CB(WRITELINE(taito_state, votrax_request))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "voxsnd", 0.15) // todo: fix - it makes noise continuously

	MCFG_DEVICE_REMOVE("pia")
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(taito_state, pia_pa_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_device, write_unsigned8))
	MCFG_PIA_READPB_HANDLER(READ8(taito_state, pia_pb_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(taito_state, pia_pb_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(taito_state, pia_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(taito_state, pia_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, nmi_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( taito_ay_audio )
	MCFG_CPU_MODIFY( "audiocpu" )
	MCFG_CPU_PROGRAM_MAP(taito_sub_map5)

	MCFG_SPEAKER_STANDARD_MONO("aysnd")
	MCFG_SOUND_ADD("aysnd_0", AY8910, XTAL_3_579545MHz/2) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "aysnd", 0.8)
	MCFG_SOUND_ADD("aysnd_1", AY8910, XTAL_3_579545MHz/2) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "aysnd", 0.8)
MACHINE_CONFIG_END

// add ay
static MACHINE_CONFIG_DERIVED( taito5, taito )
	MCFG_FRAGMENT_ADD( taito_ay_audio )
MACHINE_CONFIG_END

// add vox and ay
static MACHINE_CONFIG_DERIVED( taito6, taito4 )
	MCFG_FRAGMENT_ADD( taito_ay_audio )
MACHINE_CONFIG_END




/*--------------------------------
/ Apache
/-------------------------------*/

/*--------------------------------
/ Black Hole
/-------------------------------*/

/*--------------------------------
/ Cavaleiro Negro
/-------------------------------*/
ROM_START(cavnegro)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "cn1.bin", 0x0000, 0x0800, CRC(6b414089) SHA1(5f6042cc85a9319b3e34bdf39fd1f7feb5db0ec2))
	ROM_LOAD( "cn2.bin", 0x0800, 0x0800, CRC(9641f2e5) SHA1(4d7e522bd1d691901868abd191010b62a9032fda))
	ROM_LOAD( "cn3.bin", 0x1000, 0x0800, CRC(4ca99983) SHA1(88c806f013cc31443c842fb7925f97b0ed1bbdc9))
	ROM_LOAD( "cn4.bin", 0x1800, 0x0800, CRC(0cf4c1fa) SHA1(f0170da2c3fb138cc9f6c076a2d3f4fbf529e923))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("cn_s2.bin", 0x1000, 0x0800, CRC(a0508863) SHA1(b4f343ed48960048c6b2b36c5ce0bad0fdb7ac62))
	ROM_LOAD("cn_s1.bin", 0x1800, 0x0800, CRC(aec5069a) SHA1(4ec1f1f054e010caf9ffdda60071f96ba772c01a))
ROM_END

ROM_START(cavnegro1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "cn1.bin", 0x0000, 0x0800, CRC(6b414089) SHA1(5f6042cc85a9319b3e34bdf39fd1f7feb5db0ec2))
	ROM_LOAD( "cn2.bin", 0x0800, 0x0800, CRC(9641f2e5) SHA1(4d7e522bd1d691901868abd191010b62a9032fda))
	ROM_LOAD( "cn3a.bin", 0x1000, 0x0800, CRC(7e489691) SHA1(af020d2a88ade5084508c2d134823af6e5c81b02))
	ROM_LOAD( "cn4a.bin", 0x1800, 0x0800, CRC(0a4c7c00) SHA1(ada0bb7aa33bac6238a9b3e62f0c9b1dffb06194))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("cn_s2.bin", 0x1000, 0x0800, CRC(a0508863) SHA1(b4f343ed48960048c6b2b36c5ce0bad0fdb7ac62))
	ROM_LOAD("cn_s1.bin", 0x1800, 0x0800, CRC(aec5069a) SHA1(4ec1f1f054e010caf9ffdda60071f96ba772c01a))
ROM_END

ROM_START(cavnegro2)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "cn1.bin", 0x0000, 0x0800, CRC(6b414089) SHA1(5f6042cc85a9319b3e34bdf39fd1f7feb5db0ec2))
	ROM_LOAD( "cn2.bin", 0x0800, 0x0800, CRC(9641f2e5) SHA1(4d7e522bd1d691901868abd191010b62a9032fda))
	ROM_LOAD( "cn3b.bin", 0x1000, 0x0800, CRC(e1c5afd8) SHA1(0995325444ada4aa5cd19a90230bcad58c6cd072))
	ROM_LOAD( "cn4b.bin", 0x1800, 0x0800, CRC(b5130b00) SHA1(79efae0e8041dc152b68b304c632c9de857ad620))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("cn_s2.bin", 0x1000, 0x0800, CRC(a0508863) SHA1(b4f343ed48960048c6b2b36c5ce0bad0fdb7ac62))
	ROM_LOAD("cn_s1.bin", 0x1800, 0x0800, CRC(aec5069a) SHA1(4ec1f1f054e010caf9ffdda60071f96ba772c01a))
ROM_END

/*--------------------------------
/ Cosmic
/-------------------------------*/
ROM_START(cosmic)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "cosmic1.bin", 0x0000, 0x0800, CRC(1864f295) SHA1(f92fb88a945a946536c50e6b6ccc99ef34f5cdb9))
	ROM_LOAD( "cosmic2.bin", 0x0800, 0x0800, CRC(818e8621) SHA1(4c1dbb1504487ef5c75ddcedf92c803739490806))
	ROM_LOAD( "cosmic3.bin", 0x1000, 0x0800, CRC(c3e0cf5d) SHA1(9b0a6174a1fcb8934a91679645b64b7d9abaa705))
	ROM_LOAD( "cosmic4.bin", 0x1800, 0x0800, CRC(09ed5ecd) SHA1(182f0b01b9dad229e1a323253b32105098bdcfe7))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("cosmc_s2.bin", 0x1000, 0x0800, CRC(84b98b95) SHA1(1946856de6d1ae05888826416bef9bdb25d652ed))
	ROM_LOAD("cosmc_s1.bin", 0x1800, 0x0800, CRC(09f082c1) SHA1(653d6f9f9cc62b46aa2df2fa8dd0ad4e1e9f7c49))
ROM_END

/*--------------------------------
/ Drakor
/-------------------------------*/
ROM_START(drakor)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "drakor1.bin", 0x0000, 0x0800, CRC(7ecf377b) SHA1(b55b0ae3b591768621553a2b0afd1a795b4d592b))
	ROM_LOAD( "drakor2.bin", 0x0800, 0x0800, CRC(91dbb199) SHA1(fa351462c5616f591b7705259dfe96e97eda5548))
	ROM_LOAD( "drakor3.bin", 0x1000, 0x0800, CRC(b0ba866e) SHA1(dfea60523578b8def310922d17f442a8a031bba1))
	ROM_RELOAD( 0x1800, 0x0800)

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("drako_s1.bin", 0x1800, 0x0800, CRC(5cd9452e) SHA1(fdef06f823204174a144bc36e94a977386121f64))
ROM_END

/*--------------------------------
/ Fire Action
/-------------------------------*/
ROM_START(fireact)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "fire1.bin", 0x0000, 0x0800, CRC(3059876d) SHA1(1ea214b592adb156c8e9df7fafa59d9ed059f112))
	ROM_LOAD( "fire2.bin", 0x0800, 0x0800, CRC(7906a193) SHA1(9555233f24f044972fd7267ba970108695f52fb1))
	ROM_LOAD( "fire3.bin", 0x1000, 0x0800, CRC(92135de4) SHA1(28b3b496ae8a404542fc2b0128f3f88229d91cba))
	ROM_LOAD( "fire4.bin", 0x1800, 0x0800, CRC(68de7753) SHA1(b829ddc7e94d00b854e9290acc034038a60a8c1d))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("fire_s2.bin", 0x1000, 0x0800, CRC(b76bda3f) SHA1(be5dfa3caa3b29a40287d535d158599587af8c05))
	ROM_LOAD("fire_s1.bin", 0x1800, 0x0800, CRC(13bdd72a) SHA1(f271bfe61617293b28b1a8ea7da9035127870d6c))
ROM_END

/*--------------------------------
/ Fire Action Deluxe
/-------------------------------*/
ROM_START(fireactd)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "fired1.bin", 0x0000, 0x0800, CRC(2f923913) SHA1(c35dcf37e6957523f6762b95f5f6503037b607d6))
	ROM_LOAD( "fired2.bin", 0x0800, 0x0800, CRC(4d268048) SHA1(f1c4cb1c91f73e2a145725b4696b7996d311883f))
	ROM_LOAD( "fired3.bin", 0x1000, 0x0800, CRC(f5e07ed1) SHA1(3da566ea2fb56998fc56db3f373ec813b5b627e1))
	ROM_LOAD( "fired4.bin", 0x1800, 0x0800, CRC(da1a4ed5) SHA1(e39be103dfcfa004061d2249292b023bc3fac9bd))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("fired_s1.bin", 0x5000, 0x1000, CRC(b821d324) SHA1(db00416592467a5917dd75e437842aea822fffa8))
	ROM_LOAD("fired_s2.bin", 0x6000, 0x1000, CRC(d427d0f6) SHA1(bcd1cf15f4ff1df30a42d8889879cff9d3f16e6e))
	ROM_LOAD("fired_s3.bin", 0x7000, 0x1000, CRC(ecff8399) SHA1(7615da5a6952cbc0769963a9563017bd46e4a73f))
ROM_END

/*--------------------------------
/ Football
/-------------------------------*/

/*--------------------------------
/ Gemini 2000
/-------------------------------*/
ROM_START(gemini2k)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "gemini1.bin", 0x0000, 0x0800, CRC(4f952799) SHA1(8433850945d020253090d829a70fba1c9f9eaa5c))
	ROM_LOAD( "gemini2.bin", 0x0800, 0x0800, CRC(8903ee53) SHA1(81f0c02872327b2b589001265f2761666bf45ba2))
	ROM_LOAD( "gemini3.bin", 0x1000, 0x0800, CRC(1f11b5e5) SHA1(043dd68e51428e9123cb3c50c499b87478062c86))
	ROM_LOAD( "gemini4.bin", 0x1800, 0x0800, CRC(cac64ea6) SHA1(eed32defaa03394395d7b9d7bbdc205004789337))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("gemin_s2.bin", 0x1000, 0x0800, CRC(312a5c35) SHA1(82be0ca6f4430e54bbf963a879b85636537146a1))
	ROM_LOAD("gemin_s1.bin", 0x1800, 0x0800, CRC(b9a80ab2) SHA1(9fdfeae5c9bc735e6a9ad42d925a1217c30a3386))
ROM_END

ROM_START(gemini2k1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "gemini1a.bin", 0x0000, 0x0800, CRC(947017c5) SHA1(81456bc0f09e2d3418941b3d254ba1d4999a2fea))
	ROM_LOAD( "gemini2.bin", 0x0800, 0x0800, CRC(8903ee53) SHA1(81f0c02872327b2b589001265f2761666bf45ba2))
	ROM_LOAD( "gemini3.bin", 0x1000, 0x0800, CRC(1f11b5e5) SHA1(043dd68e51428e9123cb3c50c499b87478062c86))
	ROM_LOAD( "gemini4a.bin", 0x1800, 0x0800, CRC(63d3a705) SHA1(157e45d05afde69dedb43c5987ad4f6e9c1e228b))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("gemin_s2.bin", 0x1000, 0x0800, CRC(312a5c35) SHA1(82be0ca6f4430e54bbf963a879b85636537146a1))
	ROM_LOAD("gemin_s1.bin", 0x1800, 0x0800, CRC(b9a80ab2) SHA1(9fdfeae5c9bc735e6a9ad42d925a1217c30a3386))
ROM_END

/*--------------------------------
/ Gork
/-------------------------------*/
ROM_START(gork)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "gork1.bin", 0x0000, 0x0800, CRC(d8c7bfee) SHA1(96319e60cf77d0cb7afc326de785d5255f73623f))
	ROM_LOAD( "gork2.bin", 0x0800, 0x0800, CRC(540abe17) SHA1(ee0ea029ba4b4de5f69146b7ccf9482b4812ef4f))
	ROM_LOAD( "gork3.bin", 0x1000, 0x0800, CRC(0ea1a2dc) SHA1(3ab58bc25a4512aae5c16f497bddf713413c02fe))
	ROM_LOAD( "gork4.bin", 0x1800, 0x0800, CRC(0e6260fb) SHA1(b2f7190991d63701210a25a3970293b8f4c34022))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("gork_s1.bin", 0x2000, 0x1000, CRC(6611a4cb) SHA1(3ab840b162f9bfe2aebe1d3afeb1fddaf849d9c5))
	ROM_LOAD("gork_s2.bin", 0x3000, 0x1000, CRC(440739cb) SHA1(6172bf000f854ccf5c24c7700a0ad208596d24f8))
	ROM_RELOAD( 0x7000, 0x1000)
ROM_END

/*--------------------------------
/ Hawkman
/-------------------------------*/
ROM_START(hawkman)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "hawk1.bin", 0x0000, 0x0800, CRC(cf991a68) SHA1(491d6776685b3664fae104ff3011ca3e5b0ffd41))
	ROM_LOAD( "hawk2.bin", 0x0800, 0x0800, CRC(568ac529) SHA1(d1f8034c9980f4a525d55189f68ab2a63abcf2a5))
	ROM_LOAD( "hawk3.bin", 0x1000, 0x0800, CRC(14be7e31) SHA1(86877bedb2df6edefc436dea20fcf04bf5a31641))
	ROM_LOAD( "hawk4.bin", 0x1800, 0x0800, CRC(e6df08a5) SHA1(bc1f7042b404d01c0cc8cccf1fdf1f42f37f8e02))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("hawk_s2.bin", 0x1000, 0x0800, CRC(29bef82f) SHA1(5f393cc1cb6047cba1186e332e840bce8e59509b))
	ROM_LOAD("hawk_s1.bin", 0x1800, 0x0800, CRC(47549394) SHA1(f5731200db73e8751d2ec4a072b679127b6f0afa))
ROM_END

ROM_START(hawkman1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "hawk1a.bin", 0x0000, 0x0800, CRC(b4fe0cbd) SHA1(5b0cdcbcc144eb94d3c6be8d1282488d54e8578e))
	ROM_LOAD( "hawk2.bin", 0x0800, 0x0800, CRC(568ac529) SHA1(d1f8034c9980f4a525d55189f68ab2a63abcf2a5))
	ROM_LOAD( "hawk3.bin", 0x1000, 0x0800, CRC(14be7e31) SHA1(86877bedb2df6edefc436dea20fcf04bf5a31641))
	ROM_LOAD( "hawk4a.bin", 0x1800, 0x0800, CRC(a5928ac3) SHA1(598462783fb27c6657ca0eac2d5daef8eff8e5c9))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("hawk_s2.bin", 0x1000, 0x0800, CRC(29bef82f) SHA1(5f393cc1cb6047cba1186e332e840bce8e59509b))
	ROM_LOAD("hawk_s1.bin", 0x1800, 0x0800, CRC(47549394) SHA1(f5731200db73e8751d2ec4a072b679127b6f0afa))
ROM_END

/*--------------------------------
/ Hot Ball
/-------------------------------*/

/*--------------------------------
/ Lady Luck
/-------------------------------*/
ROM_START(ladylukt)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "lluck1.bin", 0x0000, 0x0800, CRC(be242895) SHA1(0528e9049e44b5ae7bba4a21ca5c0a2e5ffa4ca5))
	ROM_LOAD( "lluck2.bin", 0x0800, 0x0800, CRC(48169726) SHA1(282a322178e007df1183620dfcf3411bc67d8a0a))
	ROM_LOAD( "lluck3.bin", 0x1000, 0x0800, CRC(f22666f6) SHA1(2b92007cc4c91a2804d9f6229fa68be35be849ce))
	ROM_LOAD( "lluck4.bin", 0x1800, 0x0800, CRC(1715ee7e) SHA1(45677053f501d687d7482e70b7902a67d277eee9))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("lluck_s2.bin", 0x1000, 0x0800, CRC(b0b05e9f) SHA1(1b5b5701ece241913367960eba7f58ca1a528548))
	ROM_LOAD("lluck_s1.bin", 0x1800, 0x0800, CRC(78ed85b4) SHA1(72fee3e337f2d2174a41434084699c3a472d798e))
ROM_END

/*--------------------------------
/ Lunelle
/-------------------------------*/
ROM_START(lunelle)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "lunelle1.bin", 0x0000, 0x0800, CRC(d471349a) SHA1(fb43daa94035dc3abe0e0b16cbb239d7f97437ea))
	ROM_LOAD( "lunelle2.bin", 0x0800, 0x0800, CRC(83b132a3) SHA1(ab52f7ae20a823a9bc2986a32ef4e32a3ec2acd4))
	ROM_LOAD( "lunelle3.bin", 0x1000, 0x0800, CRC(69ec6079) SHA1(df36daa221d27f97f69231c19cbbb80347f51dd3))
	ROM_LOAD( "lunelle4.bin", 0x1800, 0x0800, CRC(492f5de7) SHA1(5bfa0a7b1e3612baebc4c598b43121e7846ae0ff))

	ROM_REGION(0x4000, "cpu2", 0)
	ROM_LOAD("lunel_s1.bin", 0x2000, 0x1000, CRC(910dfa3a) SHA1(a0694c90b4de7a02f9032c7b07d09194739640e7))
	ROM_LOAD("lunel_s2.bin", 0x3000, 0x1000, CRC(3c57b605) SHA1(b119cb5c93c035c8ffd68071d4e9f92a45a18f7f))
ROM_END

/*--------------------------------
/ Meteor
/-------------------------------*/
ROM_START(meteort)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "meteor1.bin", 0x0000, 0x0800, CRC(301a9f94) SHA1(7619b975c13c65e8c57ca50e77dc6385c5c5be49))
	ROM_LOAD( "meteor2.bin", 0x0800, 0x0800, CRC(6d136853) SHA1(f8fa555570b877c37457d84c41b1efca08ead612))
	ROM_LOAD( "meteor3.bin", 0x1000, 0x0800, CRC(c818e889) SHA1(40350e168c0e19edd5a8d11f11d76ed6cc5e4169))
	ROM_RELOAD( 0x1800, 0x0800)

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("meteo_s1.bin", 0x1800, 0x0800, CRC(23971d1e) SHA1(77b5b8855e28cdd9b31b7e33f61258716738d57d))
ROM_END

/*--------------------------------
/ Mr. Black
/-------------------------------*/
ROM_START(mrblack)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "mrb1.bin", 0x0000, 0x0800, CRC(c2a43f6f) SHA1(14a461b6416e3b024cc3d7743b75e29ca1876b64))
	ROM_LOAD( "mrb2.bin", 0x0800, 0x0800, CRC(ddf2a88e) SHA1(8de67f4032811ec3b7da1655207d05e52d4e5e01))
	ROM_LOAD( "mrb3.bin", 0x1000, 0x0800, CRC(f319f68f) SHA1(f4b408837eeab8a7cd7dedc031f0b9332363a7d4))
	ROM_LOAD( "mrb4.bin", 0x1800, 0x0800, CRC(84367699) SHA1(a9a7b21fe31f12b0888bc3bbf82d0b13cf8bad49))
	ROM_LOAD( "mrb5.bin", 0x2000, 0x0800, CRC(a22ee400) SHA1(d55a60ef68d8b671764d79c5ccaeacc8d9821040))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("mrb_s1.bin", 0x5000, 0x1000, CRC(ff28b2b9) SHA1(3106811740e0206ad4ba7845e204e721b0da70e2))
	ROM_LOAD("mrb_s2.bin", 0x6000, 0x1000, CRC(34d52449) SHA1(bdd5db5e58ca997d413d18f291928ad1a45c194e))
	ROM_LOAD("mrb_s3.bin", 0x7000, 0x1000, CRC(276fb897) SHA1(b1a4323a4d921e3ae4beefaa04cd95e18cc33b9d))
ROM_END

ROM_START(mrblack1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "mrb1a.bin", 0x0000, 0x0800, CRC(a97c986a) SHA1(315b3410eb495aa471da20bc199754ff0d8e9a3b))
	ROM_LOAD( "mrb2.bin", 0x0800, 0x0800, CRC(ddf2a88e) SHA1(8de67f4032811ec3b7da1655207d05e52d4e5e01))
	ROM_LOAD( "mrb3.bin", 0x1000, 0x0800, CRC(f319f68f) SHA1(f4b408837eeab8a7cd7dedc031f0b9332363a7d4))
	ROM_LOAD( "mrb4.bin", 0x1800, 0x0800, CRC(84367699) SHA1(a9a7b21fe31f12b0888bc3bbf82d0b13cf8bad49))
	ROM_LOAD( "mrb5a.bin", 0x2000, 0x0800, CRC(18d8f2cc) SHA1(e14c20440753a1996e618e407ef97f3059775c46))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("mrb_s1.bin", 0x5000, 0x1000, CRC(ff28b2b9) SHA1(3106811740e0206ad4ba7845e204e721b0da70e2))
	ROM_LOAD("mrb_s2.bin", 0x6000, 0x1000, CRC(34d52449) SHA1(bdd5db5e58ca997d413d18f291928ad1a45c194e))
	ROM_LOAD("mrb_s3.bin", 0x7000, 0x1000, CRC(276fb897) SHA1(b1a4323a4d921e3ae4beefaa04cd95e18cc33b9d))
ROM_END

/*--------------------------------
/ Oba-Oba
/-------------------------------*/
ROM_START(obaoba)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "ob1.bin", 0x0000, 0x0800, CRC(85cddf4f) SHA1(25c7146b0ec79740704d62878f113dd43918021b))
	ROM_LOAD( "ob2.bin", 0x0800, 0x0800, CRC(7a110b82) SHA1(67cb34603de689438ecae8877674f01273bc711f))
	ROM_LOAD( "ob3.bin", 0x1000, 0x0800, CRC(8f32a7c0) SHA1(378a5434d3f4fe1b07f0116f2558bda030d2258c))
	ROM_RELOAD( 0x1800, 0x0800)

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("ob_s2.bin", 0x1000, 0x0800, CRC(f7dbb715) SHA1(70d1331612fe497f48520726c5f39accdcbdb205))
	ROM_LOAD("ob_s1.bin", 0x1800, 0x0800, CRC(812a362b) SHA1(22b5f5f2d467ca1b0ab55db2e01ef6579f8ee390))
ROM_END

ROM_START(obaoba1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "ob1a.bin", 0x0000, 0x0800, CRC(f5a468d6) SHA1(01108281298fd092834f3a771eeda85b34a21745))
	ROM_LOAD( "ob2a.bin", 0x0800, 0x0800, CRC(a2cb84ad) SHA1(f7efb4474a8b3ca79e9f37ca342f8373fcbde56d))
	ROM_LOAD( "ob3a.bin", 0x1000, 0x0800, CRC(9fe1e0fd) SHA1(e0ae32ed1f45fbf9de4daa73f662e4e2c91d5c0b))
	ROM_RELOAD( 0x1800, 0x0800)

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("ob_s2a.bin", 0x1000, 0x0800, CRC(08d22ca7) SHA1(9121f0d21a796c10adf443b63e1c5451468d9f9f))
	ROM_LOAD("ob_s1a.bin", 0x1800, 0x0800, CRC(fa106de6) SHA1(be4dee9c2f10cf64a3b71cf65386e02323f040c7))
ROM_END

/*--------------------------------
/ Polar Explorer
/-------------------------------*/
ROM_START(polar)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "polar1.bin", 0x0000, 0x0800, CRC(f92944b6) SHA1(04ff22977a5036eee46a9e1decd2ec4d7046eb0d))
	ROM_LOAD( "polar2.bin", 0x0800, 0x0800, CRC(e6391071) SHA1(2793ad9ee3018069a93c739daca03787f7d81de7))
	ROM_LOAD( "polar3.bin", 0x1000, 0x0800, CRC(318d0702) SHA1(27c4856ea098286142c70552f07fd689e35d5288))
	ROM_LOAD( "polar4.bin", 0x1800, 0x0800, CRC(1c02f0c9) SHA1(663c1f4841cb0bd7139e4063d4e7e35a51470686))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("polar_s1.bin", 0x5000, 0x1000, CRC(baff1a67) SHA1(d93736b8d232034047f463b43ac51f9fd4a28536))
	ROM_LOAD("polar_s2.bin", 0x6000, 0x1000, CRC(84fe1dc8) SHA1(96f52fc9245d0f7626da9cf41979c5a84a63f4bb))
	ROM_LOAD("polar_s3.bin", 0x7000, 0x1000, CRC(d574bc94) SHA1(f6060b60708cebd1d546dc5b9e3cec0781454af5))
ROM_END

/*--------------------------------
/ Rally
/-------------------------------*/
ROM_START(rally)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "rally1.bin", 0x0000, 0x0800, CRC(d0d6b32e) SHA1(ef144de5916b78ceabcea19465c23567473a41d5))
	ROM_LOAD( "rally2.bin", 0x0800, 0x0800, CRC(e7611e06) SHA1(5443c255eea2b3e0778d63064cf952259862170e))
	ROM_LOAD( "rally3.bin", 0x1000, 0x0800, CRC(45d28cd3) SHA1(dda00ac5aad24a359ff894a2abe0db967826165d))
	ROM_LOAD( "rally4.bin", 0x1800, 0x0800, CRC(7fb471ee) SHA1(d161836528380b3d18606aa082dfc1d7a5959147))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("rally_s2.bin", 0x1000, 0x0800, CRC(a409d9d1) SHA1(3005cfaedd6edf3d80cac539563655f3bcc342ca))
	ROM_LOAD("rally_s1.bin", 0x1800, 0x0800, CRC(0c7ca1bc) SHA1(09df10b1b295b9a7f5c337eb4f1e1e4db0f3d113))
ROM_END

/*--------------------------------
/ Shark (Taito)
/-------------------------------*/
ROM_START(sharkt)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "shark1.bin", 0x0000, 0x0800, CRC(efe19b88) SHA1(a206537aad1e27abc86eb5366bdde7da8bb03726))
	ROM_LOAD( "shark2.bin", 0x0800, 0x0800, CRC(ab11c287) SHA1(958279e0cd610fb5522eccc9764ecbaaefb6c744))
	ROM_LOAD( "shark3.bin", 0x1000, 0x0800, CRC(7ccf945b) SHA1(683d8d8e4ec9c36dcf4cad240644d54f580a8bb6))
	ROM_LOAD( "shark4.bin", 0x1800, 0x0800, CRC(8ca33f37) SHA1(ec08923fb04c92f4f01a8289f924792708869cf2))

	ROM_REGION(0x4000, "cpu2", 0)
	ROM_LOAD("shark_s1.bin", 0x3000, 0x1000, CRC(75969a7d) SHA1(a37ec84641172ec7a7936fee10c1a36d567d33bb))
ROM_END

/*--------------------------------
/ Shock
/-------------------------------*/
ROM_START(shock)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "shock1.bin", 0x0000, 0x0400, CRC(d844287a) SHA1(c2ff9e2585fc625623c6351c74063f7a09f80cd7))
	ROM_LOAD( "shock2.bin", 0x0400, 0x0400, CRC(068b84c7) SHA1(622bd3b24df175cd783cdf46e5b7e910159d2bea))
	ROM_LOAD( "shock3.bin", 0x0800, 0x0400, CRC(a7f0e116) SHA1(bdb5d6120f7802ce4e1dad434158010b3150233a))
	ROM_LOAD( "shock4.bin", 0x0c00, 0x0400, CRC(549cc14f) SHA1(38ce6ed4cf330a5596394c752257ac0f4b972eda))
	ROM_LOAD( "shock5.bin", 0x1800, 0x0400, CRC(d1f33c6b) SHA1(c3c1061f2f55cefe8037b19d5ebe087579854992))

	ROM_REGION(0x0800, "cpu2", 0)
	ROM_LOAD("shock_s2.bin", 0x0000, 0x0400, CRC(c03e8009) SHA1(33e7e90f313d4dd2555feae9bd9912989c7d2de2))
	ROM_LOAD("shock_s1.bin", 0x0400, 0x0400, CRC(1f8543e9) SHA1(209c88198659844aeba1e4c39c04eb4d96b10de4))
ROM_END

/*--------------------------------
/ Snake Machine
/-------------------------------*/
ROM_START(snake)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "snake1.bin", 0x0000, 0x0800, CRC(7bb79585) SHA1(6e1bb1e33733bc2c41ad9fc43540190df24adc63))
	ROM_LOAD( "snake2.bin", 0x0800, 0x0800, CRC(55c946f7) SHA1(b77549063c99ee194608abb45aa0cec958336636))
	ROM_LOAD( "snake3.bin", 0x1000, 0x0800, CRC(6f054bc0) SHA1(08ab82131888756e8178b2fe2bbc24fc4f494ef2))
	ROM_LOAD( "snake4.bin", 0x1800, 0x0800, CRC(ed231064) SHA1(42410dbbef36dea9d0163c65406bc86b35bb0bd7))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("snake_s1.bin", 0x2000, 0x1000, CRC(f7c1623c) SHA1(77e79ccc4b074b715008de37332baf76791d471e))
	ROM_LOAD("snake_s2.bin", 0x3000, 0x1000, CRC(18316d73) SHA1(422a093ff245f0c8f710aeba91acd59666e2398b))
	ROM_RELOAD( 0x7000, 0x1000)
ROM_END

/*--------------------------------
/ Space Shuttle
/-------------------------------*/
ROM_START(sshuttle)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "sshtl1.bin", 0x0000, 0x0800, CRC(ab67ed50) SHA1(0f627b007d74b81aba6b4ad0f4cf6782e42e24c9))
	ROM_LOAD( "sshtl2.bin", 0x0800, 0x0800, CRC(ed5130a4) SHA1(3e99c151d6649c4b19d59ab2128ee3160c6462a9))
	ROM_LOAD( "sshtl3.bin", 0x1000, 0x0800, CRC(17d43a16) SHA1(dd9a503460db9af64d6e22303d8a5b5b578ff950))
	ROM_LOAD( "sshtl4.bin", 0x1800, 0x0800, CRC(2719dbac) SHA1(3519dbac6fc0314d3277d59211bad4abf844ee02))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("sshtl_s1.bin", 0x5000, 0x1000, CRC(5a6211e7) SHA1(9e53f76f76203c20f1933bf491b3f60279708c46))
	ROM_LOAD("sshtl_s2.bin", 0x6000, 0x1000, CRC(3af4707e) SHA1(b7231ede973a0c83e009333f0377b81c34826117))
	ROM_LOAD("sshtl_s3.bin", 0x7000, 0x1000, CRC(0788990b) SHA1(7197018d1ede74def864411afad99f98ddbab78a))
ROM_END

ROM_START(sshuttle1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "sshtl1.bin", 0x0000, 0x0800, CRC(ab67ed50) SHA1(0f627b007d74b81aba6b4ad0f4cf6782e42e24c9))
	ROM_LOAD( "sshtl2.bin", 0x0800, 0x0800, CRC(ed5130a4) SHA1(3e99c151d6649c4b19d59ab2128ee3160c6462a9))
	ROM_LOAD( "sshtl3a.bin", 0x1000, 0x0800, CRC(b1ddb78b) SHA1(ffa2aa6f501a06b2a3a92b1926050bd3ca053d0d))
	ROM_LOAD( "sshtl4a.bin", 0x1800, 0x0800, CRC(163a569d) SHA1(9fe259d09944eacd30582e36d9a1dcbb6f5e1ea2))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("sshtl_s1.bin", 0x5000, 0x1000, CRC(5a6211e7) SHA1(9e53f76f76203c20f1933bf491b3f60279708c46))
	ROM_LOAD("sshtl_s2.bin", 0x6000, 0x1000, CRC(3af4707e) SHA1(b7231ede973a0c83e009333f0377b81c34826117))
	ROM_LOAD("sshtl_s3.bin", 0x7000, 0x1000, CRC(0788990b) SHA1(7197018d1ede74def864411afad99f98ddbab78a))
ROM_END

/*--------------------------------
/ Speed Test
/-------------------------------*/
ROM_START(stest)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "stest1.bin", 0x0000, 0x0800, CRC(e13ed60c) SHA1(f2f89f7a1e7681ac3ea17c24c89ac1bee3ffa6e9))
	ROM_LOAD( "stest2.bin", 0x0800, 0x0800, CRC(584d683d) SHA1(8e52226a85366c8aebd011df014ab01f78d7e02d))
	ROM_LOAD( "stest3.bin", 0x1000, 0x0800, CRC(271129a2) SHA1(c20755f6b661502ce43fea03fb654046ed1a747d))
	ROM_LOAD( "stest4.bin", 0x1800, 0x0800, CRC(1cdd4e08) SHA1(bc7e3efd194396efb63115186bf586439732519d))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("stest_s2.bin", 0x1000, 0x0800, CRC(d7ac9369) SHA1(6085341a32bc5cc17a631aeb0d5a792a9de675be))
	ROM_LOAD("stest_s1.bin", 0x1800, 0x0800, CRC(dc71d4b2) SHA1(c2d3523019f63162aa23e0141263179b9f219609))
ROM_END

/*--------------------------------
/ Sultan
/-------------------------------*/

/*--------------------------------
/ Sure Shot
/-------------------------------*/
ROM_START(sureshop)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "ssh1.bin", 0x0000, 0x0800, CRC(46b96e00) SHA1(2cdbc0994bf0ff55330988a07c078dd2364a304c))
	ROM_LOAD( "ssh2.bin", 0x0800, 0x0800, CRC(655a7ff2) SHA1(f57852cd37e7fd4d054ad0f7a26e07d5932ad419))
	ROM_LOAD( "ssh3.bin", 0x1000, 0x0800, CRC(4dec25d6) SHA1(314052b0f5d750411ed597bb0461e9e847ccc2df))
	ROM_LOAD( "ssh4.bin", 0x1800, 0x0800, CRC(ced8f9df) SHA1(ba6b50df3ad2cb28885542748a61777df2010d69))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("ssh_s1.bin", 0x0800, 0x0800, CRC(acb7e92f) SHA1(103da5c87d0f1e0444575193e760b667d42fea73))
	ROM_LOAD("ssh_s3.bin", 0x1000, 0x0800, CRC(5e7f5275) SHA1(48eb1a499d2485b317ad769d876ec4cd57980285))
	ROM_LOAD("ssh_s2.bin", 0x1800, 0x0800, CRC(c1351b31) SHA1(a306ff7abe5b032cd05195200fc56a97c1d2eef3))
ROM_END

/*--------------------------------
/ Titan
/-------------------------------*/
ROM_START(titan)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "titan1.bin", 0x0000, 0x0800, CRC(625f58fb) SHA1(52f884faaa109243a0091882cef6e480ea5e4bcc))
	ROM_LOAD( "titan2.bin", 0x0800, 0x0800, CRC(f2e5a7d0) SHA1(e0c6a969765e433c448d54f2307767adda1254f9))
	ROM_LOAD( "titan3.bin", 0x1000, 0x0800, CRC(e0827a82) SHA1(7245bab117234c0286aad4a5f45bbb8cb843a3f0))
	ROM_LOAD( "titan4.bin", 0x1800, 0x0800, CRC(fb3d0282) SHA1(d0f47deab82bcf15e6129c0960c94493e78a1c51))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("titan_s2.bin", 0x1000, 0x0800, CRC(3bd0e6ab) SHA1(1a0b7ddde004020aaae5095071acc4b552ced1bf))
	ROM_LOAD("titan_s1.bin", 0x1800, 0x0800, CRC(36b5c196) SHA1(b3788ed5b53e4a8fe35e7be2b6b7b943e518f68c))
ROM_END

ROM_START(titan1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "titan1a.bin", 0x0000, 0x0800, CRC(d5437261) SHA1(649e1852dece8fcd036b9162d262fb535fb4a4e2))
	ROM_LOAD( "titan2.bin", 0x0800, 0x0800, CRC(f2e5a7d0) SHA1(e0c6a969765e433c448d54f2307767adda1254f9))
	ROM_LOAD( "titan3.bin", 0x1000, 0x0800, CRC(e0827a82) SHA1(7245bab117234c0286aad4a5f45bbb8cb843a3f0))
	ROM_LOAD( "titan4.bin", 0x1800, 0x0800, CRC(fb3d0282) SHA1(d0f47deab82bcf15e6129c0960c94493e78a1c51))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("titn_s2a.bin", 0x1000, 0x0800, CRC(5c91592d) SHA1(567d646652e441f83bc4797d1c8c004b3d071744))
	ROM_LOAD("titn_s1a.bin", 0x1800, 0x0800, CRC(9840dd80) SHA1(44217dcf7ae5c6f4f4801568e020ee770b4c994b))
ROM_END

/*--------------------------------
/ Vegas
/-------------------------------*/
ROM_START(vegast)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "lluck1.bin", 0x0000, 0x0800, CRC(be242895) SHA1(0528e9049e44b5ae7bba4a21ca5c0a2e5ffa4ca5))
	ROM_LOAD( "lluck2.bin", 0x0800, 0x0800, CRC(48169726) SHA1(282a322178e007df1183620dfcf3411bc67d8a0a))
	ROM_LOAD( "vegas3.bin", 0x1000, 0x0800, CRC(bd1fdbc3) SHA1(e184cec644b2d5cc05c3d458a06299359322df00))
	ROM_LOAD( "vegas4.bin", 0x1800, 0x0800, CRC(61f733a9) SHA1(a86ac621d81eb69a56706f9b0d49c0816f14a016))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("lluck_s2.bin", 0x1000, 0x0800, CRC(b0b05e9f) SHA1(1b5b5701ece241913367960eba7f58ca1a528548))
	ROM_LOAD("lluck_s1.bin", 0x1800, 0x0800, CRC(78ed85b4) SHA1(72fee3e337f2d2174a41434084699c3a472d798e))
ROM_END


/*--------------------------------
/ Volcano
/-------------------------------*/

/*--------------------------------
/ Voley Ball
/-------------------------------*/
ROM_START(voleybal)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "voley1.bin", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "voley2.bin", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "voley3.bin", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "voley4.bin", 0x1800, 0x0800, NO_DUMP)

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("voley_s1.bin", 0x2000, 0x1000, CRC(9c825666) SHA1(330ecd9caccb8a1555c5e7302095ae25558c020e))
	ROM_LOAD("voley_s2.bin", 0x3000, 0x1000, CRC(79a8228c) SHA1(e71d9347a8fc230c70703164ae0e4d44423bbb5d))
	ROM_RELOAD( 0x7000, 0x1000)
ROM_END

/*--------------------------------
/ Vortex
/-------------------------------*/
ROM_START(vortexp)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "vortex1.bin", 0x0000, 0x0800, CRC(abe193e7) SHA1(8ba7e82deb3461c0723a278596d02a6d74cfad68))
	ROM_LOAD( "vortex2.bin", 0x0800, 0x0800, CRC(0dd68604) SHA1(788e527e945d7edc8d30200ddf04f0a2cf4312ff))
	ROM_LOAD( "vortex3.bin", 0x1000, 0x0800, CRC(a46e3722) SHA1(b91ea5eb8b05a642e756fe3942ce4adc6bf75a29))
	ROM_LOAD( "vortex4.bin", 0x1800, 0x0800, CRC(39ef8112) SHA1(acde00a6c13fff1173a8fbe2ec31fdf662502032))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("vrtex_s2.bin", 0x1000, 0x0800, CRC(4250e02e) SHA1(5a67aac55728e6661d85e31b01a5263b9d4a22db))
	ROM_LOAD("vrtex_s1.bin", 0x1800, 0x0800, CRC(740bdd3e) SHA1(ed86bd65ac4b6d43f91a95d44d48b04adb631ee3))
ROM_END

/*--------------------------------
/ Zarza
/-------------------------------*/
ROM_START(zarza)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "zarza1.bin", 0x0000, 0x0800, CRC(81a35f85) SHA1(3086f47573c683f86c371954c2be6ee51b75c83b))
	ROM_LOAD( "zarza2.bin", 0x0800, 0x0800, CRC(cbf88eee) SHA1(1ef46098259f469b6fa3af05040a7ff2ace8c865))
	ROM_LOAD( "zarza3.bin", 0x1000, 0x0800, CRC(a5faf4d5) SHA1(84bb1e89dac9008e226c5d64f62f245632fe9634))
	ROM_LOAD( "zarza4.bin", 0x1800, 0x0800, CRC(ddfcdd20) SHA1(6c7761d9b11e4e62a5bf2346d9ec8278610131ec))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("zarza_s2.bin", 0x1000, 0x0800, CRC(a98e13b7) SHA1(7416a941ee87fd456a5c4115e6933b8b7ad69681))
	ROM_LOAD("zarza_s1.bin", 0x1800, 0x0800, CRC(f076c2a8) SHA1(f626556e1aea7a36a801e8f0fc9a762f8eea636f))
ROM_END

ROM_START(zarza1)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "zarza1.bin", 0x0000, 0x0800, CRC(81a35f85) SHA1(3086f47573c683f86c371954c2be6ee51b75c83b))
	ROM_LOAD( "zarza2a.bin", 0x0800, 0x0800, CRC(a1ada4be) SHA1(59709faad7f059766bc28e99901b24fed1fd9780))
	ROM_LOAD( "zarza3.bin", 0x1000, 0x0800, CRC(a5faf4d5) SHA1(84bb1e89dac9008e226c5d64f62f245632fe9634))
	ROM_LOAD( "zarza4a.bin", 0x1800, 0x0800, CRC(dc124f7b) SHA1(a513013bbd173dfe80c108e140e9546b17e3cedd))

	ROM_REGION(0x2000, "cpu2", 0)
	ROM_LOAD("zarza_s2.bin", 0x1000, 0x0800, CRC(a98e13b7) SHA1(7416a941ee87fd456a5c4115e6933b8b7ad69681))
	ROM_LOAD("zarza_s1.bin", 0x1800, 0x0800, CRC(f076c2a8) SHA1(f626556e1aea7a36a801e8f0fc9a762f8eea636f))
ROM_END

/*-----------
/ Test Eprom
/-----------*/
ROM_START(taitest)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "ttest1.bin", 0x0000, 0x0800, CRC(a9729e2f) SHA1(2c13bc9d6eab2101316fa795a18d5c5afac936d8))
	ROM_RELOAD( 0x1800, 0x0800)

	ROM_REGION(0x2000, "cpu2", ROMREGION_ERASEFF)
ROM_END


/*--------------------------------
/Mr. Black (Z-80 CPU)
/-------------------------------*/
ROM_START(mrblkz80)
	ROM_REGION(0x2800, "roms", 0)
	ROM_LOAD( "mb01z80.dat", 0x0000, 0x0800, CRC(7f883a70) SHA1(848783123b55ade769cac3c1b3d4a2c759a6c5b6))
	ROM_LOAD( "mb02z80.dat", 0x0800, 0x0800, CRC(68de8f50) SHA1(7076297060e927da1aefae8bf75c8cda18031660))
	ROM_LOAD( "mb03z80.dat", 0x1000, 0x0800, CRC(5a8e55e8) SHA1(b93102254004d258998bd6ab7d7b333361b37830))
	ROM_LOAD( "mb04z80.dat", 0x1800, 0x0800, CRC(ecf30c2f) SHA1(404c891bc420cfe540e829a1cd05ced10ea5a09c))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("mrb_s1.bin", 0x5000, 0x1000, CRC(ff28b2b9) SHA1(3106811740e0206ad4ba7845e204e721b0da70e2))
	ROM_LOAD("mrb_s2.bin", 0x6000, 0x1000, CRC(34d52449) SHA1(bdd5db5e58ca997d413d18f291928ad1a45c194e))
	ROM_LOAD("mrb_s3.bin", 0x7000, 0x1000, CRC(276fb897) SHA1(b1a4323a4d921e3ae4beefaa04cd95e18cc33b9d))
ROM_END

// no sound
GAME(198?,  taitest,    0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Taito Test Fixture", MACHINE_MECHANICAL )

// dac (sintetizador)
GAME(1979,  shock,      0,          shock,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Shock", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1980,  obaoba,     0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Oba-Oba (set 1)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1980,  obaoba1,    obaoba,     taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Oba-Oba (set 2)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1980,  drakor,     0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Drakor", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1980,  meteort,    0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Meteor (Taito)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1981,  sureshop,   0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Sure Shot (Pinball)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1981,  cosmic,     0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Cosmic", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1982,  gemini2k,   0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Gemini 2000 (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  gemini2k1,  gemini2k,   taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Gemini 2000 (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  vortexp,    0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Vortex (Pinball)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1982,  zarza,      0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Zarza (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  zarza1,     zarza,      taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Zarza (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  sharkt,     0,          taito2, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Shark (Taito)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  stest,      0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Speed Test", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1982,  lunelle,    0,          taito2, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Lunelle", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  rally,      0,          taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Rally", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )

// dac and vox (sintevox)
GAME(1981,  fireact,    0,          taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Fire Action", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  cavnegro,   0,          taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Cavaleiro Negro (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  cavnegro1,  cavnegro,   taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Cavaleiro Negro (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  cavnegro2,  cavnegro,   taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Cavaleiro Negro (set 3)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1981,  ladylukt,   0,          taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Lady Luck (Taito)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(198?,  vegast,     ladylukt,   taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Vegas (Taito)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(1982,  titan,      0,          taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Titan (set 1)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(1982,  titan1,     titan,      taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Titan (set 2)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(1982,  hawkman,    0,          taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Hawkman (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  hawkman1,   hawkman,    taito4, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Hawkman (set 2)", MACHINE_IS_SKELETON_MECHANICAL)

// dac and ay
GAME(1982,  snake,      0,          taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Snake Machine", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(198?,  voleybal,   0,          taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Voley Ball",  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  mrblack,    0,          taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Mr. Black (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  mrblack1,   mrblack,    taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Mr. Black (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  sshuttle,   0,          taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Space Shuttle (Taito) (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  sshuttle1,  sshuttle,   taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Space Shuttle (Taito) (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?,  polar,      0,          taito5, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Polar Explorer", MACHINE_IS_SKELETON_MECHANICAL)

// dac, vox and ay
GAME(1982,  gork,       0,          taito6, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Gork", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME(198?,  fireactd,   0,          taito6, taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Fire Action Deluxe", MACHINE_IS_SKELETON_MECHANICAL)

// different hardware
GAME(198?,  mrblkz80,   mrblack,    taito,  taito, taito_state, taito,  ROT0,   "Taito do Brasil",  "Mr. Black (Z-80 CPU)", MACHINE_IS_SKELETON_MECHANICAL)
