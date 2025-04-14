// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************

 Eolith 16 bits hardware

 Supported Games:
 - KlonDike+      (c) 1999 Eolith

 driver by Pierpaolo Prazzoli

**********************************************************************/

#include "emu.h"
#include "eolith_speedup.h"

#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class eolith16_state : public eolith_e1_speedup_state_base
{
public:
	eolith16_state(const machine_config &mconfig, device_type type, const char *tag)
		: eolith_e1_speedup_state_base(mconfig, type, tag)
		, m_special_io(*this, "SPECIAL")
		, m_eepromoutport(*this, "EEPROMOUT")
		, m_vram(*this, "vram", 0x20000, ENDIANNESS_BIG)
		, m_vrambank(*this, "vrambank")
	{
	}

	void eolith16(machine_config &config) ATTR_COLD;

	void init_eolith16() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_ioport m_special_io;
	required_ioport m_eepromoutport;
	memory_share_creator<uint8_t> m_vram;
	required_memory_bank m_vrambank;

	void eeprom_w(uint16_t data);
	uint16_t eolith16_custom_r();

	void eolith16_palette(palette_device &palette) const ATTR_COLD;

	uint32_t screen_update_eolith16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void eolith16_map(address_map &map) ATTR_COLD;
};



void eolith16_state::eeprom_w(uint16_t data)
{
	m_vrambank->set_entry(((data & 0x80) >> 7) ^ 1);
	machine().bookkeeping().coin_counter_w(0, data & 1);

	m_eepromoutport->write(data, 0xff);

	//data & 0x100 and data & 0x004 always set
}

uint16_t eolith16_state::eolith16_custom_r()
{
	speedup_read();
	return m_special_io->read();
}

void eolith16_state::eolith16_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0x50000000, 0x5000ffff).bankrw("vrambank");
	map(0x90000000, 0x9000002f).nopw(); //?
	map(0xff000000, 0xff1fffff).rom().region("maindata", 0);
	map(0xffe40001, 0xffe40001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xffe80000, 0xffe80001).w(FUNC(eolith16_state::eeprom_w));
	map(0xffea0000, 0xffea0001).r(FUNC(eolith16_state::eolith16_custom_r));
	map(0xffea0002, 0xffea0003).portr("SYSTEM");
	map(0xffec0000, 0xffec0001).nopr(); // not used?
	map(0xffec0002, 0xffec0003).portr("INPUTS");
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( eolith16 )
	PORT_START("SPECIAL")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(eolith16_state::speedup_vblank_r))
	PORT_BIT( 0xff6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
INPUT_PORTS_END

void eolith16_state::video_start()
{
	eolith_e1_speedup_state_base::video_start();

	m_vrambank->configure_entries(0, 2, memshare("vram")->ptr(), 0x10000);
	m_vrambank->set_entry(0);
}

uint32_t eolith16_state::screen_update_eolith16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= std::min(cliprect.bottom(), 203); y++)
	{
		auto *pix = &bitmap.pix(y);
		for (int x = 0; x < 320; x++)
			*pix++ = m_vram[(y * 320) + x] & 0xff;
	}
	return 0;
}


// setup a custom palette because pixels use 8 bits per color
void eolith16_state::eolith16_palette(palette_device &palette) const
{
	for (int c = 0; c < 256; c++)
	{
		int bit0, bit1, bit2;
		bit0 = BIT(c, 0);
		bit1 = BIT(c, 1);
		bit2 = BIT(c, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(c, 3);
		bit1 = BIT(c, 4);
		bit2 = BIT(c, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(c, 6);
		bit1 = BIT(c, 7);
		int const b = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(c, rgb_t(r, g, b));
	}
}


void eolith16_state::eolith16(machine_config &config)
{
	E116(config, m_maincpu, 60_MHz_XTAL);        // E1-16T (TQFP), no internal multiplier
	m_maincpu->set_addrmap(AS_PROGRAM, &eolith16_state::eolith16_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(eolith16_state::eolith_speedup), "screen", 0, 1);

	EEPROM_93C66_8BIT(config, "eeprom")
			.erase_time(attotime::from_usec(250))
			.write_time(attotime::from_usec(250));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(512, 262);
	m_screen->set_visarea(0, 319, 0, 199);
	m_screen->set_screen_update(FUNC(eolith16_state::screen_update_eolith16));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(eolith16_state::eolith16_palette), 256);

	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

/*

KlonDike+
Eolith, 1999

This game is like Freecell which comes with Windows XP etc

PCB Layout
----------

9812C
|-------------------------------------------|
|TDA1519A      KD.U28  PAL        RESET_SW  |
|     VOL  M6295   1MHz                60MHz|
|               EV0514-001                  |
|        14.31818MHz            E1-16T      |
|                                           |
|                          GM71C18160       |
|   TEST_SW                                 |
|J                                          |
|A                  KM6161002               |
|M  SERV_SW                                 |
|M                              KD.U5       |
|A                                          |
|                               93C66       |
|                                           |
|                                           |
|                                     TL7705|
|                                           |
|    *    *    *    *    *    KD.U31        |
|                                           |
|-------------------------------------------|
Notes:
      E1-16T - Main CPU, HyperStone E1-16T, clock input 60.000MHz (TQFP100)
      M6294  - Oki M6295 sound chip, clock 1.000MHz, sample rate = 1000000 / 132 (QFP44)
      93C66  - Atmel 93C66 4096bit Serial EEPROM (DIP8)
   KM6161002 - Samsung Electronics KM6161002BJ-10 64k x16 High Speed CMOS Static RAM (SOJ44)
  GM71C18160 - LG Semiconductor GM71C18160CJ6 1M x16 DRAM (SOJ42)
  EV0514-001 - Custom Eolith IC (QFP100)
      VSync  - 60Hz
      HSync  - 15.64kHz
          *  - Empty DIP42 sockets
       ROMs  -
               KD.U28 - TMS27C040 EPROM, M6295 samples (DIP32)
               KD.U5  - TMS27C040 EPROM, Main Program (DIP32)
               KD.U31 - ST M27C160 EPROM, Graphics Data (DIP42)
*/

ROM_START( klondkp )
	ROM_REGION16_BE( 0x80000, "maincpu", 0 ) /* E1-16T program code */
	ROM_LOAD( "kd.u5",  0x000000, 0x080000, CRC(591f0c73) SHA1(a9f338204c77a724fa6a6e08d78ca89bd5191aba) )

	ROM_REGION16_BE( 0x200000, "maindata", 0 ) /* gfx data */
	ROM_LOAD16_WORD_SWAP( "kd.u31", 0x000000, 0x200000, CRC(e5dd12b5) SHA1(0a0cd75cbcdccce3575e5a58ba09c88452e1a5ee) )

	ROM_REGION( 0x80000, "oki", 0 ) /* oki samples */
	ROM_LOAD( "kd.u28", 0x000000, 0x080000, CRC(c12112a1) SHA1(729bbaca6db933a730099a4a560a10ed99cae1c3) )
ROM_END

void eolith16_state::init_eolith16()
{
	init_speedup();
}

} // anonymous namespace


GAME( 1999, klondkp, 0, eolith16, eolith16, eolith16_state, init_eolith16, ROT0, "Eolith", "KlonDike+", MACHINE_SUPPORTS_SAVE )
