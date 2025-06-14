// license:BSD-3-Clause
// copyright-holders:Roberto Ventura, Leandro Dardini, Yochizo, Nicola Salmoria
/******************************************************************************

UPL "sprite framebuffer" hardware

driver by Roberto Ventura, Leandro Dardini, Yochizo, Nicola Salmoria

The peculiar feature of this hardware is the ability to disable clearing of the
sprite framebuffer, therefore overdrawing new sprites on top of the ones drawn
in the previous frames.
When sprite overdrawing is enabled, not all sprites leave trails: only the ones
using specific color codes. and the other ones clear the sprite framebuffer like
an eraser. (See notes)


Game              Board
----------------  ---------
Ninja Kid II      UPL-87003
Mutant Night      UPL-87007
Ark Area          UPL-87007
Atomic Robo-Kid   UPL-88013
Omega Fighter     UPL-89016


Hardware Overview:
------------------
1xZ80 @ 6MHz (main CPU)
1xZ80 @ 5MHz (sound CPU)
2xYM2203 @ 1.5MHz
PCM samples (Ninja Kid II only?)

Resolution: 256x192
96 sprites
Tilemaps: fg 256x256 (8x8) + bg 512x512 (16x16)
Atomic Robo-Kid adds two more bg tilemaps, at the same resolution
Omega Fighter further improves by increasing the bg resolution to 2048x512.

768 colors on screen out of a palette of 4096


Designers:
----------
Ninja Kid II:
* Game designer : Tsutomu Fuzisawa
* Program designer : Satoru Kinjo
* Character designers : Tsutomu Fuzisawa, Akemi Tsunoda
* Sound composer : Tsutomu Fuzisawa
* BGM creator: Mecano Associates
* Data maker : Takashi Hayashi

Mutant Night:
* Game designed by : Tsutomu Fuzisawa
* Characters designers : Tsutomu Fuzisawa, Takashige Shichijyo, Akemi Tsunoda, Noriko Nihei
* Programmer : Takashi Hayashi
* BGM & Sound composer : Yukari Shimada
* Data make : Tsutomu Fuzisawa, Takashige Shichijyo

Ark Area:
* Tsutomu Fuzisawa, Hiropi, Mingma, Nihei, Nozawa

Atomic Robo-Kid:
* Game designer : Tsutomu Fuzisawa
* Chief programmer : Toshio Arai
* Character designers : Tsutomu Fuzisawa, Tokuhisa Tazima
* Background designers : Noriko Nihei, Akemi Tsunoda
* Effects : Kohji Abe
* Compose & Music : Mecano Associates

Omega Fighter:
* Iwatani, Nobuyuki Narita, Abe, Akemi Tsunoda, Nikei, Shigksa, T.T, Ohnuma


Notes:
------
- Press 2 to skip the settings screen on startup.

- The sound CPU in Ninja Kid II is a Sega MC8123 encrypted Z80. In some sets, the
  ROM is replaced with a double-sized decrypted version. They are presumably
  bootlegs but this isn't known for sure.

- For Ninja Kid II: On Nov 15, 2008 a fully decrypted sound rom was created by
  www.segaresurrection.com It allows you to replace the NEC MC8123 and encrypted
  sound rom with a standard Z80B and the newly decrypted sound without any other
  mods to the Z80 & sound rom as required by the other "double-sized" version.
  The CRCs are listed here so it doesn't show up as a newly "found" bootleg version:
  nk2_06.rom   CRC32: 73b1b0d2  SHA1: 56add8c5f959a86b98aa4870b32aa23455be21ef

- The Ninja Kid II sound program writes to unmapped locations 0xeff5, 0xeff6, and
  0xefee due to a bug. This happens in both the encrypted and decrypted versions,
  so it appears to be a genuine bug with no ill effect.

- All the games write to the PCM sample player port used by Ninja Kid II (they
  all write the 0xF0 "silence" command), however since these games don't have PCM
  samples it seems unlikely that the boards actually have the PCM circuitry. The
  command written might be a leftover from the code used for Ninja Kid II.
  Atomic Robo-Kid definitely doesn't have the DAC and counters. The other boards
  have not been verified.

- The "credit service" in Ninja Kid II gives "extra credit(s)".
  5C_1C -> 5C_1C 10C_2C 15C_4C# 20C_5C 25C_6C 30C_8C# 35C_9C 40C_10C 45C_12C#
  4C_1C -> 4C_1C  8C_2C 12C_4C# 16C_5C 20C_6C 24C_8C# 28C_9C 32C_10C 36C_12C#
  3C_1C -> 3C_1C  6C_2C  9C_4C# 12C_5C 15C_6C 18C_8C# 21C_9C 24C_10C 27C_12C#
  2C_1C -> 2C_1C  4C_2C  6C_4C#  8C_5C 10C_6C 12C_8C# 14C_9C 16C_10C 18C_12C#
  1C_1C -> 1C_1C  2C_2C  3C_4C#  4C_5C  5C_6C  6C_8C#  7C_9C  8C_10C  9C_12C#
  1C_2C -> 1C_2C  2C_6C# 3C_10C# 4C_12C
  1C_3C -> 1C_3C  2C_6C  3C_12C#
  1C_4C -> 1C_4C  2C_8C  3C_12C                   '#' = added extra credit(s)

- Ark Area has no explicit copyright year on the title screen, however it was
  reportedly released in December 1987.
  Text in the ROM says:
  -ARK AREA-
  DIRECTED BY
  UPL COMPANY LIMITED 1988
  UPL-87007 BORD A STANDARD.
  This all appears in the end credits, except for "1988"

- On levels 16 and 23 of Ark Area, sprites leave trails on screen in a very
  awkward way. This has been verified to happen on the real PCB.

- To enter the level code in Atomic Robokid, after inserting coins press 1P start
  while keeping pressed button 1.
  Codes are given on the continue screen after Act 5.

- Omega Fighter has some unknown protection device interfacing with the player
  and dip switch inputs. There isn't enough evidence to determine what the
  device does, so it is roughly simulated just enough to get the game running.
  Most of the time the device just passes over the inputs. It is interrogated
  for protection check only on startup.

- Omega Fighter does some very strange reads from unmapped memory location C1E7.
  The purpose is unclear, it seems to be related to enemies on screen. The top
  three bits of the returned value ar ORed with register B, and some code is
  executed if the result is not 0. Nothing obvious happens either way.

- Overdrawing color codes
            OVERDRAW     STENCIL     UNKNOWN
  NINJAKD2  023459ABCDE  F           1678
    MNIGHT  0134568ABCDE F           279
   ARKAREA  012345679BDE             8ACF
   ROBOKID  EF           01236       45789ABCD
    OMEGAF  -            -           -         (unused)


TODO:
-----
- Find out how to sort color codes enabled overdrawing.


******************************************************************************/

#include "emu.h"
#include "ninjakd2.h"

#include "cpu/z80/mc8123.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"

#include "sound/ymopn.h"
#include "speaker.h"



/*************************************
 *
 *  Ninja Kid II PCM
 *
 *************************************/

// PCM playback is controlled by a 555 timer
#define NE555_FREQUENCY 16300   // measured on PCB
//#define NE555_FREQUENCY   (1.0f / (0.693 * (560 + 2*51) * 0.1e-6))    // theoretical: this gives 21.8kHz which is too high

void ninjakd2_state::ninjakd2_init_samples()
{
	const uint8_t* const rom = m_pcm_region->base();
	const int length = m_pcm_region->bytes();
	m_sampledata = std::make_unique<int16_t[]>(length);

	// convert unsigned 8-bit PCM to signed 16-bit
	for (int i = 0; i < length; ++i)
	{
		m_sampledata[i] = rom[i] << 7;
	}
}

void ninjakd2_state::ninjakd2_pcm_play_w(uint8_t data)
{
	const uint8_t* const rom = m_pcm_region->base();
	const int length = m_pcm_region->bytes();
	const int start = data << 8;

	// find end of sample
	int end = start;
	while (end < length && rom[end] != 0x00)
	{
		++end;
	}

	if (end - start)
	{
		m_pcm->start_raw(0, &m_sampledata[start], end - start, NE555_FREQUENCY);
	}
	else
	{
		m_pcm->stop(0);
	}
}



/*************************************
 *
 *  Omega Fighter I/O protection simulation
 *
 *************************************/

void omegaf_state::io_protection_start()
{
	// register for save states
	save_item(NAME(m_io_protection));
	save_item(NAME(m_io_protection_input));
	save_item(NAME(m_io_protection_tick));
}

void omegaf_state::io_protection_reset()
{
	// make sure protection starts in a known state
	m_io_protection[0] = 0;
	m_io_protection[1] = 0;
	m_io_protection[2] = 0;
	m_io_protection_input = 0;
	m_io_protection_tick = 0;
}

uint8_t omegaf_state::io_protection_r(offs_t offset)
{
	uint8_t result = 0xff;

	switch (m_io_protection[1] & 3)
	{
		case 0:
			switch (offset)
			{
				case 1:
					switch (m_io_protection[0] & 0xe0)
					{
						case 0x00:
							if (!machine().side_effects_disabled())
								++m_io_protection_tick;
							if (m_io_protection_tick & 1)
							{
								result = 0x00;
							}
							else
							{
								switch (m_io_protection_input)
								{
									// first interrogation
									// this happens just after setting mode 0.
									// input is not explicitly loaded so could be anything
									case 0x00:
										result = 0x80 | 0x02;
										break;

									// second interrogation
									case 0x8c:
										result = 0x80 | 0x1f;
										break;

									// third interrogation
									case 0x89:
										result = 0x80 | 0x0b;
										break;
								}
							}
							break;

						case 0x20:
							result = 0xc7;
							break;

						case 0x60:
							result = 0x00;
							break;

						case 0x80:
							result = 0x20 | (m_io_protection_input & 0x1f);
							break;

						case 0xc0:
							result = 0x60 | (m_io_protection_input & 0x1f);
							break;
					}
					break;
			}
			break;

		case 1: // dip switches
			switch (offset)
			{
				case 0:
				case 1: result = m_dsw_io[offset & 1]->read(); break;
				case 2: result = 0x02; break;
			}
			break;

		case 2: // player inputs
			switch (offset)
			{
				case 0:
				case 1: result = m_pad_io[offset & 1]->read(); break;
				case 2: result = 0x01; break;
			}
			break;
	}

	return result;
}

void omegaf_state::io_protection_w(offs_t offset, uint8_t data)
{
	// load parameter on c006 bit 0 rise transition
	if (offset == 2 && (data & 1) && !(m_io_protection[2] & 1))
	{
		logerror("loading protection input %02x\n", m_io_protection[0]);
		m_io_protection_input = m_io_protection[0];
	}

	m_io_protection[offset] = data;
}



/*****************************************************************************/

void ninjakd2_state::bankselect_w(uint8_t data)
{
	m_mainbank->set_entry(data & m_rom_bank_mask);
}

void ninjakd2_state::soundreset_w(uint8_t data)
{
	// bit 4 resets sound CPU
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7 flips screen
	flip_screen_set(data & 0x80);

	// other bits unused
}

template<int Layer>
void robokid_state::robokid_bg_bank_w(uint8_t data)
{
	m_robokid_bg_bank[Layer] = data & m_vram_bank_mask;
}

template<int Layer>
uint8_t robokid_state::robokid_bg_videoram_r(offs_t offset)
{
	return m_robokid_bg_videoram[Layer][(m_robokid_bg_bank[Layer] << 10) | offset];
}

template<int Layer>
void robokid_state::robokid_bg_videoram_w(offs_t offset, uint8_t data)
{
	int const address = (m_robokid_bg_bank[Layer] << 10 ) | offset;

	m_robokid_bg_videoram[Layer][address] = data;
	m_robokid_tilemap[Layer]->mark_tile_dirty(address >> 1);
}

template<int Layer>
void robokid_state::robokid_bg_ctrl_w(offs_t offset, uint8_t data)
{
	bg_ctrl(offset, data, m_robokid_tilemap[Layer]);
}

// omega fighter compares port $c1e7 with and $e0
// returning 0 and no small enemies shoot any bullet.
// returning 0xff seems enough
// TODO: find a better reference and verify if there are more gameplay quirks, this might really be anything!
uint8_t omegaf_state::unk_r()
{
	return 0xff;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void ninjakd2_state::ninjakd2_main_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc000).portr("KEYCOIN");
	map(0xc001, 0xc001).portr("PAD1");
	map(0xc002, 0xc002).portr("PAD2");
	map(0xc003, 0xc003).portr("DIPSW1");
	map(0xc004, 0xc004).portr("DIPSW2");
	map(0xc200, 0xc200).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc201, 0xc201).w(FUNC(ninjakd2_state::soundreset_w));
	map(0xc202, 0xc202).w(FUNC(ninjakd2_state::bankselect_w));
	map(0xc203, 0xc203).w(FUNC(ninjakd2_state::sprite_overdraw_w));
	map(0xc208, 0xc20c).w(FUNC(ninjakd2_state::ninjakd2_bg_ctrl_w));
	map(0xc800, 0xcdff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd000, 0xd7ff).ram().w(FUNC(ninjakd2_state::fgvideoram_w)).share("fg_videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(ninjakd2_state::ninjakd2_bgvideoram_w)).share("bg_videoram");
	map(0xe000, 0xf9ff).ram();
	map(0xfa00, 0xffff).ram().share(m_spriteram);
}

void mnight_state::mnight_main_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xd9ff).ram();
	map(0xda00, 0xdfff).ram().share(m_spriteram);
	map(0xe000, 0xe7ff).ram().w(FUNC(mnight_state::ninjakd2_bgvideoram_w)).share("bg_videoram");
	map(0xe800, 0xefff).ram().w(FUNC(mnight_state::fgvideoram_w)).share("fg_videoram");
	map(0xf000, 0xf5ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xf800).portr("KEYCOIN");
	map(0xf801, 0xf801).portr("PAD1");
	map(0xf802, 0xf802).portr("PAD2");
	map(0xf803, 0xf803).portr("DIPSW1");
	map(0xf804, 0xf804).portr("DIPSW2");
	map(0xfa00, 0xfa00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xfa01, 0xfa01).w(FUNC(mnight_state::soundreset_w));
	map(0xfa02, 0xfa02).w(FUNC(mnight_state::bankselect_w));
	map(0xfa03, 0xfa03).w(FUNC(mnight_state::sprite_overdraw_w));
	map(0xfa08, 0xfa0c).w(FUNC(mnight_state::ninjakd2_bg_ctrl_w));
}


void robokid_state::robokid_main_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc800, 0xcfff).ram().w(FUNC(robokid_state::fgvideoram_w)).share("fg_videoram");
	map(0xd000, 0xd3ff).rw(FUNC(robokid_state::robokid_bg_videoram_r<2>), FUNC(robokid_state::robokid_bg_videoram_w<2>));   // banked
	map(0xd400, 0xd7ff).rw(FUNC(robokid_state::robokid_bg_videoram_r<1>), FUNC(robokid_state::robokid_bg_videoram_w<1>));   // banked
	map(0xd800, 0xdbff).rw(FUNC(robokid_state::robokid_bg_videoram_r<0>), FUNC(robokid_state::robokid_bg_videoram_w<0>));   // banked
	map(0xdc00, 0xdc00).portr("KEYCOIN").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xdc01, 0xdc01).portr("PAD1").w(FUNC(robokid_state::soundreset_w));
	map(0xdc02, 0xdc02).portr("PAD2").w(FUNC(robokid_state::bankselect_w));
	map(0xdc03, 0xdc03).portr("DIPSW1").w(FUNC(robokid_state::sprite_overdraw_w));
	map(0xdc04, 0xdc04).portr("DIPSW2");
	map(0xdd00, 0xdd04).w(FUNC(robokid_state::robokid_bg_ctrl_w<0>));
	map(0xdd05, 0xdd05).w(FUNC(robokid_state::robokid_bg_bank_w<0>));
	map(0xde00, 0xde04).w(FUNC(robokid_state::robokid_bg_ctrl_w<1>));
	map(0xde05, 0xde05).w(FUNC(robokid_state::robokid_bg_bank_w<1>));
	map(0xdf00, 0xdf04).w(FUNC(robokid_state::robokid_bg_ctrl_w<2>));
	map(0xdf05, 0xdf05).w(FUNC(robokid_state::robokid_bg_bank_w<2>));
	map(0xe000, 0xf9ff).ram();
	map(0xfa00, 0xffff).ram().share(m_spriteram);
}


void omegaf_state::omegaf_main_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc000).portr("KEYCOIN").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc001, 0xc003).r(FUNC(omegaf_state::io_protection_r));
	map(0xc001, 0xc001).w(FUNC(omegaf_state::soundreset_w));
	map(0xc002, 0xc002).w(FUNC(omegaf_state::bankselect_w));
	map(0xc003, 0xc003).w(FUNC(omegaf_state::sprite_overdraw_w));
	map(0xc004, 0xc006).w(FUNC(omegaf_state::io_protection_w));
	map(0xc100, 0xc104).w(FUNC(omegaf_state::robokid_bg_ctrl_w<0>));
	map(0xc105, 0xc105).w(FUNC(omegaf_state::robokid_bg_bank_w<0>));
	map(0xc1e7, 0xc1e7).r(FUNC(omegaf_state::unk_r)); // see notes
	map(0xc200, 0xc204).w(FUNC(omegaf_state::robokid_bg_ctrl_w<1>));
	map(0xc205, 0xc205).w(FUNC(omegaf_state::robokid_bg_bank_w<1>));
	map(0xc300, 0xc304).w(FUNC(omegaf_state::robokid_bg_ctrl_w<2>));
	map(0xc305, 0xc305).w(FUNC(omegaf_state::robokid_bg_bank_w<2>));
	map(0xc400, 0xc7ff).rw(FUNC(omegaf_state::robokid_bg_videoram_r<0>), FUNC(omegaf_state::robokid_bg_videoram_w<0>));   // banked
	map(0xc800, 0xcbff).rw(FUNC(omegaf_state::robokid_bg_videoram_r<1>), FUNC(omegaf_state::robokid_bg_videoram_w<1>));   // banked
	map(0xcc00, 0xcfff).rw(FUNC(omegaf_state::robokid_bg_videoram_r<2>), FUNC(omegaf_state::robokid_bg_videoram_w<2>));   // banked
	map(0xd000, 0xd7ff).ram().w(FUNC(omegaf_state::fgvideoram_w)).share("fg_videoram");
	map(0xd800, 0xdfff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe000, 0xf9ff).ram();
	map(0xfa00, 0xffff).ram().share(m_spriteram);
}


void ninjakd2_state::ninjakid_nopcm_sound_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void ninjakd2_state::ninjakd2_sound_cpu(address_map &map)
{
	ninjakid_nopcm_sound_cpu(map);
	map(0xf000, 0xf000).w(FUNC(ninjakd2_state::ninjakd2_pcm_play_w));
}

void ninjakd2_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).rom().region("soundcpu", 0x8000);
}

void ninjakd2_state::ninjakd2_sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("2203.1", FUNC(ym2203_device::write));
	map(0x80, 0x81).w("2203.2", FUNC(ym2203_device::write));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("KEYCOIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    // keep pressed during boot to enter service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("PAD1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PAD2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( common_2p )
	PORT_START("KEYCOIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    // keep pressed during boot to enter service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("PAD1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PAD2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

	/*************************************************************
	Note:
	These all games' DIP switch manufacturer settings are All Off.
	(looked into Japanese manuals only)
	*************************************************************/

static INPUT_PORTS_START( ninjakd2 )
	PORT_INCLUDE(common)

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x04, "20000 and every 50000" )
	PORT_DIPSETTING(    0x06, "30000 and every 50000" )
	PORT_DIPSETTING(    0x02, "50000 and every 50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes )  )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("DIPSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Credit Service" ) PORT_DIPLOCATION("SW2:6")  // extra credit(s)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit, 6/4" )       PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x18, "1 Coin/1 Credit, 3/4" )        PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "1 Coin/2 Credits, 2/6, 3/10" ) PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "1 Coin/3 Credits, 3/12" )      PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, "5 Coins/1 Credit, 15/4" )      PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x20, "4 Coins/1 Credit, 12/4" )      PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x40, "3 Coins/1 Credit, 9/4" )       PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x60, "2 Coins/1 Credit, 6/4" )       PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xe0, "1 Coin/1 Credit, 3/4" )        PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, "1 Coin/2 Credits, 2/6, 3/10" ) PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xa0, "1 Coin/3 Credits, 3/12" )      PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )              PORT_CONDITION("DIPSW2", 0x04, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )              PORT_CONDITION("DIPSW2", 0x04, EQUALS, 0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( rdaction )
	PORT_INCLUDE(ninjakd2)

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x04, "20000 and every 50000" )
	PORT_DIPSETTING(    0x06, "30000 and every 50000" )
	PORT_DIPSETTING(    0x02, "50000 and every 100000" )
INPUT_PORTS_END

static INPUT_PORTS_START( mnight )
	PORT_INCLUDE(common)

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "30k and every 50k" )
	PORT_DIPSETTING(    0x00, "50k and every 80k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x08, 0x08, "Infinite Lives" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off )  )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DIPSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( arkarea )
	PORT_INCLUDE(common_2p)

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:5" )  // manual says "Table_Type Off=Table On=Upright", but not work?
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3")  // listed on Japanese manual only ?
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "50000 and every 50000" )
	PORT_DIPSETTING(    0x00, "100000 and every 100000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DIPSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:1" )
INPUT_PORTS_END

static INPUT_PORTS_START( robokid )
	PORT_INCLUDE(common)

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "50000 and every 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DIPSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( robokidj )
	PORT_INCLUDE(robokid)

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "30000 and every 50000" )
	PORT_DIPSETTING(    0x00, "50000 and every 80000" )
INPUT_PORTS_END

static INPUT_PORTS_START( omegaf )
	PORT_INCLUDE(common_2p)

	PORT_MODIFY("KEYCOIN")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:4" )  // manual says "Off=Table_Type On=Upright_Type", but not work?
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("DIPSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x00, "200000" )
	PORT_DIPSETTING(    0x03, "300000" )
	PORT_DIPSETTING(    0x01, "500000" )
	PORT_DIPSETTING(    0x02, "1000000" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static GFXDECODE_START( gfx_ninjakd2 )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_msb,               0x200, 16)    // fg
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x100, 16)    // sprites
	GFXDECODE_ENTRY( "tiles1",  0, gfx_8x8x4_row_2x2_group_packed_msb, 0x000, 16)    // bg
GFXDECODE_END

static GFXDECODE_START( gfx_robokid )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_msb,               0x300, 16) // fg
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x200, 16) // sprites
	GFXDECODE_ENTRY( "tiles1",  0, gfx_8x8x4_col_2x2_group_packed_msb, 0x000, 16) // bg0
	GFXDECODE_ENTRY( "tiles2",  0, gfx_8x8x4_col_2x2_group_packed_msb, 0x000, 16) // bg1
	GFXDECODE_ENTRY( "tiles3",  0, gfx_8x8x4_col_2x2_group_packed_msb, 0x000, 16) // bg2
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void ninjakd2_state::machine_start()
{
	// initialize main Z80 bank
	int const num_banks = (memregion("maincpu")->bytes() - 0x10000) / 0x4000;
	m_mainbank->configure_entries(0, num_banks, memregion("maincpu")->base() + 0x10000, 0x4000);
	// ...

	m_rom_bank_mask = num_banks - 1;
}

void ninjakd2_state::machine_reset()
{
	m_mainbank->set_entry(0);
}

void omegaf_state::machine_start()
{
	io_protection_start();

	ninjakd2_state::machine_start();
}

void omegaf_state::machine_reset()
{
	io_protection_reset();

	ninjakd2_state::machine_reset();
}

/*****************************************************************************/

static constexpr XTAL MAIN_CLOCK_12 = XTAL(12'000'000);
static constexpr XTAL MAIN_CLOCK_5 = XTAL(5'000'000);

void ninjakd2_state::ninjakd2_core(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MAIN_CLOCK_12/2); // verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ninjakd2_state::ninjakd2_main_cpu);

	Z80(config, m_soundcpu, MAIN_CLOCK_5);     // verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &ninjakd2_state::ninjakd2_sound_cpu);
	m_soundcpu->set_addrmap(AS_IO, &ninjakd2_state::ninjakd2_sound_io);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.61);    // verified on pcb
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 4*8, 28*8-1);
	m_screen->set_screen_update(FUNC(ninjakd2_state::screen_update_ninjakd2));
	m_screen->screen_vblank().set(FUNC(ninjakd2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ninjakd2);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 0x300);
	m_palette->set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym2203_1(YM2203(config, "2203.1", MAIN_CLOCK_12/8)); // verified
	ym2203_1.irq_handler().set_inputline("soundcpu", 0);
	ym2203_1.add_route(0, "mono", 0.10);
	ym2203_1.add_route(1, "mono", 0.10);
	ym2203_1.add_route(2, "mono", 0.10);
	ym2203_1.add_route(3, "mono", 0.50);

	ym2203_device &ym2203_2(YM2203(config, "2203.2", MAIN_CLOCK_12/8)); // verified
	ym2203_2.add_route(0, "mono", 0.10);
	ym2203_2.add_route(1, "mono", 0.10);
	ym2203_2.add_route(2, "mono", 0.10);
	ym2203_2.add_route(3, "mono", 0.50);

	SAMPLES(config, m_pcm);
	m_pcm->set_channels(1);
	m_pcm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void ninjakd2_state::ninjakd2(machine_config &config)
{
	ninjakd2_core(config);
	MC8123(config.replace(), m_soundcpu, MAIN_CLOCK_5); // verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &ninjakd2_state::ninjakd2_sound_cpu);
	m_soundcpu->set_addrmap(AS_IO, &ninjakd2_state::ninjakd2_sound_io);
	m_soundcpu->set_addrmap(AS_OPCODES, &ninjakd2_state::decrypted_opcodes_map);
}

void ninjakd2_state::ninjakd2b(machine_config &config)
{
	ninjakd2_core(config);
	m_soundcpu->set_addrmap(AS_PROGRAM, &ninjakd2_state::ninjakd2_sound_cpu);
	m_soundcpu->set_addrmap(AS_OPCODES, &ninjakd2_state::decrypted_opcodes_map);
}

void mnight_state::mnight(machine_config &config)
{
	ninjakd2_core(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mnight_state::mnight_main_cpu);
	m_soundcpu->set_addrmap(AS_PROGRAM, &mnight_state::ninjakid_nopcm_sound_cpu);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(mnight_state,mnight)

	// sound hardware
	config.device_remove("pcm");
}

void mnight_state::arkarea(machine_config &config)
{
	mnight(config);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(mnight_state,arkarea)
}

void robokid_state::robokid(machine_config &config)
{
	mnight(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &robokid_state::robokid_main_cpu);

	// video hardware
	m_gfxdecode->set_info(gfx_robokid);
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 0x400); // RAM is this large, but still only 0x300 colors used
	m_palette->set_endianness(ENDIANNESS_BIG);

	MCFG_VIDEO_START_OVERRIDE(robokid_state,robokid)

	m_screen->set_screen_update(FUNC(robokid_state::screen_update_robokid));
}

void omegaf_state::omegaf(machine_config &config)
{
	robokid(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &omegaf_state::omegaf_main_cpu);

	m_soundcpu->set_addrmap(AS_PROGRAM, &omegaf_state::ninjakid_nopcm_sound_cpu);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(omegaf_state,omegaf)

	m_screen->set_screen_update(FUNC(omegaf_state::screen_update_omegaf));
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ninjakd2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3s", 0x00000, 0x8000, CRC(3cdbb906) SHA1(f48f82528b5fc581ee3b1ccd0ef9cdecc7249bb3) )
	ROM_LOAD( "2.3q", 0x10000, 0x8000, CRC(b5ce9a1a) SHA1(295a7e1d41e1a8ee45f1250086a0c9314837eded) ) // banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // NEC MC-8123 custom CPU block
	ROM_LOAD( "6.3h", 0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) ) // encrypted

	ROM_REGION( 0x2000, "soundcpu:key", 0 ) // MC8123 key
	ROM_LOAD( "ninjakd2.key", 0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) ) // unsigned 8-bit pcm samples
ROM_END

ROM_START( ninjakd2a )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3s", 0x00000, 0x8000, CRC(e6adca65) SHA1(33d483dde0853f37455cde32b461f4e919601b4b) ) // sldh
	ROM_LOAD( "2.3q", 0x10000, 0x8000, CRC(d9284bd1) SHA1(e790fb1a718a1f7997931f2f390fe053655f231d) ) // sldh - banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) ) // sldh

	ROM_REGION( 2*0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.bin", 0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) ) // decrypted opcodes
	ROM_CONTINUE(           0x00000, 0x8000 )                                                               // decrypted data

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) ) // unsigned 8-bit pcm samples
ROM_END

ROM_START( ninjakd2b )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3s", 0x00000, 0x8000, CRC(cb4f4624) SHA1(4fc66641adc0a2c0eca332f27c5777df62fa507b) ) // sldh
	ROM_LOAD( "2.3q", 0x10000, 0x8000, CRC(0ad0c100) SHA1(c5bbc107ba07bd6950bb4d7377e827c084b8229b) ) // sldh - banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )

	ROM_REGION( 2*0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.bin", 0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) ) // 6.3g  decrypted opcodes
	ROM_CONTINUE(           0x00000, 0x8000 )                                                               // decrypted data

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) ) // unsigned 8-bit pcm samples
ROM_END

ROM_START( ninjakd2c )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3u", 0x00000, 0x8000, CRC(06096412) SHA1(4a84a9326248ff899a04f32950b4c4a5ff58cf75) ) // sldh
	ROM_LOAD( "2.3t", 0x10000, 0x8000, CRC(9ed9a994) SHA1(ec95e09066ad51cf4514e269384b7609d6c345d9) ) // sldh - banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(800d4951) SHA1(878516bd03a61ac970cd9e8c35116f8ec3020e79) ) // sldh

	ROM_REGION( 0x10000, "soundcpu", 0 ) // NEC MC-8123 custom CPU block
	ROM_LOAD( "6.3h", 0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) ) // encrypted

	ROM_REGION( 0x2000, "soundcpu:key", 0 ) // MC8123 key
	ROM_LOAD( "ninjakd2.key", 0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // unsigned 8-bit pcm samples
ROM_END

ROM_START( rdaction )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3u", 0x00000, 0x8000, CRC(5c475611) SHA1(2da88a95b5d68b259c8ae48af1438a82a1d601c1) ) // sldh
	ROM_LOAD( "2.3s", 0x10000, 0x8000, CRC(a1e23bd2) SHA1(c3b6574dc9fa66b4f41c37754a0d20a865f8bc28) ) // sldh - banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) ) // sldh - == 5.3m from ninjakd2a

	ROM_REGION( 0x10000, "soundcpu", 0 ) // NEC MC-8123 custom CPU block
	ROM_LOAD( "6.3h", 0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) ) // encrypted

	ROM_REGION( 0x2000, "soundcpu:key", 0 ) // MC8123 key
	ROM_LOAD( "ninjakd2.key", 0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(0936b365) SHA1(3705f42b76ab474357e77c1a9b8e3755c7ab2c0c) ) // sldh - this rom contains the new title / license

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) ) // unsigned 8-bit pcm samples
ROM_END

// Found on an official UPL board with an original encrypted sound CPU block and official UPL serial number sticker
// Other "JT 104" sets have been found with mismatched ROMs and the decrypted sound CPU + ROM hack
ROM_START( jt104 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3u", 0x00000, 0x8000, CRC(5c475611) SHA1(2da88a95b5d68b259c8ae48af1438a82a1d601c1) ) // sldh - == 1.3u from rdaction
	ROM_LOAD( "2.3s", 0x10000, 0x8000, CRC(87a91d11) SHA1(08bb9a0a691681c86ddd551af96d5742daa878c3) ) // sldh - banked at 8000-bfff
	ROM_LOAD( "3.3r", 0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "4.3p", 0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "5.3m", 0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) ) // sldh - == 5.3m from ninjakd2a

	ROM_REGION( 0x10000, "soundcpu", 0 ) // NEC MC-8123 custom CPU block
	ROM_LOAD( "6.3h", 0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) ) // encrypted

	ROM_REGION( 0x2000, "soundcpu:key", 0 ) // MC8123 key
	ROM_LOAD( "ninjakd2.key", 0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n", 0x00000, 0x08000, CRC(c038fadb) SHA1(59e9b125ead3e9bdc9d66de75dffd58956eb922e) )  // sldh - this rom contains the new title / license

	ROM_REGION( 0x20000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "8.6l", 0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "7.6n", 0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "11.2n", 0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "10.2r", 0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "9.6c", 0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) ) // unsigned 8-bit pcm samples
ROM_END


ROM_START( mnight )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.j19",  0x00000, 0x8000, CRC(56678d14) SHA1(acf3a97ca29db8ab9cad69599c5567464af3ae44) )
	ROM_LOAD( "2.j17",  0x10000, 0x8000, CRC(2a73f88e) SHA1(0a7b769174f2b976650453d808cb23668dff0260) )   // banked at 8000-bfff
	ROM_LOAD( "3.j16",  0x18000, 0x8000, CRC(c5e42bb4) SHA1(1956e737ae393e987cb7e8eae520518f1e0f597f) )
	ROM_LOAD( "4.j14",  0x20000, 0x8000, CRC(df6a4f7a) SHA1(ce3cb84b514220d686b12c03567289fd23da0fe1) )
	ROM_LOAD( "5.j12",  0x28000, 0x8000, CRC(9c391d1b) SHA1(06e8c202337e3eba38c479e8b7e29a3f8ffc4ed1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6.j7",   0x00000, 0x10000, CRC(a0782a31) SHA1(8abd2f0b0c2c2eb876f324f7a095a5cdc773c187) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "13.b10", 0x00000, 0x08000, CRC(8c177a19) SHA1(328df41b5bacd1999f97d99781c6ef8afc9989a3) )

	ROM_REGION( 0x30000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "9.e11",  0x00000, 0x10000, CRC(4883059c) SHA1(53d4b9b0f0725c25e302ee1549a306778ec74d85) )
	ROM_LOAD( "8.e12",  0x10000, 0x10000, CRC(02b91445) SHA1(f0cf85f9e17c40248de16bca8df6d745e359b92d) )
	ROM_LOAD( "7.e14",  0x20000, 0x10000, CRC(9f08d160) SHA1(1a0041ad138e7e6598d4d03d7cbd52a7244557ac) )

	ROM_REGION( 0x30000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "12.b20", 0x00000, 0x10000, CRC(4d37e0f4) SHA1(a6d9aaccd97769197622cda45474e223c2ee1d98) )
	ROM_LOAD( "11.b22", 0x10000, 0x10000, CRC(b22cbbd3) SHA1(70984f1051fd236730d97011bc87dacb3ca38594) )
	ROM_LOAD( "10.b23", 0x20000, 0x10000, CRC(65714070) SHA1(48f3c130c97d00e8f0535904dc2237277067c475) )
ROM_END

ROM_START( mnightj )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.j19",  0x00000, 0x8000, CRC(56678d14) SHA1(acf3a97ca29db8ab9cad69599c5567464af3ae44) )
	ROM_LOAD( "2.j17",  0x10000, 0x8000, CRC(2a73f88e) SHA1(0a7b769174f2b976650453d808cb23668dff0260) )   // banked at 8000-bfff
	ROM_LOAD( "3.j16",  0x18000, 0x8000, CRC(c5e42bb4) SHA1(1956e737ae393e987cb7e8eae520518f1e0f597f) )
	ROM_LOAD( "4.j14",  0x20000, 0x8000, CRC(df6a4f7a) SHA1(ce3cb84b514220d686b12c03567289fd23da0fe1) )
	ROM_LOAD( "5.j12",  0x28000, 0x8000, CRC(9c391d1b) SHA1(06e8c202337e3eba38c479e8b7e29a3f8ffc4ed1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6.j7",   0x00000, 0x10000, CRC(a0782a31) SHA1(8abd2f0b0c2c2eb876f324f7a095a5cdc773c187) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "13.b10", 0x00000, 0x08000, CRC(37b8221f) SHA1(ac86e0ae8039fd30a028a893d08ce099f7765615) )

	ROM_REGION( 0x30000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "9.e11",  0x00000, 0x10000, CRC(4883059c) SHA1(53d4b9b0f0725c25e302ee1549a306778ec74d85) )
	ROM_LOAD( "8.e12",  0x10000, 0x10000, CRC(02b91445) SHA1(f0cf85f9e17c40248de16bca8df6d745e359b92d) )
	ROM_LOAD( "7.e14",  0x20000, 0x10000, CRC(9f08d160) SHA1(1a0041ad138e7e6598d4d03d7cbd52a7244557ac) )

	ROM_REGION( 0x30000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "12.b20", 0x00000, 0x10000, CRC(4d37e0f4) SHA1(a6d9aaccd97769197622cda45474e223c2ee1d98) )
	ROM_LOAD( "11.b22", 0x10000, 0x10000, CRC(b22cbbd3) SHA1(70984f1051fd236730d97011bc87dacb3ca38594) )
	ROM_LOAD( "10.b23", 0x20000, 0x10000, CRC(65714070) SHA1(48f3c130c97d00e8f0535904dc2237277067c475) )
ROM_END

ROM_START( arkarea )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "arkarea.008",  0x00000, 0x8000, CRC(1ce1b5b9) SHA1(ab7755523d58736b124deb59779dedee870fd7d2) )
	ROM_LOAD( "arkarea.009",  0x10000, 0x8000, CRC(db1c81d1) SHA1(64a2f51c218d84c4eaeb8c5a5a3f0f4cdf5d174c) )   // banked at 8000-bfff
	ROM_LOAD( "arkarea.010",  0x18000, 0x8000, CRC(5a460dae) SHA1(e64d3662bb074a528cd71061621c0dd3765928b6) )
	ROM_LOAD( "arkarea.011",  0x20000, 0x8000, CRC(63f022c9) SHA1(414f52d2b9584f08285b261d1dafcc9e6e5e0c74) )
	ROM_LOAD( "arkarea.012",  0x28000, 0x8000, CRC(3c4c65d5) SHA1(e11f840f8b1da0933a0a4342f5fa1a17f0d6d3e2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "arkarea.013",  0x00000, 0x8000, CRC(2d409d58) SHA1(6344b43db5459691728c3f843b643c84ea71dd8e) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "arkarea.004",  0x00000, 0x08000, CRC(69e36af2) SHA1(2bccef8f396dcb5261af0140af04c95ee8ecae11) )

	ROM_REGION( 0x30000, "sprites", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "arkarea.007",  0x00000, 0x10000, CRC(d5684a27) SHA1(4961e8a5df2510afb1ef3e937d0a5d52e91893a3) )
	ROM_LOAD( "arkarea.006",  0x10000, 0x10000, CRC(2c0567d6) SHA1(f36a2a3ff487660f89470516617482331f008da0) )
	ROM_LOAD( "arkarea.005",  0x20000, 0x10000, CRC(9886004d) SHA1(4050756af5c00ab1a368780fe091460fd9e2cb05) )

	ROM_REGION( 0x30000, "tiles1", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "arkarea.003",  0x00000, 0x10000, CRC(6f45a308) SHA1(b6994fe1f50d5e9cf38d3efbd69a2c5f76f33c56) )
	ROM_LOAD( "arkarea.002",  0x10000, 0x10000, CRC(051d3482) SHA1(3ebef1a7280f52df6d5ee34e3d4e7567aac0c165) )
	ROM_LOAD( "arkarea.001",  0x20000, 0x10000, CRC(09d11ab7) SHA1(14f68e93e7173069f790493eafe9e1adc1a074cc) )
ROM_END

ROM_START( robokid )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "robokid1.18j", 0x00000, 0x08000, CRC(378c21fc) SHA1(58163bd6fbfa8385b1bd648cfde3d75bf81ac07d) )
	ROM_IGNORE(                        0x08000 )
	ROM_RELOAD(               0x10000, 0x10000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "robokid2.18k", 0x20000, 0x10000, CRC(ddef8c5a) SHA1(a1dd2f51205863c3d5d3527991d538ca8adf7587) )
	ROM_LOAD( "robokid3.15k", 0x30000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x40000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )

	// Atomic Robokid's Z80 is inside a module with a PROM, replacing the module with a plain Z80 does not work.
	// this PROM is from a bootleg, but appears to be an exact clone of the one found in the original module
	// see http://blog.system11.org/?p=1687 for details.  Until the original is verified it's marked as BAD_DUMP
	ROM_REGION( 0x100, "maincpu_prom", 0 )
	ROM_LOAD( "prom82s129.cpu", 0x000, 0x100, BAD_DUMP CRC(4dd96f67) SHA1(01b415d9e86ff0c5aad7cfe81e903f2c202bb541))

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )
ROM_END



ROM_START( robokidj3 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "robokid1.18j", 0x00000, 0x08000, CRC(77a9332a) SHA1(60464ff8bdea5ee2256f24210dc7246fcffb0fca) )
	ROM_IGNORE(                        0x08000 )
	ROM_RELOAD(               0x10000, 0x10000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "robokid2.18k", 0x20000, 0x10000, CRC(715ecee4) SHA1(1c00b9233b24ca70b0d097c2f62b85db280a9c9b) )
	ROM_LOAD( "robokid3.15k", 0x30000, 0x10000, CRC(ce12fa86) SHA1(73beaeea5b50ec90d612813845f8345a465750f7) )
	ROM_LOAD( "robokid4.12k", 0x40000, 0x10000, CRC(97e86600) SHA1(e140a2e323e9a91ae415fd5539b3e0226dff3a69) )

	ROM_REGION( 0x100, "maincpu_prom", 0 )
	ROM_LOAD( "prom82s129.cpu", 0x000, 0x100, BAD_DUMP CRC(4dd96f67) SHA1(01b415d9e86ff0c5aad7cfe81e903f2c202bb541))

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )
ROM_END


ROM_START( robokidj )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1.29",         0x00000, 0x08000, CRC(59a1e2ec) SHA1(71f9d28dd8d2cf77a27fab163ce9562e3e75a540) )
	ROM_IGNORE(                        0x08000 )
	ROM_RELOAD(               0x10000, 0x10000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "2.30",         0x20000, 0x10000, CRC(e3f73476) SHA1(bd1c8946d637df21432bd52ae9324255251570b9) )
	ROM_LOAD( "robokid3.15k", 0x30000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x40000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )

	// see note in above set
	ROM_REGION( 0x100, "maincpu_prom", 0 )
	ROM_LOAD( "prom82s129.cpu", 0x000, 0x100, BAD_DUMP CRC(4dd96f67) SHA1(01b415d9e86ff0c5aad7cfe81e903f2c202bb541))

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )
ROM_END

ROM_START( robokidj2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1_rom29.18j",  0x00000, 0x08000, CRC(969fb951) SHA1(aa32f0cb33ba2ccbb933dab5444a7e0dbbb84b3d) )
	ROM_IGNORE(                        0x08000 )
	ROM_RELOAD(               0x10000, 0x10000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "2_rom30.18k",  0x20000, 0x10000, CRC(c0228b63) SHA1(8f7e3a29a35723abc8b10bf511fc8611e31a2961) )
	ROM_LOAD( "robokid3.15k", 0x30000, 0x10000, CRC(05295ec3) SHA1(33dd0853a2064cb4301cfbdc7856def81f6e1223) )
	ROM_LOAD( "robokid4.12k", 0x40000, 0x10000, CRC(3bc3977f) SHA1(da394e12d197b0e109b03c854da06b1267bd9d59) )

	// see note in above set
	ROM_REGION( 0x100, "maincpu_prom", 0 )
	ROM_LOAD( "prom82s129.cpu", 0x000, 0x100, BAD_DUMP CRC(4dd96f67) SHA1(01b415d9e86ff0c5aad7cfe81e903f2c202bb541))

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robokid.k7",   0x00000, 0x10000, CRC(f490a2e9) SHA1(861d1256c090ce3d1f45f95cc894affbbc3f1466) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "robokid.12a",  0x00000, 0x10000, CRC(e64d1c10) SHA1(d1073c80c9788aba65410f88691747a37b2a9d4a) )
	ROM_LOAD( "robokid.14a",  0x10000, 0x10000, CRC(8f9371e4) SHA1(0ea06d62bf4673ebda49a849cead832a24e5b886) )
	ROM_LOAD( "robokid.15a",  0x20000, 0x10000, CRC(469204e7) SHA1(8c2e94635b2b304e7dfa2e6ad58ba526dcf02453) )
	ROM_LOAD( "robokid.16a",  0x30000, 0x10000, CRC(4e340815) SHA1(d204b830c5809f25f7dfa451bbcbeda8b81ced54) )
	ROM_LOAD( "robokid.17a",  0x40000, 0x10000, CRC(f0863106) SHA1(ff7e44d0aa5a07ec9a7eddef1a55181bd2e867b1) )
	ROM_LOAD( "robokid.18a",  0x50000, 0x10000, CRC(fdff7441) SHA1(843b2c92bbc6f7319568677d50cbd9b03475b34a) )
ROM_END

ROM_START( omegaf )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1.5",          0x00000, 0x08000, CRC(57a7fd96) SHA1(65ca290b48f8579fcce00db5b3b3f8694667a136) )
	ROM_IGNORE(                        0x18000 )
	ROM_RELOAD(               0x10000, 0x20000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "6.4l",         0x30000, 0x20000, CRC(6277735c) SHA1(b0f91f0cc51d424a1a7834c126736f24c2e23c17) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "7.7m",         0x00000, 0x10000, CRC(d40fc8d5) SHA1(4f615a0fb786cafc20f82f0b5fa112a9c356378f) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "3back3.5f",    0x00000, 0x80000, CRC(c31cae56) SHA1(4cc2d0d70990ca04b0e3abd15e5afe183e98e4ab) )
ROM_END

ROM_START( omegafs )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "5.3l",         0x00000, 0x08000, CRC(503a3e63) SHA1(73420aecb653cd4fd3b6afe67d6f5726f01411dd) )
	ROM_IGNORE(                        0x18000 )
	ROM_RELOAD(               0x10000, 0x20000 )                                                                // banked at 8000-bfff
	ROM_LOAD( "6.4l",         0x30000, 0x20000, CRC(6277735c) SHA1(b0f91f0cc51d424a1a7834c126736f24c2e23c17) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "7.7m",         0x00000, 0x10000, CRC(d40fc8d5) SHA1(4f615a0fb786cafc20f82f0b5fa112a9c356378f) )

	ROM_REGION( 0x08000, "chars", 0 )    // fg tiles
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )

	ROM_REGION( 0x20000, "sprites", 0 )    // sprite tiles
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x80000, "tiles1", 0 )    // bg0 tiles
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, "tiles2", 0 )    // bg1 tiles
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, "tiles3", 0 )    // bg2 tiles
	ROM_LOAD( "3back3.5f",    0x00000, 0x80000, CRC(c31cae56) SHA1(4cc2d0d70990ca04b0e3abd15e5afe183e98e4ab) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

/******************************************************************************

Gfx ROMs in the older games have an unusual layout, where a high address bit
(which is not the top bit) separates parts of the same tile. To make it possible
to decode graphics without resorting to ROM_CONTINUE trickery, this function
makes an address line rotation, bringing bit "bit" to bit 0 and shifting left
by one place all the intervening bits.

******************************************************************************/

void ninjakd2_state::lineswap_gfx_roms(const char *region, const int bit)
{
	const int length = memregion(region)->bytes();
	uint8_t *const src = memregion(region)->base();
	std::vector<uint8_t> temp(length);
	const int mask = (1 << (bit + 1)) - 1;

	for (int sa = 0; sa < length; sa++)
	{
		const int da = (sa & ~mask) | ((sa << 1) & mask) | ((sa >> bit) & 1);
		temp[da] = src[sa];
	}

	memcpy(src, &temp[0], length);
}

void ninjakd2_state::gfx_unscramble()
{
	lineswap_gfx_roms("chars", 13);     // fg tiles
	lineswap_gfx_roms("sprites", 14);   // sprites
	lineswap_gfx_roms("tiles1", 14);    // bg tiles
}


void ninjakd2_state::init_ninjakd2()
{
	downcast<mc8123_device &>(*m_soundcpu).decode(memregion("soundcpu")->base(), m_decrypted_opcodes, 0x8000);

	gfx_unscramble();
	ninjakd2_init_samples();
}

void ninjakd2_state::init_bootleg()
{
	memcpy(m_decrypted_opcodes, memregion("soundcpu")->base() + 0x10000, 0x8000);

	gfx_unscramble();
	ninjakd2_init_samples();
}

void mnight_state::init_mnight()
{
	gfx_unscramble();
}

/*****************************************************************************/

uint8_t robokid_state::motion_error_verbose_r()
{
	if (!machine().side_effects_disabled())
	{
		popmessage("%s MOTION ERROR, contact MAMEdev", machine().system().name);
		logerror("maincpu %04x MOTION ERROR\n", m_maincpu->pc());
	}
	return 0xe6;
}

void robokid_state::motion_error_kludge(uint16_t offset)
{
	// patch out rare "5268 MOTION ERROR" (MT 05024)
	// It looks like it's due to a buggy random number generator,
	// then it possibly happens on the real arcade cabinet too.
	// I doubt it is protection related, but you can never be sure.
	// Update 131016: there's also a 5208 VECTOR ERROR happening at worm mid-boss on stage 10, I'm prone to think it's
	// a timing/sprite fault on our side therefore marking as UNEMULATED_PROTECTION until this is resolved. -AS
	uint8_t *ROM = memregion("maincpu")->base() + offset;
	ROM[0] = 0xe6;
	ROM[1] = 0x03; // and 3
	ROM[2] = 0x18;
	ROM[3] = 0xf6; // jr $-8

	m_maincpu->space(AS_PROGRAM).install_read_handler(offset, offset, read8smo_delegate(*this, FUNC(robokid_state::motion_error_verbose_r)));
}

void robokid_state::init_robokid()
{
	motion_error_kludge(0x5247);
}

void robokid_state::init_robokidj()
{
	motion_error_kludge(0x5266);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,      PARENT,   MACHINE,   INPUT,    STATE,          INIT,     MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1987, ninjakd2,  0,        ninjakd2,  ninjakd2, ninjakd2_state, init_ninjakd2, ROT0,   "UPL",                             "Ninja-Kid II / NinjaKun Ashura no Shou (set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1987, ninjakd2a, ninjakd2, ninjakd2b, ninjakd2, ninjakd2_state, init_bootleg,  ROT0,   "UPL",                             "Ninja-Kid II / NinjaKun Ashura no Shou (set 2, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ninjakd2b, ninjakd2, ninjakd2b, rdaction, ninjakd2_state, init_bootleg,  ROT0,   "UPL",                             "Ninja-Kid II / NinjaKun Ashura no Shou (set 3, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ninjakd2c, ninjakd2, ninjakd2,  rdaction, ninjakd2_state, init_ninjakd2, ROT0,   "UPL",                             "Ninja-Kid II / NinjaKun Ashura no Shou (set 4)",           MACHINE_SUPPORTS_SAVE ) // close to set 3
GAME( 1987, rdaction,  ninjakd2, ninjakd2,  rdaction, ninjakd2_state, init_ninjakd2, ROT0,   "UPL (World Games license)",       "Rad Action / NinjaKun Ashura no Shou",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, jt104,     ninjakd2, ninjakd2,  rdaction, ninjakd2_state, init_ninjakd2, ROT0,   "UPL (United Amusements license)", "JT 104 / NinjaKun Ashura no Shou",                         MACHINE_SUPPORTS_SAVE )

GAME( 1987, mnight,    0,        mnight,    mnight,   mnight_state,   init_mnight,   ROT0,   "UPL",                             "Mutant Night",                                             MACHINE_SUPPORTS_SAVE )
GAME( 1987, mnightj,   mnight,   mnight,    mnight,   mnight_state,   init_mnight,   ROT0,   "UPL (Kawakus license)",           "Mutant Night (Japan)",                                     MACHINE_SUPPORTS_SAVE )

GAME( 1988, arkarea,   0,        arkarea,   arkarea,  mnight_state,   init_mnight,   ROT0,   "UPL",                             "Ark Area",                                                 MACHINE_SUPPORTS_SAVE )

GAME( 1988, robokid,   0,        robokid,   robokid,  robokid_state,  init_robokid,  ROT0,   "UPL",                             "Atomic Robo-kid (World, Type-2)",                          MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // 3-digit highscore names
GAME( 1988, robokidj,  robokid,  robokid,   robokidj, robokid_state,  init_robokidj, ROT0,   "UPL",                             "Atomic Robo-kid (Japan, Type-2, set 1)",                   MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, robokidj2, robokid,  robokid,   robokidj, robokid_state,  init_robokidj, ROT0,   "UPL",                             "Atomic Robo-kid (Japan, Type-2, set 2)",                   MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, robokidj3, robokid,  robokid,   robokidj, robokid_state,  empty_init,    ROT0,   "UPL",                             "Atomic Robo-kid (Japan)",                                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )

GAME( 1989, omegaf,    0,        omegaf,    omegaf,   omegaf_state,   empty_init,    ROT270, "UPL",                             "Omega Fighter",                                            MACHINE_SUPPORTS_SAVE )
GAME( 1989, omegafs,   omegaf,   omegaf,    omegaf,   omegaf_state,   empty_init,    ROT270, "UPL",                             "Omega Fighter Special",                                    MACHINE_SUPPORTS_SAVE )
