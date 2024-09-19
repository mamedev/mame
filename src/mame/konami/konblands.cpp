// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    GX455 - Konami Badlands

    driver by Angelo Salese

    TODO:
    - verify interrupts, service mode seems way too fast and it randomly crashes;
    - fix overlay positioning & transparency enable;
    - dipswitches;
    - add sn76496 latch mechanism (should actually be in the device itself);

    Notes:
    - to enter service mode hold start 1 & 2 at POST.

***************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "machine/ldv1000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK XTAL(18'432'000)

class konblands_state : public driver_device
{
public:
	konblands_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_laserdisc(*this, "laserdisc")
		, m_vram(*this, "vram")
		, m_gfxdecode(*this, "gfxdecode")
	{
	}

	void konblands(machine_config &config);
	void konblandsh(machine_config &config);

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void konblands_palette(palette_device &palette) const;
	uint8_t ldp_r();
	void nmi_enable_w(uint8_t data);
	void irq_enable_w(uint8_t data);
	void firq_enable_w(uint8_t data);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(timer_irq);
	void ld_command_strobe_cb(int state);

	void konblands_map(address_map &map) ATTR_COLD;
	void konblandsh_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<uint8_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;

	bool m_nmi_enable = false, m_irq_enable = false, m_firq_enable = false;
};

void konblands_state::konblands_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void konblands_state::video_start()
{
}

uint32_t konblands_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			uint8_t tile = m_vram[count];

			gfx->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 8);

			count++;
		}
	}

	return 0;
}

uint8_t konblands_state::ldp_r()
{
	return m_laserdisc->data_r();
}

void konblands_state::nmi_enable_w(uint8_t data)
{
	m_nmi_enable = bool(BIT(data,0));
}

void konblands_state::irq_enable_w(uint8_t data)
{
	m_irq_enable = bool(BIT(data,0));
}

void konblands_state::firq_enable_w(uint8_t data)
{
	m_firq_enable = bool(BIT(data,0));
}

void konblands_state::konblands_map(address_map &map)
{
	map(0x0000, 0x0000).portr("DSW1").nopw(); // sn latch
	map(0x0800, 0x0800).portr("DSW2").w(m_laserdisc, FUNC(pioneer_ldv1000_device::data_w));
	map(0x1000, 0x1000).nopw().r(FUNC(konblands_state::ldp_r)); // led
	map(0x1001, 0x1001).nopw(); // coin counter 2
	map(0x1002, 0x1002).nopw(); // coin counter 1
	map(0x1003, 0x1003).nopw(); // enable overlay transparency
	map(0x1004, 0x1004).w(FUNC(konblands_state::nmi_enable_w));
	map(0x1005, 0x1005).nopw(); // enable audio
	map(0x1006, 0x1006).w(FUNC(konblands_state::irq_enable_w));
	map(0x1007, 0x1007).w(FUNC(konblands_state::firq_enable_w));
	map(0x1800, 0x1800).portr("INPUTS").w("sn", FUNC(sn76496_device::write));
	map(0x4000, 0x47ff).ram().share("vram");
	map(0x4800, 0x4bff).ram();
	map(0x5800, 0x5800).nopw(); // watchdog
	map(0x8000, 0x9fff).nopr(); // diagnostic ROM?
	map(0xc000, 0xffff).rom().region("ipl",0);
}

void konblands_state::konblandsh_map(address_map &map)
{
	map(0x0000, 0x0000).r(FUNC(konblands_state::ldp_r));
	map(0x0400, 0x0400).w(m_laserdisc, FUNC(pioneer_ldv1000_device::data_w));
	map(0x0802, 0x0802).nopw(); // led
	map(0x0803, 0x0803).nopw(); // enable overlay transparency
	map(0x0806, 0x0806).nopr().w(FUNC(konblands_state::irq_enable_w));
	map(0x0807, 0x0807).nopr().w(FUNC(konblands_state::firq_enable_w));
	map(0x0c00, 0x0c00).portr("INPUTS");
	map(0x1000, 0x1000).portr("DSW1");
	map(0x1400, 0x1400).w("sn", FUNC(sn76496_device::write));
	map(0x1800, 0x1800).nopw(); // sn latch
	map(0x2000, 0x27ff).ram().share("vram");
	map(0x2800, 0x2fff).ram();
	map(0xc000, 0xffff).rom().region("ipl",0);
}

static INPUT_PORTS_START( konblands )
	PORT_START("DSW1")
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x22, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x55, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x44, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x11, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xff, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x77, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x33, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xee, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x66, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xdd, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xcc, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xbb, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xaa, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x99, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, "2")
	PORT_DIPSETTING(    0x02, "3")
	PORT_DIPSETTING(    0x01, "5")
	PORT_DIPSETTING(    0x00, "7")
	/* SW3-SW7 NOT IN USE.  Keep switches in OFF position (per manual) */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static GFXDECODE_START( gfx_konblands )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb,     0, 1 )
GFXDECODE_END


void konblands_state::machine_start()
{
}

void konblands_state::machine_reset()
{
	m_nmi_enable = false;
	m_irq_enable = false;
	m_firq_enable = false;
}

INTERRUPT_GEN_MEMBER(konblands_state::vblank_irq)
{
	if (m_nmi_enable == true)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(konblands_state::timer_irq)
{
	if (m_firq_enable == true)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

void konblands_state::ld_command_strobe_cb(int state)
{
	if(m_irq_enable == true)
		m_maincpu->set_input_line(M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void konblands_state::konblands(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_CLOCK/12);
	m_maincpu->set_addrmap(AS_PROGRAM, &konblands_state::konblands_map);
	m_maincpu->set_vblank_int("screen", FUNC(konblands_state::vblank_irq));
	m_maincpu->set_periodic_int(FUNC(konblands_state::timer_irq), attotime::from_hz(8)); // 8 times per frame

	/* video hardware */
	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->command_strobe_callback().set(FUNC(konblands_state::ld_command_strobe_cb));
	// TODO: might be different
	m_laserdisc->set_overlay(512, 256, FUNC(konblands_state::screen_update));

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_konblands);

	PALETTE(config, "palette", FUNC(konblands_state::konblands_palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76496(config, "sn", MASTER_CLOCK/12).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void konblands_state::konblandsh(machine_config &config)
{
	konblands(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &konblands_state::konblandsh_map);
}

/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( kbadlands )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "badlands.a13",   0x0000, 0x2000, CRC(a44776d6) SHA1(eb3b5b57a99e9bd2559ced3b279fb3566c918f9a) )
	ROM_LOAD( "badlands.a14",   0x2000, 0x2000, CRC(82cb4614) SHA1(0cab824b4f3fb29e300f9c05911422d6047d073b) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "badlands.c8",   0x0000, 0x2000, CRC(590209fe) SHA1(8dfc836420e4c3fa417ec0aefb617a7abd0ccbc2) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "badlands.c4",    0x000, 0x020, CRC(6757be8d) SHA1(1c9c24e29017f0a16b8a7dedd9776109e7e5734c) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "badlands", 0, NO_DUMP )
ROM_END


ROM_START( kbadlandsh )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "bl_hit.7a",    0x0000, 0x2000, CRC(a135e444) SHA1(7ef5394c698a5867aef200f577b8708df455b653) )
	ROM_LOAD( "bl_hit.6a",    0x2000, 0x2000, CRC(4c287f37) SHA1(b6b6b64174f1fd014b6c808015f1b0e65b56d24b) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "bl_hit.9c",    0x000000, 0x002000, CRC(44c3441e) SHA1(6b42961d31e5d025758cdfdc573648a83004577d) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "bl_hit.4f",    0x000000, 0x000020, CRC(0226f881) SHA1(b17c5681fca5ae65128793cf263725e2fe1314de) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "badlands", 0, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1984, kbadlands,  0,         konblands,  konblands, konblands_state, empty_init, ROT0, "Konami",      "Badlands (Konami, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1984, kbadlandsh, kbadlands, konblandsh, konblands, konblands_state, empty_init, ROT0, "Konami",      "Badlands (Konami, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
