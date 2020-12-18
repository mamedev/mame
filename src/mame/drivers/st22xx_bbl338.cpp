// license:BSD-3-Clause
// copyright-holders:David Haywood, AJR

#include "emu.h"
#include "cpu/m6502/st2205u.h"
#include "video/bl_handhelds_lcdc.h"
#include "screen.h"

class st22xx_bbl338_state : public driver_device
{
public:
	st22xx_bbl338_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_lcdc(*this, "lcdc")
	{
	}
	
	void st22xx_bbl338(machine_config &config);

private:
	void st22xx_bbl338_map(address_map &map);

	required_device<st2xxx_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<bl_handhelds_lcdc_device> m_lcdc;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


void st22xx_bbl338_state::st22xx_bbl338_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);

	map(0x600000, 0x600000).w(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_command_w));
	map(0x604000, 0x604000).rw(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_data_r), FUNC(bl_handhelds_lcdc_device::lcdc_data_w));
}

u32 st22xx_bbl338_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}


static INPUT_PORTS_START(st22xx_bbl338)
	PORT_START("IN0") // not confirmed, but probably similar to other units
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // maybe ON/OFF
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("SOUND")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")

	PORT_START("IN1") // not confirmed, but probably similar to other units
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT(0x06, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_CONFNAME( 0x01,  0x01, DEF_STR( Language ) )
	PORT_CONFSETTING( 0x00, "Chinese" )
	PORT_CONFSETTING( 0x01, "English" )
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED) // probably unused
INPUT_PORTS_END

void st22xx_bbl338_state::st22xx_bbl338(machine_config &config)
{
	ST2302U(config, m_maincpu, 12000000); // likely higher clock
	m_maincpu->set_addrmap(AS_DATA, &st22xx_bbl338_state::st22xx_bbl338_map);
	m_maincpu->in_pa_callback().set_ioport("IN0");
	m_maincpu->in_pb_callback().set_ioport("IN1");
	m_maincpu->in_pc_callback().set_ioport("IN2");

	// incorrect for bbl338
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128); 
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(st22xx_bbl338_state::screen_update));

	// incorrect for bbl338 (or will need changes to support higher resolutions)
	BL_HANDHELDS_LCDC(config, m_lcdc, 0);
}

ROM_START( bbl338 )
	// is internal ROM used? the code in the external ROM contains a bank for 4000-6fff including vectors at least.
	// it sets stack to 14f, but quickly jumps to 150 in RAM where there is no code? was something meant to have copied
	// code there earlier?

	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "en29lv160ab.u1", 0x000000, 0x200000, CRC(2c73e16c) SHA1(e2c69b3534e32ef384c0c2f5618118a419326e3a) )
ROM_END

ROM_START( dphh8213 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29lv160cb.u1", 0x000000, 0x200000, CRC(c8e7e355) SHA1(726f28c2c9ab012a6842f9f30a0a71538741ba14) )
ROM_END

// this is uses a higher resolution display than the common units, but not as high as the SunPlus based ones
COMP( 201?, bbl338, 0,      0,      st22xx_bbl338, st22xx_bbl338, st22xx_bbl338_state, empty_init, "BaoBaoLong", "Portable Game Player BBL-338 (BaoBaoLong, 48-in-1)", MACHINE_IS_SKELETON )

// Language controlled by port bit, set at factory, low resolution
COMP( 201?, dphh8213, 0,      0,      st22xx_bbl338, st22xx_bbl338, st22xx_bbl338_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 20-in-1 - Model 8213", MACHINE_IS_SKELETON )
