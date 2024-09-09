// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/***************************************************************************************************************************

Cherry Chance / チェリーチャンス (c) 1987 Taito Corporation

A cherry-type game that uses the tnzs video chip.

TODO:
- Undumped color proms -> ugly colors;
- Complete I/O, requires manual for DIPs and (likely non-)JAMMA pinout;
- Verify clock dividers for Z80 and YM2149;

============================================================================================================================

Cherry Chance Readme

This one should be a simple project, it uses a Z80 and a Seta graphics chipset and a YM2149 for sound. It is a slot machine.

There are 3 banks of dipswitches, 4,8,8. Battery backup of a 4364 cpu ram. 2 6264 video rams. All 5 eproms are 27512

Chip    checksum
cpu $ba0d
cha0    $2ed7
cha1    $dc81
cha2    $cca8
cha3    $10d8

2 color PROMs for the output. will get those dumped as well.


*****************************************************************************************************************************/


#include "emu.h"
#include "tnzs_video.h"

#include "cpu/z80/z80.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "taitoio_opto.h"

#include "speaker.h"


namespace {

class cchance_state : public tnzs_video_state_base
{
public:
	cchance_state(const machine_config &mconfig, device_type type, const char *tag)
		: tnzs_video_state_base(mconfig, type, tag)
		, m_opto(*this, "opto")
		, m_hopper(*this, "hopper")
		, m_dswc(*this, "DSWC")
	{ }

	void cchance(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<taitoio_opto_device> m_opto;
	required_device<hopper_device> m_hopper;
	required_ioport m_dswc;

	void output_0_w(uint8_t data);
	void output_1_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;

	bool m_vblank_irq = false, m_io_irq = false;
	uint8_t m_irq_ack = 0;
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
};


void cchance_state::output_0_w(uint8_t data)
{
	// -x-- ---- slottle sol[enoid]
	// ---- x--- Enabled on payout, untested by service mode
	// ---- -x-- Coin counter
	// ---- --x- divider / diverter
//  machine().bookkeeping().coin_lockout_w(0, BIT(data, 0));

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
}

void cchance_state::output_1_w(uint8_t data)
{
	m_hopper->motor_w(BIT(data, 6));
//  m_bell_io = (data & 0x80) >>4;
}

void cchance_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();

	map(0xa000, 0xafff).rw(m_spritegen, FUNC(x1_001_device::spritecodelow_r8), FUNC(x1_001_device::spritecodelow_w8));
	map(0xb000, 0xbfff).rw(m_spritegen, FUNC(x1_001_device::spritecodehigh_r8), FUNC(x1_001_device::spritecodehigh_w8));

	map(0xc000, 0xdfff).ram();

	map(0xe000, 0xe2ff).rw(m_spritegen, FUNC(x1_001_device::spriteylow_r8), FUNC(x1_001_device::spriteylow_w8));
	map(0xe300, 0xe303).ram().mirror(0xfc).w(m_spritegen, FUNC(x1_001_device::spritectrl_w8));  // control registers (0x80 mirror used by Arkanoid 2)
	map(0xe800, 0xe800).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));   // enable / disable background transparency

	map(0xf000, 0xf000).lrw8(
		NAME([this] () {
			// DSWC is read during startup and at input test RAM buffer=$c4c2
			// bits 4, 5 used as irq vector
			// bit 7 chip selects acknowledge on writes
			return !m_vblank_irq << 4 | !m_io_irq << 5 | (m_dswc->read() & 0xf);
		}),
		NAME([this] (u8 data) {
			if (BIT(data, 7))
			{
				if (!BIT(m_irq_ack, 4) && BIT(data, 4))
					m_vblank_irq = false;
				if (!BIT(m_irq_ack, 5) && BIT(data, 5))
					m_io_irq = false;

				if (!m_vblank_irq && !m_io_irq)
					m_maincpu->set_input_line(0, CLEAR_LINE);
			}

			m_irq_ack = data;
		})
	);
	map(0xf001, 0xf001).portr("IN0").w(FUNC(cchance_state::output_0_w));
	map(0xf002, 0xf002).portr("IN1").w(FUNC(cchance_state::output_1_w));
	map(0xf800, 0xf801).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xf801, 0xf801).r("aysnd", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( cchance )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, coin_sense_w)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto", taitoio_opto_device, opto_l_r)
	// Hopper related? Goes "hopper time out" if IP_ACTIVE_LOW as suggested by service mode (buggy?)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Over") // "Hop Over"?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Slottle")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Drop SW") // "Coin Drop SW" as per other Taito gamblers?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Key")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Last Key")

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Credit SW")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet SW")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Crt Key") // analyzer
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout SW")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	// debug/cheat
	PORT_DIPNAME( 0x01, 0x01, "Credits at start" ) PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x01, "0" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// unused according to service mode
	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "DSWB:1")
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "DSWB:2")
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "DSWB:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "DSWB:4")
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "DSWB:5")
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "DSWB:6")
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "DSWB:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "DSWB:8")

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" ) PORT_DIPLOCATION("DSWC:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWC:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWC:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSWC:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout cchance_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};



static GFXDECODE_START( gfx_cchance )
	GFXDECODE_ENTRY( "gfx1", 0, cchance_layout,   0x0, 32  )
GFXDECODE_END

void cchance_state::machine_start()
{
	tnzs_video_state_base::machine_start();

	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_io_irq));
	save_item(NAME(m_irq_ack));
//  save_item(NAME(m_hop_io));
//  save_item(NAME(m_bell_io));
}

void cchance_state::machine_reset()
{
	tnzs_video_state_base::machine_reset();

	m_vblank_irq = m_io_irq = false;
	m_irq_ack = 0;
//  m_hop_io = 0;
//  m_bell_io = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(cchance_state::scanline_cb)
{
	int scanline = param;

	// vblank irq
	if (scanline == 30*8)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_vblank_irq = true;
		m_io_irq = false;
	}

	// i/o irq, unknown source
	if (scanline == 0)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_vblank_irq = false;
		m_io_irq = true;
	}
}

void cchance_state::cchance(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // LH0080B
	m_maincpu->set_addrmap(AS_PROGRAM, &cchance_state::main_map);
	//m_maincpu->set_vblank_int("screen", FUNC(cchance_state::irq0_line_hold));
	TIMER(config, "scantimer").configure_scanline(FUNC(cchance_state::scanline_cb), "screen", 0, 1);

	TAITOIO_OPTO(config, "opto", 0);
	HOPPER(config, m_hopper, attotime::from_msec(100));

	X1_001(config, m_spritegen, 12_MHz_XTAL, m_palette, gfx_cchance);
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.5);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(cchance_state::screen_update_tnzs));
	m_screen->screen_vblank().set(FUNC(cchance_state::screen_vblank_tnzs));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(cchance_state::prompalette), 512);

	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 12_MHz_XTAL / 8));
	aysnd.port_a_read_callback().set_ioport("DSWA");
	aysnd.port_b_read_callback().set_ioport("DSWB");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.25);
}

ROM_START( cchance )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chance-cccpu.bin", 0x00000, 0x10000, CRC(77531028) SHA1(6f647dea3f1c5884c32a35e04ab6c8a61688171a) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD("chance-cccha3.bin", 0x00000, 0x10000, CRC(2a2979c9) SHA1(5036313e219ec561fa6753f0db6bb28c6fc97963) )
	ROM_LOAD("chance-cccha2.bin", 0x10000, 0x10000, CRC(fa5ccf5b) SHA1(21957a6a7b88c315d1fbb82e98a924a637a28397) )
	ROM_LOAD("chance-cccha1.bin", 0x20000, 0x10000, CRC(26fddc7d) SHA1(d89757c28f14dccdc7d898e19fea59f41f4fa903) )
	ROM_LOAD("chance-cccha0.bin", 0x30000, 0x10000, CRC(df8403cf) SHA1(997a6e07079fcbcae2fb82bbd7af0db9b90a03e0))

	ROM_REGION( 0x0400, "proms", 0 )        // color PROMs
	ROM_LOAD( "prom1", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom2", 0x0200, 0x0200, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1987, cchance, 0, cchance, cchance, cchance_state, empty_init, ROT0, "Taito Corporation", "Cherry Chance", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // year/manufacturer confirmed from Taito old "Arcade Game History" page
