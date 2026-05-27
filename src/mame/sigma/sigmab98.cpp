// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                              -= Sigma B-98 Hardware =-

                                                 driver by Luca Elia

CPU     :   TAXAN KY-80 (Yamaha)
Video   :   TAXAN KY-3211 or KY-10510
Sound   :   YMZ280B or OKI M9811
NVRAM   :   93C46 and/or battery backed RAM

Graphics are made of sprites only.
Each sprite is composed of X x Y tiles and can be zoomed / shrunk and rotated.
Tiles can be 16x16x4 or 16x16x8.

Some videos:
https://www.youtube.com/channel/UCYFiPd3FukrmNJa3pfIKf-Q/search?query=sigma
https://www.youtube.com/channel/UCYFiPd3FukrmNJa3pfIKf-Q/search?query=sammy
https://www.youtube.com/user/analysis08/search?query=sammy
http://www.nicozon.net/watch/sm14334996

Dumped games:                           ROMs:    Video:

1997 Minna Atsumare! Dodge Hero         B9802   https://youtu.be/2eXDQnKCT6A
1997 Itazura Daisuki! Sushimaru Kun     B9803   https://youtu.be/nhvbZ71KWr8
1997 GeGeGe no Kitarou Youkai Slot      B9804
1997 Burning Sanrinsya                  B9805
1997 PEPSI Man                          B9806   https://youtu.be/p3cbZ67m4lo
1998 Transformers Beast Wars II         B9808
1997 Uchuu Tokkyuu Medalian             B9809   https://youtu.be/u813kBOZbwI
2000 Minna Ganbare! Dash Hero           B9811

2001 Otakara Itadaki Luffy Kaizoku-Dan! KA108   https://youtu.be/D_4bWx3tTPw

--------------------------------------------------------------------------------------

To Do:

- Remove ROM patches: gegege checks the EEPROM output after reset, and wants a timed 0->1 transition or locks up while
  saving setting in service mode. Using a reset_delay of 7 works, unless when "play style" is set
  to "coin" (it probably changes the number of reads from port $C0).
  I guess the reset_delay mechanism should be implemented with a timer in eepromser.cpp.
- dashhero does not acknowledge the button bashing correctly, it's very hard to win (a slower pace works better!)
- dodghero and sushimar often write zeroes to 81XX1 and 00XX1 for some reason (maybe just sloppy coding?)

*************************************************************************************************************/

#include "emu.h"

#include "cpu/z80/ky80.h"
#include "machine/74165.h"
#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim9810.h"
#include "sound/ymz280b.h"
#include "video/bufsprite.h"
#include "video/ky3211_ky10510.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sigmab98_state : public driver_device
{
public:
	sigmab98_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_hopper(*this, "hopper")
		, m_spritegen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_eeprom(*this, "eeprom")
		, m_buffered_spriteram(*this, "spriteram")
		, m_spriteram(*this, "spriteram")
		, m_leds(*this, "led%u", 0U)
		, m_vblank(0)
	{ }

	void sigmab98(machine_config &config) ATTR_COLD;
	void dodghero(machine_config &config) ATTR_COLD;

	void init_b3rinsya() ATTR_COLD;
	void init_tbeastw2() ATTR_COLD;
	void init_dashhero() ATTR_COLD;
	void init_gegege() ATTR_COLD;
	void init_pepsiman() ATTR_COLD;
	void init_ucytokyu() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD { m_leds.resolve(); }

	void c4_w(u8 data);
	void c6_w(u8 data);
	void c8_w(u8 data);

	void show_outputs();
	void eeprom_w(u8 data);

	// TODO: unify these handlers
	u8 d013_r();
	u8 d021_r();

	INTERRUPT_GEN_MEMBER(sigmab98_vblank_interrupt);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dodghero_mem_map(address_map &map) ATTR_COLD;
	void gegege_io_map(address_map &map) ATTR_COLD;
	void gegege_mem_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;

	// Required devices
	required_device<ky80_device> m_maincpu;
	required_device<ticket_dispenser_device> m_hopper;
	required_device<ky3211_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// Optional devices
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<buffered_spriteram8_device> m_buffered_spriteram;   // not on sammymdl?

	// Shared pointers
	required_shared_ptr<u8> m_spriteram;

	output_finder<4> m_leds;

	u8 m_c0 = 0;
	u8 m_c4 = 0;
	u8 m_c6 = 0;
	u8 m_c8 = 0;
	u8 m_vblank;
};


class lufykzku_state : public sigmab98_state
{
public:
	lufykzku_state(const machine_config &mconfig, device_type type, const char *tag)
		: sigmab98_state(mconfig, type, tag)
		, m_watchdog(*this, "watchdog_mb3773")
		, m_dsw_shifter(*this, "ttl165_%u", 1U)
		, m_dsw_bit(0)
	{
	}

	void lufykzku(machine_config &config) ATTR_COLD;
	void mnrockman(machine_config &config) ATTR_COLD;

	void init_lufykzku() ATTR_COLD;
	void init_mnrockman() ATTR_COLD;

private:
	required_device<mb3773_device> m_watchdog;
	required_device_array<ttl165_device, 2> m_dsw_shifter;

	void dsw_w(int state);

	void lufykzku_c4_w(u8 data);
	void lufykzku_c6_w(u8 data);
	u8 lufykzku_c8_r();
	void lufykzku_c8_w(u8 data);
	void lufykzku_watchdog_w(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(mnrockman_timer_irq);

	void lufykzku_io_map(address_map &map) ATTR_COLD;
	void lufykzku_mem_map(address_map &map) ATTR_COLD;

	u8 m_dsw_bit;
	u8 m_vblank_vector = 0;
	u8 m_timer0_vector = 0;
	u8 m_timer1_vector = 0;
};


/***************************************************************************

    Video

***************************************************************************/

u32 sigmab98_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
	u8* spriteram = (m_buffered_spriteram ? m_buffered_spriteram->buffer() : m_spriteram);
	m_spritegen->draw_sprites(bitmap, cliprect, spriteram, layers_ctrl & 8);
	m_spritegen->draw_sprites(bitmap, cliprect, spriteram, layers_ctrl & 4);
	m_spritegen->draw_sprites(bitmap, cliprect, spriteram, layers_ctrl & 2);
	m_spritegen->draw_sprites(bitmap, cliprect, spriteram, layers_ctrl & 1);

	return 0;
}

u8 sigmab98_state::d013_r()
{
	// bit 5 must go 0->1 (vblank?)
	// bit 2 must be set (sprite buffered? triggered by pulsing bit 3 of port C6?)
//  return (m_screen->vblank() ? 0x20 : 0x00) | 0x04;
	return (m_screen->vblank() ? 0x20 : 0x01) | 0x04;
//  return machine().rand();
}
u8 sigmab98_state::d021_r()
{
	// bit 5 must be 0?
	return 0;
//  return machine().rand();
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void sigmab98_state::video_map(address_map &map)
{
	map(0x80000, 0x80fff).ram().share("spriteram");

	map(0x82000, 0x821ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0x82800, 0x8287f).rw(m_spritegen, FUNC(ky3211_device::vtable_r), FUNC(ky3211_device::vtable_w));

	map(0x83000, 0x83021).rw(m_spritegen, FUNC(ky3211_device::vregs_r), FUNC(ky3211_device::vregs_w));
	map(0x83013, 0x83013).r(FUNC(sigmab98_state::d013_r));
	map(0x83021, 0x83021).r(FUNC(sigmab98_state::d021_r));
}

/***************************************************************************
                          Minna Atsumare! Dodge Hero
***************************************************************************/

void sigmab98_state::dodghero_mem_map(address_map &map)
{
	video_map(map);
	map(0x00000, 0x1ffff).rom().nopw();

	map(0x40000, 0x41fff).ram().share("nvram"); // battery backed RAM

	map(0x81000, 0x811ff).nopw();
}

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

// Outputs

void sigmab98_state::show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("0: %02X  4: %02X  6: %02X  8: %02X", m_c0, m_c4, m_c6, m_c8);
#endif
}

// Port c0
void sigmab98_state::eeprom_w(u8 data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 6));

	// reset line asserted: reset.
//  if ((m_c0 ^ data) & 0x20)
		m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 4) ? ASSERT_LINE : CLEAR_LINE);

	m_c0 = data;
	//show_outputs(state);
}

// Port c4
// 10 led?
void sigmab98_state::c4_w(u8 data)
{
	m_leds[0] = BIT(data, 4);

	m_c4 = data;
	show_outputs();
}

// Port c6
// 03 lockout (active low, 02 is cleared when reaching 99 credits)
// 04 pulsed on coin in
// 08 buffer sprites?
// 10 led?
// 20 led? (starts blinking after coin in)
void sigmab98_state::c6_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 1));

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	if ((data & 0x08) && !(m_c6 & 0x08))
		m_buffered_spriteram->copy();

	m_leds[1] = BIT(data, 4);
	m_leds[2] = BIT(data, 5);

	m_c6 = data;
	show_outputs();
}

// Port c8
// 01 hopper enable?
// 02 hopper motor on (active low)?
void sigmab98_state::c8_w(u8 data)
{
	m_hopper->motor_w((!(data & 0x02) && (data & 0x01)) ? 1 : 0);

	m_c8 = data;
	show_outputs();
}

void sigmab98_state::gegege_mem_map(address_map &map)
{
	video_map(map);
	map(0x00000, 0x1ffff).rom();

	map(0x40000, 0x47fff).ram().share("nvram"); // battery backed RAM
}

void sigmab98_state::gegege_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0xc0, 0xc0).portr("EEPROM").w(FUNC(sigmab98_state::eeprom_w));
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(sigmab98_state::c4_w));
	map(0xc6, 0xc6).w(FUNC(sigmab98_state::c6_w));
	map(0xc8, 0xc8).w(FUNC(sigmab98_state::c8_w));

	map(0xe5, 0xe5).nopr(); // during irq
}


/***************************************************************************
                       Otakara Itadaki Luffy Kaizoku-Dan!
***************************************************************************/

void lufykzku_state::dsw_w(int state)
{
	m_dsw_bit = state;
}

// Port c0
void lufykzku_state::lufykzku_watchdog_w(u8 data)
{
	m_watchdog->write_line_ck(BIT(data, 7));
}

// Port c4
void lufykzku_state::lufykzku_c4_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 5)); // 100 yen lockout
//  machine().bookkeeping().coin_lockout_w(2, BIT(~data, 6)); // (unused coin lockout)
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 7)); // medal lockout

	m_c4 = data;
	show_outputs();
}

// Port c6
void lufykzku_state::lufykzku_c6_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(1, BIT(data, 0)); // 100 yen in
//  machine().bookkeeping().coin_counter_w(2, BIT(data, 1)); // (unused coin in)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2)); // medal in
	machine().bookkeeping().coin_counter_w(3, BIT(data, 3)); // medal out
	m_leds[0] = BIT(data, 4); // button led
//  m_leds[1] = BIT(data, 5); // (unused button led)
//  m_leds[2] = BIT(data, 6); // (unused button led)
//  m_leds[3] = BIT(data, 7); // (unused button led)

	m_c6 = data;
	show_outputs();
}

// Port c8
u8 lufykzku_state::lufykzku_c8_r()
{
	return 0xbf | (m_dsw_bit ? 0x40 : 0);
}

void lufykzku_state::lufykzku_c8_w(u8 data)
{
	// bit 0? on payout button
	// bit 1? when ending payment
	m_hopper->motor_w(((data & 0x01) && !(data & 0x02)) ? 1 : 0);

	m_dsw_shifter[0]->shift_load_w(BIT(data, 4));
	m_dsw_shifter[1]->shift_load_w(BIT(data, 4));
	m_dsw_shifter[0]->clock_w(BIT(data, 5));
	m_dsw_shifter[1]->clock_w(BIT(data, 5));

	m_c8 = data;
	show_outputs();
}

void lufykzku_state::lufykzku_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x80000, 0x83fff).ram().share("nvram");

	map(0xc0000, 0xc0fff).ram().share("spriteram");
	map(0xc1000, 0xc2fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // more palette entries
	map(0xc3000, 0xc3021).rw(m_spritegen, FUNC(ky10510_device::vregs_r), FUNC(ky10510_device::vregs_w));
	map(0xc3013, 0xc3013).r(FUNC(lufykzku_state::d013_r));
	map(0xc3021, 0xc3021).r(FUNC(lufykzku_state::d021_r));

	map(0xc3400, 0xc347f).rw(m_spritegen, FUNC(ky10510_device::vtable_r), FUNC(ky10510_device::vtable_w));
}

void lufykzku_state::lufykzku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("oki", FUNC(okim9810_device::write));
	map(0x01, 0x01).w("oki", FUNC(okim9810_device::tmp_register_w));

	map(0xc0, 0xc0).portr("COIN").w(FUNC(lufykzku_state::lufykzku_watchdog_w)); // bit 7 -> watchdog
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(lufykzku_state::lufykzku_c4_w)); // bit 7 = medal lock, bit 6 = coin3, bit 5 = yen
	map(0xc6, 0xc6).w(FUNC(lufykzku_state::lufykzku_c6_w));
	map(0xc8, 0xc8).rw(FUNC(lufykzku_state::lufykzku_c8_r), FUNC(lufykzku_state::lufykzku_c8_w)); // 0xc8 bit 6 read (eeprom?)
}


/***************************************************************************

    Input Ports

***************************************************************************/

/***************************************************************************
                             Sigma B-98 Games
***************************************************************************/

// 1 button (plus bet and payout)
static INPUT_PORTS_START( sigma_1b )
	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM  )                       // Related to d013. Must be 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // Related to d013. Must be 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)   // ? (coin error, pulses mask 4 of port c6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME("Medal")    // coin/medal in (coin error)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_1)  // bet / select in test menu
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )  // pay out / change option in test menu
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

// 3 buttons (plus bet and payout)
static INPUT_PORTS_START( sigma_3b )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 )
INPUT_PORTS_END

// 5 buttons (plus bet and payout)
static INPUT_PORTS_START( sigma_5b )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 )

	PORT_MODIFY("PAYOUT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON5 )
INPUT_PORTS_END

// Joystick (plus bet and payout)
static INPUT_PORTS_START( sigma_js )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("PAYOUT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )
INPUT_PORTS_END


/***************************************************************************
                           Banpresto Medal Games
***************************************************************************/

static INPUT_PORTS_START( lufykzku )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1   ) PORT_IMPULSE(2) // medal in
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2   ) PORT_IMPULSE(2) // 100 yen in
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )                 // (unused coin in)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // sw1 (button)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE       ) // test sw
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1      ) // service sw (service coin, press to go down in service mode)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // payout sw
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_DOOR          ) // "maintenance panel abnormality" when not high on boot (in that case, when it goes high the game resets)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN       )

	// Dips read serially via port 0xc8 bit 6
	PORT_START("DSW1") // stored at fc14
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW1:1" )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW1:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW1:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "DSW1:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW1:6" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7,8") // Advertize Voice
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xc0, "On (1/3 Of The Loops)" )
	PORT_DIPSETTING(    0x80, "On (1/2 Of The Loops)" )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_START("DSW2") // stored at fc15
	PORT_DIPNAME( 0x07, 0x03, "Payout" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x08, "Win Wave" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big"   )
	PORT_DIPNAME( 0xf0, 0xa0, "¥100 Medals" ) PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0,  "5" )
	PORT_DIPSETTING(    0xe0,  "6" )
	PORT_DIPSETTING(    0xd0,  "7" )
	PORT_DIPSETTING(    0xc0,  "8" )
	PORT_DIPSETTING(    0xb0,  "9" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x90, "11" )
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x70, "13" )
	PORT_DIPSETTING(    0x60, "14" )
	PORT_DIPSETTING(    0x50, "15" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x30, "17" )
	PORT_DIPSETTING(    0x20, "18" )
	PORT_DIPSETTING(    0x10, "19" )
	PORT_DIPSETTING(    0x00, "20" )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

/***************************************************************************
                             Sigma B-98 Games
***************************************************************************/

INTERRUPT_GEN_MEMBER(sigmab98_state::sigmab98_vblank_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x5a); // Z80
}

void sigmab98_state::sigmab98(machine_config &config)
{
	KY80(config, m_maincpu, 20000000);  // !! TAXAN KY-80, clock @X1? !!
	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab98_state::gegege_mem_map);
	m_maincpu->set_addrmap(AS_IO, &sigmab98_state::gegege_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sigmab98_state::sigmab98_vblank_interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_93C46_16BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0,0x140-1, 0,0xf0-1);
	m_screen->set_screen_update(FUNC(sigmab98_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	BUFFERED_SPRITERAM8(config, m_buffered_spriteram);

	KY3211(config, m_spritegen, 0, m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));    // clock @X2?
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}

void sigmab98_state::dodghero(machine_config &config)
{
	sigmab98(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab98_state::dodghero_mem_map);
}


/***************************************************************************
                           Banpresto Medal Games
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_state::lufykzku_irq)
{
	const int scanline = param;

	if (scanline == 240)
	{
		if (m_vblank_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vblank_vector); // Z80
	}
	else if (scanline == 128)
	{
		if (m_timer0_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer0_vector); // Z80
	}
	else if ((scanline % 8) == 0)
	{
		if (m_timer1_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer1_vector); // Z80 - this needs to be called often or the state of the door is not read at boot (at least 5 times before bb9 is called)
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_state::mnrockman_timer_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xfc);
}

void lufykzku_state::lufykzku(machine_config &config)
{
	KY80(config, m_maincpu, XTAL(20'000'000));  // !! TAXAN KY-80, clock @X1? !!
	m_maincpu->set_addrmap(AS_PROGRAM, &lufykzku_state::lufykzku_mem_map);
	m_maincpu->set_addrmap(AS_IO, &lufykzku_state::lufykzku_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(lufykzku_state::lufykzku_irq), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM (TC55257DFL-70L)
	// No EEPROM

	MB3773(config, m_watchdog, 0);
	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// 2 x 8-bit parallel/serial converters
	TTL165(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set_ioport("DSW2");
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl165_device::serial_w));

	TTL165(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set_ioport("DSW1");
	m_dsw_shifter[1]->qh_callback().set(FUNC(lufykzku_state::dsw_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0,0x140-1, 0,0xf0-1);
	m_screen->set_screen_update(FUNC(lufykzku_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // same as sammymdl?
	KY10510(config, m_spritegen, 0, m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "speaker", 0.80, 0);
	oki.add_route(1, "speaker", 0.80, 1);
}

void lufykzku_state::mnrockman(machine_config& config)
{
	lufykzku(config);
	TIMER(config, "unktimer").configure_periodic(FUNC(lufykzku_state::mnrockman_timer_irq), attotime::from_hz(100));
}

/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

  Minna Atsumare! Dodge Hero

***************************************************************************/

ROM_START( dodghero )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9802-1d.ic7", 0x00000, 0x20000, CRC(093492e3) SHA1(d4dd104dc2410d97add532eea031cbc1ede3b0b1) )

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "b9802-2.ic12",  0x00000, 0x80000, CRC(bb810ab8) SHA1(02bb1bb9b6dd0d24401c8a8c579f5ebcba963d8f) )
	ROM_LOAD( "b9802-3a.ic13", 0x80000, 0x80000, CRC(8792e487) SHA1(c5ed8059cd40a00656016b33762a04b9bedd7f06) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9802-5.ic16", 0x00000, 0x80000, CRC(4840bdbd) SHA1(82f286ef848df9b6dcb5ff3b3aaa11d8e93e995b) )
	ROM_LOAD( "b9802-6.ic26", 0x80000, 0x80000, CRC(d83d8537) SHA1(9a5afdc68417db828a09188d653552452930b136) )
ROM_END

/***************************************************************************

  Itazura Daisuki! Sushimaru Kun

***************************************************************************/

ROM_START( sushimar )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9803-1c.ic7", 0x00000, 0x20000, CRC(8ad3b7be) SHA1(14d8cec6723f230d4167de91b5b1103fe40755bc) )

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "b9803-2.ic12",  0x00000, 0x80000, CRC(cae710a4) SHA1(c0511412d8feaa032b8bcd72074522d1b90f22b2) )
	ROM_LOAD( "b9803-03.ic13", 0x80000, 0x80000, CRC(f69f37f6) SHA1(546045b50dbc3ef45fc4dd1c7f2f6a23dfdc53d8) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9803-5a.ic16", 0x00000, 0x80000, CRC(da3f36aa) SHA1(0caffbe6726afd41763f25378f8820724aa7bbce) )
ROM_END

/***************************************************************************

  GeGeGe no Kitarou Youkai Slot

  (C) 1997 Banpresto, Sigma

  PCB:

    (c) 1997 Sigma B-98-1 MAIN PCB
    970703 (Sticker)

  CPU:

    TAXAN KY-80 YAMAHA 9650 AZGC (@IC1)
    XTAL ?? (@X1)

  Video:

    TAXAN KY-3211 9722 AZGC (@IC11)
    XTAL 27.000 MHz (@XOSC1)
    M548262-60 (@IC24) - 262144-Word x 8-Bit Multiport DRAM

  Sound:

    YAMAHA YMZ280B-F (@IC14)
    XTAL ?? (@X2)
    Trimmer

  Other:

    93C46AN EEPROM (@IC5)
    MAX232CPE (@IC6)
    Battery (@BAT)

***************************************************************************/

ROM_START( gegege )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9804-1.ic7", 0x00000, 0x20000, CRC(f8b4f855) SHA1(598bd9f91123e9ab539ce3f33779bff2d072e731) )

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "b9804-2.ic12", 0x00000, 0x80000, CRC(4211079d) SHA1(d601c623fb909f1346fd02b8fb37b67956e2cd4e) )
	ROM_LOAD( "b9804-3.ic13", 0x80000, 0x80000, CRC(54aeb2aa) SHA1(ccf939111f6288a889846d51bab47ff4e992c542) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9804-5.ic16", 0x00000, 0x80000, CRC(ddd7984c) SHA1(3558c495776671ffd3cd5c665b87827b3959b360) )
ROM_END

void sigmab98_state::init_gegege()
{
	u8 *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x0bdd] = 0xc9;

//  rom[0x0bf9] = 0xc9;

//  rom[0x0dec] = 0x00;
//  rom[0x0ded] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Burning Sanrinsya - Burning Tricycle

***************************************************************************/

ROM_START( b3rinsya )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9805-1c.ic7", 0x00000, 0x20000, CRC(a8cde2f4) SHA1(74d1f3f1710084d788a71dec0366f2c3f756fdf8) )

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "b9805-2.ic12", 0x00000, 0x80000, CRC(7ec2e957) SHA1(1eb9095663d4f8f8f0c77f151918af1978332b3d) )
	ROM_LOAD( "b9805-3.ic13", 0x80000, 0x80000, CRC(449d0848) SHA1(63e91e4be8b58a6ebf1777ed5a9c23416bacba48) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9805-5.ic16", 0x00000, 0x80000, CRC(f686f886) SHA1(ab68d12c5cb3a9fbc8a178739f39a2ff3104a0a1) )
ROM_END

void sigmab98_state::init_b3rinsya()
{
	u8 *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  PEPSI Man

***************************************************************************/

ROM_START( pepsiman )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9806-1a.ic7", 0x00000, 0x20000, CRC(3152fe90) SHA1(98a8ae1bd3a4381cec11ba8b3e9cdad71c7bd05a) )

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "b9806-2.ic12", 0x00000, 0x80000, CRC(82f650ea) SHA1(c0b214fdc39329e2136707bc195d470d4b613509) )
	ROM_LOAD( "b9806-3.ic13", 0x80000, 0x80000, CRC(07dc548e) SHA1(9419c0cac289a9894cce1a10924f40e146e2ff8a) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9806-5.ic16", 0x00000, 0x80000, CRC(6d405dfb) SHA1(e65ffe1279680097894754e379d7ad638657eb49) )
ROM_END

void sigmab98_state::init_pepsiman()
{
	u8 *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x058a] = 0xc9;

//  rom[0x05a6] = 0xc9;

//  rom[0xa00e] = 0x00;
//  rom[0xa00f] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Transformers Beast Wars II

***************************************************************************/

ROM_START( tbeastw2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9808-1b.ic7", 0x00000, 0x20000, CRC(65f7e079) SHA1(d421da3c99d62d3228e1b9c1cfb2de51f0fcc56e) )

	ROM_REGION( 0x180000, "spritegen", 0 )
	ROM_LOAD( "b9808-2.ic12", 0x000000, 0x80000, CRC(dda5c2d2) SHA1(1bb21e7251df93b0f502b716e958d81f4e4e46dd) )
	ROM_LOAD( "b9808-3.ic13", 0x080000, 0x80000, CRC(80df49c6) SHA1(14342be3a176cdf015c0ac07a4f1c109862c67aa) )
	ROM_LOAD( "b9808-4.ic17", 0x100000, 0x80000, CRC(d90961ea) SHA1(c2f226a528238eafc1ba37200da4ee6ce9b54325) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9808-5.ic16", 0x00000, 0x80000, CRC(762c6d5f) SHA1(0d4e35b7f346c8cc0c49163474f34c1fc462998a) )
	ROM_LOAD( "b9808-6.ic26", 0x80000, 0x80000, CRC(9ed759c9) SHA1(963db80b8a107ce9292bbc776ba91bc76ad82d5b) )
ROM_END

void sigmab98_state::init_tbeastw2()
{
	u8 *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Uchuu Tokkyuu Medalian

***************************************************************************/

ROM_START( ucytokyu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9809-1.ic7", 0x00000, 0x20000, CRC(5be6adff) SHA1(7248157111be2ae23df7d51f5d071cc3b9fd79b4) )

	ROM_REGION( 0x180000, "spritegen", 0 )
	ROM_LOAD( "b9809-2.ic12", 0x000000, 0x80000, CRC(18f342b3) SHA1(09d62bb3597259e0fbae2c0f4ed163685a4a9dd9) )
	ROM_LOAD( "b9809-3.ic13", 0x080000, 0x80000, CRC(88a2a52a) SHA1(0dd10d4fa88d1a54150729026495a70dbe67bae0) )
	ROM_LOAD( "b9809-4.ic17", 0x100000, 0x80000, CRC(ea74eacd) SHA1(279fa1d2bc7bfafbafecd0e0758a47345ca95140) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9809-5.ic16", 0x00000, 0x80000, CRC(470006e6) SHA1(34c82fae7364eb5288de5c8128d72d7e5772c526) )
	ROM_LOAD( "b9809-6.ic26", 0x80000, 0x80000, CRC(4e2d5fdf) SHA1(af1357b0f6a407890ecad26a18d2b4e223802693) )
ROM_END

void sigmab98_state::init_ucytokyu()
{
	u8 *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x0bfa] = 0xc9;

//  rom[0x0c16] = 0xc9;

//  rom[0xa43a] = 0x00;
//  rom[0xa43b] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Minna Ganbare! Dash Hero

***************************************************************************/

ROM_START( dashhero )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b098111-0101.ic7", 0x00000, 0x20000, CRC(46488393) SHA1(898bafbf5273b368cf963d863fb93e9fa0da816f) )

	ROM_REGION( 0x180000, "spritegen", 0 )
	ROM_LOAD( "b98114-0100.ic12", 0x000000, 0x80000, CRC(067625ea) SHA1(f9dccfc85adbb840da7512db0c88f554b453d2d2) )
	ROM_LOAD( "b98115-0100.ic13", 0x080000, 0x80000, CRC(d6f0b89d) SHA1(33b5f2f6529fd9a145ccb1b4deffabf5fa0d46cb) )
	ROM_LOAD( "b98116-0100.ic17", 0x100000, 0x80000, CRC(c0dbe953) SHA1(a75e202a0c1be988b8fd7d4ee23ebc82f6110e5f) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b098112-0100.ic16", 0x00000, 0x80000, CRC(26e5d6f5) SHA1(6fe6a26e51097886db58a6619b12a73cd21e7130) )
ROM_END

void sigmab98_state::init_dashhero()
{
	u8 *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8162] = 0x00;
	rom[0x8163] = 0x00;
}


/***************************************************************************

  お宝いただき ルフィ海賊団! (Otakara Itadaki Luffy Kaizoku-Dan!)
  "From TV animation One Piece"
  (C) Eiichiro Oda / Shueisha - Fuji TV - Toho Animation
  (C) Banpresto 2001 Made In Japan

  PCB: BPSC-2001M-A

  Chips:
    TAXAN KY-80 YAMAHA  :  CPU
    TAXAN KY-10510      :  Graphics, slightly different sprite format than KY-3211
    OKI M9811           :  Sound, 4-channel version of OKI M9810
    MB3773 (@IC22?)     :  Power Supply Monitor with Watch Dog Timer and Reset (SOIC8)

  Misc:
    2 x DSW8 (@DSW1-2, KSD08S)
    2 x 74HC165A (8-bit parallel-in/serial out shift register)
    Cell battery (@BT1)
    Potentiometer (@VR1)
    Connectors: CN1 (52? pins), CN2 (10 pins), CN3 (6 pins), CN4 (5 pins)
    XTALs: 25.000 MHz (@OSC1?), 20.00 MHz (@X1), 15.00 MHz (@X2), 4.096 MHz (@X3)
    Jumpers: ホッパー        JP1 / JP2  =  12V / 24V     (hopper voltage)
             ウォッチドック  JP3        =                (watchdog enable?)
             PRG ロム       JP4 / JP5  =  4M? / 1 or 2M (rom size)

  Eproms:
    IC1 ST 27C1001
    IC2 ST 27C4001
    IC3 ST 27C800

***************************************************************************/

ROM_START( lufykzku )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ka-102_p1__ver1.02.ic1", 0x000000, 0x020000, CRC(5d4f3405) SHA1(e2d982465ba847e9883dbb1c9a95c24eeae4611d) ) // hand-written label, 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "spritegen", 0 )
	ROM_LOAD( "ka-102_g1__ver1.00.ic3", 0x000000, 0x100000, CRC(abaca89d) SHA1(ad42eb8536373fb4fcff8f04ffe9b67dc62e0423) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "ka-102_s1__ver1.00.ic2", 0x000000, 0x080000, CRC(65f800d5) SHA1(03afe2f7a0731e7c3bc7c86e1a0dcaea0796e87f) )
ROM_END

void lufykzku_state::init_lufykzku()
{
	m_vblank_vector = 0xfa; // nop
	m_timer0_vector = 0xfc; // write coin counters/lockout, drive hopper
	m_timer1_vector = 0xfe; // read inputs and hopper sensor, handle coin in
}

void lufykzku_state::init_mnrockman()
{
	m_vblank_vector = 0xfa; // nop
	m_timer0_vector = 0;
	m_timer1_vector = 0xfe;
}

// Banpresto BPSC-2001M-A PCB, same as lufykzku
ROM_START( mnrockman )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ka-108_p1_ver1.02.ic1", 0x000000, 0x020000, CRC(727edf2f) SHA1(51a5f89a9ba64e16a1f46cc1145efa792ebb6401) )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD( "ka-108_g1_ver1.00.ic3", 0x000000, 0x200000, CRC(ef79a6de) SHA1(50cbe7665e80b58a6bb0b20bae2deeca2e29c9da) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "ka-108_s1_ver1.00.ic2", 0x000000, 0x080000, CRC(828dd3bd) SHA1(9788a30199d81f6db54f5409fcb146098a29e6aa) )
ROM_END

}

/***************************************************************************

    Game Drivers

***************************************************************************/

// Sigma Medal Games
GAME( 1997, dodghero, 0,        dodghero,  sigma_1b, sigmab98_state, empty_init,     ROT0, "Sigma",             "Minna Atsumare! Dodge Hero",           MACHINE_IMPERFECT_TIMING ) // Incorrect speed in title screen, etc.
GAME( 1997, sushimar, 0,        dodghero,  sigma_3b, sigmab98_state, empty_init,     ROT0, "Sigma",             "Itazura Daisuki! Sushimaru Kun",       0 )
GAME( 1997, gegege,   0,        sigmab98,  sigma_1b, sigmab98_state, init_gegege,    ROT0, "Sigma / Banpresto", "GeGeGe no Kitarou Youkai Slot",        0 )
GAME( 1997, b3rinsya, 0,        sigmab98,  sigma_5b, sigmab98_state, init_b3rinsya,  ROT0, "Sigma",             "Burning Sanrinsya - Burning Tricycle", 0 ) // 1997 in the rom
GAME( 1997, pepsiman, 0,        sigmab98,  sigma_3b, sigmab98_state, init_pepsiman,  ROT0, "Sigma",             "PEPSI Man",                            0 )
GAME( 1998, tbeastw2, 0,        sigmab98,  sigma_3b, sigmab98_state, init_tbeastw2,  ROT0, "Sigma / Transformer Production Company / Takara", "Transformers Beast Wars II", 0 ) // 1997 in the rom
GAME( 1997, ucytokyu, 0,        sigmab98,  sigma_js, sigmab98_state, init_ucytokyu,  ROT0, "Sigma",             "Uchuu Tokkyuu Medalian",               0 ) // Banpresto + others in the ROM
GAME( 2000, dashhero, 0,        sigmab98,  sigma_1b, sigmab98_state, init_dashhero,  ROT0, "Sigma",             "Minna Ganbare! Dash Hero",             MACHINE_NOT_WORKING ) // 1999 in the rom
// Banpresto Medal Games
GAME( 2001, lufykzku, 0,        lufykzku,  lufykzku, lufykzku_state, init_lufykzku,  ROT0, "Banpresto / Eiichiro Oda / Shueisha - Fuji TV - Toho Animation", "Otakara Itadaki Luffy Kaizoku-Dan! (Japan, v1.02)", 0 )
GAME( 2002, mnrockman,0,        mnrockman, lufykzku, lufykzku_state, init_mnrockman, ROT0, "Banpresto / Capcom / Shogakukan / ShoPro / TV Tokyo", "Medal Network: Rockman EXE", 0 )
