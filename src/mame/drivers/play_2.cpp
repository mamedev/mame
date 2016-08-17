// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

PINBALL
Playmatic MPU 2

Status:
- Lamps, Solenoids to add
- AY chips output port adds various components across the analog outputs (including muting)
- Mechanical sounds to add
- Sound board to add
- Most games work

***********************************************************************************/


#include "machine/genpin.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "play_2.lh"

class play_2_state : public driver_device
{
public:
	play_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
		, m_keyboard(*this, "X")
	{ }

	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port02_w);
	DECLARE_WRITE8_MEMBER(port03_w);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(port05_r);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_WRITE8_MEMBER(port07_w);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef1_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	DECLARE_WRITE_LINE_MEMBER(q4013a_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(clock2_w);
	DECLARE_WRITE8_MEMBER(port01_a_w);
	DECLARE_READ8_MEMBER(port02_a_r);
	DECLARE_READ_LINE_MEMBER(clear_a_r);

private:
	UINT16 m_clockcnt;
	UINT16 m_resetcnt;
	UINT16 m_resetcnt_a;
	UINT8 m_soundlatch;
	UINT8 m_a_irqset;
	UINT16 m_a_irqcnt;
	UINT8 m_kbdrow;
	UINT8 m_segment[5];
	bool m_disp_sw;
	virtual void machine_reset() override;
	required_device<cosmac_device> m_maincpu;
	required_device<cosmac_device> m_audiocpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
	required_ioport_array<8> m_keyboard;
};


static ADDRESS_MAP_START( play_2_map, AS_PROGRAM, 8, play_2_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x20ff) AM_RAM AM_SHARE("nvram") // pair of 5101, battery-backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_2_io, AS_IO, 8, play_2_state )
	AM_RANGE(0x01, 0x01) AM_WRITE(port01_w) // digits
	AM_RANGE(0x02, 0x02) AM_WRITE(port02_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(port03_w)
	AM_RANGE(0x04, 0x04) AM_READ(port04_r)
	AM_RANGE(0x05, 0x05) AM_READ(port05_r)
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w) // segments
	AM_RANGE(0x07, 0x07) AM_WRITE(port07_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_2_audio_map, AS_PROGRAM, 8, play_2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("aysnd1", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("aysnd2", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x8000, 0x80ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_2_audio_io, AS_IO, 8, play_2_state )
	AM_RANGE(0x01, 0x01) AM_WRITE(port01_a_w) // irq counter
	AM_RANGE(0x02, 0x02) AM_READ(port02_a_r) // sound code
ADDRESS_MAP_END


static INPUT_PORTS_START( play_2 )
	PORT_START("X.0") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("X.1") // 21-28
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X.2") // 31-38
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X.3") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X.4") // 51-58
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X.5") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) // outhole
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) // zone select (door switch)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9) // reset button on the ios board

	PORT_START("X.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) // reset button on main cpu EF4
INPUT_PORTS_END

void play_2_state::machine_reset()
{
	m_clockcnt = 0;
	m_resetcnt = 0;
	m_resetcnt_a = 0;
	m_4013b->d_w(1);
	m_a_irqset = 54;  // default value of the CDP1863
	m_a_irqcnt = (m_a_irqset << 3) | 7;
	m_soundlatch = 0;
	m_kbdrow = 0;
	m_disp_sw = 0;
	for (UINT8 i = 0; i < 5; i++)
		m_segment[i] = 0;
}

WRITE8_MEMBER( play_2_state::port01_w )
{
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (UINT8 j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (UINT8 i = 0; i < 5; i++)
					output().set_digit_value(j*10 + i, m_segment[i]);
	}
}

WRITE8_MEMBER( play_2_state::port06_w )
{
	//m_soundlatch = data;
}

WRITE8_MEMBER( play_2_state::port03_w )
{
	if (BIT(data, 6))
		m_audiocpu->ef1_w(1); // inverted
}

READ8_MEMBER( play_2_state::port04_r )
{
	if (m_kbdrow & 0x3f)
		for (UINT8 i = 0; i < 6; i++)
			if (BIT(m_kbdrow, i))
				return m_keyboard[i]->read();

	return 0;
}

READ8_MEMBER( play_2_state::port05_r )
{
	return m_keyboard[6]->read();
}

WRITE8_MEMBER( play_2_state::port02_w )
{
	m_segment[4] = m_segment[3];
	m_segment[3] = m_segment[2];
	m_segment[2] = m_segment[1];
	m_segment[1] = m_segment[0];
	m_segment[0] = data;
	m_disp_sw = 1;
}

WRITE8_MEMBER( play_2_state::port07_w )
{
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
}

WRITE8_MEMBER( play_2_state::port01_a_w )
{
	m_a_irqset = data;
	m_a_irqcnt = (m_a_irqset << 3) | 7;
}

READ8_MEMBER( play_2_state::port02_a_r )
{
	m_audiocpu->ef1_w(0); // inverted
	return m_soundlatch;
}

READ_LINE_MEMBER( play_2_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_2_state::clear_a_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt_a < 0xffff)
		m_resetcnt_a++;
	return (m_resetcnt_a == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_2_state::ef1_r )
{
	return (!BIT(m_clockcnt, 10)); // inverted
}

READ_LINE_MEMBER( play_2_state::ef4_r )
{
	return BIT(m_keyboard[7]->read(), 0); // inverted test button - doesn't seem to do anything
}

WRITE_LINE_MEMBER( play_2_state::clock_w )
{
	m_4013a->clock_w(state);

	if (!state)
	{
		m_clockcnt++;
		// simulate 4020 chip
		if ((m_clockcnt & 0x3ff) == 0)
			m_4013b->preset_w(BIT(m_clockcnt, 10)); // Q10 output

		// sound irq
		m_a_irqcnt--;
		if (m_a_irqcnt == 1)
			m_audiocpu->int_w(1); // inverted
		else
		if (m_a_irqcnt == 0)
		{
			m_a_irqcnt = (m_a_irqset << 3) | 7;
			m_audiocpu->int_w(0); // inverted
		}
	}
}

WRITE_LINE_MEMBER( play_2_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(state); // inverted
}

WRITE_LINE_MEMBER( play_2_state::q4013a_w )
{
	m_clockcnt = 0;
}

static MACHINE_CONFIG_START( play_2, play_2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_2_map)
	MCFG_CPU_IO_MAP(play_2_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(play_2_state, clear_r))
	MCFG_COSMAC_EF1_CALLBACK(READLINE(play_2_state, ef1_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(play_2_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(DEVWRITELINE("4013a", ttl7474_device, clear_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_play_2)

	MCFG_DEVICE_ADD("tpb_clock", CLOCK, 2950000 / 8) // TPB line from CPU
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_2_state, clock_w))

	MCFG_DEVICE_ADD("xpoint", CLOCK, 60) // crossing-point detector
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_2_state, clock2_w))

	// This is actually a 4013 chip (has 2 RS flipflops)
	MCFG_DEVICE_ADD("4013a", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("4013a", ttl7474_device, d_w))
	MCFG_7474_OUTPUT_CB(WRITELINE(play_2_state, q4013a_w))

	MCFG_DEVICE_ADD("4013b", TTL7474, 0)
	MCFG_7474_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, ef2_w))
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, int_w)) MCFG_DEVCB_INVERT // int is reversed in mame

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_CPU_ADD("audiocpu", CDP1802, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(play_2_audio_map)
	MCFG_CPU_IO_MAP(play_2_audio_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(play_2_state, clear_a_r))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("aysnd1", AY8910, XTAL_3_579545MHz / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ADD("aysnd2", AY8910, XTAL_3_579545MHz / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Antar (11/79)
/-------------------------------------------------------------------*/
ROM_START(antar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("antar08.bin",  0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.bin",  0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10.bin",  0x0800, 0x0400, CRC(a6ce5667) SHA1(85ecd4fce94dc419e4c210262f867310b0889cd3))
	ROM_LOAD("antar11.bin",  0x0c00, 0x0400, CRC(6474b17f) SHA1(e4325ceff820393b06eb2e8e4a85412b0d01a385))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(antar2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("antar08.bin",  0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.bin",  0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10a.bin", 0x0800, 0x0400, CRC(520eb401) SHA1(1d5e3f829a7e7f38c7c519c488e6b7e1a4d34321))
	ROM_LOAD("antar11a.bin", 0x0c00, 0x0400, CRC(17ad38bf) SHA1(e2c9472ed8fbe9d5965a5c79515a1b7ea9edaa79))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END


/*-------------------------------------------------------------------
/ Evil Fight (03/80)
/-------------------------------------------------------------------*/
ROM_START(evlfight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("evfg08.bin",   0x0000, 0x0400, CRC(2cc2e79a) SHA1(17440512c419b3bb2012539666a5f052f3cd8c1d))
	ROM_LOAD("evfg09.bin",   0x0400, 0x0400, CRC(5232dc4c) SHA1(6f95a578e9f09688e6ce8b0a622bcee887936c82))
	ROM_LOAD("evfg10.bin",   0x0800, 0x0400, CRC(de2f754d) SHA1(0287a9975095bcbf03ddb2b374ff25c080c8020f))
	ROM_LOAD("evfg11.bin",   0x0c00, 0x0400, CRC(5eb8ac02) SHA1(31c80e74a4272becf7014aa96eaf7de555e26cd6))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ Mad Race (??/85?)
/-------------------------------------------------------------------*/
ROM_START(madrace)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("madrace.2a0",  0x0000, 0x0800, CRC(ab487c79) SHA1(a5df29b2af4c9d94d8bf54c5c91d1e9b5ca4d065))
	ROM_LOAD("madrace.2b0",  0x0800, 0x0800, CRC(dcb54b39) SHA1(8e2ca7180f5ea3a28feb34b01f3387b523dbfa3b))
	ROM_LOAD("madrace.2c0",  0x1000, 0x0800, CRC(b24ea245) SHA1(3f868ccbc4bfb77c40c4cc05dcd8eeca85ecd76f))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("madrace1.snd", 0x0000, 0x2000, CRC(49e956a5) SHA1(8790cc27a0fda7b8e07bee65109874140b4018a2))
	ROM_LOAD("madrace2.snd", 0x2000, 0x0800, CRC(c19283d3) SHA1(42f9770c46030ef20a80cc94fdbe6548772aa525))
ROM_END


/*-------------------------------------------------------------------
/ Attack (10/80)
/-------------------------------------------------------------------*/
ROM_START(attack)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("attack8.bin",  0x0000, 0x0400, CRC(a5204b58) SHA1(afb4b81720f8d56e88f47fc842b23313824a1085))
	ROM_LOAD("attack9.bin",  0x0400, 0x0400, CRC(bbd086b4) SHA1(6fc94b94beea482d8c8f5b3c69d3f218e2b2dfc4))
	ROM_LOAD("attack10.bin", 0x0800, 0x0400, CRC(764925e4) SHA1(2f207ef87786d27d0d856c5816a570a59d89b718))
	ROM_LOAD("attack11.bin", 0x0c00, 0x0400, CRC(972157b4) SHA1(23c90f23a34b34acfe445496a133b6022a749ccc))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ Black Fever (12/80)
/-------------------------------------------------------------------*/
ROM_START(blkfever)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("blackf8.bin",  0x0000, 0x0400, CRC(916b8ed8) SHA1(ddc7e09b68e3e1a033af5dc5ec32ab5b0922a833))
	ROM_LOAD("blackf9.bin",  0x0400, 0x0400, CRC(ecb72fdc) SHA1(d3598031b7170fab39727b3402b7053d4f9e1ca7))
	ROM_LOAD("blackf10.bin", 0x0800, 0x0400, CRC(b3fae788) SHA1(e14e09cc7da1098abf2f60f26a8ec507e123ff7c))
	ROM_LOAD("blackf11.bin", 0x0c00, 0x0400, CRC(5a97c1b4) SHA1(b9d7eb0dd55ef6d959c0fab48f710e4b1c8d8003))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ Zira (??/80)
/-------------------------------------------------------------------*/
ROM_START(zira)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("zira_u8.bin",  0x0000, 0x0800, CRC(53f8bf17) SHA1(5eb74f27bc65374a85dd44bbc8f6142488c226a2))
	ROM_LOAD("zira_u9.bin",  0x0800, 0x0800, CRC(d50a2419) SHA1(81b157f579a433389506817b1b6e02afaa2cf0d5))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("zira.snd",     0x0000, 0x0400, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Cerberus (03/82)
/-------------------------------------------------------------------*/
ROM_START(cerberup)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cerb8.cpu",    0x0000, 0x0800, CRC(021d0452) SHA1(496010e6892311b1cabcdac62296cd6aa0782c5d))
	ROM_LOAD("cerb9.cpu",    0x0800, 0x0800, CRC(0fd41156) SHA1(95d1bf42c82f480825e3d907ae3c87b5f994fd2a))
	ROM_LOAD("cerb10.cpu",   0x1000, 0x0800, CRC(785602e0) SHA1(f38df3156cd14ab21752dbc849c654802079eb33))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("cerb.snd",     0x0000, 0x2000, CRC(8af53a23) SHA1(a80b57576a1eb1b4544b718b9abba100531e3942))
ROM_END


GAME(1979,  antar,     0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Antar (set 1)",      MACHINE_MECHANICAL | MACHINE_NO_SOUND)
GAME(1979,  antar2,    antar, play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Antar (set 2)",      MACHINE_MECHANICAL | MACHINE_NO_SOUND)
GAME(1980,  evlfight,  0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Evil Fight",         MACHINE_MECHANICAL | MACHINE_NO_SOUND)
GAME(1980,  attack,    0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Attack",             MACHINE_MECHANICAL | MACHINE_NO_SOUND)
GAME(1980,  blkfever,  0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Black Fever",        MACHINE_MECHANICAL | MACHINE_NO_SOUND)
GAME(1982,  cerberup,  0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Cerberus (Pinball)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1985,  madrace,   0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Mad Race",           MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1980,  zira,      0,     play_2, play_2, driver_device, 0,  ROT0, "Playmatic", "Zira",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
