// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
/********************************************************************

Task Force Harrier       1989 UPL        68000 Z80           YM2203 2xOKIM6295
Many Block               1991 Bee-Oh     68000 Z80           YM2203 2xOKIM6295
Mustang                  1990 UPL        68000 NMK004        YM2203 2xOKIM6295
Bio-ship Paladin         1990 UPL        68000 NMK004        YM2203 2xOKIM6295
Vandyke                  1990 UPL        68000 NMK004        YM2203 2xOKIM6295
Black Heart              1991 UPL        68000 NMK004        YM2203 2xOKIM6295
Acrobat Mission          1991 UPL        68000 NMK004        YM2203 2xOKIM6295
Strahl                   1992 UPL        68000 NMK004        YM2203 2xOKIM6295
Thunder Dragon           1991 NMK/Tecmo  68000 NMK004        YM2203 2xOKIM6295
Hacha Mecha Fighter      1991 NMK        68000 NMK004        YM2203 2xOKIM6295
Macross                  1992 Banpresto  68000 NMK004        YM2203 2xOKIM6295
GunNail                  1993 NMK/Tecmo  68000 NMK004        YM2203 2xOKIM6295
Macross II               1993 Banpresto  68000 Z80           YM2203 2xOKIM6295
Thunder Dragon 2         1993 NMK        68000 Z80           YM2203 2xOKIM6295
Arcadia / Rapid Hero     1994 NMK        68000 tmp90c841     YM2203 2xOKIM6295

S.S. Mission             1992 Comad      68000 Z80           OKIM6295 (hack of Thunder Dragon)
Air Attack               1996 Comad      68000 Z80           OKIM6295 (hack of Thunder Dragon)

Mustang (bootleg)                        68000 Z80           YM3812 OKIM6295
Thunder Dragon (bootleg)                 68000 Z80           YM3812 OKIM6295

Saboten Bombers          1992 NMK/Tecmo  68000               2xOKIM6295
Bombjack Twin            1993 NMK        68000               2xOKIM6295
Nouryoku Koujou Iinkai   1995 Tecmo      68000               2xOKIM6295

driver by Mirko Buffoni, Richard Bush, Nicola Salmoria, Bryan McPhail,
          David Haywood, R. Belmont, Alex Marshal and Luca Elia.

Afega based their hardware on the NMK hardware, not surprising considering Twin
Action is simply a hack of USSAF Mustang.

The NMK004 CPU is a Toshiba TMP90C840 with internal ROM.
The dumped internal ROM has a date string of 900315 in ROM and a version number of V-00

The later games (from GunNail onwards) have an higher resolution (384x224 instead
of 256x224) but the hardware is pretty much the same. It's obvious that the higher
res is an afterthought, because the tilemap layout is weird (the left 8 screen
columns have to be taken from the rightmost 8 columns of the tilemap), and the
games rely on mirror addresses to access the tilemap sequentially.

TODO:
- tharrier performs a handshake operation which is the same as that used by the
  other games to initialize the NMK004 at boot, however it doesn't have an NMK004
  (it uses a Z80 based sound system and also predates the NMK004)
  maybe it has a pre-NMK004 chip using the same communication protocol but used
  for protection instead.

- Protection is patched in several games.

- Hacha Mecha Fighter: mcu simulation is wrong/incorrect (see notes).

- Music timing in nouryoku is a little off.
- In Bioship, there's an occasional flicker of one of the sprites composing big
  ships. Increasing CPU speed from 12 to 16 MHz improved it, but it's still not
  100% fixed. (the CPU speed has been verified to be 10Mhz??)

- (PCB owners): Measure pixel clock / vblank duration for all of these games.

- for the Afega games (Guardian Storm especially) the lives display has bad colours,
  it doesn't matter if this is drawn with the TX layer (some sets) or the sprites (others)
  so it's probably something else funky with the memory access.

NOT BUGS:
- Hacha Mecha Fighter: (BTANB) the bomb graphics are pretty weird when the game
  is in japanese mode, but it's like this on the original game.

- Vandyke: Many enemies make very strange sounds because they seem to have no
  rate limit for making their sound effect. This is normal, it's like this on all
  PCB recordings.


----

tdragon and hachamf test mode:

1)  Press player 2 buttons 1+2 during reset.  "Ready?" will appear
2)  Press player 1 button 2 14 (!) times

mustang test mode:

1)  Press player 2 buttons 1+2 during reset.  "Ready?" will appear
2)  Press player 1 button 1 14 (!) times

gunnail test mode:

1)  Press player 2 buttons 1+2 during reset.  "Ready?" will appear
2)  Press player 2 button 1 3 times

bjtwin test mode:

1)  Press player 2 buttons 1+2 during reset.  "Ready?" will appear
2)  Press player 1 buttons in this sequence:
    2,2,2, 1,1,1, 2,2,2, 1,1,1
    The release date of this program will appear.

Some code has to be patched out for this to work (see below). The program
remaps button 2 and 3 to button 1, so you can't enter the above sequence.

---

Questions / Notes

'manybloc' :

  - There are writes to 0x080010.w and 0x080012.w (MCU ?) in code between
    0x005000 to 0x005690, but I see no call to "main" routine at 0x005504 !
  - There are writes to 0x08001c.w and 0x08001e.w but I can't tell what
    the effect is ! Could it be related to sound and/or interrupts ?

  - In the "test mode", press BOTH player 1 buttons to exit

  - When help is available, press BUTTON2 twice within the timer to "solve"

---

Sound notes for games with a Z80:

mustangb and tdragonb use the Seibu Raiden sound hardware and a modified
Z80 program (but the music is intact and recognizable).  See audio/seibu.c
for more info on this.

---

Afega Games

95 Twin Action                  this is a hack of Mustang with new graphics
97 Red Hawk                     US Version of Stagger 1
98 Sen Jin - Guardian Storm
98 Stagger I

98 Bubble 2000                  By Tuning, but it seems to be the same HW
  / Hot Bubble

00 Spectrum 2000                By YomaTech -- NOTE sprite bugs happen on real hw!!

01 Fire Hawk                    By ESD with different sound hardware: 2 M6295,
                                this doesn't have the glitches present in spec2k

Afega stands for "Art-Fiction Electronic Game"


********************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/tlcs90/tlcs90.h"
#include "cpu/m68000/m68000.h"
#include "audio/seibu.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "machine/nmk004.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "includes/nmk16.h"


WRITE16_MEMBER(nmk16_state::nmk16_x0016_w)
{
	// this is part of a watchdog scheme
	// generating an NMI on the NMK004 keeps a timer alive
	// if that timer expires the NMK004 resets the system
	m_nmk004->m_cpu->set_input_line(INPUT_LINE_NMI, (data & 1) ? ASSERT_LINE : CLEAR_LINE);
}


WRITE16_MEMBER(nmk16_state::nmk16_bioship_x0016_w)
{
	// ugly, ugly logic invert hack, but otherwise bioship doesn't hit the NMI enough to keep the game alive!
	m_nmk004->m_cpu->set_input_line(INPUT_LINE_NMI, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}

/**********************************************************
  Protection: Memory Scrambling

  Several NMK and Afega games (notably the ones running at
  the lower resolution) require strange handling of work
  RAM when performing 8-bit writes.  This handling breaks
  some of the later games if used, but is essential for
  Mustang, Black Heart, Task Force Harrier, and the Afega
  shooters to work correctly without ROM patches.  Tests
  on the board would help verify this as correct.

  I'm not sure if this is actually protection, or just
  poor board design.

**********************************************************/


WRITE16_MEMBER(nmk16_state::nmk16_mainram_strange_w)
{
#if 0
	UINT16* dstram;

	dstram = m_mainram;

	if (!ACCESSING_BITS_8_15)
	{
		dstram[offset] = (data & 0x00ff) |  ((data & 0x00ff)<<8);
	}
	else if (!ACCESSING_BITS_0_7)
	{
		dstram[offset] = (data & 0xff00) |  ((data & 0xff00)>>8);
	}
	else
	{
		dstram[offset] = data;
	}
#endif
	// as of SVN 30715 the 68k core replicates the above behavior, providing mirrored bits in 'data' regardless of the value of 'mem_mask'
	m_mainram[offset] = data;

}


WRITE16_MEMBER(nmk16_state::ssmissin_sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}



WRITE8_MEMBER(nmk16_state::ssmissin_soundbank_w)
{
	membank("okibank1")->set_entry(data & 0x3);
}



WRITE16_MEMBER(nmk16_state::tharrier_mcu_control_w)
{
//  logerror("%04x: mcu_control_w %02x\n",space.device().safe_pc(),data);
}

READ16_MEMBER(nmk16_state::tharrier_mcu_r)
{
	/* The MCU is mapped as the top byte for byte accesses only,
	    all word accesses are to the input port */
	if (ACCESSING_BITS_8_15 && !ACCESSING_BITS_0_7)
	{
		static const UINT8 to_main[] =
		{
			0x82,0xc7,0x00,0x2c,0x6c,0x00,0x9f,0xc7,0x00,0x29,0x69,0x00,0x8b,0xc7,0x00
		};

		int res;

		if (space.device().safe_pc()==0x8aa) res = (m_mainram[0x9064/2])|0x20; /* Task Force Harrier */
		else if (space.device().safe_pc()==0x8ce) res = (m_mainram[0x9064/2])|0x60; /* Task Force Harrier */
		else
		{
			res = to_main[m_prot_count++];
			if (m_prot_count == sizeof(to_main))
				m_prot_count = 0;
		}

		return res << 8;
	}
	else
	{
		// the above statement appears to be incorrect, it should also read DSW1 from here, almost certainly
		// through the MCU.  The weird 0x080202 address where we read IN2 is also probably just a mirror of 0x080002 (here)

		return ~ioport("IN1")->read();
	}
}

WRITE16_MEMBER(nmk16_state::macross2_sound_reset_w)
{
	/* PCB behaviour verified by Corrado Tomaselli at MAME Italia Forum:
	   every time music changes Z80 is reset */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
}

WRITE16_MEMBER(nmk16_state::macross2_sound_command_w)
{
	if (ACCESSING_BITS_0_7)
		soundlatch_byte_w(space,0,data & 0xff);
}

WRITE8_MEMBER(nmk16_state::macross2_sound_bank_w)
{
	membank("bank1")->set_entry(data & 0x07);
}

WRITE8_MEMBER(nmk16_state::tharrier_oki6295_bankswitch_0_w)
{
	data &= 3;
	if (data != 3)
		membank("okibank1")->set_entry(data);
}

WRITE8_MEMBER(nmk16_state::tharrier_oki6295_bankswitch_1_w)
{
	data &= 3;
	if (data != 3)
		membank("okibank2")->set_entry(data);
}

WRITE16_MEMBER(nmk16_state::afega_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data&0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

/***************************************************************************/

static ADDRESS_MAP_START( vandyke_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c007) AM_WRITE(vandyke_scroll_w)
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x094000, 0x097fff) AM_RAM /* what is this? */
	AM_RANGE(0x09d000, 0x09d7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( vandykeb_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
//  AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x080016, 0x080017) AM_WRITENOP    /* IRQ enable? */
	AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x080010, 0x08001d) AM_WRITE(vandykeb_scroll_w) /* 10, 12, 1a, 1c */
//  AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c007) AM_WRITENOP    /* just in case... */
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x094000, 0x097fff) AM_RAM /* what is this? */
	AM_RANGE(0x09d000, 0x09d7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( manybloc_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")
	AM_RANGE(0x080010, 0x080011) AM_WRITENOP            /* See notes at the top of the driver */
	AM_RANGE(0x080012, 0x080013) AM_WRITENOP            /* See notes at the top of the driver */
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x08001c, 0x08001d) AM_WRITENOP            /* See notes at the top of the driver */
	AM_RANGE(0x08001e, 0x08001f) AM_READWRITE(soundlatch2_word_r,soundlatch_word_w)
	AM_RANGE(0x088000, 0x0883ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09cfff) AM_RAM_WRITE(manybloc_scroll_w) AM_SHARE("scrollram")
	AM_RANGE(0x09d000, 0x09d7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tharrier_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ(tharrier_mcu_r) // AM_READ_PORT("IN1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000e, 0x08000f) AM_READ(soundlatch2_word_r)    /* from Z80 */
	AM_RANGE(0x080010, 0x080011) AM_WRITE(tharrier_mcu_control_w)
	AM_RANGE(0x080012, 0x080013) AM_WRITENOP
//  AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
//  AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x08001e, 0x08001f) AM_WRITE(soundlatch_word_w)
	AM_RANGE(0x080202, 0x080203) AM_READ_PORT("IN2")
	AM_RANGE(0x088000, 0x0883ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
//  AM_RANGE(0x08c000, 0x08c007) AM_WRITE(nmk_scroll_w)
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM /* Unused txvideoram area? */
	AM_RANGE(0x09d000, 0x09d7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tharrier_sound_map, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(soundlatch_byte_r,soundlatch2_byte_w)
	AM_RANGE(0xf400, 0xf400) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0xf500, 0xf500) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0xf600, 0xf600) AM_WRITE(tharrier_oki6295_bankswitch_0_w)
	AM_RANGE(0xf700, 0xf700) AM_WRITE(tharrier_oki6295_bankswitch_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tharrier_sound_io_map, AS_IO, 8, nmk16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
ADDRESS_MAP_END

//Read input port 1 030c8/  BAD
//3478  GOOD

static ADDRESS_MAP_START( mustang_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff) AM_WRITENOP
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_x0016_w)    // frame number?
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c001) AM_WRITE(mustang_scroll_w)
	AM_RANGE(0x08c002, 0x08c087) AM_WRITENOP    // ??
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mustangb_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000e, 0x08000f) AM_NOP
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITENOP    // frame number?
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE("seibu_sound", seibu_sound_device, main_mustb_w)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c001) AM_WRITE(mustang_scroll_w)
	AM_RANGE(0x08c002, 0x08c087) AM_WRITENOP    // ??
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( twinactn_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000e, 0x08000f) AM_NOP
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITENOP    // frame number?
	AM_RANGE(0x08001e, 0x08001f) AM_WRITE(afega_soundlatch_w)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c001) AM_WRITE(mustang_scroll_w)
	AM_RANGE(0x08c002, 0x08c087) AM_WRITENOP    // ??
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( acrobatm_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x80000, 0x8ffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0xc0000, 0xc0001) AM_READ_PORT("IN0")
	AM_RANGE(0xc0002, 0xc0003) AM_READ_PORT("IN1")
	AM_RANGE(0xc0008, 0xc0009) AM_READ_PORT("DSW1")
	AM_RANGE(0xc000a, 0xc000b) AM_READ_PORT("DSW2")
	AM_RANGE(0xc000e, 0xc000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0xc0014, 0xc0015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0xc0016, 0xc0017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0xc0018, 0xc0019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0xc001e, 0xc001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0xc4000, 0xc45ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc8000, 0xc8007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0xcc000, 0xcffff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0xd4000, 0xd47ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bioship_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
//  AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_bioship_x0016_w)
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x084000, 0x084001) AM_WRITE(bioship_bank_w)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c007) AM_RAM_WRITE(bioshipbg_scroll_w)
	AM_RANGE(0x08c010, 0x08c017) AM_RAM_WRITE(bioship_scroll_w)
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

/******************************************************************************************

Thunder Dragon & Hacha Mecha Fighter shares some ram with the MCU,the job of the latter
is to provide some jsr vectors used by the game for gameplay calculations.Also it has
the job to give the vectors of where the inputs are to be read & to calculate the coin
settings,the latter is in a TIMER_DEVICE_CALLBACK to avoid sync problems.
To make a long story short,this MCU is an alternative version of the same protection
used by the MJ-8956 games (there are even the same kind of error codes!(i.e the number
printed on the up-left corner of the screen).

******************************************************************************************/


#define PROT_JSR(_offs_,_protvalue_,_pc_) \
	if(m_mainram[(_offs_)/2] == _protvalue_) \
	{ \
		m_mainram[(_offs_)/2] = 0xffff;  /*(MCU job done)*/ \
		m_mainram[(_offs_+2-0x10)/2] = 0x4ef9;/*JMP*/\
		m_mainram[(_offs_+4-0x10)/2] = 0x0000;/*HI-DWORD*/\
		m_mainram[(_offs_+6-0x10)/2] = _pc_;  /*LO-DWORD*/\
	}
#define PROT_INPUT(_offs_,_protvalue_,_protinput_,_input_) \
	if(m_mainram[_offs_] == _protvalue_) \
	{ \
		m_mainram[_protinput_] = ((_input_ & 0xffff0000)>>16);\
		m_mainram[_protinput_+1] = (_input_ & 0x0000ffff);\
	}

#ifdef UNUSED_FUNCTION
READ16_MEMBER(nmk16_state::mcu_shared_r)
{
	return nmk16_mcu_shared_ram[offset];
}
#endif

//td     - hmf
//008D9E - 00796e
/*
(Old notes, for reference)

007B9E: bra     7b9c
007BA0: move.w  #$10, $f907a.l
007BA8: bsr     8106
007BAC: bsr     dfc4
007BB0: bsr     c44e
007BB4: bcs     7cfa
007BB8: bsr     d9c6
007BBC: bsr     9400
007BC0: bsr     7a54
007BC4: bsr     da06
007BC8: cmpi.w  #$3, $f907a.l
007BD0: bcc     7be2
007BD2: move.w  #$a, $f530e.l
007BDA: move.w  #$a, $f670e.l
007BE2: bsr     81aa
007BE6: bsr     8994
007BEA: bsr     8c36
007BEE: bsr     8d0c
007BF2: bsr     870a
007BF6: bsr     9d66
007BFA: bsr     b3f2
007BFE: bsr     b59e
007C02: bsr     9ac2
007C06: bsr     c366

thunder dragon algorithm (level 1):
90 - spriteram update
a0 - tilemap update
b0 - player inputs
c0 - controls sprite animation
d0 - player shoots
e0 - controls power-ups
f0 - player bombs
00 - controls player shoots
10 - ?
20 - level logic
30 - enemy appearence
40 - enemy energy
50 - enemy energy 2
60 - enemy shoots

hacha mecha fighter algorithm (level 1):
90 - spriteram update (d9c6)
a0 - tilemap update (d1f8?)
b0 - player inputs (da06)
c0 - controls sprite animation (81aa)
d0 - player shoots (8994)
e0 - controls power-ups & options (8d0c)
f0 - player bombs (8c36)
00 - controls player shoots (870a)
10 - ?
20 - level logic (9642)
30 - enemy appearence (9d66)
40 - enemy energy (b3f2)
50 - enemy energy 2 (b59e)
60 - enemy shoots (9ac2)
70 - ?
80 - <unused>

*/

WRITE16_MEMBER(nmk16_state::hachamf_mainram_w)
{
	COMBINE_DATA(&m_mainram[offset]);
#define DUMMYA 0x7b9c
// 7960
	switch(offset)
	{
		case 0xe058/2: PROT_INPUT(0xe058/2,0xc71f,0xe000/2,0x00080000); break;
		case 0xe182/2: PROT_INPUT(0xe182/2,0x865d,0xe004/2,0x00080002); break;
		case 0xe51e/2: PROT_INPUT(0xe51e/2,0x0f82,0xe008/2,0x00080008); break;
		case 0xe6b4/2: PROT_INPUT(0xe6b4/2,0x79be,0xe00c/2,0x0008000a); break;
		case 0xe10e/2: PROT_JSR(0xe10e,0x8007,0x870a);//870a not 9d66
						PROT_JSR(0xe10e,0x8000,0xd9c6); break;
		case 0xe11e/2: PROT_JSR(0xe11e,0x8038,DUMMYA);//972a - (unused)
						PROT_JSR(0xe11e,0x8031,0x7a54); break;
		case 0xe12e/2: PROT_JSR(0xe12e,0x8019,0x9642);//OK-9642
						PROT_JSR(0xe12e,0x8022,0xda06); break;
		case 0xe13e/2: PROT_JSR(0xe13e,0x802a,0x9d66);//9d66 not 9400 - OK
						PROT_JSR(0xe13e,0x8013,0x81aa); break;
		case 0xe14e/2: PROT_JSR(0xe14e,0x800b,0xb3f2);//b3f2 - OK
						PROT_JSR(0xe14e,0x8004,0x8994); break;
		case 0xe15e/2: PROT_JSR(0xe15e,0x803c,0xb59e);//b59e - OK
						PROT_JSR(0xe15e,0x8035,0x8c36); break;
		case 0xe16e/2: PROT_JSR(0xe16e,0x801d,0x9ac2);//9ac2 - OK
						PROT_JSR(0xe16e,0x8026,0x8d0c); break;
		case 0xe17e/2: PROT_JSR(0xe17e,0x802e,0xc366);//c366 - OK
						PROT_JSR(0xe17e,0x8017,0x870a); break;
		case 0xe18e/2: PROT_JSR(0xe18e,0x8004,DUMMYA);        //unused
						PROT_JSR(0xe18e,0x8008,DUMMYA); break; //unused
		case 0xe19e/2: PROT_JSR(0xe19e,0x8030,0xd9c6);//OK-d9c6
						PROT_JSR(0xe19e,0x8039,0x9642); break;
		case 0xe1ae/2: PROT_JSR(0xe1ae,0x8011,0x7a54);//d1f8 not c67e
						PROT_JSR(0xe1ae,0x802a,0x9d66); break;
		case 0xe1be/2: PROT_JSR(0xe1be,0x8022,0xda06);//da06
						PROT_JSR(0xe1be,0x801b,0xb3f2); break;
		case 0xe1ce/2: PROT_JSR(0xe1ce,0x8003,0x81aa);//81aa
						PROT_JSR(0xe1ce,0x800c,0xb59e); break;
		case 0xe1de/2: PROT_JSR(0xe1de,0x8034,0x8994);//8994 - OK
						PROT_JSR(0xe1de,0x803d,0x9ac2); break;
		case 0xe1ee/2: PROT_JSR(0xe1ee,0x8015,0x8c36);//8d0c not 82f6
						PROT_JSR(0xe1ee,0x802e,0xc366); break;
		case 0xe1fe/2: PROT_JSR(0xe1fe,0x8026,0x8d0c);//8c36
						PROT_JSR(0xe1fe,0x8016,DUMMYA); break;  //unused
		case 0xef00/2:
			if(m_mainram[0xef00/2] == 0x60fe)
			{
				m_mainram[0xef00/2] = 0x0000; //this is the coin counter
				m_mainram[0xef02/2] = 0x0000;
				m_mainram[0xef04/2] = 0x4ef9;
				m_mainram[0xef06/2] = 0x0000;
				m_mainram[0xef08/2] = 0x7dc2;
			}
			break;
	}
#undef DUMMYA
}


static ADDRESS_MAP_START( hachamf_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	/* I/O Region */
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	/* Video Region */
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c007) AM_WRITE(nmk_scroll_w)
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	/* Main RAM, inc sprites, shared with MCU */
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram") // ram is shared with MCU
ADDRESS_MAP_END



WRITE16_MEMBER(nmk16_state::tdragon_mainram_w)
{
	COMBINE_DATA(&m_mainram[offset]);

	switch(offset)
	{
		case 0xe066/2: PROT_INPUT(0xe066/2,0xe23e,0xe000/2,0x000c0000); break;
		case 0xe144/2: PROT_INPUT(0xe144/2,0xf54d,0xe004/2,0x000c0002); break;
		case 0xe60e/2: PROT_INPUT(0xe60e/2,0x067c,0xe008/2,0x000c0008); break;
		case 0xe714/2: PROT_INPUT(0xe714/2,0x198b,0xe00c/2,0x000c000a); break;
		case 0xe70e/2: PROT_JSR(0xe70e,0x8007,0x9e22);
						PROT_JSR(0xe70e,0x8000,0xd518); break;
		case 0xe71e/2: PROT_JSR(0xe71e,0x8038,0xaa0a);
						PROT_JSR(0xe71e,0x8031,0x8e7c); break;
		case 0xe72e/2: PROT_JSR(0xe72e,0x8019,0xac48);
						PROT_JSR(0xe72e,0x8022,0xd558); break;
		case 0xe73e/2: PROT_JSR(0xe73e,0x802a,0xb110);
						PROT_JSR(0xe73e,0x8013,0x96da); break;
		case 0xe74e/2: PROT_JSR(0xe74e,0x800b,0xb9b2);
						PROT_JSR(0xe74e,0x8004,0xa062); break;
		case 0xe75e/2: PROT_JSR(0xe75e,0x803c,0xbb4c);
						PROT_JSR(0xe75e,0x8035,0xa154); break;
		case 0xe76e/2: PROT_JSR(0xe76e,0x801d,0xafa6);
						PROT_JSR(0xe76e,0x8026,0xa57a); break;
		case 0xe77e/2: PROT_JSR(0xe77e,0x802e,0xc6a4);
						PROT_JSR(0xe77e,0x8017,0x9e22); break;
		case 0xe78e/2: PROT_JSR(0xe78e,0x8004,0xaa0a);
						PROT_JSR(0xe78e,0x8008,0xaa0a); break;
		case 0xe79e/2: PROT_JSR(0xe79e,0x8030,0xd518);
						PROT_JSR(0xe79e,0x8039,0xac48); break;
		case 0xe7ae/2: PROT_JSR(0xe7ae,0x8011,0x8e7c);
						PROT_JSR(0xe7ae,0x802a,0xb110); break;
		case 0xe7be/2: PROT_JSR(0xe7be,0x8022,0xd558);
						PROT_JSR(0xe7be,0x801b,0xb9b2); break;
		case 0xe7ce/2: PROT_JSR(0xe7ce,0x8003,0x96da);
						PROT_JSR(0xe7ce,0x800c,0xbb4c); break;
		case 0xe7de/2: PROT_JSR(0xe7de,0x8034,0xa062);
						PROT_JSR(0xe7de,0x803d,0xafa6); break;
		case 0xe7ee/2: PROT_JSR(0xe7ee,0x8015,0xa154);
						PROT_JSR(0xe7ee,0x802e,0xc6a4); break;
		case 0xe7fe/2: PROT_JSR(0xe7fe,0x8026,0xa57a);
						PROT_JSR(0xe7fe,0x8016,0xa57a); break;
		case 0xef00/2:
			if(m_mainram[0xef00/2] == 0x60fe)
			{
				m_mainram[0xef00/2] = 0x0000; //this is the coin counter
				m_mainram[0xef02/2] = 0x0000;
				m_mainram[0xef04/2] = 0x4ef9;
				m_mainram[0xef06/2] = 0x0000;
				m_mainram[0xef08/2] = 0x92f4;
			}
			break;
	}
}

/*coin setting MCU simulation*/
void nmk16_state::mcu_run(UINT8 dsw_setting)
{
	UINT16 coin_input;
	UINT8 dsw[2];
	UINT8 i;

	/*Accept the start button but needs some m68k processing first,otherwise you can't start a play with 1 credit inserted*/
	if(m_start_helper & 1 && m_mainram[0x9000/2] & 0x0200) /*start 1 */
	{
		m_mainram[0xef00/2]--;
		m_start_helper = m_start_helper & 2;
	}
	if(m_start_helper & 2 && m_mainram[0x9000/2] & 0x0100) /*start 2*/
	{
		m_mainram[0xef00/2]--;
		m_start_helper = m_start_helper & 1;
	}

	/*needed because of the uncompatibility of the dsw settings.*/
	if(dsw_setting) // Thunder Dragon
	{
		dsw[0] = (ioport("DSW2")->read() & 0x7);
		dsw[1] = (ioport("DSW2")->read() & 0x38) >> 3;
		for(i=0;i<2;i++)
		{
			switch(dsw[i] & 7)
			{
				case 0: m_mainram[0x9000/2]|=0x4000; break; //free play
				case 1: m_coin_count_frac[i] = 1; m_coin_count[i] = 4; break;
				case 2: m_coin_count_frac[i] = 1; m_coin_count[i] = 3; break;
				case 3: m_coin_count_frac[i] = 1; m_coin_count[i] = 2; break;
				case 4: m_coin_count_frac[i] = 4; m_coin_count[i] = 1; break;
				case 5: m_coin_count_frac[i] = 3; m_coin_count[i] = 1; break;
				case 6: m_coin_count_frac[i] = 2; m_coin_count[i] = 1; break;
				case 7: m_coin_count_frac[i] = 1; m_coin_count[i] = 1; break;
			}
		}
	}
	else // Hacha Mecha Fighter
	{
		dsw[0] = (ioport("DSW1")->read() & 0x0700) >> 8;
		dsw[1] = (ioport("DSW1")->read() & 0x3800) >> 11;
		for(i=0;i<2;i++)
		{
			switch(dsw[i] & 7)
			{
				case 0: m_mainram[0x9000/2]|=0x4000; break; //free play
				case 1: m_coin_count_frac[i] = 4; m_coin_count[i] = 1; break;
				case 2: m_coin_count_frac[i] = 3; m_coin_count[i] = 1; break;
				case 3: m_coin_count_frac[i] = 2; m_coin_count[i] = 1; break;
				case 4: m_coin_count_frac[i] = 1; m_coin_count[i] = 4; break;
				case 5: m_coin_count_frac[i] = 1; m_coin_count[i] = 3; break;
				case 6: m_coin_count_frac[i] = 1; m_coin_count[i] = 2; break;
				case 7: m_coin_count_frac[i] = 1; m_coin_count[i] = 1; break;
			}
		}
	}

	/*read the coin port*/
	coin_input = (~(ioport("IN0")->read()));

	if(coin_input & 0x01)//coin 1
	{
		if((m_input_pressed & 0x01) == 0)
		{
			if(m_coin_count_frac[0] != 1)
			{
				m_mainram[0xef02/2]+=m_coin_count[0];
				if(m_coin_count_frac[0] == m_mainram[0xef02/2])
				{
					m_mainram[0xef00/2]+=m_coin_count[0];
					m_mainram[0xef02/2] = 0;
				}
			}
			else
				m_mainram[0xef00/2]+=m_coin_count[0];
		}
		m_input_pressed = (m_input_pressed & 0xfe) | 1;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfe);

	if(coin_input & 0x02)//coin 2
	{
		if((m_input_pressed & 0x02) == 0)
		{
			if(m_coin_count_frac[1] != 1)
			{
				m_mainram[0xef02/2]+=m_coin_count[1];
				if(m_coin_count_frac[1] == m_mainram[0xef02/2])
				{
					m_mainram[0xef00/2]+=m_coin_count[1];
					m_mainram[0xef02/2] = 0;
				}
			}
			else
				m_mainram[0xef00/2]+=m_coin_count[1];
		}
		m_input_pressed = (m_input_pressed & 0xfd) | 2;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfd);

	if(coin_input & 0x04)//service 1
	{
		if((m_input_pressed & 0x04) == 0)
			m_mainram[0xef00/2]++;
		m_input_pressed = (m_input_pressed & 0xfb) | 4;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfb);

	/*The 0x9000 ram address is the status */
	if(m_mainram[0xef00/2] > 0 && m_mainram[0x9000/2] & 0x8000) //enable start button
	{
		if(coin_input & 0x08)//start 1
		{
			if((m_input_pressed & 0x08) == 0 && (!(m_mainram[0x9000/2] & 0x0200))) //start 1
				m_start_helper = 1;

			m_input_pressed = (m_input_pressed & 0xf7) | 8;
		}
		else
			m_input_pressed = (m_input_pressed & 0xf7);

		if(coin_input & 0x10)//start 2
		{
			/*Decrease two coins to let two players play with one start 2 button and two credits inserted at the insert coin screen.*/
			if((m_input_pressed & 0x10) == 0 && (!(m_mainram[0x9000/2] & 0x0100))) // start 2
				m_start_helper = (m_mainram[0x9000/2] == 0x8000) ? (3) : (2);

			m_input_pressed = (m_input_pressed & 0xef) | 0x10;
		}
		else
			m_input_pressed = (m_input_pressed & 0xef);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(nmk16_state::tdragon_mcu_sim)
{
	mcu_run(1);
}

TIMER_DEVICE_CALLBACK_MEMBER(nmk16_state::hachamf_mcu_sim)
{
	mcu_run(0);
}

static ADDRESS_MAP_START( tdragon_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x044022, 0x044023) AM_READNOP  /* No Idea */
//  AM_RANGE(0x0b0000, 0x0b7fff) AM_RAM    /* Work RAM */
//  AM_RANGE(0x0b8000, 0x0b8fff) AM_RAM AM_SHARE("spriteram") /* Sprite RAM */
//  AM_RANGE(0x0b9000, 0x0bdfff) AM_RAM AM_SHARE("mcu_work_ram")   /* Work RAM */
//  AM_RANGE(0x0be000, 0x0befff) AM_READWRITE(mcu_shared_r,tdragon_mcu_shared_w) AM_SHARE("mcu_shared_ram")  /* Work RAM */
//  AM_RANGE(0x0bf000, 0x0bffff) AM_RAM    /* Work RAM */
	AM_RANGE(0x0b0000, 0x0bffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("IN0")
	AM_RANGE(0x0c0002, 0x0c0003) AM_READ_PORT("IN1")
	AM_RANGE(0x0c0008, 0x0c0009) AM_READ_PORT("DSW1")
	AM_RANGE(0x0c000a, 0x0c000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x0c000e, 0x0c000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x0c0014, 0x0c0015) AM_WRITE(nmk_flipscreen_w) /* Maybe */
	AM_RANGE(0x0c0016, 0x0c0017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x0c0018, 0x0c0019) AM_WRITE(nmk_tilebank_w) /* Tile Bank ? */
	AM_RANGE(0x0c001e, 0x0c001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x0c4000, 0x0c4007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0x0c8000, 0x0c87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0cc000, 0x0cffff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x0d0000, 0x0d07ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
ADDRESS_MAP_END

// No sprites without this. Is it actually protection?
READ16_MEMBER(nmk16_state::tdragonb_prot_r)
{
	return 0x0003;
}

static ADDRESS_MAP_START( tdragonb_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x044022, 0x044023) AM_READ(tdragonb_prot_r)
	AM_RANGE(0x0b0000, 0x0bffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("IN0")
	AM_RANGE(0x0c0002, 0x0c0003) AM_READ_PORT("IN1")
	AM_RANGE(0x0c0008, 0x0c0009) AM_READ_PORT("DSW1")
	AM_RANGE(0x0c000a, 0x0c000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x0c0014, 0x0c0015) AM_WRITE(nmk_flipscreen_w) /* Maybe */
	AM_RANGE(0x0c0018, 0x0c0019) AM_WRITE(nmk_tilebank_w) /* Tile Bank ? */
	AM_RANGE(0x0c001e, 0x0c001f) AM_DEVWRITE("seibu_sound", seibu_sound_device, main_mustb_w)
	AM_RANGE(0x0c4000, 0x0c4007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0x0c8000, 0x0c87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0cc000, 0x0cffff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x0d0000, 0x0d07ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ssmissin_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0b0000, 0x0bffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("IN0")
	AM_RANGE(0x0c0004, 0x0c0005) AM_READ_PORT("IN1")
	AM_RANGE(0x0c0006, 0x0c0007) AM_READ_PORT("DSW1")
//  AM_RANGE(0x0c000e, 0x0c000f) AM_READ(??)
	AM_RANGE(0x0c0014, 0x0c0015) AM_WRITE(nmk_flipscreen_w) /* Maybe */
	AM_RANGE(0x0c0018, 0x0c0019) AM_WRITE(nmk_tilebank_w) /* Tile Bank ? */
	AM_RANGE(0x0c001e, 0x0c001f) AM_WRITE(ssmissin_sound_w)
	AM_RANGE(0x0c4000, 0x0c4007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0x0c8000, 0x0c87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0cc000, 0x0cffff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x0d0000, 0x0d07ff) AM_MIRROR(0x1800) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram") //mirror for airattck
ADDRESS_MAP_END

static ADDRESS_MAP_START( ssmissin_sound_map, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(ssmissin_soundbank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( oki1_map, AS_0, 8, nmk16_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( oki2_map, AS_0, 8, nmk16_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( strahl_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x80000, 0x80001) AM_READ_PORT("IN0")
	AM_RANGE(0x80002, 0x80003) AM_READ_PORT("IN1")
	AM_RANGE(0x80008, 0x80009) AM_READ_PORT("DSW1")
	AM_RANGE(0x8000a, 0x8000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x8000e, 0x8000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x80014, 0x80015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x80016, 0x80017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x8001e, 0x8001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x84000, 0x84007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0x88000, 0x88007) AM_RAM_WRITE(nmk_scroll_2_w)
	AM_RANGE(0x8c000, 0x8c7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x90000, 0x93fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x94000, 0x97fff) AM_RAM_WRITE(nmk_fgvideoram_w) AM_SHARE("nmk_fgvideoram")
	AM_RANGE(0x9c000, 0x9c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0xf0000, 0xfffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( macross_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c007) AM_RAM_WRITE(nmk_scroll_w)
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( gunnail_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x08000e, 0x08000f) AM_DEVREAD8("nmk004", nmk004_device, read, 0x00ff)
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x080016, 0x080017) AM_WRITE(nmk16_x0016_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x08001e, 0x08001f) AM_DEVWRITE8("nmk004", nmk004_device, write, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08c1ff) AM_WRITEONLY AM_SHARE("scrollram")
	AM_RANGE(0x08c200, 0x08c3ff) AM_WRITEONLY AM_SHARE("scrollramy")
	AM_RANGE(0x08c400, 0x08c7ff) AM_WRITEONLY   // unknown
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x09c000, 0x09cfff) AM_MIRROR(0x001000) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( macross2_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("IN0")
	AM_RANGE(0x100002, 0x100003) AM_READ_PORT("IN1")
	AM_RANGE(0x100008, 0x100009) AM_READ_PORT("DSW1")
	AM_RANGE(0x10000a, 0x10000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x10000e, 0x10000f) AM_READ(soundlatch2_word_r)    /* from Z80 */
	AM_RANGE(0x100014, 0x100015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x100016, 0x100017) AM_WRITE(macross2_sound_reset_w)   /* Z80 reset */
	AM_RANGE(0x100018, 0x100019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x10001e, 0x10001f) AM_WRITE(macross2_sound_command_w) /* to Z80 */
	AM_RANGE(0x120000, 0x1207ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x130000, 0x1301ff) AM_RAM AM_SHARE("scrollram")
	AM_RANGE(0x130200, 0x1303ff) AM_RAM AM_SHARE("scrollramy")
	AM_RANGE(0x130400, 0x1307ff) AM_RAM

	AM_RANGE(0x140000, 0x143fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x144000, 0x147fff) AM_RAM_WRITE(nmk_bgvideoram1_w) AM_SHARE("nmk_bgvideoram1")
	AM_RANGE(0x148000, 0x14bfff) AM_RAM_WRITE(nmk_bgvideoram2_w) AM_SHARE("nmk_bgvideoram2")
	AM_RANGE(0x14c000, 0x14ffff) AM_RAM_WRITE(nmk_bgvideoram3_w) AM_SHARE("nmk_bgvideoram3")

	AM_RANGE(0x170000, 0x170fff) AM_MIRROR(0x1000) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( raphero_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("IN0")
	AM_RANGE(0x100002, 0x100003) AM_READ_PORT("IN1")
	AM_RANGE(0x100008, 0x100009) AM_READ_PORT("DSW1")
	AM_RANGE(0x10000a, 0x10000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x10000e, 0x10000f) AM_READ(soundlatch2_word_r)    /* from Z80 */
	AM_RANGE(0x100014, 0x100015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x100016, 0x100017) AM_WRITENOP    /* IRQ enable or z80 sound reset like in Macross 2? */
	AM_RANGE(0x100018, 0x100019) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x10001e, 0x10001f) AM_WRITE(macross2_sound_command_w) /* to Z80 */
	AM_RANGE(0x120000, 0x1207ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x130000, 0x1301ff) AM_RAM AM_SHARE("scrollram")
	AM_RANGE(0x130200, 0x1303ff) AM_RAM AM_SHARE("scrollramy")
	AM_RANGE(0x130400, 0x1307ff) AM_RAM

	AM_RANGE(0x140000, 0x143fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x144000, 0x147fff) AM_RAM_WRITE(nmk_bgvideoram1_w) AM_SHARE("nmk_bgvideoram1")
	AM_RANGE(0x148000, 0x14bfff) AM_RAM_WRITE(nmk_bgvideoram2_w) AM_SHARE("nmk_bgvideoram2")
	AM_RANGE(0x14c000, 0x14ffff) AM_RAM_WRITE(nmk_bgvideoram3_w) AM_SHARE("nmk_bgvideoram3")


	AM_RANGE(0x170000, 0x170fff) AM_MIRROR(0x1000) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( raphero_sound_mem_map, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xc800, 0xc800) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0xc808, 0xc808) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0xc810, 0xc817) AM_DEVWRITE("nmk112", nmk112_device, okibank_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(macross2_sound_bank_w)
	AM_RANGE(0xd800, 0xd800) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w)    // main cpu
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( macross2_sound_map, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")    /* banked ROM */
	AM_RANGE(0xa000, 0xa000) AM_READNOP /* IRQ ack? watchdog? */
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe001, 0xe001) AM_WRITE(macross2_sound_bank_w)
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(soundlatch_byte_r,soundlatch2_byte_w) /* from 68000 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( macross2_sound_io_map, AS_IO, 8, nmk16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0x88, 0x88) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0x90, 0x97) AM_DEVWRITE("nmk112", nmk112_device, okibank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bjtwin_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
	AM_RANGE(0x084000, 0x084001) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x084010, 0x084011) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x084020, 0x08402f) AM_DEVWRITE8("nmk112", nmk112_device, okibank_w, 0x00ff)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x094000, 0x094001) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x094002, 0x094003) AM_WRITENOP    /* IRQ enable? */
	AM_RANGE(0x09c000, 0x09cfff) AM_MIRROR(0x1000) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( atombjt_map, AS_PROGRAM, 16, nmk16_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0x0C2010, 0x0C2011) AM_READ_PORT("IN0")
	AM_RANGE(0x0C2012, 0x0C2013) AM_READ(atombjt_unkr_r)
	AM_RANGE(0x0C2014, 0x0C2015) AM_READ(atombjt_unkr_r)

	AM_RANGE(0x0C2016, 0x0C2017) AM_READ_PORT("DSW1")
	AM_RANGE(0x0C2018, 0x0C2019) AM_READ_PORT("DSW2")

	AM_RANGE(0x100000, 0x101fff) AM_RAM

	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")

	AM_RANGE(0x080014, 0x080015) AM_WRITE(nmk_flipscreen_w)
//  AM_RANGE(0x084000, 0x084001) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x084010, 0x084011) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x084020, 0x08402f) AM_DEVWRITE("nmk112", nmk112_device, okibank_lsb_w)
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x094000, 0x094001) AM_WRITE(nmk_tilebank_w)
	AM_RANGE(0x094002, 0x094003) AM_WRITENOP    /* IRQ enable? */
	AM_RANGE(0x09c000, 0x09cfff) AM_MIRROR(0x1000) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static INPUT_PORTS_START( vandyke )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:8")   /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x00,  "2" )
	PORT_DIPSETTING(    0x01,  "3" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:7")   /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6")   /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:5" )                 /* The manual states this dip is "Unused" */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")   /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vandykeb )
	PORT_INCLUDE( vandyke )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* Tested on boot */
INPUT_PORTS_END

static INPUT_PORTS_START( blkheart )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x40,  "2" )
	PORT_DIPSETTING(    0xc0,  "3" )
	PORT_DIPSETTING(    0x80,  "4" )
	PORT_DIPSETTING(    0x00,  "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( manybloc )
	PORT_START("IN0")   /* 0x080000 */
	PORT_BIT( 0x7fff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     // VBLANK ? Check code at 0x005640

	PORT_START("IN1")   /* 0x080002 */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // select fruits
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // help
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // select fruits
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // help
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("DSW1")  /* 0x080004 -> 0x0f0036 */
	PORT_DIPNAME( 0x0001, 0x0000, "Slot System" )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Explanation" )           PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:4")   // "Play Type"
	PORT_DIPSETTING(      0x0008, DEF_STR( Upright ) )                      //   "Uplight" !
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )                     //   "Table"
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW1:5" )                        // "Test Mode"
	PORT_DIPNAME( 0x0060, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0060, DEF_STR( Easy ) )                 //   "Level 1
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )               //   "Level 2
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )                 //   "Level 3
	PORT_DIPSETTING(      0x0040, DEF_STR( Hardest ) )              //   "Level 4
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8")   // "Display"
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )                          //   "Normal"
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )                           //   "Inverse"
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0700, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x0000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x3800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc000, 0x0000, "Plate Probability" )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "Bad" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, "Better" )
	PORT_DIPSETTING(      0x8000, "Best" )
INPUT_PORTS_END


/**********************************************************
  Input Ports: Task Force Harrier

  this is a little strange compared to the other games, the
  protection might be more involved here than it first
  appears, however, this works.
**********************************************************/

static INPUT_PORTS_START( tharrier )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x7fe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL ) /* Mcu status? */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2) //title
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2  ) //in game
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x3000, "200k" )
	PORT_DIPSETTING(      0x2000, "200k and 1 Mil" )
	PORT_DIPSETTING(      0x0000, "200k, 500k & 1,2,3,5 Mil" )
	PORT_DIPSETTING(      0x1000, DEF_STR( None ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 )PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )//coin ?
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //coin ?
INPUT_PORTS_END


static INPUT_PORTS_START( mustang )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // TEST in service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )

	PORT_START("COIN")  /* referenced by seibu sound board */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( hachamf_prot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //bryan:  test mode in some games?

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(    0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0040, "2" )
	PORT_DIPSETTING(    0x00c0, "3" )
	PORT_DIPSETTING(    0x0080, "4" )

	PORT_START("DSW2")
INPUT_PORTS_END


static INPUT_PORTS_START( hachamfb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //bryan:  test mode in some games?

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6") // turning these 3 off results in a broken freeplay mode, probably a leftover from the bootleg not simulating coinage settings
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5") // ^
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4") // ^
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3") // likewise turning these 3 off
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2") // ^
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1") // ^
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( strahl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //bryan:  test mode in some games?

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, "100k and every 200k" )
	PORT_DIPSETTING(    0x60, "200k and every 200k" )
	PORT_DIPSETTING(    0x20, "300k and every 300k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( acrobatm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* used by secret code */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x001C, 0x001C, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001C, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000C, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00E0, 0x00E0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00C0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00E0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00A0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(    0x02, "50k and 100k" )
	PORT_DIPSETTING(    0x06, "100k and 100k" )
	PORT_DIPSETTING(    0x04, "100k and 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( bioship )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //bryan:  test mode in some games?

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0008, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x00C0, 0x00C0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x00C0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0040, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x001C, 0x001C, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001C, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000C, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00E0, 0x00E0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00C0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00E0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00A0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tdragon_prot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // TEST in service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:3") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7") /* The MCU (undumped/unemulated) takes care of this */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tdragon )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // TEST in service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:3") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )

	PORT_START("DSW2") // reverse bit order compared to tdragon_prot?
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tdragonb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // TEST in service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:3") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("COIN")  /* referenced by seibu sound board */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ssmissin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // "Servise" in "test mode"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       // "Bomb"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)       // "Fire"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)       // "Bomb"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
#if 0
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")   // initialised but not read back
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#endif
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( airattck )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // "Servise" in "test mode"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       // "Fire"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       // "Bomb"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)       // "Fire"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)       // "Bomb"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( macross )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( macross2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3") /* Initial points needed for 1st Stage Clear */
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )     /* 100,000 */
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )       /* 150,000 */
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )     /* 200,000 */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      /* 250,000 */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tdragon2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gunnail )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:7")   /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x02, DEF_STR( Japanese ) )                     /* Will add "Distributed by TECMO" to the title screen */
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")    /* The manual states dips 1-4 are "Unused" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( raphero )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:7") /* Main characters text "talk" at Stage Clear screen, but only when set to Japanese */
	PORT_DIPSETTING(    0x02, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x00, DEF_STR( Off) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sabotenb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* shown in service mode, but no effect */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bjtwin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* shown in service mode, but no effect */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Starting level" )        PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(    0x08, "Germany" )
	PORT_DIPSETTING(    0x04, "Thailand" )
	PORT_DIPSETTING(    0x0c, "Nevada" )
	PORT_DIPSETTING(    0x0e, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x0a, "England" )
	PORT_DIPSETTING(    0x02, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x00, DEF_STR( China ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nouryoku )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Life Decrease Speed" )   PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x02, "Slow" )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:7") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:6") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:5") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:4") /* The manual states this dip is "Unused" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END




/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                                Stagger I
***************************************************************************/

static INPUT_PORTS_START( afega_common )
	PORT_START("IN0")   // $080000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("IN1")   // $080002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( stagger1 )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0300, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0200, "Horizontally" )
	PORT_DIPSETTING(      0x0100, "Vertically" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/* everything seems active high.. not low */
static INPUT_PORTS_START( redhawkb )
	PORT_START("IN0")   // $080000.w
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_COIN1    )
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_COIN2    )
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_START1   )
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_START2   )
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT(  0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_START("IN1")   // $080002.w
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")  // $080004.w  -- probably just redhawk but inverted
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8") /* Other sets, this is TEST MODE, but here it doesn't work */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0080, "5" )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8,7")    /* not supported */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0100, "Horizontally" )
	PORT_DIPSETTING(      0x0200, "Vertically" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0xe000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/***************************************************************************
                            Sen Jin - Guardian Storm
***************************************************************************/

static INPUT_PORTS_START( grdnstrm )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Bombs" )             PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:4" )            /* Listed as "Unused" & doesn't show in test mode */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:3" )            /* Listed as "Unused" & doesn't show in test mode */
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Mirror Screen" )         PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( grdnstrk )
	PORT_INCLUDE( grdnstrm )

	PORT_MODIFY("DSW1") // $080004.w
	PORT_DIPNAME( 0x0200, 0x0200, "Mirror Screen" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                            Pop's Pop's
***************************************************************************/

static INPUT_PORTS_START( popspops )
	PORT_INCLUDE( afega_common )

	/* the dips on this are a mess.. service mode doesn't seem to be 100% trustable */
	PORT_START("DSW1")  // $080004.w
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) // if ON it tells you the answers?!
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/***************************************************************************
                                Bubble 2000
***************************************************************************/

static INPUT_PORTS_START( bubl2000 )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8") /* Manual lists as "Screen Flip Horizontal"  Doesn't work??? */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:7") /* Manual lists as "Screen Flip Vertical"  Doesn't work??? */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Free Credit" )           PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x0080, "500k" )
	PORT_DIPSETTING(      0x00c0, "800k" )
	PORT_DIPSETTING(      0x0040, "1000k" )
	PORT_DIPSETTING(      0x0000, "1500k" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7") /* Manual lists as "Unused" */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(      0x0000, "Disabled" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x8000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(      0x0000, "Disabled" )
INPUT_PORTS_END

/***************************************************************************
                                Mang Chi
***************************************************************************/

static INPUT_PORTS_START( mangchi )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_DIPNAME( 0x0001, 0x0001, "DSWS" )      /* Setting to on cuases screen issues, Flip Screen? or unfinished test mode? */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Vs Rounds" )
	PORT_DIPSETTING(      0x0018, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )   /* Hard to tell levels of difficulty by play :-( */
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


/***************************************************************************
                                Fire Hawk
***************************************************************************/

static INPUT_PORTS_START( firehawk )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0006, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
//  PORT_DIPSETTING(      0x000a, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( Normal ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Number of Bombs" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Region ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( China ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, "Continue Coins" )        PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x1800, "1 Coin" )
	PORT_DIPSETTING(      0x0800, "2 Coins" )
	PORT_DIPSETTING(      0x1000, "3 Coins" )
	PORT_DIPSETTING(      0x0000, "4 Coins" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( firehawkv )
	PORT_INCLUDE( firehawk )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0100, 0x0000, "Orientation" )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, "Vertical" )
	PORT_DIPSETTING(      0x0000, "Horizontal" )
INPUT_PORTS_END


/***************************************************************************
                             Spectrum 2000
***************************************************************************/

static INPUT_PORTS_START( spec2k )
	PORT_INCLUDE( afega_common )

	PORT_START("DSW1")  // $080004.w
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Number of Bombs" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0010, 0x0010, "Copyright Notice" )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


/***************************************************************************
                                Twin Action
***************************************************************************/

static INPUT_PORTS_START( twinactn )
	PORT_INCLUDE( afega_common )

	PORT_MODIFY("IN0")  // $080000.w
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW   )    // Test in service mode

	PORT_MODIFY("IN1")  // $080002.w
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH,IPT_UNKNOWN ) // Tested at boot
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH,IPT_UNKNOWN ) // Tested at boot

	PORT_START("DSW1")  // $080004.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( dolmen )
	PORT_INCLUDE( afega_common )

	PORT_MODIFY("IN0")  // $080000.w
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW   )    // Test in service mode

	PORT_MODIFY("IN1")  // $080002.w
	PORT_BIT(  0x0080, IP_ACTIVE_LOW,IPT_UNKNOWN ) // Tested at boot
	PORT_BIT(  0x8000, IP_ACTIVE_LOW,IPT_UNKNOWN ) // Tested at boot

	PORT_START("DSW1")  // $080004.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Free Credit" )  PORT_DIPLOCATION("SW1:2,1") /* Not verified - From Bubble 2000 */
	PORT_DIPSETTING(      0x0080, "500k" )
	PORT_DIPSETTING(      0x00c0, "800k" )
	PORT_DIPSETTING(      0x0040, "1000k" )
	PORT_DIPSETTING(      0x0000, "1500k" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "Credits Don't Register" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0x8000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "Credits Don't Register" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( tharrier )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x000, 16 ) /* color 0x200-0x2ff */
	GFXDECODE_ENTRY( "bgtile", 0, tilelayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
GFXDECODE_END

static GFXDECODE_START( macross )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x200, 16 ) /* color 0x200-0x2ff */
	GFXDECODE_ENTRY( "bgtile", 0, tilelayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
GFXDECODE_END

static GFXDECODE_START( macross2 )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x300, 16 ) /* color 0x300-0x3ff */
	GFXDECODE_ENTRY( "bgtile", 0, tilelayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 32 ) /* color 0x100-0x2ff */
GFXDECODE_END

static GFXDECODE_START( bjtwin )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "bgtile", 0, charlayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
GFXDECODE_END




static GFXDECODE_START( bioship )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x300, 16 ) /* color 0x300-0x3ff */
	GFXDECODE_ENTRY( "bgtile", 0, tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x200, 16 ) /* color 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 0x000, 16 ) /* color 0x000-0x0ff */
GFXDECODE_END

static GFXDECODE_START( strahl )
	GFXDECODE_ENTRY( "fgtile", 0, charlayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "bgtile", 0, tilelayout, 0x300, 16 ) /* color 0x300-0x3ff */
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 0x200, 16 ) /* color 0x200-0x2ff */
GFXDECODE_END



static const gfx_layout atombjt_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24},
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout atombjt_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24},
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 256+0*32, 256+1*32, 256+2*32, 256+3*32, 256+4*32, 256+5*32, 256+6*32, 256+7*32 },
	128*8
};

static GFXDECODE_START( atombjt )
	GFXDECODE_ENTRY( "fgtile", 0, atombjt_charlayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "bgtile", 0, atombjt_charlayout, 0x000, 16 ) /* color 0x000-0x0ff */
	GFXDECODE_ENTRY( "sprites", 0, atombjt_tilelayout, 0x100, 16 ) /* color 0x100-0x1ff */
GFXDECODE_END


/*
----

IRQ1 - Half-blanking interrupt
IRQ2 - Display interrupt
IRQ4 - V-blanking interrupt

Timing:

  17.8 msec
 |<---------------------------->|
 | 3.45 msec |    14.35 msec    |
 |<--------->|<---------------->|
 |           |   | 8.9 msec |   |
 |           |   |<-------->|   |
LV4         LV2 LV1        LV1
 |        DISPLAY
 |<->|<--->| |
 |256| 694 | |
 | us| usec| |
 |   | DMA | |

 CPU is stopped during DMA

 */

// todo:total scanlines is 263, adjust according to that!
// todo: replace with raw screen timings
TIMER_DEVICE_CALLBACK_MEMBER(nmk16_state::nmk16_scanline)
{
	const int NUM_SCANLINES = 256;
	const int IRQ1_SCANLINE = 25; // guess
	const int VBIN_SCANLINE = 0;
	const int VBOUT_sCANLINE = 240;
	const int SPRDMA_SCANLINE = 241; // 256 USEC after VBOUT

	int scanline = param;

	if(scanline == VBOUT_sCANLINE) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == SPRDMA_SCANLINE)
	{
		// 2 buffers confirmed on PCB
		memcpy(m_spriteram_old2.get(),m_spriteram_old.get(), 0x1000);
		memcpy(m_spriteram_old.get(), m_mainram + m_sprdma_base / 2, 0x1000);
	}

	/* Vblank-in irq, Vandyke definitely relies that irq fires at scanline ~0 instead of 112 (as per previous
	  cpu_getiloops function implementation), mostly noticeable with sword collisions and related attract mode behaviour. */
	if(scanline == VBIN_SCANLINE)
		m_maincpu->set_input_line(2, HOLD_LINE);

	/* time from IRQ2 to first IRQ1 fire. is not stated, 25 is a guess */
	if(scanline == IRQ1_SCANLINE)
		m_maincpu->set_input_line(1, HOLD_LINE);

	/* 8.9ms from first IRQ1 to second IRQ1 fire. approx 128 lines (half frame time) */
	if(scanline == IRQ1_SCANLINE+(NUM_SCANLINES/2)) // if this happens too late bioship sprites will glitch on the left edge
		m_maincpu->set_input_line(1, HOLD_LINE);
}

#define NMK_HACKY_INTERRUPT_TIMING \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", nmk16_state, nmk16_scanline, "screen", 0, 1)
#define NMK_HACKY_SCREEN_LOWRES \
	MCFG_SCREEN_ADD("screen", RASTER) \
	MCFG_SCREEN_REFRESH_RATE(56.18) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3450)) \
	MCFG_SCREEN_SIZE(256, 256) \
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1) \
	MCFG_SCREEN_PALETTE("palette")

#define NMK_HACKY_SCREEN_HIRES \
	MCFG_SCREEN_ADD("screen", RASTER) \
	MCFG_SCREEN_REFRESH_RATE(56.18) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3450)) \
	MCFG_SCREEN_SIZE(512, 256) \
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1) \
	MCFG_SCREEN_PALETTE("palette")




static MACHINE_CONFIG_START( tharrier, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(tharrier_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(tharrier_sound_map)
	MCFG_CPU_IO_MAP(tharrier_sound_io_map)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_tharrier)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tharrier)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)
	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mustang, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(mustang_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mustangb, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(mustangb_map)
	NMK_HACKY_INTERRUPT_TIMING

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4, 1320000)

MACHINE_CONFIG_END

#define BIOSHIP_CRYSTAL1 10000000
#define BIOSHIP_CRYSTAL2 12000000

static MACHINE_CONFIG_START( bioship, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, BIOSHIP_CRYSTAL1 ) /* 10.0 MHz (verified) */
	MCFG_CPU_PROGRAM_MAP(bioship_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_bioship)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bioship)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,bioship)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, BIOSHIP_CRYSTAL2 / 8) /* 1.5 Mhz (verified) */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", BIOSHIP_CRYSTAL2 / 3 , OKIM6295_PIN7_LOW) /* 4.0 Mhz, Pin 7 High (verified) */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", BIOSHIP_CRYSTAL2 / 3 , OKIM6295_PIN7_LOW) /* 4.0 Mhz, Pin 7 High (verified) */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vandyke, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz) /* 68000p12 running at 10Mhz, verified on pcb */
	MCFG_CPU_PROGRAM_MAP(vandyke_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", XTAL_12MHz/3, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", XTAL_12MHz/3, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vandykeb, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(vandykeb_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("mcu", PIC16C57, 12000000) /* 3MHz */
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( acrobatm, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz (verified on pcb) */
	MCFG_CPU_PROGRAM_MAP(acrobatm_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000) /* (verified on pcb) */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW) /* (verified on pcb) on the pcb pin7 is not connected to gnd or +5v! */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW) /* (verified on pcb) on the pcb pin7 is not connected to gnd or +5v! */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( tdragonb, nmk16_state )    /* bootleg using Raiden sound hardware */

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(tdragonb_map)
	NMK_HACKY_INTERRUPT_TIMING

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4, 1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tdragon, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(tdragon_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", XTAL_8MHz/2, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", XTAL_8MHz/2, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END
static MACHINE_CONFIG_DERIVED( tdragon_prot, tdragon )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("coinsim", nmk16_state, tdragon_mcu_sim, attotime::from_hz(10000))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ssmissin, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) /* 8 Mhz */
	MCFG_CPU_PROGRAM_MAP(ssmissin_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, 8000000/2) /* 4 Mhz */
	MCFG_CPU_PROGRAM_MAP(ssmissin_sound_map)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 8000000/8, OKIM6295_PIN7_HIGH) /* 1 Mhz, pin 7 high */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( strahl, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000) /* 12 MHz ? */
	MCFG_CPU_PROGRAM_MAP(strahl_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_strahl)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", strahl)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,strahl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hachamf, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(hachamf_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END
static MACHINE_CONFIG_DERIVED( hachamf_prot, hachamf )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("coinsim", nmk16_state, hachamf_mcu_sim, attotime::from_hz(10000))
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( macross, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(macross_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( blkheart, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(macross_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", 8000000)

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8 ) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", XTAL_8MHz/2, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", XTAL_8MHz/2, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gunnail, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(gunnail_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_gunnail)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,gunnail)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_NMK004_ADD("nmk004", XTAL_16MHz/2) /* verified on pcb */

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(DEVWRITELINE("nmk004", nmk004_device, ym2203_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", XTAL_16MHz/4, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", XTAL_16MHz/4, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macross2, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz ? */
	MCFG_CPU_PROGRAM_MAP(macross2_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(macross2_sound_map)
	MCFG_CPU_IO_MAP(macross2_sound_io_map)

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_gunnail)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross2)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_DEVICE_ADD("nmk112", NMK112, 0)
	MCFG_NMK112_ROM0("oki1")
	MCFG_NMK112_ROM1("oki2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tdragon2, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10 MHz  */
	MCFG_CPU_PROGRAM_MAP(macross2_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* 4 MHz  */
	MCFG_CPU_PROGRAM_MAP(macross2_sound_map)
	MCFG_CPU_IO_MAP(macross2_sound_io_map)

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_tdragon2)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross2)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_DEVICE_ADD("nmk112", NMK112, 0)
	MCFG_NMK112_ROM0("oki1")
	MCFG_NMK112_ROM1("oki2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( raphero, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000) /* 14 MHz measured */
	MCFG_CPU_PROGRAM_MAP(raphero_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu",TMP90841, 8000000)
	MCFG_CPU_PROGRAM_MAP(raphero_sound_mem_map)

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_tdragon2)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross2)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,raphero)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_DEVICE_ADD("nmk112", NMK112, 0)
	MCFG_NMK112_ROM0("oki1")
	MCFG_NMK112_ROM1("oki2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bjtwin, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(bjtwin_map)
	NMK_HACKY_INTERRUPT_TIMING

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_bjtwin)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bjtwin)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,bjtwin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_DEVICE_ADD("nmk112", NMK112, 0)
	MCFG_NMK112_ROM0("oki1")
	MCFG_NMK112_ROM1("oki2")
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( atombjt, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* there is a 28mhz crystal and a 10mhz crystal near the 12 rated CPU */
	MCFG_CPU_PROGRAM_MAP(atombjt_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nmk16_state,  irq6_line_hold) // recoded to use this irq

	/* video hardware */
	NMK_HACKY_SCREEN_HIRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_bjtwin)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atombjt)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,bjtwin)

	// the bootleg just has a single OKI
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW) // ?? mhz, I see no 16 crystal
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END



TIMER_DEVICE_CALLBACK_MEMBER(nmk16_state::manybloc_scanline)
{
	int scanline = param;

	if(scanline == 248) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == 248)
	{
		// only a single buffer
		memcpy(m_spriteram_old2.get(), m_mainram + m_sprdma_base / 2, 0x1000);
	}

	/* This is either vblank-in or sprite dma irq complete */
	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

// non-nmk board, different to the others, very timing sensitive
static MACHINE_CONFIG_START( manybloc, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* 10? MHz - check */
	MCFG_CPU_PROGRAM_MAP(manybloc_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nmk16_state, irq1_line_hold, 56)/* this needs to equal the framerate on this, rather than being double it .. */
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", nmk16_state, manybloc_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)
	MCFG_CPU_PROGRAM_MAP(tharrier_sound_map)
	MCFG_CPU_IO_MAP(tharrier_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(56)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_manybloc)

	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tharrier)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	MCFG_SOUND_ROUTE(2, "mono", 0.50)
	MCFG_SOUND_ROUTE(3, "mono", 1.20)

	MCFG_OKIM6295_ADD("oki1", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki1_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_OKIM6295_ADD("oki2", 16000000/4, OKIM6295_PIN7_LOW)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki2_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


UINT8 nmk16_state::decode_byte(UINT8 src, const UINT8 *bitp)
{
	UINT8 ret, i;

	ret = 0;
	for (i=0; i<8; i++)
		ret |= (((src >> bitp[i]) & 1) << (7-i));

	return ret;
}

UINT32 nmk16_state::bjtwin_address_map_bg0(UINT32 addr)
{
	return ((addr&0x00004)>> 2) | ((addr&0x00800)>> 10) | ((addr&0x40000)>>16);
}


UINT16 nmk16_state::decode_word(UINT16 src, const UINT8 *bitp)
{
	UINT16 ret, i;

	ret=0;
	for (i=0; i<16; i++)
		ret |= (((src >> bitp[i]) & 1) << (15-i));

	return ret;
}


UINT32 nmk16_state::bjtwin_address_map_sprites(UINT32 addr)
{
	return ((addr&0x00010)>> 4) | ((addr&0x20000)>>16) | ((addr&0x100000)>>18);
}


void nmk16_state::decode_gfx()
{
	/* GFX are scrambled.  We decode them here.  (BIG Thanks to Antiriad for descrambling info) */
	UINT8 *rom;
	int A, len;

	static const UINT8 decode_data_bg[8][8] =
	{
		{0x3,0x0,0x7,0x2,0x5,0x1,0x4,0x6},
		{0x1,0x2,0x6,0x5,0x4,0x0,0x3,0x7},
		{0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0},
		{0x7,0x6,0x5,0x0,0x1,0x4,0x3,0x2},
		{0x2,0x0,0x1,0x4,0x3,0x5,0x7,0x6},
		{0x5,0x3,0x7,0x0,0x4,0x6,0x2,0x1},
		{0x2,0x7,0x0,0x6,0x5,0x3,0x1,0x4},
		{0x3,0x4,0x7,0x6,0x2,0x0,0x5,0x1},
	};

	static const UINT8 decode_data_sprite[8][16] =
	{
		{0x9,0x3,0x4,0x5,0x7,0x1,0xb,0x8,0x0,0xd,0x2,0xc,0xe,0x6,0xf,0xa},
		{0x1,0x3,0xc,0x4,0x0,0xf,0xb,0xa,0x8,0x5,0xe,0x6,0xd,0x2,0x7,0x9},
		{0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0},
		{0xf,0xe,0xc,0x6,0xa,0xb,0x7,0x8,0x9,0x2,0x3,0x4,0x5,0xd,0x1,0x0},

		{0x1,0x6,0x2,0x5,0xf,0x7,0xb,0x9,0xa,0x3,0xd,0xe,0xc,0x4,0x0,0x8}, /* Haze 20/07/00 */
		{0x7,0x5,0xd,0xe,0xb,0xa,0x0,0x1,0x9,0x6,0xc,0x2,0x3,0x4,0x8,0xf}, /* Haze 20/07/00 */
		{0x0,0x5,0x6,0x3,0x9,0xb,0xa,0x7,0x1,0xd,0x2,0xe,0x4,0xc,0x8,0xf}, /* Antiriad, Corrected by Haze 20/07/00 */
		{0x9,0xc,0x4,0x2,0xf,0x0,0xb,0x8,0xa,0xd,0x3,0x6,0x5,0xe,0x1,0x7}, /* Antiriad, Corrected by Haze 20/07/00 */
	};


	/* background */
	rom = memregion("bgtile")->base();
	len = memregion("bgtile")->bytes();
	for (A = 0;A < len;A++)
	{
		rom[A] = decode_byte( rom[A], decode_data_bg[bjtwin_address_map_bg0(A)]);
	}

	/* sprites */
	rom = memregion("sprites")->base();
	len = memregion("sprites")->bytes();
	for (A = 0;A < len;A += 2)
	{
		UINT16 tmp = decode_word( rom[A+1]*256 + rom[A], decode_data_sprite[bjtwin_address_map_sprites(A)]);
		rom[A+1] = tmp >> 8;
		rom[A] = tmp & 0xff;
	}
}

void nmk16_state::decode_tdragonb()
{
	/* Descrambling Info Again Taken from Raine, Huge Thanks to Antiriad and the Raine Team for
	   going Open Source, best of luck in future development. */

	UINT8 *rom;
	int A, len;

	/* The Main 68k Program of the Bootleg is Bitswapped */
	static const UINT8 decode_data_tdragonb[1][16] =
	{
		{0xe,0xc,0xa,0x8,0x7,0x5,0x3,0x1,0xf,0xd,0xb,0x9,0x6,0x4,0x2,0x0},
	};

	/* Graphic Roms Could Also Do With Rearranging to make things simpler */
	static const UINT8 decode_data_tdragonbgfx[1][8] =
	{
		{0x7,0x6,0x5,0x3,0x4,0x2,0x1,0x0},
	};

	rom = memregion("maincpu")->base();
	len = memregion("maincpu")->bytes();
	for (A = 0;A < len;A += 2)
	{
		int h = A+NATIVE_ENDIAN_VALUE_LE_BE(1,0), l = A+NATIVE_ENDIAN_VALUE_LE_BE(0,1);
		UINT16 tmp = decode_word( rom[h]*256 + rom[l], decode_data_tdragonb[0]);
		rom[h] = tmp >> 8;
		rom[l] = tmp & 0xff;
	}

	rom = memregion("bgtile")->base();
	len = memregion("bgtile")->bytes();
	for (A = 0;A < len;A++)
	{
		rom[A] = decode_byte( rom[A], decode_data_tdragonbgfx[0]);
	}

	rom = memregion("sprites")->base();
	len = memregion("sprites")->bytes();
	for (A = 0;A < len;A++)
	{
		rom[A] = decode_byte( rom[A], decode_data_tdragonbgfx[0]);
	}
}

void nmk16_state::decode_ssmissin()
{
	/* Like Thunder Dragon Bootleg without the Program Rom Swapping */
	UINT8 *rom;
	int A, len;

	/* Graphic Roms Could Also Do With Rearranging to make things simpler */
	static const UINT8 decode_data_tdragonbgfx[1][8] =
	{
		{0x7,0x6,0x5,0x3,0x4,0x2,0x1,0x0},
	};

	rom = memregion("bgtile")->base();
	len = memregion("bgtile")->bytes();
	for (A = 0;A < len;A++)
	{
		rom[A] = decode_byte( rom[A], decode_data_tdragonbgfx[0]);
	}

	rom = memregion("sprites")->base();
	len = memregion("sprites")->bytes();
	for (A = 0;A < len;A++)
	{
		rom[A] = decode_byte( rom[A], decode_data_tdragonbgfx[0]);
	}
}


DRIVER_INIT_MEMBER(nmk16_state,nmk)
{
	decode_gfx();
}

DRIVER_INIT_MEMBER(nmk16_state,banked_audiocpu)
{
	membank("bank1")->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
}

DRIVER_INIT_MEMBER(nmk16_state,tharrier)
{
	membank("okibank1")->configure_entries(0, 4, memregion("oki1")->base() + 0x20000, 0x20000);
	membank("okibank2")->configure_entries(0, 4, memregion("oki2")->base() + 0x20000, 0x20000);
}

DRIVER_INIT_MEMBER(nmk16_state,hachamf_prot)
{
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	//rom[0x0006/2] = 0x7dc2;   /* replace reset vector with the "real" one */

	// kludge the sound communication to let commands go through.
	rom[0x048a/2] = 0x4e71;
	rom[0x04aa/2] = 0x4e71;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0f0000, 0x0fffff, write16_delegate(FUNC(nmk16_state::hachamf_mainram_w),this));
}

DRIVER_INIT_MEMBER(nmk16_state,tdragonb)
{
	decode_tdragonb();
}

DRIVER_INIT_MEMBER(nmk16_state,tdragon_prot)
{
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	//rom[0x94b0/2] = 0; /* Patch out JMP to shared memory (protection) */
	//rom[0x94b2/2] = 0x92f4;

	// kludge the sound communication to let commands go through.
	rom[0x048a/2] = 0x4e71;
	rom[0x04aa/2] = 0x4e71;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0b0000, 0x0bffff, write16_delegate(FUNC(nmk16_state::tdragon_mainram_w),this));
}

DRIVER_INIT_MEMBER(nmk16_state,ssmissin)
{
	decode_ssmissin();

	membank("okibank1")->configure_entries(0, 4, memregion("oki1")->base() + 0x80000, 0x20000);
}

DRIVER_INIT_MEMBER(nmk16_state,bjtwin)
{
	/* Patch rom to enable test mode */

/*  008F54: 33F9 0008 0000 000F FFFC move.w  $80000.l, $ffffc.l
 *  008F5E: 3639 0008 0002           move.w  $80002.l, D3
 *  008F64: 3003                     move.w  D3, D0             \
 *  008F66: 3203                     move.w  D3, D1             |   This code remaps
 *  008F68: 0041 BFBF                ori.w   #-$4041, D1        |   buttons 2 and 3 to
 *  008F6C: E441                     asr.w   #2, D1             |   button 1, so
 *  008F6E: 0040 DFDF                ori.w   #-$2021, D0        |   you can't enter
 *  008F72: E240                     asr.w   #1, D0             |   service mode
 *  008F74: C640                     and.w   D0, D3             |
 *  008F76: C641                     and.w   D1, D3             /
 *  008F78: 33C3 000F FFFE           move.w  D3, $ffffe.l
 *  008F7E: 207C 000F 9000           movea.l #$f9000, A0
 */
#if 0
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();
	rom[0x09172/2] = 0x6006;    /* patch checksum error */
	rom[0x08f74/2] = 0x4e71;
#endif

	DRIVER_INIT_CALL(nmk);

}

/* NO NMK004, it has a PIC instead */
READ16_MEMBER(nmk16_state::vandykeb_r){ return 0x0000; }
DRIVER_INIT_MEMBER(nmk16_state,vandykeb)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x08000e, 0x08000f, read16_delegate(FUNC(nmk16_state::vandykeb_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0x08001e, 0x08001f);
}


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

READ16_MEMBER(nmk16_state::afega_unknown_r)
{
	/* This fixes the text in Service Mode. */
	return 0x0100;
}



WRITE16_MEMBER(nmk16_state::afega_scroll0_w)
{
	COMBINE_DATA(&m_afega_scroll_0[offset]);
}

WRITE16_MEMBER(nmk16_state::afega_scroll1_w)
{
	COMBINE_DATA(&m_afega_scroll_1[offset]);
}

/*
 Lines starting with an empty comment in the following MemoryReadAddress
 arrays are there for debug (e.g. the game does not read from those ranges
 AFAIK)
*/


static ADDRESS_MAP_START( afega, AS_PROGRAM, 16, nmk16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")            // Buttons
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")            // P1 + P2
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("DSW1")           // 2 x DSW
	AM_RANGE(0x080012, 0x080013) AM_READ(afega_unknown_r)
	AM_RANGE(0x080000, 0x08001d) AM_WRITEONLY               //
	AM_RANGE(0x08001e, 0x08001f) AM_WRITE(afega_soundlatch_w)   // To Sound CPU
/**/AM_RANGE(0x084000, 0x084003) AM_RAM_WRITE(afega_scroll0_w)  // Scroll on redhawkb (mirror or changed?..)
/**/AM_RANGE(0x084004, 0x084007) AM_RAM_WRITE(afega_scroll1_w)  // Scroll on redhawkb (mirror or changed?..)
	AM_RANGE(0x080020, 0x087fff) AM_WRITEONLY               //
/**/AM_RANGE(0x088000, 0x0885ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0x088600, 0x08bfff) AM_WRITEONLY               //
/**/AM_RANGE(0x08c000, 0x08c003) AM_RAM_WRITE(afega_scroll0_w) AM_SHARE("afega_scroll_0")   // Scroll
/**/AM_RANGE(0x08c004, 0x08c007) AM_RAM_WRITE(afega_scroll1_w) AM_SHARE("afega_scroll_1")   //
	AM_RANGE(0x08c008, 0x08ffff) AM_WRITEONLY               //
/**/AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")    // Layer 0                  // ?
/**/AM_RANGE(0x09c000, 0x09c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")  // Layer 1

	AM_RANGE(0x0c0000, 0x0cffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END

// firehawk has 0x100000 bytes of program rom (at least the switchable version) so the above can't work.
static ADDRESS_MAP_START( firehawk_map, AS_PROGRAM, 16, nmk16_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x280000, 0x280001) AM_READ_PORT("IN0")            // Buttons
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN1")            // P1 + P2
	AM_RANGE(0x280004, 0x280005) AM_READ_PORT("DSW1")           // 2 x DSW
	AM_RANGE(0x280012, 0x280013) AM_READ(afega_unknown_r)
	AM_RANGE(0x280000, 0x28001d) AM_WRITEONLY               //
	AM_RANGE(0x28001e, 0x28001f) AM_WRITE(afega_soundlatch_w)   // To Sound CPU
/**/AM_RANGE(0x284000, 0x284003) AM_RAM_WRITE(afega_scroll0_w)  // Scroll on redhawkb (mirror or changed?..)
/**/AM_RANGE(0x284004, 0x284007) AM_RAM_WRITE(afega_scroll1_w)  // Scroll on redhawkb (mirror or changed?..)
	AM_RANGE(0x280020, 0x287fff) AM_WRITEONLY               //
/**/AM_RANGE(0x288000, 0x2885ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0x288600, 0x28bfff) AM_WRITEONLY               //
/**/AM_RANGE(0x28c000, 0x28c003) AM_RAM_WRITE(afega_scroll0_w) AM_SHARE("afega_scroll_0")   // Scroll
/**/AM_RANGE(0x28c004, 0x28c007) AM_RAM_WRITE(afega_scroll1_w) AM_SHARE("afega_scroll_1")   //
	AM_RANGE(0x28c008, 0x28ffff) AM_WRITEONLY               //
/**/AM_RANGE(0x290000, 0x293fff) AM_RAM_WRITE(nmk_bgvideoram0_w) AM_SHARE("nmk_bgvideoram0")    // Layer 0                  // ?
/**/AM_RANGE(0x29c000, 0x29c7ff) AM_RAM_WRITE(nmk_txvideoram_w) AM_SHARE("nmk_txvideoram")  // Layer 1

	AM_RANGE(0x3c0000, 0x3cffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
	AM_RANGE(0x3f0000, 0x3fffff) AM_RAM_WRITE(nmk16_mainram_strange_w) AM_SHARE("mainram")
ADDRESS_MAP_END


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/
WRITE8_MEMBER(nmk16_state::spec2k_oki1_banking_w)
{
	if(data == 0xfe)
		m_oki2->set_bank_base(0);
	else if(data == 0xff)
		m_oki2->set_bank_base(0x40000);
}

static ADDRESS_MAP_START( afega_sound_cpu, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0003, 0x0003) AM_WRITENOP // bug in sound prg?
	AM_RANGE(0x0004, 0x0004) AM_WRITENOP // bug in sound prg?
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM                                 // RAM
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_byte_r)                 // From Main CPU
	AM_RANGE(0xf808, 0xf809) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)   // YM2151
	AM_RANGE(0xf80a, 0xf80a) AM_DEVREADWRITE("oki1", okim6295_device, read, write)      // M6295
ADDRESS_MAP_END

static ADDRESS_MAP_START( firehawk_sound_cpu, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xfff0, 0xfff0) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xfff2, 0xfff2) AM_WRITE(spec2k_oki1_banking_w )
	AM_RANGE(0xfff8, 0xfff8) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0xfffa, 0xfffa) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0xf800, 0xffff) AM_RAM // not used, only tested
ADDRESS_MAP_END


WRITE8_MEMBER(nmk16_state::twinactn_oki_bank_w)
{
	m_oki1->set_bank_base((data & 3) * 0x40000);

	if (data & (~3))
		logerror("%s: invalid oki bank %02x\n", machine().describe_context(), data);

//  logerror("%04x: oki bank %02x\n", space.device().safe_pc(), data);
}

static ADDRESS_MAP_START( twinactn_sound_cpu, AS_PROGRAM, 8, nmk16_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(twinactn_oki_bank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)     // From Main CPU
ADDRESS_MAP_END

/***************************************************************************


                            Graphics Layouts


***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1)    },
	{ STEP8(0,4)    },
	{ STEP8(0,8*4)  },
	8*8*4
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1)    },
	{ STEP8(0,4),   STEP8(8*8*4*2,4)    },
	{ STEP8(0,8*4), STEP8(8*8*4*1,8*4)  },
	16*16*4
};

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ STEP4(RGN_FRAC(0,2),1), STEP4(RGN_FRAC(1,2),1)    },
	{ STEP8(0,4),   STEP8(8*8*4*2,4)    },
	{ STEP8(0,8*4), STEP8(8*8*4*1,8*4)  },
	16*16*4
};


static const gfx_layout layout_16x16x4_swapped =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1)    },
	{ 4,0,12,8,20,16,28,24, 512+4,512+0,512+12,512+8,512+20,512+16,512+28,512+24},
	{ STEP8(0,8*4), STEP8(8*8*4*1,8*4)  },
	16*16*4
};

static GFXDECODE_START( grdnstrm )
	GFXDECODE_ENTRY( "fgtile", 0, layout_8x8x4,   256*2, 16 ) // [2] Layer 1
	GFXDECODE_ENTRY( "bgtile", 0, layout_16x16x8, 256*0, 1 ) // [1] Layer 0
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 256*1, 16 ) // [0] Sprites
GFXDECODE_END

static GFXDECODE_START( stagger1 )
	GFXDECODE_ENTRY( "fgtile", 0, layout_8x8x4,   256*2, 16 ) // [2] Layer 1
	GFXDECODE_ENTRY( "bgtile", 0, layout_16x16x4, 256*0, 16 ) // [1] Layer 0
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 256*1, 16 ) // [0] Sprites
GFXDECODE_END

static GFXDECODE_START( redhawkb )
	GFXDECODE_ENTRY( "fgtile", 0, layout_8x8x4,           256*2, 16 ) // [2] Layer 1
	GFXDECODE_ENTRY( "bgtile", 0, layout_16x16x4_swapped, 256*0, 16 ) // [1] Layer 0
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4_swapped, 256*1, 16 ) // [0] Sprites
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_CONFIG_START( stagger1, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,XTAL_12MHz) /* 68000p10 running at 12mhz, verified on pcb */
	MCFG_CPU_PROGRAM_MAP(afega)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_4MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(afega_sound_cpu)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_afega)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", stagger1)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,afega)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_4MHz) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.30)

	MCFG_OKIM6295_ADD("oki1", XTAL_4MHz/4, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.70)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( redhawki, stagger1 )

	/* basic machine hardware */
	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_redhawki)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( redhawkb, stagger1 )

	/* basic machine hardware */
	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", redhawkb)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_redhawkb)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( grdnstrm, stagger1 )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", grdnstrm)
	MCFG_VIDEO_START_OVERRIDE(nmk16_state,firehawk)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_firehawk)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( grdnstrmk, stagger1 ) /* Side by side with PCB, the music seems too fast as well */

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(57) /* Side by side with PCB, MAME is too fast at 56 */
	MCFG_GFXDECODE_MODIFY("gfxdecode", grdnstrm)
	MCFG_VIDEO_START_OVERRIDE(nmk16_state,grdnstrm)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( popspops, grdnstrm )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_bubl2000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( firehawk, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,12000000)
	MCFG_CPU_PROGRAM_MAP(firehawk_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(firehawk_sound_cpu)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_firehawk)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", grdnstrm)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,firehawk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( spec2k, firehawk )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(afega)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( twinactn, nmk16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,12000000)
	MCFG_CPU_PROGRAM_MAP(twinactn_map)
	NMK_HACKY_INTERRUPT_TIMING

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(twinactn_sound_cpu)

	/* video hardware */
	NMK_HACKY_SCREEN_LOWRES
	MCFG_SCREEN_UPDATE_DRIVER(nmk16_state, screen_update_macross)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macross)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(nmk16_state,macross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/* Address lines scrambling */

static void decryptcode( running_machine &machine, int a23, int a22, int a21, int a20, int a19, int a18, int a17, int a16, int a15, int a14, int a13, int a12,
	int a11, int a10, int a9, int a8, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0 )
{
	int i;
	UINT8 *RAM = machine.root_device().memregion( "maincpu" )->base();
	size_t  size = machine.root_device().memregion( "maincpu" )->bytes();
	dynamic_buffer buffer( size );

	memcpy( &buffer[0], RAM, size );
	for( i = 0; i < size; i++ )
	{
		RAM[ i ] = buffer[ BITSWAP24( i, a23, a22, a21, a20, a19, a18, a17, a16, a15, a14, a13, a12,
			a11, a10, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0 ) ];
	}
}



ROM_START( vandyke )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "vdk-1.16",  0x00000, 0x20000, CRC(c1d01c59) SHA1(04a7fd31ca4d87d078070390660edf08bf1d96b5) )
	ROM_LOAD16_BYTE( "vdk-2.15",  0x00001, 0x20000, CRC(9d741cc2) SHA1(2d101044fba5fc5b7d63869a0a053c42fdc2598b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "vdk-4.127",    0x00000, 0x10000, CRC(eba544f0) SHA1(36f6d048d15a392542a9220a244d8a7049aaff8b) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "vdk-3.222",      0x000000, 0x010000, CRC(5a547c1b) SHA1(2d61f51ce2f91ebf0053ce3a00911d1bcbaba816) )  /* 8x8 tiles */

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "vdk-01.13",      0x000000, 0x080000, CRC(195a24be) SHA1(3a20dd746a87efc5c1fdc5025b709efeff82e05e) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "vdk-07.202",  0x000000, 0x080000, CRC(42d41f06) SHA1(69fd1d38187b8081f65acea2424bc1a0d455d90c) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-06.203",  0x000001, 0x080000, CRC(d54722a8) SHA1(47f8e97b29ae0ff1a1d7d50734e4219a87a2ed57) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-04.2-1",  0x100000, 0x080000, CRC(0a730547) SHA1(afac0549eb86d1fab5ca8ae2a0dad14144f55c02) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-05.3-1",  0x100001, 0x080000, CRC(ba456d27) SHA1(5485a560ae2c2c8b6fdec314393c02a3de758ef3) )  /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-02.126",     0x000000, 0x080000, CRC(b2103274) SHA1(6bbdc912393607cd5306be946327c5ea0178c7a6) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-03.165",     0x000000, 0x080000, CRC(631776d3) SHA1(ffd76e5b03130252c55eaa6ae7edfee5632dae73) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic100.bpr", 0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) )   /* V-sync hw (unused) */
	ROM_LOAD( "ic101.bpr", 0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )   /* H-sync hw (unused) */
ROM_END

ROM_START( vandykejal )
	ROM_REGION( 0x40000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_BYTE( "vdk-1.16",   0x00000, 0x20000, CRC(c1d01c59) SHA1(04a7fd31ca4d87d078070390660edf08bf1d96b5) )
	ROM_LOAD16_BYTE( "jaleco2.15", 0x00001, 0x20000, CRC(170e4d2e) SHA1(6009d19d30e345fea93e039d165061e2b20ff058) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "vdk-4.127",    0x00000, 0x10000, CRC(eba544f0) SHA1(36f6d048d15a392542a9220a244d8a7049aaff8b) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "vdk-3.222",      0x000000, 0x010000, CRC(5a547c1b) SHA1(2d61f51ce2f91ebf0053ce3a00911d1bcbaba816) )  /* 8x8 tiles */

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "vdk-01.13",      0x000000, 0x080000, CRC(195a24be) SHA1(3a20dd746a87efc5c1fdc5025b709efeff82e05e) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "vdk-07.202",  0x000000, 0x080000, CRC(42d41f06) SHA1(69fd1d38187b8081f65acea2424bc1a0d455d90c) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-06.203",  0x000001, 0x080000, CRC(d54722a8) SHA1(47f8e97b29ae0ff1a1d7d50734e4219a87a2ed57) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-04.2-1",  0x100000, 0x080000, CRC(0a730547) SHA1(afac0549eb86d1fab5ca8ae2a0dad14144f55c02) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-05.3-1",  0x100001, 0x080000, CRC(ba456d27) SHA1(5485a560ae2c2c8b6fdec314393c02a3de758ef3) )  /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-02.126",     0x000000, 0x080000, CRC(b2103274) SHA1(6bbdc912393607cd5306be946327c5ea0178c7a6) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-03.165",     0x000000, 0x080000, CRC(631776d3) SHA1(ffd76e5b03130252c55eaa6ae7edfee5632dae73) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic100.bpr", 0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) )   /* V-sync hw (unused) */
	ROM_LOAD( "ic101.bpr", 0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )   /* H-sync hw (unused) */
ROM_END

ROM_START( vandykejal2 )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "vdk-even.16",  0x00000, 0x20000, CRC(cde05a84) SHA1(dab5981d7dad9abe86cfe011da8ca0b11d484a3f) ) /* Hand written labels, dated 2/12 */
	ROM_LOAD16_BYTE( "vdk-odd.15",   0x00001, 0x20000, CRC(0f6fea40) SHA1(3acbe72c251d51b028d8c66274263a2b39b042ea) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "vdk-4.127",    0x00000, 0x10000, CRC(eba544f0) SHA1(36f6d048d15a392542a9220a244d8a7049aaff8b) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "vdk-3.222",      0x000000, 0x010000, CRC(5a547c1b) SHA1(2d61f51ce2f91ebf0053ce3a00911d1bcbaba816) )  /* 8x8 tiles */

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "vdk-01.13",      0x000000, 0x080000, CRC(195a24be) SHA1(3a20dd746a87efc5c1fdc5025b709efeff82e05e) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "vdk-07.202",  0x000000, 0x080000, CRC(42d41f06) SHA1(69fd1d38187b8081f65acea2424bc1a0d455d90c) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-06.203",  0x000001, 0x080000, CRC(d54722a8) SHA1(47f8e97b29ae0ff1a1d7d50734e4219a87a2ed57) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-04.2-1",  0x100000, 0x080000, CRC(0a730547) SHA1(afac0549eb86d1fab5ca8ae2a0dad14144f55c02) )  /* Sprites */
	ROM_LOAD16_BYTE( "vdk-05.3-1",  0x100001, 0x080000, CRC(ba456d27) SHA1(5485a560ae2c2c8b6fdec314393c02a3de758ef3) )  /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-02.126",     0x000000, 0x080000, CRC(b2103274) SHA1(6bbdc912393607cd5306be946327c5ea0178c7a6) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "vdk-03.165",     0x000000, 0x080000, CRC(631776d3) SHA1(ffd76e5b03130252c55eaa6ae7edfee5632dae73) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic100.bpr", 0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) )   /* V-sync hw (unused) */
	ROM_LOAD( "ic101.bpr", 0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )   /* H-sync hw (unused) */
ROM_END

ROM_START( vandykeb )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "2.bin",  0x00000, 0x20000, CRC(9c269702) SHA1(831ff9d499aa94d85f62b8613477a95f00f62b34) )
	ROM_LOAD16_BYTE( "1.bin",  0x00001, 0x20000, CRC(dd6303a1) SHA1(3c225ff1696adc1af05b1b36d8cf1f220181861c) )

	ROM_REGION(0x10000, "mcu", 0 ) /* PIC is read protected */
	ROM_LOAD( "pic16c57",    0x00000, 0x2d4c, BAD_DUMP CRC(bdb3920d) SHA1(2ef8d2aa3817cebea8e2443bc995cec3a3f88835) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "3.bin",      0x000000, 0x010000, CRC(5a547c1b) SHA1(2d61f51ce2f91ebf0053ce3a00911d1bcbaba816) )  /* 8x8 tiles */

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "4.bin",      0x000000, 0x040000, CRC(4ba4138d) SHA1(56f9c9422085eaf74ddec8977663a33c122b7e8b) )  /* 16x16 tiles */
	ROM_LOAD( "5.bin",      0x040000, 0x040000, CRC(9a1ac697) SHA1(a8200b10606edf4578c7e2f53a0046bb1209a041) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "13.bin",  0x000000, 0x040000, CRC(bb561871) SHA1(33dcaf956112181eed531320d3ececb90b17a599) )  /* Sprites */
	ROM_LOAD16_BYTE( "17.bin",  0x000001, 0x040000, CRC(346e3b66) SHA1(34df7167ed4048e1f236e7d8fa6dcdffb0965c71) )  /* Sprites */
	ROM_LOAD16_BYTE( "12.bin",  0x080000, 0x040000, CRC(cdef9b17) SHA1(ec024a21685b87c82dc574cd050118d856a3cf57) )  /* Sprites */
	ROM_LOAD16_BYTE( "16.bin",  0x080001, 0x040000, CRC(beda678c) SHA1(3dfb8763241a97b9d65113c6eb99b52ec5245cd6) )  /* Sprites */
	ROM_LOAD16_BYTE( "11.bin",  0x100000, 0x020000, CRC(823185d9) SHA1(eaf0f3ab0921d894eb1d09d5b2e9d5b785928804) )  /* Sprites */
	ROM_LOAD16_BYTE( "15.bin",  0x100001, 0x020000, CRC(149f3247) SHA1(5f515cb10468da048c89b543807280bd3e39e45a) )  /* Sprites */
	ROM_LOAD16_BYTE( "10.bin",  0x140000, 0x020000, CRC(388b1abc) SHA1(9d1c43070130672a5e1a41807d796c944b0676ae) )  /* Sprites */
	ROM_LOAD16_BYTE( "14.bin",  0x140001, 0x020000, CRC(32eeba37) SHA1(0d0218e864ed647bd33bbe379f0ef76ccefbd06c) )  /* Sprites */

	ROM_REGION( 0x0a0000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "9.bin",      0x000000, 0x020000, CRC(56bf774f) SHA1(5ece618fff22483adb5dff062dd4ec212aab0f01) )
	ROM_LOAD( "8.bin",      0x000000, 0x020000, CRC(89851fcf) SHA1(7b6284cb929059371dd2b5410cd18373834ba76b) )
	ROM_LOAD( "7.bin",      0x000000, 0x020000, CRC(d7bf0f6a) SHA1(413713576692676a831949e0d4dc5574da338380) )
	ROM_LOAD( "6.bin",      0x000000, 0x020000, CRC(a7fcf709) SHA1(dc6298b43a472e92e99b8286bd4d26f7e72fd278) )
ROM_END

ROM_START( tharrier )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin" ,   0x00000, 0x20000, CRC(f3887a44) SHA1(4e5b660d33ba1d1e00263030efa67e2db376a234) )
	ROM_LOAD16_BYTE( "3.bin" ,   0x00001, 0x20000, CRC(65c247f6) SHA1(9f35f2b6f54814b4c4d23e2d78db8043e678fef2) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "12" ,   0x00000, 0x10000, CRC(b959f837) SHA1(073b14935e7d5b0cad19a3471fd26e9e3a363827) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1.bin" ,        0x000000, 0x10000, CRC(005c26c3) SHA1(ee88d8f956b9b0a8ba5fb49c5c05f6ed6f01729c) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "89050-4" ,  0x000000, 0x80000, CRC(64d7d687) SHA1(dcfeac71fd577439e31cc1186b720388fbdc6ca0) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "89050-13",    0x000000, 0x80000, CRC(24db3fa4) SHA1(e0d76c479dfcacf03c04ec4760caecf3fd1e2ff7) )   /* Sprites */
	ROM_LOAD16_BYTE( "89050-17",    0x000001, 0x80000, CRC(7f715421) SHA1(bde5e0e1e22519e51ca0fd806909e90cc5b1c5b8) )

	ROM_REGION(0x80000, "oki1", 0 ) /* Oki sample data */
	ROM_LOAD( "89050-8",     0x00000, 0x80000, CRC(11ee4c39) SHA1(163295c385cff963a5bf87dc3e7bef6019e10ba8) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION(0x80000, "oki2", 0 ) /* Oki sample data */
	ROM_LOAD( "89050-10",    0x00000, 0x80000, CRC(893552ab) SHA1(b0a34291f4e482858ed295203ae031b17c2dbabc) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "21.bpr",  0x00000, 0x100, CRC(fcd5efea) SHA1(cbda6b14127dabd1788cc256743cf62efaa5e8c4) )
	ROM_LOAD( "22.bpr",  0x00000, 0x100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )
	ROM_LOAD( "23.bpr",  0x00000, 0x020, CRC(fc3569f4) SHA1(e1c498085e4ae9d0a995c94530544b0a5b760fbf) )
	ROM_LOAD( "24.bpr",  0x00000, 0x100, CRC(e0a009fe) SHA1(a66a27bb405d4ff8e4c0062273ee9b11e76ee520) )
	ROM_LOAD( "25.bpr",  0x00000, 0x100, CRC(e0a009fe) SHA1(a66a27bb405d4ff8e4c0062273ee9b11e76ee520) ) /* same as 24.bin */
	ROM_LOAD( "26.bpr",  0x00120, 0x020, CRC(0cbfb33e) SHA1(5dfee031a0a14bcd667fe2af2fa9cdfac3941d22) )
ROM_END

ROM_START( tharrieru )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2" ,   0x00000, 0x20000, CRC(78923aaa) SHA1(28338f49581180604403e1bd200f524fc4cb8b9f) )
	ROM_LOAD16_BYTE( "3" ,   0x00001, 0x20000, CRC(99cea259) SHA1(75abfb08b2358dd13809ade5a2dfffeb8b8df82c) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "12" ,   0x00000, 0x10000, CRC(b959f837) SHA1(073b14935e7d5b0cad19a3471fd26e9e3a363827) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1" ,        0x000000, 0x10000, CRC(c7402e4a) SHA1(25cade2f8d4784887f0f51beb48b1e6b695629c2) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "89050-4" ,  0x000000, 0x80000, CRC(64d7d687) SHA1(dcfeac71fd577439e31cc1186b720388fbdc6ca0) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "89050-13",    0x000000, 0x80000, CRC(24db3fa4) SHA1(e0d76c479dfcacf03c04ec4760caecf3fd1e2ff7) )   /* Sprites */
	ROM_LOAD16_BYTE( "89050-17",    0x000001, 0x80000, CRC(7f715421) SHA1(bde5e0e1e22519e51ca0fd806909e90cc5b1c5b8) )

	ROM_REGION(0x80000, "oki1", 0 ) /* Oki sample data */
	ROM_LOAD( "89050-8",     0x00000, 0x80000, CRC(11ee4c39) SHA1(163295c385cff963a5bf87dc3e7bef6019e10ba8) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION(0x80000, "oki2", 0 ) /* Oki sample data */
	ROM_LOAD( "89050-10",    0x00000, 0x80000, CRC(893552ab) SHA1(b0a34291f4e482858ed295203ae031b17c2dbabc) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "21.bpr",  0x00000, 0x100, CRC(fcd5efea) SHA1(cbda6b14127dabd1788cc256743cf62efaa5e8c4) )
	ROM_LOAD( "22.bpr",  0x00000, 0x100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )
	ROM_LOAD( "23.bpr",  0x00000, 0x020, CRC(fc3569f4) SHA1(e1c498085e4ae9d0a995c94530544b0a5b760fbf) )
	ROM_LOAD( "24.bpr",  0x00000, 0x100, CRC(e0a009fe) SHA1(a66a27bb405d4ff8e4c0062273ee9b11e76ee520) )
	ROM_LOAD( "25.bpr",  0x00000, 0x100, CRC(e0a009fe) SHA1(a66a27bb405d4ff8e4c0062273ee9b11e76ee520) ) /* same as 24.bin */
	ROM_LOAD( "26.bpr",  0x00120, 0x020, CRC(0cbfb33e) SHA1(5dfee031a0a14bcd667fe2af2fa9cdfac3941d22) )
ROM_END

ROM_START( mustang )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin",    0x00000, 0x20000, CRC(bd9f7c89) SHA1(a0af46a8ff82b90bece2515e1bd74e7a7ddf5379) )
	ROM_LOAD16_BYTE( "3.bin",    0x00001, 0x20000, CRC(0eec36a5) SHA1(c549fbcd3e2741a6d0f2633ded6a85909d37f633) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "90058-7",    0x00000, 0x10000, CRC(920a93c8) SHA1(7660ca419e2fd98848ae7f5994994eaed023151e) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "90058-1",    0x00000, 0x20000, CRC(81ccfcad) SHA1(70a0f769c0d4588f6f17bd52cc86a745f30e9f00) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "90058-4",    0x000000, 0x80000, CRC(a07a2002) SHA1(55720d84a251c33c52ae8c33aa41ff8ac9727941) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "90058-8",    0x00000, 0x80000, CRC(560bff04) SHA1(b005642adc81d878971ecbdead8ef5e604c90ae2) )
	ROM_LOAD16_BYTE( "90058-9",    0x00001, 0x80000, CRC(b9d72a03) SHA1(43ee9def1b6c491c6832562d66c1af54d81d9b3c) )

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90058-5",    0x00000, 0x80000, CRC(c60c883e) SHA1(8a01950cad820b2e781ec81cd12737829edc4f19) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90058-6",    0x00000, 0x80000, CRC(233c1776) SHA1(7010a2f914611698a65bf4f22bc1753a9ed26277) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "10.bpr",  0x00000, 0x100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) ) /* unknown */
	ROM_LOAD( "90058-11",  0x00100, 0x100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
ROM_END

ROM_START( mustangs )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "90058-2",    0x00000, 0x20000, CRC(833aa458) SHA1(a9924f7044397e3a36c674b064173ffae80a79ec) )
	ROM_LOAD16_BYTE( "90058-3",    0x00001, 0x20000, CRC(e4b80f06) SHA1(ce589cebb5ea85c89eb44796b821a4bd0c44b9a8) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "90058-7",    0x00000, 0x10000, CRC(920a93c8) SHA1(7660ca419e2fd98848ae7f5994994eaed023151e) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "90058-1",    0x00000, 0x20000, CRC(81ccfcad) SHA1(70a0f769c0d4588f6f17bd52cc86a745f30e9f00) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "90058-4",    0x000000, 0x80000, CRC(a07a2002) SHA1(55720d84a251c33c52ae8c33aa41ff8ac9727941) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "90058-8",    0x00000, 0x80000, CRC(560bff04) SHA1(b005642adc81d878971ecbdead8ef5e604c90ae2) )
	ROM_LOAD16_BYTE( "90058-9",    0x00001, 0x80000, CRC(b9d72a03) SHA1(43ee9def1b6c491c6832562d66c1af54d81d9b3c) )

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90058-5",    0x00000, 0x80000, CRC(c60c883e) SHA1(8a01950cad820b2e781ec81cd12737829edc4f19) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90058-6",    0x00000, 0x80000, CRC(233c1776) SHA1(7010a2f914611698a65bf4f22bc1753a9ed26277) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "90058-10",  0x00000, 0x100, CRC(de156d99) SHA1(07b70deca74e23bab7c13e5e9aee32d0dbb06509) ) /* unknown */
	ROM_LOAD( "90058-11",  0x00100, 0x100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
ROM_END

ROM_START( mustangb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mustang.14",    0x00000, 0x20000, CRC(13c6363b) SHA1(e2c1985d1c8ec9751c47cd7e1b85e007f3aeb6fd) )
	ROM_LOAD16_BYTE( "mustang.13",    0x00001, 0x20000, CRC(d8ccce31) SHA1(e8e3e34a480fcd298f11833c6c968c5df77c0e2a) )

	ROM_REGION(0x20000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "mustang.16",    0x00000, 0x8000, CRC(99ee7505) SHA1(b97c8ee5e26e8554b5de506fba3b32cc2fde53c9) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "90058-1",    0x00000, 0x20000, CRC(81ccfcad) SHA1(70a0f769c0d4588f6f17bd52cc86a745f30e9f00) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "90058-4",    0x000000, 0x80000, CRC(a07a2002) SHA1(55720d84a251c33c52ae8c33aa41ff8ac9727941) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "90058-8",    0x00000, 0x80000, CRC(560bff04) SHA1(b005642adc81d878971ecbdead8ef5e604c90ae2) )
	ROM_LOAD16_BYTE( "90058-9",    0x00001, 0x80000, CRC(b9d72a03) SHA1(43ee9def1b6c491c6832562d66c1af54d81d9b3c) )

	ROM_REGION( 0x040000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "mustang.17",    0x00000, 0x10000, CRC(f6f6c4bf) SHA1(ea4cf74d968e254ae47c16c2f4c2f4bc1a528808) )
ROM_END

ROM_START( mustangb2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "05.bin",    0x00000, 0x20000, CRC(13c6363b) SHA1(e2c1985d1c8ec9751c47cd7e1b85e007f3aeb6fd) )  /* bootleg manufacturered by TAB AUSTRIA */
	ROM_LOAD16_BYTE( "04.bin",    0x00001, 0x20000, CRC(0d06f723) SHA1(28d5899114746d186e1ddd207deb177b31ff614d) )

	ROM_REGION(0x20000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "01.bin",    0x00000, 0x8000, CRC(90820499) SHA1(ddd43373eb1891a05159085b52bf74760824e5aa) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "06.bin",    0x00000, 0x20000, CRC(81ccfcad) SHA1(70a0f769c0d4588f6f17bd52cc86a745f30e9f00) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_LOAD( "07.bin",    0x00000, 0x20000, CRC(5f8fdfb1) SHA1(529494a317409da978d44610682ef56ebc24e0af) )
	ROM_LOAD( "10.bin",    0x20000, 0x20000, CRC(39757d6a) SHA1(71acf748c752df70f437b3ffa759d68d283c22cf) )
	ROM_LOAD( "08.bin",    0x40000, 0x20000, CRC(b3dd5243) SHA1(38b71dad7d392319ecef690fb230fa9ca46c7d0a) )
	ROM_LOAD( "09.bin",    0x60000, 0x20000, CRC(c6c9752f) SHA1(41a3581af7a10eab9eb15580760a99d27e67f085) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "18.bin",    0x00000, 0x20000, CRC(d13f0722) SHA1(3e9c0a3e124f8b2616bb4a39d2d3fb25623b8c85) )
	ROM_LOAD16_BYTE( "13.bin",    0x00001, 0x20000, CRC(54773f95) SHA1(2c57f54efa069907dfb59f15fbc2c580180df3cc) )
	ROM_LOAD16_BYTE( "17.bin",    0x40000, 0x20000, CRC(87c1fb43) SHA1(e874ab8aba448b002f64197dacb5d6c47fb83af2) )
	ROM_LOAD16_BYTE( "14.bin",    0x40001, 0x20000, CRC(932d3e33) SHA1(a784f288fa99e605a0bf396bc7694319980d1cd1) )
	ROM_LOAD16_BYTE( "16.bin",    0x80000, 0x20000, CRC(23d03ad5) SHA1(2cde1accd1d97ce9ea3d0ef24ae4d54e04b8f12f) )
	ROM_LOAD16_BYTE( "15.bin",    0x80001, 0x20000, CRC(a62b2f87) SHA1(bcffc6d10bed84c509e5cb57125d08127ab2c89d) )
	ROM_LOAD16_BYTE( "12.bin",    0xc0000, 0x20000, CRC(42a6cfc2) SHA1(46fc3b30a50efc94613e3b34aaf0543fa4cdc919) )
	ROM_LOAD16_BYTE( "11.bin",    0xc0001, 0x20000, CRC(9d3bee66) SHA1(e8db57b9a5581d3d54e69bb7ba229a49a7cc224f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "02.bin",    0x00000, 0x10000, CRC(f6f6c4bf) SHA1(ea4cf74d968e254ae47c16c2f4c2f4bc1a528808) )
ROM_END

ROM_START( acrobatm )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "02_ic100.bin",    0x00000, 0x20000, CRC(3fe487f4) SHA1(29aba5debcfddff14e584a1c7c5a403e85fc6ec0) )
	ROM_LOAD16_BYTE( "01_ic101.bin",    0x00001, 0x20000, CRC(17175753) SHA1(738865744badb78a0414ff650a94b97e516d0ea0) )

	ROM_REGION( 0x20000, "fgtile", 0 )
	ROM_LOAD( "03_ic79.bin",   0x000000, 0x10000, CRC(d86c186e) SHA1(2e263d4780f2ba7acc7faa88472c85216fbae6a3) ) /* Characters */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "09_ic8.bin",  0x000000, 0x100000, CRC(7c12afed) SHA1(ae793e41599355a126cbcce91cd2c9f212d21853) ) /* Foreground */

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "07_ic42.bin",  0x000000, 0x100000, CRC(5672bdaa) SHA1(5401a104d72904de19b73125451767bc63d36809) ) /* Sprites */
	ROM_LOAD( "08_ic29.bin",  0x100000, 0x080000, CRC(b4c0ace3) SHA1(5d638781d588cfbf4025d002d5a2309049fe1ee5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "04_ic74.bin",    0x00000, 0x10000, CRC(176905fb) SHA1(135a184f44bedd93b293b9124fa0bd725e0ee93b) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "05_ic54.bin",    0x00000, 0x80000, CRC(3b8c2b0e) SHA1(72491da32512823540b67dc5027f21c74af08c7d) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x80000, "oki2", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "06_ic53.bin",    0x00000, 0x80000, CRC(c1517cd4) SHA1(5a91ddc608c7a6fbdd9f93e503d39eac02ef04a4) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "10_ic81.bin",    0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )  /* unknown */
	ROM_LOAD( "11_ic80.bin",    0x0100, 0x0100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) )  /* unknown */
ROM_END

/*

S.B.S. Gomorrah (and Bio-ship Paladin with correct roms in place)
UPL, 1993

PCB Layout
----------

UPL-90062
|-------------------------------------------------------------------------|
| LA4460  4558  YM2203   6116       NMK004                   68000        |
|         3014  M6295    6.IC120                                          |
|               M6295               8MHz                 62256  10.IC15   |
|        SBS-G_05.IC160                                                   |
|               SBS-G_04.IC139                           62256  11.IC14   |
|DSW2   DSW1             12MHz                                            |
|                                   82S129.IC69                           |
|    NMK005                                                               |
|                           82S135.IC94                                   |
|                                   NMK902                                |
|J        6116                             NMK903     NMK901              |
|A                                                                        |
|M        6116                      7.IC46                                |
|M           82S123.IC154     6116                 6264           NMK903  |
|A                                                                        |
|                     6116                         6264                   |
|                             6116                            SBS-G_01.IC9|
|                     6116                                                |
|                                                                         |
|                                                  NMK901                 |
|                                                                         |
|                                                                 NMK903  |
|                                                                         |
|                            62256    62256        8.IC27     SBS-G_02.IC4|
|                                                                         |
|                            62256    62256        9.IC26                 |
|SBS-G_03.IC194                                                    10MHz  |
|-------------------------------------------------------------------------|
Notes:
      680000 @ 10.0MHz
      YM2203 @ 1.5MHz [12/8]
      M6295 @ 4.0MHz [12/3], pin 7 HIGH
      VSync 60Hz
      HSync 15.27kHz

*/

ROM_START( bioship )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.ic14",    0x00000, 0x20000, CRC(acf56afb) SHA1(0e8ec494ab406cfee24cf586059878332265de75) )
	ROM_LOAD16_BYTE( "1.ic15",    0x00001, 0x20000, CRC(820ef303) SHA1(d2ef29557b05abf8ae79a2c7ce0d15a91b36eeff) )

	ROM_REGION( 0x20000, "fgtile", 0 )
	ROM_LOAD( "7",         0x000000, 0x10000, CRC(2f3f5a10) SHA1(c1006eb755eec75f69dc7972d78d0c59088eb140) ) /* Characters */

	ROM_REGION( 0x80000, "bgtile", 0 )
	ROM_LOAD( "sbs-g_01.ic9",  0x000000, 0x80000, CRC(21302e78) SHA1(a17939c0529c8e9ec2a4edd5e6be4bcb67f86787) ) /* Foreground */

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "sbs-g_03.ic194",  0x000000, 0x80000, CRC(60e00d7b) SHA1(36fd02a7842ce1e79b8c4cfbe9c97052bef4aa62) ) /* Sprites */

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "sbs-g_02.ic4",  0x000000, 0x80000, CRC(f31eb668) SHA1(67d6d56ea203edfbae4db658399bf61f14134206) ) /* Background */

	ROM_REGION16_BE(0x20000, "gfx5", 0 )    /* Background tilemaps (used at runtime) */
	ROM_LOAD16_BYTE( "8.ic27",    0x00000, 0x10000, CRC(75a46fea) SHA1(3d78cfc482b42779bb5aedb722c4a39cbc71bd10) )
	ROM_LOAD16_BYTE( "9.ic26",    0x00001, 0x10000, CRC(d91448ee) SHA1(7f84ca3605edcab4bf226dab8dd7218cd5c3e5a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.ic120",    0x00000, 0x10000, CRC(5f39a980) SHA1(2a440f86685249f9c317634cad8cdedc8a8f1491) )

	ROM_REGION(0x80000, "oki1", 0 ) /* Oki sample data */
	ROM_LOAD( "sbs-g_04.ic139",    0x00000, 0x80000, CRC(7c74cc4e) SHA1(92097b372eacabdb9e8e261b0bc4223821ff9273) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION(0x80000, "oki2", 0 ) /* Oki sample data */
	ROM_LOAD( "sbs-g_05.ic160",    0x00000, 0x80000, CRC(f0a782e3) SHA1(d572226b8e597f1c34d246cb284e047a6e2d9290) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "82s135.ic94", 0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) ) /* V-sync hw (unused) */
	ROM_LOAD( "82s129.ic69", 0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* H-sync hw (unused) */
	ROM_LOAD( "82s123.ic154",0x0200, 0x0020, CRC(0f789fc7) SHA1(31936c21720802da20e39b4cb030e448353e7f19) ) /* ?? */
ROM_END

ROM_START( sbsgomo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "11.ic14",    0x00000, 0x20000, CRC(7916150b) SHA1(cbcc8918f35ded5130058860a7af6f1d3ecdbdd8) )
	ROM_LOAD16_BYTE( "10.ic15",    0x00001, 0x20000, CRC(1d7accb8) SHA1(f80fb8748017e545c96bdc7d964aa18dcd42f528) )

	ROM_REGION( 0x20000, "fgtile", 0 )
	ROM_LOAD( "7.ic46",         0x000000, 0x10000, CRC(f2b77f80) SHA1(6cb9e33994dc2741faef912416ebd57b654dfb36) ) /* Characters */

	ROM_REGION( 0x80000, "bgtile", 0 )
	ROM_LOAD( "sbs-g_01.ic9",  0x000000, 0x80000, CRC(21302e78) SHA1(a17939c0529c8e9ec2a4edd5e6be4bcb67f86787) ) /* Foreground */

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "sbs-g_03.ic194",  0x000000, 0x80000, CRC(60e00d7b) SHA1(36fd02a7842ce1e79b8c4cfbe9c97052bef4aa62) ) /* Sprites */

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "sbs-g_02.ic4",  0x000000, 0x80000, CRC(f31eb668) SHA1(67d6d56ea203edfbae4db658399bf61f14134206) ) /* Background */

	ROM_REGION16_BE(0x20000, "gfx5", 0 )    /* Background tilemaps (used at runtime) */
	ROM_LOAD16_BYTE( "8.ic27",    0x00000, 0x10000, CRC(75a46fea) SHA1(3d78cfc482b42779bb5aedb722c4a39cbc71bd10) )
	ROM_LOAD16_BYTE( "9.ic26",    0x00001, 0x10000, CRC(d91448ee) SHA1(7f84ca3605edcab4bf226dab8dd7218cd5c3e5a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.ic120",    0x00000, 0x10000, CRC(5f39a980) SHA1(2a440f86685249f9c317634cad8cdedc8a8f1491) )

	ROM_REGION(0x80000, "oki1", 0 ) /* Oki sample data */
	ROM_LOAD( "sbs-g_04.ic139",    0x00000, 0x80000, CRC(7c74cc4e) SHA1(92097b372eacabdb9e8e261b0bc4223821ff9273) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION(0x80000, "oki2", 0 ) /* Oki sample data */
	ROM_LOAD( "sbs-g_05.ic160",    0x00000, 0x80000, CRC(f0a782e3) SHA1(d572226b8e597f1c34d246cb284e047a6e2d9290) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "82s135.ic94", 0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) ) /* V-sync hw (unused) */
	ROM_LOAD( "82s129.ic69", 0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* H-sync hw (unused) */
	ROM_LOAD( "82s123.ic154",0x0200, 0x0020, CRC(0f789fc7) SHA1(31936c21720802da20e39b4cb030e448353e7f19) ) /* ?? */
ROM_END

ROM_START( blkheart )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "blkhrt.7",  0x00000, 0x20000, CRC(5bd248c0) SHA1(0649f4f8682404aeb3fc80643fcabc2d7836bb23) )
	ROM_LOAD16_BYTE( "blkhrt.6",  0x00001, 0x20000, CRC(6449e50d) SHA1(d8cd126d921c95478346da96c20da01212395d77) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Code for (unknown?) CPU */
	ROM_LOAD( "4.bin",      0x00000, 0x10000, CRC(7cefa295) SHA1(408f46613b3620cee31dec43281688d231b47ddd) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "3.bin",    0x000000, 0x020000, CRC(a1ab3a16) SHA1(3fb57c9d2ef94ee188cbadd70378ae6f4407e71d) )    /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "90068-5.bin", 0x000000, 0x100000, CRC(a1ab4f24) SHA1(b9f8104d53eda87ccd4000d049ee74ac9aa20b3e) ) /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "90068-8.bin", 0x000000, 0x100000, CRC(9d3204b2) SHA1(b37a246ad37f9ce092b371f01122ddf2bc8b2db6) ) /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90068-2.bin", 0x00000, 0x80000, CRC(3a583184) SHA1(9226f1ea7725e4b48bb055d1c17389cf960d75f8) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90068-1.bin", 0x00000, 0x80000, CRC(e7af69d2) SHA1(da050880e186954bcf0e0adf00750dd5a371551b) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "9.bpr",      0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) )  /* unknown */
	ROM_LOAD( "10.bpr",     0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )  /* unknown */
ROM_END

ROM_START( blkheartj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "7.bin",  0x00000, 0x20000, CRC(e0a5c667) SHA1(3ef39b2dc1f7ffdddf586f0b3080ecd1f362ec37) )
	ROM_LOAD16_BYTE( "6.bin",  0x00001, 0x20000, CRC(7cce45e8) SHA1(72491e30d1f9be2eede21fdde5a7484d4f65cfbf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Code for (unknown?) CPU */
	ROM_LOAD( "4.bin",      0x00000, 0x10000, CRC(7cefa295) SHA1(408f46613b3620cee31dec43281688d231b47ddd) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "3.bin",    0x000000, 0x020000, CRC(a1ab3a16) SHA1(3fb57c9d2ef94ee188cbadd70378ae6f4407e71d) )    /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "90068-5.bin", 0x000000, 0x100000, CRC(a1ab4f24) SHA1(b9f8104d53eda87ccd4000d049ee74ac9aa20b3e) ) /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "90068-8.bin", 0x000000, 0x100000, CRC(9d3204b2) SHA1(b37a246ad37f9ce092b371f01122ddf2bc8b2db6) ) /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90068-2.bin", 0x00000, 0x80000, CRC(3a583184) SHA1(9226f1ea7725e4b48bb055d1c17389cf960d75f8) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "90068-1.bin", 0x00000, 0x80000, CRC(e7af69d2) SHA1(da050880e186954bcf0e0adf00750dd5a371551b) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "9.bpr",      0x0000, 0x0100, CRC(98ed1c97) SHA1(f125ad05c3cbd1b1ab356161f9b1d814781d4c3b) )  /* unknown */
	ROM_LOAD( "10.bpr",     0x0100, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )  /* unknown */
ROM_END

ROM_START( tdragon )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code -bitswapped- */
	ROM_LOAD16_BYTE( "91070_68k.8",  0x00000, 0x20000, CRC(121c3ae7) SHA1(b88446df3b177d40e0b59a481f8e4de212e3afbc) )
	ROM_LOAD16_BYTE( "91070_68k.7",  0x00001, 0x20000, CRC(6e154d8e) SHA1(29baea24d670ab63149efe281de25cca15b7b863) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "91070.6",        0x000000, 0x20000, CRC(fe365920) SHA1(7581931cb95cd5a8ed40e4f5385b533e3d19af22) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "91070.5",        0x000000, 0x100000, CRC(d0bde826) SHA1(3b74d5fc88a4a9329e101ee72f393608d327d816) )  /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "91070.4",    0x000000, 0x100000, CRC(3eedc2fe) SHA1(9f48986c231a8fbc07f2b39b2017d1e967b2ed3c) )  /* Sprites */

	ROM_REGION( 0x010000, "audiocpu", 0 )       /* Code for (unknown?) CPU */
	ROM_LOAD( "91070.1",      0x00000, 0x10000, CRC(bf493d74) SHA1(6f8f5eff4b71fb6cabda10075cfa88a3f607859e) )

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91070.3",     0x00000, 0x80000, CRC(ae6875a8) SHA1(bfdb350b3d3fce2bead1ac60875beafe427765ed) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91070.2",     0x00000, 0x80000, CRC(ecfea43e) SHA1(d664dfa6698fec8e602523bdae16068f1ff6547b) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "91070.9",  0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )    /* unknown */
	ROM_LOAD( "91070.10", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) )    /* unknown */
ROM_END

ROM_START( tdragon1 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code -bitswapped- */
	ROM_LOAD16_BYTE( "thund.8",  0x00000, 0x20000, CRC(edd02831) SHA1(d6bc8d2c37707768a8bf666090f33eea12dda336) )
	ROM_LOAD16_BYTE( "thund.7",  0x00001, 0x20000, CRC(52192fe5) SHA1(9afef197410e7feb71dc48003e181fbbaf5c99b2) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "91070.6",        0x000000, 0x20000, CRC(fe365920) SHA1(7581931cb95cd5a8ed40e4f5385b533e3d19af22) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "91070.5",        0x000000, 0x100000, CRC(d0bde826) SHA1(3b74d5fc88a4a9329e101ee72f393608d327d816) )  /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "91070.4",    0x000000, 0x100000, CRC(3eedc2fe) SHA1(9f48986c231a8fbc07f2b39b2017d1e967b2ed3c) )  /* Sprites */

	ROM_REGION( 0x010000, "audiocpu", 0 )       /* Code for (unknown?) CPU */
	ROM_LOAD( "91070.1",      0x00000, 0x10000, CRC(bf493d74) SHA1(6f8f5eff4b71fb6cabda10075cfa88a3f607859e) )

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91070.3",     0x00000, 0x80000, CRC(ae6875a8) SHA1(bfdb350b3d3fce2bead1ac60875beafe427765ed) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91070.2",     0x00000, 0x80000, CRC(ecfea43e) SHA1(d664dfa6698fec8e602523bdae16068f1ff6547b) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "91070.9",  0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )    /* unknown */
	ROM_LOAD( "91070.10", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) )    /* unknown */
ROM_END

ROM_START( tdragonb )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code -bitswapped- */
	ROM_LOAD16_BYTE( "td_04.bin",  0x00000, 0x20000, CRC(e8a62d3e) SHA1(dd221bcd80149fffb1bdddfd3d394996bd2f8ec5) )
	ROM_LOAD16_BYTE( "td_03.bin",  0x00001, 0x20000, CRC(2fa1aa04) SHA1(ddf2b2ff179c31a1677d15d0403b00d77f9f0a6c) )

	ROM_REGION(0x20000, "audiocpu", 0 ) /* 64k for sound cpu code */
	ROM_LOAD( "td_02.bin",    0x00000, 0x8000, CRC(99ee7505) SHA1(b97c8ee5e26e8554b5de506fba3b32cc2fde53c9) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "td_08.bin",      0x000000, 0x20000, CRC(5144dc69) SHA1(e64d88dc0e7672f811868621f74ec209aeafbc6f) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "td_06.bin",      0x000000, 0x80000, CRC(c1be8a4d) SHA1(6269fd7fccf1546a01bab755d8b6b7dcffc1166e) )   /* 16x16 tiles */
	ROM_LOAD( "td_07.bin",      0x080000, 0x80000, CRC(2c3e371f) SHA1(77956425661f4f81c370fff63845d42057fcaec3) )   /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "td_10.bin",   0x000000, 0x080000, CRC(bfd0ec5d) SHA1(7983661f74e8695f56e45c6e5c278d7d86431052) )  /* Sprites */
	ROM_LOAD16_BYTE( "td_09.bin",   0x000001, 0x080000, CRC(b6e074eb) SHA1(bdde068f03415391b5edaa42f1389df0f7eef899) )  /* Sprites */

	ROM_REGION( 0x040000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "td_01.bin",     0x00000, 0x10000, CRC(f6f6c4bf) SHA1(ea4cf74d968e254ae47c16c2f4c2f4bc1a528808) )
ROM_END

ROM_START( ssmissin )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ssm14.165",    0x00001, 0x20000, CRC(eda61b74) SHA1(6247682c27d2be7dff1fad407ccf86fe2a25f11c) )
	ROM_LOAD16_BYTE( "ssm15.166",    0x00000, 0x20000, CRC(aff15927) SHA1(258c2722ac7ca50360bfefa7b4e621373975a835) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "ssm16.172",      0x000000, 0x20000, CRC(5cf6eb1f) SHA1(d406b11cf06ae1afc57a50685689e358e5677a45) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "ssm17.147",      0x000000, 0x080000, CRC(c9c28455) SHA1(6a3e754aff3f368bde0e8905c33074084ad6ac30) )  /* 16x16 tiles */
	ROM_LOAD( "ssm18.148",      0x080000, 0x080000, CRC(ebfdaad6) SHA1(0814cdfe83f36a7dd7b5416f9d0478192733dac0) )  /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ssm20.34",        0x000001, 0x080000, CRC(a0c16c4d) SHA1(e198f69b4d8660e33851a2631b5411611b1b2ea6) )  /* 16x16 tiles */
	ROM_LOAD16_BYTE( "ssm19.33",        0x000000, 0x080000, CRC(b1943657) SHA1(97c05483b634315af338434bd2f565cc151a7283) )  /* 16x16 tiles */

	ROM_REGION( 0x010000, "audiocpu", 0 )       /* Code for Sound CPU */
	ROM_LOAD( "ssm11.188",      0x00000, 0x08000, CRC(8be6dce3) SHA1(d9a235c36e0bc44025c291247d6b0b753e4bc0c8) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* OKIM6295 samples? */
	ROM_LOAD( "ssm13.190",     0x00000, 0x20000, CRC(618f66f0) SHA1(97637a03d9fd82305e872e9bfa489862c974bb6c) )
	ROM_LOAD( "ssm12.189",     0x80000, 0x80000, CRC(e8219c83) SHA1(68673d071a58ca2bfd2de344a830417d10bc5757) ) /* banked */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "ssm-pr2.113",  0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )    /* unknown */
	ROM_LOAD( "ssm-pr1.114",  0x0100, 0x0200, CRC(ed0bd072) SHA1(66a6d435d8587c82ae96dd09c39ed5749fe00e24) )    /* unknown */
ROM_END

/*

Air Attack
Comad, 1996

68000 @ 8MHz
Z80A @ 2MHz [8/4]
M6295 @ 1MHz [8/8]. Pin 7 HIGH
VSync 50Hz
HSync 15.35kHz
XTALs 8MHz (for 68000/Z80/M6295), 12MHz (for FPGAs)
2x 62256 RAM (main program RAM)
4x 62256 RAM (graphics)
2x 6264 RAM (graphics)
1x 6116 RAM (sound program)
6x 6116 RAM (other/ shared RAM etc)
2x PROMs
1x Lattice pLSI1032 FPGA
1x Actel 1020B FPGA

*/

ROM_START( airattck )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ue10.bin",     0x000000, 0x20000, CRC(71deb9d8) SHA1(21da5a68a13c9017d787e88f7b293f263fbc6b20) )
	ROM_LOAD16_BYTE( "uc10.bin",     0x000001, 0x20000, CRC(1837d4ba) SHA1(8dd5636a3a75c5d25d8850381e566a150ddc8ef1) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "4.ul10",     0x000000, 0x20000, CRC(e9362ab4) SHA1(d3e7d90e459bd4a80a189cc77821a6668103a640) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "9.uw9",      0x000000, 0x80000, CRC(86e59966) SHA1(50944dddb4c9f28e6f9b7c610a205310f4d7a076) )
	ROM_LOAD( "10.ux9",      0x080000, 0x80000, CRC(122c8d04) SHA1(70a348b1a94f1bc69532ba92dafc91a2c0e41d58) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "7.uo81",     0x000000, 0x80000, CRC(3c38d671) SHA1(f9c9aaa1622ee0c20f569f6048e2b78bd507a1e5) )    /* 16x16 tiles */
	ROM_LOAD16_BYTE( "8.uo82",     0x000001, 0x80000, CRC(9a83e3d8) SHA1(c765c4d278cc7f54ccdf6f00f8c6902a56abc2b8) )    /* 16x16 tiles */

	ROM_REGION( 0x010000, "audiocpu", 0 )       /* Code for Sound CPU */
	ROM_LOAD( "3.su6",      0x000000, 0x08000, CRC(3e352370) SHA1(6e84881dc0b09a23f8b589431005459adc334c34) )

	ROM_REGION( 0x100000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "2.su12",     0x000000, 0x20000, CRC(93ab615b) SHA1(f670ac60f5f88148e55200e5e3591aa18b81c325) )
	ROM_LOAD( "1.su13",     0x080000, 0x80000, CRC(09a836bb) SHA1(43fbd35c2ef3d201a4c82b0d3b7d7b971b385a14) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.ug6",  0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
	ROM_LOAD( "82s147.uh6",  0x0100, 0x0200, CRC(ed0bd072) SHA1(66a6d435d8587c82ae96dd09c39ed5749fe00e24) ) /* unknown */
ROM_END

ROM_START( airattcka )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "6.uc10",     0x000000, 0x20000, CRC(3572baf0) SHA1(0a2fe3be16d95896dc757ef231b3708093fc7ffa) )
	ROM_LOAD16_BYTE( "5.ue10",     0x000001, 0x20000, CRC(6589c005) SHA1(350a7b8685cacde6b72c10458c33962c5a45a255) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "4.ul10",     0x000000, 0x20000, CRC(e9362ab4) SHA1(d3e7d90e459bd4a80a189cc77821a6668103a640) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "9.uw9",      0x000000, 0x80000, CRC(86e59966) SHA1(50944dddb4c9f28e6f9b7c610a205310f4d7a076) )
	ROM_LOAD( "10.ux9",      0x080000, 0x80000, CRC(122c8d04) SHA1(70a348b1a94f1bc69532ba92dafc91a2c0e41d58) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "7.uo81",     0x000000, 0x80000, CRC(3c38d671) SHA1(f9c9aaa1622ee0c20f569f6048e2b78bd507a1e5) )    /* 16x16 tiles */
	ROM_LOAD16_BYTE( "8.uo82",     0x000001, 0x80000, CRC(9a83e3d8) SHA1(c765c4d278cc7f54ccdf6f00f8c6902a56abc2b8) )    /* 16x16 tiles */

	ROM_REGION( 0x010000, "audiocpu", 0 )       /* Code for Sound CPU */
	ROM_LOAD( "3.su6",      0x000000, 0x08000, CRC(3e352370) SHA1(6e84881dc0b09a23f8b589431005459adc334c34) )

	ROM_REGION( 0x100000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "2.su12",     0x000000, 0x20000, CRC(93ab615b) SHA1(f670ac60f5f88148e55200e5e3591aa18b81c325) )
	ROM_LOAD( "1.su13",     0x080000, 0x80000, CRC(09a836bb) SHA1(43fbd35c2ef3d201a4c82b0d3b7d7b971b385a14) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.ug6",  0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
	ROM_LOAD( "82s147.uh6",  0x0100, 0x0200, CRC(ed0bd072) SHA1(66a6d435d8587c82ae96dd09c39ed5749fe00e24) ) /* unknown */
ROM_END

ROM_START( strahl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "strahl-2.82", 0x00000, 0x20000, CRC(c9d008ae) SHA1(e9218a3143d5887e702df051354a9083a806c69c) )
	ROM_LOAD16_BYTE( "strahl-1.83", 0x00001, 0x20000, CRC(afc3c4d6) SHA1(ab3dd7db692eb01e3a87f4216d322a702f3beaad) )

	ROM_REGION( 0x20000, "fgtile", 0 )
	ROM_LOAD( "strahl-3.73",  0x000000, 0x10000, CRC(2273b33e) SHA1(fa53e91b80dfea3f8b2c1f0ce66e5c6920c4960f) ) /* Characters */

	ROM_REGION( 0x40000, "bgtile", 0 )
	ROM_LOAD( "str7b2r0.275", 0x000000, 0x40000, CRC(5769e3e1) SHA1(7d7a16b11027d0a7618df1ec1e3484224b772e90) ) /* Tiles */

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "strl3-01.32",  0x000000, 0x80000, CRC(d8337f15) SHA1(4df23fff2506b66a94dae4e0cf7d25499936b942) ) /* Sprites */
	ROM_LOAD( "strl4-02.57",  0x080000, 0x80000, CRC(2a38552b) SHA1(82335fc6aa3de9145dd84952e5ed423493bf7141) )
	ROM_LOAD( "strl5-03.58",  0x100000, 0x80000, CRC(a0e7d210) SHA1(96a762a3a1cdeaa91bde50429e0ac665fb81190b) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "str6b1w1.776", 0x000000, 0x80000, CRC(bb1bb155) SHA1(83a02e89180e15f0e7817e0e92b4bf4e209bb69a) ) /* Tiles */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "strahl-4.66",    0x00000, 0x10000, CRC(60a799c4) SHA1(8ade3cf827a389f7cb4080957dc4d67077ea4166) )

	ROM_REGION( 0xa0000, "oki1", 0 )    /* Oki sample data */
	ROM_LOAD( "str8pmw1.540", 0x00000, 0x20000, CRC(01d6bb6a) SHA1(b157f6f921483ed8067a7e13e370f73fdb60d136) )
	/* this is a mess */
	ROM_CONTINUE(             0x60000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x40000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x20000, 0x20000 )    /* banked */

	ROM_REGION( 0xa0000, "oki2", 0 )    /* Oki sample data */
	ROM_LOAD( "str9pew1.639", 0x00000, 0x20000, CRC(6bb3eb9f) SHA1(9c1394df4f8a08f9098c85eb3d38fb862d6eabbb) )
	/* this is a mess */
	ROM_CONTINUE(             0x60000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x40000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x20000, 0x20000 )    /* banked */
ROM_END

ROM_START( strahla )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom2", 0x00000, 0x20000, CRC(f80a22ef) SHA1(22099eb0bbb445702e0276713c3e48d60de60c30) )
	ROM_LOAD16_BYTE( "rom1", 0x00001, 0x20000, CRC(802ecbfc) SHA1(cc776023c7bd6b6d6af9659a0c822a2887e50199) )

	ROM_REGION( 0x20000, "fgtile", 0 )
	ROM_LOAD( "strahl-3.73",  0x000000, 0x10000, CRC(2273b33e) SHA1(fa53e91b80dfea3f8b2c1f0ce66e5c6920c4960f) ) /* Characters */

	ROM_REGION( 0x40000, "bgtile", 0 )
	ROM_LOAD( "str7b2r0.275", 0x000000, 0x40000, CRC(5769e3e1) SHA1(7d7a16b11027d0a7618df1ec1e3484224b772e90) ) /* Tiles */

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "strl3-01.32",  0x000000, 0x80000, CRC(d8337f15) SHA1(4df23fff2506b66a94dae4e0cf7d25499936b942) ) /* Sprites */
	ROM_LOAD( "strl4-02.57",  0x080000, 0x80000, CRC(2a38552b) SHA1(82335fc6aa3de9145dd84952e5ed423493bf7141) )
	ROM_LOAD( "strl5-03.58",  0x100000, 0x80000, CRC(a0e7d210) SHA1(96a762a3a1cdeaa91bde50429e0ac665fb81190b) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "str6b1w1.776", 0x000000, 0x80000, CRC(bb1bb155) SHA1(83a02e89180e15f0e7817e0e92b4bf4e209bb69a) ) /* Tiles */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "strahl-4.66",    0x00000, 0x10000, CRC(60a799c4) SHA1(8ade3cf827a389f7cb4080957dc4d67077ea4166) )

	ROM_REGION( 0xa0000, "oki1", 0 )    /* Oki sample data */
	ROM_LOAD( "str8pmw1.540", 0x00000, 0x20000, CRC(01d6bb6a) SHA1(b157f6f921483ed8067a7e13e370f73fdb60d136) )
	/* this is a mess */
	ROM_CONTINUE(             0x60000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x40000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x20000, 0x20000 )    /* banked */

	ROM_REGION( 0xa0000, "oki2", 0 )    /* Oki sample data */
	ROM_LOAD( "str9pew1.639", 0x00000, 0x20000, CRC(6bb3eb9f) SHA1(9c1394df4f8a08f9098c85eb3d38fb862d6eabbb) )
	/* this is a mess */
	ROM_CONTINUE(             0x60000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x40000, 0x20000 )    /* banked */
	ROM_CONTINUE(             0x20000, 0x20000 )    /* banked */
ROM_END

ROM_START( hachamfb )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "8.bin",  0x00000, 0x20000, CRC(14845b65) SHA1(5cafd07a8a6f5ccbb36de7a90571f8b33ecf273e) )
	ROM_LOAD16_BYTE( "7.bin",  0x00001, 0x20000, CRC(069ca579) SHA1(0db4c3c41e17fca613d11de89b388a4af206ec6b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* unknown  - sound cpu ?????? */
	ROM_LOAD( "1.70",  0x00000, 0x10000, CRC(9e6f48fc) SHA1(aeb5bfecc025b5478f6de874792fc0f7f54932be) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "5.95",  0x000000, 0x020000, CRC(29fb04a2) SHA1(9654b90a66d0e2a0f9cd369cab29cdd0c6f77869) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 ) /* 16x16 tiles */
	ROM_LOAD( "91076-4.101",  0x000000, 0x100000, CRC(df9653a4) SHA1(4a3204a98d7738c7895169fcece922fdf355f4fa) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "91076-8.57",  0x000000, 0x100000, CRC(7fd0f556) SHA1(d1b4bec0946869d3d7bcb870d9ae3bd17395a231) ) /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91076-2.46",   0x00000, 0x80000, CRC(3f1e67f2) SHA1(413e78587d8a043a0eb94447313ba1b3c5b35be5) ) /* 1st & 2nd half identical, needs verifying */
	/* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91076-3.45",   0x00000, 0x80000, CRC(b25ed93b) SHA1(d7bc686bbccf982f40420a11158aa8e5dd4207c5) ) /* 1st & 2nd half identical, needs verifying */
	/* 0x20000 - 0x80000 banked */
ROM_END

ROM_START( hachamf )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "7.93",  0x00000, 0x20000, CRC(9d847c31) SHA1(1d370d8db9cadadb9c2cb213e32f681947d81b7f) )
	ROM_LOAD16_BYTE( "6.94",  0x00001, 0x20000, CRC(de6408a0) SHA1(2df77fecd44d2d8b0444abd4545923213ed76b2d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* unknown  - sound cpu ?????? */
	ROM_LOAD( "1.70",  0x00000, 0x10000, CRC(9e6f48fc) SHA1(aeb5bfecc025b5478f6de874792fc0f7f54932be) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "5.95",  0x000000, 0x020000, CRC(29fb04a2) SHA1(9654b90a66d0e2a0f9cd369cab29cdd0c6f77869) )   /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 ) /* 16x16 tiles */
	ROM_LOAD( "91076-4.101",  0x000000, 0x100000, CRC(df9653a4) SHA1(4a3204a98d7738c7895169fcece922fdf355f4fa) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "91076-8.57",  0x000000, 0x100000, CRC(7fd0f556) SHA1(d1b4bec0946869d3d7bcb870d9ae3bd17395a231) ) /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91076-2.46",   0x00000, 0x80000, CRC(3f1e67f2) SHA1(413e78587d8a043a0eb94447313ba1b3c5b35be5) ) /* 1st & 2nd half identical, needs verifying */
	/* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "91076-3.45",   0x00000, 0x80000, CRC(b25ed93b) SHA1(d7bc686bbccf982f40420a11158aa8e5dd4207c5) ) /* 1st & 2nd half identical, needs verifying */
	/* 0x20000 - 0x80000 banked */
ROM_END

ROM_START( macross )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "921a03",        0x00000, 0x80000, CRC(33318d55) SHA1(c99f85e09bd334dc8ce138b08cbed2331b0d67dd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound program (unknown CPU) */
	ROM_LOAD( "921a02",      0x00000, 0x10000, CRC(77c082c7) SHA1(be07aa14d0116f830f98e11a19f1debb48a5230e) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "921a01",      0x000000, 0x020000, CRC(bbd8242d) SHA1(7cf4897be1278e1190f499f00bc78384817a5160) ) /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "921a04",      0x000000, 0x200000, CRC(4002e4bb) SHA1(281433d798ac85c84d4f1f3751a3032e8a3b5cd4) ) /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "921a07",      0x000000, 0x200000, CRC(7d2bf112) SHA1(1997c99c2d3998096842abd1cee89e0e6ab43a47) ) /* Sprites */

	ROM_REGION( 0x80000, "oki1", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "921a05",      0x00000, 0x80000, CRC(d5a1eddd) SHA1(42b5b255f02b9c6d856b1578af9a5dfc51ea6ebb) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x80000, "oki2", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "921a06",      0x00000, 0x80000, CRC(89461d0f) SHA1(b7d27d0ee0b7ab44c20ab710b567f64fc3afb90c) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "921a08",      0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
	ROM_LOAD( "921a09",      0x0100, 0x0100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) ) /* unknown */
	ROM_LOAD( "921a10",      0x0200, 0x0020, CRC(8371e42d) SHA1(6cfd70dfa00e85ec1df8832d41df331cc3e3733a) ) /* unknown */
ROM_END


/*

Gun Nail
NMK/Tecmo, 1993

PCB Layout
----------

AK92077
|-------------------------------------------------------------|
|  LA4460  VOL YM2203  6116    92077-2.U101  62256    62256   |
|-|                       16MHz |------|     62256    62256   |
  |   4558   6295 92077-6.U57   |NMK004|     62256    62256   |
|-|                       12MHz |      |     62256    62256   |
|    YM3014  6295 92077-5.U56   |------|                      |
|                 |------| DIP2             |------| |------| |
|J                |NMK005|                  |NMK009| |NMK009| |
|A                |      | DIP1             |      | |      | |
|M                |------|     92077-10.U96 |------| |------| |
|M                             6116                           |
|A   |------| 6116             6116     |------| |----------| |
|    |NMK111| 6116                      |NMK008| |  NMK214  | |
|    |------|                           |      | |----------| |
|-|      92077-8.U35 |------|92077-9.U72|------|              |
  |                  |NMK902|       |----------|              |
|-|         6116     |------|       |  NMK215  | 92077-7.U134 |
|           6116         |------|   |----------|              |
|  |------| 92077-1.U21  |NMK903|                92077-3O.U133|
|  |NMK111| |----------| |------|         62256  92077-3E.U131|
|  |------| |  NMK214  | |------|         62256               |
|           |----------| |NMK903|          |----------------| |
|  |------|              |------|    6116  |                | |
|  |NMK111| 92077-4.U19  |------|          |     68000      | |
|  |------|              |NMK901|    6116  |                | |
|   6264                 |------|          |----------------| |
|   6264                                               10MHz  |
|-------------------------------------------------------------|
Notes:
      68000 - Motorola MC68000P12 CPU running at 10MHz (DIP64)
      6116  - 2Kb x8 SRAM (x9, DIP24)
      6264  - 8Kb x8 SRAM (x2, DIP28)
      62256 - 32Kb x8 SRAM (x10, DIP28)
      YM2203- Yamaha YM2203 running at 1.5MHz [12/8] (DIP40)
      YM3014- Yamaha YM3014 (DIP8)
      6295  - OKI M6295 running at 4MHz, pin 7 low [16/4] (x2, QFP44)
      4558  - BA4558 Op Amp (DIP8)
      LA4460- Power Amplifier
      DIP1/2- 8 position DIP Switches
      VOL   - Volume Potentiometer
      OSC   - 12MHz, 16MHz, 10MHz
      HSync - 15.367kHz
      VSync - 56.205Hz

      NMK CUSTOM IC'S
          - NMK004 marked "NMK004 0840-1324". Actually a TLCS90-based Toshiba TMP90C840AF
            Microcontroller with 256 bytes RAM & 8Kb ROM, running at 8.000MHz [16/2] (QFP64)
          - NMK005 (x1, Square QFP64)
          - NMK008 (x1, Square QFP84)
          - NMK009 (x2, Square QFP100)
          - NMK111 (x3, QFP64)
          - NMK901 (x1, QFP80)
          - NMK903 (x2, QFP44)
          - NMK214 (x2, SDIP64)
          - NMK215 (x1, SDIP64)

*/

ROM_START( gunnail )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "3e.u131",  0x00000, 0x40000, CRC(61d985b2) SHA1(96daca603f18accb47f98a3e584b2c84fc5a2ca4) )
	ROM_LOAD16_BYTE( "3o.u133",  0x00001, 0x40000, CRC(f114e89c) SHA1(a12f5278167f446bb5277e87289c41b5aa365c86) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* Code for NMK004 CPU */
	ROM_LOAD( "92077_2.u101",      0x00000, 0x10000, CRC(cd4e55f8) SHA1(92182767ca0ec37ec4949bd1a88c2efdcdcb60ed) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1.u21",    0x000000, 0x020000, CRC(3d00a9f4) SHA1(91a82e3e74c8774d7f8b2adceb228b97010facfd) )    /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "92077-4.u19", 0x000000, 0x100000, CRC(a9ea2804) SHA1(14dbdb3c7986db5e44dc7c5be6fcf39f3d1e50b0) ) /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "92077-7.u134", 0x000000, 0x200000, CRC(d49169b3) SHA1(565ff7725dd6ace79b55706114132d8d867e81a9) ) /* Sprites */

	ROM_REGION( 0x080000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "92077-5.u56", 0x00000, 0x80000, CRC(feb83c73) SHA1(b44e9d20b4af02e218c4bc875d66a7d6b8551cae) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x080000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "92077-6.u57", 0x00000, 0x80000, CRC(6d133f0d) SHA1(8a5e6e27a297196f20e4de0d060f1188115809bb) ) /* 0x20000 - 0x80000 banked */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "8_82s129.u35",   0x0000, 0x0100, CRC(4299776e) SHA1(683d14d2ace14965f0fcfe0f0540c1b77d2cece5) )  /* unknown */
	ROM_LOAD( "9_82s135.u72",   0x0100, 0x0100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) )  /* unknown */
	ROM_LOAD( "10_82s123.u96",  0x0200, 0x0020, CRC(c60103c8) SHA1(dfb05b704bb5e1f75f5aaa4fa36e8ddcc905f8b6) )  /* unknown */
ROM_END

ROM_START( macross2 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mcrs2j.3",      0x00000, 0x80000, CRC(36a618fe) SHA1(56fdb2bcb4a39888cfbaf9692d66335524a6ac0c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "mcrs2j.2",    0x00000, 0x20000, CRC(b4aa8ac7) SHA1(73a6de56cbfb468450d9b39fcbae0362f242f37b) ) /* banked */

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "mcrs2j.1",    0x000000, 0x020000, CRC(c7417410) SHA1(41431d8f1ff4d66baf1a8518a0b0c0125d1d71d4) ) /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "bp932an.a04", 0x000000, 0x200000, CRC(c4d77ff0) SHA1(aca60a3f5f89265e7e3799e5d80ea8196fb11ff3) ) /* 16x16 tiles */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "bp932an.a07", 0x000000, 0x200000, CRC(aa1b21b9) SHA1(133822e3d8628aa4eb3e62fbd054956799423b98) ) /* Sprites */
	ROM_LOAD16_WORD_SWAP( "bp932an.a08", 0x200000, 0x200000, CRC(67eb2901) SHA1(25e0f9fda1a8c0c2b59616dd153cb6dcb459d2d9) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "bp932an.a06", 0x040000, 0x200000, CRC(ef0ffec0) SHA1(fd72cc77e02d1a00bf27e77a33d7dab5f6ba1cb4) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "bp932an.a05", 0x040000, 0x100000, CRC(b5335abb) SHA1(f4eaf4e465eeca31741d432ee46ed39ffcd92cca) ) /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mcrs2bpr.9",  0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) ) /* unknown */
	ROM_LOAD( "mcrs2bpr.10", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) ) /* unknown */
ROM_END

ROM_START( macross2g )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "3.u11",      0x00000, 0x80000, CRC(151f9d39) SHA1(d0454627f019c60615cc8bd11e6cbec1f885cf13) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "mcrs2j.2",    0x00000, 0x20000, CRC(b4aa8ac7) SHA1(73a6de56cbfb468450d9b39fcbae0362f242f37b) ) /* banked */

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "mcrs2j.1",    0x000000, 0x020000, CRC(c7417410) SHA1(41431d8f1ff4d66baf1a8518a0b0c0125d1d71d4) ) /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "bp932an.a04", 0x000000, 0x200000, CRC(c4d77ff0) SHA1(aca60a3f5f89265e7e3799e5d80ea8196fb11ff3) ) /* 16x16 tiles */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "bp932an.a07", 0x000000, 0x200000, CRC(aa1b21b9) SHA1(133822e3d8628aa4eb3e62fbd054956799423b98) ) /* Sprites */
	ROM_LOAD16_WORD_SWAP( "bp932an.a08", 0x200000, 0x200000, CRC(67eb2901) SHA1(25e0f9fda1a8c0c2b59616dd153cb6dcb459d2d9) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "bp932an.a06", 0x040000, 0x200000, CRC(ef0ffec0) SHA1(fd72cc77e02d1a00bf27e77a33d7dab5f6ba1cb4) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "bp932an.a05", 0x040000, 0x100000, CRC(b5335abb) SHA1(f4eaf4e465eeca31741d432ee46ed39ffcd92cca) ) /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mcrs2bpr.9",  0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) ) /* unknown */
	ROM_LOAD( "mcrs2bpr.10", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) ) /* unknown */
ROM_END

ROM_START( tdragon2 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "6.rom",      0x00000, 0x80000, CRC(ca348caf) SHA1(7c5b0b92560baf413591230e061d2d57b25deafe) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "5.bin",    0x00000, 0x20000, CRC(b870be61) SHA1(ea5d45c3a3ab805e55806967f00167cf6366212e) ) /* banked */

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1.bin",    0x000000, 0x020000, CRC(d488aafa) SHA1(4d05e7ca075b638dd90ae4c9f224817a8a3ae9f3) )    /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ww930914.2", 0x000000, 0x200000, CRC(f968c65d) SHA1(fd6d21bba53f945b1597d7d0735bc62dd44d5498) )  /* 16x16 tiles */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ww930917.7", 0x000000, 0x200000, CRC(b98873cb) SHA1(cc19200865176e940ff68e12de81f029b51c2084) )  /* Sprites */
	ROM_LOAD16_WORD_SWAP( "ww930918.8", 0x200000, 0x200000, CRC(baee84b2) SHA1(b325b00e6147266dbdc840e03556004531dc2038) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930916.4", 0x040000, 0x200000, CRC(07c35fe6) SHA1(33547bd88764704310f2ef8cf3bfe21ceb56d5b7) )  /* all banked */

	ROM_REGION( 0x240000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930915.3", 0x040000, 0x200000, CRC(82025bab) SHA1(ac6053700326ea730d00ec08193e2c8a2a019f0b) )  /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "9.bpr",  0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) )  /* unknown */
	ROM_LOAD( "10.bpr", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) )  /* unknown */
ROM_END

ROM_START( tdragon2a )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "6.bin",      0x00000, 0x80000, CRC(310d6bca) SHA1(f46ad1d13cf5014aef1f0e8862b369ab31c22866) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "5.bin",    0x00000, 0x20000, CRC(b870be61) SHA1(ea5d45c3a3ab805e55806967f00167cf6366212e) ) /* banked */

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1.bin",    0x000000, 0x020000, CRC(d488aafa) SHA1(4d05e7ca075b638dd90ae4c9f224817a8a3ae9f3) )    /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ww930914.2", 0x000000, 0x200000, CRC(f968c65d) SHA1(fd6d21bba53f945b1597d7d0735bc62dd44d5498) )  /* 16x16 tiles */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ww930917.7", 0x000000, 0x200000, CRC(b98873cb) SHA1(cc19200865176e940ff68e12de81f029b51c2084) )  /* Sprites */
	ROM_LOAD16_WORD_SWAP( "ww930918.8", 0x200000, 0x200000, CRC(baee84b2) SHA1(b325b00e6147266dbdc840e03556004531dc2038) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930916.4", 0x040000, 0x200000, CRC(07c35fe6) SHA1(33547bd88764704310f2ef8cf3bfe21ceb56d5b7) )  /* all banked */

	ROM_REGION( 0x240000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930915.3", 0x040000, 0x200000, CRC(82025bab) SHA1(ac6053700326ea730d00ec08193e2c8a2a019f0b) )  /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "9.bpr",  0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) )  /* unknown */
	ROM_LOAD( "10.bpr", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) )  /* unknown */
ROM_END

ROM_START( bigbang )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "eprom.3",      0x00000, 0x80000, CRC(28e5957a) SHA1(fe4f870a9c2235cc02b4e036a2a4116f071d59ad) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "5.bin",    0x00000, 0x20000, CRC(b870be61) SHA1(ea5d45c3a3ab805e55806967f00167cf6366212e) ) /* banked */

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "1.bin",    0x000000, 0x020000, CRC(d488aafa) SHA1(4d05e7ca075b638dd90ae4c9f224817a8a3ae9f3) )    /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ww930914.2", 0x000000, 0x200000, CRC(f968c65d) SHA1(fd6d21bba53f945b1597d7d0735bc62dd44d5498) )  /* 16x16 tiles */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ww930917.7", 0x000000, 0x200000, CRC(b98873cb) SHA1(cc19200865176e940ff68e12de81f029b51c2084) )  /* Sprites */
	ROM_LOAD16_WORD_SWAP( "ww930918.8", 0x200000, 0x200000, CRC(baee84b2) SHA1(b325b00e6147266dbdc840e03556004531dc2038) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930916.4", 0x040000, 0x200000, CRC(07c35fe6) SHA1(33547bd88764704310f2ef8cf3bfe21ceb56d5b7) )  /* all banked */

	ROM_REGION( 0x240000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ww930915.3", 0x040000, 0x200000, CRC(82025bab) SHA1(ac6053700326ea730d00ec08193e2c8a2a019f0b) )  /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "9.bpr",  0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) )  /* unknown */
	ROM_LOAD( "10.bpr", 0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) )  /* unknown */
ROM_END

/*

Rapid Hero
NMK, 1994

The main board has no ROMs at all except 3 PROMs. There is a plug-in daughter
board that holds all the ROMs. It has the capacity for 3 socketed EPROMS and 7x
16M MASK ROMs total.


PCB Layout (Main board)
-----------------------

AWA94099
-----------------------------------------------------------------------
|    YM2203  TMP90C841 6264 DSW2(8) 62256 62256 6116 62256 62256 6116 |
|   6295 NMK112      12MHz     DSW1(8)                                |
|  YM3014B 6295      16MHz NMK005   62256 62256 6116 62256 62256 6116 |
|J       -----------------                                            |
|A       |               |                                            |
|M       -----------------                                            |
|M                   PROM3                           NMK009  NMK009   |
|A   NMK111   6116                    6116  NMK008                    |
|       |-|   6116                    6116                            |
|6116   | | NMK902   -----------------                                |
|6116   | |          |               |                                |
|PROM1  | |          -----------------                                |
|       | |                                                           |
|NMK111 | | NMK903                                                    |
|       | | NMK903       PROM2                                        |
|NMK111 | |                                                           |
|       |-|                   6116                  TMP68HC000P-16    |
| 62256     NMK901            6116                             14MHz  |
| 62256                                                               |
-----------------------------------------------------------------------

Notes:
      68k clock: 14.00MHz
          VSync: 56Hz
          HSync: 15.35kHz
   90c841 clock: 8.000MHz


PCB Layout (Daughter board)
---------------------------

AWA94099-ROME
--------------------------
| 2      6   7   5    3  |
|                        |
| 1                      |
|                        |
|                        |
|                        |
| 4           8   9   10 |
|                        |
|                        |
|                        |
--------------------------

*/

ROM_START( arcadian )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "arcadia.3",      0x00000, 0x80000, CRC(8b46d609) SHA1(793870d74c9d7d04c53d898610c682b2dc90d0af) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* tmp90c841 */
	ROM_LOAD( "rhp94099.2",    0x00000, 0x20000, CRC(fe01ece1) SHA1(c469fb79f2774089848c814f92ddd3c9e384050f) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "arcadia.1",    0x000000, 0x020000, CRC(1c2c4008) SHA1(583d74a0a44519a7050b1d8490011ff60222f466) )   /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "rhp94099.4", 0x000000, 0x200000,  CRC(076eee7b) SHA1(7c315fe33d0fcd92e0ce2f274996c8059228b005) ) /* 16x16 tiles */

	ROM_REGION( 0x600000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_WORD_SWAP( "rhp94099.8", 0x000000, 0x200000, CRC(49892f07) SHA1(2f5d20cd193cffcba9041aa11d6665adebeffffa) )  /* 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "rhp94099.9", 0x200000, 0x200000, CRC(ea2e47f0) SHA1(97dfa8f95f27b36deb5ce1c80e3d727bad24e52b) )  /* 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "rhp94099.10",0x400000, 0x200000, CRC(512cb839) SHA1(4a2c5ac88e4bf8a6f07c703277c4d33e649fd192) )  /* 16x16 tiles */

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "rhp94099.6", 0x040000, 0x200000, CRC(f1a80e5a) SHA1(218bd7b0c3d8b283bf96b95bf888228810699370) )  /* all banked */
	ROM_LOAD( "rhp94099.7", 0x240000, 0x200000, CRC(0d99547e) SHA1(2d9630bd55d27010f9d1d2dbdbd07ac265e8ebe6) )  /* all banked */

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "rhp94099.5", 0x040000, 0x200000, CRC(515eba93) SHA1(c35cb5f31f4bc7327be5777624af168f9fb364a5) )  /* all banked */
	ROM_LOAD( "rhp94099.6", 0x240000, 0x200000, CRC(f1a80e5a) SHA1(218bd7b0c3d8b283bf96b95bf888228810699370) )  /* all banked */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "prom1.u19",      0x0000, 0x0100, CRC(4299776e) SHA1(683d14d2ace14965f0fcfe0f0540c1b77d2cece5) ) /* unknown */
	ROM_LOAD( "prom2.u53",      0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) ) /* unknown */
	ROM_LOAD( "prom3.u60",      0x0200, 0x0100, CRC(304f98c6) SHA1(8dfd9bf719087ec30c83efe95c4561666c7d1801) ) /* unknown */
ROM_END

ROM_START( raphero )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rhp94099.3",      0x00000, 0x80000, CRC(ec9b4f05) SHA1(e5bd797620dc449fd78b41d87e9ba5a764eb8b44) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* tmp90c841 */
	ROM_LOAD( "rhp94099.2",    0x00000, 0x20000, CRC(fe01ece1) SHA1(c469fb79f2774089848c814f92ddd3c9e384050f) )

	ROM_REGION( 0x020000, "fgtile", 0 )
	ROM_LOAD( "rhp94099.1",    0x000000, 0x020000, CRC(55a7a011) SHA1(87ded56bfdd38cbf8d3bd8b3789831f768550a12) )   /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "rhp94099.4", 0x000000, 0x200000,  CRC(076eee7b) SHA1(7c315fe33d0fcd92e0ce2f274996c8059228b005) ) /* 16x16 tiles */

	ROM_REGION( 0x600000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_WORD_SWAP( "rhp94099.8", 0x000000, 0x200000, CRC(49892f07) SHA1(2f5d20cd193cffcba9041aa11d6665adebeffffa) )  /* 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "rhp94099.9", 0x200000, 0x200000, CRC(ea2e47f0) SHA1(97dfa8f95f27b36deb5ce1c80e3d727bad24e52b) )  /* 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "rhp94099.10",0x400000, 0x200000, CRC(512cb839) SHA1(4a2c5ac88e4bf8a6f07c703277c4d33e649fd192) )  /* 16x16 tiles */

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "rhp94099.6", 0x040000, 0x200000, CRC(f1a80e5a) SHA1(218bd7b0c3d8b283bf96b95bf888228810699370) )  /* all banked */
	ROM_LOAD( "rhp94099.7", 0x240000, 0x200000, CRC(0d99547e) SHA1(2d9630bd55d27010f9d1d2dbdbd07ac265e8ebe6) )  /* all banked */

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "rhp94099.5", 0x040000, 0x200000, CRC(515eba93) SHA1(c35cb5f31f4bc7327be5777624af168f9fb364a5) )  /* all banked */
	ROM_LOAD( "rhp94099.6", 0x240000, 0x200000, CRC(f1a80e5a) SHA1(218bd7b0c3d8b283bf96b95bf888228810699370) )  /* all banked */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "prom1.u19",      0x0000, 0x0100, CRC(4299776e) SHA1(683d14d2ace14965f0fcfe0f0540c1b77d2cece5) ) /* unknown */
	ROM_LOAD( "prom2.u53",      0x0100, 0x0100, CRC(e6ead349) SHA1(6d81b1c0233580aa48f9718bade42d640e5ef3dd) ) /* unknown */
	ROM_LOAD( "prom3.u60",      0x0200, 0x0100, CRC(304f98c6) SHA1(8dfd9bf719087ec30c83efe95c4561666c7d1801) ) /* unknown */
ROM_END

ROM_START( sabotenb )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "ic76.sb1",  0x00000, 0x40000, CRC(b2b0b2cf) SHA1(219f1cefdb107d8404f4f8bfa0700fd3218d9320) )
	ROM_LOAD16_BYTE( "ic75.sb2",  0x00001, 0x40000, CRC(367e87b7) SHA1(c950041529b5117686e4bb1ae77db82fe758c1d0) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "ic35.sb3",       0x000000, 0x010000, CRC(eb7bc99d) SHA1(b3063afd58025a441d4750c22483e9129da402e7) )  /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ic32.sb4",       0x000000, 0x200000, CRC(24c62205) SHA1(3ab0ca5d7c698328d91421ccf6f7dafc20df3c8d) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ic100.sb5",  0x000000, 0x200000, CRC(b20f166e) SHA1(074d770fd6d233040a80a92f4467d81f961c650b) )  /* Sprites */

	ROM_REGION( 0x140000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic30.sb6",    0x040000, 0x100000, CRC(288407af) SHA1(78c08fae031337222681c593dc86a08df6a34a4b) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic27.sb7",    0x040000, 0x100000, CRC(43e33a7e) SHA1(51068b63f4415712eaa25dcf1ee6b0cc2850974e) ) /* all banked */
ROM_END

ROM_START( sabotenba )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "sb1.76",  0x00000, 0x40000, CRC(df6f65e2) SHA1(6ad9e9f13539310646895c5e7992c6546e75684b) )
	ROM_LOAD16_BYTE( "sb2.75",  0x00001, 0x40000, CRC(0d2c1ab8) SHA1(abb43a8c5398195c0ad48d8d772ef47635bf25c2) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "ic35.sb3",       0x000000, 0x010000, CRC(eb7bc99d) SHA1(b3063afd58025a441d4750c22483e9129da402e7) )  /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ic32.sb4",       0x000000, 0x200000, CRC(24c62205) SHA1(3ab0ca5d7c698328d91421ccf6f7dafc20df3c8d) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ic100.sb5",  0x000000, 0x200000, CRC(b20f166e) SHA1(074d770fd6d233040a80a92f4467d81f961c650b) )  /* Sprites */

	ROM_REGION( 0x140000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic30.sb6",    0x040000, 0x100000, CRC(288407af) SHA1(78c08fae031337222681c593dc86a08df6a34a4b) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic27.sb7",    0x040000, 0x100000, CRC(43e33a7e) SHA1(51068b63f4415712eaa25dcf1ee6b0cc2850974e) ) /* all banked */
ROM_END

ROM_START( cactus )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "02.bin",  0x00000, 0x40000, CRC(15b2ff2f) SHA1(432cfd58daa0fdbe62157b36ca73eb9af6ce91e9) ) /* PCB is marked 'Cactus', actual game has no title screen */
	ROM_LOAD16_BYTE( "01.bin",  0x00001, 0x40000, CRC(5b8ba46a) SHA1(617e414fda1bd3e9f391676d312b0cdd4700adee) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "i03.bin",        0x000000, 0x010000, CRC(eb7bc99d) SHA1(b3063afd58025a441d4750c22483e9129da402e7) )  /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "s-05.bin",    0x000000, 0x100000, CRC(fce962b9) SHA1(abd4311a17dac819d5bf8d81fe289a8b3a793b32) )
	ROM_LOAD( "s-06.bin",    0x100000, 0x100000, CRC(16768fbc) SHA1(fe3667fc2e8fd0c6690e09f7b24466cc3eb34403) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "s-03.bin",    0x000001, 0x100000, CRC(bc1781b8) SHA1(5000f2111c5981428a772a9dcae2c7c8f1f6958b) )
	ROM_LOAD16_BYTE( "s-04.bin",    0x000000, 0x100000, CRC(f823885e) SHA1(558b2bed207ccff8f1425cbb9dadc1ec0b70a65b) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "s-01.bin",    0x040000, 0x100000, CRC(288407af) SHA1(78c08fae031337222681c593dc86a08df6a34a4b) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "s-02.bin",    0x040000, 0x100000, CRC(43e33a7e) SHA1(51068b63f4415712eaa25dcf1ee6b0cc2850974e) ) /* all banked */
ROM_END

ROM_START( bjtwin )
	ROM_REGION( 0x80000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_BYTE( "93087-1.bin",  0x00000, 0x20000, CRC(93c84e2d) SHA1(ad0755cabfef78e7e689856379d6f8c88a9b27c1) )
	ROM_LOAD16_BYTE( "93087-2.bin",  0x00001, 0x20000, CRC(30ff678a) SHA1(aa3ce4905e448e371e254545ef9ed7edb00b1cc3) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "93087-3.bin",  0x000000, 0x010000, CRC(aa13df7c) SHA1(162d4f12364c68028e86fe97ee75c262daa4c699) ) /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "93087-4.bin",  0x000000, 0x100000, CRC(8a4f26d0) SHA1(be057a2b6d28c623ac1f16cf02ddbe12ca430b4a) ) /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "93087-5.bin", 0x000000, 0x100000, CRC(bb06245d) SHA1(c91e2284d95370b8ef2eb1b9d6305fdd6cde23a0) ) /* Sprites */

	ROM_REGION( 0x140000, "oki1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "93087-6.bin",    0x040000, 0x100000, CRC(372d46dd) SHA1(18f44e777241af50787730652fa018c51b65ea15) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "93087-7.bin",    0x040000, 0x100000, CRC(8da67808) SHA1(f042574c097f5a8c2684fcc23f2c817c168254ef) ) /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "8.bpr",      0x0000, 0x0100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) ) /* unknown */
	ROM_LOAD( "9.bpr",      0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) ) /* unknown */
ROM_END

ROM_START( bjtwina )
	ROM_REGION( 0x80000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_BYTE( "93087.1",  0x00000, 0x20000, CRC(c82b3d8e) SHA1(74435ba7842f1be9968006894cfa5eef05c47395) )
	ROM_LOAD16_BYTE( "93087.2",  0x00001, 0x20000, CRC(9be1ec47) SHA1(bf37d9254a7bbdf49b006971886ed9845d72e4b3) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "93087-3.bin",  0x000000, 0x010000, CRC(aa13df7c) SHA1(162d4f12364c68028e86fe97ee75c262daa4c699) ) /* 8x8 tiles */

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_LOAD( "93087-4.bin",  0x000000, 0x100000, CRC(8a4f26d0) SHA1(be057a2b6d28c623ac1f16cf02ddbe12ca430b4a) ) /* 16x16 tiles */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "93087-5.bin", 0x000000, 0x100000, CRC(bb06245d) SHA1(c91e2284d95370b8ef2eb1b9d6305fdd6cde23a0) ) /* Sprites */

	ROM_REGION( 0x140000, "oki1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "93087-6.bin",    0x040000, 0x100000, CRC(372d46dd) SHA1(18f44e777241af50787730652fa018c51b65ea15) ) /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "93087-7.bin",    0x040000, 0x100000, CRC(8da67808) SHA1(f042574c097f5a8c2684fcc23f2c817c168254ef) ) /* all banked */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "8.bpr",      0x0000, 0x0100, CRC(633ab1c9) SHA1(acd99fcca41eaab7948ca84988352f1d7d519c61) ) /* unknown */
	ROM_LOAD( "9.bpr",      0x0000, 0x0100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) ) /* unknown */
ROM_END


ROM_START( bjtwinp )
	ROM_REGION( 0x80000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_BYTE( "ic76",  0x00000, 0x20000, CRC(c2847f0d) SHA1(2659e642586fcd199928d3f10ec300a1f13f2e3b) )
	ROM_LOAD16_BYTE( "ic75",  0x00001, 0x20000, CRC(dd8fdfce) SHA1(8b2da3b97acd07783b68ee270ae678dab6e538ec) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "ic35",  0x000000, 0x010000, CRC(45d67683) SHA1(004a85ecf34e97fad40195e7e20a11bf8cafe41e) ) /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "u1.ic32", 0x000000, 0x080000, CRC(b4960ba0) SHA1(4194bcd55fe48da08d5e951dc78daa457b1d76af) )
	ROM_LOAD( "u2.ic32", 0x080000, 0x080000, CRC(99ee571d) SHA1(85db0c9c3bdf5367dd4868daf9de40bdeeda9426) )
	ROM_LOAD( "u3.ic32", 0x100000, 0x080000, CRC(25720ffb) SHA1(361961e06467c7f4126e774a179087fe424160f5) ) /* Contains Gun Dealer + Dooyong logos + lots of adult pics! - these are used after the bonus game in this set.. */

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "u4.ic100", 0x000000, 0x080000, CRC(6501b1fb) SHA1(1c0832c3bb33aac1e5cd8845d77bc09222548ef8) )
	ROM_LOAD16_BYTE( "u5.ic100", 0x000001, 0x080000, CRC(8394e2ba) SHA1(bb921ccf1f5221611449ed3537d60395d8a1c1e9) )

	ROM_REGION( 0x140000, "oki1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "bottom.ic30",    0x040000, 0x80000, CRC(b5ef197f) SHA1(89d675f921dead585c2fef44105a7aea2f1f399c) ) /* all banked */
	ROM_LOAD( "top.ic30",       0x0c0000, 0x80000, CRC(ab50531d) SHA1(918987f01a8b1b007721d2b365e2b2fc536bd676) )

	ROM_REGION( 0x140000, "oki2", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "top.ic27",       0x040000, 0x80000, CRC(adb2f256) SHA1(ab7bb6683799203d0f46705f2fd241c6de914e77) ) /* all banked */
	ROM_LOAD( "bottom.ic27",    0x0c0000, 0x80000, CRC(6ebeb9e4) SHA1(b547b2fbcc0a35d6183dd4f19684b04839690a2b) )
ROM_END

ROM_START( atombjt ) // based off bjtwina set
	ROM_REGION( 0x40000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_BYTE( "22.u67",  0x00000, 0x20000, CRC(bead8c70) SHA1(2694bb0639f6b94119c21faf3810f00ef20b50da) )
	ROM_LOAD16_BYTE( "21.u66",  0x00001, 0x20000, CRC(73e3d488) SHA1(7deed6e3aeda1902b75746a9b0a2737632425867) )

	ROM_REGION( 0x200000, "gfxtemp", ROMREGION_ERASEFF ) // first half of these is the text tiles (repeated multiple times) second half the bgs
	ROM_LOAD32_BYTE( "23.u36",  0x000003, 0x80000, CRC(a3fb6b91) SHA1(477f5722a6bb23f089f32b677efbf69e9dce4b74) )
	ROM_LOAD32_BYTE( "24.u42",  0x000002, 0x80000, CRC(4c30e15f) SHA1(f92185743594e4e4573ac3f6c0c091802a08d5bd) )
	ROM_LOAD32_BYTE( "25.u39",  0x000001, 0x80000, CRC(ff1af60f) SHA1(4fe626c9d59ab9b945535b2f796f13adc900f1ed) )
	ROM_LOAD32_BYTE( "26.u45",  0x000000, 0x80000, CRC(6cc4e817) SHA1(70f2ab50e228a029d3157c94fe0a79e7aad010bd) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_COPY( "gfxtemp", 0x000000, 0x00000, 0x100000 )

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_COPY( "gfxtemp", 0x100000, 0x00000, 0x100000 )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD32_BYTE( "27.u86",  0x000003, 0x40000, CRC(5a853e5c) SHA1(dfa4e891f716bbf8a038a14a24276cb690f65230) )
	ROM_LOAD32_BYTE( "28.u85",  0x000002, 0x40000, CRC(41970bf6) SHA1(85b5677585dbdf96acabb59e6369d62d4c2f0e8e) )
	ROM_LOAD32_BYTE( "29.u84",  0x000001, 0x40000, CRC(59a7d610) SHA1(0dc39c09f7f55dbd12ddb5e2e4ba9d86a2ba24d8) )
	ROM_LOAD32_BYTE( "30.u83",  0x000000, 0x40000, CRC(9b2dfebd) SHA1(562ab22dc01a129e1b8c201665bbab0561254c2a) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "20.u16",    0x00000, 0x80000, CRC(71c74ff9) SHA1(3c22fb2976ab332e9bb1e208432ca985f274adac) )
ROM_END


ROM_START( nouryoku )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "ic76.1",  0x00000, 0x40000, CRC(26075988) SHA1(c3d0eef0417be3f78008c026915fd7e2fd589563) )
	ROM_LOAD16_BYTE( "ic75.2",  0x00001, 0x40000, CRC(75ab82cd) SHA1(fb828f87eebbe9d61766535efc18de9dfded110c) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "ic35.3",     0x000000, 0x010000, CRC(03d0c3b1) SHA1(4d5427c324e2141d0a953cc5133d10b327827e0b) )  /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "ic32.4",     0x000000, 0x200000, CRC(88d454fd) SHA1(c79c48d9b3602266499a5dd0b15fd2fb032809be) )  /* 16x16 tiles */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "ic100.5",    0x000000, 0x200000, CRC(24d3e24e) SHA1(71e38637953ec98bf308824aaef5628803aead21) )  /* Sprites */

	ROM_REGION( 0x140000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic30.6",     0x040000, 0x100000, CRC(feea34f4) SHA1(bee467e74dbad497c6f5f6b38b7e52001e767012) )  /* all banked */

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD( "ic27.7",     0x040000, 0x100000, CRC(8a69fded) SHA1(ee73f1789bcc672232606a4b3b28087fea1c5c69) )  /* all banked */
ROM_END

ROM_START( nouryokup )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "ic76.1",  0x00000, 0x40000, CRC(26075988) SHA1(c3d0eef0417be3f78008c026915fd7e2fd589563) )
	ROM_LOAD16_BYTE( "ic75.2",  0x00001, 0x40000, CRC(75ab82cd) SHA1(fb828f87eebbe9d61766535efc18de9dfded110c) )

	ROM_REGION( 0x010000, "fgtile", 0 )
	ROM_LOAD( "ic35.3",     0x000000, 0x010000, CRC(03d0c3b1) SHA1(4d5427c324e2141d0a953cc5133d10b327827e0b) )  /* 8x8 tiles */

	ROM_REGION( 0x200000, "bgtile", 0 )
	ROM_LOAD( "bg0.u1.ic32", 0x000000, 0x080000, CRC(1fec8e14) SHA1(7c596a455f829f31a801ea3d9fbb6a63810436a6) )
	ROM_LOAD( "bg1.u2.ic32", 0x080000, 0x080000, CRC(7b8ea3f0) SHA1(14722f7dcf5e86f32126ccb975f0a592c065f836) )
	ROM_LOAD( "bg2.u3.ic32", 0x100000, 0x080000, CRC(6f4eb408) SHA1(7f10676b7263bdf0fd5cfc4e5449f932984d4eb3) )
	ROM_LOAD( "bg3.u4.ic32", 0x180000, 0x080000, CRC(dea8c120) SHA1(c3f36fc0c97ee54f8ae3a55098c743980496eaa5) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "obj0even.u7.ic100", 0x000000, 0x080000, CRC(7966ce07) SHA1(231644bafd8970da2c57aeffc2fdaab60f4a512a) )
	ROM_LOAD16_BYTE( "obj0odd.u6.ic100",  0x000001, 0x080000, CRC(d4913a08) SHA1(49082a71c71176ff0e122844a40ac4f893342e45) )
	ROM_LOAD16_BYTE( "obj1even.u9.ic100", 0x100000, 0x080000, CRC(e01567e8) SHA1(69775752b61ce103d91e127f1fbf7c94b960b835) )
	ROM_LOAD16_BYTE( "obj1odd.u8.ic100",  0x100001, 0x080000, CRC(4a383085) SHA1(45351eb67c90936e500b527e9f93c1f70b67bd9a) )


	ROM_REGION( 0x140000, "oki1", 0 )   /* OKIM6295 samples */
	ROM_LOAD("soundpcm0.bottom.ic30", 0x040000, 0x080000, CRC(34ded136) SHA1(00fe1d6327483bb9e73802beca3ce6d808a20ceb) )
	ROM_LOAD("soundpcm1.top.ic30",    0x0c0000, 0x080000, CRC(a8d2abf7) SHA1(5619437e3e1f70f78cb2aeb2d619511be11e02e1) )

	ROM_REGION( 0x140000, "oki2", 0 )   /* OKIM6295 samples */
	ROM_LOAD("soundpcm2.top.ic27",    0x040000, 0x080000, CRC(29d0a15d) SHA1(a235eec225dd5006dd1f4e21d78fd647335f45dc) )
	ROM_LOAD("soundpcm3.bottom.ic27", 0x0c0000, 0x080000, CRC(c764e749) SHA1(8399d3b6807bd263eee607c5625618d19688b394) )
ROM_END

ROM_START( manybloc )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_BYTE( "1-u33.bin",  0x00001, 0x20000, CRC(07473154) SHA1(e67f637e74dfe5f1be558f963c0b3225254afe33) )
	ROM_LOAD16_BYTE( "2-u35.bin",  0x00000, 0x20000, CRC(04acd8c1) SHA1(3ef329e8d25565c7f7166f12137f4df5a057022f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80? CPU */
	ROM_LOAD( "3-u146.bin",      0x00000, 0x10000, CRC(7bf5fafa) SHA1(d17feca628775860d6c7019a9725bd40fbc5b7d7) )

	ROM_REGION( 0x80000, "fgtile", 0 )
	ROM_LOAD( "12-u39.bin",    0x000000, 0x10000, CRC(413b5438) SHA1(af366ce998ebe0d25255cc0cb1cd81689d3696ec) )    /* 8x8 tiles */

	ROM_REGION( 0x80000, "bgtile", 0 )
	ROM_LOAD( "5-u97.bin", 0x000000, 0x40000, CRC(536699e6) SHA1(13ec233f5e4f2a65ac7bc55511e988508269acd5) )
	ROM_LOAD( "4-u96.bin", 0x040000, 0x40000, CRC(28af2640) SHA1(08fa57de66cf58fe2256455538261c2d05d27e1e) )

	ROM_REGION( 0x080000, "sprites", 0 ) /* 16x16 sprite tiles */
	ROM_LOAD16_BYTE( "8-u54b.bin",  0x000000, 0x20000, CRC(03eede77) SHA1(2476a488bb0d39790b2cc7f261ddb973378022ff) )
	ROM_LOAD16_BYTE( "10-u86b.bin", 0x000001, 0x20000, CRC(9eab216f) SHA1(616f3ee2d06aa7151af634773a5e8633bff9588e) )
	ROM_LOAD16_BYTE( "9-u53b.bin",  0x040000, 0x20000, CRC(dfcfa040) SHA1(f1561defe9746afdb1a5327d0a4435a6f3e87a77) )
	ROM_LOAD16_BYTE( "11-u85b.bin", 0x040001, 0x20000, CRC(fe747dd5) SHA1(6ba57a45f4d77e2574de95d4a2f0718c601e7214) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "6-u131.bin",  0x00000, 0x40000, CRC(79a4ae75) SHA1(f7609d0ca18b4af8c5f37daa1795a7a6c6d768ae) )
	ROM_LOAD( "7-u132.bin",  0x40000, 0x40000, CRC(21db875e) SHA1(e1d96155b6d8825f7c449f276d02f9769258345d) )   /* banked */

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )    /* OKIM6295 samples */
	/* empty */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "u200.bpr",    0x0000, 0x0020, CRC(1823600b) SHA1(7011156ebcb815b176856bd67898ce655ea1b5ab) ) /* unknown */
	ROM_LOAD( "u7.bpr",      0x0020, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) ) /* unknown */
	ROM_LOAD( "u10.bpr",     0x0120, 0x0200, CRC(8e9b569a) SHA1(1d8d633fbeb72d5e55ad4b282df02e9ca5e240eb) ) /* unknown */
	ROM_LOAD( "u120.bpr",    0x0320, 0x0100, CRC(576c5984) SHA1(6e9b7f30de0d91cb766a62abc5888ec9af085a27) ) /* unknown */
ROM_END

/***************************************************************************

                                    Stagger I
(AFEGA 1998)

Parts:

1 MC68HC000P10
1 Z80
2 Lattice ispLSI 1032E

***************************************************************************/

ROM_START( stagger1 ) /* Japan only, with later (c) year of 1998 */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "2.bin", 0x000000, 0x020000, CRC(8555929b) SHA1(b405d81c2a45191111b1a4458ac6b5c0a129b8f1) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x020000, CRC(5b0b63ac) SHA1(239f793b6845a88d1630da790a2762da730a450d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1.bin", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "7.bin", 0x00000, 0x80000, CRC(048f7683) SHA1(7235b7dcfbb72abf44e60b114e3f504f16d29ebf) )
	ROM_LOAD16_BYTE( "6.bin", 0x00001, 0x80000, CRC(051d4a77) SHA1(664182748e72b3e44202caa20f337d02e946ca62) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x4 */
	ROM_LOAD( "4.bin", 0x00000, 0x80000, CRC(46463d36) SHA1(4265bc4d24ff64e39d9273965701c740d7e3fee0) )

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

/***************************************************************************

                            Red Hawk (c)1997 Afega

  CPU: MC68HC000P10 (68000)
Sound: Z0840006PSC (Z80)
       AD-65  rebadged OKI M6295
       PD2001 rebadged YM2151, 24 pin DIP
       KA3002 rebadged YM3012, 16 pin DIP
  OSC: 12.000MHz & 4.000MHz
  RAM: GM76C256CLL-70 x 6, HT6116-70 x 5, GM76C88AL-12 X 2
 Dips: 2 x 8 position
Other: Lattice pLSI 1032 x 2
       GAL22V10B x 2, GAL16V8B

+-----------------------------------------+
|   6116  YM3012 YM2151  M6295   5  4MHz  |
|VOL 1                                    |
|   Z80        pLSI1032  4                |
|                                76C88    |
|J       6116   76C256           76C88    |
|A       6116   76C256                    |
|M    2  76C256 76C256                    |
|M    3  76C256 76C256       GAL          |
|A SW1                                    |
|     68000-10       6116                 |
|                    6116                 |
|                                   6     |
|  SW2                 pLSI1032     7     |
|      12MHz GAL   GAL                    |
+-----------------------------------------+

***************************************************************************/

DRIVER_INIT_MEMBER(nmk16_state,redhawk)
{
	decryptcode( machine(), 23, 22, 21, 20,  19, 18, 16, 15,  14, 17, 13, 12,  11, 10, 9, 8,  7, 6, 5, 4,  3, 2, 1, 0 );
}

ROM_START( redhawk ) /* U.S.A., Canada & South America, (c) 1997 */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "2", 0x000000, 0x020000, CRC(3ef5f326) SHA1(e89c7c24a05886a14995d7c399958dc00ad35d63) )
	ROM_LOAD16_BYTE( "3", 0x000001, 0x020000, CRC(9b3a10ef) SHA1(d03480329b23474e5a9e42a75b09d2140eed4443) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1.bin", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "6", 0x000001, 0x080000, CRC(61560164) SHA1(d727ab2d037dab40745dec9c4389744534fdf07d) )
	ROM_LOAD16_BYTE( "7", 0x000000, 0x080000, CRC(66a8976d) SHA1(dd9b89cf29eb5557845599d55ef3a15f53c070a4) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "4", 0x000000, 0x080000, CRC(d6427b8a) SHA1(556de1b5ce29d1c3c54bb315dcaa4dd0848ca462) )

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( redhawke ) /* Excellent Co., Ldt license (no code scramble), (c) 1997 */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "rhawk2.bin", 0x000000, 0x020000, CRC(6d2e23b4) SHA1(54579d460844e022ab61f32bfec28f00f2d27140) )
	ROM_LOAD16_BYTE( "rhawk3.bin", 0x000001, 0x020000, CRC(5e0d6188) SHA1(c6ce8a3adf940893fcb6281348fdb0cdd65fe654) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1.bin", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "rhawk6.bin", 0x000001, 0x080000, CRC(3f980ab6) SHA1(2b9202555f09d99e3575123dfed415bfd815bb2e) )
	ROM_LOAD16_BYTE( "rhawk7.bin", 0x000000, 0x080000, CRC(0264ef54) SHA1(1124007538161dfc582f9c7692a20cdee459720c) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "rhawk4.bin", 0x000000, 0x080000, CRC(d79aa288) SHA1(b8598ab77d2019e5943b22f551e0a38eee5e52b6) )

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( redhawki )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "rhit-2.bin", 0x000000, 0x004000, CRC(30cade0e) SHA1(2123ca858bcaed5165739107ccc2830561af0b38) )
	ROM_CONTINUE(0x20000,0x4000)
	ROM_CONTINUE(0x10000,0x4000)
	ROM_CONTINUE(0x30000,0x4000)
	ROM_CONTINUE(0x08000,0x4000)
	ROM_CONTINUE(0x28000,0x4000)
	ROM_CONTINUE(0x18000,0x4000)
	ROM_CONTINUE(0x38000,0x4000)

	ROM_LOAD16_BYTE( "rhit-3.bin", 0x000001, 0x004000, CRC(37dbb3c2) SHA1(d1f8258f357b885d38f87d288f98046dbd7d56aa) )
	ROM_CONTINUE(0x20001,0x4000)
	ROM_CONTINUE(0x10001,0x4000)
	ROM_CONTINUE(0x30001,0x4000)
	ROM_CONTINUE(0x08001,0x4000)
	ROM_CONTINUE(0x28001,0x4000)
	ROM_CONTINUE(0x18001,0x4000)
	ROM_CONTINUE(0x38001,0x4000)

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1.bin", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "rhit-6.bin", 0x000001, 0x080000, CRC(7cbd5c60) SHA1(69bd728861ea5a02f514d5aed837b549f3c86019) )
	ROM_LOAD16_BYTE( "rhit-7.bin", 0x000000, 0x080000, CRC(bcb367c7) SHA1(a8f0527bf75a227cdfd98385549892fb16330aea) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "rhit-4.bin", 0x000000, 0x080000, CRC(aafb3cc4) SHA1(b5f6608c1e05470fdfb22e0a35a8a74974c4d3cf) )

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( redhawkb )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "rhb-1.bin", 0x000000, 0x020000, CRC(e733ea07) SHA1(b1ffeda633d5e701f0e97c79930a54d7b89a85c5) )
	ROM_LOAD16_BYTE( "rhb-2.bin", 0x000001, 0x020000, CRC(f9fa5684) SHA1(057ea3eebbaa1a208a72beef21b9368df7032ce1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1.bin", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "rhb-3.bin", 0x000000, 0x080000, CRC(0318d68b) SHA1(c773de7b6f9c706e62349dc73af4339d1a3f9af6) )
	ROM_LOAD( "rhb-4.bin", 0x080000, 0x080000, CRC(ba21c1ef) SHA1(66b0dee67acb5b3a21c7dba057be4093a92e10a9) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "rhb-5.bin", 0x000000, 0x080000, CRC(d0eaf6f2) SHA1(6e946e13b06df897a63e885c9842816ec908a709) )

	ROM_REGION( 0x080000, "fgtile", ROMREGION_ERASEFF )   /* Layer 1, 8x8x4 */

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( redhawkk )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "2", 0x000000, 0x020000, CRC(8c02e81d) SHA1(e79b0369adfe4111d7596df5270c1db8e3618ce5) )
	ROM_LOAD16_BYTE( "3", 0x000001, 0x020000, CRC(ab3597ee) SHA1(e9a2e085fa24cb2f500600b84ce2fe3924cf0827) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "6", 0x000001, 0x080000, CRC(6a0b8224) SHA1(07e68a6d13534ff51964d5abeb991508e8c8ea1a) )
	ROM_LOAD16_BYTE( "7", 0x000000, 0x080000, CRC(f4fa8211) SHA1(c3fed284127c9f837ab6cbd41d89ad827b423c9e) )

	ROM_REGION( 0x080000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "4", 0x000000, 0x080000, CRC(6255d6a1) SHA1(dcde3149c15717d624ca184454703a15db54bcde) )

	ROM_REGION( 0x080000, "fgtile", ROMREGION_ERASEFF )   /* Layer 1, 8x8x4 */

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "5", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END



/***************************************************************************

                  Guardian Storm / Sen Jin - Guardian Storm

(C) Afega 1998

  CPU: 68HC000FN10 (68000, 68 pin PLCC)
Sound: Z84C000FEC (Z80, 44 pin PQFP), AD-65 (OKI M6295),
       BS901 (YM2151, 24 pin DIP), BS901 (YM3012, 16 pin DIP)
  OSC: 12.000MHz (near 68000), 4.000MHz (Near Z84000)
  RAM: LH52B256-70LL x 6, HM61S16 x 7, UM6264BK-10L X 2 (6264* on some boards are 52B256)
 Dips: 2 x 8 position
Other: AFEGA AFI-GFSK (68 pin PLCC, located next to 68000)
       AFEGA AFI-GFLK (208 pin PQFP)

+-------------------------------------------------------------+
|      YM3012 4MHz                                            |
|             AD-65 AFEGA1.U95 +-------+ +-------+ AFEGA4.U112|
| VOL YM2151                   | AFEGA | |MC68000|            |
+-+   6116    Z80              |AF1-GFS| | FN10  | AFEGA5.U107|
  |   AFEGA7.U92               |       | |       |            |
+-+                AFEGA1.U4   +-------+ +-------+     52B256 |
|  6116   6116                                                |
|J 6116   6116                                 12MHz   52B256 |
|A                                                            |
|M        6116                                                |
|M        6116                                                |
|A            +--------+   52B256                             |
|             |        |                                      |
+-+  6264*    | AFEGA  |   52B256                             |
  |           |AF1-GFLK|                          AF1-SP.UC13 |
+-+  6264*    |        |   52B256                 AF1-B2.UC8  |
|             +--------+                          AF1-B1.UC3  |
|                          52B256                             |
|                                                             |
+-------------------------------------------------------------+

ROMS:
AFEGA7.U92   27C512 - Z80 sound CPU code
AFEGA1.U95   27C020 - OKI M6295 sound samples

AFEGA1.U4    27C512 - Graphics / text Layer

AFEGA4.U112  27C020 + M68000 program code
AFEGA5.U107  27C020 |

AFEGA3.UC13  ST M27C160  - Sprites
AF1-B2.UC8   MASK ROM read as 27C160  - Backgrounds
AF1-B1.UC3   MASK ROM read as 27C160  - Backgrounds

ROMS for Sen Jin:

AFEGA7.U92   27C512  - Z80 sound CPU code
AFEGA1.U95   27C2000 - OKI M6295 sound samples

GST-03.U4    27C512  - Graphics / text Layer

GST-04.U112  27C2000 + M68000 program code
GST-05.U107  27C2000 |

AF1-SP.UC13  MASK ROM read as 27C160  - Sprites
AF1-B2.UC8   MASK ROM read as 27C160  - Backgrounds
AF1-B1.UC3   MASK ROM read as 27C160  - Backgrounds

***************************************************************************/

DRIVER_INIT_MEMBER(nmk16_state,grdnstrm)
{
	decryptcode( machine(), 23, 22, 21, 20, 19, 18, 16, 17, 14, 15, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 );
}

ROM_START( grdnstrm )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega4.u112", 0x000000, 0x040000, CRC(2244713a) SHA1(41ae66a38931c12462ecae53e1e44c3420d0d235) )
	ROM_LOAD16_BYTE( "afega5.u107", 0x000001, 0x040000, CRC(5815c806) SHA1(f6b7809b2e3b29b89289ecc994909434fe34e10d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega7.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) /* MASK ROM (read as 27C020) */

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "afega3.uc13", 0x000000, 0x200000, CRC(0218017c) SHA1(5a8a4f07cd3f9dcf62455ddaceaec0cfba8c2de9) ) /* ST M27C160 EPROM */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) ) /* MASK ROM (read as 27C160) */
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega1.u4",  0x00000, 0x10000, CRC(9e7ef086) SHA1(db086bb2ceb11f3e24548aa131cc74fe79a2b516) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega1.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( grdnstrmk )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "gst-04.u112", 0x000000, 0x040000, CRC(922c931a) SHA1(1d1511033c8c424535a73f5c5bf58560a8b1842e) )
	ROM_LOAD16_BYTE( "gst-05.u107", 0x000001, 0x040000, CRC(d22ca2dc) SHA1(fa21c8ec804570d64f4b167b7f65fd5811435e46) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega7.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "afega_af1-sp.uc13", 0x000000, 0x200000, CRC(7d4d4985) SHA1(15c6c1aecd3f12050c1db2376f929f1a26a1d1cf) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) ) /* MASK ROM (read as 27C160) */
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "gst-03.u4",  0x00000, 0x10000, CRC(a1347297) SHA1(583f4da991eeedeb523cf4fa3b6900d40e342063) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega1.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( grdnstrmj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega_3.u112", 0x000000, 0x040000, CRC(e51a35fb) SHA1(acb733d0e5c9c54477d0475a64f53d68a84218c6) )
	ROM_LOAD16_BYTE( "afega_4.u107", 0x000001, 0x040000, CRC(cb10aa54) SHA1(bb0cb837b5651df4ff8f215854353631a39b730c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega7.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "afega_af1-sp.uc13", 0x000000, 0x200000, CRC(7d4d4985) SHA1(15c6c1aecd3f12050c1db2376f929f1a26a1d1cf) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) ) /* MASK ROM (read as 27C160) */
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "gst-03.u4",  0x00000, 0x10000, CRC(a1347297) SHA1(583f4da991eeedeb523cf4fa3b6900d40e342063) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega1.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

ROM_START( grdnstrmv ) /* Apples Industries license - Vertical version */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega2.u112", 0x000000, 0x040000, CRC(16d41050) SHA1(79b6621dccb286e5adf60c40690083a37746a4f9) )
	ROM_LOAD16_BYTE( "afega3.u107", 0x000001, 0x040000, CRC(05920a99) SHA1(ee77da303d6b766c529c426a836777827ac31676) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega7.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) /* MASK ROM (read as 27C020) */

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "afega6.uc13", 0x000000, 0x200000, CRC(9b54ff84) SHA1(9e120d85cf2fa899e6426dcb4302c8051746facc) ) /* ST M27C160 EPROM */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) ) /* MASK ROM (read as 27C160) */
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega1.u4",  0x00000, 0x10000, CRC(9e7ef086) SHA1(db086bb2ceb11f3e24548aa131cc74fe79a2b516) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega1.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

DRIVER_INIT_MEMBER(nmk16_state,grdnstrmg)
{
	// todo collapse to one bitswap
	decryptcode( machine(),23, 22, 21, 20,   19,  18, 13,17,   16,15, 14, 12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
	decryptcode( machine(),23, 22, 21, 20,   19,  18, 17,16,   14,15, 13, 12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
	decryptcode( machine(),23, 22, 21, 20,   19,  18, 17,15,   16,14, 13, 12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
	decryptcode( machine(),23, 22, 21, 20,   19,  18, 16,17,   15,14, 13, 12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );

}

ROM_START( grdnstrmg ) /* Germany */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "gs5_c1.uc1", 0x000001, 0x040000, CRC(c0263e4a) SHA1(8cae60bd59730aaba215f825016a780eced3a12d) )
	ROM_LOAD16_BYTE( "gs6_c2.uc9", 0x000000, 0x040000, CRC(ea363e4d) SHA1(2958dcddc409a11006beb52485975689182f3677) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "gs1_s1.uc14", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) //

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "gs8_br3.uc10", 0x000001, 0x080000, CRC(7b42a57a) SHA1(f45d9d86bc0388bbf220633e59f7a749c42e9046) )
	ROM_LOAD16_BYTE( "gs7_br1.uc3",  0x000000, 0x080000, CRC(e6794265) SHA1(39a6ebf2377aaf3a10b4c9c51607d81599eec35d) )
	ROM_LOAD16_BYTE( "gs10_br4.uc11",0x100001, 0x080000, CRC(1d3b57e1) SHA1(a2da598d6cbe257de5b66905a5ad9de90711ccc7) )
	ROM_LOAD16_BYTE( "gs9_br2.uc4",  0x100000, 0x080000, CRC(4d2c220b) SHA1(066067f7e80973ba0483559ac04f99292cc82dce) )

	// the other sets have larger regions here because they contain 2 sets of tiles in the roms, one for each orientation.
	// this set only contains the tile data for the required orientation.
	ROM_REGION( 0x200000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "gs10_cr5.uc15", 0x000000, 0x080000, CRC(2c8c23e3) SHA1(4c1a460dfc250f9aea77e2ddd82278ee816365be) )
	ROM_LOAD( "gs4_cr7.uc19",  0x080000, 0x080000, CRC(c3f6c908) SHA1(37873e28ca337d97ce301a4f79668fad8e6fca66) )
	ROM_LOAD( "gs8_cr1.uc6",   0x100000, 0x080000, CRC(dc0125f0) SHA1(f215b53378ec0366b1dc1614f19a67288ff7a865) )
	ROM_LOAD( "gs9_cr3.uc12",  0x180000, 0x080000, CRC(d8a0636b) SHA1(d278a4a19e6573e5aa02486a9b68b2e147b7b292) )

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "gs3_t1.uc2",  0x00000, 0x10000, CRC(88c423ef) SHA1(44e000f38312a1775a1207fd553eac1fe0f5e089) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "gs2_s2.uc18", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) ) //
ROM_END

ROM_START( redfoxwp2 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "u112", 0x000000, 0x040000, CRC(3f31600b) SHA1(6c56e36178effb60ec27dfcd205393e2cfac4ed6) ) /* No label */
	ROM_LOAD16_BYTE( "u107", 0x000001, 0x040000, CRC(daa44ab4) SHA1(7edaf8c7383dd31250478aeebc3247c525c75fef) ) /* No label */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "u92", 0x00000, 0x10000, CRC(864b55c2) SHA1(43475b05e35549ad301c3d4a25d4f4f0bcbe3f2c) ) /* Winbond W27E512-12 with no label */

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD( "afega_af1-sp.uc13", 0x000000, 0x200000, CRC(7d4d4985) SHA1(15c6c1aecd3f12050c1db2376f929f1a26a1d1cf) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) ) /* MASK ROM (read as 27C160) */
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) ) /* MASK ROM (read as 27C160) */

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "u4",  0x00000, 0x10000, CRC(19239401) SHA1(7876335dd97418bd9130dc894a517f3ceca20135) ) /* Winbond W27E512-12 with no label */

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega1.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

DRIVER_INIT_MEMBER(nmk16_state,redfoxwp2a)
{
	// todo collapse to one bitswap
	decryptcode( machine(),23, 22, 21, 20,   19, 18, 17, 16, /* */  13,15, 14, /* */ 12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
	decryptcode( machine(),23, 22, 21, 20,   19, 18, 17, 16, /* */14,15, /* */    13,12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
	decryptcode( machine(),23, 22, 21, 20,   19, 18, /* */ 16, 17, /* */    15,14,13,12,    11, 10, 9, 8,       7, 6, 5, 4,     3, 2, 1, 0 );
}

ROM_START( redfoxwp2a )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega_4.u112", 0x000000, 0x040000, CRC(e6e6682a) SHA1(1a70ca3881b4ecc6d329814ff1fdafce16550ca2) )
	ROM_LOAD16_BYTE( "afega_5.u107", 0x000001, 0x040000, CRC(2faa2ed6) SHA1(c6ca3ca0cff85379007a44648c6de87864095c2e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega_1.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* Sprites, 16x16x4 */ // not dumped, it is correct?
	ROM_LOAD( "afega_af1-sp.uc13", 0x000000, 0x200000, CRC(7d4d4985) SHA1(15c6c1aecd3f12050c1db2376f929f1a26a1d1cf) )

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */ // not dumped, it is correct?
	ROM_LOAD( "afega_af1-b2.uc8", 0x000000, 0x200000, CRC(d68588c2) SHA1(c5f397d74a6ecfd2e375082f82e37c5a330fba62) )
	ROM_LOAD( "afega_af1-b1.uc3", 0x200000, 0x200000, CRC(f8b200a8) SHA1(a6c43dd57b752d87138d7125b47dc0df83df8987) )

	ROM_REGION( 0x10000, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega_3.u4", 0x000000, 0x10000, CRC(64608687) SHA1(c13e55429171653437c8e8c7c8e9c6c5ffa2d2dc) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega_2.u95", 0x00000, 0x40000, CRC(e911ce33) SHA1(a29c4dea98a22235122303325c63c15fadd3431d) )
ROM_END

/***************************************************************************

Pop's Pop's by Afega (1999)

The pcb might be missing an eprom in a socket
  --- i just think it uses a generic PCB but no sprites in this case,.

1x 68k
1x z80
1x Ad65 (oki 6295)
1x OSC 12mhz (near 68k)
1x OSC 4mhz (near z80)
1x ym2151
1x Afega AF1-CFLK custom chip Smt
1x Afega AF1-CF5K custom chip socketed
2x dipswitch banks

****************************************************************************/

ROM_START( popspops )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega4.u112", 0x000000, 0x040000, CRC(db191762) SHA1(901fdc20374473127d694513d4291e29e65eafe8) )
	ROM_LOAD16_BYTE( "afega5.u107", 0x000001, 0x040000, CRC(17e0c48b) SHA1(833c61c4b3ee293b0bcddfa86dfa9c1014375115) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "afega1.u92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASEFF )   /* Sprites, 16x16x4 */
	/* no sprite roms? */

	ROM_REGION( 0x400000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega6.uc8", 0x000000, 0x200000, CRC(6d506c97) SHA1(4909c0b530f9526c8bf76e502c914ef10a50d1fc) )
	ROM_LOAD( "afega7.uc3", 0x200000, 0x200000, CRC(02d7f9de) SHA1(10102ffbf37a57afa300b01cb5067b7e672f4999) )

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega3.u4",  0x00000, 0x10000, CRC(f39dd5d2) SHA1(80d05d57a621b0063f63ce05be9314f718b3c111) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega2.u95", 0x00000, 0x40000, CRC(ecd8eeac) SHA1(849beba8f04cc322bb8435fa4c26551a6d0dec64) )
ROM_END

/****************************************************************************
Mang-chi by Afega

1x osc 4mhz
1x osc 12mhz
1x tmp68hc0000p-10
1x z80c006
1x AD65 (MSM6295)
1x CY5001 (YM2151 rebadged)
2x dipswitch
1x fpga
1x smd ASIC not marked

Dumped by Corrado Tomaselli
****************************************************************************/

ROM_START( mangchi )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega9.u112", 0x00000, 0x40000, CRC(0b1517a5) SHA1(50e307641759bb2a35aff56ef9598364740803a0) )
	ROM_LOAD16_BYTE( "afega10.u107", 0x00001, 0x40000, CRC(b1d0f33d) SHA1(68b5be3f7911f7299566c5bf5801e90099433613) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "sound.u92", 0x00000, 0x10000, CRC(bec4f9aa) SHA1(18fb2ee06892983c117a62b70cd72a98f60a08b6) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "afega6.uc11", 0x000000, 0x040000, CRC(979efc30) SHA1(227fe1e20137253aac04585d2bbf67091d032e56) )
	ROM_LOAD16_BYTE( "afega7.uc14", 0x000001, 0x040000, CRC(c5cbcc38) SHA1(86070a9598e80f90ec7892d623e1a975ccc68178) )

	ROM_REGION( 0x100000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega5.uc6",  0x000000, 0x80000, CRC(c73261e0) SHA1(0bb66aa315aaecb26169812cf47a6504a74f0db5) )
	ROM_LOAD( "afega4.uc1",  0x080000, 0x80000, CRC(73940917) SHA1(070305c81de959c9d00b6cf1cc20bbafa204976a) )

	ROM_REGION( 0x100000, "fgtile", ROMREGION_ERASEFF )   /* Layer 1, 8x8x4 */

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "afega2.u95", 0x00000, 0x40000, CRC(78c8c1f9) SHA1(eee0d03164a0ac0ddc5186ab56090320e9d33aa7) )
ROM_END

/***************************************************************************

                            Bubble 2000 (c)1998 Tuning

Bubble 2000
Tuning, 1998

CPU   : TMP68HC000P-10 (68000)
SOUND : Z840006 (Z80, 44 pin QFP), YM2151, OKI M6295
OSC   : 4.000MHZ, 12.000MHz
DIPSW : 8 position (x2)
RAM   : 6116 (x5, gfx related?) 6116 (x1, sound program ram), 6116 (x1, near rom3)
        64256 (x4, gfx related?), 62256 (x2, main program ram), 6264 (x2, gfx related?)
PALs/PROMs: None
Custom: Unknown 208 pin QFP labelled LTC2 (Graphics generator)
        Unknown 68 pin PLCC labelled LTC1 (?, near rom 2 and rom 3)
ROMs  :

Filename    Type        Possible Use
----------------------------------------------
rom01.92    27C512      Sound Program
rom02.95    27C020      Oki Samples
rom03.4     27C512      ? (located near rom 1 and 2 and near LTC1)
rom04.1     27C040   \
rom05.3     27C040    |
rom06.6     27C040    |
rom07.9     27C040    | Gfx
rom08.11    27C040    |
rom09.14    27C040    |
rom12.2     27C040    |
rom13.7     27C040   /

rom10.112   27C040   \  Main Program
rom11.107   27C040   /




***************************************************************************/

DRIVER_INIT_MEMBER(nmk16_state,bubl2000)
{
	decryptcode( machine(), 23, 22, 21, 20, 19, 18, 13, 14, 15, 16, 17, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 );
}

ROM_START( bubl2000 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "rom10.112", 0x00000, 0x20000, CRC(87f960d7) SHA1(d22fe1740217ac20963bd9003245850598ccecf2) )
	ROM_LOAD16_BYTE( "rom11.107", 0x00001, 0x20000, CRC(b386041a) SHA1(cac36e22a39b5be0c5cd54dce5c912ff811edb28) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "rom01.92", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) /* same as the other games on this driver */

	ROM_REGION( 0x080000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "rom08.11", 0x000000, 0x040000, CRC(519dfd82) SHA1(116b06f6e7b283a5417338f716bbaab6cfadb41d) )
	ROM_LOAD16_BYTE( "rom09.14", 0x000001, 0x040000, CRC(04fcb5c6) SHA1(7594fa6bf98fc01b8848473a222a621c7c9ff00d) )

	ROM_REGION( 0x300000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "rom06.6",  0x000000, 0x080000, CRC(ac1aabf5) SHA1(abce6ba381b189ab3ec703a8ef74bccbe10876e0) )
	ROM_LOAD( "rom07.9",  0x080000, 0x080000, CRC(69aff769) SHA1(89b98c1023710861e622c8a186b6ec48f5109d42) )
	ROM_LOAD( "rom13.7",  0x100000, 0x080000, CRC(3a5b7226) SHA1(1127740c5bc2f830d73a77c8831e1b0db6606375) )
	ROM_LOAD( "rom04.1",  0x180000, 0x080000, CRC(46acd054) SHA1(1bd7a1b6b2ce6a3daa8c92843c546beb377af8fb) )
	ROM_LOAD( "rom05.3",  0x200000, 0x080000, CRC(37deb6a1) SHA1(3a8a3d961800bb15fd389429b92fa1e5b5f416df) )
	ROM_LOAD( "rom12.2",  0x280000, 0x080000, CRC(1fdc59dd) SHA1(d38e21c878241b4315a36e0590397211ca63f2c4) )

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "rom03.4",  0x00000, 0x10000, CRC(f4c15588) SHA1(a21ae71c0a8c7c1df63f9905fd86303bc2d3991c) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "rom02.95", 0x00000, 0x40000, CRC(859a86e5) SHA1(7b51964227411a40aac54b9cd9ff64f091bdf2b0) )
ROM_END

/***************************************************************************

Hot Bubble
Afega, 1998

PCB Layout
----------

Bottom Board

|------------------------------------------|
| BS902  BS901   Z80                  4MHz |
|                                          |
|        6116    6295                      |
|                                   62256  |
|      6116                         62256  |
|      6116                                |
|J           6116           |------------| |
|A           6116           |   68000    | |
|M                          |------------| |
|M  DSW2   6264                            |
|A         6264                            |
|                                          |
|         |-------|                        |
|         |       |                        |
|         |       |                        |
| DSW1    |       |     62256   62256      |
|         |-------|                        |
|              6116     62256   62256      |
|12MHz         6116                        |
|------------------------------------------|
Notes:
      68000 - running at 12.000MHz
      Z80   - running at 4.000MHz
      62256 - 32K x8 SRAM
      6264  - 8K x8 SRAM
      6116  - 2K x8 SRAM
      BS901 - YM2151, running at 4.000MHz
      BS902 - YM3012
      6295  - OKI MSM6295 running at 1.000MHz [4/4], sample rate = 1000000 / 132
      *     - Unknown QFP208
      VSync - 56.2Hz (measured on 68000 IPL1)

Top Board

|---------------------------|
|                           |
|   S1     S2        T1     |
|                           |
|   CR5    CR7       C1     |
|                           |
|   CR6   +CR8       C2     |
|                           |
|          BR1       BR3    |
|                           |
|         +BR2      +BR4    |
|                           |
|  CR1     CR3     |------| |
|                  |  *   | |
|  CR2    +CR4     |      | |
|                  |------| |
|---------------------------|
Notes:
      * - Unknown PLCC68 IC
      + - Not populated

***************************************************************************/

ROM_START( hotbubl ) /* Korean release - Nude images of women for backgrounds */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "c1.uc1",  0x00001, 0x40000, CRC(7bb240e9) SHA1(99048fa275182c3da3bfb0dedd790f4b5858bd92) )
	ROM_LOAD16_BYTE( "c2.uc9",  0x00000, 0x40000, CRC(7917b95d) SHA1(0344bae9c373c5943e7693720e5e531bc2e0d7ee) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "s1.uc14", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) /* same as the other games on this driver */

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "br1.uc3",  0x000000, 0x080000, CRC(6fc18de4) SHA1(57b4823fc41637780f64eadd1ddf61db531a2599) )
	ROM_LOAD16_BYTE( "br3.uc10", 0x000001, 0x080000, CRC(bb677240) SHA1(d7a26bcd33d491cee441edda6d092a1d08308b0e) )

	ROM_REGION( 0x300000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "cr6.uc16",  0x100000, 0x080000, CRC(99d6523c) SHA1(0b628585d749e175d5a4dc600af1ba9cb936bfeb) )
	ROM_LOAD( "cr7.uc19",  0x080000, 0x080000, CRC(a89d9ce4) SHA1(5965b2b4b67bc91bc0e7474e593c7e1953b75adc) )
	ROM_LOAD( "cr5.uc15",  0x000000, 0x080000, CRC(65bd5159) SHA1(627ccc0ab131e643c3c52ee9bb41c7a85153c35e) )

	ROM_LOAD( "cr2.uc7",  0x280000, 0x080000, CRC(27ad6fc8) SHA1(00b1a5c5e1a245590b300b9baf71585d41813e3e) )
	ROM_LOAD( "cr3.uc12", 0x200000, 0x080000, CRC(c841a4f6) SHA1(9b0ee5623c87a0cfc63d3741a65d399bd6593f18) )
	ROM_LOAD( "cr1.uc6",  0x180000, 0x080000, CRC(fc9101d2) SHA1(1d5b8484264b6d73fe032946096a469226cce901) )

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "t1.uc2",  0x00000, 0x10000, CRC(ce683a93) SHA1(aeee2671051f1badf2255375cd7c5fa847d1746c) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "s2.uc18", 0x00000, 0x40000, CRC(401c980f) SHA1(e47710c47cfeecce3ccf87f845b219a9c9f21ee3) )
ROM_END

ROM_START( hotbubla ) /* Korean release - Nude images replaced with pictures of satellite dishes */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "6_c1.uc1",  0x00001, 0x40000, CRC(7c65bf47) SHA1(fe578d3336c5f437bfd1bc81bfe3763b12f3e63f) )
	ROM_LOAD16_BYTE( "7_c2.uc9",  0x00000, 0x40000, CRC(74eb11c3) SHA1(88aeb02c4088706a56b4c930ffe6fdfbc99031c6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "1_s1.uc14", 0x00000, 0x10000, CRC(5d8cf28e) SHA1(2a440bf5136f95af137b6688e566a14e65be94b1) ) /* same as the other games on this driver */

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "8_br1.uc3",  0x000000, 0x040000, CRC(7e132eff) SHA1(f3ec5750c73017f0a2eb87f6f39ab49e59d39711) )
	ROM_LOAD16_BYTE( "9_br3.uc10", 0x000001, 0x040000, CRC(22707728) SHA1(8a27aa2d1b6f902276c02bd7098526243661cff8) )

	ROM_REGION( 0x300000, "bgtile", 0 )   /* Layer 0, 16x16x8 */
	ROM_LOAD( "5_cr6.uc16",  0x100000, 0x080000, CRC(324429c5) SHA1(8cf90abf32697b269d4ec03b5b20bf4046fa53aa) )
	ROM_LOAD( "5_cr7.uc19",  0x080000, 0x080000, CRC(d293f1d0) SHA1(33c40c67bda477a2112cca4bfe9661edbcdf7689) )
	ROM_LOAD( "2_cr5.uc15",  0x000000, 0x080000, CRC(dd7e92de) SHA1(954f18887ac7737abce363985255a747c0de1fa2) )

	ROM_LOAD( "9_cr2.uc7",   0x280000, 0x080000, CRC(c5516087) SHA1(ae3692ecd7cd96b5d3653afb4c3a3b8f5931cbad) )
	ROM_LOAD( "10_cr3.uc12", 0x200000, 0x080000, CRC(312c38d8) SHA1(1e706b3e8b381083575ef4a01c615408940d5d0f) )
	ROM_LOAD( "8_cr1.uc6",   0x180000, 0x080000, CRC(7e2840b4) SHA1(333bf5631ee033ce528348d26888854eb1b063a0) )

	ROM_REGION( 0x10000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "2_t1.uc2",  0x00000, 0x10000, CRC(ce683a93) SHA1(aeee2671051f1badf2255375cd7c5fa847d1746c) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "1_s2.uc18", 0x00000, 0x40000, CRC(401c980f) SHA1(e47710c47cfeecce3ccf87f845b219a9c9f21ee3) )
ROM_END

ROM_START( dolmen ) /* Original source of the caveman concept for Bubble 2000 / Hot Bubble, much earlier and completely different hardware */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega8.uj3", 0x00000, 0x20000, CRC(f1b73e4c) SHA1(fe5bbd1e91d1a81744c373effbd96adbbc896133) )
	ROM_LOAD16_BYTE( "afega7.uj2", 0x00001, 0x20000, CRC(c91bda0b) SHA1(8c09e3020e72e8ab2ca3a3dad708d64f9bf75a4f) )

	ROM_REGION( 0x8000, "audiocpu", 0 )     /* Z80 Code */
	ROM_LOAD( "afega1.su6", 0x0000, 0x8000, CRC(166b53cb) SHA1(44864d1518205bdc445dc95e5825924f73d334b2) )   // 1111xxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "afega4.ub11", 0x00000, 0x80000, CRC(5a259393) SHA1(62c41ef4f398295d5cc1122c64487e12c4226ede) )
	ROM_LOAD16_BYTE( "afega5.ub13", 0x00001, 0x80000, CRC(7f6a683d) SHA1(ab7026906b68aa9f4d75b0e56564216727decfde) )

	ROM_REGION( 0x80000, "bgtile", 0 )    /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega9.ui20", 0x00000, 0x80000, CRC(b3fa7be6) SHA1(7ef8d902bd954960fbae727aae02dce9750f740e) )

	ROM_REGION( 0x20000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega6.uj11", 0x00000, 0x20000, CRC(13fa4415) SHA1(193524ebccbaae6b8c00893c42399c38cafdbd79) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "afega2.su12", 0x000000, 0x20000, CRC(1a2ce1c2) SHA1(ae6991fbfe57d35f32b541367d3b31244456713e) )
	ROM_RELOAD(              0x040000, 0x20000 )
	ROM_RELOAD(              0x080000, 0x20000 )
	ROM_RELOAD(              0x0c0000, 0x20000 )
	ROM_RELOAD(              0x020000, 0x20000 )
	ROM_RELOAD(              0x060000, 0x20000 )
	ROM_LOAD( "afega3.su13", 0x0a0000, 0x20000, CRC(d3531018) SHA1(940067a8634339258666c89319cb0e1b43f2af56) )
	ROM_CONTINUE(            0x0e0000, 0x20000 )
ROM_END


/***************************************************************************

Fire Hawk - ESD, 2001
---------------------

- To enter test mode, hold on button 1 at boot up


PCB Layout
----------

ESD-PROT-002
|------------------------------------------------|
|      FHAWK_S1.U40   FHAWK_S2.U36               |
|      6116     6295  FHAWK_S3.U41               |
|               6295                 FHAWK_G1.UC6|
|    PAL   Z80                       FHAWK_G2.UC5|
|    4MHz                             |--------| |
|                                     | ACTEL  | |
|J   6116             62256           |A54SX16A| |
|A   6116             62256           |        | |
|M                                    |(QFP208)| |
|M                                    |--------| |
|A     DSW1              FHAWK_G3.UC2            |
|      DSW2                           |--------| |
|      DSW3                           | ACTEL  | |
|                     6116            |A54SX16A| |
|                     6116            |        | |
|      62256                          |(QFP208)| |
| FHAWK_P1.U59                        |--------| |
| FHAWK_P2.U60  PAL                  62256  62256|
|                                                |
|12MHz 62256   68000                 62256  62256|
|------------------------------------------------|
Notes:
      68000 clock: 12.000MHz
        Z80 clock: 4.000MHz
      6295 clocks: 1.000MHz (both), sample rate = 1000000 / 132 (both)
            VSync: 56Hz

***************************************************************************/

ROM_START( firehawk )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_BYTE( "fhawk_p1.u59", 0x00001, 0x80000, CRC(d6d71a50) SHA1(e947720a0600d049b7ea9486442e1ba5582536c2) )
	ROM_LOAD16_BYTE( "fhawk_p2.u60", 0x00000, 0x80000, CRC(9f35d245) SHA1(5a22146f16bff7db924550970ed2a3048bc3edab) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "fhawk_s1.u40", 0x00000, 0x20000, CRC(c6609c39) SHA1(fe9b5f6c3ab42c48cb493fecb1181901efabdb58) )

	ROM_REGION( 0x200000, "sprites",0 ) /* Sprites, 16x16x4 */
	ROM_LOAD( "fhawk_g3.uc2", 0x00000, 0x200000,  CRC(cae72ff4) SHA1(7dca7164015228ea039deffd234778d0133971ab) )

	ROM_REGION( 0x400000, "bgtile", 0 ) /* Layer 0, 16x16x8 */
	ROM_LOAD( "fhawk_g1.uc6", 0x000000, 0x200000, CRC(2ab0b06b) SHA1(25362f6a517f188c62bac28b1a7b7b49622b1518) )
	ROM_LOAD( "fhawk_g2.uc5", 0x200000, 0x200000, CRC(d11bfa20) SHA1(15142004ab49f7f1e666098211dff0835c61df8d) )

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x040000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "fhawk_s2.u36", 0x00000, 0x40000, CRC(d16aaaad) SHA1(96ca173ca433164ed0ae51b41b42343bd3cfb5fe) )

	ROM_REGION( 0x040000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "fhawk_s3.u41", 0x00000, 0x40000, CRC(3fdcfac2) SHA1(c331f2ea6fd682cfb00f73f9a5b995408eaab5cf) )
ROM_END

ROM_START( firehawkv )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 Code */
	ROM_LOAD16_BYTE( "fire_hawk_cn1.u53", 0x00001, 0x80000, CRC(c09db3ec) SHA1(5beab9f837d8821fea1ceeac1be01c2c3ceaabf2) )
	ROM_LOAD16_BYTE( "fire_hawk_cn2.u59", 0x00000, 0x80000, CRC(68b0737c) SHA1(d8eac5b0f4023556f39ffb187f6d75270a5b782f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "fhawk_s1.u38", 0x00000, 0x20000, CRC(c6609c39) SHA1(fe9b5f6c3ab42c48cb493fecb1181901efabdb58) )

	ROM_REGION( 0x400000, "sprites",0 ) /* Sprites, 16x16x4 */
	ROM_LOAD( "rom.uc1", 0x000000, 0x200000, NO_DUMP ) // for vertical mode, missing
	ROM_LOAD( "fhawk_g3.uc2", 0x200000, 0x200000, BAD_DUMP CRC(cae72ff4) SHA1(7dca7164015228ea039deffd234778d0133971ab) ) // for horizontal mode, taken from above

	ROM_REGION( 0x800000, "bgtile", 0 ) /* Layer 0, 16x16x8 */
	ROM_LOAD( "rom.uc3", 0x000000, 0x200000, NO_DUMP ) // for vertical mode, missing
	ROM_LOAD( "rom.uc4", 0x400000, 0x200000, NO_DUMP ) // for vertical mode, missing
	ROM_LOAD( "fhawk_g1.uc6", 0x200000, 0x200000, BAD_DUMP CRC(2ab0b06b) SHA1(25362f6a517f188c62bac28b1a7b7b49622b1518) ) // for horizontal mode, taken from above
	ROM_LOAD( "fhawk_g2.uc5", 0x600000, 0x200000, BAD_DUMP CRC(d11bfa20) SHA1(15142004ab49f7f1e666098211dff0835c61df8d) ) // for horizontal mode, taken from above

	ROM_REGION( 0x00100, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	// Unused

	ROM_REGION( 0x040000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "fhawk_s2.u36", 0x00000, 0x40000, CRC(d16aaaad) SHA1(96ca173ca433164ed0ae51b41b42343bd3cfb5fe) )

	ROM_REGION( 0x040000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "fhawk_s3.u41", 0x00000, 0x40000, CRC(3fdcfac2) SHA1(c331f2ea6fd682cfb00f73f9a5b995408eaab5cf) )
ROM_END

/***************************************************************************

Spectrum 2000 (c) 2000 YONA Tech

  CPU: 68HC000FN10 (68000, 68 pin PLCC)
Sound: Z84C000FEC (Z80, 44 pin PQFP)
       AD-65 x 2 rebadged OKI M6295
  OSC: 12.000MHz & 4.000MHz
  RAM: IS61C256AH-20N x 6, HT6116-70 x 7, UM6164DK-12 X 2
 Dips: 2 x 8 position
Other: 208 pin PQFP labeled YONA Tech 2000 K (silkscreened on the PCB as LTC1)
       GAL16V8B (not dumped)

+-----------------------------------------------------+
|         6116    4MHz  AD-65  2.U101    29F1610.UC1  |
|VOL    1.U103          AD-65  3.U106                 |
|         Z80                           61C256        |
+-+                 6116                61C256        |
  | SW1 SW2         6116                61C256        |
+-+                                     61C256        |
|                                                     |
|J                           +--------+               |
|A                           |  YONA  |               |
|M                           |  Tech  |               |
|M      61C256 61C256        | 2000 K |               |
|A      5.U124 6.U120  GAL   |        |               |
|             +-------+      +--------+     6116      |
+-+           |MC68000|                     6116      |
  |           | FN10  |    6164                       |
+-+           |       |    6164          29F1610.UC2  |
|             +-------+                  29F1610.UC3  |
|      6116                                      12MHz|
|      6116                      4.U3                 |
+-----------------------------------------------------+

ROMs
YONATech1 is a TMS27C512
YONATech3 is a MX27C4000
YONATech2 & YONATech4 are TMS27C010A
YONATech5 & YONATech6 are TMS27C020

UC1, UC2 & UC3 are all Micronix MX29F1610ML 16Mb Flash roms

UC1, UC2 & UC3 have solder pads for both MX29F1610 Flash & 27C160 EPROMs

***************************************************************************/

DRIVER_INIT_MEMBER(nmk16_state,spec2k)
{
	decryptcode( machine(), 23, 22, 21, 20, 19, 18, 17, 13, 14, 15, 16, 12, 11, 10, 9,  8, 7,  6,  5,  4, 3,  2,  1,  0 );
}

ROM_START( spec2kh )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "yonatech5.u124", 0x00000, 0x40000, CRC(72ab5c05) SHA1(182a811982b89b8cda0677547ef0625c274f5c6b) )
	ROM_LOAD16_BYTE( "yonatech6.u120", 0x00001, 0x40000, CRC(7e44bd9c) SHA1(da59685be14a09ec037743fcec34fb293f7d588d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "yonatech1.u103", 0x00000, 0x10000, CRC(ef5acda7) SHA1(e55b36a1598ecbbbad984997d61599dfa3958f60) )

	ROM_REGION( 0x200000, "sprites",0 ) /* Sprites, 16x16x4 */
	ROM_LOAD( "u154.bin", 0x00000, 0x200000, CRC(f77b764e) SHA1(37e249bd4d7174c5232261880ce8debf42723716) ) /* UC1 MX29F1610ML Flash ROM */

	ROM_REGION( 0x400000, "bgtile", 0 ) /* Layer 0, 16x16x8 */
	ROM_LOAD( "u153.bin", 0x000000, 0x200000, CRC(a00bbf8f) SHA1(622f52ef50d52cdd5e6b250d68439caae5c13404) ) /* UC2 MX29F1610ML Flash ROM */
	ROM_LOAD( "u152.bin", 0x200000, 0x200000, CRC(f6423fab) SHA1(253e0791eb58efa1df42e9c74d397e6e65c8c252) ) /* UC3 MX29F1610ML Flash ROM */

	ROM_REGION( 0x20000, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "yonatech4.u3", 0x00000, 0x20000, CRC(5626b08e) SHA1(63207ed6b4fc8684690bf3fe1991a4f3babd73e8) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "yonatech2.u101", 0x00000, 0x20000, CRC(4160f172) SHA1(0478a5a4bbba115e6cfb5501aa55aa2836c963bf) )

	ROM_REGION( 0x080000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "yonatech3.u106", 0x00000, 0x80000, CRC(6644c404) SHA1(b7ad3f9f08971432d024ef8be3fa3140f0bbae67) )
ROM_END

ROM_START( spec2k )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u124", 0x00000, 0x40000, CRC(dbd6f65d) SHA1(0fad9836689fcbee60904ccad59a2a5be09f3139) )
	ROM_LOAD16_BYTE( "u120", 0x00001, 0x40000, CRC(be53e243) SHA1(38144b90a35ba144921824a0c4f133339e07f9a1) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "u103", 0x00000, 0x10000, CRC(f4e4fb10) SHA1(d19953d37e31fc753b50f0047d5be16f1f2daf09) )

	ROM_REGION( 0x200000, "sprites",0 ) /* Sprites, 16x16x4 */
	ROM_LOAD( "uc1", 0x00000, 0x200000, CRC(3139a213) SHA1(5ec4be0e27cbf1c4556ab10d7e1408ea64aa9e17) )

	ROM_REGION( 0x400000, "bgtile", 0 ) /* Layer 0, 16x16x8 */
	ROM_LOAD( "uc3", 0x000000, 0x200000, CRC(1d087122) SHA1(9e82c5f26c1387c6006cbd9248b333921388146c) )
	ROM_LOAD( "uc2", 0x200000, 0x200000, CRC(998dc05c) SHA1(cadf8bb0b8944372fbce9934b93684749ebc3ba0) )

	ROM_REGION( 0x20000, "fgtile", ROMREGION_ERASEFF )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "u3", 0x00000, 0x20000, CRC(921503b8) SHA1(dea6e9d47c9db83e79907bc0609a64176aff26bc) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "u101", 0x00000, 0x40000, CRC(d16aaaad) SHA1(96ca173ca433164ed0ae51b41b42343bd3cfb5fe) )

	ROM_REGION( 0x080000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "u106", 0x00000, 0x80000, CRC(65d61f3a) SHA1(a8f7ad61ae29a5c852820e5cbe886a8cd437634a) )
ROM_END

/***************************************************************************
    1995, Afega

    1x TMP68000P-10 (main)
    1x GOLDSTAR Z8400A (sound)
    1x AD-65 (equivalent to OKI6295)
    1x LATTICE pLSI 1032 60LJ A428A48
    1x oscillator 8.000MHz
    1x oscillator 12.000MHz

    1x 27256 (SU6)
    1x 27C010 (SU12)
    1x 27C020 (SU13)
    2x 27c4001 (UB11, UB13)
    3x 27C010 (UJ11, UJ12, UJ13)
    1x 27C4001 (UI20)

    1x JAMMA edge connector
    1x trimmer (volume)
***************************************************************************/

ROM_START( twinactn )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_BYTE( "afega.uj13", 0x00000, 0x20000, CRC(9187701d) SHA1(1da8d1e3969f60c7b0521cd22c723cb51619df9d) )
	ROM_LOAD16_BYTE( "afega.uj12", 0x00001, 0x20000, CRC(fe8cff9c) SHA1(a1a04deff9e2cb54c69601898cf4e5133c2bc437) )

	ROM_REGION( 0x8000, "audiocpu", 0 )     /* Z80 Code */
	ROM_LOAD( "afega.su6", 0x0000, 0x8000, CRC(3a52dc88) SHA1(87941987d34d93df6df9ff33ccfbd1f5d4a39c51) )   // 1111xxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "sprites", 0 )   /* Sprites, 16x16x4 */
	ROM_LOAD16_BYTE( "afega.ub11", 0x00000, 0x80000, CRC(287f20d8) SHA1(11faa36b97593c0b5cee70343750ae1ecd2f5b71) )
	ROM_LOAD16_BYTE( "afega.ub13", 0x00001, 0x80000, CRC(f525f819) SHA1(78ffcb709a3a900d3851392630a11ab58fc0bc75) )

	ROM_REGION( 0x80000, "bgtile", 0 )    /* Layer 0, 16x16x8 */
	ROM_LOAD( "afega.ui20", 0x00000, 0x80000, CRC(237c8f92) SHA1(bb3131b450bd78d03b789626a465fb9e7a4604a7) )

	ROM_REGION( 0x20000, "fgtile", 0 )    /* Layer 1, 8x8x4 */
	ROM_LOAD( "afega.uj11", 0x00000, 0x20000, CRC(3f439e92) SHA1(27e5b1b0aa3b13fa35e3f83793037314b2942aa2) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "afega.su12", 0x000000, 0x20000, CRC(91d665f3) SHA1(10b5b07ed28ea78b6d3493afc03e003a8468c007) )
	ROM_RELOAD(             0x040000, 0x20000 )
	ROM_RELOAD(             0x080000, 0x20000 )
	ROM_RELOAD(             0x0c0000, 0x20000 )
	ROM_RELOAD(             0x020000, 0x20000 )
	ROM_RELOAD(             0x060000, 0x20000 )
	ROM_LOAD( "afega.su13", 0x0a0000, 0x20000, CRC(30e1c306) SHA1(c859f11fd329793b11e96264e91c79a557b488a4) )
	ROM_CONTINUE(           0x0e0000, 0x20000 )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/


GAME( 1989, tharrier,  0,       tharrier, tharrier, nmk16_state, tharrier,   ROT270, "UPL",                          "Task Force Harrier", 0 )
GAME( 1989, tharrieru, tharrier,tharrier, tharrier, nmk16_state, tharrier,   ROT270, "UPL (American Sammy license)", "Task Force Harrier (US?)", 0 )

GAME( 1990, mustang,  0,        mustang,  mustang, driver_device,  0,        ROT0,   "UPL",                          "US AAF Mustang (25th May. 1990)", 0 )
GAME( 1990, mustangs, mustang,  mustang,  mustang, driver_device,  0,        ROT0,   "UPL (Seoul Trading license)",  "US AAF Mustang (25th May. 1990 / Seoul Trading)", 0 )

GAME( 1990, bioship,  0,        bioship,  bioship, driver_device,  0,        ROT0,   "UPL (American Sammy license)", "Bio-ship Paladin", 0 )
GAME( 1990, sbsgomo,  bioship,  bioship,  bioship, driver_device,  0,        ROT0,   "UPL",                          "Space Battle Ship Gomorrah", 0 )

GAME( 1990, vandyke,    0,       vandyke,  vandyke, driver_device, 0,        ROT270, "UPL",                          "Vandyke (Japan)",  0 )
GAME( 1990, vandykejal, vandyke, vandyke,  vandyke, driver_device, 0,        ROT270, "UPL (Jaleco license)",         "Vandyke (Jaleco, set 1)",  0 )
GAME( 1990, vandykejal2,vandyke, vandyke,  vandyke, driver_device, 0,        ROT270, "UPL (Jaleco license)",         "Vandyke (Jaleco, set 2)",  0 )
GAME( 1990, vandykeb,  vandyke, vandykeb, vandykeb, nmk16_state,   vandykeb, ROT270, "bootleg",                      "Vandyke (bootleg with PIC16c57)",  MACHINE_NO_SOUND )

GAME( 1991, blkheart, 0,        blkheart,  blkheart, driver_device, 0,       ROT0,   "UPL",                          "Black Heart", 0 )
GAME( 1991, blkheartj,blkheart, blkheart,  blkheart, driver_device, 0,       ROT0,   "UPL",                          "Black Heart (Japan)", 0 )

GAME( 1991, acrobatm, 0,        acrobatm, acrobatm, driver_device, 0,        ROT270, "UPL (Taito license)",          "Acrobat Mission", 0 )

GAME( 1992, strahl,   0,        strahl,   strahl, driver_device,   0,        ROT0,   "UPL",                          "Koutetsu Yousai Strahl (Japan set 1)", 0 )
GAME( 1992, strahla,  strahl,   strahl,   strahl, driver_device,   0,        ROT0,   "UPL",                          "Koutetsu Yousai Strahl (Japan set 2)", 0 )

GAME( 1991, tdragon,  0,        tdragon,       tdragon,      driver_device,  0,             ROT270, "NMK (Tecmo license)","Thunder Dragon (8th Jan. 1992, unprotected)", 0 )
GAME( 1991, tdragon1, tdragon,  tdragon_prot,  tdragon_prot, nmk16_state,    tdragon_prot,  ROT270, "NMK (Tecmo license)","Thunder Dragon (4th Jun. 1991, protected)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )

GAME( 1991, hachamf,    0,      hachamf_prot,  hachamf_prot, nmk16_state,    hachamf_prot,  ROT0,   "NMK",          "Hacha Mecha Fighter (19th Sep. 1991, protected)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND ) // lots of things wrong due to protection
GAME( 1991, hachamfb,   hachamf,hachamf,       hachamfb,     driver_device,  0,             ROT0,   "bootleg",      "Hacha Mecha Fighter (19th Sep. 1991, unprotected, bootleg Thunder Dragon conversion)", 0 ) // appears to be a Thunder Dragon conversion, could be bootleg?

GAME( 1992, macross,  0,        macross,  macross, nmk16_state,    nmk,      ROT270, "Banpresto",                    "Super Spacefortress Macross / Chou-Jikuu Yousai Macross", 0 )

GAME( 1993, gunnail,  0,        gunnail,  gunnail, nmk16_state,    nmk,      ROT270, "NMK / Tecmo",                  "GunNail (28th May. 1992)", 0 ) // Tecmo is displayed only when set to Japan
// a 1992 version of Gunnail exists, see https://www.youtube.com/watch?v=tf15Wz0zUiA  3:10

GAME( 1993, macross2, 0,        macross2, macross2, nmk16_state,  banked_audiocpu,        ROT0,   "Banpresto",                    "Super Spacefortress Macross II / Chou-Jikuu Yousai Macross II", MACHINE_NO_COCKTAIL )
GAME( 1993, macross2g, macross2,macross2, macross2, nmk16_state,  banked_audiocpu,        ROT0,   "Banpresto",                    "Super Spacefortress Macross II / Chou-Jikuu Yousai Macross II (GAMEST review build)", MACHINE_NO_COCKTAIL ) // Service switch pauses game

GAME( 1993, tdragon2, 0,        tdragon2, tdragon2, nmk16_state,  banked_audiocpu,        ROT270, "NMK",                          "Thunder Dragon 2 (9th Nov. 1993)", MACHINE_NO_COCKTAIL )
GAME( 1993, tdragon2a,tdragon2, tdragon2, tdragon2, nmk16_state,  banked_audiocpu,        ROT270, "NMK",                          "Thunder Dragon 2 (1st Oct. 1993)", MACHINE_NO_COCKTAIL )
GAME( 1993, bigbang,  tdragon2, tdragon2, tdragon2, nmk16_state,  banked_audiocpu,        ROT270, "NMK",                          "Big Bang (9th Nov. 1993)", MACHINE_NO_COCKTAIL )

/* arcadia was a name conflict to the Emerson Arcadia 2001 in mess */
GAME( 1994, arcadian, 0,        raphero,  raphero, nmk16_state,  banked_audiocpu,        ROT270, "NMK",                          "Arcadia (NMK)", 0 ) // 23rd July 1993 in test mode, (c)1994 on title screen
GAME( 1994, raphero,  arcadian, raphero,  raphero, nmk16_state,  banked_audiocpu,        ROT270, "NMK (Media Trading license)",  "Rapid Hero", 0 )    // ^^

/* both sets of both these games show a date of 9th Mar 1992 in the test mode, they look like different revisions so I doubt this is accurate */
GAME( 1992, sabotenb, 0,        bjtwin,   sabotenb, nmk16_state, nmk,      ROT0,   "NMK / Tecmo",                  "Saboten Bombers (set 1)", MACHINE_NO_COCKTAIL )
GAME( 1992, sabotenba,sabotenb, bjtwin,   sabotenb, nmk16_state, nmk,      ROT0,   "NMK / Tecmo",                  "Saboten Bombers (set 2)", MACHINE_NO_COCKTAIL )
GAME( 1992, cactus,   sabotenb, bjtwin,   sabotenb, nmk16_state, nmk,      ROT0,   "bootleg",                      "Cactus (bootleg of Saboten Bombers)", MACHINE_NO_COCKTAIL ) // PCB marked 'Cactus', no title screen

GAME( 1993, bjtwin,   0,        bjtwin,   bjtwin, nmk16_state,   bjtwin,   ROT270, "NMK",                          "Bombjack Twin (set 1)", MACHINE_NO_COCKTAIL )
GAME( 1993, bjtwina,  bjtwin,   bjtwin,   bjtwin, nmk16_state,   bjtwin,   ROT270, "NMK",                          "Bombjack Twin (set 2)", MACHINE_NO_COCKTAIL )
GAME( 1993, bjtwinp,  bjtwin,   bjtwin,   bjtwin, driver_device, 0,        ROT270, "NMK",                          "Bombjack Twin (prototype? with adult pictures)", MACHINE_NO_COCKTAIL ) // Genuine NMK PCB but GFX aren't encrypted
GAME( 1993, atombjt,  bjtwin,   atombjt,  bjtwin, driver_device, 0,        ROT270, "bootleg",                      "Atom (bootleg of Bombjack Twin)", MACHINE_NO_COCKTAIL | MACHINE_NOT_WORKING ) // some non-trivial mods to the gfx and sound hw


GAME( 1995, nouryoku, 0,        bjtwin,   nouryoku, nmk16_state,   nmk,      ROT0,   "Tecmo",                        "Nouryoku Koujou Iinkai", MACHINE_NO_COCKTAIL )
GAME( 1995, nouryokup,nouryoku, bjtwin,   nouryoku, driver_device, 0,        ROT0,   "Tecmo",                        "Nouryoku Koujou Iinkai (prototype)", MACHINE_NO_COCKTAIL ) // GFX aren't encrypted

/* Non NMK boards */

// these use the seibu sound system (sound / music stolen from Raiden) rather than the bootleggers copying the nmk004
GAME( 1990, mustangb, mustang,  mustangb, mustang, driver_device,  0,        ROT0,   "bootleg",                       "US AAF Mustang (bootleg)", 0 )
GAME( 1990, mustangb2,mustang,  mustangb, mustang, driver_device,  0,        ROT0,   "bootleg (TAB Austria)",         "US AAF Mustang (TAB Austria bootleg)", 0 ) // PCB and roms have TAB Austria stickers
GAME( 1991, tdragonb, tdragon,  tdragonb, tdragonb, nmk16_state,   tdragonb, ROT270, "bootleg",                       "Thunder Dragon (bootleg)", 0 )

// these are from Comad, based on the Thunder Dragon code?
GAME( 1992, ssmissin, 0,        ssmissin, ssmissin, nmk16_state, ssmissin, ROT270, "Comad",                         "S.S. Mission", MACHINE_NO_COCKTAIL )

GAME( 1996, airattck, 0,        ssmissin, airattck, nmk16_state, ssmissin, ROT270, "Comad",                         "Air Attack (set 1)", MACHINE_NO_COCKTAIL )
GAME( 1996, airattcka,airattck, ssmissin, airattck, nmk16_state, ssmissin, ROT270, "Comad",                         "Air Attack (set 2)", MACHINE_NO_COCKTAIL )

// afega & clones
GAME( 1995, twinactn, 0,        twinactn, twinactn, driver_device, 0,        ROT0,   "Afega",                             "Twin Action", 0 ) // hacked from USSAF Mustang

GAME( 1995, dolmen, 0,          twinactn, dolmen,   driver_device, 0,        ROT0,   "Afega",                             "Dolmen", 0 )

GAME( 1998, stagger1, 0,        stagger1, stagger1, driver_device, 0,        ROT270, "Afega",                             "Stagger I (Japan)", 0 )
GAME( 1997, redhawk,  stagger1, stagger1, stagger1, nmk16_state,   redhawk,  ROT270, "Afega (New Vision Ent. license)",   "Red Hawk (US)", 0 )
GAME( 1997, redhawki, stagger1, redhawki, stagger1, driver_device, 0,        ROT0,   "Afega (Hea Dong Corp license)",     "Red Hawk (Italy)", 0 ) // bootleg? strange scroll regs
GAME( 1997, redhawke, stagger1, stagger1, stagger1, driver_device, 0,        ROT270, "Afega (Excellent Co. license)",     "Red Hawk (Excellent Co., Ltd)", 0 )
GAME( 1997, redhawkk, stagger1, stagger1, stagger1, driver_device, 0,        ROT270, "Afega",                             "Red Hawk (Korea)", 0 )
GAME( 1997, redhawkb, stagger1, redhawkb, redhawkb, driver_device, 0,        ROT0,   "bootleg",                           "Red Hawk (bootleg)", 0 )

GAME( 1998, grdnstrm, 0,        grdnstrm, grdnstrm, driver_device, 0,        ORIENTATION_FLIP_Y, "Afega (Apples Industries license)", "Guardian Storm (horizontal, not encrypted)", 0 )
GAME( 1998, grdnstrmv,grdnstrm, grdnstrmk,grdnstrk, nmk16_state,   grdnstrm, ROT270,             "Afega (Apples Industries license)", "Guardian Storm (vertical)", 0 )
GAME( 1998, grdnstrmj,grdnstrm, grdnstrmk,grdnstrk, nmk16_state,   grdnstrmg, ROT270,            "Afega",                             "Sen Jing - Guardian Storm (Japan)", 0 )
GAME( 1998, grdnstrmk,grdnstrm, grdnstrmk,grdnstrk, nmk16_state,   grdnstrm, ROT270,             "Afega",                             "Jeon Sin - Guardian Storm (Korea)", 0 )
GAME( 1998, redfoxwp2,grdnstrm, grdnstrmk,grdnstrk, nmk16_state,   grdnstrm, ROT270,             "Afega",                             "Red Fox War Planes II (China, set 1)", 0 )
GAME( 1998, redfoxwp2a,grdnstrm,grdnstrmk,grdnstrk, nmk16_state,  redfoxwp2a,ROT270,             "Afega",                             "Red Fox War Planes II (China, set 2)", 0 )
GAME( 1998, grdnstrmg,grdnstrm, grdnstrmk,grdnstrk, nmk16_state,   grdnstrmg,ROT270,             "Afega",                             "Guardian Storm (Germany)", 0 )

// is there a 'bubble 2000' / 'hot bubble' version with Afega copyright, or is the only Afega release dolmen above, this seems like a sequel, not a clone?
GAME( 1998, bubl2000, 0,        popspops, bubl2000, nmk16_state,   bubl2000, ROT0,               "Afega (Tuning license)",                            "Bubble 2000", 0 ) // on a tuning board (bootleg?)
GAME( 1998, hotbubl,  bubl2000, popspops, bubl2000, nmk16_state,   bubl2000, ROT0,               "Afega (Pandora license)",                           "Hot Bubble (Korea, with adult pictures)" , 0 ) // on an afega board ..
GAME( 1998, hotbubla, bubl2000, popspops, bubl2000, nmk16_state,   bubl2000, ROT0,               "Afega (Pandora license)",                           "Hot Bubble (Korea)" , 0 ) // on an afega board ..

GAME( 1999, popspops, 0,        popspops, popspops, nmk16_state,   grdnstrm, ROT0,               "Afega",                             "Pop's Pop's", 0 )

GAME( 2000, mangchi,  0,        popspops, mangchi, nmk16_state,    bubl2000, ROT0,               "Afega",                             "Mang-Chi", 0 )

// these two are very similar games, but the exact parent/clone relationship is unknown
GAME( 2000, spec2k,   0,       spec2k,   spec2k, nmk16_state,     spec2k,   ROT270,             "Yona Tech",             "Spectrum 2000 (vertical)", MACHINE_IMPERFECT_GRAPHICS ) // the ships sometimes scroll off the screen if you insert a coin during the attract demo?  verify it doesn't happen on real hw(!)
GAME( 2000, spec2kh,  spec2k,  spec2k,   spec2k, nmk16_state,     spec2k,   ORIENTATION_FLIP_Y, "Yona Tech",             "Spectrum 2000 (horizontal, buggy) (Europe)", 0 ) // this has odd bugs even on real hardware, eg glitchy 3 step destruction sequence of some larger enemies
GAME( 2001, firehawk, spec2k,  firehawk, firehawk, driver_device, 0,        ORIENTATION_FLIP_Y, "ESD",                   "Fire Hawk (horizontal)", 0 )
GAME( 2001, firehawkv,spec2k,  firehawk, firehawkv,driver_device, 0,        ORIENTATION_FLIP_Y, "ESD",                   "Fire Hawk (switchable orientation)", MACHINE_NOT_WORKING ) // incomplete dump, vertical mode gfx not dumped

// bee-oh board - different display / interrupt timing to others?
GAME( 1991, manybloc, 0,        manybloc, manybloc, nmk16_state, tharrier,  ROT270,             "Bee-Oh",                "Many Block", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND )
