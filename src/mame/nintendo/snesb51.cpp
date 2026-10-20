// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SNES bootlegs controlled by an MCS-51 core

    The following systems are dumped:
    - Fatal Fury Special
    - Mortal Kombat 3
    - 1993 4 Slot board (two different BIOS versions)
    - 1994 4 Slot board
    - 4 Slot board with built-in NBA Jam

    Hardware (for the 4 slot switcher):
    - Unknown PLC44 chip (markings removed) near a 8192-word x 8-bit Static RAM
    - WD1016D-PL (MCS-51 MCU) or SAB 8051A-P
    - TC5565PL-15 (8k SRAM)
    - 12 MHz XTAL
    - 4 Position Dipswitch (time, game handling and cycling/slot count)

    Connector pinout:

                 JAMMA                      P1 (from SNES)
    GND          A | 1           GND         1 - GND
    GND          B | 2           GND         2 -
    +5v          C | 3           +5v         3 -
    +5v          D | 4           +5v         4 -
    -5v          E | 5           -5v         5 -
    +12v         F | 6          +12v         6 -
                 H | 7                       7 -
                 J | 8                       8 -
                 K | 9                       9 -
    SND-         L | 10         SND+        10 -
    P2 Button R  M | 11  P1 Button R        11 -
    Video GREEN  N | 12    Video RED        12 -
    Video SYNC   P | 13   Video BLUE
                 R | 14    Video GND        P2 (unknown)
    P2 Button L  S | 15  P2 Button L        1 -
                 T | 16      Coin #1        2 -
    P2 Start     U | 17     P1 Start        3 -
    P2 Up        V | 18        P1 Up
    P2 Down      W | 19      P1 Down        P3 (unknown)
    P2 Left      X | 20      P1 Left        1 - +5v
    P2 Right     Y | 21     P1 Right        2 - GND
    P2 Button X  Z | 22  P1 Button X
    P2 Button Y  a | 23  P1 Button Y        P4 (unknown)
    P2 Button A  b | 24  P1 Button A        1 - +5v
    P2 Button B  c | 25  P1 Button B        2 -
    P2 Select    d | 26    P1 Select        3 -
    GND          e | 27          GND        4 -
    GND          f | 28          GND        5 - N/C
                                            6 -
    * Note - P5 ribbon to SNES controls     7 - GND

    Dumper's notes for MK3:
    "Ok can confirm with this style the MCU provides an overlay similar to the "NBA jam 4 slot" snes bootlegs.
    I have an onscreen insert coin display from the MCU but the underlying snes hardware is dead so actual game is not running."

    TODO:
    - Identify the overlay chip and mix its output with the SNES video
    - Figure out how NBA Jam can override the first slot

***************************************************************************/

#include "emu.h"
#include "snes.h"
#include "cpu/mcs51/i8051.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class snesb51_state : public snes_state
{
public:
	snesb51_state(const machine_config &mconfig, device_type type, const char *tag) :
		snes_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_mcu_speaker(*this, "mcu_speaker"),
		m_speaker(*this, "speaker"),
		m_snes_rom(*this, "user3"),
		m_slots(*this, "slot%u", 0U),
		m_pad(*this, "P%u", 1U)
		{ }

	void base(machine_config &config);
	void mk3snes(machine_config &config);
	void snes4sl(machine_config &config);
	void snes4sln(machine_config &config);

	void init_fatfurspb();

	template<int N> int snes_address_status_r();
	template<int N> ioport_value mk3_controller_r();
	template<int N> ioport_value switcher_controller_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	virtual void hblank_w(int state) override;

private:
	void switcher_mem_map(address_map &map) ATTR_COLD;
	void mk3_io_map(address_map &map) ATTR_COLD;
	void switcher_io_map(address_map &map) ATTR_COLD;
	void snes_map(address_map &map) ATTR_COLD;
	void spc_map(address_map &map) ATTR_COLD;

	void mcu_p1_w(uint8_t data);
	void mcu_p3_w(uint8_t data);

	void switch_game_select_w(uint8_t data);
	void switch_overlay_color_mute_w(uint8_t data);

	void mk3_controller_w(uint8_t data);
	void mk3_controller_aux_w(uint8_t data);
	void mk3_sound_ctrl_w(uint8_t data);
	void mk3_overlay_color_w(uint8_t data);

	void vram_address_high_w(uint8_t data);
	void vram_address_low_w(uint8_t data);
	void vram_w(uint8_t data);

	void rom_address_high_w(uint8_t data);
	void rom_address_low_w(uint8_t data);
	uint8_t rom_r();

	template<int N> void install_watcher();
	template<int N> uint8_t snes_bus_watch_ack_r();
	template<int N> void snes_bus_watch_w(uint8_t data);

	uint32_t overlay_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mcs51_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_mcu_speaker;
	required_device<speaker_device> m_speaker;
	required_memory_region m_snes_rom;
	optional_memory_region_array<4> m_slots;
	required_ioport_array<2> m_pad;

	std::unique_ptr<uint8_t[]> m_vram;
	uint16_t m_vram_address = 0;
	uint16_t m_rom_address = 0;
	uint8_t m_selected_slot = 0;

	uint8_t m_p1 = 0xff;
	uint8_t m_p3 = 0xff;
	uint8_t m_mk3_controller = 0xff;
	uint8_t m_mk3_controller_aux = 0xff;

	bool m_overlay_start = false;
	uint8_t m_overlay_y = 0;
	rgb_t m_overlay_color = rgb_t::white();

	memory_passthrough_handler m_snes_address_tap[4];
	bool m_snes_address_tap_active[4]{};
	offs_t m_snes_address_offset[4]{};
	offs_t m_snes_address_offset_latch[4]{};
	bool m_snes_address_status[4]{};
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void snesb51_state::snes_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snesb51_state::snes_r_bank1), FUNC(snesb51_state::snes_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share(m_wram);                 // 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM
	map(0x800000, 0xffffff).rw(FUNC(snesb51_state::snes_r_bank2), FUNC(snesb51_state::snes_w_bank2));    // Mirror and ROM
}

void snesb51_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}

void snesb51_state::switcher_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mcu", 0);
}

void snesb51_state::mk3_io_map(address_map &map)
{
	map(0x8000, 0x8000).portr("MCU");
	map(0x8200, 0x8200).r(FUNC(snesb51_state::snes_bus_watch_ack_r<0>));
	map(0x8401, 0x8401).w(FUNC(snesb51_state::mk3_sound_ctrl_w));
	map(0x8600, 0x8600).r(FUNC(snesb51_state::snes_bus_watch_ack_r<1>));
	map(0x8800, 0x8800).w(FUNC(snesb51_state::snes_bus_watch_w<0>));
	map(0x8c00, 0x8c00).w(FUNC(snesb51_state::vram_address_high_w));
	map(0x8e00, 0x8e00).w(FUNC(snesb51_state::vram_address_low_w));
	map(0x9000, 0x9000).w(FUNC(snesb51_state::mk3_controller_w));
	map(0xa000, 0xa000).w(FUNC(snesb51_state::vram_w));
	map(0xb000, 0xb000).w(FUNC(snesb51_state::mk3_overlay_color_w));
	map(0xc000, 0xc000).r(FUNC(snesb51_state::snes_bus_watch_ack_r<2>));
	map(0xd000, 0xd000).r(FUNC(snesb51_state::snes_bus_watch_ack_r<3>));
	map(0xd200, 0xd200).portr("DSW1");
	map(0xd400, 0xd400).portr("DSW2");
	map(0xd600, 0xd600).w(FUNC(snesb51_state::snes_bus_watch_w<2>));
	map(0xda00, 0xda00).w(FUNC(snesb51_state::snes_bus_watch_w<3>));
	map(0xe000, 0xe000).w(FUNC(snesb51_state::snes_bus_watch_w<1>));
	map(0xf000, 0xf000).w(FUNC(snesb51_state::mk3_controller_aux_w));
}

void snesb51_state::switcher_io_map(address_map &map)
{
	map(0x8000, 0x8000).portr("DSW1");
	map(0x8200, 0x8200).portr("MCU");
	map(0x8401, 0x8401).w(FUNC(snesb51_state::switch_game_select_w));
	map(0x8800, 0x8800).w(FUNC(snesb51_state::snes_bus_watch_w<0>));
	map(0x8a00, 0x8a00).r(FUNC(snesb51_state::snes_bus_watch_ack_r<0>));
	map(0x8c00, 0x8c00).w(FUNC(snesb51_state::vram_address_high_w));
	map(0x8e00, 0x8e00).w(FUNC(snesb51_state::vram_address_low_w));
	map(0xa000, 0xa000).w(FUNC(snesb51_state::vram_w));
	map(0xc000, 0xc000).r(FUNC(snesb51_state::snes_bus_watch_ack_r<1>));
	map(0xd000, 0xd000).w(FUNC(snesb51_state::rom_address_low_w));
	map(0xd400, 0xd400).w(FUNC(snesb51_state::rom_address_high_w));
	map(0xd800, 0xd800).r(FUNC(snesb51_state::rom_r));
	map(0xb000, 0xb000).w(FUNC(snesb51_state::switch_overlay_color_mute_w));
	map(0xe000, 0xe000).w(FUNC(snesb51_state::snes_bus_watch_w<1>));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( snesb51 )
	PORT_START("MCU")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL1_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("SERIAL2_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("SERIAL1_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mk3snes )
	PORT_INCLUDE(snesb51)

	PORT_MODIFY("MCU")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<0>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<1>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<2>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<3>))

	PORT_START("START")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Panel Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Panel Start")
	PORT_BIT( 0xf9, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERIAL1_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(snesb51_state::mk3_controller_r<0>))

	PORT_MODIFY("SERIAL2_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(snesb51_state::mk3_controller_r<1>))

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "DSW1:2")
	PORT_DIPNAME(0x1c, 0x0c, DEF_STR(Coin_A)) PORT_DIPLOCATION("DSW1:3,4,5")
	PORT_DIPSETTING(0x00, DEF_STR(1C_10C))
	PORT_DIPSETTING(0x04, DEF_STR(1C_5C))
	PORT_DIPSETTING(0x08, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x0c, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x10, DEF_STR(2C_1C))
	PORT_DIPSETTING(0x14, DEF_STR(5C_1C))
	PORT_DIPSETTING(0x18, DEF_STR(10C_1C))
	PORT_DIPSETTING(0x1c, DEF_STR(1C_1C)) // duplicate
	PORT_DIPNAME(0xe0, 0x60, DEF_STR(Coin_B)) PORT_DIPLOCATION("DSW1:6,7,8")
	PORT_DIPSETTING(0x00, DEF_STR(1C_10C))
	PORT_DIPSETTING(0x20, DEF_STR(1C_5C))
	PORT_DIPSETTING(0x40, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x60, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x80, DEF_STR(2C_1C))
	PORT_DIPSETTING(0xa0, DEF_STR(5C_1C))
	PORT_DIPSETTING(0xc0, DEF_STR(10C_1C))
	PORT_DIPSETTING(0xe0, DEF_STR(1C_1C)) // duplicate

	PORT_START("DSW2")
	PORT_DIPNAME(0x01, 0x01, "Game Program") PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(0x00, "Killer Instinct")
	PORT_DIPSETTING(0x01, "Mortal Kombat 3")
	PORT_DIPNAME(0x02, 0x00, "Test/Free Play?") PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On)) // enables controllers without coining up
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPNAME(0x70, 0x00, "Time Limit") PORT_DIPLOCATION("DSW2:5,6,7")
	PORT_DIPSETTING(0x00, "3 Minutes")
	PORT_DIPSETTING(0x10, "5 Minutes")
	PORT_DIPSETTING(0x20, "7 Minutes")
	PORT_DIPSETTING(0x30, "9 Minutes")
	PORT_DIPSETTING(0x40, "11 Minutes")
	PORT_DIPSETTING(0x50, "13 Minutes")
	PORT_DIPSETTING(0x60, "15 Minutes")
	PORT_DIPSETTING(0x70, "17 Minutes")
	PORT_DIPNAME(0x80, 0x80, "Time Limit Enable") PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
INPUT_PORTS_END

static INPUT_PORTS_START( kinstsnes )
	PORT_INCLUDE(mk3snes)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x00, "Game Program") PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(0x00, "Killer Instinct")
	PORT_DIPSETTING(0x01, "Mortal Kombat 3")
INPUT_PORTS_END

static INPUT_PORTS_START( snes4sl )
	PORT_INCLUDE(snesb51)

	PORT_MODIFY("MCU")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Panel Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Panel Start")

	PORT_MODIFY("SERIAL1_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(snesb51_state::switcher_controller_r<0>))

	PORT_MODIFY("SERIAL2_DATA1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(snesb51_state::switcher_controller_r<1>))

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x03, "Game Time")    PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(   0x00, "5 Minutes")
	PORT_DIPSETTING(   0x01, "20 Minutes")
	PORT_DIPSETTING(   0x02, "15 Minutes")
	PORT_DIPSETTING(   0x03, "10 Minutes")
	PORT_DIPNAME(0x04, 0x04, "Automatic Game Cycling") PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x08, "Game Mode")    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(   0x00, "Timer")
	PORT_DIPSETTING(   0x08, "Game-specific")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<0>))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(snesb51_state::snes_address_status_r<1>))
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( snes4sln )
	PORT_INCLUDE(snes4sl)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x01, 0x01, "Game Time")    PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(   0x00, "5 Minutes")
	PORT_DIPSETTING(   0x01, "10 Minutes")
	PORT_DIPNAME(0x06, 0x02, "Enabled Slots") PORT_DIPLOCATION("S1:2,3")
	PORT_DIPSETTING(   0x02, "1")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPSETTING(   0x04, "3")
	PORT_DIPSETTING(   0x06, "4")
INPUT_PORTS_END

static INPUT_PORTS_START( snes4sl94 )
	PORT_INCLUDE(snes4sln)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x01, 0x01, "Game Time")    PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(   0x00, "3 Minutes")
	PORT_DIPSETTING(   0x01, "5 Minutes")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void snesb51_state::mk3_overlay_color_w(uint8_t data)
{
	// 765432--  overlay color, format rrbbgg
	// ------10  unknown

	const int r = BIT(data, 6, 2);
	const int b = BIT(data, 4, 2);
	const int g = BIT(data, 2, 2);

	m_overlay_color = rgb_t(r * 0x55, g * 0x55, b * 0x55);
}

void snesb51_state::hblank_w(int state)
{
	m_mcu->set_input_line(MCS51_T0_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t snesb51_state::overlay_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
//	bitmap.fill(0, cliprect);

	if (m_overlay_start)
	{
		m_overlay_y = (m_vram_address & 0xff) + 1;
		m_overlay_start = false;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		m_ppu->refresh_scanline(bitmap, y, y);

		if (!BIT(m_p1, 7))
		{
			for (int x = cliprect.min_x; x <= cliprect.max_x; x+=2)
			{
				u8 dot = BIT(m_vram[((x >> 4) << 8) | m_overlay_y], 7 - ((x >> 1) & 7));
				if (dot)
				{
					bitmap.pix(y, x + 0) = m_overlay_color;
					bitmap.pix(y, x + 1) = m_overlay_color;
				}
			}
			m_overlay_y++;
		}
	}

	return 0;
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void snesb51_state::mk3_sound_ctrl_w(uint8_t data)
{
	// 765432--  unknown
	// ------1-  audio mute (audio is unmuted after inserting a coin)
	// -------0  unknown

	const float gain = BIT(data, 1) ? 0.0f : 1.0f;
	m_speaker->set_input_gain(0, gain);
	m_speaker->set_input_gain(1, gain);
}

void snesb51_state::switch_overlay_color_mute_w(uint8_t data)
{
	// 765432--  overlay color, format bbggrr
	// ------1-  unknown
	// -------0  audio mute (audio is unmuted after inserting a coin)

	const int b = BIT(data, 6, 2);
	const int g = BIT(data, 4, 2);
	const int r = BIT(data, 2, 2);
	m_overlay_color = rgb_t(r * 0x55, g * 0x55, b * 0x55);

	const float gain = BIT(data, 0) ? 1.0f : 0.0f;
	m_speaker->set_input_gain(0, gain);
	m_speaker->set_input_gain(1, gain);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

template<int N>
ioport_value snesb51_state::mk3_controller_r()
{
	uint16_t data = 0;

	// controller enabled?
	if (!BIT(m_mk3_controller, 3 + N * 4))
		data = m_pad[N]->read();

	// start buttons
	if (!BIT(m_p1, 3 + N))
		data |= 0x1000;

	// down button: kinstsnes can automatically push it to select 2 player mode
	if (N == 0 && !BIT(m_mk3_controller_aux, 5))
		data |= 0x0400;

	return data;
}

void snesb51_state::mk3_controller_w(uint8_t data)
{
	// 7-------  player 2 start
	// -654----  unknown
	// ----3---  player 1 start
	// -----210  unknown

	m_mk3_controller = data;
}

void snesb51_state::mk3_controller_aux_w(uint8_t data)
{
	// 76------  unknown
	// --5-----  player 1 down (might be player 2 down)
	// ---43210  unknown

	m_mk3_controller_aux = data;
}

template<int N>
ioport_value snesb51_state::switcher_controller_r()
{
	uint16_t data = 0;

	// controller enabled?
	if (!BIT(m_p1, 6 - N))
		data = m_pad[N]->read();

	// start buttons
	if (!BIT(m_p1, 3 + N))
		data |= 0x1000;

	return data;
}

void snesb51_state::mcu_p1_w(uint8_t data)
{
	// 7-------  screen overlay active
	// -6------  player 1 controls enable (switcher)
	// --5-----  player 2 controls enable (switcher)
	// ---4----  player 2 start to snes
	// ----3---  player 1 start to snes
	// -----2--  player 2 panel start (mk3)
	// ------1-  player 1 panel start (mk3)
	// -------0  unknown

	// enable overlay on 1->0 transition
	if (BIT(m_p1, 7) == 1 && BIT(data, 7) == 0)
		m_overlay_start = true;

	m_p1 = data;
}

void snesb51_state::mcu_p3_w(uint8_t data)
{
	// 765-----  unknown
	// ---4----  horizontal blanking input
	// ----3---  reset snes
	// -----2--  vblank
	// ------10  mcu audio generation (insert coin sound)

	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);

	if (BIT(m_p3, 3) == 1 && BIT(data, 3) == 0)
		m_ppu->reset();

	m_mcu_speaker->level_w(data & 0x03);

	m_p3 = data;
}

void snesb51_state::switch_game_select_w(uint8_t data)
{
	// 76543---  unknown
	// -----2--  set for games which require the Super Scope lightgun
	// ------10  game select

	logerror("switch_ctrl_w: %02x\n", data);

	// not fully understood
	// selecting a game seems to use 0x18..0x1f
	// while reading it for identification uses 0xf4..0xf7
	if ((data & 0xf8) != 0x18 && (data & 0xfc) != 0xf4)
		return;

	m_selected_slot = data & 0x03;

	logerror("switching to %d\n", m_selected_slot);

	if (!m_slots[m_selected_slot])
		return;

	memcpy(m_snes_rom->base(), m_slots[m_selected_slot]->base(), 0x400000);

	init_snes();
}

void snesb51_state::vram_address_high_w(uint8_t data)
{
	data &= 0x1f;
	m_vram_address = (data << 8) | (m_vram_address & 0x00ff);
}

void snesb51_state::vram_address_low_w(uint8_t data)
{
	m_vram_address = (m_vram_address & 0xff00) | data;
}

void snesb51_state::vram_w(uint8_t data)
{
	m_vram[m_vram_address] = data;
}

void snesb51_state::rom_address_low_w(uint8_t data)
{
	m_rom_address = (m_rom_address & 0xff00) | data;
}

void snesb51_state::rom_address_high_w(uint8_t data)
{
	m_rom_address = (data << 8) | (m_rom_address & 0x00ff);
}

uint8_t snesb51_state::rom_r()
{
	return m_snes_rom->base()[m_rom_address];
}

template<int N>
int snesb51_state::snes_address_status_r()
{
	return m_snes_address_status[N] ? 1 : 0;
}

template<int N>
void snesb51_state::install_watcher()
{
	m_snes_address_tap[N].remove();
	m_snes_address_tap[N] = m_maincpu->space(AS_PROGRAM).install_read_tap
	(
		m_snes_address_offset_latch[N], m_snes_address_offset_latch[N],
		util::string_format("snes_bus_watcher%d", N),
		[this] (offs_t offset, uint8_t &, uint8_t)
		{
			if (machine().side_effects_disabled())
				return;

			m_snes_address_status[N] = true;
			m_snes_address_tap[N].remove();
			m_snes_address_tap_active[N] = false;
		},
		&m_snes_address_tap[N]
	);
}

template<int N>
uint8_t snesb51_state::snes_bus_watch_ack_r()
{
	if (machine().side_effects_disabled())
		return 0xff;

	m_snes_address_tap_active[N] = true;
	m_snes_address_offset_latch[N] = m_snes_address_offset[N];
	m_snes_address_status[N] = false;

	install_watcher<N>();

	return 0xff;
}

template<int N>
void snesb51_state::snes_bus_watch_w(uint8_t data)
{
	m_snes_address_offset[N] = (data << 16) | (m_snes_address_offset[N] >> 8);
}

void snesb51_state::machine_start()
{
	snes_state::machine_start();

	m_vram = std::make_unique<uint8_t[]>(0x2000);

	// register for save states
	save_pointer(NAME(m_vram), 0x2000);
	save_item(NAME(m_vram_address));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_selected_slot));
	save_item(NAME(m_p1));
	save_item(NAME(m_p3));
	save_item(NAME(m_mk3_controller));
	save_item(NAME(m_mk3_controller_aux));
	save_item(NAME(m_overlay_start));
	save_item(NAME(m_overlay_y));
	save_item(NAME(m_overlay_color));
	save_item(NAME(m_snes_address_tap_active));
	save_item(NAME(m_snes_address_offset));
	save_item(NAME(m_snes_address_offset_latch));
	save_item(NAME(m_snes_address_status));
}

void snesb51_state::machine_reset()
{
	snes_state::machine_reset();

	m_p1 = 0xff;
	m_p3 = 0xff;
	m_mk3_controller = 0xff;
	m_mk3_controller_aux = 0xff;

	m_overlay_start = false;
	m_overlay_y = 0;

	// reset bus watcher
	for (auto &tap : m_snes_address_tap)
		tap.remove();

	std::fill(std::begin(m_snes_address_tap_active), std::end(m_snes_address_tap_active), false);
	std::fill(std::begin(m_snes_address_offset), std::end(m_snes_address_offset), 0);
	std::fill(std::begin(m_snes_address_status), std::end(m_snes_address_status), false);
}

void snesb51_state::device_post_load()
{
	if (m_slots[m_selected_slot])
	{
		memcpy(m_snes_rom->base(), m_slots[m_selected_slot]->base(), 0x400000);
		init_snes();
	}

	// restore watcher
	for (auto &tap : m_snes_address_tap)
		tap.remove();

	if (m_snes_address_tap_active[0]) install_watcher<0>();
	if (m_snes_address_tap_active[1]) install_watcher<1>();
	if (m_snes_address_tap_active[2]) install_watcher<2>();
	if (m_snes_address_tap_active[3]) install_watcher<3>();
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void snesb51_state::base(machine_config &config)
{
	// basic machine hardware
	_5A22(config, m_maincpu, 3580000 * 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &snesb51_state::snes_map);

	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snesb51_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	SCREEN(config, m_screen);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	// TODO: can't use VIDEO_VARIABLE_WIDTH with an overlay
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->set_screen_update(FUNC(snesb51_state::overlay_update));
	m_screen->screen_vblank().set_inputline(m_mcu, MCS51_INT0_LINE);

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set([this] { return snes_open_bus_r(); });
	m_ppu->set_screen("screen");

	// sound hardware
	SPEAKER(config, m_speaker, 2).front();

	S_DSP(config, m_s_dsp, XTAL(24'576'000));
	m_s_dsp->set_addrmap(0, &snesb51_state::spc_map);
	m_s_dsp->add_route(0, "speaker", 1.00, 0);
	m_s_dsp->add_route(1, "speaker", 1.00, 1);

	// unverified weights
	static constexpr double speaker_levels[] = { 0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0 };

	SPEAKER_SOUND(config, m_mcu_speaker);
	m_mcu_speaker->set_levels(4, speaker_levels);
	m_mcu_speaker->add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	m_mcu_speaker->add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}

void snesb51_state::mk3snes(machine_config &config)
{
	base(config);

	I8751(config, m_mcu, 12_MHz_XTAL);
	m_mcu->set_addrmap(AS_DATA, &snesb51_state::mk3_io_map);
	m_mcu->port_in_cb<1>().set_ioport("START");
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set_constant(0xff);
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}

void snesb51_state::snes4sl(machine_config &config)
{
	base(config);

	// exact type unknown
	I8031(config, m_mcu, 12_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &snesb51_state::switcher_mem_map);
	m_mcu->set_addrmap(AS_DATA, &snesb51_state::switcher_io_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set_constant(0xff);
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}

void snesb51_state::snes4sln(machine_config &config)
{
	base(config);

	I8051(config, m_mcu, 12_MHz_XTAL); // SAB 8051A-P
	m_mcu->set_addrmap(AS_PROGRAM, &snesb51_state::switcher_mem_map);
	m_mcu->set_addrmap(AS_DATA, &snesb51_state::switcher_io_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set_constant(0xff);
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// This is identical to the SNES release apart from a single byte
ROM_START( mk3snes )
	ROM_REGION(0x400000, "user3", 0)
	ROM_LOAD("5.u5", 0x000000, 0x080000, CRC(c21ee1ac) SHA1(12fc526e39b0b998b39d558fbe5660e72c7fad14))
	ROM_LOAD("6.u6", 0x080000, 0x080000, CRC(0e064323) SHA1(a11175516892beb862c7cc1e186034ef1b55ee8f))
	ROM_LOAD("7.u7", 0x100000, 0x080000, CRC(7db6b7be) SHA1(a7653c04f5321fd83062425a492c7ed0a4f1fdb0))
	ROM_LOAD("8.u8", 0x180000, 0x080000, CRC(28771750) SHA1(d6c469ca2640935b6687f5bf5f6e85275157abb0))
	ROM_LOAD("1.u1", 0x200000, 0x080000, CRC(4cab6332) SHA1(3c417ba6d35532b4e2ca9ae4a3b730c589d26aee))
	ROM_LOAD("2.u2", 0x280000, 0x080000, CRC(0327999b) SHA1(dc6bb11a925e893453e0e5e5d88b8ace8d6cf859))
	ROM_LOAD("3.u3", 0x300000, 0x080000, CRC(229af2de) SHA1(1bbb02aec08afab979ffbe4b68a48dc4cc923f73))
	// this ROM has 1 byte changed compared to sns-a3me-0.u1 (mk3u in snes softlist). It has been confirmed on 2 different PCBs
	ROM_LOAD("4.u4", 0x380000, 0x080000, CRC(b51930d9) SHA1(220f00d64809a6218015a738e53f11d8dc81578f))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("d87c51.u9", 0x0000, 0x1000, CRC(f447620a) SHA1(ac0d78c7b339f13d5f96a6727a0f2147158697f9))

	ROM_REGION(0x100, "audiocpu", 0)
	ROM_LOAD("spc700.rom", 0x00, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0))
ROM_END

ROM_START( kinstsnes )
	ROM_REGION(0x400000, "user3", 0)
	ROM_LOAD("5.u5", 0x000000, 0x080000, CRC(8212bf7f) SHA1(d52b581f5ec29638b373678b3eac692822c5065e))
	ROM_LOAD("6.u6", 0x080000, 0x080000, CRC(39f2dad2) SHA1(2fa3cb2cd4c984484367bce443c526c399e3efec))
	ROM_LOAD("7.u7", 0x100000, 0x080000, CRC(972b6ba9) SHA1(3f85921b4806b0f3959fa20176a985d2621907d8))
	ROM_LOAD("8.u8", 0x180000, 0x080000, CRC(6d1b3a01) SHA1(3dc26e5e508242330971847d6d132a4742646f4f))
	ROM_LOAD("1.u1", 0x200000, 0x080000, CRC(462362da) SHA1(448f5a363680e2deba0d75935f18eb431d11d151))
	ROM_LOAD("2.u2", 0x280000, 0x080000, CRC(3eaf9f96) SHA1(7c79af81fb16e6aef702af269de9d0411df5be68))
	ROM_LOAD("3.u3", 0x300000, 0x080000, CRC(05f37f39) SHA1(2b4d3c117bb0188620cc3890a7b3a135922d2369))
	ROM_LOAD("4.u4", 0x380000, 0x080000, CRC(7151f37d) SHA1(37106e06fa8c3016d8b03f5272905a3a555729b9))

	// BAD_DUMP because it's taken from mk3snes, but likely correct
	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("d87c51.u9", 0x0000, 0x1000, BAD_DUMP CRC(f447620a) SHA1(ac0d78c7b339f13d5f96a6727a0f2147158697f9))
ROM_END

ROM_START( snes4sl )
	ROM_REGION(0x400000, "user3", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot0", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot1", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot2", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot3", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "mcu", 0)
	ROM_DEFAULT_BIOS("931207")
	ROM_SYSTEM_BIOS(0, "931207", "93-12-07") // Found on PCB with Siemens SAB 8051A-P (4KBytes internal ROM undumped)
	ROMX_LOAD("27c256_12-07.bin", 0x0000, 0x8000, CRC(0922314d) SHA1(04f1265ddc753111e6fcd56162a917ae1791c164), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "931103", "93-11-03") // Found on PCB with WD1016D-PL
	ROMX_LOAD("27c256_11-03.bin", 0x0000, 0x8000, CRC(4e471581) SHA1(0f23ad065d448097f56ab45c3850d53cf85f3670), ROM_BIOS(1))
ROM_END

ROM_START( snes4sl94 )
	ROM_REGION(0x400000, "user3", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot0", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot1", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot2", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot3", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "mcu", 0)
	// Found on PCB with Intel P80C31BH, has 'SISTEME FRANCE' string in ROM
	ROM_LOAD("27c256_06_01.u43", 0x0000, 0x8000, CRC(d0f4849f) SHA1(971743c9b5a2c0ba267dd8a1ce9ff3821759b4fe))
ROM_END

ROM_START( snes4sln )
	// identical to NBA Jam (nbajamua)
	ROM_REGION(0x400000, "user3", ROMREGION_ERASEFF)
	ROM_LOAD("4.bin", 0x000000, 0x080000, CRC(d27bbebe) SHA1(fcf614fcd3c0fb06037038514d828f0cb597838e))
	ROM_LOAD("3.bin", 0x080000, 0x080000, CRC(c0ca8c3c) SHA1(af5caa1b0254f6b42e4f7f3ba07d0af904017e3c))
	ROM_LOAD("2.bin", 0x100000, 0x080000, CRC(4de70641) SHA1(1f575282a1bb842afcc80e29cebc13d96f8158a4))
	ROM_LOAD("1.bin", 0x180000, 0x080000, CRC(9d59835f) SHA1(061872d9a2b39d271fbfc90b9e8f9abefcfa4282))

	ROM_REGION(0x400000, "slot0", 0)
	ROM_COPY("user3", 0, 0, 0x400000) // temporary

	ROM_REGION(0x400000, "slot1", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot2", ROMREGION_ERASEFF)

	ROM_REGION(0x400000, "slot3", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "mcu", 0)
	ROM_LOAD("5.bin", 0x0000, 0x8000, CRC(af8a64e3) SHA1(f13187d213fe7c2a0edcb88d4e828bd24112e812))
ROM_END

ROM_START( fatfurspb )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD("1.u14", 0x000000, 0x100000, CRC(7cb9192c) SHA1(0247e303902e86eaa9443f2a39d352430df5f46f))
	ROM_LOAD("2.u15", 0x100000, 0x100000, CRC(440e3017) SHA1(7d1a2077032c761676bff7f841ab1fc669d322fa))
	ROM_LOAD("3.u16", 0x200000, 0x100000, CRC(dbbe10de) SHA1(27c590bff5a762a2528d0819b7544914ab6cae7c))
	ROM_LOAD("4.u17", 0x300000, 0x100000, CRC(a356e60c) SHA1(c403eff4e7c7deefed68a34a0dbeefadac8c7a0e))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("d87c51.u9", 0x0000, 0x1000, NO_DUMP)
ROM_END

void snesb51_state::init_fatfurspb()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x400000; i++)
	{
		rom[i] = bitswap<8>(rom[i], 5, 0, 6, 1, 7, 4, 3, 2);
	}

	init_snes_hirom();
}


} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT   MACHINE   INPUT      CLASS          INIT             ROT   COMPANY    FULLNAME                                          FLAGS
GAME( 199?, mk3snes,    0,       mk3snes,  mk3snes,   snesb51_state, init_snes_hirom, ROT0, "bootleg", "Mortal Kombat 3 (SNES bootleg with overlay)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, kinstsnes,  0,       mk3snes,  kinstsnes, snesb51_state, init_snes_hirom, ROT0, "bootleg", "Killer Instinct (SNES bootleg with overlay)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, snes4sl,    0,       snes4sl,  snes4sl,   snesb51_state, init_snes,       ROT0, "bootleg", "SNES 4 Slot arcade switcher (1993)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1994, snes4sl94,  snes4sl, snes4sl,  snes4sl94, snesb51_state, init_snes,       ROT0, "bootleg", "SNES 4 Slot arcade switcher (1994)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1994, snes4sln,   0,       snes4sln, snes4sln,  snesb51_state, init_snes,       ROT0, "bootleg", "SNES 4 Slot arcade switcher (NBA Jam)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, fatfurspb,  0,       mk3snes,  mk3snes,   snesb51_state, init_fatfurspb,  ROT0, "bootleg", "Fatal Fury Special (SNES bootleg with overlay)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
