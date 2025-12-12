// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Nicola Salmoria
/*******************************************************************************************

Fever Soccer (c) 2004 Seibu Kaihatsu

A down-grade of the Seibu SPI Hardware with SH-2 as main cpu.

driver by Angelo Salese & Nicola Salmoria

The input routines are very convoluted in comparison to previous Seibu games,
looping over a complex control structure. Could these be derived from the
"Touch Panel System" DVD mahjong games Seibu released under the CATS label?

TODO:
- Layout including lamps
- Hopper only works in "COIN HOPPER" mode
- Do button 5 or remaining DIPs actually do anything outside service mode?
- Make device of RISE11 sprite hardware also used in seibu/seibuspi.cpp
  and seibu/seibucats.cpp

============================================================================

Fever Soccer (JAMMA based Gambling Game)

Seibu Kaihatsu Inc.

PCB (c) 2004 SYS_SH2B + SYS_SH2B Rom Board

Very simple PCB contains:

     CPU: Hitachi SH-2 (HD6417604F28)
   Audio: OKI 6295 (rebadged as AD-65)
GFX CHIP: RISE11 (custom graphics chip with programmable decryption)
  EEPROM: ST93C56A
     OSC: 28.63636MHz
     DSW: Single 4 switch

Other: Battery (CR2032)
       Sigma XLINX 9572
       JRC 6355E Serial Real Time Clock (connected to a 32.768KHz OSC)

RAM: BSI BS62LV1024SC-70 (x2)
     EtronTech EM51256C-15J (x4)


PRG0.U0139 ST M27C1001 (PCB can handle up to 27C080)
PRG1.U0140 ST M27C1001

PCM.U0743 ST M27C4001

On the SYS_SH2B ROM BOARD:
OBJ1.U011 ST M27C160
OBJ2.U012 ST M27C160
OBJ3.U013 ST M27C160

Not used / unpopulated:

U0145 LH28F800SU (Alt program ROM near SH-2)

U0744 LH28F800SU (Alt PCM ROM near AD-65)

U0561 LH28F800SU OBJ1-1 (near ROM BOARD connector & RISE11 chip)
U0562 LH28F800SU OBJ2-1
U0563 LH28F800SU OBJ3-1
U0564 LH28F800SU OBJ4-1

U0218 RTL8019AS Realtek Full-Duplex Ethernet Controller with Plug and Play Function
X221 20MHz
U0225 YCL20F001N 10 Base T Low Pass Filter

U089 MAX232 Dual EIA Driver/Receiver

*******************************************************************************************/

#include "emu.h"

#include "sei25x_rise1x_spr.h"
#include "seibuspi_m.h"

#include "cpu/sh/sh7604.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/rtc4543.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class feversoc_state : public driver_device
{
public:
	feversoc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainram1(*this, "workram1"),
		m_mainram2(*this, "workram2"),
		m_nvram(*this, "nvram"),
		m_spriteram(*this, "spriteram", 0x2000, ENDIANNESS_BIG),
		m_in(*this, {"IN1", "IN0"}),
		m_lamps(*this, "lamp%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_rtc(*this, "rtc"),
		m_hopper(*this, "hopper"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen")
	{ }

	void init_feversoc() ATTR_COLD;

	void feversoc(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t in_r(offs_t offset);
	void output_w(uint16_t data);
	void output2_w(uint16_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void irq_ack_w(uint16_t data);

	void main_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint32_t> m_mainram1;
	required_shared_ptr<uint32_t> m_mainram2;
	required_shared_ptr<uint32_t> m_nvram;
	memory_share_creator<uint16_t> m_spriteram;
	required_ioport_array<2> m_in;
	output_finder<7> m_lamps;

	required_device<sh7604_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<jrc6355e_device> m_rtc;
	required_device<ticket_dispenser_device> m_hopper;
	required_device<palette_device> m_palette;
	required_device<sei25x_rise1x_device> m_spritegen;
};


uint32_t feversoc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect); //black pen

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes());

	return 0;
}

uint16_t feversoc_state::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void feversoc_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

uint16_t feversoc_state::in_r(offs_t offset)
{
	return m_in[offset]->read() & 0xffff;
}

void feversoc_state::output_w(uint16_t data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 6));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 6));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	// BIT(data, 1) coin out counter
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));
	m_hopper->motor_w(BIT(data, 3)); // coin hopper or prize hopper
	m_oki->set_rom_bank(BIT(data, 5));

	m_eeprom->di_write(BIT(data, 15));
	m_eeprom->clk_write(BIT(data, 14) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write(BIT(data, 13) ? ASSERT_LINE : CLEAR_LINE);

	m_rtc->data_w(BIT(data, 11));
	m_rtc->wr_w(BIT(data, 10) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->clk_w(BIT(data, 9) ? ASSERT_LINE : CLEAR_LINE);
	m_rtc->ce_w(BIT(data, 8) ? ASSERT_LINE : CLEAR_LINE);
}

void feversoc_state::output2_w(uint16_t data)
{
	for (int n = 0; n < 7; n++)
		m_lamps[n] = BIT(data, n); // LAMP1-LAMP7

	machine().bookkeeping().coin_counter_w(2, BIT(data, 13)); // key in
	//BIT(data, 14) key out
}


void feversoc_state::main_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom();
	map(0x02000000, 0x0202ffff).ram().share(m_mainram1); //work ram
	map(0x02030000, 0x02033fff).ram().share(m_nvram);
	map(0x02034000, 0x0203dfff).ram().share(m_mainram2); //work ram
	map(0x0203e000, 0x0203ffff).rw(FUNC(feversoc_state::spriteram_r), FUNC(feversoc_state::spriteram_w));
	map(0x06000000, 0x06000001).w(FUNC(feversoc_state::output_w));
	map(0x06000002, 0x06000003).w(FUNC(feversoc_state::output2_w));
	map(0x06000006, 0x06000007).w(FUNC(feversoc_state::irq_ack_w));
	map(0x06000008, 0x0600000b).r(FUNC(feversoc_state::in_r));
	map(0x0600000d, 0x0600000d).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0x06010000, 0x0601007f).rw("obj", FUNC(seibu_encrypted_sprite_device::read), FUNC(seibu_encrypted_sprite_device::write));
	map(0x06010060, 0x06010063).nopw(); // sprite buffering
	map(0x06018000, 0x06019fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
}

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+0,RGN_FRAC(0,3)+8,RGN_FRAC(1,3)+0,RGN_FRAC(1,3)+8,RGN_FRAC(2,3)+0,RGN_FRAC(2,3)+8 },
	{ STEP8(7,-1), STEP8(16+7,-1) },
	{ STEP16(0,32) },
	16*32
};


static GFXDECODE_START( gfx_feversoc )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0, 0x40 )
GFXDECODE_END

static INPUT_PORTS_START( feversoc )
	// The "ANALIZE" input shown in test mode does not exist on this hardware.
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key In (Service)")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", FUNC(rtc4543_device::data_r))
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION( "DIP1:1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "DIP1:2" )
	PORT_DIPNAME( 0x0400, 0x0400, "Backup Memory" ) PORT_DIPLOCATION( "DIP1:3" )
	PORT_DIPSETTING(    0x0400, "Use" )
	PORT_DIPSETTING(    0x0000, "Reset" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "DIP1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "DIP1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "DIP1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "DIP1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "DIP1:8" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 (BTN1)")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 (BTN2)")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 (BTN3)")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet (BTN4)")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unknown (BTN5)") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out (BTN6)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Coin Out (BTN7)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void feversoc_state::screen_vblank(int state)
{
	if (state)
		m_maincpu->set_input_line(8, ASSERT_LINE);
}

void feversoc_state::irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(8, CLEAR_LINE);
}

void feversoc_state::machine_start()
{
	m_lamps.resolve();
}

void feversoc_state::feversoc(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(28'636'363);

	/* basic machine hardware */
	SH7604(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &feversoc_state::main_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1); //dynamic resolution?
	screen.set_screen_update(FUNC(feversoc_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(feversoc_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x1000);

	SEI25X_RISE1X(config, m_spritegen, 0, m_palette, gfx_feversoc);
	m_spritegen->set_screen("screen");
	m_spritegen->set_pix_raw_shift(6);
	m_spritegen->set_pri_raw_shift(14);
	m_spritegen->set_transpen(63);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, MASTER_CLOCK / 16, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.6); //pin 7 & frequency not verified (clock should be 28,6363 / n)

	EEPROM_93C56_16BIT(config, m_eeprom);

	JRC6355E(config, m_rtc, XTAL(32'768));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(60));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

// Date of build, as displayed in service mode, is Apr 30 2004/22:44:21.
// Program ROMs also contain leftover strings and tables from a previous build dated Apr 26 2004/20:25:31.
ROM_START( feversoc )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prog0.u0139",   0x00001, 0x20000, CRC(fa699503) SHA1(96a834d4f7d5b764aa51db745afc2cd9a7c9783d) )
	ROM_LOAD16_BYTE( "prog1.u0140",   0x00000, 0x20000, CRC(fd4d7943) SHA1(d7d782f878656bc79d70589f9df2cbcfff0adb5e) )

	ROM_REGION( 0x600000, "sprites", 0)    /* sprites */
	ROM_LOAD("obj1.u011", 0x000000, 0x200000, CRC(d8c8dde7) SHA1(3ef815fb1e21a0bd907ee835bc7a32d80f6a9d28) )
	ROM_LOAD("obj2.u012", 0x200000, 0x200000, CRC(8e93bfda) SHA1(3b4740cefb164efc320fb69f58e8800d2646fea6) )
	ROM_LOAD("obj3.u013", 0x400000, 0x200000, CRC(8c8c6e8b) SHA1(bed4990d6eebb7aefa200ad2bed9b7e71e6bd064) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pcm.u0743", 0x00000, 0x80000, CRC(20b0c0e3) SHA1(dcf2f620a8fe695688057dbaf5c431a32a832440) )
ROM_END

void feversoc_state::init_feversoc()
{
	uint32_t *rom = (uint32_t *)memregion("maincpu")->base();

	seibuspi_rise11_sprite_decrypt_feversoc(memregion("sprites")->base(), 0x200000);

	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0003ffff, 1, rom);
	m_maincpu->sh2drc_add_fastram(0x02000000, 0x0202ffff, 0, &m_mainram1[0]);
	m_maincpu->sh2drc_add_fastram(0x02030000, 0x02033fff, 0, &m_nvram[0]);
	m_maincpu->sh2drc_add_fastram(0x02034000, 0x0203dfff, 0, &m_mainram2[0]);
	m_maincpu->sh2drc_add_fastram(0x0203e000, 0x0203ffff, 0, &m_spriteram[0]);
}

} // anonymous namespace


GAME( 2004, feversoc, 0, feversoc, feversoc, feversoc_state, init_feversoc, ROT0, "Seibu Kaihatsu", "Fever Soccer", 0 )
