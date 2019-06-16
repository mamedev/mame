// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Fruit Dream (c) 1993 Nippon Data Kiki / Star Fish
    Gemcrush (c) 1996 Star Fish
	
    driver by Angelo Salese

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - inputs are grossly mapped, lack of any input test doesn't help at all;
    - lamps?
    - service mode?
    - nvram?
    - dfruit: (possible bug) has an X on top-left corner after POST, 
              is it supposed to be disabled somehow?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/tc009xlvc.h"
#include "machine/timer.h"
#include "sound/2203intf.h"
#include "screen.h"
#include "speaker.h"

class dfruit_state : public driver_device
{
public:
	dfruit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "tc0091lvc")
	{ }

	void dfruit(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<tc0091lvc_device> m_vdp;

	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	DECLARE_WRITE8_MEMBER(output_w);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void dfruit_map(address_map &map);
	void tc0091lvc_map(address_map &map);
};

void dfruit_state::video_start()
{
}

uint32_t dfruit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_vdp->screen_update(screen, bitmap, cliprect);

	return 0;
}

WRITE_LINE_MEMBER(dfruit_state::screen_vblank)
{
	if (state)
	{
		m_vdp->screen_eof();
	}
}


void dfruit_state::tc0091lvc_map(address_map &map)
{
	map(0x0000, 0xfdff).m(m_vdp, FUNC(tc0091lvc_device::cpu_map));

	map(0x8000, 0x9fff).ram();

	map(0xfe00, 0xfeff).rw(m_vdp, FUNC(tc0091lvc_device::vregs_r), FUNC(tc0091lvc_device::vregs_w));
	map(0xff00, 0xff02).rw(m_vdp, FUNC(tc0091lvc_device::irq_vector_r), FUNC(tc0091lvc_device::irq_vector_w));
	map(0xff03, 0xff03).rw(m_vdp, FUNC(tc0091lvc_device::irq_enable_r), FUNC(tc0091lvc_device::irq_enable_w));
	map(0xff04, 0xff07).rw(m_vdp, FUNC(tc0091lvc_device::ram_bank_r), FUNC(tc0091lvc_device::ram_bank_w));
	map(0xff08, 0xff08).rw(m_vdp, FUNC(tc0091lvc_device::rom_bank_r), FUNC(tc0091lvc_device::rom_bank_w));
}

void dfruit_state::dfruit_map(address_map &map)
{
	tc0091lvc_map(map);
	map(0xa000, 0xa003).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa004, 0xa005).rw("opn", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa008, 0xa008).nopr(); //watchdog
}


static INPUT_PORTS_START( dfruit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Bookkeeping")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_DIPNAME( 0x20, 0x20, "IN0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Alt Bookkeeping") // same as above
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop Reel 1 / Double-Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop Reel 3 / Black")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop Reel 2 / Red")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gemcrush )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x02, 0x02, "IN0" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// guess: these hardlocks game if held for too much time unlike bit 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// guess: these works only on round select
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	
	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DS1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DS1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) ) /* Manual states "Somewhat Hard" */
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Nudity" ) PORT_DIPLOCATION("DS1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:8") /* Manual states Not Used */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(dfruit_state::scanline_callback)
{
	int scanline = param;

	if (scanline == 240 && (m_vdp->irq_enable() & 4))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(2)); // TC0091LVC
	}

	if (scanline == 0 && (m_vdp->irq_enable() & 2))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(1)); // TC0091LVC
	}

	if (scanline == 196 && (m_vdp->irq_enable() & 1))
	{
		//m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vdp->irq_vector(0)); // TC0091LVC
	}
}

WRITE8_MEMBER(dfruit_state::output_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
	machine().bookkeeping().coin_lockout_w(0, data & 4);
	machine().bookkeeping().coin_lockout_w(1, data & 8);
}

#define MASTER_CLOCK XTAL(14'000'000)

void dfruit_state::dfruit(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/2); //!!! TC0091LVC !!!
	m_maincpu->set_addrmap(AS_PROGRAM, &dfruit_state::dfruit_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(dfruit_state::scanline_callback), "screen", 0, 1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(dfruit_state::screen_update));
	screen.screen_vblank().set(FUNC(dfruit_state::screen_vblank));
	screen.set_palette("tc0091lvc:palette");

	TC0091LVC(config, m_vdp, 0);

	i8255_device &ppi(I8255A(config, "i8255"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("IN2");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ym2203_device &opn(YM2203(config, "opn", MASTER_CLOCK/4));
	//opn.port_a_read_callback().set_ioport("IN4");
	opn.port_b_read_callback().set_ioport("DSW");
	opn.port_a_write_callback().set(FUNC(dfruit_state::output_w));
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dfruit )
	ROM_REGION( 0x40000, "tc0091lvc", 0 )
	ROM_LOAD( "n-3800ii_ver.1.20.ic2", 0x00000, 0x40000, CRC(4e7c3700) SHA1(17bc731a91460d8f67c2b2b6e038641d57cf93be) )

	ROM_REGION( 0x80000, "tc0091lvc:gfx", 0 )
	ROM_LOAD( "c2.ic10", 0x00000, 0x80000, CRC(d869ab24) SHA1(382e874a846855a7f6f8811625aaa30d9dfa1ce2) )
ROM_END

ROM_START( gemcrush )
	ROM_REGION( 0x40000, "tc0091lvc", ROMREGION_ERASE00 )
    ROM_LOAD( "gcj_00.ic2",   0x000000, 0x040000, CRC(e1431390) SHA1(f1f63e23d4b73cc099adddeadcf1ea3e27688bcd) ) /* ST M27C2001 EPROM */

 	ROM_REGION( 0x80000, "tc0091lvc:gfx", ROMREGION_ERASE00 )
    ROM_LOAD( "gcj_01.ic10",  0x000000, 0x080000, CRC(5b9e7a6e) SHA1(345357feed8e80e6a06093fcb69f2b38063d057a) ) /* HN27C4096 EPROM */
ROM_END

GAME( 1993, dfruit,    0,   dfruit,  dfruit,   dfruit_state, empty_init, ROT0,   "Nippon Data Kiki / Star Fish", "Fruit Dream (Japan)", 0 )
GAME( 1996, gemcrush,  0,   dfruit,  gemcrush, dfruit_state, empty_init, ROT270, "Star Fish", "Gemcrush (Japan)", MACHINE_NO_COCKTAIL )
