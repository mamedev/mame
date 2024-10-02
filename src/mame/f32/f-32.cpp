// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************

 F-E1-32 driver


 Supported Games      PCB-ID
 ----------------------------------
 Mosaic               F-E1-32-009
 Royal Poker 2        F-E1-32N-COM9e

 driver by Pierpaolo Prazzoli

 Outputs for Royal Poker 2 (royalpk2):
  0: High button
  1: Stop button
  2: Double Up button
  3: Low button *and* Bet button
  4: Max Bet button
  5: Change button
  6: Gift button
  7: Hold 1 button
  8: Hold 2 button
  9: Hold 3 button
 10: Hold 4 button
 11: Hold 5 button
 12: Hold Clear button
 13: Staff Call button
 14: Start / Deal / Draw button
 15: Half Double button
 16: 2x Double button
 17: Gift 1 lamp
 18: Gift 2 lamp
 19: Gift 3 lamp
 20: 'Medal' (coin output) lamp
 21: Counter 1 lamp
 22: Counter 2 lamp
 23: Counter 3 lamp
 24: Counter 4 lamp
 25: Counter 5 lamp
 26: Light 1 lamp
 27: Light 2 lamp
 28: Light 3 lamp
 29: Light 4 lamp
 30: Light 5 lamp

 TODO: royalpk2 is marked non-working, as the machine will soft-lock
 when attempting to use either settings-save options in the service
 menu.

*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class fe132_state : public driver_device
{
public:
	fe132_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_videoram(*this, "videoram"),
		m_p1(*this, "P1"),
		m_system_p2(*this, "SYSTEM_P2")
	{ }

protected:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_map(address_map &map) ATTR_COLD;

	required_device<hyperstone_device>  m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<uint32_t> m_videoram;
	required_ioport m_p1;
	required_ioport m_system_p2;
};

class mosaicf2_state : public fe132_state
{
public:
	mosaicf2_state(const machine_config &mconfig, device_type type, const char *tag) :
		fe132_state(mconfig, type, tag)
	{ }

	void mosaicf2(machine_config &config);

protected:
	uint32_t input_port_1_r();
	void mosaicf2_io(address_map &map) ATTR_COLD;
};

class royalpk2_state : public fe132_state
{
public:
	royalpk2_state(const machine_config &mconfig, device_type type, const char *tag) :
		fe132_state(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamps%u", 0U),
		m_okibank(*this, "okibank")
	{ }

	void royalpk2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void royalpk2_io(address_map &map) ATTR_COLD;
	void royalpk2_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	void protection_seed_w(offs_t offset, uint32_t data);
	uint32_t protection_response_r();

	template <int Bank> void outputs_w(uint32_t data);

	required_device<nvram_device> m_nvram;
	required_device<hopper_device> m_hopper;
	output_finder<31> m_lamps;
	required_memory_bank m_okibank;

	uint16_t m_protection_index;
	uint8_t m_protection_response_byte;
	int m_protection_response_bit;
};

uint32_t fe132_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x10000; offs++)
	{
		int y = offs >> 8;
		int x = offs & 0xff;

		if ((x < 0xa0) && (y < 0xe0))
		{
			bitmap.pix(y, (x * 2) + 0) = (m_videoram[offs] >> 16) & 0x7fff;
			bitmap.pix(y, (x * 2) + 1) = (m_videoram[offs] >>  0) & 0x7fff;
		}
	}

	return 0;
}



void fe132_state::common_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0x40000000, 0x4003ffff).ram().share("videoram");
	map(0x80000000, 0x80ffffff).rom().region("user2", 0);
	map(0xfff00000, 0xffffffff).rom().region("user1", 0);
}

uint32_t mosaicf2_state::input_port_1_r()
{
	// burn a bunch of cycles because this is polled frequently during busy loops

	offs_t pc = m_maincpu->pc();
	if (pc == 0x000379de || pc == 0x000379cc)
		m_maincpu->eat_cycles(100);
	//else printf("PC %08x\n", pc );
	return m_system_p2->read();
}


void mosaicf2_state::mosaicf2_io(address_map &map)
{
	map(0x4003, 0x4003).r("oki", FUNC(okim6295_device::read));
	map(0x4813, 0x4813).r("ymsnd", FUNC(ym2151_device::status_r));
	map(0x5000, 0x5003).portr("P1");
	map(0x5200, 0x5203).r(FUNC(mosaicf2_state::input_port_1_r));
	map(0x5400, 0x5403).portr("EEPROMIN");
	map(0x6003, 0x6003).w("oki", FUNC(okim6295_device::write));
	map(0x6803, 0x6803).w("ymsnd", FUNC(ym2151_device::data_w));
	map(0x6813, 0x6813).w("ymsnd", FUNC(ym2151_device::address_w));
	map(0x7000, 0x7003).portw("EEPROMCLK");
	map(0x7200, 0x7203).portw("EEPROMCS");
	map(0x7400, 0x7403).portw("EEPROMOUT");
}


static INPUT_PORTS_START( mosaicf2 )
	PORT_START("P1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00007800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START( "EEPROMCLK" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START( "EEPROMCS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END




void mosaicf2_state::mosaicf2(machine_config &config)
{
	/* basic machine hardware */
	E132XN(config, m_maincpu, XTAL(20'000'000)*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &mosaicf2_state::common_map);
	m_maincpu->set_addrmap(AS_IO, &mosaicf2_state::mosaicf2_io);
	m_maincpu->set_vblank_int("screen", FUNC(mosaicf2_state::irq0_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom);
	m_eeprom->erase_time(attotime::from_usec(1));
	m_eeprom->write_time(attotime::from_usec(1));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(512, 512);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(mosaicf2_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); /* 3.579545 MHz */
	ymsnd.add_route(0, "lspeaker", 1.0);
	ymsnd.add_route(1, "rspeaker", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181)/8, okim6295_device::PIN7_HIGH)); /* 1.7897725 MHz */
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



static INPUT_PORTS_START( royalpk2 )
	PORT_START("P1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Max Bet")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Change")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gift")
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Staff Call") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Start/Deal/Draw") PORT_CODE(KEYCODE_1)

	PORT_START("SYSTEM_P2")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_GAMBLE_HALF )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("2x Double") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Credit Pay Out") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Income") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Income Clear") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Medal Clear") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Operator Gift") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("Over Flow") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Medal Empty") PORT_CODE(KEYCODE_L)

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START( "EEPROMCLK" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START( "EEPROMCS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

void royalpk2_state::royalpk2_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();
	map(0x40000000, 0x4003ffff).ram().share("videoram");
	map(0x80000000, 0x807fffff).rom().region("user2", 0);
	map(0xfff00000, 0xffffffff).rom().region("user1", 0);
}

void royalpk2_state::royalpk2_io(address_map &map)
{
	// map(0x4000, 0x41ff).readonly().share("nvram"); // seems to expect to read NVRAM from here

	map(0x4603, 0x4603).r("oki", FUNC(okim6295_device::read));

	map(0x4800, 0x4803).portr("P1");
	map(0x4900, 0x4903).portr("SYSTEM_P2");

	map(0x4a00, 0x4a03).portr("EEPROMIN");

	map(0x4b00, 0x4b03).r(FUNC(royalpk2_state::protection_response_r));

	map(0x6000, 0x61ff).ram().share("nvram");

	map(0x6603, 0x6603).w("oki", FUNC(okim6295_device::write));

	map(0x6800, 0x6803).portw("EEPROMCLK");
	map(0x6900, 0x6903).portw("EEPROMCS");
	map(0x6a00, 0x6a03).portw("EEPROMOUT");

	// map(0x6b00, 0x6b03).nopw(); // bits 8, 9, 10 and 13, 14 used

	map(0x6c03, 0x6c03).lw8(NAME([this] (uint8_t data) { m_okibank->set_entry(data & 0x03); })); // TODO: double check this

	map(0x6d00, 0x6d03).w(FUNC(royalpk2_state::protection_seed_w));

	map(0x7000, 0x7003).w(FUNC(royalpk2_state::outputs_w<0>));
	map(0x7100, 0x7103).w(FUNC(royalpk2_state::outputs_w<1>));
	map(0x7200, 0x7203).w(FUNC(royalpk2_state::outputs_w<2>));
	map(0x7300, 0x7303).w(FUNC(royalpk2_state::outputs_w<3>));
	map(0x7400, 0x7403).w(FUNC(royalpk2_state::outputs_w<4>));
}

void royalpk2_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

void royalpk2_state::machine_start()
{
	save_item(NAME(m_protection_index));
	save_item(NAME(m_protection_response_byte));
	save_item(NAME(m_protection_response_bit));

	m_lamps.resolve();

	for (int i = 0; i < 31; i++)
		m_lamps[i] = 0;

	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
}

void royalpk2_state::machine_reset()
{
	m_protection_index = 0;
	m_protection_response_byte = 0;
	m_protection_response_bit = 0;
}

void royalpk2_state::protection_seed_w(offs_t offset, uint32_t data)
{
	if ((m_protection_index % 2) == 0)
		m_protection_response_byte = 0x85;
	else if ((m_protection_index % 2) == 1)
		m_protection_response_byte = 0x81;

	m_protection_index++;
	m_protection_response_bit = 7;
}

uint32_t royalpk2_state::protection_response_r()
{
	uint32_t data = BIT(m_protection_response_byte, m_protection_response_bit) << 6;
	if (m_protection_response_bit > 0)
	{
		m_protection_response_bit--;
	}
	else
	{
		logerror("Protection response byte fully sent.\n");
	}
	return data;
}

template <int Bank>
void royalpk2_state::outputs_w(uint32_t data)
{
	// TODO: Lamps 26 to 30 occupy multiple bits of a given output port. Find out if it's due to brightness control or something else.

	switch (Bank)
	{
		case 0:
			m_lamps[0] = BIT(data, 8);
			for (int i = 0; i < 6; i++)
				m_lamps[1 + i] = BIT(data, 10 + i);
			break;
		case 1:
			for (int i = 0; i < 8; i++)
				m_lamps[7 + i] = BIT(data, 8 + i);
			break;
		case 2:
			for (int i = 0; i < 8; i++)
				m_lamps[15 + i] = BIT(data, 8 + i);
			break;
		case 3:
			for (int i = 0; i < 3; i++)
				m_lamps[23 + i] = BIT(data, 8 + i);
			m_lamps[26] = (data & 0x3000) ? 1 : 0;
			m_lamps[27] = (data & 0xc000) ? 1 : 0;
			break;
		case 4:
			m_lamps[28] = (data & 0x0f00) ? 1 : 0;
			m_lamps[29] = (data & 0x3000) ? 1 : 0;
			m_lamps[30] = (data & 0xc000) ? 1 : 0;
			break;
	}
}

void royalpk2_state::royalpk2(machine_config &config)
{
	/* basic machine hardware */
	GMS30C2132(config, m_maincpu, XTAL(50'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &royalpk2_state::royalpk2_map);
	m_maincpu->set_addrmap(AS_IO, &royalpk2_state::royalpk2_io);
	m_maincpu->set_vblank_int("screen", FUNC(royalpk2_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom);
	m_eeprom->erase_time(attotime::from_usec(1));
	m_eeprom->write_time(attotime::from_usec(1));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper, attotime::from_msec(100));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(512, 512);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(royalpk2_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

//  ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4)); /* 3.579545 MHz */
//  ymsnd.add_route(0, "lspeaker", 1.0);
//  ymsnd.add_route(1, "rspeaker", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181)/8, okim6295_device::PIN7_HIGH)); /* 1.7897725 MHz */
	oki.set_addrmap(0, &royalpk2_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	// there is a 16c550 for communication
}


/*

Mosaic (c) 1999 F2 System

   CPU: Hyperstone E1-32XN
 Video: QuickLogic QL2003-XPL84C FPGA
 Sound: OKI 6295, BS901 (YM2151) & BS902 (YM3012)
   OSC: 20MHz & 14.31818MHz
EEPROM: 93C46 (controlled through FPGA)

F-E1-32-009
+------------------------------------------------------------------+
|            VOL                               +---------+         |
+-+                              YM3012        |   SND   |         |
  |                                            +---------+         |
+-+                              YM2151            OKI6295         |
|                                                                  |
|                                   +---------------+              |
|                                   |               |              |
|J                   +-------+      |               |              |
|A                   | VRAML |      | QuickLogic    |  14.31818MHz |
|M                   +-------+      | QL2003-XPL84C |              |
|M                   +-------+      | 9819 BA       |   +-----+    |
|A                   | VRAMU |      |               |   |93C46|    |
|                    +-------+      +---------------+   +-----+    |
|C                                                                 |
|O                                      +---------+   +---------+  |
|N                                      |   L00   |   |   U00   |  |
|N                                      |         |   |         |  |
|E                                      +---------+   +---------+  |
|C                   +------------+     +---------+   +---------+  |
|T                   |            |     |   L01   |   |   U01   |  |
|O                   |            |     |         |   |         |  |
|R                   | HyperStone |     +---------+   +---------+  |
|                    |  E1-32XN   |     +---------+   +---------+  |
|                    |            |     |   L02   |   |   U02   |  |
|          +-----+   |            |     |         |   |         |  |
|          |DRAML|   +------------+     +---------+   +---------+  |
+-+        +-----+                      +---------+   +---------+  |
  |        +-----+               20MHz  |   L03   |   |   U03   |  |
+-+        |DRAMU|                      |         |   |         |  |
|          +-----+    +----------+      +---------+   +---------+  |
|  +--+ +--+          |   ROM1   |                                 |
|  |S3| |S1|          +----------+                                 |
+------------------------------------------------------------------+

S3 is a reset button
S1 is the setup button

VRAML & VRAMU are KM6161002CJ-12
DRAML & DRAMU are GM71C18163CJ6

ROM1 & SND are standard 27C040 and/or 27C020 eproms
L00-L03 & U00-U03 are 29F1610ML Flash roms


todo: royalpk2 layout (it's very different)
*/

ROM_START( mosaicf2 ) /* Released October 1999 */
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "rom1.bin",            0x80000, 0x080000, CRC(fceb6f83) SHA1(b98afb477627c3b2d584c0f0fb26c4dd5b1a31e2) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )  /* gfx data */
	ROM_LOAD32_WORD_SWAP( "u00.bin", 0x000000, 0x200000, CRC(a2329675) SHA1(bff8974fab9120274821c9c9646744317f47c79c) )
	ROM_LOAD32_WORD_SWAP( "l00.bin", 0x000002, 0x200000, CRC(d96fe93b) SHA1(005d9889077825fc0e308d2981f6fca5e6b51fe8) )
	ROM_LOAD32_WORD_SWAP( "u01.bin", 0x400000, 0x200000, CRC(6379e73f) SHA1(fe5abafbcbd828795cb06a08763fae1bbe2a75ad) )
	ROM_LOAD32_WORD_SWAP( "l01.bin", 0x400002, 0x200000, CRC(a269ea82) SHA1(d962a8b3293c6f46dbefa49859b2b3e594e7a386) )
	ROM_LOAD32_WORD_SWAP( "u02.bin", 0x800000, 0x200000, CRC(c17f95cd) SHA1(1c701185be138b615d2851866288647f40809c28) )
	ROM_LOAD32_WORD_SWAP( "l02.bin", 0x800002, 0x200000, CRC(69cd9c5c) SHA1(6b4d204a6ab5f36dfba9053bb3be2d094fcfdd00) )
	ROM_LOAD32_WORD_SWAP( "u03.bin", 0xc00000, 0x200000, CRC(0e47df20) SHA1(6f6c3e7fc8c99db7ddc73d8d10a661373bb72a1a) )
	ROM_LOAD32_WORD_SWAP( "l03.bin", 0xc00002, 0x200000, CRC(d79f6ca8) SHA1(4735dda9269aa05ba1251d335dc73914f5cb43b0) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.bin",             0x000000, 0x040000, CRC(4584589c) SHA1(5f9824724f840767c3dc1dc04b203ddf3d78b84c) )
ROM_END

ROM_START( royalpk2 )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prog1",            0x80000, 0x080000, CRC(e1546304) SHA1(b628b347ba7fbbae948e98e72aa5ea190c5d0f2b) )

	ROM_REGION32_BE( 0x800000, "user2", 0 )  /* gfx data */
	ROM_LOAD32_WORD_SWAP( "1.u00", 0x000000, 0x200000, CRC(b397a805) SHA1(3fafa8533c793f41d0567b76667d3f3478eb9c1d) )
	ROM_LOAD32_WORD_SWAP( "2.l00", 0x000002, 0x200000, CRC(83a67d20) SHA1(9bf4c3da0cd1aab2488f260f694493d8ee25883e) )
	ROM_LOAD32_WORD_SWAP( "3.u01", 0x400000, 0x200000, CRC(f7b9d508) SHA1(5d98687c6cf158df8134d88d3726778d3762b411) )
	ROM_LOAD32_WORD_SWAP( "4.l01", 0x400002, 0x200000, CRC(dcff4960) SHA1(f742c7a3b62262c4b0210db9df03f51b3f600bf2) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd2",             0x000000, 0x080000, CRC(f25e3315) SHA1(ce5350ecba6769b17bb01d82b55f26ded4d51773) )
ROM_END

} // anonymous namespace

GAME( 1999, mosaicf2, 0, mosaicf2, mosaicf2, mosaicf2_state, empty_init, ROT0, "F2 System", "Mosaic (F2 System)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, royalpk2, 0, royalpk2, royalpk2, royalpk2_state, empty_init, ROT0, "F2 System", "Royal Poker 2 (Network version 3.12)", MACHINE_NOT_WORKING )
