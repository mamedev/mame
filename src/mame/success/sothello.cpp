// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
 Super Othello (c)1986 Fujiwara/Success

    driver by Tomasz Slanina

         1    2    3    4     5     6     7      8      9     10     11    12
+---------------------------------------------------------------------------------+
|                                                                                 |
+-+    LA460  LA6324  M5205  X3   Z80A    1      2    5816P  74374  74138         |  A
  |                                                                               |
  |                                                                               |
+-+        74367 Y3014 74174 74174       7404   7474  74138  7404   7432          |  B
|                                                                                 |
|                                                                                 |
|            74367 DSW1 YM2203    Z80A    3      4      5           6264          |  C
| J                                                                               |
| A                                                                               |
| M   C1663 74367  DSW2           7408 74125   7404  74138   74139  74174  7408   |  D
| M           X2       7414  7474                                                 |
| A                                                                               |
|     C1663 V9938 41464 41464       X1   7474  74139  7432   74157  74244  7432   |  E
|                                                                                 |
|                                                                                 |
+-+   C1663       41464 41464     6809B   6     6264   6264  6264   74244  74245  |  F
  |                                                                               |
  |                                                                               |
+-+   C1663                                                                       |  H
|                                                                                 |
+---------------------------------------------------------------------------------+

CPU  : Z80A(x2) HD68B09P
Sound: YM2203?(surface scratched) + M5205
OSC  : 8.0000MHz(X1)   21.477 MHz(X2)   384kHz(X3)

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "video/v9938.h"
#include "speaker.h"


namespace {

class sothello_state : public driver_device
{
public:
	sothello_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "subcpu"),
		m_msm(*this, "msm"),
		m_mainbank(*this, "mainbank")
	{ }

	void sothello(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void bank_w(uint8_t data);
	uint8_t subcpu_comm_clear_r();
	uint8_t subcpu_comm_nmi_r();
	uint8_t subcpu_comm_status_r();
	uint8_t soundcpu_status_r();
	void msm_data_w(uint8_t data);
	void soundcpu_busyflag_set_w(uint8_t data);
	void soundcpu_busyflag_reset_w(uint8_t data);
	void soundcpu_int_clear_w(uint8_t data);
	void subcpu_status_w(uint8_t data);
	void msm_cfg_w(uint8_t data);
	void adpcm_int_w(int state);

	void maincpu_io_map(address_map &map) ATTR_COLD;
	void maincpu_mem_map(address_map &map) ATTR_COLD;
	void soundcpu_io_map(address_map &map) ATTR_COLD;
	void soundcpu_mem_map(address_map &map) ATTR_COLD;
	void subcpu_mem_map(address_map &map) ATTR_COLD;

	int m_subcpu_status;
	int m_soundcpu_busy;
	int m_msm_data;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_memory_bank m_mainbank;
};

void sothello_state::machine_start()
{
	m_mainbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_subcpu_status));
	save_item(NAME(m_soundcpu_busy));
	save_item(NAME(m_msm_data));
}

void sothello_state::machine_reset()
{
	m_subcpu_status = 0;
	m_soundcpu_busy = 0;
	m_msm_data = 0;
}



/******************************************************************************
    I/O & Address Maps
******************************************************************************/

// main Z80

void sothello_state::bank_w(uint8_t data)
{
	int bank=0;
	switch(data^0xff)
	{
		case 1: bank=0; break;
		case 2: bank=1; break;
		case 4: bank=2; break;
		case 8: bank=3; break;
	}
	m_mainbank->set_entry(bank);
}

uint8_t sothello_state::subcpu_comm_clear_r()
{
	if(!machine().side_effects_disabled())
		m_subcpu_status = 0;
	return 0;
}

uint8_t sothello_state::subcpu_comm_nmi_r()
{
	if(!machine().side_effects_disabled())
		m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	return 0;
}

uint8_t sothello_state::subcpu_comm_status_r()
{
	return m_subcpu_status;
}

uint8_t sothello_state::soundcpu_status_r()
{
	return m_soundcpu_busy;
}

void sothello_state::maincpu_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xc7ff).ram().mirror(0x1800).share("mainsub");
	map(0xe000, 0xffff).ram();
}

void sothello_state::maincpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).portr("INPUT1");
	map(0x10, 0x1f).portr("INPUT2");
	map(0x20, 0x2f).portr("SYSTEM");
	map(0x30, 0x30).r(FUNC(sothello_state::subcpu_comm_clear_r));
	map(0x31, 0x31).r(FUNC(sothello_state::subcpu_comm_nmi_r));
	map(0x32, 0x32).r(FUNC(sothello_state::subcpu_comm_status_r));
	map(0x33, 0x33).r(FUNC(sothello_state::soundcpu_status_r));
	map(0x40, 0x4f).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x50, 0x50).w(FUNC(sothello_state::bank_w));
	map(0x60, 0x61).mirror(0x02).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write)); // not sure, but the A1 line is ignored, code @ $8b8
	map(0x70, 0x73).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
}


// sound Z80

void sothello_state::adpcm_int_w(int state)
{
	// only 4 bits are used
	m_msm->data_w(m_msm_data & 0x0f);
	m_soundcpu->set_input_line(0, ASSERT_LINE);
}

void sothello_state::msm_cfg_w(uint8_t data)
{
/*
     bit 0 = RESET
     bit 1 = 4B/3B 0
     bit 2 = S2    1
     bit 3 = S1    2
*/
	m_msm->playmode_w(bitswap<3>(data,1,2,3));
	m_msm->reset_w(data & 1);
}

void sothello_state::msm_data_w(uint8_t data)
{
	m_msm_data = data;
}

void sothello_state::soundcpu_busyflag_set_w(uint8_t data)
{
	m_soundcpu_busy=1;
}

void sothello_state::soundcpu_busyflag_reset_w(uint8_t data)
{
	m_soundcpu_busy=0;
}

void sothello_state::soundcpu_int_clear_w(uint8_t data)
{
	m_soundcpu->set_input_line(0, CLEAR_LINE);
}

void sothello_state::soundcpu_mem_map(address_map &map)
{
	map(0x0000, 0xdfff).rom().region("soundcpu", 0);
	map(0xf800, 0xffff).ram();
}

void sothello_state::soundcpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w(FUNC(sothello_state::msm_data_w));
	map(0x02, 0x02).w(FUNC(sothello_state::msm_cfg_w));
	map(0x03, 0x03).w(FUNC(sothello_state::soundcpu_busyflag_set_w));
	map(0x04, 0x04).w(FUNC(sothello_state::soundcpu_busyflag_reset_w));
	map(0x05, 0x05).w(FUNC(sothello_state::soundcpu_int_clear_w));
}


// sub 6809

void sothello_state::subcpu_status_w(uint8_t data)
{
	m_subcpu_status = 3;
}

void sothello_state::subcpu_mem_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(sothello_state::subcpu_status_w)).nopr();
	map(0x2000, 0x77ff).ram();
	map(0x7800, 0x7fff).ram().share("mainsub"); // upper 0x800 of 6264 is shared with main cpu
	map(0x8000, 0xffff).rom().region("subcpu", 0);
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sothello )
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf2, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x1c, 0x10, "Timer" )
	PORT_DIPSETTING(    0x1c, "15" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x14, "25" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x0c, "35" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x04, "45" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWB")
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x10, "Matta" ) // undo moves
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x08, "Games for 2 players" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void sothello_state::sothello(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(21'477'272) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &sothello_state::maincpu_mem_map);
	m_maincpu->set_addrmap(AS_IO, &sothello_state::maincpu_io_map);

	Z80(config, m_soundcpu, XTAL(21'477'272) / 6);
	m_soundcpu->set_addrmap(AS_PROGRAM, &sothello_state::soundcpu_mem_map);
	m_soundcpu->set_addrmap(AS_IO, &sothello_state::soundcpu_io_map);

	MC6809(config, m_subcpu, XTAL(8'000'000)); // divided by 4 internally
	m_subcpu->set_addrmap(AS_PROGRAM, &sothello_state::subcpu_mem_map);

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	v9938_device &v9938(V9938(config, "v9938", XTAL(21'477'272)));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(0x40000);
	v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(21'477'272) / 12));
	ymsnd.irq_handler().set_inputline(m_subcpu, 0);
	ymsnd.port_a_read_callback().set_ioport("DSWA");
	ymsnd.port_b_read_callback().set_ioport("DSWB");
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.50);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(sothello_state::adpcm_int_w)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // changed on the fly
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sothello )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "3.7c",   0x00000, 0x8000, CRC(47f97bd4) SHA1(52c9638f098fdcf66903fad7dafe3ab171758572) )
	ROM_LOAD( "4.8c",   0x08000, 0x8000, CRC(a98414e9) SHA1(6d14e1f9c79b95101e0aa101034f398af09d7f32) )
	ROM_LOAD( "5.9c",   0x10000, 0x8000, CRC(e5b5d61e) SHA1(2e4b3d85f41d0796a4d61eae40dd824769e1db86) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.7a",   0x0000, 0x8000, CRC(6951536a) SHA1(64d07a692d6a167334c825dc173630b02584fdf6) )
	ROM_LOAD( "2.8a",   0x8000, 0x8000, CRC(9c535317) SHA1(b2e69b489e111d6f8105e68fade6e5abefb825f7) )

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD( "6.7f",   0x0000, 0x8000, CRC(ee80fc78) SHA1(9a9d7925847d7a36930f0761c70f67a9affc5e7c) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

GAME( 1986, sothello, 0, sothello, sothello, sothello_state, empty_init, ROT0, "Success / Fujiwara", "Super Othello", MACHINE_SUPPORTS_SAVE )
