// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/****************************************************************************************

  GI (General Instrument) dual 6809 hardware.
  Driver by Roberto Fresca & Grull Osgo.

  Games running on this hardware:

  * Cast Away (dual 6809 GI Australasia PCB),      1987, Tranex Australia Pty Ltd.
  * Jester Chance (dual 6809 GI Australasia PCB),  1987, Tranex Australia Pty Ltd.
  * Good Luck! (dual 6809 GI Australasia PCB),     198?, unknown.


*****************************************************************************************

  Hardware Notes:

  Specs:

  PCB etched:
  GENERAL INSTRUMENT
  AUSTRALASIA PTY LTD.

  2x Hitachi HD6809EP CPUs.
  2x Hitachi HD46821P PIAs.
  1x Hitachi HD46505SP CRTC.

  1x 2732 ROM (charset) @ U5.
  3x 2732 ROM (tileset) @ U2, U3, U4.

  2x 74S287N Bipolar PROM (256 Bit x 4, 3-state) Cross reference: 82S129, @ U31 & U35.

  1x Xtal 10.000 MHz.

  -----------------------------------------------------------

  This hardware is composed of two 6809 microprocessors.
  One manages the game. The second manages the Accounting System.

  Both processors share RAM memory through an unknown method.
  This hardware has a sophisticated and unknown design to manage processor's interrupts via PIAs 6821.
  Coin-In system has two sensors and a complex way to validate via IRQs system.
  glk6809 has different hardware from castawayt and jesterch.
  glk6809 has an unknown device at 4000h that provides a Serial Number.


  * Game glck6809:

  Payout is enabled with at least 30 Credits and it's only Hand Paid.
  Game has a lot of Statistics but a it hasn't any Test Mode.
  Game hasn't any active lamp for user buttons.
  Game hasn't hopper device.
  Game has a lack of sound effects (Bet action, Hold Action, Winning action, Prize pay action , etc).
  Game has a lack of gaming stages messages.
  Game hasn't DSW Settings or Menu Driven Settings. (Pay tables, Jack or Better, Auto Hold, etc).


  * Game Jester Chance:

  Incomplete due to a lack of hardware info and a pre-loaded NVRAM.
  System runs partially.
  Driver hasn't Coin-In system hooked up.
  Unknown NMI source on Master CPU.


****************************************************************************************

  To Do:
  ------

  - Discover the proper way to tie PIA's signals to Microprocessor's IRQs.
  - Coin In device support.


****************************************************************************************/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/netlist.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "video/mc6845.h"

#include "nl_gi6809.h"

namespace {

#define MASTER_CLOCK    XTAL(10'000'000)
#define CPU_CLOCK_GL    (MASTER_CLOCK / 16)
#define CPU_CLOCK       (MASTER_CLOCK / 2)
#define PIXEL_CLOCK     (MASTER_CLOCK / 16)

class gi6809_state : public driver_device
{
public:
	gi6809_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slavecpu"),
		m_crtc(*this, "crtc"),
		m_pia(*this, "pia%u", 0U),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_input(*this, "IN0-%u", 0U),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U),
		m_sound_bit0(*this, "sound_nl:bit0"),
		m_sound_bit1(*this, "sound_nl:bit1"),
		m_sound_bit2(*this, "sound_nl:bit2"),
		m_sound_bit3(*this, "sound_nl:bit3")
	{ }

	void castawayt(machine_config &config);
	void glck6809(machine_config &config);
	void jesterch(machine_config &config);
	void init_cast();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void gi6809_base(machine_config &config);

private:

	// Address Maps
	void glckmain_map(address_map &map) ATTR_COLD;
	void glckslave_map(address_map &map) ATTR_COLD;
	void castmain_map(address_map &map) ATTR_COLD;
	void castslave_map(address_map &map) ATTR_COLD;
	void jestmain_map(address_map &map) ATTR_COLD;
	void jestslave_map(address_map &map) ATTR_COLD;

	// Devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slavecpu;
	required_device<mc6845_device> m_crtc;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_ioport_array<4> m_input;
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<8> m_lamps;

	// Audio triggers
	required_device<netlist_mame_logic_input_device> m_sound_bit0;
	required_device<netlist_mame_logic_input_device> m_sound_bit1;
	required_device<netlist_mame_logic_input_device> m_sound_bit2;
	required_device<netlist_mame_logic_input_device> m_sound_bit3;

	// Video Hardware
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void gi6809_videoram_w(offs_t offset, u8 data);
	void gi6809_colorram_w(offs_t offset, u8 data);
	void gi6809_palette(palette_device &palette) const;
	uint32_t screen_update_gi6809(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// Input ports
	u8 gi6809_mux_port_r();

	// Output ports
	void lamps3_w(u8 data);
	void lamps3h_w(u8 data);
	void lamps5_w(u8 data);
	void lamps8_w(u8 data);
	void snd_mux_w(u8 data);

	// internal
	u8 sn_read1_r() { return m_sernum1[(m_ser_ptr++) & 0x0f]; }
	u8 sn_read2_r() { return m_sernum2[(m_ser_ptr++) & 0x0f]; }
	u8 cast_sens_r() { return 0xff; }

	// vars
	tilemap_t *m_bg_tilemap;
	u8 m_mux_data = 0xff;
	u8 m_ser_ptr = 0;
	u8 m_sernum1[0x10] = { 3, 0, 3, 4, 4,  1, 4, 1, 0, 0, 0, 0, 0, 0, 2, 6 };
	u8 m_sernum2[0x10] = { 4, 7, 4, 9, 2, 13, 4, 3, 0, 0, 0, 0, 0, 0, 0, 1 };
};


/*********************************************
*           Memory Map Information           *
*********************************************/

void gi6809_state::glckmain_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("ram");
	map(0x1000, 0x13ff).ram().w(FUNC(gi6809_state::gi6809_videoram_w)).share("videoram");
	map(0x2000, 0x23ff).ram().w(FUNC(gi6809_state::gi6809_colorram_w)).share("colorram");
	map(0x3000, 0x3000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x3001, 0x3001).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4003).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
}

void gi6809_state::glckslave_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("ram");
	map(0x1004, 0x1007).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x400f).nopw().r(FUNC(gi6809_state::sn_read1_r));  // unknown device
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0xe000, 0xffff).rom();
}

void gi6809_state::castmain_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("ram");
	map(0x0100, 0x07ff).ram();
	map(0x1000, 0x13ff).ram().w(FUNC(gi6809_state::gi6809_videoram_w)).share("videoram");
	map(0x2000, 0x23ff).ram().w(FUNC(gi6809_state::gi6809_colorram_w)).share("colorram");
	map(0x3000, 0x3000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x3001, 0x3001).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4003).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0x7000, 0x7000).r(FUNC(gi6809_state::cast_sens_r));
	map(0x8000, 0xffff).rom();
}

void gi6809_state::castslave_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("ram");
	map(0x0100, 0x07ff).ram();
	map(0x1004, 0x1007).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x400f).noprw();
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0xe000, 0xffff).rom();
}

void gi6809_state::jestmain_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("ram");
	map(0x0100, 0x07ff).ram();
	map(0x1000, 0x13ff).ram().w(FUNC(gi6809_state::gi6809_videoram_w)).share("videoram");
	map(0x2000, 0x23ff).ram().w(FUNC(gi6809_state::gi6809_colorram_w)).share("colorram");
	map(0x3000, 0x3000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x3001, 0x3001).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4003).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5000, 0x5000).portr("SENSB").w(FUNC(gi6809_state::lamps8_w));
	map(0x5001, 0x5003).noprw();  // unknown device
	map(0x8000, 0xffff).rom();
}

void gi6809_state::jestslave_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("ram");
	map(0x0100, 0x07ff).ram();
	map(0x1004, 0x1007).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x400f).nopw().r(FUNC(gi6809_state::sn_read2_r));  // unknown device
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0xe000, 0xffff).rom();
}


/*******************************************
*               Write Handlers             *
*******************************************/

void gi6809_state::lamps3_w(u8 data)
{
	// glck6809 - no lamps activity

	m_lamps[5] = BIT(data, 5);  // lamp 5 Coin lockout
	m_lamps[6] = BIT(data, 6);  // lamp 6 Hopper Motor
	m_lamps[7] = BIT(data, 7);  // lamp 7 Attendant Lamp
}

void gi6809_state::lamps3h_w(uint8_t data)
{
	m_lamps[5] = BIT(data, 5);  // lamp 5 Coin lockout
	m_lamps[6] = BIT(data, 6);  // lamp 6 Hopper Motor
	m_lamps[7] = BIT(data, 7);  // lamp 7 Attendant Lamp
	m_hopper->motor_w(!BIT(data, 6));
}

void gi6809_state::lamps5_w(u8 data)
{
	// glck6809 - no lamps activity

	m_lamps[0] = BIT(data, 0);
	m_lamps[1] = BIT(data, 1);
	m_lamps[2] = BIT(data, 2);
	m_lamps[3] = BIT(data, 3);
	m_lamps[4] = BIT(data, 4);
}

void gi6809_state::lamps8_w(u8 data)
{
	m_lamps[0] = BIT(data, 0);
	m_lamps[1] = BIT(data, 1);
	m_lamps[2] = BIT(data, 2);
	m_lamps[3] = BIT(data, 3);
	m_lamps[4] = BIT(data, 4);
	m_lamps[5] = BIT(data, 5);
	m_lamps[6] = BIT(data, 6);
	m_lamps[7] = BIT(data, 7);
}


/*******************************************
*               Read Handlers              *
*******************************************/

u8 gi6809_state::gi6809_mux_port_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 4 ; i++)
		if (BIT(~m_mux_data, i + 4))
			data &= m_input[i]->read();
	return data;
}

void gi6809_state::snd_mux_w(u8 data)
{
	m_mux_data = data;
	m_sound_bit0->write(BIT(data,0));
	m_sound_bit1->write(BIT(data,1));
	m_sound_bit2->write(BIT(data,2));
	m_sound_bit3->write(BIT(data,3));
	logerror("Sound Code:%02x\n", data & 0x0f);
}


/*********************************************
*               Video Hardware               *
*********************************************/

void gi6809_state::gi6809_videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gi6809_state::gi6809_colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t gi6809_state::screen_update_gi6809(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(gi6809_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    ---- ---x   tiles extended address (MSB).
    xx-- ----   unused.
*/

	int attr = m_colorram[tile_index];
	int code = ((attr & 1) << 8) | m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;      // bit 1 switch the gfx banks
	int color = (attr & 0x3c) >> 2;     // bits 2-3-4-5 for color

	tileinfo.set(bank, code, color, 0);
}

void gi6809_state::gi6809_palette(palette_device &palette) const
{
/*
    This hardware has a feature called BLUE KILLER.
    Using the original intensity line, the PCB has a bridge
    that allows (as default) to turn the background black.

    All games running on this hardware
    were designed with black background.

    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   blue killer.
    xxxx ----   unused.
*/

	// 0000KBGR
	u8 const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		// blue killer (from schematics)
		int const bk = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 0) * 0xff;

		// green component
		int const g = BIT(color_prom[i], 1) * 0xff;

		// blue component
		int const b = bk * BIT(color_prom[i], 2) * 0xff;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*********************************************
*                Input Ports                 *
*********************************************/

/* Notes about inputs duplication...

   Even when there are duplications (holds buttons, services, and others,
   they have different behaviours/fuctions and should be named accordingly.
   If we factor them out, just will need to dupe the common ones due to
   the above explanation, and all the scheme will looks odd and confusing.
   Will see later when we have all the games working, if could do something
   about.
   Cleaned some duplications using port modify.
*/

static INPUT_PORTS_START( glck6809 )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )     PORT_NAME("Supervisor Key")  PORT_TOGGLE  // Full Menu.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )     PORT_NAME("Attendant Key")   PORT_TOGGLE  // Partial Menu.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve Machine") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_NAME("Door - Att Menu") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("D-UP - Menu Back")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )  // Coin Upper Sensor - to be implemented
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )  // Coin Lower Sensor - to be implemented
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big - Menu Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small - Menu Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )  PORT_NAME("Take - Menu Set")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PIA0_A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Meter Sensor 1 (PIA0_A 01)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Meter Sensor 2 (PIA0_A 02)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Meter Sensor 3 (PIA0_A 04)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Meter Sensor 4 (PIA0_A 08)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Meter Sensor 5 (PIA0_A 10)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


static INPUT_PORTS_START( castawayt )

	PORT_INCLUDE( glck6809 )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )      PORT_NAME("Accountancy Key")  PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )         PORT_NAME("Reserve / Test Menu (Accountancy) / Back (Test Menu)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )  PORT_NAME("Door - Test Menu")  PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )   PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("Hopper Weight")   PORT_CODE(KEYCODE_T)

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Collect / Payout / Select (Test Menu)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("1 Coin Start / Up (Test Menu)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("2 Coin Start / Down (Test Menu)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("5 Coin Start")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("4 Coin Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("3 Coin Start")

INPUT_PORTS_END


static INPUT_PORTS_START( jesterch )

	PORT_INCLUDE( glck6809 )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("Reserve / Test Menu (Accountancy) / Back (Test Menu)") PORT_CODE(KEYCODE_8)

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Spin / Deal / Play")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper Out")  PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper Weight")  PORT_CODE(KEYCODE_T)

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Collect / Payout / Select (Test Menu)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("1 Coin Start / Up (Test Menu)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("2 Coin Start / Down (Test Menu)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("5 Coin Start")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("4 Coin Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("3 Coin Start")

	PORT_START("SENSB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Coin Mech")       PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Hopper Sensor")   PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Cashbox Switch")  PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_gi6809 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
GFXDECODE_END



/*********************************************
*              Machine Start                 *
*********************************************/

void gi6809_state::machine_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gi6809_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_lamps.resolve();
}

/*********************************************
*              Machine Reset                 *
*********************************************/

void gi6809_state::machine_reset()
{
	m_ser_ptr = 0;
}


/*********************************************
*              Machine Config                *
*********************************************/

void gi6809_state::gi6809_base(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(gi6809_state::screen_update_gi6809));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_gi6809);
	PALETTE(config, "palette", FUNC(gi6809_state::gi6809_palette), 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(gi6809))
		.add_route(ALL_OUTPUTS, "mono", 1.0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:bit0", "PA0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:bit1", "PA1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:bit2", "PA2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:bit3", "PA3.IN", 0);
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(1.0, 0.0);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(50));
}


void gi6809_state::glck6809(machine_config &config)
{
	gi6809_base(config);

	MC6809E(config, m_maincpu, CPU_CLOCK_GL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gi6809_state::glckmain_map);

	MC6809E(config, m_slavecpu, CPU_CLOCK_GL);
	m_slavecpu->set_addrmap(AS_PROGRAM, &gi6809_state::glckslave_map);

	MC6845(config, m_crtc, PIXEL_CLOCK);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	PIA6821(config, m_pia[0], 0);  // controlled by slave
	m_pia[0]->readpa_handler().set_ioport("PIA0_A");
	m_pia[0]->writepb_handler().set(FUNC(gi6809_state::lamps5_w));
	m_pia[0]->ca2_handler().set_nop();
	m_pia[0]->cb2_handler().set_nop();

	PIA6821(config, m_pia[1], 0);  // controlled by master
	m_pia[1]->writepa_handler().set(FUNC(gi6809_state::snd_mux_w));
	m_pia[1]->readpb_handler().set(FUNC(gi6809_state::gi6809_mux_port_r));
	m_pia[1]->writepb_handler().set(FUNC(gi6809_state::lamps3_w));
	m_pia[1]->readcb1_handler().set(m_crtc, FUNC(mc6845_device::vsync_r));
	m_pia[1]->irqb_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_pia[1]->ca2_handler().set_nop();
	m_pia[1]->cb2_handler().set_nop();
}


void gi6809_state::castawayt(machine_config &config)
{
	gi6809_base(config);

	MC6809(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gi6809_state::castmain_map);

	MC6809(config, m_slavecpu, CPU_CLOCK);
	m_slavecpu->set_addrmap(AS_PROGRAM, &gi6809_state::castslave_map);

	MC6845(config, m_crtc, PIXEL_CLOCK);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_crtc->out_vsync_callback().append([this](u8 state) { m_pia[1]->read(1); });

	PIA6821(config, m_pia[0], 0);  // DDRA:00 (All In) - DDRB:FF (All Out)
	m_pia[0]->readpa_handler().set_ioport("PIA0_A");
	m_pia[0]->writepb_handler().set(FUNC(gi6809_state::lamps8_w));
	m_pia[0]->readca1_handler().set(m_crtc, FUNC(mc6845_device::vsync_r));
	m_pia[0]->ca2_handler().set_nop();

	PIA6821(config, m_pia[1], 0);  // DDRA:FF (All Out) - DDRB:EO (OOOI-IIII)
	m_pia[1]->writepa_handler().set(FUNC(gi6809_state::snd_mux_w));
	m_pia[1]->readpb_handler().set(FUNC(gi6809_state::gi6809_mux_port_r));
	m_pia[1]->writepb_handler().set(FUNC(gi6809_state::lamps3h_w));

	//m_pia[1]->readca1_handler() coin in upper opto to be implemented
	//m_pia[1]->readca2_handler() coin in lower opto to be implemented

	m_pia[1]->readcb1_handler().set(m_crtc, FUNC(mc6845_device::vsync_r));
	m_pia[1]->cb2_handler().set_nop();
	m_pia[1]->irqa_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	m_pia[1]->irqb_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
}


void gi6809_state::jesterch(machine_config &config)
{
	gi6809_base(config);

	MC6809E(config, m_maincpu, CPU_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gi6809_state::jestmain_map);

	MC6809E(config, m_slavecpu, CPU_CLOCK/2);
	m_slavecpu->set_addrmap(AS_PROGRAM, &gi6809_state::jestslave_map);

	MC6845(config, m_crtc, PIXEL_CLOCK);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_crtc->out_vsync_callback().append(m_pia[0], FUNC(pia6821_device::ca1_w));
	m_crtc->out_vsync_callback().append([this](u8 state) { m_pia[1]->ca1_w(!state); });

	PIA6821(config, m_pia[0], 0);  // DDRA:00 (All In) - DDRB:1F (IIIO-OOOO)
	m_pia[0]->readpa_handler().set_ioport("PIA0_A");
	m_pia[0]->writepb_handler().set(FUNC(gi6809_state::lamps5_w));
	m_pia[0]->irqa_handler().set_inputline(m_slavecpu, M6809_IRQ_LINE);
	m_pia[0]->ca2_handler().set_nop();
	m_pia[0]->cb2_handler().set_nop();

	PIA6821(config, m_pia[1], 0);  // DDRA:FF (All Out) - DDRB:EO (OOOI-IIII)
	m_pia[1]->writepa_handler().set(FUNC(gi6809_state::snd_mux_w));
	m_pia[1]->readpb_handler().set(FUNC(gi6809_state::gi6809_mux_port_r));
	m_pia[1]->writepb_handler().set(FUNC(gi6809_state::lamps3_w));

	//m_pia[1]->readca1_handler() coin in upper opto to be implemented
	//m_pia[1]->readca2_handler() coin in lower opto to be implemented

	m_pia[1]->ca2_handler().set_nop();
	m_pia[1]->cb2_handler().set_nop();
	m_pia[1]->irqb_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_pia[1]->irqa_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( castawayt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca_5.bin",  0x8000, 0x8000, CRC(cd7aeb7c) SHA1(2d8ad18d2537db9684c0fd753e970ca6255eb1e5) )

	ROM_REGION( 0x10000, "slavecpu", 0 )
	ROM_LOAD( "ca_6.bin",  0xe000, 0x2000, CRC(74ac5034) SHA1(2169be6a563938e87328dcadc05c4c06cc4fa45c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(             0x0000, 0x2000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "ca_4.bin", 0x2000, 0x1000, CRC(1f76fc30) SHA1(7164f9de8ced1831c15eaef522272ee5acea80cc) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ca_1.bin",  0x0000, 0x1000, CRC(61b05a25) SHA1(5a9cec1c8c4d5d94fe30cceec109b5d0d0fd3eca) )
	ROM_LOAD( "ca_2.bin",  0x1000, 0x1000, CRC(6e278ac0) SHA1(93b3b1c296c1c502db265fbbca6116f44ae92fa1) )
	ROM_LOAD( "ca_3.bin",  0x2000, 0x1000, CRC(8ccbf10c) SHA1(409498eb47437b36cead5ea38593a1bf87d7e4df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ca_bp.u35",   0x0000, 0x0100, CRC(9d9a0aae) SHA1(d2ccf8576164da34e2d9b7d1c5c76cba82bc012c) )
	ROM_LOAD( "ca_bp.u31",   0x0100, 0x0100, CRC(4d233808) SHA1(ac864bc5fe23e76786c78119b052eb68fa923ad4) )
ROM_END

ROM_START( jesterch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512.u8",  0x0000, 0x10000, CRC(64045d54) SHA1(6a48e868f854153dad2e28043ceeabf3e028cc6b) )

	ROM_REGION( 0x10000, "slavecpu", 0 )
	ROM_LOAD( "2764.u52", 0xe000, 0x2000, CRC(a23cf64d) SHA1(8bcceb87a63e32812a354f064d81bdc5245cc8b9) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(             0x0000, 0x2000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "2732.u5",  0x2000, 0x1000, CRC(1f76fc30) SHA1(7164f9de8ced1831c15eaef522272ee5acea80cc) )  // same castawayt charset.

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "2732.u2",  0x0000, 0x1000, CRC(e7243596) SHA1(120261604815b03cf230894828280c3a5b05bf2e) )
	ROM_LOAD( "2732.u3",  0x1000, 0x1000, CRC(f4dfea60) SHA1(c4708cae3f3ecb0daee8d917743eb6e91f0a1119) )
	ROM_LOAD( "2732.u4",  0x2000, 0x1000, CRC(fb8fb3c4) SHA1(de0b6c4d8ca40f6c548d199eb55e1135d317f954) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "jc_bp.u35",   0x0000, 0x0100, CRC(9d9a0aae) SHA1(d2ccf8576164da34e2d9b7d1c5c76cba82bc012c) )
	ROM_LOAD( "jc_bp.u31",   0x0100, 0x0100, CRC(4d233808) SHA1(ac864bc5fe23e76786c78119b052eb68fa923ad4) )  // from CA
ROM_END

ROM_START( glck6809 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c128.u8",  0xc000, 0x4000, CRC(867d84c2) SHA1(44c2f94559aa5d11b42363d0a9b2fb7612b945c3) )

	ROM_REGION( 0x10000, "slavecpu", 0 )
	ROM_LOAD( "27c64.u52",  0xe000, 0x2000, CRC(90cebc12) SHA1(f913817f2a066d97fd369519a73a6cd18f7f9ea1) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(             0x0000, 0x2000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "2732.u5",  0x2000, 0x1000, CRC(d4ffe249) SHA1(22207a11d8f5c59a8719f0bca79effd13901f969) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "2732.u2",  0x0000, 0x1000, CRC(2fe35edd) SHA1(a92cd59b6471d249c54d5b94df781dfed7b1764d) )
	ROM_LOAD( "2732.u3",  0x1000, 0x1000, CRC(2deeb3d9) SHA1(2fdb7666ebff2b7caf52e5c81959066a3762235c) )
	ROM_LOAD( "2732.u4",  0x2000, 0x1000, CRC(0d3ac3d5) SHA1(57c6c4b0ae6099cdb7efe5b4b3a4cb31e7df8634) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "gl_bp.u35",   0x0000, 0x0100, CRC(e82e2364) SHA1(e71268e91248015c20d1359f866bf53bc355d181) )
	ROM_LOAD( "gl_bp.u31",   0x0100, 0x0100, CRC(dc4b003c) SHA1(09708d620c2f20028eae3b2c060dc30b649d2215) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void gi6809_state::init_cast()
{
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x9676] = 0x12;  // fix hangs on Bonus Game - NOP 19Ch flag check 1/2
	ROM[0x9677] = 0x12;  // fix hangs on Bonus Game - NOP 19Ch flag check 2/2
	ROM[0x837f] = 0x00;  // fix checksum


	uint8_t *ROM2 = memregion("slavecpu")->base();

	ROM2[0xf529] = 0x05;  // Max Bet - This value should come from non-existent NVRAM
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME       PARENT MACHINE    INPUT      STATE         INIT        ROT     COMPANY                      FULLNAME                                        FLAGS
GAME( 1987, castawayt, 0,     castawayt, castawayt, gi6809_state, init_cast,  ROT0,  "Tranex Australia Pty Ltd.", "Cast Away (dual 6809 GI Australasia PCB)",      MACHINE_NOT_WORKING )
GAME( 1987, jesterch,  0,     jesterch,  jesterch,  gi6809_state, empty_init, ROT0,  "Tranex Australia Pty Ltd.", "Jester Chance (dual 6809 GI Australasia PCB)",  MACHINE_NOT_WORKING )
GAME( 198?, glck6809,  0,     glck6809,  glck6809,  gi6809_state, empty_init, ROT0,  "General Instrument?",       "Good Luck! (dual 6809 GI Australasia PCB)",     MACHINE_NOT_WORKING )
