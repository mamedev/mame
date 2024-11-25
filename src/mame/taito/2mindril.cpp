// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, David Haywood
/*
 Two Minute Drill - Taito 1993
 -----------------------------
 Half Video, Half Mechanical?
(video hw + motion/acceleration sensor ?)

 preliminary driver by
  David Haywood
  Tomasz Slanina
  Angelo Salese

TODO:
 - understand the ball hit sensor
 - simulate the sensors (there are still some shutter errors/defender errors that pops up)
 - Hook-up timers for shutter/defender sensors (check service mode)
 - Dip-Switches

 Brief hardware overview:
 ------------------------

 Main processor   - 68000 16Mhz

 Sound            - Yamaha YM2610B

 Taito custom ICs - TC0400YSC (m68k -> ym2610 communication)
                  - TC0260DAR (palette chip)
                  - TC0630FDP (Taito F3 video chip)
                  - TC0510NIO (known input chip)

DAC               -26.6860Mhz
                  -32.0000Mhz

*/

#include "emu.h"
#include "taito_f3.h"

#include "cpu/m68000/m68000.h"
#include "taitoio.h"
#include "sound/ymopn.h"
#include "speaker.h"


namespace {

class _2mindril_state : public taito_f3_state
{
public:
	_2mindril_state(const machine_config &mconfig, device_type type, const char *tag) :
		taito_f3_state(mconfig, type, tag),
		m_in0(*this, "IN0")
	{ }

	void drill(machine_config &config);

	void init_drill();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* input-related */
	required_ioport m_in0;
	u8         m_defender_sensor;
	u8         m_shutter_sensor;
	u16        m_irq_reg;

	/* devices */
	u8 arm_pwr_r();
	u8 sensors_r();
	void coins_w(u8 data);
	void sensors_w(u16 data);
	u16 irq_r();
	void irq_w(offs_t offset, u16 data, u16 mem_mask);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	//INTERRUPT_GEN_MEMBER(drill_device_irq);
	void irqhandler(int state);

	void drill_map(address_map &map) ATTR_COLD;

	#ifdef UNUSED_FUNCTION
protected:
	TIMER_CALLBACK_MEMBER(set_shutter_req);
	TIMER_CALLBACK_MEMBER(set_defender_req);

	emu_timer *m_shutter_req_timer;
	emu_timer *m_defender_req_timer;
	#endif
};


u8 _2mindril_state::arm_pwr_r()
{
	int arm_pwr = m_in0->read();//throw

	if (arm_pwr > 0xe0) return ~0x18;
	if (arm_pwr > 0xc0) return ~0x14;
	if (arm_pwr > 0x80) return ~0x12;
	if (arm_pwr > 0x40) return ~0x10;
	else return ~0x00;
}

u8 _2mindril_state::sensors_r()
{
	return (m_defender_sensor) | (m_shutter_sensor);
}

void _2mindril_state::coins_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
}

/*
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )//up sensor <- shutter
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )//down sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )//left sensor <-defender
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )//right sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
*/
#ifdef UNUSED_FUNCTION
TIMER_CALLBACK_MEMBER(_2mindril_state::set_shutter_req)
{
	m_shutter_sensor = param;
}

TIMER_CALLBACK_MEMBER(_2mindril_state::set_defender_req)
{
	m_defender_sensor = param;
}
#endif

void _2mindril_state::sensors_w(u16 data)
{
	/*---- xxxx ---- ---- select "lamps" (guess)*/
	/*---- ---- ---- -x-- lamp*/
	if (data & 1)
	{
		//m_shutter_req_timer->adjust(attotime::from_seconds(2), 0x01);
		m_shutter_sensor = 0x01;
	}
	else if (data & 2)
	{
		//m_shutter_req_timer->adjust(attotime::from_seconds(2), 0x02);
		m_shutter_sensor = 0x02;
	}

	if (data & 0x1000 || data & 0x4000)
	{
		//m_defender_req_timer->adjust(attotime::from_seconds(2), 0x08);
		m_defender_sensor = 0x08;
	}
	else if (data & 0x2000 || data & 0x8000)
	{
		//m_defender_req_timer->adjust(attotime::from_seconds(2), 0x04);
		m_defender_sensor = 0x04;
	}
}

u16 _2mindril_state::irq_r()
{
	return m_irq_reg;
}

void _2mindril_state::irq_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
	(note: could rather be irq mask)
	---- ---- ---x ---- irq lv 5 ack, 0->1 latch
	---- ---- ---- x--- irq lv 4 ack, 0->1 latch
	---- ---- -??- -??? connected to the other levels?
	*/
	if (((m_irq_reg & 8) == 0) && data & 8)
		m_maincpu->set_input_line(4, CLEAR_LINE);

	if (((m_irq_reg & 0x10) == 0) && data & 0x10)
		m_maincpu->set_input_line(5, CLEAR_LINE);

	if (data & 0xffe7)
		printf("%04x\n",data);

	COMBINE_DATA(&m_irq_reg);
}

void _2mindril_state::drill_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x3000ff).ram();
	map(0x400000, 0x40ffff).ram().share("spriteram");
	map(0x410000, 0x41bfff).ram().w(FUNC(_2mindril_state::pf_ram_w)).share("pf_ram");
	map(0x41c000, 0x41dfff).ram().w(FUNC(_2mindril_state::textram_w)).share("textram");
	map(0x41e000, 0x41ffff).ram().w(FUNC(_2mindril_state::charram_w)).share("charram");
	map(0x420000, 0x42ffff).ram().share("line_ram");
	map(0x430000, 0x43ffff).ram().w(FUNC(_2mindril_state::pivot_w)).share("pivot_ram");
	map(0x460000, 0x46000f).w(FUNC(_2mindril_state::control_0_w));
	map(0x460010, 0x46001f).w(FUNC(_2mindril_state::control_1_w));
	map(0x500000, 0x501fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x502022, 0x502023).nopw(); //countinously switches between 0 and 2
	map(0x600000, 0x600007).rw("ymsnd", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write)).umask16(0x00ff);
	map(0x60000c, 0x60000d).rw(FUNC(_2mindril_state::irq_r), FUNC(_2mindril_state::irq_w));
	map(0x60000e, 0x60000f).ram(); // unknown purpose, zeroed at start-up and nothing else
	map(0x700000, 0x70000f).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write)).umask16(0xff00);
	map(0x800000, 0x800001).w(FUNC(_2mindril_state::sensors_w));
}

static INPUT_PORTS_START( drill )
	PORT_START("DSW") //Dip-Switches. PCB labelled DIPSWA
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:2")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:5")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:6")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIPSWA:8")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN0")//sensors
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select SW-1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select SW-2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select SW-3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Select SW-4")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ STEP8(0,4*8) },
	32*8
};

static const gfx_layout pivotlayout =
{
	8,8,
	2048,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ STEP8(0,4*8) },
	32*8
};

static const gfx_layout layout_6bpp_sprite_hi =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ STEP2(0,1)/**/,0,0,0,0/**/ },
	{ STEP4(3*2,-2), STEP4(7*2,-2), STEP4(11*2,-2), STEP4(15*2,-2) },
	{ STEP16(0,16*2) },
	16*16*2
};

static const gfx_layout layout_6bpp_tile_hi =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ 8,0/**/,0,0,0,0/**/ },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*2*2) },
	16*16*2
};

static GFXDECODE_START( gfx_2mindril )
	GFXDECODE_ENTRY( nullptr,      0, charlayout,             0x0000, 0x0400>>4 ) /* Dynamically modified */
	GFXDECODE_ENTRY( nullptr,      0, pivotlayout,            0x0000,  0x400>>4 ) /* Dynamically modified */
	GFXDECODE_ENTRY( "sprites",    0, gfx_16x16x4_packed_lsb, 0x1000, 0x1000>>4 ) // low 4bpp of 6bpp sprite data
	GFXDECODE_ENTRY( "tilemap",    0, gfx_16x16x4_packed_lsb, 0x0000, 0x2000>>4 ) // low 4bpp of 6bpp tilemap data
	GFXDECODE_ENTRY( "tilemap_hi", 0, layout_6bpp_tile_hi,    0x0000, 0x2000>>4 ) // hi 2bpp of 6bpp tilemap data
	GFXDECODE_ENTRY( "sprites_hi", 0, layout_6bpp_sprite_hi,  0x1000, 0x1000>>4 ) // hi 2bpp of 6bpp sprite data
GFXDECODE_END


INTERRUPT_GEN_MEMBER(_2mindril_state::vblank_irq)
{
	device.execute().set_input_line(4, ASSERT_LINE);
}

#if 0
INTERRUPT_GEN_MEMBER(_2mindril_state::drill_device_irq)
{
	device.execute().set_input_line(5, ASSERT_LINE);
}
#endif

/* WRONG,it does something with 60000c & 700002,likely to be called when the player throws the ball.*/
void _2mindril_state::irqhandler(int state)
{
//  m_maincpu->set_input_line(5, state ? ASSERT_LINE : CLEAR_LINE);
}


void _2mindril_state::machine_start()
{
	save_item(NAME(m_defender_sensor));
	save_item(NAME(m_shutter_sensor));
	save_item(NAME(m_irq_reg));

#ifdef UNUSED_FUNCTION
	m_shutter_req_timer = timer_alloc(FUNC(_2mindril_state::set_shutter_req), this);
	m_defender_req_timer = timer_alloc(FUNC(_2mindril_state::set_defender_req), this);
#endif
}

void _2mindril_state::machine_reset()
{
	m_defender_sensor = 0;
	m_shutter_sensor = 0;
	m_irq_reg = 0;
}

void _2mindril_state::drill(machine_config &config)
{
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &_2mindril_state::drill_map);
	m_maincpu->set_vblank_int("screen", FUNC(_2mindril_state::vblank_irq));
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_2mindril);

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("DSW");
	tc0510nio.read_1_callback().set(FUNC(_2mindril_state::arm_pwr_r));
	tc0510nio.read_2_callback().set(FUNC(_2mindril_state::sensors_r));
	tc0510nio.write_4_callback().set(FUNC(_2mindril_state::coins_w));
	tc0510nio.read_7_callback().set_ioport("COINS");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* inaccurate, same as Taito F3? (needs screen raw params anyway) */
	m_screen->set_size(40*8+48*2, 32*8);
	m_screen->set_visarea(46, 40*8-1 + 46, 24, 24+224-1);
	m_screen->set_screen_update(FUNC(_2mindril_state::screen_update));
	m_screen->screen_vblank().set(FUNC(_2mindril_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x2000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16000000/2));
	ymsnd.irq_handler().set(FUNC(_2mindril_state::irqhandler));
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}


ROM_START( 2mindril )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d58-38.ic11", 0x00000, 0x40000, CRC(c58e8e4f) SHA1(648db679c3bfb5de1cd6c1b1217773a2fe56f11b) ) // Ver 2.93A 1994/02/16 09:45:00
	ROM_LOAD16_BYTE( "d58-37.ic9",  0x00001, 0x40000, CRC(19e5cc3c) SHA1(04ac0eef893c579fe90d91d7fd55c5741a2b7460) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "d58-11.ic31", 0x000000, 0x200000,  CRC(dc26d58d) SHA1(cffb18667da18f5367b02af85a2f7674dd61ae97) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE00 )
	ROM_REGION( 0x200000, "sprites_hi", ROMREGION_ERASE00 )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD32_WORD( "d58-08.ic27", 0x000000, 0x200000, CRC(9f5a3f52) SHA1(7b696bd823819965b974c853cebc1660750db61e) )
	ROM_LOAD32_WORD( "d58-09.ic28", 0x000002, 0x200000, CRC(d8f6a86a) SHA1(d6b2ec309e21064574ee63e025ae4716b1982a98) )

	ROM_REGION( 0x200000, "tilemap_hi", 0 )
	ROM_LOAD       ( "d58-10.ic29", 0x000000, 0x200000, CRC(74c87e08) SHA1(f39b3a64f8338ccf5ca6eb76cee92a10fe0aad8f) )
ROM_END

ROM_START( 2mindrila )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d58-17.ic11", 0x00000, 0x40000, CRC(3abc3e2c) SHA1(ee70c9fc20bf31a427384230672110895eeb0b90) ) // Ver 2.2A 1993/10/18 03:20:00
	ROM_LOAD16_BYTE( "d58-16.ic9",  0x00001, 0x40000, CRC(a82101dd) SHA1(4eacac76fc47294f28af65e2a02d7c279bd52f2d) ) // Program ROMs have asterisks on the labels, "16*" and "17*"

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) /* Samples */
	ROM_LOAD( "d58-11.ic31", 0x000000, 0x200000,  CRC(dc26d58d) SHA1(cffb18667da18f5367b02af85a2f7674dd61ae97) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE00 )
	ROM_REGION( 0x200000, "sprites_hi", ROMREGION_ERASE00 )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD32_WORD( "d58-08.ic27", 0x000000, 0x200000, CRC(9f5a3f52) SHA1(7b696bd823819965b974c853cebc1660750db61e) )
	ROM_LOAD32_WORD( "d58-09.ic28", 0x000002, 0x200000, CRC(d8f6a86a) SHA1(d6b2ec309e21064574ee63e025ae4716b1982a98) )

	ROM_REGION( 0x200000, "tilemap_hi", 0 )
	ROM_LOAD       ( "d58-10.ic29", 0x000000, 0x200000, CRC(74c87e08) SHA1(f39b3a64f8338ccf5ca6eb76cee92a10fe0aad8f) )
ROM_END

void _2mindril_state::init_drill()
{
	m_game = TMDRILL;
	tile_decode();
}

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE  INPUT  CLASS            INIT        ROT   COMPANY                      FULLNAME                                   FLAGS
GAME( 1993, 2mindril,  0,        drill,   drill, _2mindril_state, init_drill, ROT0, "Taito America Corporation", "Two Minute Drill (Ver 2.93A 1994/02/16)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_MECHANICAL)
GAME( 1993, 2mindrila, 2mindril, drill,   drill, _2mindril_state, init_drill, ROT0, "Taito America Corporation", "Two Minute Drill (Ver 2.2A 1993/10/18)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_MECHANICAL)
