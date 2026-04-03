// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                               -= Sammy Medal Games =-

                                                 driver by Luca Elia

CPU     :   Kawasaki KL5C80A12CFP (Z80 Compatible High Speed Microcontroller)
Video   :   TAXAN KY-3211
Sound   :   OKI M9810B
NVRAM   :   93C46 and battery backed RAM

Cartridge based system. Carts contain just some 16Mb flash eeproms.

Info from Tatsuya Fujita:

According to some news articles for the AOU show 2002 and 2003 the correct system name
is "Treasure Fall" (despite the cart label is "Treasure Hall").

Dumped games:

1999 Dokidoki Kingyo Sukui        https://youtu.be/Z0tOjG_tteU
1999 Shatekids                    https://youtu.be/aWzYlvm6uIs
2000 Animal Catch                 https://youtu.be/U4L5EwWbxqw
2000 Itazura Monkey               https://youtu.be/GHxiqUQRpV8
2000 Pye-nage Taikai              https://youtu.be/oL2OIbrv-KI
2000 Taihou de Doboon             https://youtu.be/loPP3jt0Ob0
2001 Hae Hae Ka Ka Ka             https://youtu.be/37IxYCg0tic
2001 Kotekitai Slot               https://youtu.be/IohrnGIma4A
2002 Gun Kids
2003 Go Go Cowboy (EN, prize)     https://youtu.be/rymtzmSXjuA
2003 Wantouchable                 https://youtu.be/aRcTCdZZLRo

Games with the same cabinet, or in the Treasure Fall series, which might be on the same hardware:

1999 Otakara Locomo               https://youtu.be/J0NwMWO3SdY
2000 Otoshicha Ottotto            https://youtu.be/AybhPHTFvMo
2001 Mushitori Meijin
2001 Morino Dodgeball Senshuken   https://youtu.be/k98KIRjTYbY
2001 Waiwai Wanage                https://youtu.be/4GmwPTk_Er4
2001 Zarigani Tsuri               https://youtu.be/NppRdebkUaQ
2002 Shateki Yokochou             https://youtu.be/LPZLWP1x5o8
2002 Ipponzuri Slot
2002 Karateman                    https://youtu.be/EIrVHEAv3Sc
2002 Perfect Goal (screenless)    https://youtu.be/ilneyp-8dBI
2003 Go Go Cowboy (JP, medal)     https://youtu.be/qYDw2sxNRqE
2003 Kurukuru Train               https://youtu.be/Ef7TQX4C9fA
2003 Safari Kingdom (screenless)
2003 Zakuzaku Kaizokudan
2004 Animal Punch
2004 Dotabata Zaurus              https://youtu.be/Gxt6klOYZ9A
2004 Excite Hockey (screenless)
2004 Fishing Battle (screenless)
2004 Home Run Derby (screenless)
2004 Ninchuu Densetsu             https://youtu.be/_ifev_CJROs
2004 Outer Space (screenless)
2004 Pretty Witch Pointe          https://youtu.be/lYAwfHyywfA

Original list from:
http://www.tsc-acnet.com/index.php?sort=8&action=cataloglist&s=1&mode=3&genre_id=40&freeword=%25A5%25B5%25A5%25DF%25A1%25BC

--------------------------------------------------------------------------------------

PCB:

  Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
  MAIN PCB VER2 VM12-6001-0 (Sticker)

CPU:

  KAWASAKI KL5C80A12CFP (@U1) - Z80 Compatible High Speed Microcontroller
  XTAL 20 MHz  (@X1)
  MX29F040TC-12 VM1211L01 (@U2) - 4M-bit [512kx8] CMOS Equal Sector Flash Memory
  BSI BS62LV256SC-70      (@U4) - Very Low Power/Voltage CMOS SRAM 32K X 8 bit
  (or Toshiba TC55257DFL-70L)

Video:

  TAXAN KY-3211 (@U17)
  OKI M548262-60 (@U18) - 262144-Word x 8-Bit Multiport DRAM
  XTAL 27 MHz (@X3)

Sound:

  OKI M9810B (@U11)
  XTAL 4.09 MHz (@X2)
  Trimmer (@VR1)
  Toshiba TA7252AP (@U16) - 5.9W Audio Power Amplifier

Other:

  Xilinx XC9536 VM1212F01 (@U5) - In-System Programmable CPLD
  MX29F0?TC (@U3) - Empty 32 Pin DIP Socket
  M5295A (@U8) - Watchdog Timer (Near CUT-DEBUG MODE Jumper)
  M93C46MN6T (@U6) - Serial EEPROM
  RTC8564 (@U7) - not populated
  SN75C1168 (@U10) - Dual RS-422 Transceiver
  Cell Battery (@BAT)
  25 Pin Edge Connector
  56 Pin Cartridge Connector
  6 Pin Connector - +5V (1), GND (2), TCLK (3), TDO (4), TDI (5), TMS (6)

On Go Go Cowboy, U2 and U10 are unpopulated, but U3 is occupied by a
ST M27C4001-10F1 EPROM.

--------------------------------------------------------------------------------------

To Do:

- Consolidate the sammymdl games in one memory map and run the BIOS without ROM patches
- pyenaget intro: when the theater scrolls out to the left, the train should scroll in from the right,
  with no visible gaps. It currently leaves a small gap.
- tdoboon: no smoke from hit planes as shown in the video? Tiles are present (f60-125f) and used in demo mode.

Notes:

- "BACKUP RAM NG" error: in test mode, choose "SET MODE" -> "RAM CLEAR" and keep the button pressed for long.

*************************************************************************************************************/

#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim9810.h"
//#include "video/bufsprite.h"
#include "video/ky3211_ky10510.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sammymdl_state : public driver_device
{
public:
	sammymdl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kp69(*this, "maincpu:kp69")
		, m_eeprom(*this, "eeprom")
		, m_spritegen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		//, m_buffered_spriteram(*this, "spriteram")
		, m_hopper(*this, "hopper")
		, m_hopper_small(*this, "hopper_small")
		, m_hopper_large(*this, "hopper_large")
		, m_spriteram(*this, "spriteram")
		, m_io_coin(*this, "COIN")
		, m_leds(*this, "led%u", 0U)
		, m_vblank(0)
	{ }

	void haekaka(machine_config &config) ATTR_COLD;
	void gocowboy(machine_config &config) ATTR_COLD;
	void tdoboon(machine_config &config) ATTR_COLD;
	void animalc(machine_config &config) ATTR_COLD;
	void sammymdl(machine_config &config) ATTR_COLD;
	void itazuram(machine_config &config) ATTR_COLD;
	void pyenaget(machine_config &config) ATTR_COLD;

	void init_itazuram() ATTR_COLD;
	void init_animalc() ATTR_COLD;
	void init_haekaka() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD { m_leds.resolve(); }

private:
	TIMER_DEVICE_CALLBACK_MEMBER(gocowboy_int);

	void coin_counter_w(u8 data);
	void leds_w(u8 data);
	void hopper_w(u8 data);
	u8 coin_hopper_r();

	void gocowboy_leds_w(u8 data);

	void haekaka_leds_w(u8 data);
	void haekaka_coin_counter_w(u8 data);

	void show_3_outputs();
	u8 eeprom_r();
	void eeprom_w(u8 data);

	void vblank_w(u8 data);
	u8 vblank_r();
	u8 haekaka_vblank_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	[[maybe_unused]] void screen_vblank(int state);

	void animalc_io(address_map &map) ATTR_COLD;
	void animalc_map(address_map &map) ATTR_COLD;
	void base_map(address_map &map) ATTR_COLD;
	void gocowboy_io(address_map &map) ATTR_COLD;
	void gocowboy_map(address_map &map) ATTR_COLD;
	void haekaka_map(address_map &map) ATTR_COLD;
	void itazuram_map(address_map &map) ATTR_COLD;
	void tdoboon_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;

	// Required devices
	required_device<kl5c80a12_device> m_maincpu;
	required_device<kp69_device> m_kp69;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ky3211_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// Optional devices
	//optional_device<buffered_spriteram8_device> m_buffered_spriteram;   // not on sammymdl?
	optional_device<ticket_dispenser_device> m_hopper;
	optional_device<ticket_dispenser_device> m_hopper_small;
	optional_device<ticket_dispenser_device> m_hopper_large;

	// Shared pointers
	required_shared_ptr<u8> m_spriteram;

	optional_ioport m_io_coin;

	output_finder<8> m_leds;

	u8 m_out[3]{};
	u8 m_vblank;
};


/***************************************************************************

    Video

***************************************************************************/

u32 sammymdl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_R))  msk |= 8;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	bitmap.fill(m_palette->pens()[0x1000], cliprect);

	// Draw from priority 3 (bottom, converted to a bitmask) to priority 0 (top)
	//u8* spriteram = (m_buffered_spriteram ? m_buffered_spriteram->buffer() : m_spriteram);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, layers_ctrl & 8);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, layers_ctrl & 4);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, layers_ctrl & 2);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, layers_ctrl & 1);

	return 0;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void sammymdl_state::video_map(address_map &map)
{
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).rw(m_spritegen, FUNC(ky3211_device::vtable_r), FUNC(ky3211_device::vtable_w));
	map(0x73000, 0x73021).rw(m_spritegen, FUNC(ky3211_device::vregs_r), FUNC(ky3211_device::vregs_w));
}

void sammymdl_state::base_map(address_map &map)
{
	video_map(map);
	map(0x00000, 0x0ffff).rom().region("mainbios", 0);
	map(0x10000, 0x2ffff).rom().region("maincpu", 0x400);
}

/***************************************************************************
                                 Animal Catch
***************************************************************************/

u8 sammymdl_state::eeprom_r()
{
	return m_eeprom->do_read() ? 0x80 : 0;
}

void sammymdl_state::eeprom_w(u8 data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 6));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 4) ? ASSERT_LINE : CLEAR_LINE);

	if (data & 0x8f)
		logerror("%s: unknown eeeprom bits written %02x\n", machine().describe_context(), data);
}

u8 sammymdl_state::vblank_r()
{
	// mask 0x04 must be set before writing sprite list
	// mask 0x10 must be set or irq/00 hangs?
	return  (m_vblank & ~0x01) | 0x14;
}

void sammymdl_state::vblank_w(u8 data)
{
	m_vblank = (m_vblank & ~0x03) | (data & 0x03);
}

void sammymdl_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vblank &= ~0x01;
	}
}

void sammymdl_state::show_3_outputs()
{
#ifdef MAME_DEBUG
	popmessage("COIN: %02X  LED: %02X  HOP: %02X", m_out[0], m_out[1], m_out[2]);
#endif
}
// Port 31
void sammymdl_state::coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));  // coin1 in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));  // coin2 in
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2));  // medal in

//  machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3)); // coin2 lockout?
//  machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4)); // coin1 lockout
//  machine().bookkeeping().coin_lockout_w(2, BIT(~data, 5)); // medal lockout?

//  data & 0x80? (gocowboy)

	m_out[0] = data;
	show_3_outputs();
}

// Port 32
void sammymdl_state::leds_w(u8 data)
{
	m_leds[0] = BIT(data, 0);   // button

	m_out[1] = data;
	show_3_outputs();
}

// Port b0
// 02 hopper enable?
// 01 hopper motor on (active low)?
void sammymdl_state::hopper_w(u8 data)
{
	m_hopper->motor_w((!(data & 0x01) && (data & 0x02)) ? 1 : 0);

	m_out[2] = data;
	show_3_outputs();
}

u8 sammymdl_state::coin_hopper_r()
{
	u8 ret = m_io_coin->read();

//  if (!m_hopper->read(0))
//      ret &= ~0x01;

	return ret;
}

void sammymdl_state::animalc_map(address_map &map)
{
	base_map(map);
	map(0x60000, 0x63fff).ram().share("nvram");
	map(0x6a000, 0x6ffff).ram();
	map(0x73011, 0x73011).nopw();  // IRQ Enable? Screen disable?
	map(0x73013, 0x73013).rw(FUNC(sammymdl_state::vblank_r), FUNC(sammymdl_state::vblank_w));    // IRQ Ack?
}

void sammymdl_state::animalc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sammymdl_state::hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                               Go Go Cowboy
***************************************************************************/

void sammymdl_state::gocowboy_map(address_map &map)
{
	video_map(map);
	map(0x00000, 0x5ffff).rom().region("mainbios", 0);
	map(0x60000, 0x67fff).ram().share("nvram");
}


void sammymdl_state::gocowboy_leds_w(u8 data)
{
	m_leds[0] = BIT(data, 0);   // button
	m_leds[1] = BIT(data, 1);   // coin lockout? (after coining up, but not for service coin)
	m_leds[2] = BIT(data, 2);   // ? long after a prize is not collected
	m_leds[3] = BIT(data, 3);   // ? "don't forget the large prizes"

	// 10 hopper enable?
	// 20 hopper motor on (active low)?
	m_hopper_small->motor_w(BIT(~data, 5) && BIT(data, 4));
	m_hopper_large->motor_w(BIT(~data, 7) && BIT(data, 6));

	m_out[1] = data;
	show_3_outputs();
}

void sammymdl_state::gocowboy_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x90, 0x90).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).nopw();
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                             Hae Hae Ka Ka Ka
***************************************************************************/

u8 sammymdl_state::haekaka_vblank_r()
{
	return m_screen->vblank() ? 0 : 0x1c;
}

void sammymdl_state::haekaka_leds_w(u8 data)
{
	// All used
	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);
	m_leds[2] = BIT(data, 2);
	m_leds[3] = BIT(data, 3);
	m_leds[4] = BIT(data, 4);
	m_leds[5] = BIT(data, 5);
	m_leds[6] = BIT(data, 6);
	m_leds[7] = BIT(data, 7);

	m_out[1] = data;
	show_3_outputs();
}

void sammymdl_state::haekaka_coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));   // medal out
//                                 data & 0x02 ?
//                                 data & 0x04 ?
//                                 data & 0x10 ?

	m_out[0] = data;
	show_3_outputs();
}

void sammymdl_state::haekaka_map(address_map &map)
{
	base_map(map);
	map(0x60000, 0x61fff).ram().share("nvram");
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r));
}

/***************************************************************************
                              Itazura Monkey
***************************************************************************/

void sammymdl_state::itazuram_map(address_map &map)
{
	base_map(map);
	map(0x60000, 0x63fff).ram().share("nvram");
	map(0x73011, 0x73011).nopw();  // IRQ Enable? Screen disable?
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r)).nopw();  // IRQ Ack?
}

/***************************************************************************
                             Taihou de Doboon
***************************************************************************/

void sammymdl_state::tdoboon_map(address_map &map)
{
	base_map(map);
	map(0x60000, 0x61fff).ram().share("nvram");
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r));
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( sammymdl )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // freeze (itazuram)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5)   // coin1 in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(5)   // coin2 in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3   ) PORT_IMPULSE(5) PORT_NAME("Medal")    // medal in
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )   // test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( haekaka )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME( "Medal" )   // medal in ("chacker")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE  )  // test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1  )  // button
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM   ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )  // service coin / set in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

static INPUT_PORTS_START( gocowboy )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) // shoot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(20) // coin
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM   ) PORT_READ_LINE_DEVICE_MEMBER("hopper_small", FUNC(ticket_dispenser_device::line_r)) // 1/2' pay sensor (small)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM   ) PORT_READ_LINE_DEVICE_MEMBER("hopper_large", FUNC(ticket_dispenser_device::line_r)) // 3/4' pay sensor (large)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Switch") // capsule test (pressed while booting) / next in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Switch") // reset backup ram (pressed while booting) / previous in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  )                           // test mode (keep pressed in game) / select in test mode / service coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

/***************************************************************************

    Machine Drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(sammymdl_state::gocowboy_int)
{
	const int scanline = param;

	// TODO: what really triggers these?
	if (scanline == 240)
	{
		m_kp69->ir_w<0>(1);
		m_kp69->ir_w<0>(0);
	}

	if (scanline == 128)
	{
		m_kp69->ir_w<1>(1);
		m_kp69->ir_w<1>(0);
	}
}

void sammymdl_state::sammymdl(machine_config &config)
{
	KL5C80A12(config, m_maincpu, XTAL(20'000'000));    // !! KL5C80A12CFP @ 10MHz? (actually 4 times faster than Z80) !!
	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::animalc_map);
	m_maincpu->set_addrmap(AS_IO, &sammymdl_state::animalc_io);
	m_maincpu->in_p0_callback().set(FUNC(sammymdl_state::eeprom_r));
	m_maincpu->out_p0_callback().set(FUNC(sammymdl_state::eeprom_w));
	m_maincpu->in_p1_callback().set(FUNC(sammymdl_state::coin_hopper_r));
	m_maincpu->in_p2_callback().set_ioport("BUTTON");
	m_maincpu->out_p3_callback().set(FUNC(sammymdl_state::coin_counter_w));
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::leds_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM
	EEPROM_93C46_8BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0, 0x140-1, 0, 0xf0-1);
	m_screen->set_screen_update(FUNC(sammymdl_state::screen_update));
	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<0>));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // not on sammymdl?
	KY3211(config, m_spritegen, 0, m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "speaker", 0.80, 0);
	oki.add_route(1, "speaker", 0.80, 1);
}

void sammymdl_state::animalc(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::animalc_map);
}

void sammymdl_state::gocowboy(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::gocowboy_map);
	m_maincpu->set_addrmap(AS_IO, &sammymdl_state::gocowboy_io);
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::gocowboy_leds_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(sammymdl_state::gocowboy_int), "screen", 0, 1);

	config.device_remove("hopper");
	TICKET_DISPENSER(config, m_hopper_small, attotime::from_msec(200));
	TICKET_DISPENSER(config, m_hopper_large, attotime::from_msec(200));

	m_screen->screen_vblank().set_nop();
}

void sammymdl_state::haekaka(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::haekaka_map);
	m_maincpu->out_p3_callback().set(FUNC(sammymdl_state::haekaka_coin_counter_w));
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::haekaka_leds_w));

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
}

void sammymdl_state::itazuram(machine_config &config)
{
	sammymdl(config);

	TIMER(config, "scantimer").configure_scanline(FUNC(sammymdl_state::gocowboy_int), "screen", 0, 1);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::itazuram_map);

	m_screen->screen_vblank().set_nop();
}

void sammymdl_state::pyenaget(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::haekaka_map);
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::haekaka_leds_w));

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
}

void sammymdl_state::tdoboon(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::tdoboon_map);

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
	m_screen->set_visarea(0,0x140-1, 0+4,0xf0+4-1);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/

#define SAMMYMDL_BIOS                                                                                                           \
	ROM_REGION( 0x80000, "mainbios", 0 )                                                                                        \
	ROM_SYSTEM_BIOS( 0, "v5", "IPL Ver. 5.0" ) /* (c)2003 */                                                                    \
	ROMX_LOAD( "vm1211l01.u2",   0x000000, 0x080000, CRC(c3c74dc5) SHA1(07352e6dba7514214e778ba39e1ca773e4698858), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "v4", "IPL Ver. 4.0" ) /* (c)2000, ROM patches not correct for this BIOS */                             \
	ROMX_LOAD( "mt201l04.u012",  0x000000, 0x080000, CRC(c8c6d25f) SHA1(5be39fa72b65f2e455ccc146dbab58d24ab46505), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "v1", "IPL Ver. 1.0" ) /* (c)1999, ROM patches not correct for this BIOS */                             \
	ROMX_LOAD( "vm1201l01.u012", 0x000000, 0x080000, CRC(21b2ccbd) SHA1(c3ceea196e5c1c6a9e03cf3a22e9f25f0099a38e), ROM_BIOS(2) )

ROM_START( sammymdl )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )

	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_COPY( "mainbios", 0x00fc00, 0x0000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", ROMREGION_ERASEFF )
ROM_END

/***************************************************************************

  Animal Catch ( VX2002L02 ANIMALCAT 200011211536 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  1 x MX29F1610ATC-12 (@U021)
  1 x MX29F1610TC-12  (@U016)

***************************************************************************/

ROM_START( animalc )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(84cf123b) SHA1(d8b425c93ff1a560e3f92c70d7eb93a05c3581af) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(4ae14ff9) SHA1(1273d15ea642452fecacff572655cd3ab47a5884) )   // 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( gunkids )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(3d989a45) SHA1(f4e1dc13bfe9ef8bf733735b6647946dda6962f2) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(5e356b68) SHA1(0e4e28b02dcb5ff7d2a7139c5cdf31cbd08167f4) )
ROM_END

ROM_START( wantouch ) // ワンタッチャブル
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(fe00ac87) SHA1(97c3defafd04f48984e5f2fe34528ada024355c5) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(cd8d4478) SHA1(224e8e0e9a49f10515e0ebf9a5e97d8adc7eed13) )
ROM_END

ROM_START( kotekita ) // こてき隊（たい）スロット
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(37faa4a9) SHA1(619135d74052fb2272ce78976c2aa59a65244f12) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(e471c1ef) SHA1(4ddf3261d8736888ebbdbd0d755aad75987b3b3d) )
ROM_END

void sammymdl_state::init_animalc()
{
	u8 *rom = memregion("mainbios")->base();

	// video timing loops
	rom[0x015d9] = 0x00;
	rom[0x015da] = 0x00;
	rom[0x01605] = 0x00;
	rom[0x01606] = 0x00;
	rom[0x01750] = 0x00;
	rom[0x01751] = 0x00;

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

/***************************************************************************

  Go Go Cowboy

  Cart:

    ガムボール     <- gumball
    with プライズ  <- with prize
    EM5701L01
    EM5701L01

  PCB:

    Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
    SC MAIN PCB VER2(GUM) 5049-6001-0 (Sticker)

***************************************************************************/

ROM_START( gocowboy )
	ROM_REGION( 0x80000, "mainbios", 0 )
	ROM_LOAD( "go_go_cowboy_gpt_2c9c.u3", 0x00000, 0x80000, CRC(ad9b1de6) SHA1(24809ec3a579d28189a98190db70a33217e4f8bc) ) // uses custom BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em702l01.u021", 0x000000, 0x200000, CRC(4c4289fe) SHA1(517b5a1e9d91e7ed322b4792d863e7abda835d4a) )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "em701l01.u016", 0x000000, 0x200000, CRC(c1f07320) SHA1(734717140e66ddcf0bded1489156c51cdaf1b50c) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "st93c46.u6", 0x00, 0x80, CRC(1af2d376) SHA1(afbe953f1a9ff0152fe1092a83482695dbe5e75d) )

	ROM_REGION( 0x5cde, "pld", 0 )
	ROM_LOAD( "vm1212f01.u5.jed", 0x0000, 0x5cde, CRC(b86a1825) SHA1(cc2e633fb8a24cfc93291a778b0964089f6b8ac7) )
ROM_END

/***************************************************************************

  Itazura Monkey ( VX1902L02 ITZRMONKY 200011211639 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( itazuram )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2002l01.u021", 0x00000, 0x200000, CRC(ddbdd2f3) SHA1(91f67a938929be0261442e066e3d2c03b5e9f06a) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx2001l01.u016", 0x00000, 0x200000, CRC(9ee95222) SHA1(7154d43ef312a48a882207ca37e1c61e8b215a9b) )
ROM_END

void sammymdl_state::init_itazuram()
{
	u8 *rom = memregion("mainbios")->base();

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

/***************************************************************************

  Taihou de Doboon ( EM4210L01 PUSHERDBN 200203151028 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Taihou de Doboon
  2003.02.14
  Char Rev: EM4209L01
  Pro  Rev: EM4210L01

***************************************************************************/

ROM_START( tdoboon )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em4210l01.u021", 0x00000, 0x200000, CRC(3523e314) SHA1(d07c5d17d3f285be4cde810547f427e84f98968f) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "em4209l01.u016", 0x00000, 0x200000, CRC(aca220fa) SHA1(7db441add16af554700e597fd9926b6ccd19d628) )   // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

/***************************************************************************

  Pye-nage Taikai ( VX1802L01 PAINAGETK 200011021216 SAMMY CORP. AM. )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( pyenaget )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx1802l01.u021", 0x00000, 0x200000, CRC(7a22a657) SHA1(2a98085862fd958209253c5401e41eae4f7c06ea) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx1801l01.u016", 0x00000, 0x200000, CRC(c4607403) SHA1(f4f4699442afccc5ed4354447f91b1bee36ae3e5) )
ROM_END

/***************************************************************************

  Hae Hae Ka Ka Ka ( EM4208L01 PUSHERHAEHAE 200203151032 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Hae Hae Ka Ka Ka
  2003.02.14
  Char Rev: EM4207L01
  Pro  Rev: EM4208L01

***************************************************************************/

ROM_START( haekaka )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em4208l01.u021", 0x00000, 0x200000, CRC(d23bb748) SHA1(38d5b6c4b2cd470b3a68574aeca3f9fa9032245e) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "em4207l01.u016", 0x00000, 0x200000, CRC(3876961c) SHA1(3d842c1f63ea5aa7e799967928b86c5fabb4e65e) )
ROM_END

void sammymdl_state::init_haekaka()
{
	u8 *rom = memregion("mainbios")->base();

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

ROM_START( kingyosu )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx1302l02.u021", 0x00000, 0x200000, CRC(45e43e3c) SHA1(71ada38c1f65738ca75ca75d2272f310ba43c72c) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "vx1301l01.u016", 0x00000, 0x200000, CRC(25dfbf4c) SHA1(8f9552cf54bdf7490055ebdb23fa758f5cddafb5) )
ROM_END

ROM_START( shatekds )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "29f016a.u015", 0x00000, 0x200000, CRC(7c779286) SHA1(db688ac2cad14c642e6da770212217a466e1bb64) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "mx29f1610tc.u016", 0x00000, 0x200000, CRC(6e7d2c7d) SHA1(747d54eee26631966a91fd26b8b4beb32b67ddb7) )
ROM_END

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1999, sammymdl, 0,        sammymdl, sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Sammy Medal Game System BIOS",         MACHINE_IS_BIOS_ROOT )
GAME( 1999, kingyosu, sammymdl, itazuram, sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Dokidoki Kingyo Sukui",                0 )
GAME( 1999, shatekds, sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Shatekids",                            0 )
GAME( 2000, animalc,  sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Animal Catch",                         0 )
GAME( 2000, itazuram, sammymdl, itazuram, sammymdl, sammymdl_state, init_itazuram, ROT0, "Sammy",             "Itazura Monkey",                       0 )
GAME( 2000, pyenaget, sammymdl, pyenaget, sammymdl, sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Pye-nage Taikai",                      0 )
GAME( 2000, tdoboon,  sammymdl, tdoboon,  haekaka,  sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Taihou de Doboon",                     0 )
GAME( 2001, haekaka,  sammymdl, haekaka,  haekaka,  sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Hae Hae Ka Ka Ka",                     0 )
GAME( 2001, kotekita, sammymdl, haekaka,  sammymdl, sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Kotekitai Slot",                       0 )
GAME( 2002, gunkids,  sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Hayauchi Gun Kids",                    0 )
GAME( 2003, wantouch, sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Wantouchable",                         0 )
GAME( 2003, gocowboy, 0,        gocowboy, gocowboy, sammymdl_state, empty_init,    ROT0, "Sammy",             "Go Go Cowboy (English, prize)",        0 )
