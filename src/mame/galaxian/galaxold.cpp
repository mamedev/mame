// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Stephane Humbert
/***************************************************************************

 Galaxian/Moon Cresta hardware

NOTE:  Eventually to be merged into GALAXIAN.CPP

Main clock: XTAL = 18.432 MHz
Z80 Clock: XTAL/6 = 3.072 MHz
Horizontal video frequency: HSYNC = XTAL/3/192/2 = 16 kHz
Video frequency: VSYNC = HSYNC/132/2 = 60.606060 Hz
VBlank duration: 1/VSYNC * (20/132) = 2500 us


Notes:
-----

- The only code difference between 'galaxian' and 'galmidw' is that the
  'BONUS SHIP' text is printed on a different line.


TODO:
----

- Problems with Galaxian based on the observation of a real machine:

  - Starfield is incorrect.  The speed and flashing frequency is fine, but the
    stars appear in different positions.
  - Background humming is incorrect.  It's faster on a real machine
  - Explosion sound is much softer.  Filter involved?

- $4800-4bff in Streaking/Ghost Muncher


Stephh's notes (based on the games Z80 code and some tests) for other games :

1) 'scramblb' and 'scramb2'

  - Player 2 controls are used for player 2 regardless of the "Cabinet" Dip Switch
    (check code at 0x1043 which changes player and routines that handle players inputs :
    0x1741 UP and DOWN - 0x1796 LEFT and RIGHT - 0x24e6 BUTTON1 - 0x2615 BUTTON2).

2) 'tazzmang'

  - If you press COIN2 during the boot-up sequence, you enter sort of "test mode"
    where you can access to all inputs, but this doesn't give a clue about what
    they do (only the status - 0 or 1 - is displayed).
  - IN1 bit 0 has 2 effects : it starts a one player game (code at 0x5585),
    but it also acts as 2nd button (bomb) for BOTH players regardless of "Cabinet"
    settings (code at 0x5805 for player 1 and player 2 in "Upright" cabinet, or
    0x5563 for player 2 in "Cocktail" cabinet).

***************************************************************************/

#include "emu.h"
#include "galaxold.h"
#include "galaxian_a.h"

#include "cpu/z80/z80.h"
#include "cpu/s2650/s2650.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "speaker.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (XTAL(18'432'000))

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

// H counts from 128->511, HBLANK starts at 128 and ends at 256
#define HTOTAL              (384)
#define HBEND               (0)     // (256)
#define HBSTART             (256)   // (128)

#define VTOTAL              (264)
#define VBEND               (16)
#define VBSTART             (224+16)



// Send sound data to the sound CPU and cause an NMI
uint8_t galaxold_state::drivfrcg_port0_r()
{
	switch (m_maincpu->pc())
	{
		case 0x002e:
		case 0x0297:
			return 0x01;
	}

	return 0;
}

void galaxold_state::galaxold_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x5000, 0x53ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x5400, 0x57ff).r(FUNC(galaxold_state::galaxold_videoram_r));
	map(0x5800, 0x583f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x5840, 0x585f).ram().share("spriteram");
	map(0x5860, 0x587f).ram().share("bulletsram");
	map(0x5880, 0x58ff).ram();
	map(0x6000, 0x6000).portr("IN0");
	map(0x6000, 0x6001).w(FUNC(galaxold_state::galaxold_leds_w));
	map(0x6002, 0x6002).w(FUNC(galaxold_state::galaxold_coin_lockout_w));
	map(0x6003, 0x6003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x6004, 0x6007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x6800, 0x6800).portr("IN1");
	map(0x6800, 0x6802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0x6803, 0x6803).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	map(0x6805, 0x6805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0x6806, 0x6807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x7000, 0x7000).portr("IN2");
	map(0x7001, 0x7001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x7004, 0x7004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x7006, 0x7006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x7007, 0x7007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7800, 0x7fff).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x7800, 0x7800).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0xfffc, 0xffff).ram();
}


void galaxold_state::mooncrst_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x93ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x9400, 0x97ff).r(FUNC(galaxold_state::galaxold_videoram_r));
	map(0x9800, 0x983f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9840, 0x985f).ram().share("spriteram");
	map(0x9860, 0x987f).ram().share("bulletsram");
	map(0x9880, 0x98ff).ram();
	map(0xa000, 0xa000).portr("IN0");
	map(0xa002, 0xa002).w(FUNC(galaxold_state::galaxold_gfxbank_w));
	map(0xa003, 0xa003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0xa004, 0xa007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0xa800, 0xa800).portr("IN1");
	map(0xa800, 0xa802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0xa803, 0xa803).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	map(0xa805, 0xa805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0xa806, 0xa807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0xb000, 0xb000).portr("DSW0").w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0xb004, 0xb004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0xb006, 0xb006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0xb007, 0xb007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xb800, 0xb800).w("cust", FUNC(galaxian_sound_device::pitch_w));
}


void galaxold_state::hustlerb3_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x93ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x9800, 0x983f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9840, 0x985f).ram().share("spriteram");
	map(0x9860, 0x987f).ram().share("bulletsram");
	map(0x9880, 0x98ff).ram();
	map(0xb001, 0xb001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0xb006, 0xb006).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0xb007, 0xb007).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0xa000, 0xa000).portr("IN0");
	map(0xa800, 0xa800).portr("IN1");
	map(0xb000, 0xb000).portr("DSW0");
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r));

	map(0xa004, 0xa007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0xa800, 0xa802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0xa803, 0xa803).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	map(0xa805, 0xa805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0xa806, 0xa807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0xb800, 0xb800).w("cust", FUNC(galaxian_sound_device::pitch_w));

}



void galaxold_state::scramblb_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x4800, 0x4bff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x5000, 0x503f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x5040, 0x505f).ram().share("spriteram");
	map(0x5060, 0x507f).ram().share("bulletsram");
	map(0x5080, 0x50ff).ram();
	map(0x6000, 0x6000).portr("IN0");
	map(0x6000, 0x6001).nopw();  // sound triggers
	map(0x6003, 0x6003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x6004, 0x6007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x6800, 0x6800).portr("IN1");
	map(0x6800, 0x6802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0x6803, 0x6803).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	map(0x6805, 0x6805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0x6806, 0x6807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x7000, 0x7000).portr("IN2");
	map(0x7001, 0x7001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x7002, 0x7002).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x7003, 0x7003).w(FUNC(galaxold_state::scrambold_background_enable_w));
	map(0x7004, 0x7004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x7006, 0x7006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x7007, 0x7007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7800, 0x7800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x7800, 0x7800).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x8102, 0x8102).r(FUNC(galaxold_state::scramblb_protection_1_r));
	map(0x8202, 0x8202).r(FUNC(galaxold_state::scramblb_protection_2_r));
}

uint8_t galaxold_state::scramb2_protection_r(){ return 0x25; }
uint8_t galaxold_state::scramb2_port0_r(offs_t offset){ return (ioport("IN0")->read() >> offset) & 0x1; }
uint8_t galaxold_state::scramb2_port1_r(offs_t offset){ return (ioport("IN1")->read() >> offset) & 0x1; }
uint8_t galaxold_state::scramb2_port2_r(offs_t offset){ return (ioport("IN2")->read() >> offset) & 0x1; }

void galaxold_state::scramb_common_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x4800, 0x4bff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x4c00, 0x4fff).w(FUNC(galaxold_state::galaxold_videoram_w)); // mirror
	map(0x5000, 0x503f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x5040, 0x505f).ram().share("spriteram");
	map(0x5060, 0x507f).ram().share("bulletsram");
	map(0x5080, 0x50ff).ram();
	map(0x5800, 0x5fff).r(FUNC(galaxold_state::scramb2_protection_r)); // must return 0x25
	map(0x6000, 0x6007).r(FUNC(galaxold_state::scramb2_port0_r)); // reads from 8 addresses, 1 bit per address
	map(0x6800, 0x6807).r(FUNC(galaxold_state::scramb2_port1_r)); // reads from 8 addresses, 1 bit per address
	map(0x6801, 0x6801).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x6804, 0x6804).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x6806, 0x6806).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x6807, 0x6807).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7000, 0x7007).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x7006, 0x7006).nopw();
	map(0x7007, 0x7007).nopw();
	map(0x7800, 0x7807).r(FUNC(galaxold_state::scramb2_port2_r)); // reads from 8 addresses, 1 bit per address
	map(0x7800, 0x7800).w("cust", FUNC(galaxian_sound_device::pitch_w));
}

void galaxold_state::scramb2_map(address_map &map)
{
	scramb_common_map(map);
	map(0x6802, 0x6802).w(FUNC(galaxold_state::galaxold_coin_counter_w));
}

void galaxold_state::scramb3_map(address_map &map)
{
	scramb_common_map(map);
	map(0x6003, 0x6003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
}

uint8_t galaxold_state::scrambler_protection_2_r()
{
	// if this doesn't return a value the code is happy with then it jumps out of ROM space after reading the port
	return 0xff;
}

// there are still unmapped reads / writes, it's not really clear what gets hooked up to where on these bootlegs, if they go anywhere at all
void galaxold_state::scrambler_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();

	map(0x4800, 0x4bff).ram(); // mirror, leftovers?

	map(0x5000, 0x53ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");

	map(0x5800, 0x587f).ram();
	map(0x5880, 0x58bf).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x58c0, 0x58df).ram().share("spriteram");
	map(0x58e0, 0x58ff).ram().share("bulletsram");
	map(0x6000, 0x6000).portr("IN0");
	map(0x6000, 0x6001).nopw();  // sound triggers
	map(0x6003, 0x6003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x6004, 0x6007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x6800, 0x6800).portr("IN1");
	map(0x6800, 0x6802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0x6803, 0x6803).w("cust", FUNC(galaxian_sound_device::noise_enable_w)); // should this disable the stars too?
	map(0x6805, 0x6805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0x6806, 0x6807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x7000, 0x7000).portr("IN2").w(FUNC(galaxold_state::galaxold_nmi_enable_w));
//  map(0x7001, 0x7001)
	map(0x7002, 0x7002).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x7003, 0x7003).w(FUNC(galaxold_state::scrambold_background_enable_w));
	map(0x7004, 0x7004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x7006, 0x7006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x7007, 0x7007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7800, 0x7800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x7800, 0x7800).w("cust", FUNC(galaxian_sound_device::pitch_w));
//  map(0x8102, 0x8102).r(FUNC(galaxold_state::scramblb_protection_1_r));
	map(0x8202, 0x8202).r(FUNC(galaxold_state::scrambler_protection_2_r));
}

void galaxold_state::scrambleo_map(address_map &map)
{
	scrambler_map(map);

	map(0x7000, 0x7000).unmapw();
	map(0x7001, 0x7001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
}



void galaxold_state::_4in1_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");    // banked game code
	map(0x4000, 0x47ff).ram();
	map(0x5000, 0x53ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x5400, 0x57ff).r(FUNC(galaxold_state::galaxold_videoram_r));
	map(0x5800, 0x583f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x5840, 0x585f).ram().share("spriteram");
	map(0x5860, 0x587f).ram().share("bulletsram");
	map(0x5880, 0x58ff).ram();
	map(0x6000, 0x6000).portr("IN0");
	map(0x6000, 0x6001).w(FUNC(galaxold_state::galaxold_leds_w));
//  map(0x6002, 0x6002).w(FUNC(galaxold_state::galaxold_coin_lockout_w));
	map(0x6003, 0x6003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x6004, 0x6007).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x6800, 0x6800).portr("IN1");
	map(0x6800, 0x6802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
//  map(0x6803, 0x6803).w(FUNC(galaxold_state::galaxian_noise_enable_w)); // not hooked up?
	map(0x6805, 0x6805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0x6806, 0x6807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x7000, 0x7000).portr("DSW0");
	map(0x7001, 0x7001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x7004, 0x7004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x7006, 0x7006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x7007, 0x7007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7800, 0x78ff).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x7800, 0x78ff).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x8000, 0x8000).w(FUNC(galaxold_state::_4in1_bank_w));
	map(0xc000, 0xdfff).rom();     // fixed menu code
}


void galaxold_state::dkongjrm_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7000, 0x7fff).rom();
	map(0x9000, 0x93ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x9800, 0x983f).w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9840, 0x987f).writeonly().share("spriteram");
	map(0x98c0, 0x98ff).writeonly().share("spriteram2");
	map(0xa000, 0xa0ff).portr("IN0");
	map(0xa003, 0xa003).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	//map(0xa004, 0xa007).w(FUNC(galaxold_state::galaxian_lfo_freq_w));
	map(0xa800, 0xa8ff).portr("IN1");
	map(0xa800, 0xa802).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0xa803, 0xa803).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	//map(0xa805, 0xa805).w(FUNC(galaxold_state::galaxian));
	map(0xa806, 0xa807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0xb000, 0xb0ff).portr("DSW");
	map(0xb000, 0xb000).w(FUNC(galaxold_state::galaxold_gfxbank_w));
	map(0xb001, 0xb001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	//map(0xb004, 0xb004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0xb006, 0xb006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0xb007, 0xb007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w("cust", FUNC(galaxian_sound_device::pitch_w));
}


void galaxold_state::dkongjrmc_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7000, 0x70ff).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x7100, 0x71ff).ram().share("spriteram");
	map(0x7400, 0x77ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x7800, 0x7800).portr("DSW");
	map(0x7801, 0x7801).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x7802, 0x7802).w(FUNC(galaxold_state::galaxold_leds_w));
	map(0x7804, 0x7804).w(FUNC(galaxold_state::galaxold_gfxbank_w));
	map(0x7806, 0x7806).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x7807, 0x7807).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x7d00, 0x7d02).mirror(0x0080).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0x7d03, 0x7d03).mirror(0x0080).w("cust", FUNC(galaxian_sound_device::noise_enable_w));
	map(0x7d05, 0x7d05).mirror(0x0080).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0x7d06, 0x7d07).mirror(0x0080).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x8000, 0x8000).portr("IN0");
	map(0x8100, 0x8100).portr("IN1");
	map(0x8103, 0x8103).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x8104, 0x8107).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x8200, 0x8200).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x9000, 0x9fff).rom();
}


void galaxold_state::tazzmang_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x7000, 0x7000).portr("DSW0"); // mirror
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x883f).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x8840, 0x885f).ram().share("spriteram");
	map(0x8860, 0x887f).ram().share("bulletsram");
	map(0x8880, 0x8bff).nopw();
	map(0x9000, 0x93ff).ram().w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x9800, 0x9800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xa000, 0xa000).portr("IN0");
	map(0xa7ff, 0xa7ff).portr("IN0"); // mirror
	map(0xa800, 0xa800).portr("IN1").w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0xa803, 0xa803).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0xa805, 0xa805).w("cust", FUNC(galaxian_sound_device::fire_enable_w));
	map(0xa806, 0xa807).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0xb000, 0xb000).portr("DSW0");
	map(0xb001, 0xb001).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0xb004, 0xb004).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0xb006, 0xb006).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0xb007, 0xb007).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w("cust", FUNC(galaxian_sound_device::pitch_w));
}


void galaxold_state::hunchbkg_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1480, 0x14bf).mirror(0x6000).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x14c0, 0x14ff).mirror(0x6000).writeonly().share("spriteram");
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1500, 0x1501).mirror(0x6000).w(FUNC(galaxold_state::galaxold_leds_w));            // not connected...
	map(0x1502, 0x1502).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_lockout_w));    // not connected...
	map(0x1503, 0x1503).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x1504, 0x1507).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x1580, 0x1580).mirror(0x6000).portr("IN1");
	map(0x1580, 0x1587).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::sound_w));
	map(0x1600, 0x1600).mirror(0x6000).portr("DSW0");
	map(0x1601, 0x1601).mirror(0x6000).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x1604, 0x1604).mirror(0x6000).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x1606, 0x1606).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x1607, 0x1607).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x1680, 0x1680).mirror(0x6000).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

// hunchbkg style
void galaxold_state::spcwarp_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1480, 0x14bf).mirror(0x6000).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x14c0, 0x14ff).mirror(0x6000).writeonly().share("spriteram");
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1500, 0x1501).mirror(0x6000).w(FUNC(galaxold_state::galaxold_leds_w));
	map(0x1502, 0x1502).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_lockout_w));
	map(0x1503, 0x1503).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x1504, 0x1507).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x1580, 0x1580).mirror(0x6000).portr("IN1");
	map(0x1580, 0x1587).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::sound_w));
	// everything else in the $16xx range is moved to $17xx
	map(0x1680, 0x1680).mirror(0x6000).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x1700, 0x1700).mirror(0x6000).portr("DSW0");
	map(0x1701, 0x1701).mirror(0x6000).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	map(0x1704, 0x1704).mirror(0x6000).w(FUNC(galaxold_state::galaxold_stars_enable_w));
	map(0x1706, 0x1706).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x1707, 0x1707).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	// the rest
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void galaxold_state::hunchbkg_data(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).nopr(); // not used
}


void galaxold_state::superbikg_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1480, 0x14bf).mirror(0x6000).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x14c0, 0x14df).mirror(0x6000).writeonly().share("spriteram");
	map(0x14e0, 0x14ff).mirror(0x6000).writeonly().share("bulletsram");
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1500, 0x1501).mirror(0x6000).w(FUNC(galaxold_state::galaxold_leds_w));
	map(0x1502, 0x1502).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_lockout_w));
	map(0x1503, 0x1503).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x1504, 0x1507).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x1580, 0x1580).mirror(0x6000).portr("IN1");
	map(0x1580, 0x1587).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::sound_w));
	map(0x1680, 0x1680).mirror(0x6000).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x1700, 0x1700).mirror(0x6000).portr("DSW0").nopw();
	map(0x1701, 0x1701).mirror(0x6000).w(FUNC(galaxold_state::galaxold_nmi_enable_w));
	//map(0x1704, 0x1704).mirror(0x6000).w(FUNC(galaxold_state::galaxold_stars_enable_w)); // TODO: writes here, verify on PCB if stars are actually shown
	map(0x1706, 0x1706).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x1707, 0x1707).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void galaxold_state::superbikg_io(address_map &map)
{
	map(0x00, 0x00).lr8(NAME([this] () -> uint8_t { return m_superbikg_latch; }));
}

void galaxold_state::superbikg_data(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).lw8(NAME([this] (uint8_t data) { m_superbikg_latch = (data << 4); }));
}

void galaxold_state::drivfrcg_program(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1480, 0x14bf).mirror(0x6000).w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x14c0, 0x14ff).mirror(0x6000).writeonly().share("spriteram");
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1503, 0x1503).mirror(0x6000).w(FUNC(galaxold_state::galaxold_coin_counter_w));
	map(0x1580, 0x1580).mirror(0x6000).portr("IN1");
	map(0x1580, 0x1582).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::background_enable_w));
	map(0x1583, 0x1583).mirror(0x6000).nopw();
	map(0x1585, 0x1585).mirror(0x6000).nopw();
	map(0x1586, 0x1587).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::lfo_freq_w));
	map(0x1600, 0x1600).mirror(0x6000).portr("DSW0").w("cust", FUNC(galaxian_sound_device::pitch_w));
	map(0x1700, 0x1700).mirror(0x6000).portr("DSW1").nopw();
	map(0x1701, 0x1701).mirror(0x6000).nopw();
	map(0x1704, 0x1707).mirror(0x6000).w("cust", FUNC(galaxian_sound_device::vol_w));
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void galaxold_state::drivfrcg_io(address_map &map)
{
	map(0x00, 0x00).r(FUNC(galaxold_state::drivfrcg_port0_r));
}


void galaxold_state::racknrol_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1400, 0x143f).mirror(0x6000).ram().w(FUNC(galaxold_state::galaxold_attributesram_w)).share("attributesram");
	map(0x1440, 0x14bf).mirror(0x6000).ram().share("spriteram");
	map(0x14c0, 0x14ff).mirror(0x6000).ram();
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1580, 0x1580).mirror(0x6000).portr("IN1");
	map(0x1600, 0x1600).mirror(0x6000).portr("DSW0");
	map(0x1600, 0x1601).mirror(0x6000).nopw();
	map(0x1606, 0x1606).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_x_w));
	map(0x1607, 0x1607).mirror(0x6000).w(FUNC(galaxold_state::galaxold_flip_screen_y_w));
	map(0x1680, 0x1680).mirror(0x6000).nopr();
//  map(0x1700, 0x1700).mirror(0x6000).r(FUNC(galaxold_state::trvchlng_question_r));
//  map(0x1701, 0x1703).mirror(0x6000).w(FUNC(galaxold_state::trvchlng_question_w));
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(galaxold_state::galaxold_videoram_w)).share("videoram");
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}

void galaxold_state::racknrol_io(address_map &map)
{
	map(0x1d, 0x1d).w("snsnd", FUNC(sn76489a_device::write));
//  map(0x1e, 0x1e).nopw();
//  map(0x1f, 0x1f).nopw();
	map(0x20, 0x3f).w(FUNC(galaxold_state::racknrol_tiles_bank_w)).share("racknrol_tbank");
}

uint8_t galaxold_state::hexpoola_data_port_r()
{
	switch (m_maincpu->pc())
	{
		case 0x0022:
			return 0;

		case 0x0031:
			return 1;
	}

	return 0;
}

void galaxold_state::hexpoola_io(address_map &map)
{
	map(0x00, 0x00).nopr();
	map(0x20, 0x3f).w(FUNC(galaxold_state::racknrol_tiles_bank_w)).share("racknrol_tbank");
}

void galaxold_state::hexpoola_data(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(galaxold_state::hexpoola_data_port_r)).w("snsnd", FUNC(sn76496_device::write));
}

uint8_t galaxold_state::bullsdrtg_data_port_r()
{
	switch (m_maincpu->pc())
	{
		case 0x0083:
		case 0x008c:
		case 0x0092:
		case 0x6b54:
			return 0;

		case 0x009b:
		case 0x6b58:
			return 1;
		default:
			logerror("Reading data port at PC=%04X\n", m_maincpu->pc());
			break;
	}

	return 0;
}

void galaxold_state::bullsdrtg_data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(galaxold_state::bullsdrtg_data_port_r)).w("snsnd", FUNC(sn76496_device::write));
}

// Lives Dips are spread across two input ports
template <int Mask>
int galaxold_state::vpool_lives_r()
{
	switch (Mask)
	{
		case 0x40:  // vpool : IN1 (0xa800) bit 6
			return ((ioport("LIVES")->read() & Mask) >> 6);
		case 0x01:  // vpool : DSW (0xb000) bit 0
			return ((ioport("LIVES")->read() & Mask) >> 0);

		default:
			logerror("vpool_lives_r : invalid %02X bit_mask\n",Mask);
			return 0;
	}
}

// verified from Z80 code
static INPUT_PORTS_START( vpool )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )           // uses same coinage as COIN1 and COIN2

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, vpool_lives_r<0x40>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, vpool_lives_r<0x01>)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIVES")
	PORT_DIPNAME( 0x41, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x41, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)" )          // also gives 99 credits after coin insertion regardless of coinage
INPUT_PORTS_END


static INPUT_PORTS_START( hustlerb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// 6-pos dipswitch on mainboard K4
	PORT_DIPNAME( 0x40, 0x00, "Half Coinage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "Infinite (Cheat)" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



static INPUT_PORTS_START( froggerv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// 6-pos dipswitch on mainboard K4
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x00, "3" )
	PORT_DIPSETTING(      0x01, "5" )
	PORT_DIPSETTING(      0x02, "7" )
	PORT_DIPSETTING(      0x03, "256 (Cheat)" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x08, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/6" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END




// verified from Z80 code
static INPUT_PORTS_START( scramblb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "255 (Cheat)")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// verified from Z80 code
static INPUT_PORTS_START( scramb2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )           // uses same coinage as COIN1
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 2/1" )
	PORT_DIPSETTING(    0x02, "A 1/2  B 1/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( scrambler )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1") // cocktail mode inputs conflict with player 1 inputs, either wiring would be attached to the same bit as the flipscreen, or this set never worked in cocktail mode
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // also ends up being P2 left in cocktail mode
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) // also ends up being P2 right in cocktail mode
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 1/6" )
	PORT_DIPSETTING(    0x40, "A 2/1  B 1/3" )
	PORT_DIPSETTING(    0x80, "A 1/2  B 1/6" )
	PORT_DIPSETTING(    0xc0, "A 2/2  B 1/3" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	// probably unused
	PORT_DIPNAME( 0x08, 0x08, "IN4:3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scrambleo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Button 1 / Start 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Button 2 / Start 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 1/6" )
	PORT_DIPSETTING(    0x40, "A 2/1  B 1/3" )
	PORT_DIPSETTING(    0x80, "A 1/2  B 1/6" )
	PORT_DIPSETTING(    0xc0, "A 2/2  B 1/3" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	// probably unused
	PORT_DIPNAME( 0x08, 0x08, "IN4:3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

template <int Mask>
int galaxold_state::_4in1_fake_port_r()
{
	static const char *const portnames[] = { "FAKE1", "FAKE2", "FAKE3", "FAKE4" };

	return (ioport(portnames[m__4in1_bank])->read() & Mask) ? 1 : 0;
}

static INPUT_PORTS_START( 4in1 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x40>)   // See fake ports
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x80>)   // See fake ports

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x01>)   // See fake ports
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x02>)   // See fake ports
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )         // 2 when continue (Scramble PT2)
	PORT_DIPSETTING(    0x04, "5" )         // 2 when continue (Scramble PT2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x08>)   // See fake ports
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x10>)   // See fake ports
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxold_state, _4in1_fake_port_r<0x20>)   // fake ports
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FAKE1")      // The Ghost Muncher PT3 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7)
	PORT_DIPNAME( 0x03, 0x00, "Bonus Life (GM PT3)" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x03, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (GM PT3)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE2")      // Scramble PT2 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7)
//  PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( On ) )
//  PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Lives
	PORT_DIPNAME( 0x08, 0x00, "Allow Continue (S PT2)" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  // Scramble PT2 - Check code at 0x00c2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  // Scramble PT2 - Check code at 0x00cc
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (S PT2)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE3")      // Galaxian PT5 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7)
	PORT_DIPNAME( 0x03, 0x00, "Bonus Life (G PT5)" )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (G PT5)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE4")      // Galactic Convoy - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7)
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life (GC)" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x01, "80000" )
//  PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )   // Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (GC)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    // 1 credit for 1st coin !
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )
INPUT_PORTS_END


// Coinage Dips are spread across two input ports
template <int Mask>
CUSTOM_INPUT_MEMBER(galaxold_state::dkongjrm_coinage_r)
{
	switch (Mask)
	{
		case 0xc0:  // dkongjrm : IN1 (0xa8??) bits 6 and 7
			return ((ioport("COINAGE")->read() & Mask) >> 6);
		case 0x01:  // dkongjrm : DSW (0xb0??) bit 0
			return ((ioport("COINAGE")->read() & Mask) >> 0);

		default:
			logerror("dkongjrm_coinage_r : invalid %02X bit_mask\n",Mask);
			return 0;
	}
}

// verified from Z80 code
static INPUT_PORTS_START( dkongjrm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(galaxold_state, dkongjrm_coinage_r<0xc0>)

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(galaxold_state, dkongjrm_coinage_r<0x01>)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0xc1, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc1, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x41, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x81, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( dkongjrmc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// verified from Z80 code
static INPUT_PORTS_START( tazzmang )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 and P2 Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 4C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x02, "A 3C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x06, "A 2C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hunchbkg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )    // labeled "NOT USED" in galaxian schematics
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Down") PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Down") PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Start/Red") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Start/Red") PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )    // labeled "TABLE" in galaxian schematics
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x06, "80000" )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( drivfrcg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


// 1 bank of 6 DIP-switches
static INPUT_PORTS_START( superbikg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Green") PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Start/Red")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Start/Green")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Red") PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( racknrol )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( trvchlng )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract Mode" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "True or False Bonus" )
	PORT_DIPSETTING(    0x00, "200k" )
	PORT_DIPSETTING(    0x20, "300k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bullsdrtg )
	PORT_INCLUDE( racknrol )

	PORT_MODIFY("IN1")
	PORT_CONFNAME(0x40, 0x00, DEF_STR( Cabinet ) ) // Sense line on wiring harness
	PORT_CONFSETTING(   0x00, DEF_STR( Upright ) )
	PORT_CONFSETTING(   0x40, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, "Award Free Game" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END




static const gfx_layout galaxold_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
static const gfx_layout galaxold_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

#if 0
static const gfx_layout pacmanbl_spritelayout =
{
	16,16,
	64,
	2,
	{ 0, 64*16*16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};
#endif

static const gfx_layout _4in1_charlayout =
{
	8,8,
	1024,
	2,
	{ 0, 1024*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
static const gfx_layout _4in1_spritelayout =
{
	16,16,
	256,
	2,
	{ 0, 256*16*16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};


static GFXDECODE_START( gfx_galaxian )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( gfx_gmgalax )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_spritelayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_4in1 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, _4in1_charlayout,      0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x4000, _4in1_spritelayout,    0, 8 )
GFXDECODE_END


void galaxold_state::galaxold_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, PIXEL_CLOCK/2); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::galaxold_map);

	MCFG_MACHINE_RESET_OVERRIDE(galaxold_state,galaxold)

	TTL7474(config, m_7474_9m_1, 0);
	m_7474_9m_1->output_cb().set(FUNC(galaxold_state::galaxold_7474_9m_1_callback));

	TTL7474(config, m_7474_9m_2, 0);
	m_7474_9m_2->comp_output_cb().set(FUNC(galaxold_state::galaxold_7474_9m_2_q_callback));

	TIMER(config, "int_timer").configure_generic(FUNC(galaxold_state::galaxold_interrupt_timer));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxian);
	PALETTE(config, m_palette, FUNC(galaxold_state::galaxold_palette), 32+2+64); // 32 for the characters, 2 for the bullets, 64 for the stars

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(galaxold_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,galaxold)

	// sound hardware
	SPEAKER(config, "speaker").front_center();
}

void galaxold_state::galaxian_audio(machine_config &config)
{
	GALAXIAN_SOUND(config, "cust", 0);
}

void galaxold_state::mooncrst_audio(machine_config &config)
{
	MOONCRST_SOUND(config, "cust", 0);
}


void galaxold_state::galaxian(machine_config &config)
{
	galaxold_base(config);

	// basic machine hardware

	// sound hardware
	galaxian_audio(config);
}


void galaxold_state::mooncrst(machine_config &config)
{
	galaxold_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::mooncrst_map);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(galaxold_state,mooncrst)

	// sound hardware
	mooncrst_audio(config);
}

// 'Videotron'
// this is a 'cartridge' based system, taking plug-in game boards.
// a large sub-PCB actually contains an additional Z80 and AY8910 (with a socket for another AY8910)
// but neither of the games we have (froggerv and hustlerb3) make use of either. There are a number
// of unpopulated positions on the game board which presumably can be populated with code for the
// 2nd Z80.
void galaxold_state::videotron(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::hustlerb3_map);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(galaxold_state,mooncrst)
}


void galaxold_state::scramblb(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::scramblb_map);

	// video hardware
	m_palette->set_entries(32+2+64+1); // 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background
	m_palette->set_init(FUNC(galaxold_state::scrambold_palette));

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,scrambold)
}


void galaxold_state::scramb2(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::scramb2_map);

	// video hardware
	m_palette->set_entries(32+2+64+1); // 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background
	m_palette->set_init(FUNC(galaxold_state::scrambold_palette));

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,scrambold)
}

void galaxold_state::scramb3(machine_config &config)
{
	scramb2(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::scramb3_map);
}

void galaxold_state::scrambler(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::scrambler_map);

	// video hardware
	m_palette->set_entries(32+2+64+1); // 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background
	m_palette->set_init(FUNC(galaxold_state::scrambold_palette));

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,scrambold)
}

void galaxold_state::scrambleo(machine_config &config)
{
	scrambler(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::scrambleo_map);
}

void galaxold_state::_4in1(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::_4in1_map);

	// video hardware
	m_gfxdecode->set_info(gfx_4in1);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,pisces)
}


void galaxold_state::dkongjrm(machine_config &config)
{
	mooncrst(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::dkongjrm_map);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(galaxold_state,dkongjrm)
}


void galaxold_state::dkongjrmc(machine_config &config)
{
	mooncrst(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::dkongjrmc_map);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,dkongjrmc)
}




void galaxold_state::drivfrcg(machine_config &config)
{
	// basic machine hardware
	s2650_device &maincpu(S2650(config, m_maincpu, MASTER_CLOCK/6));
	maincpu.set_addrmap(AS_PROGRAM, &galaxold_state::drivfrcg_program);
	maincpu.set_addrmap(AS_IO, &galaxold_state::drivfrcg_io);
	maincpu.sense_handler().set("screen", FUNC(screen_device::vblank)); // ???
	maincpu.intack_handler().set(FUNC(galaxold_state::hunchbkg_intack));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(16000.0/132/2);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(galaxold_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	PALETTE(config, m_palette, FUNC(galaxold_state::s2650_palette), 64);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gmgalax);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,drivfrcg)

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	galaxian_audio(config);
}


void galaxold_state::hunchbkg(machine_config &config)
{
	galaxold_base(config);

	// basic machine hardware
	s2650_device &s2650(S2650(config.replace(), m_maincpu, PIXEL_CLOCK / 4));
	s2650.set_addrmap(AS_PROGRAM, &galaxold_state::hunchbkg_map);
	s2650.set_addrmap(AS_DATA, &galaxold_state::hunchbkg_data);
	s2650.intack_handler().set(FUNC(galaxold_state::hunchbkg_intack));

	// the NMI line seems to be inverted on the CPU plugin board
	m_7474_9m_1->comp_output_cb().set_inputline("maincpu", S2650_SENSE_LINE);

	MCFG_MACHINE_RESET_OVERRIDE(galaxold_state,hunchbkg)

	galaxian_audio(config);
}


void galaxold_state::spcwarp(machine_config &config)
{
	hunchbkg(config);
	// hunchbkg, but with a different memory map
	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::spcwarp_map);
}

void galaxold_state::superbikg(machine_config &config)
{
	galaxold_base(config);

	s2650_device &s2650(S2650(config.replace(), m_maincpu, 1'500'000)); // 1.53292 MHz measured with logic analyzer
	s2650.set_addrmap(AS_PROGRAM, &galaxold_state::superbikg_map);
	s2650.set_addrmap(AS_IO, &galaxold_state::superbikg_io);
	s2650.set_addrmap(AS_DATA, &galaxold_state::superbikg_data);
	s2650.intack_handler().set(FUNC(galaxold_state::hunchbkg_intack));

	galaxian_audio(config);
}

void galaxold_state::tazzmang(machine_config &config)
{
	galaxian(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxold_state::tazzmang_map);
}


void galaxold_state::racknrol(machine_config &config)
{
	// basic machine hardware
	s2650_device &maincpu(S2650(config, m_maincpu, PIXEL_CLOCK/2));
	maincpu.set_addrmap(AS_PROGRAM, &galaxold_state::racknrol_map);
	maincpu.set_addrmap(AS_IO, &galaxold_state::racknrol_io);
	maincpu.sense_handler().set(m_screen, FUNC(screen_device::vblank)).invert(); // ???
	maincpu.intack_handler().set(FUNC(galaxold_state::hunchbkg_intack));

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxian);
	PALETTE(config, m_palette, FUNC(galaxold_state::s2650_palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(galaxold_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,racknrol)

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76489A(config, "snsnd", PIXEL_CLOCK/2).add_route(ALL_OUTPUTS, "speaker", 1.0); // SN76489AN
}


void galaxold_state::hexpoola(machine_config &config)
{
	// basic machine hardware
	s2650_device &maincpu(S2650(config, m_maincpu, PIXEL_CLOCK/2));
	maincpu.set_addrmap(AS_PROGRAM, &galaxold_state::racknrol_map);
	maincpu.set_addrmap(AS_IO, &galaxold_state::hexpoola_io);
	maincpu.set_addrmap(AS_DATA, &galaxold_state::hexpoola_data);
	maincpu.sense_handler().set(m_screen, FUNC(screen_device::vblank)).invert(); // ???
	maincpu.intack_handler().set(FUNC(galaxold_state::hunchbkg_intack));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxian);
	PALETTE(config, m_palette, FUNC(galaxold_state::s2650_palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(galaxold_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	MCFG_VIDEO_START_OVERRIDE(galaxold_state,racknrol)

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, "snsnd", PIXEL_CLOCK/2).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


void galaxold_state::bullsdrtg(machine_config &config)
{
	hexpoola(config);

	m_maincpu->set_addrmap(AS_DATA, &galaxold_state::bullsdrtg_data_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vpool )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vidpool1.bin", 0x0000, 0x0800, CRC(333f4732) SHA1(b57460c039c69137645bd4280ad877aa789277d6) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	ROM_LOAD( "vidpool2.bin", 0x0800, 0x0800, CRC(eea6c0f1) SHA1(5b18caa78e246f55fd9cd778d6e83f79f0b3f157) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_LOAD( "vidpool3.bin", 0x1000, 0x0800, CRC(309972a6) SHA1(8269d2f677f55dda71d6a7b0796d2d53a4def59d) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "vidpool4.bin", 0x1800, 0x0800, CRC(c4f71c1d) SHA1(e1d01135d5ccc1a53308ce89dc2a8fc0992207d5) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f", 0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) ) // vidpoolh.bin
	ROM_LOAD( "hustler.5h", 0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) ) // vidpoolk.bin

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END


ROM_START( hustlerb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "billiard_ic4.a4", 0x0000, 0x2000, CRC(545a27fd) SHA1(8b064dd6a9a82248301e0f53dc1c4fd91e506025) )
	ROM_LOAD( "billiard_ic3.a3", 0x2000, 0x2000, CRC(bec503b1) SHA1(cdbe650b829cd4424141058467cd64cfffe1b1e1) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "billiard_ic11.d1",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "billiard_ic12.d2",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic10.c3",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )

	ROM_REGION( 0x0020, "user1", 0 ) // decode PROMs
	ROM_LOAD( "ic7.b3", 0x0000, 0x0020, CRC(4ac17114) SHA1(1fa34a556fe445a6bdabfe75b4b679cab6553c8b) )
ROM_END

// https://www.youtube.com/watch?v=r7di0_Yt1l8
ROM_START( froggerv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rana_ic4.ic4",   0x0000, 0x2000, CRC(ed39f6d8) SHA1(8ca60be30dfc5c54fbc129fa0024987d853aad39) )
	ROM_LOAD( "rana_ic3.ic3",   0x2000, 0x2000, CRC(f8313d5d) SHA1(76f8e382d5cfad4eafbcd8d42bc9a9f03a5eb5f8) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rana_ic11.ic11",  0x0000, 0x0800, CRC(a1199087) SHA1(4492b021a6b5ae9a9e2ab97914ce1a5e5e5b64ab) )
	ROM_LOAD( "rana_ic12.ic12",  0x0800, 0x0800, CRC(c1690dfc) SHA1(c6fdb1b9ec4fb7da2566b0c71e3e2f931cdece68) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic10",     0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )  // SN74288 or equivalent BPROM

	ROM_REGION( 0x0020, "user1", 0 ) // decode PROMs
	ROM_LOAD( "ic7",      0x0000, 0x0020, CRC(4ac17114) SHA1(1fa34a556fe445a6bdabfe75b4b679cab6553c8b) )  // SN74288 or equivalent BPROM
ROM_END



ROM_START( scramblb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scramble.1k",  0x0000, 0x0800, CRC(9e025c4a) SHA1(a8cc9391bdd01a5a2fe7f0c4e889b4e2495df891) )
	ROM_LOAD( "scramble.2k",  0x0800, 0x0800, CRC(306f783e) SHA1(92d19f90f1123cd211706294d668ab23c8b0760b) )
	ROM_LOAD( "scramble.3k",  0x1000, 0x0800, CRC(0500b701) SHA1(54c84ccad2aae34f42fdddcfcd92cd9da2cd7119) )
	ROM_LOAD( "scramble.4k",  0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "scramble.5k",  0x2000, 0x0800, CRC(df0b9648) SHA1(4ae9150c9441897d5ab7c5a0b3f10e1e8c8e2f6c) )
	ROM_LOAD( "scramble.1j",  0x2800, 0x0800, CRC(b8c07b3c) SHA1(33eaedef4b7f49eeef072425541c17206d0a00ec) )
	ROM_LOAD( "scramble.2j",  0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "scramble.3j",  0x3800, 0x0800, CRC(c67d57ca) SHA1(ba8b14289aef47d48d9750cf2ef3c368e74a60e8) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.k",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "5h.k",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scramb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1.7f1",  0x0000, 0x0800, CRC(4a43148c) SHA1(ea27fd3acf661101296a58a7a50fb8e4d5292760) )
	ROM_LOAD( "r1.7f2",  0x0800, 0x0800, CRC(215a3b86) SHA1(bfddfea9f74064123629d89556240c7a59f7bea2) )
	ROM_LOAD( "r2.7h1",  0x1000, 0x0800, CRC(28779444) SHA1(0abd3a89c8cdd5af2ac06afd38bcd2dcd6010bee) )
	ROM_LOAD( "r2.7h2",  0x1800, 0x0800, CRC(5b4b300b) SHA1(6d69dbdab66bc8f4a16c3d9d3b4581799e4bbfab) )
	ROM_LOAD( "r3.7k1",  0x2000, 0x0800, CRC(b478aa53) SHA1(68cf134482092534ef0a3ceee3aa842f86660065) )
	ROM_LOAD( "r3.7k2",  0x2800, 0x0800, CRC(c33f072e) SHA1(28d61e35f3d5c971e070d7e0cc20b831fe8d52c5) )
	ROM_LOAD( "r4.7l1",  0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "r4.7l2",  0x3800, 0x0800, CRC(321fd003) SHA1(61f33c2709913da4cb20f311501df707d755917e) )
	// Also exists in the following ROM config
//  ROM_LOAD( "r1.7f",  0x0000, 0x1000, CRC(75208a74) SHA1(e77afe4b906d08d6763f31dd70d7cb772be97102) )
//  ROM_LOAD( "r2.7h",  0x1000, 0x1000, CRC(f2179cf5) SHA1(5c38aa9bd1d5ebdccf16d2e50acc56f0b3f042d0) )
//  ROM_LOAD( "r3.7k",  0x2000, 0x1000, CRC(941c804e) SHA1(f1eedf719a234cf98071e6a46120765e231f0730) )
//  ROM_LOAD( "r4.7l",  0x3000, 0x1000, CRC(f1506edc) SHA1(66689bb3d7570848e4d020a5f44d6de03b4bff99) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "r6.1j",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "r5.1l",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scramb3 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ra.7f",        0x0000, 0x1000, CRC(979be487) SHA1(74817d4d7f616e244ca331d05f1dcea30050a98e) )
	ROM_LOAD( "r2.7h",        0x1000, 0x1000, CRC(f2179cf5) SHA1(5c38aa9bd1d5ebdccf16d2e50acc56f0b3f042d0) )
	ROM_LOAD( "r3.7k",        0x2000, 0x1000, CRC(941c804e) SHA1(f1eedf719a234cf98071e6a46120765e231f0730) )
	ROM_LOAD( "top32",        0x3000, 0x1000, CRC(0c3297a6) SHA1(6d6bb183139b30e78d74e8272d3a9f7234d394c6) ) // on a smaller top PCB

	ROM_REGION( 0x1000, "gfx1", 0 ) // identical to scramble and a lot of other sets
	ROM_LOAD( "6.1h",       0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "5.1k",       0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 ) // identical to scramble
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scrambler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "principal 1.bin",        0x0000, 0x0800, CRC(926958d2) SHA1(eda1ddca0d43f40d684886d5ec54d40d40ce5715) )
	ROM_LOAD( "principal 2.bin",        0x0800, 0x0800, CRC(655c6eca) SHA1(a9f4484d9c5c6716018e234563798b3c54124878) )
	ROM_LOAD( "principal 3.bin",        0x1000, 0x0800, CRC(cd31749a) SHA1(0250530e76cf5019df61bfe3e0a85969e011b8f9) )
	ROM_LOAD( "principal 4.bin",        0x1800, 0x0800, CRC(f055e1e3) SHA1(51eabb4915ceade37def5fe39129b15f0a37bd65) )
	ROM_LOAD( "principal 5.bin",        0x2000, 0x0800, CRC(15f10df7) SHA1(93c9abcd61fbe5c5dd33a9387527565af6117c40) )
	ROM_LOAD( "principal 6.bin",        0x2800, 0x0800, CRC(4bd1c703) SHA1(b91465d6ef2ae8e1b1d24bc9895e7e596b9e422b) )
	ROM_LOAD( "principal 7.bin",        0x3000, 0x0800, CRC(0bb49470) SHA1(05a6fe3010c2136284ca76352dac147797c79778) ) // == s7.2m
	ROM_LOAD( "principal 8.bin",        0x3800, 0x0800, CRC(6db9f380) SHA1(0678ffe16886ba76314ea14f15b4bb5752366dd5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "graph hj.bin",        0x0000, 0x0800, CRC(4c017c9c) SHA1(36ea77be224aac16578b26c6f69601c7f10d1c7b) )
	ROM_LOAD( "graph kl.bin",        0x0800, 0x0800, CRC(28a66399) SHA1(fe4c900e80a3a04c5c12e037ae2ae23339b9a9f8) )

	ROM_REGION( 0x0020, "proms", 0 ) // not dumped, assumed to be the same
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scrambleo ) // MR-1A + MP-28 PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(55313343) SHA1(3a5988e108aaf7af6498fb1cabef12b2f75858d5) )
	ROM_LOAD( "2.bin",        0x0800, 0x0800, CRC(af61bdfe) SHA1(f3414adc243e4687db3ae9c4f003663215a52b6b) )
	ROM_LOAD( "3.bin",        0x1000, 0x0800, CRC(c85c613b) SHA1(622d8566dc199624aad02380f69280c8b4c06bf2) )
	ROM_LOAD( "4.bin",        0x1800, 0x0800, CRC(f055e1e3) SHA1(51eabb4915ceade37def5fe39129b15f0a37bd65) )
	ROM_LOAD( "5.bin",        0x2000, 0x0800, CRC(cc059dc5) SHA1(2a80637f6fb38b8db06293e92d80b4be640e23d2) )
	ROM_LOAD( "6.bin",        0x2800, 0x0800, CRC(dd15f10e) SHA1(d7efd498ab7702127dc48c5ab57d207732924c38) )
	ROM_LOAD( "7.bin",        0x3000, 0x0800, CRC(0bb49470) SHA1(05a6fe3010c2136284ca76352dac147797c79778) )
	ROM_LOAD( "8.bin",        0x3800, 0x0800, CRC(29a15aec) SHA1(bf98bb931b934eeca7c996143b0900ffe8757f18) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx_h.bin",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "gfx_m.bin",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sc_cprom.bin",      0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

/* Seems like a modification of the Okapi bootleg to add a "continue" option.
   Probably was not done by Okapi itself, but by a Spanish bootlegger (the "continue" screen is in Spanish). */
ROM_START( scrabbleo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scr-1-2716.bin",  0x0000, 0x0800, CRC(b363dcc9) SHA1(3803c71da0fe37081fc6d29af6188edfc4332dbb) )
	ROM_LOAD( "scr-2-2716.bin",  0x0800, 0x0800, CRC(4f78a15d) SHA1(be5702354bfc070c29172e3dd902195442531b73) )
	ROM_LOAD( "scr-3-2716.bin",  0x1000, 0x0800, CRC(cd31749a) SHA1(0250530e76cf5019df61bfe3e0a85969e011b8f9) )
	ROM_LOAD( "scr-4-2716.bin",  0x1800, 0x0800, CRC(f055e1e3) SHA1(51eabb4915ceade37def5fe39129b15f0a37bd65) )
	ROM_LOAD( "scr-5-2716.bin",  0x2000, 0x0800, CRC(15f10df7) SHA1(93c9abcd61fbe5c5dd33a9387527565af6117c40) )
	ROM_LOAD( "scr-6-2716.bin",  0x2800, 0x0800, CRC(dd15f10e) SHA1(d7efd498ab7702127dc48c5ab57d207732924c38) )
	ROM_LOAD( "scr-7-2716.bin",  0x3000, 0x0800, CRC(0bb49470) SHA1(05a6fe3010c2136284ca76352dac147797c79778) )
	ROM_LOAD( "scr-8-2716.bin",  0x3800, 0x0800, CRC(6db9f380) SHA1(0678ffe16886ba76314ea14f15b4bb5752366dd5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "scr-9-2716.bin",  0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "scr-10-2716.bin", 0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.bin",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( 4in1 )
	ROM_REGION( 0x20000, "maincpu", 0 )   // 64k for code  64k for banked code, encrypted
	// Menu Code, Fixed at 0xc000 - 0xdfff
	ROM_LOAD( "rom1a",        0xc000, 0x1000, CRC(ce1af4d9) SHA1(260d81cb703ab33fa5f282454214dea06e59a5d6) )
	ROM_LOAD( "rom1b",        0xd000, 0x1000, CRC(18484f9b) SHA1(2439841ba5882c287bd9656fbf79190ff9efe4ee) )
	// Ghost Muncher PT3 - banked at 0x0000 - 0x3fff
	ROM_LOAD( "rom1c",       0x10000, 0x1000, CRC(83248a8b) SHA1(65af22b9a4516ab52c3327cb3b714d90c2c77284) )
	ROM_LOAD( "rom1d",       0x11000, 0x1000, CRC(053f6da0) SHA1(fa69de09a2162dfaa82ea566f0808433f26e4854) )
	ROM_LOAD( "rom1e",       0x12000, 0x1000, CRC(43c546f3) SHA1(c32a2281f8dca1f2b218dc76192d8e09f2eee460) )
	ROM_LOAD( "rom1f",       0x13000, 0x1000, CRC(3a086b46) SHA1(1fd65fd139a650a5c246cead5141b81764faf98c) )
	// Scramble PT2 - banked at 0x0000 - 0x3fff
	ROM_LOAD( "rom1g",       0x14000, 0x1000, CRC(ac0e2050) SHA1(02961a41f54d55f2ae07a2694a14fb6e6e4a766b) )
	ROM_LOAD( "rom1h",       0x15000, 0x1000, CRC(dc11a513) SHA1(2785c08d890f2f8e86b7f793f7989d7605570cc3) )
	ROM_LOAD( "rom1i",       0x16000, 0x1000, CRC(a5fb6be4) SHA1(f575ca70037134084aff152fcee7fdd0a1163c33) )
	ROM_LOAD( "rom1j",       0x17000, 0x1000, CRC(9054cfbe) SHA1(99ad74491cf8682daf45f2786e0bf275160c9826) )
	// Galaxian PT5 - banked at 0x0000 - 0x3fff
	ROM_LOAD( "rom2c",       0x18000, 0x1000, CRC(7cd98e11) SHA1(7ef49866a5c5fd871acf5bfe3d899a9ae0d37405) )
	ROM_LOAD( "rom2d",       0x19000, 0x1000, CRC(9402f32e) SHA1(feb5cb09ea719612a22949f34fb97e172305c7b0) )
	ROM_LOAD( "rom2e",       0x1a000, 0x1000, CRC(468e81df) SHA1(4ac30c170ce63637c77227833cef8839e2b0b8ab) )
	// Galactic Convoy - banked at 0x0000 - 0x3fff
	ROM_LOAD( "rom2g",       0x1c000, 0x1000, CRC(b1ce3976) SHA1(365e643948e982126198714bb1e07340ded7d4a5) )
	ROM_LOAD( "rom2h",       0x1d000, 0x1000, CRC(7eab5670) SHA1(d9648fc314bc6a685536c6acb17b17737813d902) )
	ROM_LOAD( "rom2i",       0x1e000, 0x1000, CRC(44565ac5) SHA1(cc8141cbdb9280a15b40761448e00a3b30a94ec7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	// Ghost Muncher PT3 GFX
	ROM_LOAD( "rom4b",        0x4000, 0x0800, CRC(7e6495af) SHA1(32db70bca5c60eea6b37a943e076bc5a8dc3870b) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "rom3b",        0x6000, 0x0800, CRC(7475f72f) SHA1(834873b6a587760cbbd0ac9435af55f6cb20097a) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	// Scramble PT2 GFX
	ROM_LOAD( "rom4c",        0x4800, 0x0800, CRC(3355d46d) SHA1(e5476d2053298958f141e11a97017ea465621d89) )
	ROM_RELOAD(               0x0800, 0x0800)
	ROM_LOAD( "rom3c",        0x6800, 0x0800, CRC(ac755a25) SHA1(70af05d32554682be6c3f74936e57b4050d283c7) )
	ROM_RELOAD(               0x2800, 0x0800)
	// Galaxians PT5 GFX
	ROM_LOAD( "rom4d",        0x5000, 0x0800, CRC(bbdddb65) SHA1(fc2dcfd969b1ee51a6413117e83f8a0c29278658) )
	ROM_CONTINUE(             0x1000, 0x0800)
	ROM_LOAD( "rom3d",        0x7000, 0x0800, CRC(91a00204) SHA1(eea8a8bd8439260dde9131693e9b53b0238ce7a7) )
	ROM_CONTINUE(             0x3000, 0x0800)
	// Galactic Convoy GFX
	ROM_LOAD( "rom4e",        0x5800, 0x0800, CRC(0cb9e297) SHA1(a9be2951851deed0ffefb980fc7751a399dc131e) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "rom3e",        0x7800, 0x0800, CRC(a1fe77f9) SHA1(dc7972b7aa77fb4f95d7349d4cd7fc4674f9032d) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( dkongjrm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1",           0x0000, 0x1000, CRC(299486e9) SHA1(cc4143ff8cb7a37c151bebab007a932381ae733b) )
	ROM_LOAD( "a2",           0x1000, 0x1000, CRC(a74a193b) SHA1(46f208293c0944b468550738d1238de9b672f403) )
	ROM_LOAD( "b2",           0x2000, 0x1000, CRC(7bc4f236) SHA1(84e7f5fcbea7d047f2a9a9006ae3ed646417c5e0) )
	ROM_LOAD( "c1",           0x3000, 0x1000, CRC(0f594c21) SHA1(eb15bd9cc37794786e2ad24753172e88aa7c4f98) )
	ROM_LOAD( "d1",           0x4000, 0x1000, CRC(cf7d7296) SHA1(9a817eca2ebef3f5208bb29ee7eece2ec0efe158) )
	ROM_LOAD( "e2",           0x5000, 0x1000, CRC(f7528a52) SHA1(e9d3c57934ee97fcc1f17ecdf3bc954574212220) )
	ROM_LOAD( "f1",           0x7000, 0x1000, CRC(9b1d4cc5) SHA1(9a412fec82f39b9389ff99cceba2e49b2a74df17) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "v_3pa.bin",    0x0000, 0x1000, CRC(4974ffef) SHA1(7bb1e207dd3c5214e405bf32c57ec1b048061050) )
	ROM_LOAD( "a2.gfx",       0x1000, 0x1000, CRC(51845eaf) SHA1(43970d69329f3d49ea1ff57d54abe8340ceef275) )
	ROM_LOAD( "v_3na.bin",    0x2000, 0x1000, CRC(a95c4c63) SHA1(75e312b6872958f3bfc7bafd0743efdf7a74e8f0) )
	ROM_LOAD( "b2.gfx",       0x3000, 0x1000, CRC(7b39c3d0) SHA1(4b8cebb4cdaaca9e1b6fd378f6c390ab05984590) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( dkongjrmc ) // "CENTROMATIC - G/G" main board
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732-1.bin",   0x0000, 0x1000, CRC(1bcb41be) SHA1(74b1700babd26cb0c781bc702130a63da0386463) )
	ROM_LOAD( "2732-2.bin",   0x1000, 0x1000, CRC(f027a6b9) SHA1(557ab9737c31b4c57b1d37d62bb5ce51c574c4b9) )
	ROM_LOAD( "2732-3.bin",   0x2000, 0x1000, CRC(e2484ff4) SHA1(fdd77db1ba0dc7876e0503aace7273d7a8dedd53) )
	ROM_LOAD( "2732-4.bin",   0x3000, 0x1000, CRC(37e5dd92) SHA1(69bbc765a54e98b69cf6831342469a9c94659f8f) )
	ROM_LOAD( "2732-5.bin",   0x4000, 0x1000, CRC(b2d89348) SHA1(209f3ca7c97afa3e746f778df0c34664f8791855) )
	ROM_LOAD( "2732-6.bin",   0x5000, 0x1000, CRC(24b0c560) SHA1(0b7e099ad2aace542fedb8b7c8c28204d3ea7a46) )
	ROM_LOAD( "2732-7.bin",   0x9000, 0x1000, CRC(7428eefa) SHA1(82e5c8461fe48e5d6222bb5d0a259e6fd0c5cac7) )

	ROM_REGION( 0x4000, "gfx1", 0 ) // GFX ROM sub board marked "CALFESA" on solder side
	ROM_LOAD( "2732-a.bin",   0x0000, 0x1000, CRC(526ed721) SHA1(5fd317908b9d9aa70768f8f7cfbfdfa6d1e654b6) )
	ROM_LOAD( "2732-b.bin",   0x1000, 0x1000, CRC(d2cb5130) SHA1(b87d2c74cc74782fec7268f0a637613450a2fb4b) )
	ROM_LOAD( "2732-c.bin",   0x2000, 0x1000, CRC(7ceae713) SHA1(1f85d3b34cf10bd6277b408aed051a18cf9b1090) )
	ROM_LOAD( "2732-d.bin",   0x3000, 0x1000, CRC(2aa1acf1) SHA1(fdf58e737253ca5f0cf3667d3a74efeafdb66e21) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )

	ROM_REGION( 0x0200, "mainproms", 0 )
	ROM_LOAD( "tbp28l22n-8240a.bin", 0x0000, 0x0100, CRC(df72ed74) SHA1(fce9a846b7238c2cc8898e6338485f3df0a56755) )
	ROM_LOAD( "tbp28l22n-a8240a.bin", 0x0100, 0x0100, CRC(9575df2b) SHA1(9360730fc230d17f6be5fc7f8d46d79566839cfa) )
ROM_END

ROM_START( tazzmang )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tazzm1.4k",    0x0000, 0x1000, CRC(a14480a1) SHA1(60dac6b57e8331cc4daedaf87faf3e3acc68f378) )
	ROM_LOAD( "tazzm2.5j",    0x1000, 0x1000, CRC(5609f5db) SHA1(3fc50109ea0e012e3e310ae4f5dd0cf460bdca52) )
	ROM_LOAD( "tazzm3.6f",    0x2000, 0x1000, CRC(fe7f7002) SHA1(ac4134c07a798328b18994010bcaf6b3f728466a) )
	ROM_LOAD( "tazzm4.7e",    0x3000, 0x1000, CRC(c9ca1d0a) SHA1(d420ca2e926174e17215212278c86ba9bbb3d9dc) )
	ROM_LOAD( "tazzm5.7l",    0x4000, 0x1000, CRC(f50cd8a6) SHA1(b59ca37171b9acc9854f1beae43cfa5643219a5f) )
	ROM_LOAD( "tazzm6.7l",    0x5000, 0x1000, CRC(5cf2e7d2) SHA1(ad89e2655164e0fc5ecc9af70c5f0dd9b094d432) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "tazm8.1lk",    0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "tazzm7.1jh",   0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6l",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( tazzmang2 )  // Original Sparcade set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tazmania.1",   0x000000, 0x000800, CRC(6ecc84a2) SHA1(6f31e69bd613b93e1fac26163f39676299c65a76) )
	ROM_LOAD( "tazmania.2",   0x000800, 0x000800, CRC(e27b09f6) SHA1(1a419c8f45639e2c2351eeb94bf62fca35d5928e) )
	ROM_LOAD( "tazmania.3",   0x001000, 0x000800, CRC(954868f3) SHA1(3882e17ffd9bcfcff383ed95279606962f89dafd) )
	ROM_LOAD( "tazmania.4",   0x001800, 0x000800, CRC(238520e6) SHA1(eec76b54058a6a6139f13f188d243f20d1a7aa12) )
	ROM_LOAD( "tazmania.5",   0x002000, 0x000800, CRC(0527e513) SHA1(20175c293f1cf45fa21dd400cb2718dd8ee0dcea) )
	ROM_LOAD( "tazmania.6",   0x002800, 0x000800, CRC(af2b92d8) SHA1(5642666eb66d549390cd5b13a7029daede6d3ff8) )
	ROM_LOAD( "tazmania.7",   0x003000, 0x000800, CRC(bbdc41d3) SHA1(17de825efd56541dbdbacdc83f77f3ccaef2d07f) )
	ROM_LOAD( "tazmania.8",   0x003800, 0x000800, CRC(eb35f49c) SHA1(0f2bf1043092e746fdbc5d2e0292aeaa7b7f0218) )
	ROM_LOAD( "tazmania.a",   0x004000, 0x001000, CRC(38f326f8) SHA1(5c5463666b6ed15cbcc874faf79cc06ae1cba59a) )
	ROM_LOAD( "tazmania.b",   0x005000, 0x001000, CRC(2a22a9dc) SHA1(07aecdff852065671e488682cf710fd48273b88c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "tazm8.1lk",    0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) ) // tazmania.g1
	ROM_LOAD( "tazzm7.1jh",   0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) ) // tazmania.g2

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6l",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


ROM_START( hunchbkg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "gal_hb_1",     0x0000, 0x0800, CRC(46590e9b) SHA1(5d26578c91adec20d8d8a17d5dade9ef2febcbe5) )
	ROM_LOAD( "gal_hb_2",     0x0800, 0x0800, CRC(4e6e671c) SHA1(5948fc7f390f0343b367d333395427ce2f9b2931) )
	ROM_LOAD( "gal_hb_3",     0x2000, 0x0800, CRC(d29dc242) SHA1(3f6087fe962ee63c2886ad3f502c1a37d357ba87) )
	ROM_LOAD( "gal_hb_4",     0x2800, 0x0800, CRC(d409d292) SHA1(d631c9106106b31b605b6fdf1d4f40e237a725ac) )
	ROM_LOAD( "gal_hb_5",     0x4000, 0x0800, CRC(29d3a8c4) SHA1(2e1ef20d980e5033503d8095e9576dcb8f532f41) )
	ROM_LOAD( "gal_hb_6",     0x4800, 0x0800, CRC(b016fd15) SHA1(cdfbd531e23438f05a7c3aad99a94ce55912aac3) )
	ROM_LOAD( "gal_hb_7",     0x6000, 0x0800, CRC(d2731d27) SHA1(8c4a3d2303d85c3b11803c577a9ad21e6e69011e) )
	ROM_LOAD( "gal_hb_8",     0x6800, 0x0800, CRC(e4b1a666) SHA1(9f73d17cff208374d587536e783be024fc9ab700) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gal_hb_kl",    0x0000, 0x0800, CRC(3977650e) SHA1(1de05d6ceed3f2ed0925caa8235b63a93f03f61e) )
	ROM_LOAD( "gal_hb_hj",    0x0800, 0x0800, CRC(db489c3d) SHA1(df08607ad07222c1c1c4b3589b50b785bdeefbf2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "gal_hb_cp",    0x0000, 0x0020, CRC(cbff6762) SHA1(4515a6e12a0a5c485a55291feee17a571120a549) )
ROM_END

ROM_START( hunchbgb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(46590e9b) SHA1(5d26578c91adec20d8d8a17d5dade9ef2febcbe5) )
	ROM_LOAD( "2.bin",        0x0800, 0x0800, CRC(4e6e671c) SHA1(5948fc7f390f0343b367d333395427ce2f9b2931) )
	ROM_LOAD( "3.bin",        0x2000, 0x0800, CRC(d29dc242) SHA1(3f6087fe962ee63c2886ad3f502c1a37d357ba87) )
	ROM_LOAD( "4.bin",        0x2800, 0x0800, CRC(d409d292) SHA1(d631c9106106b31b605b6fdf1d4f40e237a725ac) )
	ROM_LOAD( "5.bin",        0x4000, 0x0800, CRC(29d3a8c4) SHA1(2e1ef20d980e5033503d8095e9576dcb8f532f41) )
	ROM_LOAD( "6.bin",        0x4800, 0x0800, CRC(b016fd15) SHA1(cdfbd531e23438f05a7c3aad99a94ce55912aac3) )
	ROM_LOAD( "7.bin",        0x6000, 0x0800, CRC(d2731d27) SHA1(8c4a3d2303d85c3b11803c577a9ad21e6e69011e) )
	ROM_LOAD( "8.bin",        0x6800, 0x0800, CRC(e4b1a666) SHA1(9f73d17cff208374d587536e783be024fc9ab700) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "lk.bin",       0x0000, 0x0800, CRC(83ecf8f4) SHA1(1eb8ca1ed9d82001fc4a62fef5e13d63a5ab7884) )
	ROM_LOAD( "jh.bin",       0x0800, 0x0800, CRC(106889ec) SHA1(bbeca0d36e3f117bcd1e6361c808368d2f90f00a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288n.bin",  0x0000, 0x0020, CRC(2430f47c) SHA1(f7725f4768cb57717feb18891766642f6d7cbcde) )
ROM_END

/*
For all we know, this could be anything, but the text in ROM confirms the
copyright (swarpt7f.bin):

"COPYRIGHT 1983"
"CENTURY ELECTRONICS LTD"

...and the GFX ROMs contain graphics similar to Cosmos, so it could be
Space Warp after all.

Due to how incomplete this dump is (missing ROM, one corrupted), there is
very little to be worked on, but so far, using a variation of hunchbkg's
memory map and inputs work, atleast until it crashes on the title screen.
*/

ROM_START( spcwarp )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "swarpt7f.bin", 0x0000, 0x1000, CRC(04d744e3) SHA1(db8218510052a05670cb0b722b73d3f10464788c) )
	ROM_LOAD( "swarpt7h.bin", 0x2000, 0x1000, CRC(34a36536) SHA1(bc438515618683b2a7c29637871ee00ed95ad7f8) )
	// missing ROM at $4000
	ROM_LOAD( "swarpt7m.bin", 0x6000, 0x1000, BAD_DUMP CRC(a2dff6c8) SHA1(d1c72848450dc5ff386dc94a26e4bf704ccc7121) ) // ROMCMP reports "BADADDR            xxxxxx-xxxxx".  Observed data sequence repeated every 32 bytes

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "swarpb1h.bin", 0x0000, 0x0800, CRC(6ee3b5f7) SHA1(8150f2ecd59d3a165c0541b550664c56d049edd5) )
	ROM_LOAD( "swarpb1k.bin", 0x0800, 0x0800, CRC(da4cee6b) SHA1(28b91381658f598fa62049489beee443232825c6) )

	// using hunchbkg PROMs for now
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "gal_hb_cp",    0x0000, 0x0020, BAD_DUMP CRC(cbff6762) SHA1(4515a6e12a0a5c485a55291feee17a571120a549) )
ROM_END

// GX-01 main PCB + MV-2 ROM board + HB CPU board
// In-game it still shows the Century copyright but the PCB has no sign of being an original
ROM_START( superbikg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "moto1-2516.bin", 0x0000, 0x0800, CRC(2903f8c8) SHA1(288401a941853751caa8d1d69e6908dbfbcfb5ac) )
	ROM_LOAD( "moto2-2516.bin", 0x0800, 0x0800, CRC(057c1473) SHA1(e2021e006642aacb1e51b2ca6e2c0d22d7033d54) )
	ROM_LOAD( "moto3-2516.bin", 0x2000, 0x0800, CRC(6c8f82fa) SHA1(b698e4af9462e915b4c1b6126ea475f25903d22e) )
	ROM_LOAD( "moto4-2516.bin", 0x2800, 0x0800, CRC(ba38fe2c) SHA1(29b5d9b66d1d80107852f647e7f89479747dcf3a) )
	ROM_LOAD( "moto5-2532.bin", 0x4000, 0x1000, CRC(ac1180a3) SHA1(dcd6c0f95ac017a29f4f928b70ef16975aaa178e) )
	ROM_LOAD( "moto6-2532.bin", 0x6000, 0x1000, CRC(f5b7627a) SHA1(02dfa62b0bf5962ad56d922084888f2216eca497) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // these are the same as sbdk in nintendo/dkong.cpp
	ROM_LOAD( "m0hj-2716.bin",  0x0000, 0x0800, CRC(b1d76b59) SHA1(aed57ec67d80abdff1a4bfc3a713fa01c0dd15a2) )
	ROM_LOAD( "m0kl-2716.bin",  0x0800, 0x0800, CRC(ea5f9f88) SHA1(5742d3554d967ed1e90f7c6f73dafbd302f0f244) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l",     0x0000, 0x0020, CRC(c5f12bc3) SHA1(b746ba06b596d4227fdc730a23bdf495f84e6a72) ) // same as amidarc, bongoa and froggervd in galaxian/galaxian.cpp

	ROM_REGION( 0x0120, "other_proms", 0 )
	ROM_LOAD( "mmi6331-db.bin", 0x0000, 0x0020, CRC(39376e52) SHA1(f988c95e3837fe5e5a0d6815cd0bf644ff1eb081) )
	ROM_LOAD( "mmi6309-mb.bin", 0x0020, 0x0100, CRC(b6b9ff46) SHA1(8f16938d9c9c66308823b702f5d75843bd7de046) ) // near CPU, address decoding related?
ROM_END

ROM_START( drivfrcg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dfgp1.bin",    0x2800, 0x0400, CRC(52d5e77d) SHA1(4e68ac1274bbc8cb5b6a7dfb511232bd83482453) )
	ROM_CONTINUE(             0x2c00, 0x0400 )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x0400, 0x0400 )
	ROM_LOAD( "dfgp2.bin",    0x0800, 0x0400, CRC(9cf4dbce) SHA1(028c168ad0987f21d76c6ac4f756f4fa86c2f8e3) )
	ROM_CONTINUE(             0x0c00, 0x0400 )
	ROM_CONTINUE(             0x2000, 0x0400 )
	ROM_CONTINUE(             0x2400, 0x0400 )
	ROM_LOAD( "dfgp3.bin",    0x6800, 0x0400, CRC(79763f62) SHA1(2bb8921fcd2a8b9543e398e248fd47d7e03dc24d) )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "dfgp4.bin",    0x4800, 0x0400, CRC(dd95338b) SHA1(9054986f7b8fee36f458362836ae969e7d1e2456) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_CONTINUE(             0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "dfgj2.bin",    0x0000, 0x1000, CRC(8e19f1e7) SHA1(addd5add2117ef29ce38c0c80584e5d481b9d820) )
	ROM_LOAD( "dfgj1.bin",    0x1000, 0x1000, CRC(86b60ca8) SHA1(be266e2d69e12a196c2195d48b495c0fb9ef8a43) )
	ROM_LOAD( "dfgl2.bin",    0x2000, 0x1000, CRC(ea5e9959) SHA1(6b638d22adf19224cf741458c8ad34d7f7e17e58) )
	ROM_LOAD( "dfgl1.bin",    0x3000, 0x1000, CRC(b7ed195c) SHA1(81b2b444153dacb962a33a5d86a280ed5088637a) )

	// piggy-backed colour PROMs
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( drivfrcsg ) // This PCB has a big epoxy block by Tanaka Enterprises marked E-0010, providing ROM addressing
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "6n-2-2764a.bin", 0x2800, 0x0400, CRC(85242241) SHA1(bad2609c7f6d83a15809b602a0c141793909ceb0) )
	ROM_CONTINUE(               0x2c00, 0x0400 )
	ROM_CONTINUE(               0x0000, 0x0400 )
	ROM_CONTINUE(               0x0400, 0x0400 )
	ROM_CONTINUE(               0x0800, 0x0400 )
	ROM_CONTINUE(               0x0c00, 0x0400 )
	ROM_CONTINUE(               0x2000, 0x0400 )
	ROM_CONTINUE(               0x2400, 0x0400 )
	ROM_LOAD( "6m-1-2764a.bin", 0x6800, 0x0400, CRC(42d99594) SHA1(1b03132279a3a6edd2281a2f55ef2d3133003a16) )
	ROM_CONTINUE(               0x6c00, 0x0400 )
	ROM_CONTINUE(               0x4000, 0x0400 )
	ROM_CONTINUE(               0x4400, 0x0400 )
	ROM_CONTINUE(               0x4800, 0x0400 )
	ROM_CONTINUE(               0x4c00, 0x0400 )
	ROM_CONTINUE(               0x6000, 0x0400 )
	ROM_CONTINUE(               0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "1j-2764a.bin", 0x0000, 0x2000, CRC(156e20bd) SHA1(8ec4020d179674856f43e543ce5e54730752568a) )
	ROM_LOAD( "1l-2764a.bin", 0x2000, 0x2000, CRC(88d0f70b) SHA1(c91aa798f7450c0cf1a8db4225d4a4efa25555d8) )

	// piggy-backed colour PROMs
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123-1.bin",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "82s123-2.bin",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( drivfrcsga ) // This PCB has a big epoxy block by Tanaka Enterprises marked E-0237, possibly providing ROM addressing
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "xx.bin", 0x2800, 0x0400, NO_DUMP ) // missing
	ROM_CONTINUE(       0x2c00, 0x0400 )
	ROM_CONTINUE(       0x0000, 0x0400 )
	ROM_CONTINUE(       0x0400, 0x0400 )
	ROM_CONTINUE(       0x0800, 0x0400 )
	ROM_CONTINUE(       0x0c00, 0x0400 )
	ROM_CONTINUE(       0x2000, 0x0400 )
	ROM_CONTINUE(       0x2400, 0x0400 )
	ROM_LOAD( "1p.bin", 0x6800, 0x0400, CRC(58b12215) SHA1(73c8bd16994704d8b1d007c5924ba5e83f7233ea) ) // unique
	ROM_CONTINUE(       0x6c00, 0x0400 )
	ROM_CONTINUE(       0x4000, 0x0400 )
	ROM_CONTINUE(       0x4400, 0x0400 )
	ROM_CONTINUE(       0x4800, 0x0400 )
	ROM_CONTINUE(       0x4c00, 0x0400 )
	ROM_CONTINUE(       0x6000, 0x0400 )
	ROM_CONTINUE(       0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "kj_5.22.bin", 0x0000, 0x2000, CRC(7b6d837a) SHA1(925ca351635e77cacfb5a2d6e31487c5e4aaf0ec) ) // unique
	ROM_LOAD( "kl_5.22.bin", 0x2000, 0x2000, CRC(86cd5438) SHA1(c921d8cd031fd0fa78488ae95a1570dd1be919e9) ) // unique

	// piggy-backed colour PROMs
	ROM_REGION( 0x0040, "proms", 0 )  // missing
	ROM_LOAD( "82s123-1.bin", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "82s123-2.bin", 0x0020, 0x0020, NO_DUMP )
ROM_END

ROM_START( drivfrcb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dfp.bin",      0x2800, 0x0400, CRC(b5b2981d) SHA1(c9ff19791895bf05b569457b1e53dfa0aaeb8e95) )
	ROM_CONTINUE(             0x2c00, 0x0400 )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x0400, 0x0400 )
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_CONTINUE(             0x0c00, 0x0400 )
	ROM_CONTINUE(             0x2000, 0x0400 )
	ROM_CONTINUE(             0x2400, 0x0400 )
	ROM_CONTINUE(             0x6800, 0x0400 )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_CONTINUE(             0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "df1.bin",      0x1000, 0x1000, CRC(8adc3de0) SHA1(046fb92913171c621bb62edb0174f04298bfd283) )
	ROM_CONTINUE(             0x0000, 0x1000 )
	ROM_LOAD( "df2.bin",      0x3000, 0x1000, CRC(6d95ec35) SHA1(c745ee2bc7b1fb53e8bc1ac3a4238bbe00f30cfe) )
	ROM_CONTINUE(             0x2000, 0x1000 )

	// piggy-backed colour PROMs
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( drivfrct )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "03.bin",       0x2800, 0x0400, CRC(9ab604cb) SHA1(772a5c0d93377f5bd7fc1f5e4050d44321a1bb8d) )
	ROM_CONTINUE(             0x2c00, 0x0400 )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x0400, 0x0400 )
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_CONTINUE(             0x0c00, 0x0400 )
	ROM_CONTINUE(             0x2000, 0x0400 )
	ROM_CONTINUE(             0x2400, 0x0400 )
	ROM_CONTINUE(             0x6800, 0x0400 )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_CONTINUE(             0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "01.bin",       0x1000, 0x1000, CRC(300a6750) SHA1(0760eb852706ef72c61e889309ee94edc49a13dc) )
	ROM_CONTINUE(             0x0000, 0x1000 )
	ROM_LOAD( "02.bin",       0x3000, 0x1000, CRC(f04e14c4) SHA1(f628da48ad19c86000c56345fd96d415992bf9a9) )
	ROM_CONTINUE(             0x2000, 0x1000 )

	// piggy-backed colour PROMs
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "tbp18s030.02", 0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "tbp18s030.01", 0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )

	// PROMs inside epoxy block with CPU
	ROM_REGION( 0x0300, "user1", 0 )
	ROM_LOAD( "tbp24s10.bin", 0x0000, 0x0100, CRC(8c0d886d) SHA1(03bb942861a639f30797fcb22f048f7908404955) )
	ROM_LOAD( "tbp28s42.bin", 0x0100, 0x0200, CRC(9b8f310a) SHA1(8e17cc1adf441aec56d98d0809e1359d5175e8ed) )
ROM_END

ROM_START( racknrol ) // has an AY-3-8910 on main pcb, but is unused? SN76489AN on daughterboard is used.
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "horz_p.bin",   0x0000, 0x1000, CRC(32ca5b43) SHA1(f3e7662f947dcdd80f6eae4f002d2fe64a825aff) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "horz_g.bin",   0x0000, 0x4000, CRC(97069ad5) SHA1(50199c7bc5083be23a34849cff17906795bf4067) )
	ROM_LOAD( "horz_r.bin",   0x4000, 0x4000, CRC(ff64e84b) SHA1(ceabd522bae26743804987632f35f3c4b5aff179) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpool )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "vert_p.bin",   0x0000, 0x1000, CRC(bdb078fc) SHA1(85a65c3038dc05a98eae71edf9efdd6659a2966a) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "vert_g.bin",   0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "vert_r.bin",   0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpoola )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom.4l",       0x0000, 0x1000, CRC(2ca8018d) SHA1(f0784d18bc7e77515bf2140d8993ae8178919853) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rom.1m",       0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "rom.1l",       0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.11r",   0x0000, 0x0020, CRC(deb2fcf4) SHA1(cdec737a9d9feae912f7cc04ca0adb48f859b5c2) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "82s147.5pr",   0x0000, 0x0200, CRC(cf496b1e) SHA1(5b5ca52b3cc46e18990dae53a98984aeaf264241) )

	ROM_REGION( 0x00eb, "plds", 0 )
	ROM_LOAD( "82s153.6pr.bin", 0x0000, 0x00eb, CRC(bc07939a) SHA1(615b085575ad215662eab2777a2d8b9167c4b9c3) )
ROM_END

ROM_START( trvchlng )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "senko11.bin",  0x0000, 0x1000, CRC(3657331d) SHA1(d9a9a4e4e2e696e70dfb888725c959ec8ce24e3d) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "questions",    0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "senko10.bin",  0x0000, 0x4000, CRC(234b59d0) SHA1(5eafdfc6d6a73575835b68361fe29a2dc61e8a83) )
	ROM_LOAD( "senko12.bin",  0x4000, 0x4000, CRC(0bf6b92d) SHA1(6ca993c0642949a52fafea3bc57a08c6881e8120) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "senko1.bin",   0x0000, 0x0020, CRC(1434c7ff) SHA1(0ee5f5351dd84fbf8d3d8eaafbdbe86dd29960f8) )
ROM_END



/*
Bulls Eye Darts conversion by Senko Industries Ltd (1984)

The base board for the conversion dates from 1981.

Base Board
----------
There are 2 x Toshiba 2114, 2 x Mitsubushi 2101 and 5 x Intel 2115 making up
the video RAM. There are 6 ROM sockets for the video ROM but the daughter card
only connects to two of them. I believe that this is a version of
Scramble/Galaxians video hardware.

There are 4 x Toshiba 2114 that make up the CPU RAM. There are 8 ROM sockets
for the CPU ROM. Tha daughter board does not connect to any of them but instead
connects into the CPU socket (I'm guessing originally a Z80).

Sound is provided at least by an AY-3-8910.

Daughter Board
--------------
The daughter board houses a 2650A CPU and another 40-pin cermet coated uncased
device with the number scratched off. There is an 82S153 and an 82S147 hooked
into the 2650. Amongst the TTL near the video ROMS is a single 2114.


Now the bad news...

There are three EPROMS and one PROM on the board. Alas, one of the graphics
EPROMS does not verify consistently so I have provided three reads. A quick
compare suggest one data line is random :-(
*/

ROM_START( bullsdrtg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cpu.bin",   0x0000, 0x1000, CRC(db34f130) SHA1(691f8a69a7157df49460f5927728ba52660eeede) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "vid_j1.bin",   0x0000, 0x4000, BAD_DUMP CRC(c2ad5c84) SHA1(8e3048607693afc40a775000f45790910e4d9312) )
	ROM_LOAD( "vid_p.bin",    0x4000, 0x4000, CRC(535be505) SHA1(c20c7ac4e74e29e8954c443cca9dcc0df453a512) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.bin",   0x0000, 0x0020, CRC(16b19bfa) SHA1(a0e9217f9bc5b06212d5f22dcc3dc4b2838788ba) )
ROM_END



// Z80 games
//    YEAR  NAME       PARENT    MACHINE    INPUT      STATE          INIT             ROT     COMPANY,                         FULLNAME,                                                   FLAGS
GAME( 1981, vpool,     hustler,  mooncrst,  vpool,     galaxold_state, empty_init,     ROT90,  "bootleg",                       "Video Pool (bootleg on Moon Cresta hardware)",             MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scramblb,  scramble, scramblb,  scramblb,  galaxold_state, empty_init,     ROT90,  "bootleg",                       "Scramble (bootleg on Galaxian hardware)",                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scramb2,   scramble, scramb2,   scramb2,   galaxold_state, empty_init,     ROT90,  "bootleg",                       "Scramble (bootleg, set 1)",                                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scramb3,   scramble, scramb3,   scramb2,   galaxold_state, empty_init,     ROT90,  "bootleg",                       "Scramble (bootleg, set 2)",                                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrambler, scramble, scrambler, scrambler, galaxold_state, empty_init,     ROT90,  "bootleg (Reben S.A.)",          "Scramble (Reben S.A. Spanish bootleg)",                    MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrambleo, scramble, scrambleo, scrambleo, galaxold_state, empty_init,     ROT90,  "bootleg (Okapi)",               "Scramble (Okapi bootleg)",                                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrabbleo, scramble, scrambler, scrambler, galaxold_state, empty_init,     ROT90,  "bootleg (Okapi?)",              "Scrabble (Spanish bootleg of Scramble)",                   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, 4in1,      0,        _4in1,     4in1,      galaxold_state, init_4in1,      ROT90,  "Armenia / Food and Fun",        "4 Fun in 1",                                               MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjrm,  dkongjr,  dkongjrm,  dkongjrm,  galaxold_state, empty_init,     ROT90,  "bootleg",                       "Donkey Kong Jr. (bootleg on Moon Cresta hardware, set 1)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjrmc, dkongjr,  dkongjrmc, dkongjrmc, galaxold_state, empty_init,     ROT90,  "bootleg (Centromatic)",         "Donkey Kong Jr. (bootleg on Moon Cresta hardware, set 2)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // sprites leave artifacts
GAME( 1982, tazzmang,  tazmania, tazzmang,  tazzmang,  galaxold_state, empty_init,     ROT90,  "bootleg",                       "Tazz-Mania (bootleg on Galaxian hardware)",                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, tazzmang2, tazmania, tazzmang,  tazzmang,  galaxold_state, empty_init,     ROT90,  "bootleg",                       "Tazz-Mania (bootleg on Galaxian hardware with Starfield)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

// Videotron cartridge system
GAME( 1981, hustlerb3, hustler,  videotron, hustlerb3, galaxold_state, empty_init,     ROT90,  "bootleg (Videotron)",            "Video Pool (bootleg of Video Hustler)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggerv,  frogger,  videotron, froggerv,  galaxold_state, empty_init,     ROT90,  "bootleg (Videotron / Gamepack)", "Frogger (Videotron bootleg)",           MACHINE_SUPPORTS_SAVE )

// S2650 games
//    YEAR  NAME       PARENT    MACHINE    INPUT      STATE           INIT            ROT     COMPANY,                                               FULLNAME,                                                      FLAGS
GAME( 1983, hunchbkg,  hunchbak, hunchbkg,  hunchbkg,  galaxold_state, empty_init,     ROT90,  "Century Electronics",                                 "Hunchback (Galaxian hardware)",                               MACHINE_SUPPORTS_SAVE )
GAME( 1983, hunchbgb,  hunchbak, hunchbkg,  hunchbkg,  galaxold_state, empty_init,     ROT90,  "bootleg (FAR S.A.)",                                  "Hunchback (FAR S.A. bootleg on Galaxian hardware)",           MACHINE_SUPPORTS_SAVE )
GAME( 1983, spcwarp,   0,        spcwarp,   hunchbkg,  galaxold_state, empty_init,     ROT90,  "Century Electronics",                                 "Space Warp? (Cosmos conversion on Galaxian hardware)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS ) // bad dump
GAME( 1983, superbikg, superbik, superbikg, superbikg, galaxold_state, init_superbikg, ROT90,  "bootleg",                                             "Superbike (bootleg on Galaxian hardware)",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // colors look strange but match real hw video
GAME( 1984, drivfrcg,  drivfrcp, drivfrcg,  drivfrcg,  galaxold_state, empty_init,     ROT90,  "Shinkai Inc. (Magic Electronics USA license)",        "Driving Force (Galaxian conversion)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1984, drivfrct,  drivfrcp, drivfrcg,  drivfrcg,  galaxold_state, empty_init,     ROT90,  "bootleg (EMT Germany)",                               "Top Racer (bootleg of Driving Force)",                        MACHINE_SUPPORTS_SAVE ) // Video Klein PCB
GAME( 1985, drivfrcb,  drivfrcp, drivfrcg,  drivfrcg,  galaxold_state, empty_init,     ROT90,  "bootleg (Elsys Software)",                            "Driving Force (Galaxian conversion bootleg)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, drivfrcsg, drivfrcp, drivfrcg,  drivfrcg,  galaxold_state, empty_init,     ROT90,  "Shinkai Inc. (Seatongrove UK, Ltd. license)",         "Driving Force (Galaxian conversion, Seatongrove UK, E-0010)", MACHINE_SUPPORTS_SAVE ) // assume they got permission through Magic, not directly from Shinkai
GAME( 1985, drivfrcsga,drivfrcp, drivfrcg,  drivfrcg,  galaxold_state, empty_init,     ROT90,  "Shinkai Inc. (Seatongrove UK, Ltd. license)",         "Driving Force (Galaxian conversion, Seatongrove UK, E-0237)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // incomplete dump
GAME( 1986, racknrol,  0,        racknrol,  racknrol,  galaxold_state, empty_init,     ROT0,   "Senko Industries (Status license from Shinkai Inc.)", "Rack + Roll",                                                 MACHINE_SUPPORTS_SAVE )
GAME( 1986, hexpool,   racknrol, racknrol,  racknrol,  galaxold_state, empty_init,     ROT90,  "Senko Industries (Shinkai Inc. license)",             "Hex Pool (Shinkai)",                                          MACHINE_SUPPORTS_SAVE ) // still has Senko logo in gfx rom
GAME( 1985, hexpoola,  racknrol, hexpoola,  racknrol,  galaxold_state, empty_init,     ROT90,  "Senko Industries",                                    "Hex Pool (Senko)",                                            MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvchlng,  0,        racknrol,  trvchlng,  galaxold_state, empty_init,     ROT90,  "Joyland (Senko license)",                             "Trivia Challenge",                                            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1985, bullsdrtg, bullsdrt, bullsdrtg, bullsdrtg, galaxold_state, init_bullsdrtg, ROT90,  "Senko Industries",                                    "Bulls Eye Darts (Galaxian conversion)",                       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS )
