// license:BSD-3-Clause
// copyright-holders:David Haywood

// seems to be a GPCExxx series SunPlus CPU
// GPCE4064 series seems very likely for the Super Impulse titles
//
// GPCE4P064B is an OTP part with support for 30Kbyte OTP ROM (+2Kbyte 'test' area) which matches mapacman
// it also lacks Port C, which appears to be unused here

/*
   NOTES:

   Exact size of internal ROM is unknown, but it appears to occupy at least 0x4000 - 0xffff (word addresses)
   in unSP space and is different for each game.  Data is pulled for jump tables, where the addresses are in
   internal ROM and the code in external

   For correctly offset disassembly use
   unidasm xxx.bin -arch unsp12 -xchbytes -basepc 200000 >palace.txt

   Multiple different units appear to share the same ROM with a jumper to select game.
   It should be verified in each case that the external ROM was not changed

   It is confirmed that in some cases the external ROMs contain both versions for Micro Arcade and Tiny Arcade
   units, with different sound, but in some cases the expected game behavior differs too, so the code revisions
   could be different.  (for example the Micro Arcade Pac-Man doesn't display points when you eat a ghost)
*/


#include "emu.h"

#include "machine/generalplus_gpce4_soc.h"
#include "machine/generic_spi_flash.h"
#include "machine/timer.h"
#include "video/st7735_lcdc.h"

#include "screen.h"

#include "logmacro.h"


namespace {

class generalplus_gpce4_state : public driver_device
{
public:
	generalplus_gpce4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lcdc(*this, "lcdc"),
		m_genspi(*this, "spi")
	{ }

	void generalplus_gpce4(machine_config &config) ATTR_COLD;

	void init_siddr();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void spi_from_soc(u8 data) = 0;
	void spi_rom_access_from_soc(u8 data);
	void spi_reset(u8 data);

	void portb_from_soc(u16 data);
	void process_lcdc_command_params(u8 data);

	u16 m_iob;

	TIMER_DEVICE_CALLBACK_MEMBER(timer);

	required_device<generalplus_gpce4_soc_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<st7735_lcdc_device> m_lcdc;
	required_device<generic_spi_flash_device> m_genspi;
};

class generalplus_gpce4_mapacman_state : public generalplus_gpce4_state
{
public:
	generalplus_gpce4_mapacman_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpce4_state(mconfig, type, tag)
	{
	}

	virtual void spi_from_soc(u8 data) override;
};

class generalplus_gpce4_digicolr_state : public generalplus_gpce4_state
{
public:
	generalplus_gpce4_digicolr_state(const machine_config &mconfig, device_type type, const char *tag) :
		generalplus_gpce4_state(mconfig, type, tag)
	{
	}

	void digicolr(machine_config &config) ATTR_COLD;

	virtual void spi_from_soc(u8 data) override;
private:
	virtual void machine_start() override ATTR_COLD;

};

u32 generalplus_gpce4_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}

static INPUT_PORTS_START( generalplus_gpce4 )
	PORT_START("PORTA")
	PORT_DIPNAME( 0x0001, 0x0001, "PORTA: 0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "PORTA: 0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "PORTA: 0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "PORTA: 0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "PORTA: 0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "PORTA: 0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PORTA: 0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "PORTA: 0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "PORTA: 0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "PORTA: 0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "PORTA: 0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "PORTA: 0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "PORTA: 1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "PORTA: 2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "PORTA: 4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "PORTA: 8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("PORTB")
	PORT_DIPNAME( 0x0001, 0x0001, "PORTB: 0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "PORTB: 0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "PORTB: 0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "PORTB: 0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "PORTB: 0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "PORTB: 0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PORTB: 0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "PORTB: 0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "PORTB: 0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "PORTB: 0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "PORTB: 0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "PORTB: 0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "PORTB: 1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "PORTB: 2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "PORTB: 4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "PORTB: 8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("PORTC")
	PORT_DIPNAME( 0x0001, 0x0001, "PORTC: 0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "PORTC: 0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "PORTC: 0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "PORTC: 0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "PORTC: 0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "PORTC: 0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PORTC: 0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "PORTC: 0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "PORTC: 0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "PORTC: 0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "PORTC: 0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "PORTC: 0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "PORTC: 1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "PORTC: 2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "PORTC: 4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "PORTC: 8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mapacman )
	PORT_INCLUDE( generalplus_gpce4 )

	PORT_MODIFY("PORTA")
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT(0x0003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT(0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( digicolr )
	PORT_INCLUDE( generalplus_gpce4 )

	PORT_MODIFY("PORTA")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("PORTB")
	PORT_DIPNAME( 0x0004, 0x0004, "Battery Charging" )
	PORT_DIPSETTING(      0x0004, "Not Charging" )
	PORT_DIPSETTING(      0x0000, "Charging")
	PORT_DIPNAME( 0x0010, 0x0000, "Charger Connected" ) // if ADC is returning 0 and this is off you'll get the no battery state
	PORT_DIPSETTING(      0x0010, "Yes")
	PORT_DIPSETTING(      0x0000, "No" )
INPUT_PORTS_END

void generalplus_gpce4_state::machine_start()
{
	save_item(NAME(m_iob));

	m_genspi->set_rom_ptr(memregion("spi")->base());
	m_genspi->set_rom_size(memregion("spi")->bytes());

	m_genspi->set_single_byte_status_read();
}

void generalplus_gpce4_state::machine_reset()
{
	m_maincpu->set_spi_romregion(memregion("spi")->base(), memregion("spi")->bytes());
}

void generalplus_gpce4_digicolr_state::machine_start()
{
	generalplus_gpce4_state::machine_start();
	m_genspi->set_single_byte_status_writes();
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpce4_state::timer )
{
	m_maincpu->request_interrupt(24);
}

void generalplus_gpce4_state::portb_from_soc(u16 data)
{
	m_iob = data;
}

void generalplus_gpce4_mapacman_state::spi_from_soc(u8 data)
{
	if (!(m_iob & 0x0800))
	{
		m_lcdc->lcdc_command_w(data);
	}
	else
	{
		m_lcdc->lcdc_data_w(data);
	}
}

void generalplus_gpce4_digicolr_state::spi_from_soc(u8 data)
{
	if (!(m_iob & 0x0400))
	{
		m_lcdc->lcdc_command_w(data);
	}
	else
	{
		m_lcdc->lcdc_data_w(data);
	}
}

void generalplus_gpce4_state::spi_reset(u8 data)
{
	m_genspi->reset();
}

void generalplus_gpce4_state::spi_rom_access_from_soc(u8 data)
{
	logerror("get SPI byte from SoC %02x\n", data);
	m_genspi->write(data);
	m_maincpu->recieve_spi_fifo_data(m_genspi->read());
}

void generalplus_gpce4_state::generalplus_gpce4(machine_config &config)
{
	GPCE4(config, m_maincpu, 96000000); // internal ROM uses unsp2.0 opcodes, should be 48MHz (but unSP2.0 opcode take fewer cycles?)
	m_maincpu->porta_in().set_ioport("PORTA");
	m_maincpu->portb_in().set_ioport("PORTB");
	m_maincpu->portc_in().set_ioport("PORTC");
	m_maincpu->portb_out().set(FUNC(generalplus_gpce4_state::portb_from_soc));
	m_maincpu->spi2_out().set(FUNC(generalplus_gpce4_state::spi_from_soc));
	m_maincpu->spi_out().set(FUNC(generalplus_gpce4_state::spi_rom_access_from_soc));
	m_maincpu->spi_reset().set(FUNC(generalplus_gpce4_state::spi_reset));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(128, 128);
	m_screen->set_visarea(0, 128-1, 0, 128-1);
	m_screen->set_screen_update(FUNC(generalplus_gpce4_state::screen_update));

	// this triggers the SPI2 interrupt, causing pixels to be pushed to the display
	TIMER(config, "timer").configure_periodic(FUNC(generalplus_gpce4_state::timer), attotime::from_hz(300000));

	GENERIC_SPI_FLASH(config, m_genspi, 0);

	ST7735(config, m_lcdc, 0);
}

void generalplus_gpce4_digicolr_state::digicolr(machine_config &config)
{
	generalplus_gpce4(config);
	m_screen->set_visarea(0, 96-1, 18, 114-1);
}

void generalplus_gpce4_state::init_siddr()
{
	u8* spirom8 = (u8*)memregion("spi")->base();
	for (int i = 0x3000; i < 0x400000; i++)
	{
		spirom8[i] = bitswap<8>(spirom8[i] ^ 0x68,
			3, 5, 0, 7,    1, 4, 2, 6);
	}
}


ROM_START( mapacman ) // this is the single game (no games hidden behind solder pads) release
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mapacman_internal.rom", 0x000000, 0x10000, CRC(9ea69d2a) SHA1(17f5001794f4454bf5856cfa170834509d68bed0) )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END

ROM_START( taspinv )
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcade_spaceinvaders.bin", 0x000000, 0x200000, CRC(11ac4c77) SHA1(398d5eff83a4e94487ed810819085a0e44582908) )
ROM_END

ROM_START( tagalaga )
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadegalaga_fm25q16a_a14015.bin", 0x000000, 0x200000, CRC(2a91460c) SHA1(ce297642d2d51ce568e93c0c57432446633b2077) )
ROM_END

ROM_START( parcade )
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "palacearcade_gpr25l3203_c22016.bin", 0x000000, 0x400000, CRC(98fbd2a1) SHA1(ffc19aadd53ead1f9f3472475606941055ca09f9) )
ROM_END

ROM_START( taturtf )
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x000000, 0x18000, NO_DUMP )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tinyarcadeturtlefighter_25q32bst16_684016.bin", 0x000000, 0x400000, CRC(8e046f2d) SHA1(e48492cf953f22a47fa2b88a8f96a1e459b8c487) )
ROM_END

/*

(this is correct for at least some of the digimon units)
MCU : Generalplus GPCE4064B
SPI Flash IC : Macronix MX25L6433F
LCD Driver : Sitronix ST7735V
LCD module : TCXD011IBLON-3
Sound unit is a piezo disc.

*/

ROM_START( digicolr )
	ROM_REGION16_BE( 0x18000, "maincpu:internal", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "internal_rom_221017a.bin", 0x000000, 0x10000, CRC(10591b51) SHA1(78c11913e8ebd89a64b575d85bfe9b37aafb3bc0) )

	ROM_REGION16_BE( 0x800000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "dmc_v1_base.bin", 0x000000, 0x800000, CRC(23227806) SHA1(ebae139f708d36151efcaf2db7c329c6614d7e68) )
ROM_END

ROM_START( siddr )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION16_BE(0x800000, "spi", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP( "ddr-toy.bin", 0x0000, 0x400000, CRC(873cbcc8) SHA1(bdd3d12adb1284991a3f8aaa8e451e3a55931267) )
ROM_END


} // anonymous namespace


// The 'Micro Arcade' units are credit card sized handheld devices
CONS( 2017, mapacman,      0,       0,      generalplus_gpce4,   mapacman,          generalplus_gpce4_mapacman_state, empty_init, "Super Impulse", "Pac-Man (Micro Arcade)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// The 'Tiny Arcade' units are arcade cabinets on a keyring.
CONS( 2017, taspinv,       0,       0,      generalplus_gpce4,   generalplus_gpce4, generalplus_gpce4_mapacman_state, empty_init, "Super Impulse", "Space Invaders (Tiny Arcade)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2017, tagalaga,      0,       0,      generalplus_gpce4,   generalplus_gpce4, generalplus_gpce4_mapacman_state, empty_init, "Super Impulse", "Galaga (Tiny Arcade)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2017, parcade,       0,       0,      generalplus_gpce4,   generalplus_gpce4, generalplus_gpce4_mapacman_state, empty_init, "Hasbro", "Palace Arcade (Tiny Arcade)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2019, taturtf,       0,       0,      generalplus_gpce4,   generalplus_gpce4, generalplus_gpce4_mapacman_state, empty_init, "Super Impulse", "Teenage Mutant Ninja Turtles - Turtle Fighter (Tiny Arcade)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2022, digicolr,   0,         0,      digicolr,            digicolr,          generalplus_gpce4_digicolr_state, empty_init, "Bandai", "Digimon Color", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// Probably not identical hardware, but still not direct mapped SPI.  External ROM after 0x3000 is encrypted (maybe decrypted in software) seems to have jumps to internal ROM
CONS( 2021, siddr,         0,       0,      generalplus_gpce4,   generalplus_gpce4, generalplus_gpce4_mapacman_state, init_siddr, "Super Impulse", "Dance Dance Revolution - Broadwalk Arcade", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

