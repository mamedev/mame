// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    GPL16250 / GPAC800 / GMC384 / GCM420 related support

    GPL16250 is the GeneralPlus / SunPlus part number
    GPAC800 is the JAKKS Pacific codename
    GMC384 / GCM420 is what is printed on the die

    ----

    GPL16250 games using NAND + RAM configuration
*/

/*
    map info (NAND type)

    map(0x000000, 0x006fff) internal RAM
    map(0x007000, 0x007fff) internal peripherals
    map(0x008000, 0x00ffff) internal ROM (lower 32kwords) - can also be configured to mirror CS0 308000 area with external pin for boot from external ROM
    map(0x010000, 0x027fff) internal ROM (upper 96kwords) - can't be switched
    map(0x028000, 0x02ffff) reserved

    map(0x030000, 0x0.....) view into external spaces (CS0 area starts here. followed by CS1 area, CS2 area etc.)

    map(0x200000, 0x3fffff) continued view into external spaces, but this area is banked with m_membankswitch_7810 (valid bank values 0x00-0x3f)
*/



#include "emu.h"
#include "generalplus_gpl16250.h"
#include "generalplus_gpl16250_nand.h"
#include "softlist_dev.h"

uint16_t generalplus_gpac800_game_state::cs0_r(offs_t offset)
{
	return m_sdram2[offset & 0xffff];
}

void generalplus_gpac800_game_state::cs0_w(offs_t offset, uint16_t data)
{
	m_sdram2[offset & 0xffff] = data;
}

uint16_t generalplus_gpac800_game_state::cs1_r(offs_t offset)
{
	return m_sdram[offset & (m_sdram_kwords-1)];
}

void generalplus_gpac800_game_state::cs1_w(offs_t offset, uint16_t data)
{
	m_sdram[offset & (m_sdram_kwords-1)] = data;
}

uint8_t generalplus_gpac800_game_state::read_nand(offs_t offset)
{
	if (!m_nandregion)
		return 0x0000;

	if (offset < m_size)
	{
		return m_nandregion[offset];
	}
	else
	{
		popmessage("read outside of NAND ROM space (offset %08x) (size %08x)\n", offset, m_size);
		return 0xff;
	}

	return 0x00;
}

void generalplus_gpac800_game_state::generalplus_gpac800(machine_config &config)
{
	GPAC800(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(generalplus_gpac800_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(generalplus_gpac800_game_state::portb_r));
	m_maincpu->portc_in().set(FUNC(generalplus_gpac800_game_state::portc_r));
	m_maincpu->porta_out().set(FUNC(generalplus_gpac800_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(generalplus_gpac800_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(generalplus_gpac800_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->set_bootmode(0); // boot from internal ROM (NAND bootstrap)
	m_maincpu->set_cs_config_callback(FUNC(gcm394_game_state::cs_callback));

	m_maincpu->nand_read_callback().set(FUNC(generalplus_gpac800_game_state::read_nand));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpac800_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "speaker", 2).front();
}

DEVICE_IMAGE_LOAD_MEMBER(generalplus_gpac800_vbaby_game_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void generalplus_gpac800_vbaby_game_state::generalplus_gpac800_vbaby(machine_config &config)
{
	generalplus_gpac800_game_state::generalplus_gpac800(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vbaby_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(generalplus_gpac800_vbaby_game_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("vbaby_cart");
}

static INPUT_PORTS_START( jak_car2 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) // unused
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_gtg )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END




static INPUT_PORTS_START( jak_hsm )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



ROM_START( wlsair60 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END

ROM_START( kiugames )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x21000000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "hy27084g2m.u2", 0x0000, 0x21000000, CRC(65cc3864) SHA1(b759ec9816fe98a33ee7d5e12e5492f0160c5b31) )
ROM_END


ROM_START( jak_gtg )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "goldentee.bin", 0x0000, 0x4200000, CRC(87d5e815) SHA1(5dc46cd753b791449cc41d5eff4928c0dcaf35c0) )
ROM_END

ROM_START( jak_car2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "cars2.bin", 0x0000, 0x4200000, CRC(4d610e09) SHA1(bc59f5f7f676a8f2a78dfda7fb62c804bbf850b6) )
ROM_END


/*  The following pinout was used when dumping jak_sspop, jak_hmhsm, jak_umdf
    For the 256Mbyte parts the parameters of the programmer had to be overridden to dump the full capacity as there were no equivalent parts.

       Sandisk TSOP32 NAND Flash

       +----------------------------------------------+
    NC-|01                                          32|-NC
   VSS-|02                                          31|-NC
   R/B-|03                  SanDisk                 30|-I/O7
    NC-|04                   NAND                   29|-I/O6
    RE-|05                                          28|-I/O5
    CE-|06                   32PIN                  27|-I/O4
    NC-|07                                          26|-VCC
   VCC-|08                                          25|-VSS
   VSS-|09                                          24|-NC
    NC-|10                                          23|-I/O3
    NC-|11                                          22|-I/O2
   CLE-|12                                          21|-I/O1
   ALE-|13                                          20|-I/O0
    WE-|14                                          19|-NC
    WP-|15                                          18|-NC
    NC-|16                                          17|-NC
       +----------------------------------------------+

One of the games has pin 2 grounded, and the other 2 have it N/C.  I'm not sure what it would be, since all the signals are accounted for.

*/

ROM_START( jak_sspop )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	/* TSOP32 NAND ROM

	S976172-1
	SanDisk
	11015-128B
	POC142
	0845
	<obscured #>

	appears to be a 128MByte part (or at least that is how much service mode tests)

	*/

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "singscenepop_as_hy27us081g1m_4579.bin", 0x0000, 0x8400000, CRC(4c8123fe) SHA1(388fda8ddd90b541a53eac4bcbe66bebe7360724) )
ROM_END

ROM_START( jak_hmhsm )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	/* TSOP32 NAND ROM

	5769522.1
	SanDisk
	11354-256B
	P44247.00
	xx20
	01xxxx_HSM

	256Mbyte part, 2nd half is just 0xff filled tho so a 128Mbyte part would have been fine

	*/

	ROM_REGION( 0x10800000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "hmhsm.bin", 0x0000, 0x10800000, CRC(e63ad24c) SHA1(a7844b14af701914150aa7c06743a410f478ff7b) )
ROM_END

ROM_START( jak_camp )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x10800000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "camprockguitar1_sandisk11352-256b_45da.bin", 0x0000, 0x10800000, CRC(f52a4289) SHA1(d027ae274cd4ac97924d2344df1a96456e8e7c55) )
ROM_END

ROM_START( jak_hmpt )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x10800000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "hmsecretstar_sandisk11270_9876.bin", 0x0000, 0x10800000, CRC(fbe09633) SHA1(169a1546072f53c2da19ce97396cacd25412c5f2) )
ROM_END

ROM_START( jak_hsmg2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "hsm_as_hy27ys08121a_9876.bin", 0x0000, 0x4200000, CRC(4da61056) SHA1(d6c529a6df2703dd55b864e9d7c655203206f8b6) )
ROM_END

ROM_START( jak_hmg2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "hm_as_hy27us08121a_9876_fixed.bin", 0x0000, 0x4200000, BAD_DUMP CRC(ba97fcd6) SHA1(c02a6878910b1312009b21220d51d7c1c3adb767) ) // 4 blocks had to be fixed using data from jak_hmhsm
ROM_END


ROM_START( jak_umdf )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	/* TSOP32 NAND ROM

	S744565-1
	SanDisk
	11352-256B
	PA2777.00
	0834
	61050

	again part number would suggest that this is a 256MByte ROM, although in reality all data fits into 64Mbyte, rest is blank

	*/

	ROM_REGION( 0x10800000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "jak_umdf.bin", 0x0000, 0x10800000, CRC(05f47aca) SHA1(61b417141ccc22324224b1862ea2f5778453f206) )
ROM_END


ROM_START( jak_tsm )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "toystorymania.bin", 0x0000, 0x4200000, CRC(183b20a5) SHA1(eb4fa5ee9dfac58f5244d00d4e833b1e461cc52c) )
ROM_END


ROM_START( jak_duck )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "duckcommander_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(d9356d5b) SHA1(aca05525b4a504f7ad264ae9bbc2f1f8f399c4ca) )
ROM_END

ROM_START( jak_swc )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "jakksstarwarspistol_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(024d49b8) SHA1(9694f4c7cd083c976ffbbcfa6f626fc6b4bc8d91) )
ROM_END

ROM_START( jak_wdzh )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "walkingdeadrifle_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(b2c762f0) SHA1(7e10df517cc24924e0ec55e2a263563023d945f8) )
ROM_END

ROM_START( jak_wdbg )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "amcwalkingdeadcrossbow_gpr27p512a_c276_as_hy27us08121a.bin", 0x0000, 0x4200000, CRC(66510fd4) SHA1(3ad6347c5a7758c035654cb3e96858320875b97a) )
ROM_END


ROM_START( vbaby )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "vbaby.bin", 0x0000, 0x8400000, CRC(d904441b) SHA1(3742bc4e1e403f061ce2813ecfafc6f30a44d287) )
ROM_END

ROM_START( mgtfit )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "nandrom", ROMREGION_ERASE00 ) // Samsung 937 K9F1G08U0D  Ident: 0xEC 0xF1 Full Ident: 0xECF1001540
	ROM_LOAD( "k9f1g08u0d.bin", 0x0000, 0x8400000, CRC(1ca5ac09) SHA1(c2e123085d2198999c2c0edb1df4895361c00a99) )
ROM_END

ROM_START( beambox )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "nandrom", ROMREGION_ERASE00 )
	ROM_LOAD( "beambox.bin", 0x0000, 0x4200000, CRC(a486f04e) SHA1(73c7d99d8922eba58d94e955e254b9c3baa4443e) )
ROM_END


void generalplus_gpac800_game_state::machine_start()
{
	save_item(NAME(m_sdram));
}

void generalplus_gpac800_game_state::nand_create_stripped_region()
{
	uint8_t* rom = m_nandregion;
	int size = memregion("nandrom")->bytes();
	m_size = size;

	int numblocks = size / m_nandblocksize;
	m_strippedsize = numblocks * m_nandblocksize_stripped;
	m_strippedrom.resize(m_strippedsize);

	for (int i = 0; i < numblocks; i++)
	{
		const int base = i * m_nandblocksize;
		const int basestripped = i * m_nandblocksize_stripped;

		for (int j = 0; j < m_nandblocksize_stripped; j++)
		{
			m_strippedrom[basestripped + j] = rom[(base + j)];
		}
	}

	// debug to allow for easy use of unidasm.exe
	if (0)
	{
		auto filename = "stripped_" + std::string(machine().system().name);
		auto fp = fopen(filename.c_str(), "w+b");
		if (fp)
		{
			fwrite(&m_strippedrom[0], m_nandblocksize_stripped * numblocks, 1, fp);
			fclose(fp);
		}
	}
}

void generalplus_gpac800_game_state::machine_reset()
{
	// configure CS defaults
	address_space& mem = m_maincpu->space(AS_PROGRAM);
	mem.write_word(0x007820, 0x0047);
	mem.write_word(0x007821, 0xff47);
	mem.write_word(0x007822, 0x00c7);
	mem.write_word(0x007823, 0x0047);
	mem.write_word(0x007824, 0x0047);

	m_maincpu->set_cs_space(m_memory->get_program());

	if (m_nandregion)
	{
		nand_create_stripped_region();

		// up to 256 pages (16384kw) for each space

		// (size of cs0 + cs1 + cs2 + cs3 + cs4) <= 81920kwords

		// simulate bootstrap / internal ROM

		address_space& mem = m_maincpu->space(AS_PROGRAM);

		/* Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
		   00000000 (50 47 61 6E 64 6E 61 6E 64 6E)-- -- -- -- -- --  PGandnandn------
		   00000010  -- -- -- -- -- bb -- -- -- -- -- -- -- -- -- --  ----------------

		   bb = where to copy first block

		   The header is GPnandnand (byteswapped) then some params
		   one of the params appears to be for the initial code copy operation done
		   by the bootstrap
		*/

		// probably more bytes are used
		int dest = m_strippedrom[0x15] << 8 | (m_strippedrom[0x16] << 16);

		// copy a block of code from the NAND to RAM
		for (int i = 0; i < m_initial_copy_words; i++)
		{
			uint16_t word = m_strippedrom[(i * 2) + 0] | (m_strippedrom[(i * 2) + 1] << 8);

			mem.write_word(dest + i, word);
		}

		/* these vectors must either directly point to RAM, or at least redirect there after some code

		   kiugames has the startup code copied to 20xxx which is outside the scope of a 16-bit vector
		   so these must trampoline (although 20xxx currently isn't handled as RAM, so that needs more
		   thought anyway
		*/
		uint16_t* internal = (uint16_t*)memregion("maincpu:internal")->base();

		int addr;
		addr = (m_vectorbase + 0x0a) & 0x000fffff;
		internal[0x7f00] = 0xfe80 | (addr >> 16);
		internal[0x7f01] = (addr & 0xffff);

		addr = (m_vectorbase + 0x0c) & 0x000fffff;
		internal[0x7f02] = 0xfe80 | (addr >> 16);
		internal[0x7f03] = (addr & 0xffff);

		addr = (dest + 0x20) & 0x000fffff; // point boot vector at code in RAM (probably in reality points to internal code that copies the first block)
		internal[0x7f04] = 0xfe80 | (addr >> 16);
		internal[0x7f05] = (addr & 0xffff);

		addr = (m_vectorbase + 0x10) & 0x000fffff;
		internal[0x7f06] = 0xfe80 | (addr >> 16);
		internal[0x7f07] = (addr & 0xffff);

		addr = (m_vectorbase + 0x12) & 0x000fffff;
		internal[0x7f08] = 0xfe80 | (addr >> 16);
		internal[0x7f09] = (addr & 0xffff);

		addr = (m_vectorbase + 0x14) & 0x000fffff;
		internal[0x7f0a] = 0xfe80 | (addr >> 16);
		internal[0x7f0b] = (addr & 0xffff);

		addr = (m_vectorbase + 0x16) & 0x000fffff;
		internal[0x7f0c] = 0xfe80 | (addr >> 16);
		internal[0x7f0d] = (addr & 0xffff);

		addr = (m_vectorbase + 0x18) & 0x000fffff;
		internal[0x7f0e] = 0xfe80 | (addr >> 16);
		internal[0x7f0f] = (addr & 0xffff);

		addr = (m_vectorbase + 0x1a) & 0x000fffff;
		internal[0x7f10] = 0xfe80 | (addr >> 16);
		internal[0x7f11] = (addr & 0xffff);

		addr = (m_vectorbase + 0x1c) & 0x000fffff;
		internal[0x7f12] = 0xfe80 | (addr >> 16);
		internal[0x7f13] = (addr & 0xffff);

		addr = (m_vectorbase + 0x1e) & 0x000fffff;
		internal[0x7f14] = 0xfe80 | (addr >> 16);
		internal[0x7f15] = (addr & 0xffff);


		internal[0x7ff5] = 0xff00;

		internal[0x7ff6] = 0xff02;
		internal[0x7ff7] = 0xff04;
		internal[0x7ff8] = 0xff06;
		internal[0x7ff9] = 0xff08;
		internal[0x7ffa] = 0xff0a;
		internal[0x7ffb] = 0xff0c;
		internal[0x7ffc] = 0xff0e;
		internal[0x7ffd] = 0xff10;
		internal[0x7ffe] = 0xff12;
		internal[0x7fff] = 0xff14;
	}

	m_maincpu->reset(); // reset CPU so vector gets read etc.

	//m_maincpu->set_paldisplaybank_high_hack(0);
	m_maincpu->set_alt_tile_addressing_hack(1);
}


void generalplus_gpac800_game_state::nand_init210()
{
	m_sdram.resize(m_sdram_kwords);
	m_sdram2.resize(0x10000);

	m_nandblocksize = 0x210;
	m_nandblocksize_stripped = 0x200;

	m_vectorbase = 0x6fe0;
}

void generalplus_gpac800_game_state::nand_init210_32mb()
{
	m_sdram_kwords = 0x400000 * 4;
	nand_init210();
}

void generalplus_gpac800_game_state::nand_init840()
{
	m_sdram.resize(m_sdram_kwords);
	m_sdram2.resize(0x10000);

	m_nandblocksize = 0x840;
	m_nandblocksize_stripped = 0x800;

	m_vectorbase = 0x6fe0;
}

void generalplus_gpac800_game_state::nand_wlsair60()
{
	nand_init840();
	m_initial_copy_words = 0x2800;
}

void generalplus_gpac800_game_state::nand_kiugames()
{
	nand_init840();
	m_initial_copy_words = 0x10000;
}


void generalplus_gpac800_game_state::nand_vbaby()
{
	nand_init840();
	m_initial_copy_words = 0x1000;
	m_maincpu->set_romtype(2);
}

void generalplus_gpac800_game_state::nand_tsm()
{

	// something odd must be going on with the bootloader?
	// structure has the first 0x4000 block repeated 3 times (must appear in RAM on startup?)
	// then it has a 0x10000 block repeated 4 times (must get copied to 0x30000 by code)
	// then it has the larger, main payload, just the once.

	// the addresses written to the NAND device don't compensate for these data repeats, however dump seems ok as no other data is being repeated?
	// reads after startup still need checking
	nand_init210();
	m_maincpu->set_romtype(1);
}

void generalplus_gpac800_game_state::nand_beambox()
{
	nand_init210();
	m_vectorbase = 0x2fe0;
}

// NAND dumps w/ internal bootstrap (and u'nSP 2.0 extended opcodes)  (have gpnandnand strings)
// the JAKKS ones seem to be known as 'Generalplus GPAC800' hardware
CONS(2010, wlsair60,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_wlsair60,      "Jungle Soft / Kids Station Toys Inc",      "Wireless Air 60",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // some of th games seem to be based on ones found in the 'Millennium Arcade' multigames (WinFun related) so might have the same external timer check
CONS(200?, beambox,    0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_beambox,       "Hasbro",                                   "Playskool Heroes Transformers Rescue Bots Beam Box (Spain)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, mgtfit,     0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_wlsair60,      "MGT",                                      "Fitness Konsole (NC1470)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // probably has other names in English too? menus don't appear to be in German
CONS(200?, vbaby,      0, 0, generalplus_gpac800_vbaby, jak_car2, generalplus_gpac800_vbaby_game_state, nand_vbaby,         "VTech",                                    "V.Baby", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, kiugames,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_kiugames,      "VideoJet",                                 "Kiu Games",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // probably has other names in English too? menus don't appear to be in German

CONS(2011, jak_gtg,    0, 0, generalplus_gpac800,       jak_gtg,  generalplus_gpac800_game_state,       nand_init210,       "JAKKS Pacific Inc / HotGen Ltd",           "Golden Tee Golf (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_car2,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210,       "JAKKS Pacific Inc / HotGen Ltd",           "Cars 2 (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(2010, jak_tsm,    0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_tsm,           "JAKKS Pacific Inc / Schell Games",         "Toy Story Mania (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(2009, jak_sspop,  0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "Sing Scene Pop (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(2008, jak_hmg2,   0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "Hannah Montana G2 Deluxe - All in One (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // Jul 9 2008 11:50:08
CONS(2008, jak_hsmg2,  0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "High School Musical G2 Deluxe - All in One (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // Jun 25 2008 14:53:14
CONS(2008, jak_hmhsm,  0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "Hannah Montana G2 Deluxe / High School Musical G2 Deluxe - Two in One (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // Sep 12 2008 18:48:14 (Menu/HM) / Sep 12 2008 18:50:45 (HSM)
CONS(2008, jak_umdf,   0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / Handheld Games",       "Ultimotion - Disney Fairies Sleeping Beauty & TinkerBell (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// Ultimotion Swing Zone is SPG29xx instead
CONS(2008, jak_camp,   0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "Camp Rock - Guitar Video Game (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// 2 blocks fail the hidden ROM test in jak_hmpt set below, however this seems to be an error in the test mode, not the dump
// a different set, https://www.youtube.com/watch?v=XiEMtLzcTFw showing a date of May 14 2008 10:05:22 shows exactly the same failures
CONS(2008, jak_hmpt,   0, 0, generalplus_gpac800,       jak_hsm,  generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / HotGen Ltd",           "Hannah Montana Pop Tour - Guitar Video Game (JAKKS Pacific TV Game) (May 16 2008)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // May 16 2008 10:36:59

// There were 1 player and 2 player versions for several of the JAKKS guns.  The 2nd gun appears to be simply a controller (no AV connectors) but as they were separate products with the 2 player versions being released up to a year after the original, the code could differ.
// If they differ, it is currently uncertain which versions these ROMs are from
CONS(2012, jak_wdzh,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210,       "JAKKS Pacific Inc / Merge Interactive",    "The Walking Dead: Zombie Hunter (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // gun games all had Atmel 16CM (24C16).
CONS(2013, jak_duck,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / Merge Interactive",    "Duck Commander (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // no 2 Player version was released
CONS(2013, jak_swc,    0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / Merge Interactive",    "Star Wars Clone Trooper (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(2014, jak_wdbg,   0, 0, generalplus_gpac800,       jak_car2, generalplus_gpac800_game_state,       nand_init210_32mb,  "JAKKS Pacific Inc / Super Happy Fun Fun",  "The Walking Dead: Battleground (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
