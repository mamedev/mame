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
Mutant Night      UPL-?????
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
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "machine/mc8123.h"
#include "includes/ninjakd2.h"

#define MAIN_CLOCK_12 XTAL_12MHz
#define MAIN_CLOCK_5  XTAL_5MHz


/*************************************
 *
 *  Ninja Kid II PCM
 *
 *************************************/

// PCM playback is controlled by a 555 timer
#define NE555_FREQUENCY 16300   // measured on PCB
//#define NE555_FREQUENCY   (1.0f / (0.693 * (560 + 2*51) * 0.1e-6))    // theoretical: this gives 21.8kHz which is too high

SAMPLES_START_CB_MEMBER(ninjakd2_state::ninjakd2_init_samples)
{
	const UINT8* const rom = memregion("pcm")->base();
	const int length = memregion("pcm")->bytes();
	INT16* sampledata = auto_alloc_array(machine(), INT16, length);

	// convert unsigned 8-bit PCM to signed 16-bit
	for (int i = 0; i < length; ++i)
		sampledata[i] = rom[i] << 7;

	m_sampledata = sampledata;
}

WRITE8_MEMBER(ninjakd2_state::ninjakd2_pcm_play_w)
{
	const UINT8* const rom = memregion("pcm")->base();

	// only Ninja Kid II uses this
	if (rom)
	{
		const int length = memregion("pcm")->bytes();
		const int start = data << 8;

		// find end of sample
		int end = start;
		while (end < length && rom[end] != 0x00)
			++end;

		if (end - start)
			m_pcm->start_raw(0, &m_sampledata[start], end - start, NE555_FREQUENCY);
		else
			m_pcm->stop(0);
	}
}



/*************************************
 *
 *  Omega Fighter I/O protection simulation
 *
 *************************************/

void ninjakd2_state::omegaf_io_protection_start()
{
	// register for save states
	save_item(NAME(m_omegaf_io_protection));
	save_item(NAME(m_omegaf_io_protection_input));
	save_item(NAME(m_omegaf_io_protection_tic));
}

void ninjakd2_state::omegaf_io_protection_reset()
{
	// make sure protection starts in a known state
	m_omegaf_io_protection[0] = 0;
	m_omegaf_io_protection[1] = 0;
	m_omegaf_io_protection[2] = 0;
	m_omegaf_io_protection_input = 0;
	m_omegaf_io_protection_tic = 0;
}

READ8_MEMBER(ninjakd2_state::omegaf_io_protection_r)
{
	UINT8 result = 0xff;

	switch (m_omegaf_io_protection[1] & 3)
	{
		case 0:
			switch (offset)
			{
				case 1:
					switch (m_omegaf_io_protection[0] & 0xe0)
					{
						case 0x00:
							if (++m_omegaf_io_protection_tic & 1)
							{
								result = 0x00;
							}
							else
							{
								switch (m_omegaf_io_protection_input)
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
							result = 0x20 | (m_omegaf_io_protection_input & 0x1f);
							break;

						case 0xc0:
							result = 0x60 | (m_omegaf_io_protection_input & 0x1f);
							break;
					}
					break;
			}
			break;

		case 1: // dip switches
			switch (offset)
			{
				case 0: result = ioport("DIPSW1")->read(); break;
				case 1: result = ioport("DIPSW2")->read(); break;
				case 2: result = 0x02; break;
			}
			break;

		case 2: // player inputs
			switch (offset)
			{
				case 0: result = ioport("PAD1")->read(); break;
				case 1: result = ioport("PAD2")->read(); break;
				case 2: result = 0x01; break;
			}
			break;
	}

	return result;
}

WRITE8_MEMBER(ninjakd2_state::omegaf_io_protection_w)
{
	// load parameter on c006 bit 0 rise transition
	if (offset == 2 && (data & 1) && !(m_omegaf_io_protection[2] & 1))
	{
		logerror("loading protection input %02x\n", m_omegaf_io_protection[0]);
		m_omegaf_io_protection_input = m_omegaf_io_protection[0];
	}

	m_omegaf_io_protection[offset] = data;
}



/*****************************************************************************/

WRITE8_MEMBER(ninjakd2_state::ninjakd2_bankselect_w)
{
	membank("bank1")->set_entry(data & m_rom_bank_mask);
}

WRITE8_MEMBER(ninjakd2_state::ninjakd2_soundreset_w)
{
	// bit 4 resets sound CPU
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7 flips screen
	flip_screen_set(data & 0x80);

	// other bits unused
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( ninjakd2_main_cpu, AS_PROGRAM, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("KEYCOIN")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("PAD1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("PAD2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DIPSW1")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DIPSW2")
	AM_RANGE(0xc200, 0xc200) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc201, 0xc201) AM_WRITE(ninjakd2_soundreset_w)
	AM_RANGE(0xc202, 0xc202) AM_WRITE(ninjakd2_bankselect_w)
	AM_RANGE(0xc203, 0xc203) AM_WRITE(ninjakd2_sprite_overdraw_w)
	AM_RANGE(0xc208, 0xc20c) AM_WRITE(ninjakd2_bg_ctrl_w)
	AM_RANGE(0xc800, 0xcdff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(ninjakd2_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(ninjakd2_bgvideoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mnight_main_cpu, AS_PROGRAM, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xd9ff) AM_RAM
	AM_RANGE(0xda00, 0xdfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(ninjakd2_bgvideoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(ninjakd2_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xf000, 0xf5ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("KEYCOIN")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("PAD1")
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("PAD2")
	AM_RANGE(0xf803, 0xf803) AM_READ_PORT("DIPSW1")
	AM_RANGE(0xf804, 0xf804) AM_READ_PORT("DIPSW2")
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xfa01, 0xfa01) AM_WRITE(ninjakd2_soundreset_w)
	AM_RANGE(0xfa02, 0xfa02) AM_WRITE(ninjakd2_bankselect_w)
	AM_RANGE(0xfa03, 0xfa03) AM_WRITE(ninjakd2_sprite_overdraw_w)
	AM_RANGE(0xfa08, 0xfa0c) AM_WRITE(ninjakd2_bg_ctrl_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( robokid_main_cpu, AS_PROGRAM, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(ninjakd2_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd000, 0xd3ff) AM_READWRITE(robokid_bg2_videoram_r, robokid_bg2_videoram_w)   // banked
	AM_RANGE(0xd400, 0xd7ff) AM_READWRITE(robokid_bg1_videoram_r, robokid_bg1_videoram_w)   // banked
	AM_RANGE(0xd800, 0xdbff) AM_READWRITE(robokid_bg0_videoram_r, robokid_bg0_videoram_w)   // banked
	AM_RANGE(0xdc00, 0xdc00) AM_READ_PORT("KEYCOIN") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xdc01, 0xdc01) AM_READ_PORT("PAD1") AM_WRITE(ninjakd2_soundreset_w)
	AM_RANGE(0xdc02, 0xdc02) AM_READ_PORT("PAD2") AM_WRITE(ninjakd2_bankselect_w)
	AM_RANGE(0xdc03, 0xdc03) AM_READ_PORT("DIPSW1") AM_WRITE(ninjakd2_sprite_overdraw_w)
	AM_RANGE(0xdc04, 0xdc04) AM_READ_PORT("DIPSW2")
	AM_RANGE(0xdd00, 0xdd04) AM_WRITE(robokid_bg0_ctrl_w)
	AM_RANGE(0xdd05, 0xdd05) AM_WRITE(robokid_bg0_bank_w)
	AM_RANGE(0xde00, 0xde04) AM_WRITE(robokid_bg1_ctrl_w)
	AM_RANGE(0xde05, 0xde05) AM_WRITE(robokid_bg1_bank_w)
	AM_RANGE(0xdf00, 0xdf04) AM_WRITE(robokid_bg2_ctrl_w)
	AM_RANGE(0xdf05, 0xdf05) AM_WRITE(robokid_bg2_bank_w)
	AM_RANGE(0xe000, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( omegaf_main_cpu, AS_PROGRAM, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("KEYCOIN") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc001, 0xc003) AM_READ(omegaf_io_protection_r)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(ninjakd2_soundreset_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(ninjakd2_bankselect_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(ninjakd2_sprite_overdraw_w)
	AM_RANGE(0xc004, 0xc006) AM_WRITE(omegaf_io_protection_w)
	AM_RANGE(0xc100, 0xc104) AM_WRITE(robokid_bg0_ctrl_w)
	AM_RANGE(0xc105, 0xc105) AM_WRITE(robokid_bg0_bank_w)
	AM_RANGE(0xc1e7, 0xc1e7) AM_READNOP // see notes
	AM_RANGE(0xc200, 0xc204) AM_WRITE(robokid_bg1_ctrl_w)
	AM_RANGE(0xc205, 0xc205) AM_WRITE(robokid_bg1_bank_w)
	AM_RANGE(0xc300, 0xc304) AM_WRITE(robokid_bg2_ctrl_w)
	AM_RANGE(0xc305, 0xc305) AM_WRITE(robokid_bg2_bank_w)
	AM_RANGE(0xc400, 0xc7ff) AM_READWRITE(robokid_bg0_videoram_r, robokid_bg0_videoram_w)   // banked
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(robokid_bg1_videoram_r, robokid_bg1_videoram_w)   // banked
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(robokid_bg2_videoram_r, robokid_bg2_videoram_w)   // banked
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(ninjakd2_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe000, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( ninjakd2_sound_cpu, AS_PROGRAM, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(ninjakd2_pcm_play_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, ninjakd2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_REGION("soundcpu", 0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjakd2_sound_io, AS_IO, 8, ninjakd2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("2203.1", ym2203_device, write)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("2203.2", ym2203_device, write)
ADDRESS_MAP_END



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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    /* keep pressed during boot to enter service mode */
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    /* keep pressed during boot to enter service mode */
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

static const gfx_layout layout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(32*8,4) },
	{ STEP8(0,32), STEP8(64*8,32) },
	128*8
};

static const gfx_layout robokid_layout16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(64*8,4) },
	{ STEP16(0,32) },
	128*8
};

static GFXDECODE_START( ninjakd2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8,    0x200, 16)    // fg
	GFXDECODE_ENTRY( "gfx2", 0, layout16x16,  0x100, 16)    // sprites
	GFXDECODE_ENTRY( "gfx3", 0, layout16x16,  0x000, 16)    // bg
GFXDECODE_END

static GFXDECODE_START( robokid )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8,           0x300, 16) // fg
	GFXDECODE_ENTRY( "gfx2", 0, robokid_layout16x16, 0x200, 16) // sprites
	GFXDECODE_ENTRY( "gfx3", 0, robokid_layout16x16, 0x000, 16) // bg0
	GFXDECODE_ENTRY( "gfx4", 0, robokid_layout16x16, 0x000, 16) // bg1
	GFXDECODE_ENTRY( "gfx5", 0, robokid_layout16x16, 0x000, 16) // bg2
GFXDECODE_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

WRITE_LINE_MEMBER(ninjakd2_state::irqhandler)
{
	m_soundcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(ninjakd2_state::ninjakd2_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xd7); /* RST 10h */
}

void ninjakd2_state::machine_start()
{
}

void ninjakd2_state::machine_reset()
{
	/* initialize main Z80 bank */
	int num_banks = (memregion("maincpu")->bytes() - 0x10000) / 0x4000;
	membank("bank1")->configure_entries(0, num_banks, memregion("maincpu")->base() + 0x10000, 0x4000);
	membank("bank1")->set_entry(0);

	m_rom_bank_mask = num_banks - 1;
}

MACHINE_START_MEMBER(ninjakd2_state,omegaf)
{
	omegaf_io_protection_start();

	machine_start();
}

MACHINE_RESET_MEMBER(ninjakd2_state,omegaf)
{
	omegaf_io_protection_reset();

	machine_reset();
}

/*****************************************************************************/

static MACHINE_CONFIG_START( ninjakd2_core, ninjakd2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK_12/2)       /* verified */
	MCFG_CPU_PROGRAM_MAP(ninjakd2_main_cpu)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ninjakd2_state,  ninjakd2_interrupt)

	MCFG_CPU_ADD("soundcpu", Z80, MAIN_CLOCK_5)     /* verified */
	MCFG_CPU_PROGRAM_MAP(ninjakd2_sound_cpu)
	MCFG_CPU_IO_MAP(ninjakd2_sound_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.61)    /* verified on pcb */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ninjakd2_state, screen_update_ninjakd2)
	MCFG_SCREEN_VBLANK_DRIVER(ninjakd2_state, screen_eof_ninjakd2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ninjakd2)
	MCFG_PALETTE_ADD("palette", 0x300)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("2203.1", YM2203, MAIN_CLOCK_12/8)       /* verified */
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(ninjakd2_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("2203.2", YM2203, MAIN_CLOCK_12/8)       /* verified */
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("pcm", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(ninjakd2_state, ninjakd2_init_samples)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ninjakd2, ninjakd2_core )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mnight, ninjakd2_core )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mnight_main_cpu)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(ninjakd2_state,mnight)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("pcm")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( arkarea, ninjakd2_core )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mnight_main_cpu)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(ninjakd2_state,arkarea)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("pcm")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( robokid, mnight )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(robokid_main_cpu)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", robokid)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(0x400)  // RAM is this large, but still only 0x300 colors used
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(ninjakd2_state,robokid)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(ninjakd2_state, screen_update_robokid)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( omegaf, robokid )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(omegaf_main_cpu)

	MCFG_MACHINE_START_OVERRIDE(ninjakd2_state,omegaf)
	MCFG_MACHINE_RESET_OVERRIDE(ninjakd2_state,omegaf)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(ninjakd2_state,omegaf)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(ninjakd2_state, screen_update_omegaf)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ninjakd2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "nk2_01.rom",   0x00000, 0x8000, CRC(3cdbb906) SHA1(f48f82528b5fc581ee3b1ccd0ef9cdecc7249bb3) )
	ROM_LOAD( "nk2_02.rom",   0x10000, 0x8000, CRC(b5ce9a1a) SHA1(295a7e1d41e1a8ee45f1250086a0c9314837eded) )   // banked at 8000-bfff
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.rom",   0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.rom",   0x00000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) )  // encrypted

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "ninjakd2.key",  0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "nk2_07.rom",   0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "nk2_10.rom",   0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // unsigned 8-bit pcm samples
ROM_END

ROM_START( ninjakd2a )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "nk2_01.bin",   0x00000, 0x8000, CRC(e6adca65) SHA1(33d483dde0853f37455cde32b461f4e919601b4b) )
	ROM_LOAD( "nk2_02.bin",   0x10000, 0x8000, CRC(d9284bd1) SHA1(e790fb1a718a1f7997931f2f390fe053655f231d) )   // banked at 8000-bfff
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.bin",   0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) )

	ROM_REGION( 2*0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.bin",   0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) )   // decrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 )                                                                 // decrypted data

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "nk2_07.rom",   0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "nk2_10.rom",   0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // unsigned 8-bit pcm samples
ROM_END

ROM_START( ninjakd2b )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3s",         0x00000, 0x8000, CRC(cb4f4624) SHA1(4fc66641adc0a2c0eca332f27c5777df62fa507b) )
	ROM_LOAD( "2.3q",         0x10000, 0x8000, CRC(0ad0c100) SHA1(c5bbc107ba07bd6950bb4d7377e827c084b8229b) )   // banked at 8000-bfff
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )   // 3.3p
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )   // 4.3n
	ROM_LOAD( "nk2_05.rom",   0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )   // 5.3l

	ROM_REGION( 2*0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.bin",   0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) )   // 6.3g  decrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 )                                                                 // decrypted data

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x08000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )  // 12.5m

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )  // 8.6k
	ROM_LOAD( "nk2_07.rom",   0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )  // 7.6m

	ROM_REGION( 0x20000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )  // 11.2m
	ROM_LOAD( "nk2_10.rom",   0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )  // 10.2p

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // 9.6d  unsigned 8-bit pcm samples
ROM_END

ROM_START( rdaction )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3u",         0x00000, 0x8000, CRC(5c475611) SHA1(2da88a95b5d68b259c8ae48af1438a82a1d601c1) )
	ROM_LOAD( "2.3s",         0x10000, 0x8000, CRC(a1e23bd2) SHA1(c3b6574dc9fa66b4f41c37754a0d20a865f8bc28) )   // banked at 8000-bfff
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )   // 3.3r
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )   // 4.3p
	ROM_LOAD( "nk2_05.bin",   0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) )   // 5.3m

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.rom",   0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) )   // 6.3h  encrypted

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "ninjakd2.key",  0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "12.5n",        0x00000, 0x08000, CRC(0936b365) SHA1(3705f42b76ab474357e77c1a9b8e3755c7ab2c0c) )  // 12.5n

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )  // 8.6l
	ROM_LOAD( "nk2_07.rom",   0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )  // 7.6n

	ROM_REGION( 0x20000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )  // 11.2n
	ROM_LOAD( "nk2_10.rom",   0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )  // 10.2r

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // 9.6c  unsigned 8-bit pcm samples
ROM_END

// Is this official? works with any program rom set, but nowhere in any program roms is the title JT-104 mentioned.
// It was found on a board with RAD ACTION program roms.
//
// In addition to the title screen text change the new graphic ROM has 'UNITED AMUSEMENTS' license text replacing
// the regular UPL copyright (used on the interludes) however because it is using the RAD ACTION program roms the
// title screen still shows the 'World Games' license text.
//
// Did this board have incorrect program roms, or is this just how it was?


ROM_START( jt104 ) // identical to radaction set with different gfx rom and decrypted sound rom
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.3u",         0x00000, 0x8000, CRC(5c475611) SHA1(2da88a95b5d68b259c8ae48af1438a82a1d601c1) )
	ROM_LOAD( "2.3s",         0x10000, 0x8000, CRC(a1e23bd2) SHA1(c3b6574dc9fa66b4f41c37754a0d20a865f8bc28) )   // banked at 8000-bfff
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )   // 3.3r
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )   // 4.3p
	ROM_LOAD( "nk2_05.bin",   0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) )   // 5.3m

	ROM_REGION( 2*0x10000, "soundcpu", 0 )
	ROM_LOAD( "nk2_06.bin",   0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) )   // decrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 )                                                                 // decrypted data

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "ninjakd2.key",  0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "jt_104_12.bin",        0x00000, 0x08000, CRC(c038fadb) SHA1(59e9b125ead3e9bdc9d66de75dffd58956eb922e) )  // this rom contains the new title / license

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x10000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )
	ROM_LOAD( "nk2_07.rom",   0x10000, 0x10000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )

	ROM_REGION( 0x20000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x10000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )
	ROM_LOAD( "nk2_10.rom",   0x10000, 0x10000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )

	ROM_REGION( 0x10000, "pcm", 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )   // unsigned 8-bit pcm samples
ROM_END


ROM_START( mnight )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "mn6-j19.bin",  0x00000, 0x8000, CRC(56678d14) SHA1(acf3a97ca29db8ab9cad69599c5567464af3ae44) )
	ROM_LOAD( "mn5-j17.bin",  0x10000, 0x8000, CRC(2a73f88e) SHA1(0a7b769174f2b976650453d808cb23668dff0260) )   // banked at 8000-bfff
	ROM_LOAD( "mn4-j16.bin",  0x18000, 0x8000, CRC(c5e42bb4) SHA1(1956e737ae393e987cb7e8eae520518f1e0f597f) )
	ROM_LOAD( "mn3-j14.bin",  0x20000, 0x8000, CRC(df6a4f7a) SHA1(ce3cb84b514220d686b12c03567289fd23da0fe1) )
	ROM_LOAD( "mn2-j12.bin",  0x28000, 0x8000, CRC(9c391d1b) SHA1(06e8c202337e3eba38c479e8b7e29a3f8ffc4ed1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mn1-j7.bin",   0x00000, 0x10000, CRC(a0782a31) SHA1(8abd2f0b0c2c2eb876f324f7a095a5cdc773c187) )

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "mn10-b10.bin", 0x00000, 0x08000, CRC(37b8221f) SHA1(ac86e0ae8039fd30a028a893d08ce099f7765615) )

	ROM_REGION( 0x30000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "mn7-e11.bin",  0x00000, 0x10000, CRC(4883059c) SHA1(53d4b9b0f0725c25e302ee1549a306778ec74d85) )
	ROM_LOAD( "mn8-e12.bin",  0x10000, 0x10000, CRC(02b91445) SHA1(f0cf85f9e17c40248de16bca8df6d745e359b92d) )
	ROM_LOAD( "mn9-e14.bin",  0x20000, 0x10000, CRC(9f08d160) SHA1(1a0041ad138e7e6598d4d03d7cbd52a7244557ac) )

	ROM_REGION( 0x30000, "gfx3", 0 )    // bg tiles (need lineswapping)
	ROM_LOAD( "mn11-b20.bin", 0x00000, 0x10000, CRC(4d37e0f4) SHA1(a6d9aaccd97769197622cda45474e223c2ee1d98) )
	ROM_LOAD( "mn12-b22.bin", 0x10000, 0x10000, CRC(b22cbbd3) SHA1(70984f1051fd236730d97011bc87dacb3ca38594) )
	ROM_LOAD( "mn13-b23.bin", 0x20000, 0x10000, CRC(65714070) SHA1(48f3c130c97d00e8f0535904dc2237277067c475) )
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles (need lineswapping)
	ROM_LOAD( "arkarea.004",  0x00000, 0x08000, CRC(69e36af2) SHA1(2bccef8f396dcb5261af0140af04c95ee8ecae11) )

	ROM_REGION( 0x30000, "gfx2", 0 )    // sprites (need lineswapping)
	ROM_LOAD( "arkarea.007",  0x00000, 0x10000, CRC(d5684a27) SHA1(4961e8a5df2510afb1ef3e937d0a5d52e91893a3) )
	ROM_LOAD( "arkarea.006",  0x10000, 0x10000, CRC(2c0567d6) SHA1(f36a2a3ff487660f89470516617482331f008da0) )
	ROM_LOAD( "arkarea.005",  0x20000, 0x10000, CRC(9886004d) SHA1(4050756af5c00ab1a368780fe091460fd9e2cb05) )

	ROM_REGION( 0x30000, "gfx3", 0 )    // bg tiles (need lineswapping)
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "gfx2", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "gfx5", 0 )    // bg2 tiles
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "gfx2", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "gfx5", 0 )    // bg2 tiles
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "robokid.b9",   0x00000, 0x08000, CRC(fac59c3f) SHA1(1b202ad5c12982512129d9e097267dd31b984ae8) )

	ROM_REGION( 0x40000, "gfx2", 0 )    // sprite tiles
	ROM_LOAD( "robokid.15f",  0x00000, 0x10000, CRC(ba61f5ab) SHA1(8433ddd55f0184cd5e8bb4a94a1c2336b2f8ff05) )
	ROM_LOAD( "robokid.16f",  0x10000, 0x10000, CRC(d9b399ce) SHA1(70755c9cae27187f183ae6d61bedb95c420756f4) )
	ROM_LOAD( "robokid.17f",  0x20000, 0x10000, CRC(afe432b9) SHA1(1ec7954ccf112eddf0ffcb8b5aec6cbc5cba7a7a) )
	ROM_LOAD( "robokid.18f",  0x30000, 0x10000, CRC(a0aa2a84) SHA1(4d46c169429cd285644336c7d47e393b33bd8770) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // bg0 tiles
	ROM_LOAD( "robokid.19c",  0x00000, 0x10000, CRC(02220421) SHA1(f533e9c6cea1dccbb60e0528c470f3cb5e8fc44e) )
	ROM_LOAD( "robokid.20c",  0x10000, 0x10000, CRC(02d59bc2) SHA1(031acbb14145f9f4623de8868c6207fb9f8e8207) )
	ROM_LOAD( "robokid.17d",  0x20000, 0x10000, CRC(2fa29b99) SHA1(13dce7932e2e9c03a139a4293584838aa3d9f1c3) )
	ROM_LOAD( "robokid.18d",  0x30000, 0x10000, CRC(ae15ce02) SHA1(175e4eebdf12f1f373e01a4b1c933053ddd09abf) )
	ROM_LOAD( "robokid.19d",  0x40000, 0x10000, CRC(784b089e) SHA1(1ae3346b4afa3da9484ebc59c8a530cb95f7d277) )
	ROM_LOAD( "robokid.20d",  0x50000, 0x10000, CRC(b0b395ed) SHA1(31ec07634053793a701bbfd601b029f7da66e9d7) )
	ROM_LOAD( "robokid.19f",  0x60000, 0x10000, CRC(0f9071c6) SHA1(8bf0c35189eda98a9bc150788890e136870cb5b2) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // bg1 tiles
	ROM_LOAD( "robokid.12c",  0x00000, 0x10000, CRC(0ab45f94) SHA1(d8274263068d998c89a1b247dde7f814037cc15b) )
	ROM_LOAD( "robokid.14c",  0x10000, 0x10000, CRC(029bbd4a) SHA1(8e078cdafe608fc6cde827be85c5267ade4ecca6) )
	ROM_LOAD( "robokid.15c",  0x20000, 0x10000, CRC(7de67ebb) SHA1(2fe92e50e2894dd363e69b053db96bdb66a273eb) )
	ROM_LOAD( "robokid.16c",  0x30000, 0x10000, CRC(53c0e582) SHA1(763e6127532d022a707bf9ddf1a832413745f248) )
	ROM_LOAD( "robokid.17c",  0x40000, 0x10000, CRC(0cae5a1e) SHA1(a183a33516c81ea2c029b72ee6261c4519e095ab) )
	ROM_LOAD( "robokid.18c",  0x50000, 0x10000, CRC(56ac7c8a) SHA1(66ed5646a2e8563caeb4ff96fa7d34fde27e9899) )
	ROM_LOAD( "robokid.15d",  0x60000, 0x10000, CRC(cd632a4d) SHA1(a537d9ced45fdac490097e9162ac4d09a470be79) )
	ROM_LOAD( "robokid.16d",  0x70000, 0x10000, CRC(18d92b2b) SHA1(e6d20ea8f0fac8bd4824a3b279a0fd8a1d6c26f5) )

	ROM_REGION( 0x80000, "gfx5", 0 )    // bg2 tiles
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprite tiles
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // bg0 tiles
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // bg1 tiles
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, "gfx5", 0 )    // bg2 tiles
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

	ROM_REGION( 0x08000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "4.18h",        0x00000, 0x08000, CRC(9e2d8152) SHA1(4b50557d171d1b03a870db5891ae67d70858ad37) )

	ROM_REGION( 0x20000, "gfx2", 0 )    // sprite tiles
	ROM_LOAD( "8.23m",        0x00000, 0x20000, CRC(0bd2a5d1) SHA1(ef84f1a5554e891fc38d17314e3952ea5c9d2731) )

	ROM_REGION( 0x80000, "gfx3", 0 )    // bg0 tiles
	ROM_LOAD( "2back1.27b",   0x00000, 0x80000, CRC(21f8a32e) SHA1(26582e06e7381e09443fa99f24ca9edd0b4a2937) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // bg1 tiles
	ROM_LOAD( "1back2.15b",   0x00000, 0x80000, CRC(6210ddcc) SHA1(89c091eeafcc92750d0ea303fcde8a8dc3eeba89) )

	ROM_REGION( 0x80000, "gfx5", 0 )    // bg2 tiles
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
	UINT8* const src = memregion(region)->base();
	dynamic_buffer temp(length);
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
	lineswap_gfx_roms("gfx1", 13);     // fg tiles
	lineswap_gfx_roms("gfx2", 14);     // sprites
	lineswap_gfx_roms("gfx3", 14);     // bg tiles
}


DRIVER_INIT_MEMBER(ninjakd2_state,ninjakd2)
{
	mc8123_decode(memregion("soundcpu")->base(), m_decrypted_opcodes, memregion("user1")->base(), 0x8000);

	gfx_unscramble();
}

DRIVER_INIT_MEMBER(ninjakd2_state,bootleg)
{
	memcpy(m_decrypted_opcodes, memregion("soundcpu")->base() + 0x10000, 0x8000);

	gfx_unscramble();
}

DRIVER_INIT_MEMBER(ninjakd2_state,mnight)
{
	gfx_unscramble();
}

/*****************************************************************************/

READ8_MEMBER(ninjakd2_state::robokid_motion_error_verbose_r)
{
	popmessage("%s MOTION ERROR, contact MAMEdev", machine().system().name);
	logerror("maincpu %04x MOTION ERROR\n", space.device().safe_pc());
	return 0xe6;
}

void ninjakd2_state::robokid_motion_error_kludge(UINT16 offset)
{
	// patch out rare "5268 MOTION ERROR" (MT 05024)
	// It looks like it's due to a buggy random number generator,
	// then it possibly happens on the real arcade cabinet too.
	// I doubt it is protection related, but you can never be sure.
	UINT8 *ROM = memregion("maincpu")->base() + offset;
	ROM[0] = 0xe6;
	ROM[1] = 0x03; // and 3
	ROM[2] = 0x18;
	ROM[3] = 0xf6; // jr $-8

	m_maincpu->space(AS_PROGRAM).install_read_handler(offset, offset, read8_delegate(FUNC(ninjakd2_state::robokid_motion_error_verbose_r), this));
}

DRIVER_INIT_MEMBER(ninjakd2_state,robokid)
{
	robokid_motion_error_kludge(0x5247);
}

DRIVER_INIT_MEMBER(ninjakd2_state,robokidj)
{
	robokid_motion_error_kludge(0x5266);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    INIT,                     MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1987, ninjakd2,  0,        ninjakd2, ninjakd2, ninjakd2_state, ninjakd2, ROT0,   "UPL", "Ninja-Kid II / NinjaKun Ashura no Shou (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ninjakd2a, ninjakd2, ninjakd2, ninjakd2, ninjakd2_state, bootleg,  ROT0,   "UPL", "Ninja-Kid II / NinjaKun Ashura no Shou (set 2, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ninjakd2b, ninjakd2, ninjakd2, rdaction, ninjakd2_state, bootleg,  ROT0,   "UPL", "Ninja-Kid II / NinjaKun Ashura no Shou (set 3, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, rdaction,  ninjakd2, ninjakd2, rdaction, ninjakd2_state, ninjakd2, ROT0,   "UPL (World Games license)",       "Rad Action / NinjaKun Ashura no Shou", MACHINE_SUPPORTS_SAVE )
GAME( 1987, jt104,     ninjakd2, ninjakd2, rdaction, ninjakd2_state, bootleg,  ROT0,   "UPL (United Amusements license)", "JT-104 (title screen modification of Rad Action)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, mnight,    0,        mnight,   mnight,   ninjakd2_state, mnight,   ROT0,   "UPL (Kawakus license)", "Mutant Night", MACHINE_SUPPORTS_SAVE )
GAME( 1988, arkarea,   0,        arkarea,  arkarea,  ninjakd2_state, mnight,   ROT0,   "UPL", "Ark Area", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robokid,   0,        robokid,  robokid,  ninjakd2_state, robokid,  ROT0,   "UPL", "Atomic Robo-kid", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robokidj,  robokid,  robokid,  robokidj, ninjakd2_state, robokidj, ROT0,   "UPL", "Atomic Robo-kid (Japan, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robokidj2, robokid,  robokid,  robokidj, ninjakd2_state, robokidj, ROT0,   "UPL", "Atomic Robo-kid (Japan, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, omegaf,    0,        omegaf,   omegaf,   driver_device,  0,        ROT270, "UPL", "Omega Fighter", MACHINE_SUPPORTS_SAVE )
GAME( 1989, omegafs,   omegaf,   omegaf,   omegaf,   driver_device,  0,        ROT270, "UPL", "Omega Fighter Special", MACHINE_SUPPORTS_SAVE )
