// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

PINBALL
Playmatic MPU-C

The IOS board common to all games provides sound effects through the CDP1863.
4 different add-on sound boards were also used:
- Black Fever has the Speaking System board, which produces analog signals for
  controlling an 8-track tape player.
- Zira uses the Sound-2 board with a COP402 and AY-3-8910. The latter device is
  also supposedly used to control lights through a separate connector.
- Cerberus uses the Sound-3 board with a 90435 processor, which is most likely a
  CDP1802 by another name. The 90503 "synthesizer" is the only sound IC on this
  board; it has a TI logo and seems at least pin-compatible with TMS52xx.
- Mad Race uses a Sound Board IV (same as MPU-3 and later), but I/O ports
  that talk to it are unknown.

ToDo:
- Lamps, Solenoids to add
- Add remaining mechanical sounds
- Some sound boards to add

Notes:
- Mad Race: S is the outhole for now. Game works, but no sound.
- Zira, Cerberus: not working
- Others: X is the outhole, and these games all work.
- Hold down the outhole key (usually X), when starting a game.

***********************************************************************************/


#include "emu.h"
#include "machine/genpin.h"

#include "cpu/cop400/cop400.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/ripple_counter.h"
#include "sound/ay8910.h"
#include "sound/cdp1863.h"
#include "speaker.h"

#include "play_2.lh"


class play_2_state : public genpin_class
{
public:
	play_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
		, m_4020(*this, "4020")
		, m_1863(*this, "1863")
		, m_aysnd1(*this, "aysnd1")
		, m_keyboard(*this, "X.%u", 0)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void play_2(machine_config &config);
	void zira(machine_config &config);

	void init_zira();

private:
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
	DECLARE_WRITE16_MEMBER(clockcnt_w);
	DECLARE_WRITE_LINE_MEMBER(clock2_w);
	// Zira
	DECLARE_WRITE8_MEMBER(sound_d_w);
	DECLARE_WRITE8_MEMBER(sound_g_w);
	DECLARE_READ8_MEMBER(psg_r);
	DECLARE_WRITE8_MEMBER(psg_w);
	DECLARE_READ8_MEMBER(sound_in_r);

	void play_2_io(address_map &map);
	void play_2_map(address_map &map);
	void zira_sound_map(address_map &map);

	uint16_t m_resetcnt;
	uint8_t m_kbdrow;
	uint8_t m_segment[5];
	bool m_disp_sw;
	uint8_t m_soundlatch;
	uint8_t m_psg_latch;
	uint8_t m_port06;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cosmac_device> m_maincpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
	required_device<ripple_counter_device> m_4020;
	optional_device<cdp1863_device> m_1863;
	optional_device<ay8910_device> m_aysnd1;
	required_ioport_array<8> m_keyboard;
	output_finder<60> m_digits;
};


void play_2_state::play_2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("roms", 0);
	map(0x2000, 0x20ff).ram().share("nvram"); // pair of 5101, battery-backed
}

void play_2_state::play_2_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_2_state::port01_w)); // digits
	map(0x02, 0x02).w(FUNC(play_2_state::port02_w));
	map(0x03, 0x03).w(m_1863, FUNC(cdp1863_device::str_w));
	map(0x04, 0x04).r(FUNC(play_2_state::port04_r));
	map(0x05, 0x05).r(FUNC(play_2_state::port05_r));
	map(0x06, 0x06).w(FUNC(play_2_state::port06_w));
	map(0x07, 0x07).w(FUNC(play_2_state::port07_w));
}

void play_2_state::zira_sound_map(address_map &map)
{
	map(0x000, 0x3ff).bankr("bank1");
}


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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S) // outhole on Mad Race
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
	m_resetcnt = 0;
	m_4013b->d_w(1);
	m_kbdrow = 0;
	m_disp_sw = 0;
	m_port06 = 0;
	for (uint8_t i = 0; i < 5; i++)
		m_segment[i] = 0;
	m_1863->oe_w(1);
}

WRITE8_MEMBER( play_2_state::port01_w )
{
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (uint8_t j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (uint8_t i = 0; i < 5; i++)
					m_digits[j*10 + i] = m_segment[i] & 0x7f;
	}
	m_1863->set_output_gain(0, BIT(data, 7) ? 1.00 : 0.00);
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

WRITE8_MEMBER( play_2_state::port03_w )
{
}

READ8_MEMBER( play_2_state::port04_r )
{
	if (m_kbdrow & 0x3f)
		for (uint8_t i = 0; i < 6; i++)
			if (BIT(m_kbdrow, i))
				return m_keyboard[i]->read();

	return 0;
}

READ8_MEMBER( play_2_state::port05_r )
{
	return m_keyboard[6]->read();
}

WRITE8_MEMBER( play_2_state::port06_w )
{
	m_port06 = data & 15;
}

WRITE8_MEMBER( play_2_state::port07_w )
{
	m_soundlatch = (data & 0x70) >> 4; // Zira (manual doesn't say where data comes from)
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
	if (!BIT(data, 7))
	{
		if (m_port06 == 11)
			m_samples->start(0, 5); // outhole
		if (m_port06 == 13)
			m_samples->start(0, 6); // knocker
	}
}

READ_LINE_MEMBER( play_2_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_2_state::ef1_r )
{
	return (!BIT(m_4020->count(), 10)); // inverted
}

READ_LINE_MEMBER( play_2_state::ef4_r )
{
	return BIT(m_keyboard[7]->read(), 0); // inverted test button - doesn't seem to do anything
}

WRITE16_MEMBER( play_2_state::clockcnt_w )
{
	if ((data & 0x3ff) == 0)
		m_4013b->preset_w(BIT(data, 10)); // Q10 output
}

WRITE_LINE_MEMBER( play_2_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(state); // inverted
}

// *********** Zira Sound handlers ***************** (same as cidelsa.cpp)
WRITE8_MEMBER( play_2_state::sound_d_w )
{
//    D3      2716 A10
	membank("bank1")->set_entry(BIT(data, 3));
}

WRITE8_MEMBER( play_2_state::sound_g_w )
{
	switch (data)
	{
	case 0x01:
		m_aysnd1->data_w(m_psg_latch);
		break;

	case 0x02:
		m_psg_latch = m_aysnd1->data_r();
		break;

	case 0x03:
		m_aysnd1->address_w(m_psg_latch);
		break;
	}
}

READ8_MEMBER( play_2_state::sound_in_r )
{
	return m_soundlatch;
}

READ8_MEMBER( play_2_state::psg_r )
{
	return m_psg_latch;
}

WRITE8_MEMBER( play_2_state::psg_w )
{
	m_psg_latch = data;
}

// **************** Machine *****************************

void play_2_state::play_2(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 2.95_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &play_2_state::play_2_map);
	m_maincpu->set_addrmap(AS_IO, &play_2_state::play_2_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(play_2_state::clear_r));
	m_maincpu->ef1_cb().set(FUNC(play_2_state::ef1_r));
	m_maincpu->ef4_cb().set(FUNC(play_2_state::ef4_r));
	m_maincpu->q_cb().set(m_4013a, FUNC(ttl7474_device::clear_w));
	m_maincpu->tpb_cb().set(m_4013a, FUNC(ttl7474_device::clock_w));
	m_maincpu->tpb_cb().append(m_4020, FUNC(ripple_counter_device::clock_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_play_2);

	CLOCK(config, "xpoint", 60).signal_handler().set(FUNC(play_2_state::clock2_w)); // crossing-point detector

	// This is actually a 4013 chip (has 2 RS flipflops)
	TTL7474(config, m_4013a, 0);
	m_4013a->comp_output_cb().set(m_4013a, FUNC(ttl7474_device::d_w));
	m_4013a->output_cb().set(m_4020, FUNC(ripple_counter_device::reset_w)); // TODO: also CKD for display

	TTL7474(config, m_4013b, 0);
	m_4013b->output_cb().set(m_maincpu, FUNC(cosmac_device::ef2_w));
	m_4013b->comp_output_cb().set(m_maincpu, FUNC(cosmac_device::int_w)).invert(); // int is reversed in mame

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(14); // only Q10 is actually used
	m_4020->count_out_cb().set(FUNC(play_2_state::clockcnt_w));

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	CDP1863(config, m_1863, 0);
	m_1863->set_clock2(2.95_MHz_XTAL / 8);
	m_1863->add_route(ALL_OUTPUTS, "mono", 0.75);
}

void play_2_state::zira(machine_config &config)
{
	play_2(config);
	cop402_cpu_device &cop402(COP402(config, "cop402", 2_MHz_XTAL));
	cop402.set_addrmap(AS_PROGRAM, &play_2_state::zira_sound_map);
	cop402.set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false);
	cop402.write_d().set(FUNC(play_2_state::sound_d_w));
	cop402.write_g().set(FUNC(play_2_state::sound_g_w));
	cop402.read_l().set(FUNC(play_2_state::psg_r));
	cop402.write_l().set(FUNC(play_2_state::psg_w));
	cop402.read_in().set(FUNC(play_2_state::sound_in_r));

	AY8910(config, m_aysnd1, 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void play_2_state::init_zira()
{
	/* setup COP402 memory banking */
	membank("bank1")->configure_entries(0, 2, memregion("cop402")->base(), 0x400);
	membank("bank1")->set_entry(0);
}



/*-------------------------------------------------------------------
/ Antar (11/79)
/-------------------------------------------------------------------*/
ROM_START(antar)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("antar08.bin",  0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.bin",  0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10.bin",  0x0800, 0x0400, CRC(a6ce5667) SHA1(85ecd4fce94dc419e4c210262f867310b0889cd3))
	ROM_LOAD("antar11.bin",  0x0c00, 0x0400, CRC(6474b17f) SHA1(e4325ceff820393b06eb2e8e4a85412b0d01a385))
ROM_END

ROM_START(antar2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("antar08.bin",  0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.bin",  0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10a.bin", 0x0800, 0x0400, CRC(520eb401) SHA1(1d5e3f829a7e7f38c7c519c488e6b7e1a4d34321))
	ROM_LOAD("antar11a.bin", 0x0c00, 0x0400, CRC(17ad38bf) SHA1(e2c9472ed8fbe9d5965a5c79515a1b7ea9edaa79))
ROM_END


/*-------------------------------------------------------------------
/ Evil Fight (03/80)
/-------------------------------------------------------------------*/
ROM_START(evlfight)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("evfg08.bin",   0x0000, 0x0400, CRC(2cc2e79a) SHA1(17440512c419b3bb2012539666a5f052f3cd8c1d))
	ROM_LOAD("evfg09.bin",   0x0400, 0x0400, CRC(5232dc4c) SHA1(6f95a578e9f09688e6ce8b0a622bcee887936c82))
	ROM_LOAD("evfg10.bin",   0x0800, 0x0400, CRC(de2f754d) SHA1(0287a9975095bcbf03ddb2b374ff25c080c8020f))
	ROM_LOAD("evfg11.bin",   0x0c00, 0x0400, CRC(5eb8ac02) SHA1(31c80e74a4272becf7014aa96eaf7de555e26cd6))
ROM_END

/*-------------------------------------------------------------------
/ Attack (10/80)
/-------------------------------------------------------------------*/
ROM_START(attack)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("attack8.bin",  0x0000, 0x0400, CRC(a5204b58) SHA1(afb4b81720f8d56e88f47fc842b23313824a1085))
	ROM_LOAD("attack9.bin",  0x0400, 0x0400, CRC(bbd086b4) SHA1(6fc94b94beea482d8c8f5b3c69d3f218e2b2dfc4))
	ROM_LOAD("attack10.bin", 0x0800, 0x0400, CRC(764925e4) SHA1(2f207ef87786d27d0d856c5816a570a59d89b718))
	ROM_LOAD("attack11.bin", 0x0c00, 0x0400, CRC(972157b4) SHA1(23c90f23a34b34acfe445496a133b6022a749ccc))
ROM_END

/*-------------------------------------------------------------------
/ Black Fever (12/80)
/-------------------------------------------------------------------*/
ROM_START(blkfever)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("blackf8.bin",  0x0000, 0x0400, CRC(916b8ed8) SHA1(ddc7e09b68e3e1a033af5dc5ec32ab5b0922a833))
	ROM_LOAD("blackf9.bin",  0x0400, 0x0400, CRC(ecb72fdc) SHA1(d3598031b7170fab39727b3402b7053d4f9e1ca7))
	ROM_LOAD("blackf10.bin", 0x0800, 0x0400, CRC(b3fae788) SHA1(e14e09cc7da1098abf2f60f26a8ec507e123ff7c))
	ROM_LOAD("blackf11.bin", 0x0c00, 0x0400, CRC(5a97c1b4) SHA1(b9d7eb0dd55ef6d959c0fab48f710e4b1c8d8003))
ROM_END

/*-------------------------------------------------------------------
/ Zira (??/81)
/-------------------------------------------------------------------*/
ROM_START(zira)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("zira_u8.bin",  0x0000, 0x0800, CRC(53f8bf17) SHA1(5eb74f27bc65374a85dd44bbc8f6142488c226a2))
	ROM_LOAD("zira_u9.bin",  0x0800, 0x0800, CRC(d50a2419) SHA1(81b157f579a433389506817b1b6e02afaa2cf0d5))

	ROM_REGION(0x800, "cop402", 0) // according to the schematic this is a 2716 with a size of 0x800; according to PinMAME it contains the same code twice
	ROM_LOAD("zira.snd",     0x0000, 0x0800, CRC(008cb743) SHA1(8e9677f08189638d669b265bb6943275a08ec8b4))
ROM_END

/*-------------------------------------------------------------------
/ Cerberus (03/82)
/-------------------------------------------------------------------*/
ROM_START(cerberup)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cerb8.cpu",    0x0000, 0x0800, CRC(021d0452) SHA1(496010e6892311b1cabcdac62296cd6aa0782c5d))
	ROM_LOAD("cerb9.cpu",    0x0800, 0x0800, CRC(0fd41156) SHA1(95d1bf42c82f480825e3d907ae3c87b5f994fd2a))
	ROM_LOAD("cerb10.cpu",   0x1000, 0x0800, CRC(785602e0) SHA1(f38df3156cd14ab21752dbc849c654802079eb33))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("cerb.snd",     0x0000, 0x2000, CRC(8af53a23) SHA1(a80b57576a1eb1b4544b718b9abba100531e3942))
ROM_END

/*-------------------------------------------------------------------
/ Mad Race (??/85?)
/-------------------------------------------------------------------*/
ROM_START(madrace)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("madrace.2a0",  0x0000, 0x0800, CRC(ab487c79) SHA1(a5df29b2af4c9d94d8bf54c5c91d1e9b5ca4d065))
	ROM_LOAD("madrace.2b0",  0x0800, 0x0800, CRC(dcb54b39) SHA1(8e2ca7180f5ea3a28feb34b01f3387b523dbfa3b))
	ROM_LOAD("madrace.2c0",  0x1000, 0x0800, CRC(b24ea245) SHA1(3f868ccbc4bfb77c40c4cc05dcd8eeca85ecd76f))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("madrace1.snd", 0x0000, 0x2000, CRC(49e956a5) SHA1(8790cc27a0fda7b8e07bee65109874140b4018a2))
	ROM_LOAD("madrace2.snd", 0x2000, 0x0800, CRC(c19283d3) SHA1(42f9770c46030ef20a80cc94fdbe6548772aa525))
ROM_END


GAME(1979,  antar,     0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Antar (set 1)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1979,  antar2,    antar, play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Antar (set 2)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1980,  evlfight,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Evil Fight",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1980,  attack,    0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Attack",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1980,  blkfever,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Black Fever",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1982,  cerberup,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Cerberus (Pinball)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1985,  madrace,   0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic", "Mad Race",           MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1980,  zira,      0,     zira,   play_2, play_2_state, init_zira,  ROT0, "Playmatic", "Zira",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
