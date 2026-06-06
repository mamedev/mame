// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "unknown_bftetris_lcdc.h"

#include "machine/generalplus_gpl951xx_soc.h"
#include "machine/generic_spi_flash.h"

#include "screen.h"
#include "speaker.h"


namespace {

class generalplus_gpl951xx_game_state : public driver_device
{
public:
	generalplus_gpl951xx_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_genspi(*this, "spi"),
		m_io(*this, "IN%u", 0U),
		m_adc(*this, "ADC%u", 0U),
		m_lcdc(*this, "lcdc")
	{
	}

	void bfpacman(machine_config &config) ATTR_COLD;
	void fixitflx(machine_config &config) ATTR_COLD;
	void wiwcs(machine_config &config) ATTR_COLD;
	void poke(machine_config &config) ATTR_COLD;
	void flufflav(machine_config &config) ATTR_COLD;
	void puni(machine_config &config) ATTR_COLD;
	void bubltea(machine_config &config) ATTR_COLD;
	void dsgnpal(machine_config &config) ATTR_COLD;
	void bftetris(machine_config &config) ATTR_COLD;

	void init_fif() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void gpl951xx(machine_config &config) ATTR_COLD;

	u32 bftetris_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template <u8 Port> u16 port_r();
	template <u8 Port> u16 adc_r();

	void porta_w(u16 data);

	void spi_reset(u8 data);
	void spi_access_from_soc(u8 data);
	void spi_cmd_access_from_soc(u8 data);

	void lcd_i80_cmd(u16 data);
	void lcd_i80_data(u16 data);

	required_device<generalplus_gpl951xx_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<generic_spi_flash_device> m_genspi;
	required_ioport_array<6> m_io;
	required_ioport_array<6> m_adc;
	required_device<bftetris_lcdc_device> m_lcdc;
};

u32 generalplus_gpl951xx_game_state::bftetris_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}

template <u8 Port>
u16 generalplus_gpl951xx_game_state::port_r()
{
	u16 data = m_io[Port]->read();
	logerror("Port %c Read: %04x\n", 'A'+Port, data);
	return data;
}

template <u8 Port>
u16 generalplus_gpl951xx_game_state::adc_r()
{
	u16 data = m_adc[Port]->read();
	return data;
}

void generalplus_gpl951xx_game_state::porta_w(u16 data)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);
}

void generalplus_gpl951xx_game_state::machine_start()
{
	m_genspi->set_rom_ptr(memregion("spi")->base());
	m_genspi->set_rom_size(memregion("spi")->bytes());
	m_maincpu->set_spi_romregion(memregion("spi")->base(), memregion("spi")->bytes());
}

void generalplus_gpl951xx_game_state::machine_reset()
{
	m_maincpu->reset(); // reset CPU so vector gets read etc.
}

// default port structure for debugging inputs
#define DEBUG_PORT(_name) \
	PORT_START(#_name) \
	PORT_DIPNAME( 0x0001, 0x0001, #_name ":0001" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0002, 0x0002, #_name ":0002" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0004, 0x0004, #_name ":0004" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0008, 0x0008, #_name ":0008" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0010, 0x0010, #_name ":0010" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0020, 0x0020, #_name ":0020" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0040, 0x0040, #_name ":0040" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0080, 0x0080, #_name ":0080" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0100, 0x0100, #_name ":0100" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0200, 0x0200, #_name ":0200" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0400, 0x0400, #_name ":0400" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x0800, 0x0800, #_name ":0800" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x1000, 0x1000, #_name ":1000" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x2000, 0x2000, #_name ":2000" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x4000, 0x4000, #_name ":4000" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) ) \
	PORT_DIPNAME( 0x8000, 0x8000, #_name ":8000" ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

static INPUT_PORTS_START( base )
	DEBUG_PORT(IN0)
	DEBUG_PORT(IN1)
	DEBUG_PORT(IN2)
	DEBUG_PORT(IN3)
	DEBUG_PORT(IN4)
	DEBUG_PORT(IN5)

	PORT_START("ADC0")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_START("ADC1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_START("ADC2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_START("ADC3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_START("ADC4")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_START("ADC5")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( bfmpac )
	PORT_INCLUDE(base)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x0001, 0x0000, "0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Demo Play" ) // the units have a strip of paper that you must rip out to disable display / demo mode, this is probably related to that, have to press start to make it run a demo cycle?
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bfspyhnt )
	PORT_INCLUDE( bfmpac )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( puni )
	PORT_INCLUDE(base)

	PORT_MODIFY("IN5")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END

static INPUT_PORTS_START( segapet1 )
	PORT_INCLUDE(base)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) // hold this and 'button 3' on startup for a test mode
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_MODIFY("ADC5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( segapet2 )
	PORT_INCLUDE(base)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("<")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("OK")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME(">")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Cancel")

	PORT_MODIFY("ADC5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( bubltea ) // has 3 surface buttons and the straw
	PORT_INCLUDE(base)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Power / Next")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Increase Value")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Back")

	PORT_MODIFY("ADC5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dsgnpal )
	PORT_INCLUDE(base)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_MODIFY("ADC5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void generalplus_gpl951xx_game_state::spi_reset(u8 data)
{
	m_genspi->reset();
}

void generalplus_gpl951xx_game_state::spi_access_from_soc(u8 data)
{
	logerror("spi_access_from_soc %02x\n", data);
	m_genspi->write(data);
	m_maincpu->recieve_spi_fifo_data(m_genspi->read());
}

void generalplus_gpl951xx_game_state::spi_cmd_access_from_soc(u8 data)
{
	logerror("spi_cmd_access_from_soc %02x\n", data);
	m_genspi->write(data);
}


void generalplus_gpl951xx_game_state::lcd_i80_cmd(u16 data)
{
	m_lcdc->lcdc_command_w(data);
}

void generalplus_gpl951xx_game_state::lcd_i80_data(u16 data)
{
	m_lcdc->lcdc_data_w(data);
}



void generalplus_gpl951xx_game_state::gpl951xx(machine_config &config)
{
	GPL951XX(config, m_maincpu, 96000000/2, m_screen);
	m_maincpu->porta_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<0>));
	m_maincpu->portb_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<1>));
	m_maincpu->portc_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<2>));
	m_maincpu->portd_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<3>));
	m_maincpu->porte_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<4>));
	m_maincpu->portf_in().set(FUNC(generalplus_gpl951xx_game_state::port_r<5>));
	m_maincpu->porta_out().set(FUNC(generalplus_gpl951xx_game_state::porta_w));
	m_maincpu->adc0_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<0>));
	m_maincpu->adc1_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<1>));
	m_maincpu->adc2_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<2>));
	m_maincpu->adc3_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<3>));
	m_maincpu->adc4_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<4>));
	m_maincpu->adc5_in().set(FUNC(generalplus_gpl951xx_game_state::adc_r<5>));

	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(generalplus_gpl951xx_device::irq_vector_cb));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->spi_out().set(FUNC(generalplus_gpl951xx_game_state::spi_access_from_soc));
	m_maincpu->spi_out_cmd().set(FUNC(generalplus_gpl951xx_game_state::spi_cmd_access_from_soc));
	m_maincpu->spi_reset().set(FUNC(generalplus_gpl951xx_game_state::spi_reset));
	m_maincpu->i80_cmd_out().set(FUNC(generalplus_gpl951xx_game_state::lcd_i80_cmd));
	m_maincpu->i80_data_out().set(FUNC(generalplus_gpl951xx_game_state::lcd_i80_data));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(30); // is this coming from the LCDC? 60 is definitely too fast for the IRQ rate
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(generalplus_gpl951xx_device::screen_update));

	m_screen->screen_vblank().set(m_maincpu, FUNC(generalplus_gpl951xx_device::vblank));

	UNKNOWN_BFTETRIS_LCDC(config, m_lcdc);

	GENERIC_SPI_FLASH(config, m_genspi);

	SPEAKER(config, "speaker", 2).front();
}

void generalplus_gpl951xx_game_state::bfpacman(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc8);
	m_genspi->set_jedec_memtype(0x40);
	m_genspi->set_jedec_capacity(0x14);
}

void generalplus_gpl951xx_game_state::fixitflx(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc8);
	m_genspi->set_jedec_memtype(0x40);
	m_genspi->set_jedec_capacity(0x15);
}

void generalplus_gpl951xx_game_state::wiwcs(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc8);
	m_genspi->set_jedec_memtype(0x40);
	m_genspi->set_jedec_capacity(0x17);
}

void generalplus_gpl951xx_game_state::poke(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc2);
	m_genspi->set_jedec_memtype(0x20);
	m_genspi->set_jedec_capacity(0x19);
}

void generalplus_gpl951xx_game_state::flufflav(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc2);
	m_genspi->set_jedec_memtype(0x20);
	m_genspi->set_jedec_capacity(0x16);

	m_screen->set_visarea(0, 128-1, 0, 128-1);
	m_screen->set_physical_aspect(1, 1);
}

void generalplus_gpl951xx_game_state::dsgnpal(machine_config &config)
{
	gpl951xx(config);
	m_genspi->set_jedec_manufacturer(0xc2);
	m_genspi->set_jedec_memtype(0x20);
	m_genspi->set_jedec_capacity(0x17);
}

void generalplus_gpl951xx_game_state::puni(machine_config &config)
{
	dsgnpal(config);
	m_screen->set_visarea(0, 128-1, 0, 128-1);
	m_screen->set_physical_aspect(1, 1);
}

void generalplus_gpl951xx_game_state::bubltea(machine_config &config)
{
	dsgnpal(config);
	m_screen->set_visarea(0, 128-1, 0, 160-1);
	m_screen->set_physical_aspect(128, 160);
}


void generalplus_gpl951xx_game_state::bftetris(machine_config &config)
{
	fixitflx(config);

	// all games have an LCDC, but bftetris programs it manually, while other games
	// seem to take the GPL95xx output and show that directly
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gpl951xx_game_state::bftetris_screen_update));
}



// There should be a small internal ROM (0x4000) bytes that does some basic setup

ROM_START( fixitflx )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x200000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "fixitfelix_md25q16csig_c84015.bin", 0x0000, 0x200000, CRC(605c6863) SHA1(4f6cc2e8388e20eb90c6b05265273650eeea56eb) )
ROM_END

ROM_START( wiwcs )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "wwcs.img", 0x0000, 0x800000, CRC(9b86bc45) SHA1(17721c662642a257d3e0f56e351a9a80d75d9110) )

	ROM_REGION16_BE(0x400, "i2c", ROMREGION_ERASE00)
	ROM_LOAD( "witw_eeprom.bin", 0x0000, 0x400, CRC(9426350b) SHA1(c481ded8b1f61fbf2403532dabb9c0a5c2a33fa2) )
ROM_END

ROM_START( bfpacman )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x100000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "basicfunpacman_25q80_c84014.bin", 0x0000, 0x100000, CRC(dd39fc64) SHA1(48c0e1eb729f61b7359e1fd52b7faab56817dfe8) )
ROM_END

ROM_START( bfmpac )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x100000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mspacman_25q80_c84014.bin", 0x0000, 0x100000, CRC(c0c3f8ce) SHA1(30da9b14f1a2c966167c97da9b8329f2f7f73291) )
ROM_END

ROM_START( bfdigdug )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x100000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsdigdug_25q80csig_c84014.bin", 0x0000, 0x100000, CRC(4030bc46) SHA1(8c086c96b9822e95c1862012786d6d6e59e0387e) )
ROM_END

ROM_START( bfgalaga )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x100000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsgalaga_25q80csig_c84014.bin", 0x0000, 0x100000, CRC(69982c9d) SHA1(0f8f403fefa7d8a9fdfcc04dca5a67919b662c7e) )
ROM_END

ROM_START( bfspyhnt )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x200000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicsspyhunter_md25q16csig_c84015.bin", 0x0000, 0x200000, CRC(1f1eaabd) SHA1(1c484e0b0749123cfa1ac6d1959aefa6ed09ab20) )

	// also has a 24C04 (to store high scores?)
ROM_END

ROM_START( bftetris )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x200000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "arcadeclassicstetris_25q16ct_c84015.bin", 0x0000, 0x200000, CRC(a97e1bab) SHA1(400944d310d5d5fccb2c6d048d7bf0cb00da09de) )
ROM_END



ROM_START( punirune )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25l64.ic103", 0x0000, 0x800000, CRC(97ce057d) SHA1(6244afd57990b178b4c404d6a08735b6db5348f8) )
ROM_END

ROM_START( punirunea )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25l64.ic3", 0x0000, 0x800000, CRC(a7a9aad0) SHA1(2c81d9a831360882b79b4dab0966c828ddb5a9e3) )
ROM_END

ROM_START( punij1m )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v1pcb_mint_pink_25l6433f.ic103", 0x0000, 0x800000, CRC(b83c43f3) SHA1(f811d7ad6d28efe4f897155ca20da04168cdb975) )
ROM_END

ROM_START( punij1pu ) // different software revision to punij1m but same case style
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v1pcb_purple_25l6433f.ic103", 0x0000, 0x800000, CRC(ac6dd1f2) SHA1(00e65f272e45cc7f01f69a5d971728a9a7fdf17a) )
ROM_END

ROM_START( punij2pk )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "japan_v2pcb_pink_gpr25l64.ic103", 0x0000, 0x800000, CRC(7ae9f009) SHA1(d762634a0442ff231837f9481a1203933c070df0) )
ROM_END

ROM_START( punifrnd )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25oh64.ic3", 0x0000, 0x800000, CRC(49e14af2) SHA1(477d1335587894793bac913c877d82dba46884d3) )
ROM_END

ROM_START( punistar )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "xm25qh64c.ic3", 0x0000, 0x800000, CRC(72f54f23) SHA1(902955764d0b61decc057eb3afaf2960cf2134c6) )
ROM_END

ROM_START( flufflav )
	ROM_REGION16_BE(0x400000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "gpr25l320.u2", 0x0000, 0x400000, CRC(67bcd4cb) SHA1(5ffb140bf8e4608b5420a649aede3946923f6dac) )
ROM_END

ROM_START( pockrmsr )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "25l6433f.ic102", 0x0000, 0x800000, CRC(4b899ca1) SHA1(5e57aaa0598563c08f3b3e4f26447a3902ca51dc) )
ROM_END

ROM_START( pokgoget )
	ROM_REGION16_BE(0x2000000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25645g.u1", 0x0000, 0x2000000, CRC(a76ae22f) SHA1(3fa5eeedb3fe343a7707d76710298377b22b0681) )
ROM_END

ROM_START( pokebala )
	ROM_REGION16_BE(0x2000000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25645g.u4", 0x0000, 0x2000000, CRC(e35d434a) SHA1(74061831b25476ec8aa7dec5f9d64ff79b0db88e) )
ROM_END

ROM_START( pokeissh )
	ROM_REGION16_BE(0x2000000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25645g.u1", 0x0000, 0x2000000, CRC(1eaf3457) SHA1(a7f16ad7abfc13c67d8e50f462882a771b6777ab) )
ROM_END

ROM_START( pokemech )
	ROM_REGION16_BE(0x2000000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "mx25l25645g.u1", 0x0000, 0x2000000, CRC(e170dede) SHA1(4b07cfcc92e6af412ad0e5c9852b7075a15bd75c) )
ROM_END

ROM_START( smkcatch )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.u2", 0x0000, 0x800000,  CRC(e2f52c4a) SHA1(f79862d27152cff8f96151c672d9762a3897a593) )
ROM_END

ROM_START( smkguras )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.u3", 0x0000, 0x800000, CRC(96b61464) SHA1(59fada02e1ea983c56304b27b3f4cb1fba221cb0) )
ROM_END

ROM_START( smkgurasa ) // code is the same, some data area differs, could be different factory defaults, or user data, remove later if redundant
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "xm25qh64c.u3", 0x0000, 0x800000, CRC(2c9e82af) SHA1(8c475783eafbeaeb88d62b145758ad3487410222) )
ROM_END

ROM_START( smkgacha )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "kg25l64.u2", 0x0000, 0x800000, CRC(eb461d92) SHA1(7481e8b3f2eaa9c8d992f3d9c58c1c7e3f27b380) )
ROM_END

ROM_START( dsgnpal )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.ic2", 0x0000, 0x800000, CRC(a1017ea8) SHA1(bd4b553ff71e763cd3fd726c49f5408eac3b7984) )
ROM_END

ROM_START( segapet1 )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "pink_gpr25l64.ic3", 0x0000, 0x800000, CRC(3bb709d1) SHA1(8a1b34d6cdd856685182d19b86c5cb68a006f816) )
ROM_END

ROM_START( segapet1a )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "yellow_gpr25l64.ic3", 0x0000, 0x800000, CRC(073218b8) SHA1(8501e69dc2b2cda9e4c289a14d6a16fe832e722c) )
ROM_END

ROM_START( segapet2 )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "purple_gpr25l64.ic3", 0x0000, 0x800000, CRC(a5f2cd07) SHA1(ba510b2afce3826f5153f6a606bf9d2d3b1c4399) )
ROM_END

ROM_START( segapet2a )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "orange_gpr25l64.ic3", 0x0000, 0x800000, CRC(3776c81a) SHA1(6ba67098a38e4efe7bab63ff052c98a00e1cdabd) )
ROM_END

ROM_START( segapet3 )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "pink_gpr25l64.ic3", 0x0000, 0x800000, CRC(4b60f556) SHA1(3487c7b42e1b818ac1fe3a9429320519574ca4ac) )
ROM_END

ROM_START( segapet3a )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "blue_gpr25l64.ic3", 0x0000, 0x800000, CRC(02a63c63) SHA1(54d8c1c52a30d7b2a21f69439d9ff2ef7b2a606b) )
ROM_END

ROM_START( segaptdx )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l640.ic3", 0x0000, 0x800000, CRC(971837c5) SHA1(b0e639fbd1e2b41934a98fde8425ded84f1a8159) )
ROM_END

ROM_START( bubltea )
	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "gpr25l64.ic2", 0x0000, 0x800000, CRC(56549fa7) SHA1(4a03b4c69035baa48b146ecee3912a3b0672b845) )
ROM_END

void generalplus_gpl951xx_game_state::init_fif()
{
	u16 *spirom16 = (u16*)memregion("spi")->base();
	for (int i = 0; i < memregion("spi")->bytes() / 2; i++)
	{
		spirom16[i] = bitswap<16>(spirom16[i] ^ 0xdd0d,
			3, 1, 11, 9, 6, 14, 0, 2, 8, 7, 13, 15, 4, 5, 12, 10);
	}
}

} // anonymous namespace

CONS(2017, fixitflx, 0, 0, fixitflx, bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Fix It Felix Jr. (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, wiwcs,    0, 0, wiwcs,    bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Where in the World Is Carmen Sandiego? (handheld)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, bfpacman, 0, 0, bfpacman, bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Pac-Man (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2017, bfmpac,   0, 0, bfpacman, bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Ms. Pac-Man (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2017, bfgalaga, 0, 0, bfpacman, bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Galaga (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2018, bfdigdug, 0, 0, bfpacman, bfmpac,   generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Dig Dug (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2019, bfspyhnt, 0, 0, fixitflx, bfspyhnt, generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Spy Hunter (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2020, bftetris, 0, 0, bftetris, bfspyhnt, generalplus_gpl951xx_game_state, init_fif, "Basic Fun", "Tetris (mini arcade)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// games below use GPL95101 series chips, which might be different but are definitely unSP2.0 chips that run from SPI directly

// unclear if colour matches, but there are multiple generations of these at least
// uses PUNIRUNZU_MAIN_V3 pcb
CONS(2021, punirune,  0,        0, puni, puni, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_V3, pastel blue, Europe)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// looks similar to above, but has HXR-1 instead of the usual markings on the PCB
CONS(2021, punirunea, punirune, 0, puni, bfspyhnt, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes (HXR-1 PCB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// the case on these looks like the European release, including English title logo.  CPU is a glob, PUNIRUNZU_MAIN_DICE_V1 on PCB
CONS(2021, punij1m,  punirune, 0, puni, puni, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_DICE_V1, mint/pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, punij1pu, punirune, 0, puni, puni, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_DICE_V1, purple, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// the case on these is similar to the above, but the text is in Japanese, uses PUNIRUNZU_MAIN_V2 on pcb
CONS(2021, punij2pk, punirune, 0, puni, puni, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes (PUNIRUNZU_MAIN_V2, pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// has a link feature
CONS(2021, punifrnd, 0,        0, puni, puni, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes Punitomo Tsuushin (hot pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

CONS(2021, punistar, 0,        0, puni, base, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Punirunes Punistarz (pink, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// 'Poo' emoji shaped item, comes in multiple colours, has a solder pad which might change between units
// this was dumped from the 'Lavender' unit
CONS(2021, flufflav, 0,        0, flufflav, bubltea, generalplus_gpl951xx_game_state, empty_init, "Happinet", "Fuwatcho Uncho Fuwa Fuwa (lavender, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// available in 2 colours, ROM confirmed to be the same on both
CONS(2021, pockrmsr,  0,        0, puni, base, generalplus_gpl951xx_game_state, empty_init, "Bandai", "Pocket Room - Sanrio Characters (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// Pocket Monsters ガチッとゲットだぜ! モンスターボールゴー! - Pocket Monsters is printed on the inner shell, but not the box?
CONS(2021, pokgoget, 0,        0, poke, bfspyhnt, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Gachitto Get da ze! Monster Ball Go! (210406, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// ガチッとゲットだぜ! モンスターボール
CONS(2021, pokebala, 0,        0, poke, bfspyhnt, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Gachitto Get da ze! Monster Ball (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// ポケモンといっしょ！モンスターボール
CONS(2021, pokeissh, 0,        0, poke, bfspyhnt, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Pokemon to Issho! Monster Ball (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// めちゃナゲ! モンスターボール
CONS(2021, pokemech, 0,        0, poke, bfspyhnt, generalplus_gpl951xx_game_state, empty_init, "Takara Tomy", "Mecha Nage! Monster Ball (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// 2020 (device) / 2021 (box) version of Sumikko Gurashi a cloud shaped device
// Sumikko Gurashi - Sumikko Catch (すみっコぐらし すみっコキャッチ)
CONS( 2021, smkcatch, 0, 0, puni, bfmpac, generalplus_gpl951xx_game_state, empty_init,  "San-X / Tomy", "Sumikko Gurashi - Sumikko Catch (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// or Sumikko Gurashi - Sumikko Catch DX (すみっコぐらし すみっコキャッチDX) = Sumikko Catch with pouch and strap

// is there a subtitle for this one? it's different to the others
CONS( 201?, smkguras,  0,        0, puni, bubltea, generalplus_gpl951xx_game_state, empty_init,  "San-X / Tomy", "Sumikko Gurashi DX (Japan, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS( 201?, smkgurasa, smkguras, 0, puni, bubltea, generalplus_gpl951xx_game_state, empty_init,  "San-X / Tomy", "Sumikko Gurashi DX (Japan, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

CONS( 2021, smkgacha,  0,        0, puni, bubltea, generalplus_gpl951xx_game_state, empty_init,  "San-X / Tomy", "Sumikko Gacha (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// there seem to be different versions of this available, is the software the same?
CONS( 201?, dsgnpal, 0, 0, dsgnpal, dsgnpal, generalplus_gpl951xx_game_state, empty_init,  "Tomy", "Kiratto Pri-Chan Design Palette (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | ROT270 )

// these have different version numbers (coming from the user/config area?) correlating to the colour of the device, even if the code the same
CONS( 2018, segapet1,  0,        0, puni, segapet1, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchiri Pet Mocchimaruzu (180615B P)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS( 2018, segapet1a, segapet1, 0, puni, segapet1, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchiri Pet Mocchimaruzu (180615B Y)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// DX version of the first game, 2020 on box, updated fabric cover, still has 2018 on case, newer code revision
CONS( 2020, segaptdx,  0,        0, puni, segapet1, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchiri Pet Mocchimaruzu DX (190313A P)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// also もっちりペット もっちまるず
CONS( 2019, segapet2,  0,        0, puni, segapet2, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchiri Pet Mocchimaruzu (2019 version, purple)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS( 2019, segapet2a, segapet2, 0, puni, segapet2, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchiri Pet Mocchimaruzu (2019 version, orange)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// these ones have motors in the ears and a more fluffy cover
// もっちふわペット もっちまるず
CONS( 2020, segapet3,  0,        0, puni, base, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchifuwa Pet Mocchimaruzu (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS( 2020, segapet3a, segapet3, 0, puni, base, generalplus_gpl951xx_game_state, empty_init, "Sega Toys", "Mocchifuwa Pet Mocchimaruzu (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)


// まぜまぜミックス！ぷにタピちゃん
CONS( 201?, bubltea,   0,        0, bubltea, bubltea, generalplus_gpl951xx_game_state, empty_init, "Bandai", "Mazemaze Mix! Puni Tapi-chan (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
