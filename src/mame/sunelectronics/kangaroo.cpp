// license:BSD-3-Clause
// copyright-holders:Ville Laitinen, Aaron Giles
/***************************************************************************

    Sun Electronics hardware:

       TVG-1-CPU-B
         2 x Z80
         AY-3-9810
         8-way DipSwitch
         Service Switch
         Program ROMS (both Main & Sound)
         MB8841 (at least for Kangaroo)
       TVG-1-VIDEO-B
         10MHz OSC
         Graphic ROMS

    driver by Ville Laitinen

    Games supported:
        * Kangaroo
        * Funky Fish

    Known bugs:
        * none at this time

****************************************************************************

    0000-0fff tvg75
    1000-1fff tvg76
    2000-2fff tvg77
    3000-3fff tvg78
    4000-4fff tvg79
    5000-5fff tvg80
    8000-bfff VIDEO RAM (four banks)
    c000-cfff tvg83/84 (banked)
    d000-dfff tvg85/86 (banked)
    e000-e3ff RAM


    memory mapped ports:
    read:
    e400      DSW 0
    ec00      IN 0
    ed00      IN 1
    ee00      IN 2
    efxx      (4 bits wide) security chip in. It seems to work like a clock.

    write:
    e800-e801 low/high byte start address of data in picture ROM for DMA
    e802-e803 low/high byte start address in bitmap RAM (where picture is to be
              written) during DMA
    e804-e805 picture size for DMA, and DMA start
    e806      vertical scroll of playfield
    e807      horizontal scroll of playfield
    e808      bank select latch
    e809      A & B bitmap control latch (A=playfield B=motion)
              bit 5 FLIP A
              bit 4 FLIP B
              bit 3 EN A
              bit 2 EN B
              bit 1 PRI A
              bit 0 PRI B
    e80a      color shading latch
    ec00      command to sound CPU
    ed00      coin counters
    efxx      (4 bits wide) security chip out

    ---------------------------------------------------------------------------
    CPU #1 (sound)

    0000 0fff tvg81
    4000 43ff RAM
    6000      command from main CPU

    I/O ports:
    7000      AY-3-8910 write
    8000      AY-3-8910 control
    ---------------------------------------------------------------------------

    interrupts:
    (CPU#0) standard IM 1 interrupt mode (rst #38 every vblank)
    (CPU#1) same here


                       Kangaroo Memory Map
    HEX        R/W   D7 D6 D5 D4 D3 D2 D2 D0  function

------------  Game Microprocessor Memory Space (Z80 - IC15) --------------

    0000-5FFF   R    D  D  D  D  D  D  D  D   Z80 24K Program ROM

    E000-E3FF  R/W   D  D  D  D  D  D  D  D   1K Working RAM

    E400        R    D  D  D  D  D  D  D  D   Option Switch (DSW0 8 way dipswitch)

    E800        W    D  D  D  D  D  D  D  D   Low  Byte\ Start Address of Data in Pictures ROM for DMA
    E801        W    D  D  D  D  D  D  D  D   High Byte/
    E802        W    D  D  D  D  D  D  D  D   Low  Byte\ Start Address in Bit Map RAM (where picture is to be written) During DMA
    E803        W    D  D  D  D  D  D  D  D   High Byte/
    E804        W    D  D  D  D  D  D  D  D   Low  Byte\ Picture Size for DMA and DMA Start
    E805        W    D  D  D  D  D  D  D  D   High Byte/
    E806        W    D  D  D  D  D  D  D  D   Vertical Start Address in Bit Map
    E807        W    D  D  D  D  D  D  D  D   Horizontal Start Address in Bit Map
    E808        W                      D  D   Bank Select Latch
    E809        W          D  D  D  D  D  D   A & B Bit Map Control Latch (A=playfield, B=motion)
    E80A        W          D  D  D  D  D  D   Color Shading Latch

    EC00        W    D  D  D  D  D  D  D  D   Sound DATA to Sound Microprocessor
    EC00        R                         D   Utility Coin Switch
    EC00        R                      D      1 Player Start
    EC00        R                   D         2 Player Start
    EC00        R                D            Left Coin Input
    EC00        R             D               Right Coin Input

    ED00        W                         D   Coin Counter 1
    ED00        W                      D      Coin Counter 2 (European games)
    ED00        R                         D   Player 1 Right
    ED00        R                      D      Player 1 Left
    ED00        R                   D         Player 1 Up
    ED00        R                D            Player 1 Down
    ED00        R             D               Player 1 Punch

    EE00        R                         D   Player 2 Right
    EE00        R                      D      Player 2 Left
    EE00        R                   D         Player 2 Up
    EE00        R                D            Player 2 Down
    EE00        R             D               Player 2 Punch

    EFxx-EFxx   W                D  D  D  D   Output to Custom MB8841 Microcomputer
    EFxx-EFxx   R                D  D  D  D   Input from Custom MB8841 Microcomputer

------------  Sound Microprocessor Memory Space (Z80 - IC34) --------------

    0000-0FFF   R    D  D  D  D  D  D  D  D   Z80 4K Program ROM

    4000-43FF  R/W   D  D  D  D  D  D  D  D   1K Working RAM

    6000        R    D  D  D  D  D  D  D  D   Read DATA from Game Microprocessor

    7000        W    D  D  D  D  D  D  D  D   Write to Sound Chip (GI-AY-3-9810 - IC50)
    8000        W    D  D  D  D  D  D  D  D   Read from Sound Chip

****************************************************************************

    In test mode, to test sound press 1 and 2 player start simultaneously.
    Punch + 1 player start moves to the crosshatch pattern.

    To enter test mode in Funky Fish, keep the service coin pressed while
    resetting

    TODO:
    - There is a custom MB8841 microcontroller on the original Kangaroo board which
      is not emulated. This MIGHT cause some problems, but we don't know of any.

***************************************************************************/

#include "emu.h"

#include "cpu/mb88xx/mb88xx.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class kangaroo_state : public driver_device
{
public:
	kangaroo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_control(*this, "video_control"),
		m_videoram(*this, "videoram", 256 * 64 * 4, ENDIANNESS_LITTLE), // video RAM is accessed 32 bits at a time (two planes, 4bpp each, 4 pixels)
		m_blitbank(*this, "blitbank"),
		m_blitrom(*this, "blitter"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{ }

	void nomcu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_video_control;
	memory_share_creator<uint32_t> m_videoram;
	required_memory_bank m_blitbank;
	required_region_ptr<uint8_t> m_blitrom;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	// misc
	void coin_counter_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void video_control_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void videoram_write(uint16_t offset, uint8_t data, uint8_t mask);
	void blitter_execute();

	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


class kangaroo_mcu_state : public kangaroo_state
{
public:
	kangaroo_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		kangaroo_state(mconfig, type, tag)
	{ }

	void mcu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// MCU simulation (for now)
	uint8_t m_mcu_clock = 0U;

	uint8_t mcu_sim_r();
	void mcu_sim_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Video RAM accesses
 *
 *************************************/

void kangaroo_state::videoram_write(uint16_t offset, uint8_t data, uint8_t mask)
{
	// data contains 4 2-bit values packed as DCBADCBA; expand these into 4 8-bit values
	uint32_t expdata = 0;
	if (data & 0x01) expdata |= 0x00000055;
	if (data & 0x10) expdata |= 0x000000aa;
	if (data & 0x02) expdata |= 0x00005500;
	if (data & 0x20) expdata |= 0x0000aa00;
	if (data & 0x04) expdata |= 0x00550000;
	if (data & 0x40) expdata |= 0x00aa0000;
	if (data & 0x08) expdata |= 0x55000000;
	if (data & 0x80) expdata |= 0xaa000000;

	// determine which layers are enabled
	uint32_t layermask = 0;
	if (mask & 0x08) layermask |= 0x30303030;
	if (mask & 0x04) layermask |= 0xc0c0c0c0;
	if (mask & 0x02) layermask |= 0x03030303;
	if (mask & 0x01) layermask |= 0x0c0c0c0c;

	// update layers
	m_videoram[offset] = (m_videoram[offset] & ~layermask) | (expdata & layermask);
}


void kangaroo_state::videoram_w(offs_t offset, uint8_t data)
{
	videoram_write(offset, data, m_video_control[8]);
}



/*************************************
 *
 *  Video control writes
 *
 *************************************/

void kangaroo_state::video_control_w(offs_t offset, uint8_t data)
{
	m_video_control[offset] = data;

	switch (offset)
	{
		case 5: // blitter start
			blitter_execute();
			break;

		case 8: // bank select
			m_blitbank->set_entry((data & 0x05) ? 0 : 1);
			break;
	}
}



/*************************************
 *
 *  DMA blitter
 *
 *************************************/

void kangaroo_state::blitter_execute()
{
	uint32_t gfxhalfsize = m_blitrom.bytes() / 2;
	uint16_t src = m_video_control[0] + 256 * m_video_control[1];
	uint16_t dst = m_video_control[2] + 256 * m_video_control[3];
	uint8_t height = m_video_control[5];
	uint8_t width = m_video_control[4];
	uint8_t mask = m_video_control[8];

	// during DMA operations, the top 2 bits are ORed together, as well as the bottom 2 bits
	// adjust the mask to account for this
	if (mask & 0x0c) mask |= 0x0c;
	if (mask & 0x03) mask |= 0x03;

	// loop over height, then width
	for (int y = 0; y <= height; y++, dst += 256)
		for (int x = 0; x <= width; x++)
		{
			uint16_t effdst = (dst + x) & 0x3fff;
			uint16_t effsrc = src++ & (gfxhalfsize - 1);
			videoram_write(effdst, m_blitrom[0 * gfxhalfsize + effsrc], mask & 0x05);
			videoram_write(effdst, m_blitrom[1 * gfxhalfsize + effsrc], mask & 0x0a);
		}
}



/*************************************
 *
 *  Video updater
 *
 *************************************/

uint32_t kangaroo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t scrolly = m_video_control[6];
	uint8_t scrollx = m_video_control[7];
	uint8_t maska = (m_video_control[10] & 0x28) >> 3;
	uint8_t maskb = (m_video_control[10] & 0x07) >> 0;
	uint8_t xora = (m_video_control[9] & 0x20) ? 0xff : 0x00;
	uint8_t xorb = (m_video_control[9] & 0x10) ? 0xff : 0x00;
	uint8_t enaa = (m_video_control[9] & 0x08);
	uint8_t enab = (m_video_control[9] & 0x04);
	uint8_t pria = (~m_video_control[9] & 0x02);
	uint8_t prib = (~m_video_control[9] & 0x01);

	// iterate over pixels
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *const dest = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint8_t effxa = scrollx + ((x / 2) ^ xora);
			uint8_t effya = scrolly + (y ^ xora);
			uint8_t effxb = (x / 2) ^ xorb;
			uint8_t effyb = y ^ xorb;
			uint8_t pixa = (m_videoram[effya + 256 * (effxa / 4)] >> (8 * (effxa % 4) + 0)) & 0x0f;
			uint8_t pixb = (m_videoram[effyb + 256 * (effxb / 4)] >> (8 * (effxb % 4) + 4)) & 0x0f;

			// for each layer, contribute bits if (a) enabled, and (b) either has priority or the opposite plane is 0
			uint8_t finalpens = 0;
			if (enaa && (pria || pixb == 0))
				finalpens |= pixa;
			if (enab && (prib || pixa == 0))
				finalpens |= pixb;

			// store the first of two pixels, which is always full brightness
			dest[x + 0] = m_palette->pen_color(finalpens & 7);

			// KOS1 alternates at 5MHz, offset from the pixel clock by 1/2 clock
			// when 0, it enables the color mask for pixels with Z = 0
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
			{
				if (!(pixa & 0x08)) pixa &= maska;
				finalpens |= pixa;
			}
			if (enab && (prib || pixa == 0))
			{
				if (!(pixb & 0x08)) pixb &= maskb;
				finalpens |= pixb;
			}

			// store the second of two pixels, which is affected by KOS1 and the A/B masks
			dest[x + 1] = m_palette->pen_color(finalpens & 7);
		}
	}

	return 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void kangaroo_state::machine_start()
{
	m_blitbank->configure_entries(0, 2, memregion("blitter")->base(), 0x2000);
}


void kangaroo_mcu_state::machine_start()
{
	kangaroo_state::machine_start();

	save_item(NAME(m_mcu_clock));
}


void kangaroo_state::machine_reset()
{
	/* I think there is a bug in the startup checks of the game. At the very
	   beginning, during the RAM check, it goes one byte too far, and ends up
	   trying to write, and re-read, location dfff. To the best of my knowledge,
	   that is a ROM address, so the test fails and the code keeps jumping back
	   at 0000.
	   However, a NMI causes a successful reset. Maybe the hardware generates a
	   NMI short after power on, therefore masking the bug? The NMI is generated
	   by the MB8841 custom microcontroller, so this could be a way to disguise
	   the copy protection.
	   Anyway, what I do here is just immediately generate the NMI, so the game
	   properly starts. */
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void kangaroo_mcu_state::machine_reset()
{
	kangaroo_state::machine_reset();

	m_mcu_clock = 0;
}


/*************************************
 *
 *  Custom CPU RAM snooping
 *
 *************************************/

// The security chip is a MB8841 with 2K internal ROM. Currently it's unknown what it really does, this just seems to do the trick -V-

uint8_t kangaroo_mcu_state::mcu_sim_r()
{
	return ++m_mcu_clock & 0x0f;
}

void kangaroo_mcu_state::mcu_sim_w(uint8_t data)
{
}



/*************************************
 *
 *  Coin control
 *
 *************************************/

void kangaroo_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void kangaroo_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0xbfff).w(FUNC(kangaroo_state::videoram_w));
	map(0xc000, 0xdfff).bankr(m_blitbank);
	map(0xe000, 0xe3ff).ram();
	map(0xe400, 0xe400).mirror(0x03ff).portr("DSW0");
	map(0xe800, 0xe80a).mirror(0x03f0).w(FUNC(kangaroo_state::video_control_w)).share(m_video_control);
	map(0xec00, 0xec00).mirror(0x00ff).portr("IN0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xed00, 0xed00).mirror(0x00ff).portr("IN1").w(FUNC(kangaroo_state::coin_counter_w));
	map(0xee00, 0xee00).mirror(0x00ff).portr("IN2");
}

void kangaroo_mcu_state::main_map(address_map &map)
{
	kangaroo_state::main_map(map);

	map(0xef00, 0xefff).rw(FUNC(kangaroo_mcu_state::mcu_sim_r), FUNC(kangaroo_mcu_state::mcu_sim_w));
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void kangaroo_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("audiocpu", 0);
	map(0x4000, 0x43ff).mirror(0x0c00).ram();
	map(0x6000, 0x6000).mirror(0x0fff).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x7000, 0x7000).mirror(0x0fff).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x8000, 0x8000).mirror(0x0fff).w("aysnd", FUNC(ay8910_device::address_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( fnkyfish )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
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


static INPUT_PORTS_START( kangaroo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x00, "Music" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "10000 30000" )
	PORT_DIPSETTING(    0x0c, "20000 40000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, "A 2C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "A 1C/1C B 1C/2C" )
	PORT_DIPSETTING(    0x40, "A 1C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x50, "A 1C/1C B 1C/4C" )
	PORT_DIPSETTING(    0x60, "A 1C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x70, "A 1C/1C B 1C/6C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(    0xa0, "A 1C/2C B 1C/5C" )
	PORT_DIPSETTING(    0xe0, "A 1C/2C B 1C/6C" )
	PORT_DIPSETTING(    0xb0, "A 1C/2C B 1C/10C" )
	PORT_DIPSETTING(    0xc0, "A 1C/2C B 1C/11C" )
	PORT_DIPSETTING(    0xd0, "A 1C/2C B 1C/12C" )
	// 0xe0 gives A 1/2 B 1/6
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static constexpr XTAL MASTER_CLOCK = 10_MHz_XTAL;

void kangaroo_state::nomcu(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &kangaroo_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(kangaroo_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", MASTER_CLOCK / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &kangaroo_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &kangaroo_state::sound_map); // yes, this is identical
	audiocpu.set_vblank_int("screen", FUNC(kangaroo_state::irq0_line_hold));


	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_raw(MASTER_CLOCK, 320*2, 0*2, 256*2, 260, 8, 248);
	screen.set_screen_update(FUNC(kangaroo_state::screen_update));

	PALETTE(config, m_palette, palette_device::BGR_3BIT);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "aysnd", MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.50);
}


void kangaroo_mcu_state::mcu(machine_config &config)
{
	nomcu(config);

	subdevice<cpu_device>("maincpu")->set_addrmap(AS_PROGRAM, &kangaroo_mcu_state::main_map);

	MB8841(config, "mcu", MASTER_CLOCK / 4 / 2).set_disable(); // not dumped
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( fnkyfish )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "tvg_64.0",    0x0000,  0x1000, CRC(af728803) SHA1(1cbbf863f0eb4c759d6037ef9d9d0f4586b7b570) )
	ROM_LOAD( "tvg_65.1",    0x1000,  0x1000, CRC(71959e6b) SHA1(7336cbf3eefd081cd657a56fb6a8fbdac1b51c2c) )
	ROM_LOAD( "tvg_66.2",    0x2000,  0x1000, CRC(5ccf68d4) SHA1(c885df8b2b1bcb578ceab6615caf633dac02a5b2) )
	ROM_LOAD( "tvg_67.3",    0x3000,  0x1000, CRC(938ff36f) SHA1(bf660217ff82d5850ab97238ed2e32199d04f8c9) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "tvg_68.8",    0x0000,  0x1000, CRC(d36bb2be) SHA1(330160161857407fda62f16e7f43b8833744fd34) )

	ROM_REGION( 0x4000, "blitter", 0 )
	ROM_LOAD( "tvg_69.v0",   0x0000, 0x1000, CRC(cd532d0b) SHA1(7a64f8bab1a0feafd53a4b81ac3b624a7c1bd26a) )
	ROM_LOAD( "tvg_71.v2",   0x1000, 0x1000, CRC(a59c9713) SHA1(60dafa3d5a70b7e727b7c4688f8f3125735c31ec) )
	ROM_LOAD( "tvg_70.v1",   0x2000, 0x1000, CRC(fd308ef1) SHA1(d07f964cab875b0e47f3469fa5211684a5725dfe) )
	ROM_LOAD( "tvg_72.v3",   0x3000, 0x1000, CRC(6ae9b584) SHA1(408d26f4cdcd2abf0667fdc9c6eae58c9052981d) )
ROM_END


ROM_START( kangaroo )
	ROM_REGION( 0x6000, "maincpu", 0 ) // On TVG-1-CPU-B board
	ROM_LOAD( "tvg_75.0",    0x0000, 0x1000, CRC(0d18c581) SHA1(0e0f89d644b79e887c53e5294783843ca7e875ba) ) // IC7
	ROM_LOAD( "tvg_76.1",    0x1000, 0x1000, CRC(5978d37a) SHA1(684c1092de4a0927a03752903c86c3bbe99e868a) ) // IC8
	ROM_LOAD( "tvg_77.2",    0x2000, 0x1000, CRC(522d1097) SHA1(09fe627a46d32df2e098d9fad7757f9d61bef41f) ) // IC9
	ROM_LOAD( "tvg_78.3",    0x3000, 0x1000, CRC(063da970) SHA1(582ff21dd46c651f07a4846e0f8a7544a5891988) ) // IC10
	ROM_LOAD( "tvg_79.4",    0x4000, 0x1000, CRC(9e5cf8ca) SHA1(015387f038c5670f88c9b22453d074bd9b2a129d) ) // IC16
	ROM_LOAD( "tvg_80.5",    0x5000, 0x1000, CRC(2fc18049) SHA1(31fcac8eb660739a1672346136a1581a5ef20325) ) // IC17

	ROM_REGION( 0x1000, "audiocpu", 0 ) // On TVG-1-CPU-B board
	ROM_LOAD( "tvg_81.8",    0x0000, 0x1000, CRC(fb449bfd) SHA1(f593a0339f47e121736a927587132aeb52704557) ) // IC24

	ROM_REGION( 0x0800, "mcu", 0 )  // internal ROM from the 8841 custom MCU
	ROM_LOAD( "mb8841.ic29",    0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x0800, "user1", 0 )    // data for the 8841 custom MCU
	ROM_LOAD( "tvg_82.12",   0x0000, 0x0800, CRC(57766f69) SHA1(94a7a557d8325799523d5e1a88653a9a3fbe34f9) ) // IC28

	ROM_REGION( 0x4000, "blitter", 0 ) // On TVG-1-VIDEO-B board
	ROM_LOAD( "tvg_83.v0",   0x0000, 0x1000, CRC(c0446ca6) SHA1(fca6ba565051337c0198c93b7b8477632e0dd0b6) ) // IC76
	ROM_LOAD( "tvg_85.v2",   0x1000, 0x1000, CRC(72c52695) SHA1(87f4715fbb7d509bd9cc4e71e2afb0d475bbac13) ) // IC77
	ROM_LOAD( "tvg_84.v1",   0x2000, 0x1000, CRC(e4cb26c2) SHA1(5016db9d48fdcfb757618659d063b90862eb0e90) ) // IC52
	ROM_LOAD( "tvg_86.v3",   0x3000, 0x1000, CRC(9e6a599f) SHA1(76b4eddb4efcd8189d8cc5962d8497e82885f212) ) // IC53
ROM_END


ROM_START( kangarooa )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "136008-101.ic7",    0x0000, 0x1000, CRC(0d18c581) SHA1(0e0f89d644b79e887c53e5294783843ca7e875ba) )
	ROM_LOAD( "136008-102.ic8",    0x1000, 0x1000, CRC(5978d37a) SHA1(684c1092de4a0927a03752903c86c3bbe99e868a) )
	ROM_LOAD( "136008-103.ic9",    0x2000, 0x1000, CRC(522d1097) SHA1(09fe627a46d32df2e098d9fad7757f9d61bef41f) )
	ROM_LOAD( "136008-104.ic10",   0x3000, 0x1000, CRC(063da970) SHA1(582ff21dd46c651f07a4846e0f8a7544a5891988) )
	ROM_LOAD( "136008-105.ic16",   0x4000, 0x1000, CRC(82a26c7d) SHA1(09087552dbe4d27df79396072c0f9b916f78f89b) )
	ROM_LOAD( "136008-106.ic17",   0x5000, 0x1000, CRC(3dead542) SHA1(0b5d329b1ebbacc650d06289b4e080304e728ea7) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "136008-107.ic24",   0x0000, 0x1000, CRC(fb449bfd) SHA1(f593a0339f47e121736a927587132aeb52704557) )

	ROM_REGION( 0x0800, "mcu", 0 )  // internal ROM from the 8841 custom MCU
	ROM_LOAD( "mb8841.ic29",          0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x0800, "user1", 0 )    // data for the 8841 custom MCU
	ROM_LOAD( "136008-112.ic28",   0x0000, 0x0800, CRC(57766f69) SHA1(94a7a557d8325799523d5e1a88653a9a3fbe34f9) )

	ROM_REGION( 0x4000, "blitter", 0 )
	ROM_LOAD( "136008-108.ic76",   0x0000, 0x1000, CRC(c0446ca6) SHA1(fca6ba565051337c0198c93b7b8477632e0dd0b6) )
	ROM_LOAD( "136008-110.ic77",   0x1000, 0x1000, CRC(72c52695) SHA1(87f4715fbb7d509bd9cc4e71e2afb0d475bbac13) )
	ROM_LOAD( "136008-109.ic52",   0x2000, 0x1000, CRC(e4cb26c2) SHA1(5016db9d48fdcfb757618659d063b90862eb0e90) )
	ROM_LOAD( "136008-111.ic53",   0x3000, 0x1000, CRC(9e6a599f) SHA1(76b4eddb4efcd8189d8cc5962d8497e82885f212) )
ROM_END


ROM_START( kangaroob )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "k1.ic7",      0x0000, 0x1000, CRC(0d18c581) SHA1(0e0f89d644b79e887c53e5294783843ca7e875ba) )
	ROM_LOAD( "k2.ic8",      0x1000, 0x1000, CRC(5978d37a) SHA1(684c1092de4a0927a03752903c86c3bbe99e868a) )
	ROM_LOAD( "k3.ic9",      0x2000, 0x1000, CRC(522d1097) SHA1(09fe627a46d32df2e098d9fad7757f9d61bef41f) )
	ROM_LOAD( "k4.ic10",     0x3000, 0x1000, CRC(063da970) SHA1(582ff21dd46c651f07a4846e0f8a7544a5891988) )
	ROM_LOAD( "k5.ic16",     0x4000, 0x1000, CRC(9e5cf8ca) SHA1(015387f038c5670f88c9b22453d074bd9b2a129d) )
	ROM_LOAD( "k6.ic17",     0x5000, 0x1000, CRC(7644504a) SHA1(7a8a4bd163d2cdf27390ab2ef65fb7fa6fc1a361) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k7.ic24",     0x0000, 0x1000, CRC(fb449bfd) SHA1(f593a0339f47e121736a927587132aeb52704557) )

	// MB8841 at IC29 and 2716 at IC28 not populated

	ROM_REGION( 0x4000, "blitter", 0 )
	ROM_LOAD( "k10.ic76",    0x0000, 0x1000, CRC(c0446ca6) SHA1(fca6ba565051337c0198c93b7b8477632e0dd0b6) )
	ROM_LOAD( "k11.ic77",    0x1000, 0x1000, CRC(72c52695) SHA1(87f4715fbb7d509bd9cc4e71e2afb0d475bbac13) )
	ROM_LOAD( "k8.ic52",     0x2000, 0x1000, CRC(e4cb26c2) SHA1(5016db9d48fdcfb757618659d063b90862eb0e90) )
	ROM_LOAD( "k9.ic53",     0x3000, 0x1000, CRC(9e6a599f) SHA1(76b4eddb4efcd8189d8cc5962d8497e82885f212) )
ROM_END


ROM_START( kangarool ) // runs on earlier revision TVG-1-CPU-A + TVG-1-VIDEO-A PCBs
	ROM_REGION( 0x6000, "maincpu", 0 ) // only ic17 differs from the parent
	ROM_LOAD( "tvg_75.ic7",  0x0000, 0x1000, CRC(0d18c581) SHA1(0e0f89d644b79e887c53e5294783843ca7e875ba) )
	ROM_LOAD( "tvg_76.ic8",  0x1000, 0x1000, CRC(5978d37a) SHA1(684c1092de4a0927a03752903c86c3bbe99e868a) )
	ROM_LOAD( "tvg_77.ic9",  0x2000, 0x1000, CRC(522d1097) SHA1(09fe627a46d32df2e098d9fad7757f9d61bef41f) )
	ROM_LOAD( "tvg_78.ic10", 0x3000, 0x1000, CRC(063da970) SHA1(582ff21dd46c651f07a4846e0f8a7544a5891988) )
	ROM_LOAD( "tvg_79.ic16", 0x4000, 0x1000, CRC(9e5cf8ca) SHA1(015387f038c5670f88c9b22453d074bd9b2a129d) )
	ROM_LOAD( "tvg_80.ic17", 0x5000, 0x1000, CRC(62df0271) SHA1(4043d90d33ff04729077be7956d30bf82add103c) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "tvg_81.ic24", 0x0000, 0x1000, CRC(fb449bfd) SHA1(f593a0339f47e121736a927587132aeb52704557) )

	ROM_REGION( 0x0800, "mcu", 0 )  // internal ROM from the 8841 custom MCU
	ROM_LOAD( "mb8841.ic29", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x0800, "user1", 0 )    // data for the 8841 custom MCU
	ROM_LOAD( "tvg_82.ic28", 0x0000, 0x0800, CRC(57766f69) SHA1(94a7a557d8325799523d5e1a88653a9a3fbe34f9) )

	ROM_REGION( 0x4000, "blitter", 0 )
	ROM_LOAD( "tvg_83.ic76", 0x0000, 0x1000, CRC(c0446ca6) SHA1(fca6ba565051337c0198c93b7b8477632e0dd0b6) )
	ROM_LOAD( "tvg_85.ic77", 0x1000, 0x1000, CRC(72c52695) SHA1(87f4715fbb7d509bd9cc4e71e2afb0d475bbac13) )
	ROM_LOAD( "tvg_84.ic52", 0x2000, 0x1000, CRC(e4cb26c2) SHA1(5016db9d48fdcfb757618659d063b90862eb0e90) )
	ROM_LOAD( "tvg_86.ic53", 0x3000, 0x1000, CRC(9e6a599f) SHA1(76b4eddb4efcd8189d8cc5962d8497e82885f212) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, fnkyfish,  0,        nomcu, fnkyfish, kangaroo_state,     empty_init, ROT90, "Sun Electronics",                            "Funky Fish",                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, kangaroo,  0,        mcu,   kangaroo, kangaroo_mcu_state, empty_init, ROT90, "Sun Electronics",                            "Kangaroo",                    MACHINE_SUPPORTS_SAVE )
GAME( 1982, kangarooa, kangaroo, mcu,   kangaroo, kangaroo_mcu_state, empty_init, ROT90, "Sun Electronics (Atari license)",            "Kangaroo (Atari)",            MACHINE_SUPPORTS_SAVE )
GAME( 1982, kangaroob, kangaroo, nomcu, kangaroo, kangaroo_state,     empty_init, ROT90, "bootleg",                                    "Kangaroo (bootleg)",          MACHINE_SUPPORTS_SAVE )
GAME( 1982, kangarool, kangaroo, mcu,   kangaroo, kangaroo_mcu_state, empty_init, ROT90, "Sun Electronics (Loewen-Automaten license)", "Kangaroo (Loewen-Automaten)", MACHINE_SUPPORTS_SAVE )
