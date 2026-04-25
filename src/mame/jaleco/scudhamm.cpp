// license:BSD-3-Clause
// copyright-holders:Luca Elia
/******************************************************************************

                      -= Jaleco Scud Hammer Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)

- Note: press F2 to enter service mode -

---------------------------------------------------------------------------
Year + Game     Hardware:   Main    Sound Chips
---------------------------------------------------------------------------
[92 Arm Champs II]          68000   2xM6295

    Top board   EB91042-20048-3 AC-91106B
    Lower board EB91015-20038 GP-9189
    Same chips as Scud Hammer

[94 Scud Hammer]            68000   2xM6295

    Board CF-92128B Chips:
        GS9001501
        GS90015-02 x 2  GS600406   x 2      [Tilemaps]
        GS90015-03

    Board GP-9189 Chips:
        GS90015-03 x 2                      [Sprites]
        GS90015-06 x 2
        GS90015-07
        GS90015-08
        GS90015-09
        GS90015-10
        GS90015-11
        MR90015-35 x 2
---------------------------------------------------------------------------

----------------------------------------------------------------------------------
Main CPU                    [Scud Hammer]
----------------------------------------------------------------------------------
ROM                 R       000000-07ffff
Work RAM + Sprites  RW      0f0000-0fffff
Hardware Regs       RW      080000-087fff
Scroll RAM 0        RW      0a0000-0a7fff
Scroll RAM 1        RW      0b0000-0b7fff
Palette RAM         RW      0b8000-0bffff
    Palette Scroll 0        0b9e00-0b9fff
    Palette Sprites         0bb000-0bbfff
    Palette Scroll 1        0bce00-0bcfff
I / O + Sound       RW      100000-17ffff
----------------------------------------------------------------------------------

Common Issues:

- Some ROMs aren't used (priorities?)
- Screen control register (priorities, layers enabling etc.) - Where is it?

TODO:

- Priorities!
- scudhamm: during attract mode every other how to play screen shows up with
  wrong background layer colors;
- armchmp2: arm input device not completely understood;

******************************************************************************/

#include "emu.h"

#include "jaleco_zoomspr.h"
#include "ms1_tmap.h"

#include "cpu/m68000/m68000.h"
#include "machine/adc0804.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "captflag.lh"


#define LOG_UNKNOWN (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGUNKNOWN(...)   LOGMASKED(LOG_UNKNOWN, __VA_ARGS__) 

namespace {

class scudhamm_state : public driver_device
{
public:
	scudhamm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_watchdog(*this, "watchdog")
		, m_oki(*this, "oki%u", 1U)
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_tmap(*this, "scroll%u", 0U)
		, m_ram(*this, "ram")
		, m_io_in(*this, "IN%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{
		
	}

	void scudhamm(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	void scudhamm_motor_command_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void scudhamm_leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void scudhamm_enable_w(u16 data);
	void scudhamm_oki_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 scudhamm_motor_status_r();
	u16 scudhamm_motor_pos_r();
	u8 scudhamm_analog_r();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scudhamm_scanline);

	void scudhamm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<watchdog_timer_device> m_watchdog;
	required_device_array<okim6295_device, 2> m_oki;
	required_device<palette_device> m_palette;
	required_device<jaleco_zoomspr_device> m_spritegen;
	required_device_array<megasys1_tilemap_device, 2> m_tmap;

	required_shared_ptr<u16> m_ram;

	optional_ioport_array<3> m_io_in;

	output_finder<5> m_leds;

	s32 m_prev = 0;
	u16 m_scudhamm_motor_command = 0U;
};

class armchamp2_state : public scudhamm_state
{
public:
	armchamp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: scudhamm_state(mconfig, type, tag)
		, m_io_arm(*this, "ARM")
	{
	}

	void armchmp2(machine_config &config) ATTR_COLD;

	ioport_value left_sensor_r();
	ioport_value right_sensor_r();
	ioport_value center_sensor_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 motor_status_r();
	void motor_command_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 analog_r();
	void output_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(armchamp2_scanline);

	void armchmp2_map(address_map &map) ATTR_COLD;

	required_ioport m_io_arm;

	u16 m_arm_motor_command = 0;
	s32 m_armold = 0;
};

class captflag_state : public scudhamm_state
{
public:
	captflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: scudhamm_state(mconfig, type, tag)
		, m_hopper(*this, "hopper")
		, m_motor_left(*this, "motor_left")
		, m_motor_right(*this, "motor_right")
		, m_oki_bank(*this, "oki%u_bank", 1U)
		, m_motor_left_output(*this, "left")
		, m_motor_right_output(*this, "right")
	{
	}

	void captflag(machine_config &config) ATTR_COLD;

	template <int N> int motor_busy_r();
	template <int N> ioport_value motor_pos_r();

	void init_vscaptfl() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void motor_command_right_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void motor_command_left_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void motor_move(int side, u16 data);
	void oki_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(captflag_scanline);

	void captflag_map(address_map &map) ATTR_COLD;
	template <unsigned Which> void oki_map(address_map &map) ATTR_COLD;

	required_device<ticket_dispenser_device> m_hopper;

	required_device<timer_device> m_motor_left;
	required_device<timer_device> m_motor_right;

	required_memory_bank_array<2> m_oki_bank;

	output_finder<> m_motor_left_output;
	output_finder<> m_motor_right_output;

	u16 m_captflag_leds = 0;
	u16 m_motor_command[2]{0};
	u16 m_motor_pos[2]{0};
};

/***************************************************************************


                            Video Hardware Init


***************************************************************************/

/**************************************************************************
                                Scud Hammer
**************************************************************************/

u32 scudhamm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_tmap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_spritegen->draw_sprites(bitmap, cliprect, 0, 15, &m_ram[0x8000/2], 0x1000 / 2);
	m_tmap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/**************************************************************************


                        Memory Maps - Main CPU


**************************************************************************/

/**************************************************************************
                            Scud Hammer
**************************************************************************/

void scudhamm_state::machine_start()
{
	m_leds.resolve();
	m_scudhamm_motor_command = 0;

	save_item(NAME(m_prev));
	save_item(NAME(m_scudhamm_motor_command));
}



/*  Motor Status.

    f--- ---- ---- ----     Rotation Limit (R?)
    -e-- ---- ---- ----     Rotation Limit (L?)
    --dc ba98 7654 32--
    ---- ---- ---- --1-     Up Limit
    ---- ---- ---- ---0     Down Limit  */

u16 scudhamm_state::scudhamm_motor_status_r()
{
	return m_scudhamm_motor_command;    // Motor Status
}


u16 scudhamm_state::scudhamm_motor_pos_r()
{
	return 0x00 << 8;
}


/*  Move the motor.

    fedc ba98 7654 32--
    ---- ---- ---- --1-     Move Up
    ---- ---- ---- ---0     Move Down

    Within $20 vblanks the motor must reach the target. */

void scudhamm_state::scudhamm_motor_command_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_scudhamm_motor_command);
}


u8 scudhamm_state::scudhamm_analog_r()
{
	const int i = m_io_in[1]->read();

	if (!machine().side_effects_disabled())
	{
		if ((i ^ m_prev) & 0x4000)
		{
			if (i < m_prev)
				m_prev -= 0x8000;
			else
				m_prev += 0x8000;
		}
	}

	const int j = i - m_prev;
	if (!machine().side_effects_disabled())
		m_prev = i;

	/* effect of hammer collision 'accelerometer':
	$00 - $09 - no hit
	$0A - $3F - soft hit
	$40 - $FF - hard hit */
	if (j < 0)
		return 0;
	else if (j > 0xff)
		return 0xff;
	return j;
}

/*
    I don't know how many leds are there, but each bit in the buttons input
    port (coins, tilt, buttons, select etc.) triggers the corresponding bit
    in this word. I mapped the 3 buttons to the first 3 led.
*/
void scudhamm_state::scudhamm_leds_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_leds[0] = BIT(data, 8);    // 3 buttons
		m_leds[1] = BIT(data, 9);
		m_leds[2] = BIT(data, 10);
	}

	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
		m_leds[3] = BIT(data, 4);
		m_leds[4] = BIT(data, 5);
	}
}


/*
    $FFFC during self test, $FFFF onwards.
    It could be audio(L/R) or layers(0/2) enable.
*/
void scudhamm_state::scudhamm_enable_w(u16 data)
{
	LOGUNKNOWN("%s: Unknown scudhamm_enable_w write %04x", machine().describe_context(), data);
}


void scudhamm_state::scudhamm_oki_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki[0]->set_rom_bank((data >> 0) & 0x3);
		m_oki[1]->set_rom_bank((data >> 4) & 0x3);
	}
}

void scudhamm_state::scudhamm_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                 // ROM
	map(0x082000, 0x082005).w(m_tmap[0], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082008, 0x08200d).nopw(); //      UNUSED LAYER
	map(0x082100, 0x082105).w(m_tmap[1], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082208, 0x082209).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x0a0000, 0x0a3fff).ram().w(m_tmap[0], FUNC(megasys1_tilemap_device::write)).share("scroll0");   // Scroll RAM 0
	map(0x0b0000, 0x0b3fff).ram().w(m_tmap[1], FUNC(megasys1_tilemap_device::write)).share("scroll1");   // Scroll RAM 1
	map(0x0b8000, 0x0bffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");          // Palette
	map(0x0f0000, 0x0fffff).ram().share(m_ram);                                         // Work RAM + Spriteram
	map(0x100000, 0x100001).w(FUNC(scudhamm_state::scudhamm_oki_bank_w));                                          // Sound
	map(0x100008, 0x100009).portr("IN0").w(FUNC(scudhamm_state::scudhamm_leds_w));                          // Buttons
	map(0x100015, 0x100015).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));             // Sound
	map(0x100019, 0x100019).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));             //
	map(0x10001c, 0x10001d).w(FUNC(scudhamm_state::scudhamm_enable_w));                                            // ?
	map(0x100041, 0x100041).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));                    // A / D
	map(0x100044, 0x100045).r(FUNC(scudhamm_state::scudhamm_motor_pos_r));                                  // Motor Position
	map(0x100050, 0x100051).r(FUNC(scudhamm_state::scudhamm_motor_status_r)).w(FUNC(scudhamm_state::scudhamm_motor_command_w));        // Motor Limit Switches
	map(0x10005c, 0x10005d).portr("IN2");                                                    // 2 x DSW
}


/**************************************************************************
                            Arm Champs II

The Arm Champs II start-up arm movement and info
---------------------------------------------------------------------
At power-on, the arm moves left, then right then to center and stops.
The center position is 0000.
Note when the arm is at 0000 (center) the center limit switch is not
on. It comes on when the arm is moved just off center (+-0002 or greater
away from center) and stays on as long as the arm is not on
0001-0000-FFFF (both left and right side). When the arm hits the
left/right limit, both center and that left or right limit switch is on.

Full movement to left limit switch is 0000-0040 (0x41h) and full right
limit switch movement is 0000-FFC2 (0x3eh). Obviously the pot and arm
are not 100% accurate and there's a small amount of slop between the
arm shaft / motor mechanism and the pot.
The limit switches are triggered just before the full movement.
For the purpose of MAME emulation it can be rounded off so both sides
move +- 0x40h.
**************************************************************************/

void armchamp2_state::machine_start()
{
	scudhamm_state::machine_start();

	save_item(NAME(m_arm_motor_command));
	save_item(NAME(m_armold));
}

u16 armchamp2_state::motor_status_r()
{
	return 0x11;
}

void armchamp2_state::motor_command_w(offs_t offset, u16 data, u16 mem_mask)
{
	// 0x00xx -> disable motor (test mode)
	// 0x10ff -> automated test (test limits?)
	// 0x10xx -> apply a left-to-right force
	COMBINE_DATA(&m_arm_motor_command);
}

u8 armchamp2_state::analog_r()
{
	const int armdelta = m_io_arm->read() - m_armold;
	if (!machine().side_effects_disabled())
		m_armold = m_io_arm->read();

	return (~(m_arm_motor_command + armdelta)) & 0xff;    // + x : x<=0 and player loses, x>0 and player wins
}

/*
    f--- ---- ---- ----
    -edc ---- ---- ----     Leds
    ---- ba9- ---- ----
    ---- ---8 ---- ----     Start button led
    ---- ---- 76-- ----     Coin counters
    ---- ---- --54 3210
*/
void armchamp2_state::output_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_leds[0] = BIT(data, 8);
		m_leds[1] = BIT(data, 12);
		m_leds[2] = BIT(data, 13);
		m_leds[3] = BIT(data, 14);
	}

	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
		machine().bookkeeping().coin_counter_w(1, BIT(data, 7));
	}
}

void armchamp2_state::armchmp2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x082000, 0x082005).w(m_tmap[0], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082008, 0x08200d).nopw();
	map(0x082100, 0x082105).w(m_tmap[1], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082208, 0x082209).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x0a0000, 0x0a7fff).ram().w(m_tmap[0], FUNC(megasys1_tilemap_device::write)).share("scroll0");
	map(0x0b0000, 0x0b7fff).ram().w(m_tmap[1], FUNC(megasys1_tilemap_device::write)).share("scroll1");
	map(0x0b8000, 0x0bffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0f0000, 0x0fffff).ram().share(m_ram); // Work RAM + Spriteram
	map(0x100000, 0x100001).portr("IN2").w(FUNC(armchamp2_state::scudhamm_oki_bank_w));
	map(0x100004, 0x100005).portr("DSW"); // DSW
	map(0x100008, 0x100009).portr("IN0").w(FUNC(armchamp2_state::output_w));
	map(0x10000d, 0x10000d).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));
	map(0x100010, 0x100011).rw(FUNC(armchamp2_state::motor_status_r), FUNC(armchamp2_state::motor_command_w));
	// same hookup as acommand, most notably the "Arm Champs II" voice sample on title screen playbacks on oki1 mirror
	map(0x100014, 0x100017).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x100018, 0x10001b).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
}


/**************************************************************************
                                Captain Flag
**************************************************************************/

static constexpr int RIGHT = 0;
static constexpr int LEFT  = 1;

void captflag_state::machine_start()
{
	scudhamm_state::machine_start();

	m_oki_bank[0]->configure_entries(0, 0x100000 / 0x20000, memregion("oki1")->base(), 0x20000);
	m_oki_bank[1]->configure_entries(0, 0x100000 / 0x20000, memregion("oki2")->base(), 0x20000);
	m_oki_bank[0]->set_entry(1);
	m_oki_bank[1]->set_entry(1);

	m_motor_left_output.resolve();
	m_motor_right_output.resolve();

	save_item(NAME(m_captflag_leds));
	save_item(NAME(m_motor_command));
	save_item(NAME(m_motor_pos));
}

void captflag_state::leds_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_captflag_leds);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(1, BIT(data, 8));    // coin 2
		m_leds[0] = BIT(data, 8);    // decide
		machine().bookkeeping().coin_counter_w(0, BIT(data, 10));    // coin 1
		m_leds[1] = BIT(data, 13);    // select

		const bool power = BIT(data, 12);
		m_hopper->motor_w(power ? 1 : 0);    // prize motor
		if (!power)
			m_hopper->reset();
	}
}

void captflag_state::oki_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki_bank[0]->set_entry(((data >> 0) + 1) & 0x7);
		m_oki_bank[1]->set_entry(((data >> 4) + 1) & 0x7);
	}
}

// Motors

void captflag_state::motor_command_right_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Output check:
	// e09a up
	// 80b9 - (when not busy)
	// 0088 - (when busy)
	// e0ba down
	data = COMBINE_DATA(&m_motor_command[RIGHT]);
	motor_move(RIGHT, data);
}
void captflag_state::motor_command_left_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Output check:
	// e0ba up
	// 8099 - (when not busy)
	// 0088 - (when busy)
	// e09a down
	data = COMBINE_DATA(&m_motor_command[LEFT]);
	motor_move(LEFT, data);
}

void captflag_state::motor_move(int side, u16 data)
{
	u16 &pos = m_motor_pos[side];

	timer_device &dev((side == RIGHT) ? *m_motor_right : *m_motor_left);

//  bool busy = !(dev.remaining() == attotime::never);
	bool busy = false;

	if (BIT(data, 4))
	{
		if (!busy)
		{
			const bool down = BIT(data, 5);

			int inc;
			switch (data >> 8)
			{
				case 0xf5:  inc = +2;   break;  // -5 -6
				case 0xf8:  inc = +1;   break;  // -5 -3
				case 0xfe:  inc = -1;   break;  // -5 +3
				case 0x01:  inc = -2;   break;  // -5 +6
				default:
					if ((data >> 8) + 5 >= 0x100)
						inc = -1;
					else
						inc = +1;
			}

			if (BIT(data, 0))
				inc = 1;

			if (down)
				inc *= -1;

			int new_pos = pos + inc;
			if (new_pos > 3)
				new_pos = 3;
			else if (new_pos < 0)
				new_pos = 0;
			busy = (new_pos != pos);
			pos = new_pos;

			if (busy)
				dev.adjust(attotime::from_msec(100));
		}
	}
	else
	{
		dev.reset();
	}

	((side == RIGHT) ? m_motor_right_output : m_motor_left_output) = pos;
}

template <int N>
ioport_value captflag_state::motor_pos_r()
{
	constexpr u8 pos[4] = {1, 0, 2, 3}; // -> 2,3,1,0 offsets -> 0123
	return ~pos[m_motor_pos[N]];
}

template <int N>
int captflag_state::motor_busy_r()
{
//  timer_device & dev = ((side == RIGHT) ? m_motor_right : m_motor_left);
//  return (dev.remaining() == attotime::never) ? 0 : 1;
	return 0;
}

void captflag_state::captflag_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                 // ROM
	map(0x082000, 0x082005).w(m_tmap[0], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082008, 0x08200d).nopw(); //      UNUSED LAYER
	map(0x082100, 0x082105).w(m_tmap[1], FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082208, 0x082209).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x090008, 0x090009).nopw();                                                            // 0?
	map(0x0a0000, 0x0a7fff).ram().w(m_tmap[0], FUNC(megasys1_tilemap_device::write)).share("scroll0"); // Scroll RAM 0
	map(0x0b0000, 0x0b7fff).ram().w(m_tmap[1], FUNC(megasys1_tilemap_device::write)).share("scroll1"); // Scroll RAM 1
	map(0x0b8000, 0x0bffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  // Palette
	map(0x0f0000, 0x0fffff).ram().share(m_ram);                                                 // Work RAM + Spriteram
	map(0x100000, 0x100001).portr("SW1_2").w(FUNC(captflag_state::oki_bank_w));                    // 2 x DSW + Sound
	map(0x100008, 0x100009).portr("BUTTONS").w(FUNC(captflag_state::leds_w));                      // Buttons + Leds
	map(0x100015, 0x100015).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));         // Sound
	map(0x100019, 0x100019).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));         //
	map(0x10001c, 0x10001d).w(FUNC(captflag_state::scudhamm_enable_w));                                            // ?
	map(0x100040, 0x100041).portr("SW01");                                                   // DSW + Motor
	map(0x100044, 0x100045).w(FUNC(captflag_state::motor_command_left_w));                                 // Motor Command (Left)
	map(0x100048, 0x100049).w(FUNC(captflag_state::motor_command_right_w));                                // Motor Command (Right)
	map(0x100060, 0x10007f).ram().share("nvram");      // NVRAM (even bytes only)
}

template <unsigned Which>
void captflag_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_oki_bank[Which]);
}


/***************************************************************************


                                Input Ports


***************************************************************************/

/**************************************************************************
                                Scud Hammer
**************************************************************************/

static INPUT_PORTS_START( scudhamma )
	PORT_START("IN0")   // Buttons
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // PORT_IMPULSE(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // GAME OVER if pressed on the selection screen
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )     // called "Test"
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) // Select
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Gu (rock)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Choki (scissors)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // Pa (paper)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")   // A/D
	PORT_BIT( 0x7fff, 0x0000, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_START("IN2")   // DSW
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Time To Hit" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "2 s" )
	PORT_DIPSETTING(      0x0008, "3 s" )
	PORT_DIPSETTING(      0x0004, "4 s" )
	PORT_DIPSETTING(      0x0000, "5 s" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW3:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( scudhamm )
	PORT_INCLUDE( scudhamma )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x0800, 0x0800, "Win Up" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/**************************************************************************
                            Arm Champs II
**************************************************************************/

ioport_value armchamp2_state::left_sensor_r()
{
	const int arm_x = m_io_arm->read();
	return (arm_x < 0x40);
}

ioport_value armchamp2_state::right_sensor_r()
{
	const int arm_x = m_io_arm->read();
	return (arm_x > 0xc0);
}

ioport_value armchamp2_state::center_sensor_r()
{
	const int arm_x = m_io_arm->read();
	return ((arm_x > 0x60) && (arm_x < 0xa0));
}


static INPUT_PORTS_START( armchmp2 )
	PORT_START("IN0")   // Buttons + Sensors
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(armchamp2_state::left_sensor_r))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(armchamp2_state::right_sensor_r))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(armchamp2_state::center_sensor_r))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) // hard
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) // easy
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) // elbow (it always complains though)

	PORT_START("ARM")   // A/D
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN2")   // DSW
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x000c, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(      0x0000, "15 s" )
	PORT_DIPSETTING(      0x0030, "20 s" )
	PORT_DIPSETTING(      0x0020, "25 s" )
	PORT_DIPSETTING(      0x0010, "30 s" )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Region ) )   // some text is in japanese regardless!
	PORT_DIPSETTING(      0x00c0, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Europe ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Demo Arm" )
	PORT_DIPSETTING(    0x80, "Center" )
	PORT_DIPSETTING(    0x00, "Side" )
INPUT_PORTS_END


/**************************************************************************
                                Captain Flag
**************************************************************************/

static INPUT_PORTS_START( captflag )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP   ) PORT_NAME("Left Red Stick Up")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("Left Red Stick Down")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP   ) PORT_NAME("Right White Stick Up")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("Right White Stick Down")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2    ) PORT_IMPULSE(1) // coin 2
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Decide Button") // decide
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(1) // coin 1
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE  ) // test
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Select Button") // select
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH,IPT_OUTPUT   ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r)) // prize sensor
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // potery on schems?

	PORT_START("SW1_2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Qualify" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "30/40" )
	PORT_DIPSETTING(      0x00c0, "30/50" )
	PORT_DIPSETTING(      0x0080, "40/50" )
	PORT_DIPSETTING(      0x0040, "50/50" )

	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("SW01")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(captflag_state::motor_pos_r<LEFT>))
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(captflag_state::motor_pos_r<RIGHT>))

	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(captflag_state::motor_busy_r<LEFT>))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(captflag_state::motor_busy_r<RIGHT>))

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW01:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW01:3")
	PORT_DIPSETTING(      0x0400, "3/4" )
	PORT_DIPSETTING(      0x0000, "4/5" )
	PORT_DIPNAME( 0x0800, 0x0800, "BGM Volume" ) PORT_DIPLOCATION("SW01:4")
	PORT_DIPSETTING(      0x0000, "Low" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW01:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0xe000, 0xe000, "Capsule Payout" ) PORT_DIPLOCATION("SW01:6,7,8")
	PORT_DIPSETTING(      0x0000, "Nothing" )
	PORT_DIPSETTING(      0xc000, "05%" )
	PORT_DIPSETTING(      0xe000, "10%" )
	PORT_DIPSETTING(      0xa000, "15%" )
	PORT_DIPSETTING(      0x8000, "20%" )
	PORT_DIPSETTING(      0x6000, "30%" )
	PORT_DIPSETTING(      0x4000, "50%" )
	PORT_DIPSETTING(      0x2000, "All" )
INPUT_PORTS_END

static INPUT_PORTS_START( vscaptfl )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP   ) PORT_NAME("P1 Left Red Stick Up") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P1 Left Red Stick Down") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP   ) PORT_NAME("P1 Right White Stick Up") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P1 Right White Stick Down") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2    ) PORT_IMPULSE(1) // coin 2
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Decide Button") // decide
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(1) // coin 1
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE  ) // test
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Select Button") // select
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH,IPT_OUTPUT   ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r)) // prize sensor
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // potery on schems?

	PORT_START("BUTTONS2")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(captflag_state::motor_pos_r<LEFT>))
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(captflag_state::motor_pos_r<RIGHT>))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(captflag_state::motor_busy_r<LEFT>))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(captflag_state::motor_busy_r<RIGHT>))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("P2 Left Red Stick Up") PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("P2 Left Red Stick Down") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("P2 Right White Stick Up") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("P2 Right White Stick Down") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW01")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW01:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Mode") PORT_DIPLOCATION("SW01:3")
	PORT_DIPSETTING(      0x0004, "1 set" )
	PORT_DIPSETTING(      0x0000, "3 set" )
	PORT_DIPNAME( 0x0008, 0x0008, "BGM Volume" ) PORT_DIPLOCATION("SW01:4")
	PORT_DIPSETTING(      0x0000, "Low" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW01:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, "Capsule Payout" ) PORT_DIPLOCATION("SW01:6,7,8")
	PORT_DIPSETTING(      0x0000, "Nothing" )
	PORT_DIPSETTING(      0x00c0, "05%" )
	PORT_DIPSETTING(      0x00e0, "10%" )
	PORT_DIPSETTING(      0x00a0, "15%" )
	PORT_DIPSETTING(      0x0080, "20%" )
	PORT_DIPSETTING(      0x0060, "30%" )
	PORT_DIPSETTING(      0x0040, "50%" )
	PORT_DIPSETTING(      0x0020, "All" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW02:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW02:4,5,6")
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Qualify" ) PORT_DIPLOCATION("SW02:7,8")
	PORT_DIPSETTING(      0x0000, "30/40" )
	PORT_DIPSETTING(      0xc000, "30/50" )
	PORT_DIPSETTING(      0x8000, "40/50" )
	PORT_DIPSETTING(      0x4000, "50/50" )

	PORT_START("SW1_2")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW1:8" )

	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END

/**************************************************************************


                                Gfx Layouts


**************************************************************************/

/**************************************************************************
                                Scud Hammer
**************************************************************************/

static GFXDECODE_START( gfx_scudhamm )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x3000/2, 128 )   // [0] sprites
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


**************************************************************************/

/**************************************************************************
                                Scud Hammer
**************************************************************************/

/*
    1, 5-7]     busy loop
    2]          clr.w   $fc810.l + rte
    3]          game
    4]          == 3
*/

TIMER_DEVICE_CALLBACK_MEMBER(scudhamm_state::scudhamm_scanline)
{
	const int scanline = param;

	if (m_screen->frame_number() & 1)
		return;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == 16) // clears a flag, sprite end DMA?
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void scudhamm_state::scudhamm(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &scudhamm_state::scudhamm_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(scudhamm_state::scudhamm_scanline), "screen", 0, 1);

	adc0804_device &adc(ADC0804(config, "adc", 640000)); // unknown clock
	adc.vin_callback().set(FUNC(scudhamm_state::scudhamm_analog_r));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	// measured values for Arm Champs II: VSync: 59.1784Hz, HSync: 15082.0 kHz
	m_screen->set_raw(XTAL(12'000'000)/2,396,0,256,256,16,240);
//  m_screen->set_refresh_hz(30); //TODO: wrong!
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500 * 3) /* not accurate */);
//  m_screen->set_size(256, 256);
//  m_screen->set_visarea(0, 256-1, 0 +16, 256-1 -16);
	m_screen->set_screen_update(FUNC(scudhamm_state::screen_update));
	m_screen->set_palette(m_palette);

	JALECO_ZOOMSPR(config, m_spritegen, m_palette, gfx_scudhamm);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x8000/2);
	m_palette->enable_shadows();

	MEGASYS1_TILEMAP(config, m_tmap[0], m_palette, 0x1e00/2);
	MEGASYS1_TILEMAP(config, m_tmap[1], m_palette, 0x4e00/2);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	OKIM6295(config, m_oki[0], 4000000/2, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	OKIM6295(config, m_oki[1], 4000000/2, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


/**************************************************************************
                            Arm Champs II
**************************************************************************/

// TODO: intentionally duplicate scudhamm_scanline fn
// armchamp2_state shouldn't derive from scudhamm_state but share this as a common device,
// also assuming it really is using the same irq config as well ...
TIMER_DEVICE_CALLBACK_MEMBER(armchamp2_state::armchamp2_scanline)
{
	const int scanline = param;

	if (m_screen->frame_number() & 1)
		return;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == 16) // does a bit more than scudhammer
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void armchamp2_state::armchmp2(machine_config &config)
{
	scudhamm(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &armchamp2_state::armchmp2_map);
	subdevice<timer_device>("scantimer")->set_callback(FUNC(armchamp2_state::armchamp2_scanline));
	subdevice<adc0804_device>("adc")->vin_callback().set(FUNC(armchamp2_state::analog_r));
}


/**************************************************************************
                                Captain Flag
**************************************************************************/

/*
    1]          no-op
    2]          copy text buffer to tilemap, sound (~50 scanlines at 30Hz)
    3]          I/O, sound (~5 scanlines at 30Hz). Sets a flag to update text buffer and sprites
    4-7]        rte
*/

TIMER_DEVICE_CALLBACK_MEMBER(captflag_state::captflag_scanline)
{
	const int scanline = param;

	if (scanline == 240) // vblank: draw screen
		m_maincpu->set_input_line(2, HOLD_LINE);

	if (scanline == 50)
		m_maincpu->set_input_line(3, HOLD_LINE);
}

void captflag_state::captflag(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);  // TMP68000P-12
	m_maincpu->set_addrmap(AS_PROGRAM, &captflag_state::captflag_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(captflag_state::captflag_scanline), "screen", 0, 1);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(2000));

	WATCHDOG_TIMER(config, m_watchdog);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_refresh_hz(30); //TODO: wrong!
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500 * 3) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0 +16, 256-1 -16);
	m_screen->set_screen_update(FUNC(captflag_state::screen_update));
	m_screen->set_palette(m_palette);

	JALECO_ZOOMSPR(config, m_spritegen, m_palette, gfx_scudhamm);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x8000/2);
	m_palette->enable_shadows();

	MEGASYS1_TILEMAP(config, m_tmap[0], m_palette, 0x1e00/2);
	MEGASYS1_TILEMAP(config, m_tmap[1], m_palette, 0x4e00/2);

	// Motors
	TIMER(config, m_motor_left).configure_generic(nullptr);
	TIMER(config, m_motor_right).configure_generic(nullptr);

	// Layout
	config.set_default_layout(layout_captflag);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	OKIM6295(config, m_oki[0], 4000000/2, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[0]->set_addrmap(0, &captflag_state::oki_map<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	OKIM6295(config, m_oki[1], 4000000/2, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[1]->set_addrmap(0, &captflag_state::oki_map<1>);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


/***************************************************************************


                                ROMs Loading


**************************************************************************/

/***************************************************************************

                                Scud Hammer

CF-92128B:

                                                      GS9001501
 2-H 2-L  6295            62256 62256
 1-H 1-L  6295  68000-12  3     4       6  GS90015-02 8464 8464 GS600406

                    24MHz               5  GS90015-02 8464 8464 GS900406

                                                       7C199
                                                       7C199 GS90015-03

GP-9189:

 1     2      62256                            62256
 3     4      62256    GS90015-06 GS90015-06   62256
 5     6      62256                            62256
 7     8      62256    GS90015-03 GS90015-03   62256
 9     10

                  GS90015-08            GS90015-07 GS90015-10

          GS90015-11


                      MR90015-35
                      MR90015-35              GS90015-09

***************************************************************************/

ROM_START( scudhamm )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "scud_hammer_cf-92128b-3_ver.1.4.bin", 0x000000, 0x040000, CRC(4747370e) SHA1(d822e949d444d3fe4b95edb58fa02b6b7a1a64e6) )
	ROM_LOAD16_BYTE( "scud_hammer_cf-92128b-4_ver.1.4.bin", 0x000001, 0x040000, CRC(7a04955c) SHA1(57bbb3d481ebe9ae99d8ffae21984d53bdfe574a) )

	ROM_REGION( 0x080000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "5", 0x000000, 0x080000, CRC(714c115e) SHA1(c3e88b3972e3926f37968f3e84b932e1ac177142) )

	ROM_REGION( 0x020000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "6", 0x000000, 0x020000, CRC(b39aab63) SHA1(88275cce8b1323b2d835390a8fc2380b90d50d95) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x500000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "1.bot",  0x000000, 0x080000, CRC(46450d73) SHA1(c9acdf1cef760e5194c346d721e859c61afbfce6) )
	ROM_LOAD16_BYTE( "2.bot",  0x000001, 0x080000, CRC(fb7b66dd) SHA1(ad6bbae4fa72f957e5c0fc7bf6199ac45f837dac) )
	ROM_LOAD16_BYTE( "3.bot",  0x100000, 0x080000, CRC(7d45960b) SHA1(abf59cf85f28c90d4c08e3a1e5408a9a700071cc) )
	ROM_LOAD16_BYTE( "4.bot",  0x100001, 0x080000, CRC(393b6a22) SHA1(0d002a8c09de2fb8aaa7f5f020badc6fc096fa41) )
	ROM_LOAD16_BYTE( "5.bot",  0x200000, 0x080000, CRC(7a3c33ad) SHA1(fe0e3722e15919ae3acfeeacae57716aae43647c) )
	ROM_LOAD16_BYTE( "6.bot",  0x200001, 0x080000, CRC(d19c4bf7) SHA1(b8aa21920d5a02f10a7ae65ade8a0a88ad23f373) )
	ROM_LOAD16_BYTE( "7.bot",  0x300000, 0x080000, CRC(9e5edf59) SHA1(fcd4136e39d40bcce365153c96e06181a24a480a) )
	ROM_LOAD16_BYTE( "8.bot",  0x300001, 0x080000, CRC(4980051e) SHA1(10de91239b5b4dab8e7fa4bf51d93356c5111ddf) )
	ROM_LOAD16_BYTE( "9.bot",  0x400000, 0x080000, CRC(c1b301f1) SHA1(776b9889703d73afc4fb0ff77498b98c943246d3) )
	ROM_LOAD16_BYTE( "10.bot", 0x400001, 0x080000, CRC(dab4528f) SHA1(f5ddc37a2d106d5438ad1b7d23a2bbbce07f2c89) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "2.l",  0x000000, 0x080000, CRC(889311da) SHA1(fcaee3e6c98a784cfde06fc2e0e8f5abbfb4df6c) )
	ROM_LOAD( "2.h",  0x080000, 0x080000, CRC(347928fc) SHA1(36903c38b9f13594de40dfc697326327c7010d65) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "1.l",  0x000000, 0x080000, CRC(3c94aa90) SHA1(f9278fec9d93dac0309f30e35c727bd481f347d4) )
	ROM_LOAD( "1.h",  0x080000, 0x080000, CRC(5caee787) SHA1(267f4d3c28e71e53180a5d0ff27a6555ac6fa4a0) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

ROM_START( scudhamma )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "3", 0x000000, 0x040000, CRC(a908e7bd) SHA1(be0a8f959ab5c19122eee6c3def6137f37f1a9c6) )
	ROM_LOAD16_BYTE( "4", 0x000001, 0x040000, CRC(981c8b02) SHA1(db6c8993bf1c3993ab31dd649022ab76169975e1) )

	ROM_REGION( 0x080000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "5", 0x000000, 0x080000, CRC(714c115e) SHA1(c3e88b3972e3926f37968f3e84b932e1ac177142) )

	ROM_REGION( 0x020000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "6", 0x000000, 0x020000, CRC(b39aab63) SHA1(88275cce8b1323b2d835390a8fc2380b90d50d95) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x500000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "1.bot",  0x000000, 0x080000, CRC(46450d73) SHA1(c9acdf1cef760e5194c346d721e859c61afbfce6) )
	ROM_LOAD16_BYTE( "2.bot",  0x000001, 0x080000, CRC(fb7b66dd) SHA1(ad6bbae4fa72f957e5c0fc7bf6199ac45f837dac) )
	ROM_LOAD16_BYTE( "3.bot",  0x100000, 0x080000, CRC(7d45960b) SHA1(abf59cf85f28c90d4c08e3a1e5408a9a700071cc) )
	ROM_LOAD16_BYTE( "4.bot",  0x100001, 0x080000, CRC(393b6a22) SHA1(0d002a8c09de2fb8aaa7f5f020badc6fc096fa41) )
	ROM_LOAD16_BYTE( "5.bot",  0x200000, 0x080000, CRC(7a3c33ad) SHA1(fe0e3722e15919ae3acfeeacae57716aae43647c) )
	ROM_LOAD16_BYTE( "6.bot",  0x200001, 0x080000, CRC(d19c4bf7) SHA1(b8aa21920d5a02f10a7ae65ade8a0a88ad23f373) )
	ROM_LOAD16_BYTE( "7.bot",  0x300000, 0x080000, CRC(9e5edf59) SHA1(fcd4136e39d40bcce365153c96e06181a24a480a) )
	ROM_LOAD16_BYTE( "8.bot",  0x300001, 0x080000, CRC(4980051e) SHA1(10de91239b5b4dab8e7fa4bf51d93356c5111ddf) )
	ROM_LOAD16_BYTE( "9.bot",  0x400000, 0x080000, CRC(c1b301f1) SHA1(776b9889703d73afc4fb0ff77498b98c943246d3) )
	ROM_LOAD16_BYTE( "10.bot", 0x400001, 0x080000, CRC(dab4528f) SHA1(f5ddc37a2d106d5438ad1b7d23a2bbbce07f2c89) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "2.l",  0x000000, 0x080000, CRC(889311da) SHA1(fcaee3e6c98a784cfde06fc2e0e8f5abbfb4df6c) )
	ROM_LOAD( "2.h",  0x080000, 0x080000, CRC(347928fc) SHA1(36903c38b9f13594de40dfc697326327c7010d65) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "1.l",  0x000000, 0x080000, CRC(3c94aa90) SHA1(f9278fec9d93dac0309f30e35c727bd481f347d4) )
	ROM_LOAD( "1.h",  0x080000, 0x080000, CRC(5caee787) SHA1(267f4d3c28e71e53180a5d0ff27a6555ac6fa4a0) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END


/***************************************************************************

Arm Champs II
Jaleco, 1992

An arm wrestling game from Jaleco. Game is part mechanical, part video. The video part
is shown on a vertical screen, being your opponent you are wrestling. The mechanical
part is an arm you force against to pin your opponent's 'arm'. The mechanical arm is powered
by a motor. The arm probably has a 5k potentiometer connected to it to give feedback to the
hardware about the position of the mechanical arm.
The opponents get progressively harder and harder, it's almost impossible to beat the final
few opponents unless you have a few friends handy to swing on the arm ;-))
The hardware appears to be similar to Jaleco's 'Grand Prix Star' etc, using many of
the same custom IC's, and even some of the same ROMs.


PCB Layouts
-----------

(top board)
EB91042-20048-3
AC-91106B
|---------------------------------------------------------------------------|
| uPC1318                                         GS9001501   TL7705        |
|          |-------------|                                                  |
|POWER     |   68000-12  |                                  PR88004Q_8.IC102|
|          |-------------|                                                  |
|                         24MHz                        AC91106_VER1.2_7.IC99|
|                                                                           |
|                                                 GS9000406    GS90015-02   |
|                                                                          |-|
|                                                   6264          6264     | |
|                                     AC91106_VER1.7_4.IC63                | |
|C                                                                         | |
|U                                    AC91106_VER1.7_3.IC62                | |
|S                                                                         | |
|T                                                    MR91042-07-R66_6.IC95| |
|O                                       62256                             |-|
|M                                                                          |
|7                                       62256    GS9000406    GS90015-02   |
|2                     ADC0804                                              |
|P                                                               6264       |
|I                                                                         |-|
|N                                                GS90015-03     6264      | |
|                                                                          | |
|                                 M6295                     PR91042_5.IC91 | |
|                                      MR91042-08_2.IC57                   | |
|                                                                          | |
|                                 M6295                                    | |
|                PMI_PM7528            AC91106_VER1.2_1.IC56               | |
|                                                     62256                | |
|                                                     62256                | |
| POWER     DSW1   DSW2            4MHz                                    |-|
|---------------------------------------------------------------------------|
Notes:
      68000 clock       - 12.0MHz [24/2]
      Jaleco Custom ICs -
                         GS9001501 (QFP44)
                         GS9000406 (QFP80, x2)
                         GS90015-02 (QFP100, x2)
                         GS90015-03 (QFP80)

      ROMs -
            PR88004Q_8.IC102      - 82S147 PROM
            AC91106_VER1.2_7.IC99 - 27C010 EPROM
            MR91042007-R66_6.IC95 - 4M mask ROM
            AC91106_VER1.7_4.IC63 - 27C010 EPROM
            AC91106_VER1.7_3.IC62 - 27C010 EPROM
            MR91042-08_2.IC57     - 4M mask ROM
            AC91106_VER1.2_1.IC56 - 27C4001 EPROM
            PR91042_5.IC91        - 82S129 PROM


(lower board)
GP-9189
EB91015-20038
|---------------------------------------------------------------------------|
|MR91042-01-R60_1.IC1                                                       |
|        MR91042-02-R61_2.IC2    62256                             62256    |
|                                       GS90015-06    GS90015-06            |
|MR91042-03-R62_3.IC5                                                       |
|        MR91042-04-R63_4.IC6    62256                             62256    |
|                                                                           |
|MR91042-05-R64_5.IC11                                                      |
|        MR91042-06-R65_6.IC12   62256                             62256   |-|
|                                       GS90015-03    GS90015-03           | |
|                                                                          | |
|                                62256                             62256   | |
|                                                                          | |
|                                                                          | |
|                        GS90015-08                                        | |
|                                               GS90015-07   GS90015-10    |-|
|                                                                           |
|                                                                           |
| CH9072-5_11.IC33                                                          |
|              GS90015-11              CH9072-6_12.IC35                     |
|                                      CH9072-4_13.IC39                    |-|
|                                                                          | |
|                                                                          | |
|                          MR90015-35-W33_14.IC54                          | |
|                                                                          | |
|      CH9072-8_15.IC59    MR90015-35-W33_17.IC67                          | |
|                                                    GS90015-09            | |
|              PR88004W_16.IC66                                            | |
|                                                                          | |
|                                                      6116                | |
| POWER                                                6116                |-|
|---------------------------------------------------------------------------|
Notes:
      Jaleco Custom ICs -
                         GS90015-03 (QFP80, x2)
                         GS90015-06 (QFP100, x2)
                         GS90015-07 (QFP64)
                         GS90015-08 (QFP64)
                         GS90015-09 (QFP64)
                         GS90015-10 (QFP64)
                         GS90015-11 (QFP100)

      ROMs -
            PR88004W_16.IC66      - 82S135 PROM
            CH9072-4_13.IC39      - Atmel AT27HC642R
            CH9072-5_11.IC33      - Atmel AT27HC642R
            CH9072-6_12.IC35      - Atmel AT27HC642R
            CH9072-8_15.IC59      - Atmel AT27HC642R
            MR90015-35-W33_14.IC54- 4M mask ROM  \
            MR90015-35-W33_17.IC67- 4M mask ROM  / Contain same data
            MR91042-01-R60_1.IC1  - 4M mask ROM
            MR91042-02-R61_2.IC2  - 4M mask ROM
            MR91042-03-R62_3.IC5  - 4M mask ROM
            MR91042-04-R63_4.IC6  - 4M mask ROM
            MR91042-05-R64_5.IC11 - 4M mask ROM
            MR91042-06-R65_6.IC12 - 4M mask ROM

***************************************************************************/

ROM_START( armchmp2 )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "4_ver_2.7.ic63", 0x000000, 0x020000, CRC(e0cec032) SHA1(743b022b6de3efb045c4f1cca49caed0259ccfff) ) // same rom contents as ver 2.6, it appears to be correct;
	ROM_LOAD16_BYTE( "3_ver_2.7.ic62", 0x000001, 0x020000, CRC(44186a37) SHA1(d21fbba11e9c82f48de2a699011ca7f3b90061ba) ) // modifications are in data area, where upper bytes are 00

	ROM_REGION( 0x080000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr91042-07-r66_6.ic95",  0x000000, 0x080000, CRC(d1be8699) SHA1(67563761f95892b08c7113ab1c52ab5aa7118fb8) )

	ROM_REGION( 0x020000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "ac91106_ver1.2_7.ic99", 0x000000, 0x020000, CRC(09755aef) SHA1(39c901fb9408a0ba488f0112d7f48b929b092e3b) )

	ROM_REGION( 0x300000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr91042-01-r60_1.ic1",  0x000000, 0x080000, CRC(fdfe6951) SHA1(ba6c5cd5d16fdca6f131302b19e621f8abe8136a) )
	ROM_LOAD16_BYTE( "mr91042-02-r61_2.ic2",  0x000001, 0x080000, CRC(2e6c8b30) SHA1(70503fec251606b37fea2c7f91e682aece252035) )
	ROM_LOAD16_BYTE( "mr91042-03-r62_3.ic5",  0x100000, 0x080000, CRC(07ba6d3a) SHA1(9c58e3a1931b593448c53a59e7f5b9aaac40ff88) )
	ROM_LOAD16_BYTE( "mr91042-04-r63_4.ic6",  0x100001, 0x080000, CRC(f37cb12c) SHA1(282ebbd795284d7efa335b797ca1eedc1110e9da) )
	ROM_LOAD16_BYTE( "mr91042-05-r64_5.ic11", 0x200000, 0x080000, CRC(7a3bb52d) SHA1(7f9d1dad4c89e6b55415b082363bc261115e9f96) )
	ROM_LOAD16_BYTE( "mr91042-06-r65_6.ic12", 0x200001, 0x080000, CRC(5312a4f2) SHA1(4dcd2839bb5acccecf1eb6c0e19e877a0cff6875) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "mr91042-08_2.ic57",     0x000000, 0x080000, CRC(dc015f6c) SHA1(9d0677c50a25be1d11d43e54dbf3005f18b79b66) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "ac-91106v2.0_1.ic56", 0x000000, 0x080000, CRC(0ff5cbcf) SHA1(25ef8d67749ca78afc4c13a31b3f7a87284947c1) )
	ROM_RELOAD(                      0x080000, 0x080000 )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072-4_13.ic39", 0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072-5_11.ic33", 0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072-6_12.ic35", 0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072-8_15.ic59", 0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-35-w33_17.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35-w33_14.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q_8.ic102", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w_16.ic66", 0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr91042_5.ic91", 0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
ROM_END

ROM_START( armchmp2o2 )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "ac-91106v2.6_4.ic63", 0x000000, 0x020000, CRC(e0cec032) SHA1(743b022b6de3efb045c4f1cca49caed0259ccfff) )
	ROM_LOAD16_BYTE( "ac-91106v2.6_3.ic62", 0x000001, 0x020000, CRC(5de6da19) SHA1(1f46056596924789394ad2d99ec2d7fcb7845d3c) )

	ROM_REGION( 0x080000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr91042-07-r66_6.ic95",  0x000000, 0x080000, CRC(d1be8699) SHA1(67563761f95892b08c7113ab1c52ab5aa7118fb8) )

	ROM_REGION( 0x020000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "ac91106_ver1.2_7.ic99", 0x000000, 0x020000, CRC(09755aef) SHA1(39c901fb9408a0ba488f0112d7f48b929b092e3b) )

	ROM_REGION( 0x300000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr91042-01-r60_1.ic1",  0x000000, 0x080000, CRC(fdfe6951) SHA1(ba6c5cd5d16fdca6f131302b19e621f8abe8136a) )
	ROM_LOAD16_BYTE( "mr91042-02-r61_2.ic2",  0x000001, 0x080000, CRC(2e6c8b30) SHA1(70503fec251606b37fea2c7f91e682aece252035) )
	ROM_LOAD16_BYTE( "mr91042-03-r62_3.ic5",  0x100000, 0x080000, CRC(07ba6d3a) SHA1(9c58e3a1931b593448c53a59e7f5b9aaac40ff88) )
	ROM_LOAD16_BYTE( "mr91042-04-r63_4.ic6",  0x100001, 0x080000, CRC(f37cb12c) SHA1(282ebbd795284d7efa335b797ca1eedc1110e9da) )
	ROM_LOAD16_BYTE( "mr91042-05-r64_5.ic11", 0x200000, 0x080000, CRC(7a3bb52d) SHA1(7f9d1dad4c89e6b55415b082363bc261115e9f96) )
	ROM_LOAD16_BYTE( "mr91042-06-r65_6.ic12", 0x200001, 0x080000, CRC(5312a4f2) SHA1(4dcd2839bb5acccecf1eb6c0e19e877a0cff6875) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "mr91042-08_2.ic57",     0x000000, 0x080000, CRC(dc015f6c) SHA1(9d0677c50a25be1d11d43e54dbf3005f18b79b66) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "ac-91106v2.0_1.ic56", 0x000000, 0x080000, CRC(0ff5cbcf) SHA1(25ef8d67749ca78afc4c13a31b3f7a87284947c1) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072-4_13.ic39", 0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072-5_11.ic33", 0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072-6_12.ic35", 0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072-8_15.ic59", 0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-35-w33_17.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35-w33_14.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q_8.ic102", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w_16.ic66", 0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr91042_5.ic91", 0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
ROM_END

ROM_START( armchmp2o )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "ac91106_ver1.7_4.ic63", 0x000000, 0x020000, CRC(aaa11bc7) SHA1(ac6186f45a006074d3a86d7437c5a3411bf27188) )
	ROM_LOAD16_BYTE( "ac91106_ver1.7_3.ic62", 0x000001, 0x020000, CRC(a7965879) SHA1(556fecd6ea0f977b718d132c4180bb2160b9da7e) )

	ROM_REGION( 0x080000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr91042-07-r66_6.ic95",  0x000000, 0x080000, CRC(d1be8699) SHA1(67563761f95892b08c7113ab1c52ab5aa7118fb8) )

	ROM_REGION( 0x020000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "ac91106_ver1.2_7.ic99", 0x000000, 0x020000, CRC(09755aef) SHA1(39c901fb9408a0ba488f0112d7f48b929b092e3b) )

	ROM_REGION( 0x300000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr91042-01-r60_1.ic1",  0x000000, 0x080000, CRC(fdfe6951) SHA1(ba6c5cd5d16fdca6f131302b19e621f8abe8136a) )
	ROM_LOAD16_BYTE( "mr91042-02-r61_2.ic2",  0x000001, 0x080000, CRC(2e6c8b30) SHA1(70503fec251606b37fea2c7f91e682aece252035) )
	ROM_LOAD16_BYTE( "mr91042-03-r62_3.ic5",  0x100000, 0x080000, CRC(07ba6d3a) SHA1(9c58e3a1931b593448c53a59e7f5b9aaac40ff88) )
	ROM_LOAD16_BYTE( "mr91042-04-r63_4.ic6",  0x100001, 0x080000, CRC(f37cb12c) SHA1(282ebbd795284d7efa335b797ca1eedc1110e9da) )
	ROM_LOAD16_BYTE( "mr91042-05-r64_5.ic11", 0x200000, 0x080000, CRC(7a3bb52d) SHA1(7f9d1dad4c89e6b55415b082363bc261115e9f96) )
	ROM_LOAD16_BYTE( "mr91042-06-r65_6.ic12", 0x200001, 0x080000, CRC(5312a4f2) SHA1(4dcd2839bb5acccecf1eb6c0e19e877a0cff6875) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "mr91042-08_2.ic57",     0x000000, 0x080000, CRC(dc015f6c) SHA1(9d0677c50a25be1d11d43e54dbf3005f18b79b66) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (4x40000) */
	ROM_LOAD( "ac91106_ver1.2_1.ic56", 0x000000, 0x080000, CRC(48208b69) SHA1(5dfcc7744f7cdd0326886a4a841755ab6ec5482b) )
	ROM_RELOAD(                        0x080000, 0x080000 )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "ch9072-4_13.ic39", 0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072-5_11.ic33", 0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072-6_12.ic35", 0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072-8_15.ic59", 0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )

	ROM_LOAD( "mr90015-35-w33_17.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35-w33_14.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_LOAD( "pr88004q_8.ic102", 0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr88004w_16.ic66", 0x000000, 0x000100, CRC(3d648467) SHA1(bf8dbaa2176c801f7370313425c87f0eefe8a3a4) )

	ROM_LOAD( "pr91042_5.ic91", 0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
ROM_END

/***************************************************************************

                                Captain Flag

**************************************************************************/

ROM_START( captflag )
	ROM_REGION( 0x80000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "cf-92128a_3_ver1.4.ic40", 0x000000, 0x020000, CRC(e62af6ae) SHA1(38eb581def468860e4705f25088550799303a9aa) )
	ROM_LOAD16_BYTE( "cf-92128_4_ver1.4.ic46",  0x000001, 0x020000, CRC(e773f87f) SHA1(cf9d72b0df256b69b96f1cd6b5f86282801873e3) )

	ROM_REGION( 0x80000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr92027-11_w89.ic54", 0x000000, 0x080000, CRC(d34cae3c) SHA1(622ad4645df12d34e55bbfb7194508957bb2198b) ) // 5 on the PCB

	ROM_REGION( 0x20000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "cf-92128_6.ic55", 0x000000, 0x020000, CRC(12debfc2) SHA1(f28d3f63a3c8965fcd838eedad4ef3682a28da0d) ) // 6 on the PCB

	ROM_REGION( 0x400000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "gp-9189_1.ic1",       0x000000, 0x080000, CRC(03d69f0f) SHA1(97a0552d94ca1e9c76896903c02c3f005752e5db) )
	ROM_LOAD16_BYTE( "gp-9189_2.ic2",       0x000001, 0x080000, CRC(fbfba282) SHA1(65735d8f6abdb5816b8b26ce2969959a0f2e2c7d) )
	ROM_LOAD16_BYTE( "mr92027-08_w85.ic5",  0x100000, 0x080000, CRC(1e02abff) SHA1(497aeab40c778ef6d8a76b005530fea5f4f68ab8) )
	ROM_LOAD16_BYTE( "mr92027-04_w86.ic6",  0x100001, 0x080000, CRC(89dfe3e8) SHA1(f069cf55d08e5901699e7ee7c6b2b5cdba031cf2) )
	ROM_LOAD16_BYTE( "gp-9189_5.ic11",      0x200000, 0x080000, CRC(edc33285) SHA1(a23f9a1877108eaaef6d467ae21433fe5f24261f) )
	ROM_LOAD16_BYTE( "gp-9189_6.ic12",      0x200001, 0x080000, CRC(99b8f6d8) SHA1(e01c85861d5a131bb31908460cb3bc9eb0ea045c) )
	ROM_LOAD16_BYTE( "mr92027-07_w87.ic15", 0x300000, 0x080000, CRC(07e49754) SHA1(c9fcd394badba104508e4c5ec5cf5513ecb2d69e) )
	ROM_LOAD16_BYTE( "mr92027-08_w88.ic16", 0x300001, 0x080000, CRC(fb080dd6) SHA1(49eceba8cdce76dec3f6a85327135125bb0910f0) )

	ROM_REGION( 0x80000, "user2", 0 )       /* ? Unused ROMs ? */
	ROM_LOAD( "pr91042.ic86",        0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) ) // FIXED BITS (00000xxx0000xxxx)
	ROM_LOAD( "pr88004q.ic88",       0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) ) // FIXED BITS (1xxxxxxx1111x1xx)
	ROM_LOAD( "pr92027a.ic16",       0x000000, 0x000020, CRC(bee7adc6) SHA1(cd11a3dae0317d06a69b5707a653b8997c1eb97f) ) // FIXED BITS (0000000000000xxx), 1xxx0 = 0x00
	ROM_LOAD( "pr92027a.ic17",       0x000000, 0x000020, CRC(bee7adc6) SHA1(cd11a3dae0317d06a69b5707a653b8997c1eb97f) ) // ""
	ROM_LOAD( "pr92027b.ic32.bin",   0x000000, 0x000113, CRC(483f4fb5) SHA1(84bb0300a106261634c921a37858482d3233c05a) )
	ROM_LOAD( "pr92027b.ic32.jedec", 0x000000, 0x000bd0, CRC(f0ed1845) SHA1(203438fdee05810b2265624e1a1fdd55d360f833) )
	ROM_LOAD( "pr92027b.ic36.bin",   0x000000, 0x000113, CRC(483f4fb5) SHA1(84bb0300a106261634c921a37858482d3233c05a) )
	ROM_LOAD( "pr92027b.ic36.jedec", 0x000000, 0x000bd0, CRC(f0ed1845) SHA1(203438fdee05810b2265624e1a1fdd55d360f833) )

	ROM_LOAD( "ch9072_4.ic39",       0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) ) // FIXED BITS (0000000x), 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "ch9072_5.ic33",       0x000000, 0x001000, CRC(a8025dc1) SHA1(c9bb7ea59bba3041c687b449ff1560d7d1ce2ec9) ) // FIXED BITS (xxxx0xxx)
	ROM_LOAD( "ch9072_6.ic35",       0x000000, 0x001000, CRC(5cc9c561) SHA1(10866fd0707498fe4d4415bf755c07b55af4ae18) )
	ROM_LOAD( "ch9072_8.ic59",       0x000000, 0x001000, CRC(6c99523b) SHA1(cc00b326b69a97b5bd2e2d741ab41692a14eae35) ) // FIXED BITS (0xxx0xxx)

	ROM_LOAD( "mr90015-35_w33.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) ) // not dumped yet (taken from the other games)
	ROM_LOAD( "mr90015-35_w33.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) ) // ""

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (8x20000) */
	ROM_LOAD( "mr92027-10_w27.ic19", 0x000000, 0x100000, CRC(04bd729e) SHA1(92bcedf16554f33cc3d0dbdd8807b0e2fafe5d7c) ) // 2 on the PCB

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (8x20000) */
	ROM_LOAD( "mr92027-09_w26.ic18", 0x000000, 0x100000, CRC(3aaa332a) SHA1(6c19364069e0b077a07ac4f9c4b0cf0c0985a42a) ) // 1 on the PCB
ROM_END


/***************************************************************************

                         Vs. Super Captain Flag

 PCB IDs:

 CF-93153 EB93041-20091

 CF-92128B EB92027-20060

 GP-9189 EB90015-20038

**************************************************************************/

ROM_START( vscaptfl )
	ROM_REGION( 0x80000, "maincpu", 0 )        /* Main CPU Code */
	ROM_LOAD16_BYTE( "vs.s.c.f-cf-92128-3_ver.1.3.ic40", 0x000000, 0x040000, CRC(a52595af) SHA1(6a7b3172c256015a0b46394dc1d0bf92ef826a90) )
	ROM_LOAD16_BYTE( "vs.s.c.f-cf-92128-4_ver.1.3.ic46", 0x000001, 0x040000, CRC(1d015f55) SHA1(652d8c3e5c71ae149ff95a008a15d067bcdd8293) )

	ROM_REGION( 0x80000, "scroll0", 0 ) /* Scroll 0 */
	ROM_LOAD( "mr92027_11.ic54", 0x000000, 0x080000, CRC(d34cae3c) SHA1(622ad4645df12d34e55bbfb7194508957bb2198b) )

	ROM_REGION( 0x20000, "scroll1", 0 ) /* Scroll 1 */
	ROM_LOAD( "vs-cf92128_6_ver1.0.ic55", 0x000000, 0x020000, CRC(55c8db06) SHA1(499c0de2202e873eeca98c2f0932569f3df01a21) )

	ROM_REGION( 0x500000, "sprites", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mr92027_01.ic1",           0x000000, 0x080000, CRC(03d69f0f) SHA1(97a0552d94ca1e9c76896903c02c3f005752e5db) )
	ROM_LOAD16_BYTE( "mr92027_02.ic2",           0x000001, 0x080000, CRC(fbfba282) SHA1(65735d8f6abdb5816b8b26ce2969959a0f2e2c7d) )
	ROM_LOAD16_BYTE( "mr92027_03.ic5",           0x100000, 0x080000, CRC(1e02abff) SHA1(497aeab40c778ef6d8a76b005530fea5f4f68ab8) )
	ROM_LOAD16_BYTE( "mr92027_04.ic6",           0x100001, 0x080000, CRC(89dfe3e8) SHA1(f069cf55d08e5901699e7ee7c6b2b5cdba031cf2) )
	ROM_LOAD16_BYTE( "mr92027_05.ic11",          0x200000, 0x080000, CRC(edc33285) SHA1(a23f9a1877108eaaef6d467ae21433fe5f24261f) )
	ROM_LOAD16_BYTE( "mr92027_06.ic12",          0x200001, 0x080000, CRC(99b8f6d8) SHA1(e01c85861d5a131bb31908460cb3bc9eb0ea045c) )
	ROM_LOAD16_BYTE( "vs-gp9189_7_ver1.0.ic15",  0x300000, 0x080000, CRC(63459f84) SHA1(1956ceab56f7266329b4e276876363f2334f0287) )
	ROM_LOAD16_BYTE( "vs-gp9189_8_ver1.0.ic16",  0x300001, 0x080000, CRC(ed40b351) SHA1(9fca6ec3f364ee80729d59e617b2341d39326f2b) )
	ROM_LOAD16_BYTE( "vs-gp9189_9_ver1.0.ic21",  0x400000, 0x080000, CRC(eb20c7d4) SHA1(64956c672fec1e96a97e25144b351bc24c32aa4f) )
	ROM_LOAD16_BYTE( "vs-gp9189_10_ver1.0.ic22", 0x400001, 0x080000, CRC(5e24c3b0) SHA1(714418a48d66782798abfb9394b3a527e63ad9c5) )

	ROM_REGION( 0x80000, "user2", 0 )       /* Unused ROMs */
	// Located on CF-93153
	ROM_LOAD( "pr92027b.ic31",       0x000000, 0x000117, CRC(bebd5200) SHA1(6ce078daf44b7839536dae0e6539388228f4ff47) )
	ROM_LOAD( "pr92027b.ic34",       0x000000, 0x000117, CRC(bebd5200) SHA1(6ce078daf44b7839536dae0e6539388228f4ff47) )
	// Located on CF-92128B
	ROM_LOAD( "pr88004q.ic88",       0x000000, 0x000200, CRC(9327dc37) SHA1(cfe7b144cdcd76170d47f1c4e0f72b6d4fca0c8d) )
	ROM_LOAD( "pr91042.ic86",        0x000000, 0x000100, CRC(e71de4aa) SHA1(d06e5a35ad2127df2d6328cce153073380ee7819) )
	ROM_LOAD( "pr92027a.ic16",       0x000000, 0x000020, CRC(bee7adc6) SHA1(cd11a3dae0317d06a69b5707a653b8997c1eb97f) )
	ROM_LOAD( "pr92027a.ic17",       0x000000, 0x000020, CRC(bee7adc6) SHA1(cd11a3dae0317d06a69b5707a653b8997c1eb97f) )
	// Located on CF-92128B
	ROM_LOAD( "ch9072_4.ic48",       0x000000, 0x002000, CRC(b45b4dc0) SHA1(b9fae0c9ac2d40f0a202c538d866d5f2941ba8dd) )
	ROM_LOAD( "ch9072_5.ic36",       0x000000, 0x002000, CRC(e122916b) SHA1(86d5ecc7ecc6f175ecb28459697ef33e1ee06860) )
	ROM_LOAD( "ch9072_6.ic35",       0x000000, 0x002000, CRC(05d95bf7) SHA1(78181cf71f22c090a1e62823a43757353a9ef6ab) )
	ROM_LOAD( "ch9072_8.ic65",       0x000000, 0x002000, CRC(6bf52596) SHA1(bf4e7e7df3daae4aa6a441b58b15a435aa45630e) )
	ROM_LOAD( "mr90015-35_w33.ic54", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "mr90015-35_w33.ic67", 0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples (8x20000) */
	ROM_LOAD( "vs-cf92128_2.ic19", 0x000000, 0x100000, CRC(7629969c) SHA1(94b391f230f5b5f9498bc967beaed1e6c95a028f) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* Samples (8x20000) */
	ROM_LOAD( "vs-cf92128_1.ic18", 0x000000, 0x100000, CRC(50c99a47) SHA1(c0c4ac24eebdd462176a5cbaf9e33918c6d54e10) )
ROM_END

void captflag_state::init_vscaptfl()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.unmap_readwrite(0x100044, 0x100045);
	space.unmap_readwrite(0x100048, 0x100049);

	space.install_read_port(0x100044, 0x100045, "BUTTONS2");
	space.install_write_handler(0x10004c, 0x10004d, write16s_delegate(*this, FUNC(captflag_state::motor_command_left_w)));
	space.install_write_handler(0x100050, 0x100051, write16s_delegate(*this, FUNC(captflag_state::motor_command_right_w)));
}

} // anonymous namespace 

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1992, armchmp2,  0,        armchmp2, armchmp2, armchamp2_state, empty_init,    ROT270, "Jaleco", "Arm Champs II (ver 2.7)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1992, armchmp2o2,armchmp2, armchmp2, armchmp2, armchamp2_state, empty_init,    ROT270, "Jaleco", "Arm Champs II (ver 2.6)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1992, armchmp2o, armchmp2, armchmp2, armchmp2, armchamp2_state, empty_init,    ROT270, "Jaleco", "Arm Champs II (ver 1.7)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )

GAME( 1993, captflag,  0,        captflag, captflag, captflag_state,  empty_init,    ROT270, "Jaleco", "Captain Flag (Japan)",                   MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, vscaptfl,  0,        captflag, vscaptfl, captflag_state,  init_vscaptfl, ROT270, "Jaleco", "Vs. Super Captain Flag (Japan)",         MACHINE_IMPERFECT_GRAPHICS )

GAME( 1994, scudhamm,  0,        scudhamm, scudhamm, scudhamm_state,  empty_init,    ROT270, "Jaleco", "Scud Hammer (Japan, ver 1.4)",           MACHINE_IMPERFECT_GRAPHICS ) // version on program ROMs' labels
GAME( 1994, scudhamma, scudhamm, scudhamm, scudhamma,scudhamm_state,  empty_init,    ROT270, "Jaleco", "Scud Hammer (Japan, older version?)",    MACHINE_IMPERFECT_GRAPHICS ) // maybe older cause it has one less DIP switch listed?
