// license:BSD-3-Clause
// copyright-holders:David Haywood, Mike Coates
/***************************************************************************

  Snow Brothers (Toaplan) / SemiCom Hardware
  uses Kaneko's Pandora sprite chip (also used in DJ Boy, Air Buster ..)

Snow Bros Nick & Tom
Toaplan, 1990

    PCB Layout
    ----------
    MIN16-02

    |------------------------------------------|
    | VOL     YM3812  6116  4464  4464         |
    | LA4460  YM3014        4464  4464         |
    |       458   SBROS-4.29         SBROS1.40 |
    |   2003       Z80B     PANDORA            |
    |J                    D41101C-1 LS07  LS32 |
    |A SBROS-3A.5 SBROS-2A.6        LS139 LS174|
    |M                  LS245 LS74  LS04  16MHz|
    |M   6264     6264  F32   LS74  LS74       |
    |A      68000       LS20  F138  LS04  12MHz|
    |                   LS04  LS148 LS251 LS00 |
    | LS273 LS245 LS245 LS158 LS257 LS257 LS32 |
    |                                          |
    | LS273  6116  6116 LS157   DSW2  DSW1     |
    |------------------------------------------|

    Notes:
           68k clock: 8.000MHz
          Z80B clock: 6.000MHz
        YM3812 clock: 3.000MHz
               VSync: 57.5Hz
               HSync: 15.68kHz

  driver by Mike Coates

  Hyper Pacman addition by David Haywood
   + some bits by Nicola Salmoria

 !!DO NOT TRUST THE SERVICE MODES FOR DIPSWITCH SETTINGS!!

Stephh's notes (hyperpac):

  - According to the "Language" Dip Switch, this game is a Korean game.
     (although the Language Dipswitch doesn't affect language, but yes
      I believe SemiCom to be a Korean Company)
  - There is no "cocktail mode", nor way to flip the screen.

Notes:

Cookie & Bibi 3
This game is quite buggy.  The test mode is incomplete and displays garbage
on the 'Dipswitch settings' screens, and during some of the attract mode
scenes the credit counter is not updated when you insert coins until the next
scene.  Both these bugs are verified as occurring on the original hardware.

Honey Doll / Twin Adventure

These appear to have clipping problems on the left / right edges, but this
may be correct, the sprites which should be drawn there are simply blanked
out of the sprite list at that point.. (verify on real hw)

Ma Cheon Ru

The electrified maze + ball minigame appears unresponsive to controls, this
is because it actually requires you to move the joysticks in a circular
motion through all 8 directions at a very even speed, a task which is
practically impossible to perform on keyboard, and not even that easy with
a joystick.  This is not an emulation bug.


***************************************************************************/

#include "emu.h"
#include "snowbros.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h" // for semicom mcu
#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"

#include "speaker.h"


void snowbros_state::snowbros_flipscreen_w(uint8_t data)
{
	m_pandora->flip_screen_set(!BIT(data, 7));
}


void snowbros_state::bootleg_flipscreen_w(uint8_t data)
{
	flip_screen_set(~data & 0x80);
}


uint32_t snowbros_state::screen_update_snowbros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* This clears & redraws the entire screen each pass */
	bitmap.fill(0xf0, cliprect);
	m_pandora->update(bitmap, cliprect);
	return 0;
}


void snowbros_state::screen_vblank_snowbros(int state)
{
	// rising edge
	if (state)
	{
		m_pandora->eof();
	}
}



void snowbros_state::snowbros_irq4_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}

void snowbros_state::snowbros_irq3_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(3, CLEAR_LINE);
}

void snowbros_state::snowbros_irq2_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(snowbros_state::snowbros_irq)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(2, ASSERT_LINE);

	if(scanline == 128)
		m_maincpu->set_input_line(3, ASSERT_LINE);

	if(scanline == 32)
		m_maincpu->set_input_line(4, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(snowbros_state::snowbros3_irq)
{
	int status = m_oki->read();
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(2, ASSERT_LINE);

	if(scanline == 128)
		m_maincpu->set_input_line(3, ASSERT_LINE);

	if(scanline == 32)
		m_maincpu->set_input_line(4, ASSERT_LINE);

	if (m_sb3_music_is_playing)
	{
		if ((status&0x08)==0x00)
		{
			m_oki->write(0x80|m_sb3_music);
			m_oki->write(0x00|0x82);
		}

	}
	else
	{
		if ((status&0x08)==0x08)
		{
			m_oki->write(0x40);     /* Stop playing music */
		}
	}

}

uint16_t snowbros_state::toto_read(offs_t offset, uint16_t mem_mask)
{
	int pc = m_maincpu->pc();
	if ((pc!= 0x3f010) && (pc!= 0x38008)) printf("toto prot %08x %04x\n", pc, mem_mask);
	return 0x0700;
}

/* Snow Bros Memory Map */

void snowbros_state::snowbros_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x300001, 0x300001).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x400000, 0x400000).w(FUNC(snowbros_state::snowbros_flipscreen_w));
	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");
	map(0x600000, 0x6001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w));
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

void snowbros_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void snowbros_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x04, 0x04).w("soundlatch2", FUNC(generic_latch_8_device::write)); // goes back to the main CPU, checked during boot
}


/* Semicom AT89C52 MCU */

// probably not endian safe
void snowbros_state::prot_p0_w(uint8_t data)
{
	uint16_t word = m_hyperpac_ram[m_semicom_prot_base + m_semicom_prot_offset];
	word = (word & 0xff00) | (data << 0);
	m_hyperpac_ram[m_semicom_prot_base + m_semicom_prot_offset] = word;
}

// probably not endian safe
void snowbros_state::prot_p1_w(uint8_t data)
{
	uint16_t word = m_hyperpac_ram[m_semicom_prot_base + m_semicom_prot_offset];
	word = (word & 0x00ff) | (data << 8);
	m_hyperpac_ram[m_semicom_prot_base + m_semicom_prot_offset] = word;
}

void snowbros_state::prot_p2_w(uint8_t data)
{
	// offset
	m_semicom_prot_offset = data;
}

/* Winter Bobble - bootleg GFX chip */

void snowbros_state::wintbob_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x300001, 0x300001).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x400000, 0x400000).w(FUNC(snowbros_state::bootleg_flipscreen_w));
	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");
	map(0x600000, 0x6001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).ram().share("spriteram16b");
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

/* Honey Doll */

void snowbros_state::honeydol_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram().share("hyperpac_ram");
	map(0x200000, 0x200001).nopw();    /* ? */
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x400000, 0x400001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x500000, 0x500001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0x600000, 0x600001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
	map(0x800000, 0x800001).nopw();    /* ? */
	map(0x900000, 0x900001).portr("DSW1");
	map(0x900002, 0x900003).portr("DSW2");
	map(0x900004, 0x900005).portr("SYSTEM");
	map(0xa00000, 0xa007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb01fff).ram().share("spriteram16b");
}

void snowbros_state::honeydol_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xe010, 0xe010).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void snowbros_state::honeydol_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));                                // not connected?
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x04, 0x04).nopw(); // still written but never actually read by the main CPU
}

/* Twin Adventure */

void snowbros_state::twinadv_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x300001, 0x300001).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x400000, 0x400000).w(FUNC(snowbros_state::bootleg_flipscreen_w));

	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");
	map(0x600000, 0x6001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).ram().share("spriteram16b");
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

void snowbros_state::twinadv_oki_bank_w(uint8_t data)
{
	int bank = (data &0x02)>>1;

	if (data&0xfd) logerror ("Unused bank bits! %02x\n",data);

	m_oki->set_rom_bank(bank);
}

void snowbros_state::twinadv_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x02).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x02, 0x02).w("soundlatch2", FUNC(generic_latch_8_device::write)); // back to 68k?
	map(0x04, 0x04).w(FUNC(snowbros_state::twinadv_oki_bank_w)); // oki bank?
	map(0x06, 0x06).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/* SemiCom Memory Map

the SemiCom games have slightly more ram and are protected
sound hardware is also different

*/

void snowbros_state::hyperpac_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram().share("hyperpac_ram");
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
//  map(0x400000, 0x400001) ???
	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");

	map(0x600000, 0x6001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w));
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

void snowbros_state::hyperpac_sound_map(address_map &map)
{
	map(0x0000, 0xcfff).rom();
	map(0xd000, 0xd7ff).ram();
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf002, 0xf002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf008, 0xf008).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

/* Same volume used for all samples at the Moment, could be right, we have no
   way of knowing .. */
uint16_t snowbros_state::sb3_sound_r()
{
	return 0x0003;
}

void snowbros_state::sb3_play_music(int data)
{
	uint8_t *snd;

	/* sample is actually played in interrupt function so it loops */
	m_sb3_music = data;

	switch (data)
	{
		case 0x23:
		case 0x26:
		snd = memregion("oki")->base();
		memcpy(snd+0x20000, snd+0x80000+0x00000, 0x20000);
		m_sb3_music_is_playing = 1;
		break;

		case 0x24:
		snd = memregion("oki")->base();
		memcpy(snd+0x20000, snd+0x80000+0x20000, 0x20000);
		m_sb3_music_is_playing = 1;
		break;

		case 0x25:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		snd = memregion("oki")->base();
		memcpy(snd+0x20000, snd+0x80000+0x40000, 0x20000);
		m_sb3_music_is_playing = 1;
		break;

		case 0x2e:
		m_sb3_music_is_playing = 0;
		break;
	}
}

void snowbros_state::sb3_play_sound (int data)
{
	int status = m_oki->read();

	if ((status&0x01)==0x00)
	{
		m_oki->write(0x80|data);
		m_oki->write(0x00|0x12);
	}
	else if ((status&0x02)==0x00)
	{
		m_oki->write(0x80|data);
		m_oki->write(0x00|0x22);
	}
	else if ((status&0x04)==0x00)
	{
		m_oki->write(0x80|data);
		m_oki->write(0x00|0x42);
	}


}

void snowbros_state::sb3_sound_w(uint16_t data)
{
	if (data == 0x00fe)
	{
		m_sb3_music_is_playing = 0;
		m_oki->write(0x78);       /* Stop sounds */
	}
	else /* the alternating 0x00-0x2f or 0x30-0x5f might be something to do with the channels */
	{
		data = data>>8;

		if (data <= 0x21)
		{
			sb3_play_sound(data);
		}

		if (data>=0x22 && data<=0x31)
		{
			sb3_play_music(data);
		}

		if ((data>=0x30) && (data<=0x51))
		{
			sb3_play_sound(data-0x30);
		}

		if (data>=0x52 && data<=0x5f)
		{
			sb3_play_music(data-0x30);
		}

	}
}



void snowbros_state::snowbros3_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x300000, 0x300001).r(FUNC(snowbros_state::sb3_sound_r)); // ?
	map(0x300000, 0x300001).w(FUNC(snowbros_state::sb3_sound_w));  // ?
	map(0x400000, 0x400000).w(FUNC(snowbros_state::bootleg_flipscreen_w));
	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");
	map(0x600000, 0x6003ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x7021ff).ram().share("spriteram16b");
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

/* Final Tetris */

void snowbros_state::finalttr_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram().share("hyperpac_ram");
	map(0x300001, 0x300001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
//  map(0x400000, 0x400001) ???

	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");

	map(0x600000, 0x6001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w));
	map(0x800000, 0x800001).w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}


// Yutnori protection.
// The sequence MEN is sent to the protection device, followed by the code request (4 bytes in all).
// After each byte, a number of NOPs are executed to give the device time to catch up.
// After the 4th byte, the code reads the device to get its response.
uint16_t snowbros_state::yutnori_prot_r()
{
	switch(m_yutnori_prot_val) // the 4th byte
	{
		case 0x33: // C3B6
			return 0xcc;
		case 0x60: // 4878
			return 0xf9;
		case 0xb8: // D820
			return 0x74;
	}
	logerror("%s: Unhandled protection sequence found: %02X\n", machine().describe_context(), m_yutnori_prot_val);
	return 0;
}

void snowbros_state::yutnori_prot_w(uint16_t data)
{
	m_yutnori_prot_val = data;
}

void snowbros_state::yutnori_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	// 0x100000  clr.w on startup

	map(0x200000, 0x200001).rw(FUNC(snowbros_state::yutnori_prot_r),FUNC(snowbros_state::yutnori_prot_w)); // protection

	map(0x300000, 0x300001).portr("DSW1");
	map(0x300002, 0x300003).portr("DSW2");
	map(0x300004, 0x300005).portr("SYSTEM");

	// could be one of the OKIs? but gets value to write from RAM, always seems to be 0?
	map(0x30000c, 0x30000d).nopw();
	map(0x30000e, 0x30000f).nopr(); //.r(FUNC(snowbros_state::yutnori_unk_r)); // ??

//  map(0x400000, 0x400001).w("watchdog", FUNC(watchdog_timer_device::reset16_w)); // maybe?
	map(0x400000, 0x400001).noprw();

	map(0x500000, 0x5001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x600000, 0x601fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w));

	map(0x700000, 0x70ffff).ram();

	map(0x800000, 0x800001).nopr().w(FUNC(snowbros_state::snowbros_irq4_ack_w));  /* IRQ 4 acknowledge */
	map(0x900000, 0x900001).nopr().w(FUNC(snowbros_state::snowbros_irq3_ack_w));  /* IRQ 3 acknowledge */
	map(0xa00000, 0xa00001).nopr().w(FUNC(snowbros_state::snowbros_irq2_ack_w));  /* IRQ 2 acknowledge */
}

static INPUT_PORTS_START( snowbros )
	PORT_START("DSW1")  /* 500001 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) )       PORT_DIPLOCATION("SW1:1") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x01, "America (Romstar license)" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Must be low or game stops! */
													/* probably VBlank */

	PORT_START("DSW2")  /* 500003 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100k and every 200k" )
	PORT_DIPSETTING(    0x0c, "100k Only" )
	PORT_DIPSETTING(    0x08, "200k Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( snowbroj )
	PORT_INCLUDE(snowbros)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") /* Listed as "NOT USE" in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ballboy3p )
	PORT_INCLUDE(snowbros)

	PORT_MODIFY("DSW1") // on the PCB in place of the dips there's the plug for the controls of the 3rd player
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

static INPUT_PORTS_START( honeydol )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Show Girls" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Level_Select ) ) /* Up & Down to set level, then punch to start */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Slide Show On Boot-up" )  /* Joystick to scroll. Seems to happen once at boot up, then the game auto starts */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    /* Must be low or game stops! */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Timer Speed" )     /* Based on Normal Difficulty, Stage 1-1 first try */
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) ) /* About 40 Seconds */
	PORT_DIPSETTING(      0x0008, "Fast" )            /* About 35 Seconds */
	PORT_DIPSETTING(      0x0004, "Faster" )          /* About 30 Seconds */
	PORT_DIPSETTING(      0x0000, "Fastest" )         /* About 20 Seconds */
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0030, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Max Vs Round" )
	PORT_DIPSETTING(      0x0080, "3" ) /* 44 Seconds each */
	PORT_DIPSETTING(      0x0000, "1" ) /* 89 Seconds */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( twinadv )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Level_Select ) ) /* P1 Button 2 to advance, P1 Button 1 to start, starts game with 10 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))    /* Must be low or game stops! */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0008, 0x0008, "Ticket Mode #1" ) /* Shows on title screen "EVERY 4 GAMES = 1 TICKET" same as 0x0040 below? */
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Ticket Mode #2" ) /* Shows on title screen "EVERY 4 GAMES = 1 TICKET" same as 0x0008 above? */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80,   0x0080, DEF_STR( Free_Play ) ) /* Always shows 24 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( 4in1boot )
	PORT_START("DSW1")  /* 500001 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x78, 0x78, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x78, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x68, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Must be low or game stops! */
													/* probably VBlank */

	PORT_START("DSW2")  /* 500003 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( hyperpac )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2")   // "Language" in the "test mode"
	PORT_DIPSETTING(      0x0002, "3" )                         // "Korean"
	PORT_DIPSETTING(      0x0000, "5" )                         // "English"
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )          // DEF_STR( Very_Hard )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0000, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib2 )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Max Vs Round" )      PORT_DIPLOCATION("SW1:2")   // "Language" in the "test mode"
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )          // DEF_STR( Very_Hard )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0000, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib2c )
	PORT_INCLUDE(cookbib2)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Language ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( English ) ) // Korean in test mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) ) // English in test mode
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib3 )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8") /* Will go into negative credits and cause graphics issues */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sutjarod )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) // probably all buttons, not a joystick
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( moremore )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// don't trust the test mode! <-- Verified via actual game play: Demo Sounds, Coinage & Free Play
static INPUT_PORTS_START( mcheonru )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) /* Duplicate */
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) ) /* Duplicate */
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Tile Type" )    PORT_DIPLOCATION("SW2:2") /* Does NOT show up in service mode! */
	PORT_DIPSETTING(      0x0002, "Picture" )
	PORT_DIPSETTING(      0x0000, "Traditional" )
	PORT_DIPNAME( 0x0004, 0x0004, "Mini Game Frequency" )    PORT_DIPLOCATION("SW2:3") /* Does NOT show up in service mode! */
	PORT_DIPSETTING(      0x0004, "Every 3 Rounds" )
	PORT_DIPSETTING(      0x0000, "Every Round" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( twinkle )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7") // this is what service mode lists, but I don't trust it.
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2") // unlisted in service mode
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPNAME( 0x0010, 0x0010, "Pellet Animations" )     PORT_DIPLOCATION("SW2:5") // unlisted in service mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// don't trust the test mode! <-- Verified via actual game play: Demo Sounds, Coinage & Free Play
static INPUT_PORTS_START( pzlbreak )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x0020, "Level 1" )
	PORT_DIPSETTING(      0x0010, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0070, "Level 4" )
	PORT_DIPSETTING(      0x0060, "Level 5" )
	PORT_DIPSETTING(      0x0050, "Level 6" )
	PORT_DIPSETTING(      0x0040, "Level 7" )
	PORT_DIPSETTING(      0x0030, "Level 8" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( toppyrap )
	PORT_START("DSW1")  /* 500000.w */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500002.w */
	PORT_DIPNAME( 0x03,   0x0003, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, "Time" )          PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "40 Seconds" )
	PORT_DIPSETTING(      0x0008, "50 Seconds" )
	PORT_DIPSETTING(      0x000c, "60 Seconds" )
	PORT_DIPSETTING(      0x0000, "70 Seconds" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPNAME( 0x0020, 0x0020, "God Mode" )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Internal Test" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)  // jump
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)  // fire
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)  // test mode only?
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( finalttr )
	PORT_START("DSW1")  /* 500001 */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
/*  PORT_DIPSETTING(      0x0018, DEF_STR( 1C_1C ) ) Duplicate setting? */
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Must be low or game stops! */
													/* probably VBlank */
	PORT_START("DSW2")  /* 500003 */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Time" )
	PORT_DIPSETTING(      0x0000, "60 Seconds" )
	PORT_DIPSETTING(      0x000c, "90 Seconds" )
	PORT_DIPSETTING(      0x0008, "120 Seconds" )
	PORT_DIPSETTING(      0x0004, "150 Seconds" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// ignore service mode, it shows a regular joystick + 3 buttons config, these game simply uses 6 buttons
static INPUT_PORTS_START( suhosong )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW1:7" )
	PORT_SERVICE_DIPLOC(   0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Button 4 / Stand / Stop / Drop / Full Bet / Take") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Button 1 / High") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Button 2 / Low") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Button 3 / Hit / Go / Double Up") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Button 6 / Bet / Raise") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0000, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Button 5 / Start / Double / Call / Check") PORT_CODE(KEYCODE_B) // Double != Double Up
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END




static INPUT_PORTS_START( yutnori )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x007,  0x0007, "Coin / Credit" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0004, "1 / 50" )
	PORT_DIPSETTING(      0x0003, "1 / 40" )
	PORT_DIPSETTING(      0x0002, "1 / 30" )
	PORT_DIPSETTING(      0x0007, "2 / 50" )
	PORT_DIPSETTING(      0x0006, "2 / 40" )
	PORT_DIPSETTING(      0x0005, "2 / 30" )
	PORT_DIPSETTING(      0x0001, "3 / 50" )
	PORT_DIPSETTING(      0x0000, "3 / 40" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0028, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0038, "4" )
	PORT_DIPSETTING(      0x0018, "5" )
	PORT_DIPSETTING(      0x0010, "6" )
	PORT_DIPSETTING(      0x0008, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, "Mal Count" )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Base Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x01, "30" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x03, "50" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "150" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPSETTING(    0x00, "400" )
	PORT_DIPNAME( 0x30, 0x30, "Score" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "800 1200 1600" )
	PORT_DIPSETTING(    0x30, "1000 1500 2000" )
	PORT_DIPSETTING(    0x00, "1200 1800 2400" )
	PORT_DIPSETTING(    0x20, "1000 2000 3000" )
	PORT_DIPNAME( 0x0040, 0x0040, "Bet -> Score" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* SnowBros */

static GFXDECODE_START( gfx_snowbros_spr )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 16 )
GFXDECODE_END

/* Honey Doll */

static const gfx_layout honeydol_tilelayout8bpp =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3},
	{ STEP8(0,4), STEP8(8*32,4) },
	{ STEP8(0,32), STEP8(16*32,32) },
	32*32
};

static GFXDECODE_START( gfx_honeydol )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 64 ) // how does it use 0-15
	GFXDECODE_ENTRY( "gfx2", 0, honeydol_tilelayout8bpp,            0,  4 )

GFXDECODE_END

static GFXDECODE_START( gfx_twinadv )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 64 )
GFXDECODE_END

/* Winter Bobble */

static const gfx_layout tilelayout_wb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ STEP4(3*4,-4), STEP4(7*4,-4), STEP4(11*4,-4), STEP4(15*4,-4) },
	{ STEP16(0,64) },
	16*64
};

static GFXDECODE_START( gfx_wb )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout_wb, 0, 16 )
GFXDECODE_END

/* SemiCom */

static const gfx_layout hyperpac_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 8*32+4, 8*32+0, 20,16, 8*32+20, 8*32+16,
		12, 8, 8*32+12, 8*32+8, 28, 24, 8*32+28, 8*32+24 },
	{ 0*32, 2*32, 1*32, 3*32, 16*32+0*32, 16*32+2*32, 16*32+1*32, 16*32+3*32,
		4*32, 6*32, 5*32, 7*32, 16*32+4*32, 16*32+6*32, 16*32+5*32, 16*32+7*32 },
	32*32
};


static const gfx_layout sb3_tilebglayout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{8, 9,10, 11, 0, 1, 2, 3  },
	{ 0, 4, 16, 20, 32, 36, 48, 52,
	512+0,512+4,512+16,512+20,512+32,512+36,512+48,512+52},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	1024+0*16,1024+1*64,1024+2*64,1024+3*64,1024+4*64,1024+5*64,1024+6*64,1024+7*64},
	32*64
};


static GFXDECODE_START( gfx_sb3 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sb3_tilebglayout,                   0,  2 )
GFXDECODE_END

static GFXDECODE_START( gfx_hyperpac_spr )
	GFXDECODE_ENTRY( "gfx1", 0, hyperpac_tilelayout, 0, 16 )
GFXDECODE_END

MACHINE_RESET_MEMBER(snowbros_state,semiprot)
{
	uint16_t *PROTDATA = (uint16_t*)memregion("user1")->base();
	int i;

	for (i = 0;i < 0x200/2;i++)
		m_hyperpac_ram[0xf000/2 + i] = PROTDATA[i];
}

MACHINE_RESET_MEMBER(snowbros_state,finalttr)
{
	uint16_t *PROTDATA = (uint16_t*)memregion("user1")->base();
	int i;

	for (i = 0;i < 0x200/2;i++)
		m_hyperpac_ram[0x2000/2 + i] = PROTDATA[i];
}

void snowbros_state::snowbros(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000)/2); /* 8 Mhz - confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::snowbros_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(snowbros_state::snowbros_irq), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	Z80(config, m_soundcpu, XTAL(12'000'000)/2); /* 6 MHz - confirmed */
	m_soundcpu->set_addrmap(AS_PROGRAM, &snowbros_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &snowbros_state::sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.5); /* ~57.5 - confirmed */
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_snowbros));
	m_screen->screen_vblank().set(FUNC(snowbros_state::screen_vblank_snowbros));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 256);

	KANEKO_PANDORA(config, m_pandora, 0, m_palette, gfx_snowbros_spr);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, "soundlatch2");

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(12'000'000)/4)); /* 3 MHz - confirmed */
	ymsnd.irq_handler().set_inputline("soundcpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void snowbros_state::wintbob(machine_config &config)
{
	snowbros(config);

	/* basic machine hardware */
	m_maincpu->set_clock(10000000); /* 10mhz - Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::wintbob_map);

	config.device_remove("pandora");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wb);

	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_wintbob));
	m_screen->screen_vblank().set_nop();
}

void snowbros_state::semicom(machine_config &config)
{
	snowbros(config);

	/* basic machine hardware */
	m_maincpu->set_clock(XTAL(12'000'000)); /* 12MHz - Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::hyperpac_map);

	m_soundcpu->set_clock(XTAL(16'000'000)/4); /* 4MHz - Confirmed */
	m_soundcpu->set_addrmap(AS_PROGRAM, &snowbros_state::hyperpac_sound_map);
	m_soundcpu->set_addrmap(AS_IO, address_map_constructor());

	m_pandora->set_gfxinfo(gfx_hyperpac_spr);

	m_soundlatch->data_pending_callback().set_nop();

	config.device_remove("soundlatch2");

	/* sound hardware */
	ym2151_device &ymsnd(YM2151(config.replace(), "ymsnd", XTAL(16'000'000)/4)); /* 4MHz - Confirmed */
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.10);
	ymsnd.add_route(1, "mono", 0.10);

	OKIM6295(config, m_oki, XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH); /* 1MHz & pin 7 High - Confirmed */
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void snowbros_state::semicom_mcu(machine_config &config)
{
	semicom(config);

	/* basic machine hardware */
	at89c52_device &prot(AT89C52(config, "protection", XTAL(16'000'000)));
	prot.port_out_cb<0>().set(FUNC(snowbros_state::prot_p0_w));
	prot.port_out_cb<1>().set(FUNC(snowbros_state::prot_p1_w));
	prot.port_out_cb<2>().set(FUNC(snowbros_state::prot_p2_w));
}


void snowbros_state::semiprot(machine_config &config)
{
	semicom(config);
	MCFG_MACHINE_RESET_OVERRIDE(snowbros_state, semiprot)
}


void snowbros_state::honeydol(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* MC68000P12 @ 12MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::honeydol_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(snowbros_state::snowbros_irq), "screen", 0, 1);

	Z80(config, m_soundcpu, XTAL(16'000'000)/4); /* 4Mhz (16MHz/4) like SemiCom or 6MHz (12MHz/2) like snowbros??? */
	m_soundcpu->set_addrmap(AS_PROGRAM, &snowbros_state::honeydol_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &snowbros_state::honeydol_sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.5);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_honeydol));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_honeydol);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800/2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(12'000'000)/4)); /* 3Mhz */
	ymsnd.irq_handler().set_inputline("soundcpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH); /* freq? */
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void snowbros_state::twinadv(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* 12MHz like Honey Doll ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::twinadv_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(snowbros_state::snowbros_irq), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	Z80(config, m_soundcpu, XTAL(16'000'000)/4); /* 4Mhz (16MHz/4) like SemiCom or 6MHz (12MHz/2) like snowbros??? */
	m_soundcpu->set_addrmap(AS_PROGRAM, &snowbros_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &snowbros_state::twinadv_sound_io_map);
	m_soundcpu->set_vblank_int("screen", FUNC(snowbros_state::irq0_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.5);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_twinadv));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_twinadv);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, "soundlatch2");

	/* sound hardware */
	OKIM6295(config, m_oki, XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH); /* freq? */
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*

Final Tetris

the pcb is korean and probably original even if it's very cheap.

osc 12mhz
osc 3.579545mhz
68000
z8004b
CA5101 (sound chip)
OKI M6295 (sound chip)
Intel P8752 (mcu)

2x dips banks

*/

void snowbros_state::finalttr(machine_config &config)
{
	semicom(config);

	m_maincpu->set_clock(XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::finalttr_map);

	m_soundcpu->set_clock(XTAL(3'579'545));

	MCFG_MACHINE_RESET_OVERRIDE(snowbros_state, finalttr)

	ym2151_device &ymsnd(YM2151(config.replace(), "ymsnd", XTAL(3'579'545))); /* possible but less likely 4MHz (12MHz/3) */
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(0, "mono", 0.08);
	ymsnd.add_route(1, "mono", 0.08);

	m_oki->set_clock(999900);
	m_oki->reset_routes().add_route(ALL_OUTPUTS, "mono", 0.4);
}


void snowbros_state::_4in1(machine_config &config)
{
	semicom(config);

	/* basic machine hardware */
	m_pandora->set_gfxinfo(gfx_snowbros_spr);
}

void snowbros_state::snowbro3(machine_config &config) /* PCB has 16MHz & 12MHz OSCs */
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* MC68000P10 CPU @ 12mhz or 8MHz (16MHz/2) ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::snowbros3_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(snowbros_state::snowbros3_irq), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_snowbro3));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sb3);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 512);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void snowbros_state::yutnori(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbros_state::yutnori_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(snowbros_state::snowbros_irq), "screen", 0, 1);

//  WATCHDOG_TIMER(config, "watchdog"); // maybe

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57.5);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(snowbros_state::screen_update_snowbros));
	m_screen->screen_vblank().set(FUNC(snowbros_state::screen_vblank_snowbros));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 256);

	KANEKO_PANDORA(config, m_pandora, 0, m_palette, gfx_hyperpac_spr);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki1.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki2(OKIM6295(config, "oki2", XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( snowbros )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sn6.bin",  0x00000, 0x20000, CRC(4899ddcf) SHA1(47d750d3022a80e47ffabe47566bb2556cc8d477) )
	ROM_LOAD16_BYTE( "sn5.bin",  0x00001, 0x20000, CRC(ad310d3f) SHA1(f39295b38d99087dbb9c5b00bf9cb963337a50e2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
	/* where were these from, a bootleg? */
//  ROM_LOAD( "ch0",          0x00000, 0x20000, CRC(36d84dfe) SHA1(5d45a750220930bc409de30f19282bb143fbf94f) )
//  ROM_LOAD( "ch1",          0x20000, 0x20000, CRC(76347256) SHA1(48ec03965905adaba5e50eb3e42a2813f7883bb4) )
//  ROM_LOAD( "ch2",          0x40000, 0x20000, CRC(fdaa634c) SHA1(1271c74df7da7596caf67caae3c51b4c163a49f4) )
//  ROM_LOAD( "ch3",          0x60000, 0x20000, CRC(34024aef) SHA1(003a9b9ee3aaab3d787894d3d4126d372b19d2a8) )
ROM_END

ROM_START( snowbrosa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbros-3a.5",  0x00000, 0x20000, CRC(10cb37e1) SHA1(786be4640f8df2c81a32decc189ea7657ace00c6) )
	ROM_LOAD16_BYTE( "sbros-2a.6",  0x00001, 0x20000, CRC(ab91cc1e) SHA1(8cff61539dc7d35fcbf110d3e54fc1883e7b8509) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbros3-a",     0x00000, 0x20000, CRC(301627d6) SHA1(0d1dc70091c87e9c27916d4232ff31b7381a64e1) )
	ROM_LOAD16_BYTE( "sbros2-a",     0x00001, 0x20000, CRC(f6689f41) SHA1(e4fd27b930a31479c0d99e0ddd23d5db34044666) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3-a.ic5",  0x00000, 0x20000, CRC(e1bc346b) SHA1(a20c343d9ed2ad4f785d21076499008edad251f9) )
	ROM_LOAD16_BYTE( "2-a.ic6",  0x00001, 0x20000, CRC(1be27f9d) SHA1(76dd14480b9274831e51016f7bb57459d7b15cf9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosd ) /* Korean release, but no specific "For use in Korea only..." notice screen */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sbk_3-a.bin",   0x00000, 0x20000, CRC(97174d40) SHA1(481e8c680af8b03d4bcf97b87ca0ba5a3ffca0d7) )
	ROM_LOAD16_BYTE( "sbk_2-a.bin",   0x00001, 0x20000, CRC(80cc80e5) SHA1(1eeca0924c93e9f0536683160e80c59871569088) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( snowbrosj )/* "For use in Japan only..." notice screen */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "snowbros.3",   0x00000, 0x20000, CRC(3f504f9e) SHA1(700758b114c3fde6ea8f84222af0850dba13cd3b) )
	ROM_LOAD16_BYTE( "snowbros.2",   0x00001, 0x20000, CRC(854b02bc) SHA1(4ad1548eef94dcb95119cb4a7dcdefa037591b5b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "sbros-4.29",   0x0000, 0x8000, CRC(e6eab4e4) SHA1(d08187d03b21192e188784cb840a37a7bdb5ad32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* The gfx ROM (snowbros.1) was bad, I'm using the ones from the other sets. */
	ROM_LOAD( "sbros-1.41",   0x00000, 0x80000, CRC(16f06b3a) SHA1(c64d3b2d32f0f0fcf1d8c5f02f8589d59ddfd428) )
ROM_END

ROM_START( wintbob )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wb3", 0x00000, 0x10000, CRC(b9719767) SHA1(431c97d409f2a5ff7f46116a4d8907e446434431) )
	ROM_LOAD16_BYTE( "wb1", 0x00001, 0x10000, CRC(a4488998) SHA1(4e927e31c1b865dbdba2b985c7a819a07e2e81b8) )

	/* The wb03.bin below is bad, the set has a different copyright message (IN KOREA is replaced with 1990)
	   but also clearly suffers from bitrot at the following addresses
	    4FC2, 5F02, 6642, D6C2, D742
	   in all cases bit 0x20 is incorrectly set in the bad ROM
	   ROM_LOAD16_BYTE( "wb03.bin", 0x00000, 0x10000, CRC(df56e168) SHA1(20dbabdd97e6f3d4bf6500bf9e8476942cb48ae3) )
	   ROM_LOAD16_BYTE( "wb01.bin", 0x00001, 0x10000, CRC(05722f17) SHA1(9356e2488ea35e0a2978689f2ca6dfa0d57fd2ed) )
	   A dump from a different PCB confirmed the above. Correct ROM for this copyright message hack is
	   ROM_LOAD16_BYTE( "wb03.bin", 0x00000, 0x10000, CRC(e2ae422b) SHA1(526abf88747a00527047ec578a20be36f25af257) )
	*/

	ROM_LOAD16_BYTE( "wb04.bin", 0x20000, 0x10000, CRC(53be758d) SHA1(56cf85ba23fe699031d73e8f367a1b8ac837d5f8) )
	ROM_LOAD16_BYTE( "wb02.bin", 0x20001, 0x10000, CRC(fc8e292e) SHA1(857cfeb0be121e64e6117120514ae1f2ffeae4d6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "wb05.bin",     0x0000, 0x10000, CRC(53fe59df) SHA1(a99053e82b9fed76f744fa9f67078294641c6317) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "wb13.bin",     0x00000, 0x10000, CRC(426921de) SHA1(5107c58e7e08d71895baa67fe260b17ebd61389c) )
	ROM_LOAD16_BYTE( "wb06.bin",     0x00001, 0x10000, CRC(68204937) SHA1(fd2ef93df5fd8aa2d36072858dbcfce41157ef3e) )
	ROM_LOAD16_BYTE( "wb12.bin",     0x20000, 0x10000, CRC(ef4e04c7) SHA1(17158b61b3c158e0491db9abb2e1a8c20d981d37) )
	ROM_LOAD16_BYTE( "wb07.bin",     0x20001, 0x10000, CRC(53f40978) SHA1(058bbf3b7877f0cd320383e0386c5959e0d6589b) )
	ROM_LOAD16_BYTE( "wb11.bin",     0x40000, 0x10000, CRC(41cb4563) SHA1(94f1d12d299ac08fc8522139e1927f0cf739be75) )
	ROM_LOAD16_BYTE( "wb08.bin",     0x40001, 0x10000, CRC(9497b88c) SHA1(367c6106276f3816528341f11f3a97ae458d25cd) )
	ROM_LOAD16_BYTE( "wb10.bin",     0x60000, 0x10000, CRC(5fa22b1e) SHA1(1164003d873e9738a3ca133cce689c7120061e3c) )
	ROM_LOAD16_BYTE( "wb09.bin",     0x60001, 0x10000, CRC(9be718ca) SHA1(5c195e4f13efbdb229201d2408d018861bf389cc) )
ROM_END


ROM_START( snowbroswb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "toa3.bin", 0x00000, 0x10000, CRC(55d302da) SHA1(235f1cd5adeb783f42bd7261e6f575826235c5b2) )
	ROM_LOAD16_BYTE( "toa1.bin", 0x00001, 0x10000, CRC(3c64e145) SHA1(46c3ba17aac516fc203d2d82ed5452b8212f5edb) )
	ROM_LOAD16_BYTE( "wb04.bin", 0x20000, 0x10000, CRC(53be758d) SHA1(56cf85ba23fe699031d73e8f367a1b8ac837d5f8) )
	ROM_LOAD16_BYTE( "wb02.bin", 0x20001, 0x10000, CRC(fc8e292e) SHA1(857cfeb0be121e64e6117120514ae1f2ffeae4d6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "wb05.bin",     0x0000, 0x10000, CRC(53fe59df) SHA1(a99053e82b9fed76f744fa9f67078294641c6317) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "wb13.bin",     0x00000, 0x10000, CRC(426921de) SHA1(5107c58e7e08d71895baa67fe260b17ebd61389c) )
	ROM_LOAD16_BYTE( "wb06.bin",     0x00001, 0x10000, CRC(68204937) SHA1(fd2ef93df5fd8aa2d36072858dbcfce41157ef3e) )
	ROM_LOAD16_BYTE( "wb12.bin",     0x20000, 0x10000, CRC(ef4e04c7) SHA1(17158b61b3c158e0491db9abb2e1a8c20d981d37) )
	ROM_LOAD16_BYTE( "wb07.bin",     0x20001, 0x10000, CRC(53f40978) SHA1(058bbf3b7877f0cd320383e0386c5959e0d6589b) )
	ROM_LOAD16_BYTE( "wb11.bin",     0x40000, 0x10000, CRC(41cb4563) SHA1(94f1d12d299ac08fc8522139e1927f0cf739be75) )
	ROM_LOAD16_BYTE( "wb08.bin",     0x40001, 0x10000, CRC(9497b88c) SHA1(367c6106276f3816528341f11f3a97ae458d25cd) )
	ROM_LOAD16_BYTE( "wb10.bin",     0x60000, 0x10000, CRC(5fa22b1e) SHA1(1164003d873e9738a3ca133cce689c7120061e3c) )
	ROM_LOAD16_BYTE( "wb09.bin",     0x60001, 0x10000, CRC(9be718ca) SHA1(5c195e4f13efbdb229201d2408d018861bf389cc) )
ROM_END

ROM_START( toto )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u60.5j",  0x00000, 0x20000, CRC(39203792) SHA1(4c8d560be02a514cbf91774c7a0b4a95cf573356) )
	ROM_LOAD16_BYTE( "u51.4j",  0x00001, 0x20000, CRC(7b846cd4) SHA1(04aa0bbaab4303fb08dff52d5515f7e764f1be6d))

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "u46.4c",   0x0000, 0x8000, CRC(77b1ef42) SHA1(75e3c8c2b687669cc56f972dd7375dab5185859c) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "u107.8k",          0x00000, 0x20000, CRC(4486153b) SHA1(a6dc0c17bf2328ab725bce4aaa0a413a42129fb0) )
	ROM_LOAD( "u108.8l",          0x20000, 0x20000, CRC(3286cf5f) SHA1(133366b0e10ab86111247cbedf329e8e3a7f2148) )
	ROM_LOAD( "u109.8m",          0x40000, 0x20000, CRC(464d7251) SHA1(f03ee54e9301ea87de4171cecdbad4a5e17929c4) )
	ROM_LOAD( "u110.8n",          0x60000, 0x20000, CRC(7dea56df) SHA1(7e7b9238837c6f4221cff416a2de21723d2c9272) )
ROM_END

/* Barko */

/*

Honey Doll - Barko Corp 1995

Rom Board include a Cypress cy7C382-0JC chip

Main Board :

CPU : 1 X MC68000P12
      1 X Z80B

1 X Oki M6295
2 X Cypress CY7C384A-XJC

2 x quartz - 12Mhz and 16Mhz

*/

ROM_START( honeydol )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d-16.uh12",  0x00001, 0x20000, CRC(cee1a2e3) SHA1(6d1ff5358ec704616b724eea2ab9b60b84709eb1) )
	ROM_LOAD16_BYTE( "d-17.ui12",  0x00000, 0x20000, CRC(cac44154) SHA1(2c30dc033001fc9303da7e117e3401bc7af16607) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "d-12.uh15",   0x0000, 0x8000, CRC(386f1b63) SHA1(d719875226cd3d380e2ebec49209590d91b6f07b) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // 4 bpp gfx
	ROM_LOAD( "d-13.1",          0x000000, 0x80000, CRC(ff6a57fb) SHA1(2fbf61f4ac2655a60b1fa82bb6d001f0ef8b4654) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "d-14.5",          0x000000, 0x80000, CRC(2178996f) SHA1(04368384cb191b28b23199c8175e93271ab79103) )
	ROM_LOAD( "d-15.6",          0x080000, 0x80000, CRC(6629239e) SHA1(5f462c04eb11c2b662236fd65bbf74fa08038eec) )
	ROM_LOAD( "d-18.9",          0x100000, 0x80000, CRC(0210507a) SHA1(5b7348bf253b1ae8bfa86cdee2ff80aa2b3faa79) )
	ROM_LOAD( "d-19.10",         0x180000, 0x80000, CRC(d27375d5) SHA1(2a39ce9b985e00a290c3ea75be3b1edbc00d39ec) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "d-11.u14", 0x00000, 0x40000, CRC(f3ee4861) SHA1(f24f1f855ae6c96a6d84a4b5e5c196df8f922d0a) )
ROM_END

/*

+--------------------------------+
|      Z80A                      |
|GL324  6116                  ua4|
|M6295 uh15  HY6264           ua5|
|J     sra   HY6264           ua6|
|A     srb   84256               |
|M           84256               |
|M           84256    CY7C384A   |
|A           84256               |
| SWA1       uh12                |
|      68000 84256               |
| SWA2       ui12  12MHz 16MHz   |
+--------------------------------+

Produttore: Barko
N.revisione: S16K951102
CPU
1x 68000 (main)
1x Z8400B (sound)
1x OKI M6295 (sound)
1x GL324 (sound)
1x CY7C384A
1x oscillator 12.000MHz
1x oscillator 16.000MHz

ROMs

1x 27256 (uh15)
2x M27C2001 (sra,srb)
1x AM27C010 (12)
1x D27C010 (13)
1x M27C4001 (14)
1x AT27C040 (15)
1x TMS27C040 (16)

6x HY18CV8S (read protected)

*/

ROM_START( twinadv )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "13.uh12",  0x00001, 0x20000, CRC(9f70a39b) SHA1(d49823be58b00c4c5a4f6cc4e4371531492aff1e) )
	ROM_LOAD16_BYTE( "12.ui12",  0x00000, 0x20000, CRC(d8776495) SHA1(15b93ded80bf9f240faef2d89b6076f33f1f4ece) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "uh15.bin", 0x0000, 0x8000, CRC(3d5acd08) SHA1(c19f686862dfc12d2fa91c2dd3d3b75d9cb410c3) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* 4bpp gfx */
	ROM_LOAD( "16.ua4", 0x000000, 0x80000, CRC(f491e171) SHA1(f31b945b0c4b30d1b3dc6c5928b77aad4e956bc7) )
	ROM_LOAD( "15.ua5", 0x080000, 0x80000, CRC(79a08b8d) SHA1(034c0a3b9e27ac174092d265b32fb82d6ee45d47) )
	ROM_LOAD( "14.ua6", 0x100000, 0x80000, CRC(79faee0b) SHA1(7421a5fa038d01658ba5ac1f65ea87b97ac25c36) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples - both banks are almost the same */
	/* todo, check bank ordering .. */
	ROM_LOAD( "sra.bin", 0x00000, 0x40000, CRC(82f452c4) SHA1(95ad6ede87ceafb045ed7df40496baf96190b97f) ) // bank 1
	ROM_LOAD( "srb.bin", 0x40000, 0x40000, CRC(109e51e6) SHA1(3344c68d63bbad4a02b47143b2d2f72ce9bcb4bb) ) // bank 2
ROM_END

ROM_START( twinadvk )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uh12",  0x00001, 0x20000, CRC(e0bcc738) SHA1(7fc6a793fcdd80122c0ac6409ae4cac5597b7b5a) )
	ROM_LOAD16_BYTE( "ui12",  0x00000, 0x20000, CRC(a3ee6451) SHA1(9c0b415a2f325513739f2047780c2a56df350aa5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "uh15.bin", 0x0000, 0x8000, CRC(3d5acd08) SHA1(c19f686862dfc12d2fa91c2dd3d3b75d9cb410c3) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* 4bpp gfx */
	ROM_LOAD( "ua4", 0x000000, 0x80000, CRC(a5aff49b) SHA1(ee162281ba643729ee007f9634c21fadd3c1cb48) )
	ROM_LOAD( "ua5", 0x080000, 0x80000, CRC(f83b3b97) SHA1(2e967d49ef411d164a0b6cf32444f60fcd8068a9) )
	ROM_LOAD( "ua6", 0x100000, 0x80000, CRC(4dfcffb9) SHA1(c157e031acbb321b9435389f9fc4e1ffebca106d) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples - both banks are almost the same */
	/* todo, check bank ordering .. */
	ROM_LOAD( "sra.bin", 0x00000, 0x40000, CRC(82f452c4) SHA1(95ad6ede87ceafb045ed7df40496baf96190b97f) ) // bank 1
	ROM_LOAD( "srb.bin", 0x40000, 0x40000, CRC(109e51e6) SHA1(3344c68d63bbad4a02b47143b2d2f72ce9bcb4bb) ) // bank 2
ROM_END

ROM_START( multi96 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uh12",  0x00001, 0x20000, CRC(e912ea4e) SHA1(cf0b37d6b6fbdd311ef7b404c4ba2c6a7e1f8486) ) // sldh
	ROM_LOAD16_BYTE( "ui12",  0x00000, 0x20000, CRC(ac99e837) SHA1(20bc1599f78a4eac65cae54350872fa292daa807) ) // sldh

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* 64k for z80 sound code */
	ROM_LOAD( "uh15.bin", 0x0000, 0x8000, CRC(3d5acd08) SHA1(c19f686862dfc12d2fa91c2dd3d3b75d9cb410c3) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* 4bpp gfx */
	ROM_LOAD( "ua4", 0x000000, 0x80000, CRC(66cae586) SHA1(22af524b26241a6456b777a847db73ff8d3db11f) ) // sldh
	ROM_LOAD( "ua5", 0x080000, 0x80000, CRC(0bd9f6bb) SHA1(400ddff7a76860caacfe8bfd803f9ccd2dba3356) ) // sldh
	ROM_LOAD( "ua6", 0x100000, 0x80000, CRC(0e90b26c) SHA1(fd9b40988d03db8ed797abf859a8828bb65db8d5) ) // sldh

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples - both banks are almost the same */
	/* todo, check bank ordering .. */
	ROM_LOAD( "sra.bin", 0x00000, 0x40000, CRC(82f452c4) SHA1(95ad6ede87ceafb045ed7df40496baf96190b97f) ) // bank 1
	ROM_LOAD( "srb.bin", 0x40000, 0x40000, CRC(109e51e6) SHA1(3344c68d63bbad4a02b47143b2d2f72ce9bcb4bb) ) // bank 2
ROM_END

/* SemiCom Games */

ROM_START( hyperpac )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hyperpac.h12", 0x00001, 0x20000, CRC(2cf0531a) SHA1(c4321d728845035507352d0bcf4348d28b92e85e) )
	ROM_LOAD16_BYTE( "hyperpac.i12", 0x00000, 0x20000, CRC(9c7d85b8) SHA1(432d5fbe8bef875ce4a9aeb74a7b57dc79c709fd) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "hyperpac.u1", 0x00000, 0x10000 , CRC(03faf88e) SHA1(a8da883d4b765b809452bbffca37ff224edbe86d) )

	ROM_REGION( 0x2000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "at89c52.bin", 0x0000, 0x2000 , CRC(291f9326) SHA1(e440ce7d92188faa86e02e7f9db4ec6bce21efd3) ) /* decapped */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hyperpac.j15", 0x00000, 0x40000, CRC(fb9f468d) SHA1(52857b1a04c64ac853340ebb8e92d98eabea8bc1) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "hyperpac.a4", 0x000000, 0x40000, CRC(bd8673da) SHA1(8466355894da4d2c9a54d03a833cc9b4ec0c67eb) )
	ROM_LOAD( "hyperpac.a5", 0x040000, 0x40000, CRC(5d90cd82) SHA1(56be68478a81bb4e1011990da83334929a0ac886) )
	ROM_LOAD( "hyperpac.a6", 0x080000, 0x40000, CRC(61d86e63) SHA1(974c634607993924fa098eff106b1b288bec1e26) )
ROM_END

ROM_START( hyperpacb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hpacuh12.bin", 0x00001, 0x20000, CRC(633ab2c6) SHA1(534435fa602adebf651e1d42f7c96b01eb6634ef) )
	ROM_LOAD16_BYTE( "hpacui12.bin", 0x00000, 0x20000, CRC(23dc00d1) SHA1(8d4d00f450b94912adcbb24073f9b3b01eab0450) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "hyperpac.u1", 0x00000, 0x10000 , CRC(03faf88e) SHA1(a8da883d4b765b809452bbffca37ff224edbe86d) ) // was missing from this set, using the one from the original

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hyperpac.j15", 0x00000, 0x40000, CRC(fb9f468d) SHA1(52857b1a04c64ac853340ebb8e92d98eabea8bc1) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "hyperpac.a4", 0x000000, 0x40000, CRC(bd8673da) SHA1(8466355894da4d2c9a54d03a833cc9b4ec0c67eb) )
	ROM_LOAD( "hyperpac.a5", 0x040000, 0x40000, CRC(5d90cd82) SHA1(56be68478a81bb4e1011990da83334929a0ac886) )
	ROM_LOAD( "hyperpac.a6", 0x080000, 0x40000, CRC(61d86e63) SHA1(974c634607993924fa098eff106b1b288bec1e26) )
ROM_END

ROM_START( twinkle )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "uh12.bin", 0x00001, 0x20000, CRC(a99626fe) SHA1(489098a2ceb36df97b6b1d59b7b696300deee3ab) )
	ROM_LOAD16_BYTE( "ui12.bin", 0x00000, 0x20000, CRC(5af73684) SHA1(9be43e5c71152d515366e422eb077a41dbb3fe62) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1.bin", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(00d3e4b4) SHA1(afa359a8b48605ff034133bad2a0a182429dec71) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15.bin", 0x00000, 0x40000, CRC(0a534b37) SHA1(b7d780eb4668f1f757a60884c022f5bbc424dc97) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4.bin", 0x000000, 0x80000, CRC(6b64bb09) SHA1(547eac1ad931a6b937dff0b922d06af92cc7ab73) )
ROM_END

ROM_START( twinklea )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uh12.bin", 0x00001, 0x20000, CRC(6cc6e53c) SHA1(26584d2b99878230a33c1c2c8d71e758b0c09cac) ) // sldh
	ROM_LOAD16_BYTE( "ui12.bin", 0x00000, 0x20000, CRC(79420382) SHA1(13183d819f5b7699e8e0974c193796151a0a9617) ) // sldh

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "u1.bin", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) // Intel 87C52 MCU Code
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) // needs decapping

	ROM_REGION16_BE( 0x200, "user1", 0 ) // Data from Shared RAM
	// this is not a real rom but instead the data extracted from shared ram, the MCU puts it there
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, BAD_DUMP CRC(00d3e4b4) SHA1(afa359a8b48605ff034133bad2a0a182429dec71) ) // this has been extracted for the parent set, seems to work for this set, too

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "uj15.bin", 0x00000, 0x40000, CRC(0a534b37) SHA1(b7d780eb4668f1f757a60884c022f5bbc424dc97) )

	ROM_REGION( 0x080000, "gfx1", 0 ) // Sprites
	ROM_LOAD( "ua4.bin", 0x000000, 0x80000, CRC(6b64bb09) SHA1(547eac1ad931a6b937dff0b922d06af92cc7ab73) )
ROM_END

ROM_START( pzlbreak )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "4.uh12", 0x00001, 0x20000, CRC(b3f04f80) SHA1(79b5414727004719ff172e084a672b21e955f0bc) )
	ROM_LOAD16_BYTE( "5.ui12", 0x00000, 0x20000, CRC(13c298a0) SHA1(9455de7ea45c9a61ed6105023eb909c086c44007) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "0.u1", 0x00000, 0x10000 , CRC(1ad646b7) SHA1(0132baa097e48df2450afdcd316375dc546ea4d0) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASEFF ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(092cb794) SHA1(eb2b336d97b440453ca37ee7605654b35dfb6bad) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1.uj15", 0x00000, 0x40000, CRC(dbfae77c) SHA1(cc509d52cd9c608fc80df799890e62e7b4c143c6) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "2.ua4", 0x000000, 0x80000, CRC(d211705a) SHA1(b3a7f8198dc8c034b17b843b2ab0298426de3f55) )
	ROM_LOAD( "3.ua5", 0x080000, 0x80000, CRC(6cdb73e9) SHA1(649e91ee54de2b359a207bed4d950db95515a3d8) )
ROM_END

ROM_START( pzlbreaka )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uh12", 0x00001, 0x20000, CRC(c8a82ca8) SHA1(ee19c253af9c7c8a33435d9d2494c50f16033572) )
	ROM_LOAD16_BYTE( "ui12", 0x00000, 0x20000, CRC(2f66c4ce) SHA1(4349f093ce1267c2ebcbf1233082661604f10851) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "u1", 0x00000, 0x10000 , CRC(1ad646b7) SHA1(0132baa097e48df2450afdcd316375dc546ea4d0) )

	ROM_REGION( 0x10000, "cpu2", 0 ) // Intel 87C52 MCU Code
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASEFF ) // Data from Shared RAM
	// this is not a real rom but instead the data extracted from shared ram, the MCU puts it there
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, BAD_DUMP CRC(092cb794) SHA1(eb2b336d97b440453ca37ee7605654b35dfb6bad) ) // extracted from the parent set, so marked as bad

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "uj15", 0x00000, 0x20000, CRC(5cdffcc5) SHA1(793a20bd0480cfea0dbf9397797428f6d105f724) ) // 1xxxxxxxxxxxxxxxx = 0xFF and half sized compared to the parent

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ua4", 0x000000, 0x80000, CRC(d211705a) SHA1(b3a7f8198dc8c034b17b843b2ab0298426de3f55) )
	ROM_LOAD( "ua5", 0x080000, 0x80000, CRC(eb3044bc) SHA1(c00e7b112b82c8eeb59c2a96168ea5156347f05e) ) // some differences compared to the parent
ROM_END


ROM_START( toppyrap )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "uh12.bin", 0x00001, 0x40000, CRC(6f5ad699) SHA1(42f7201d6274ff8338a7d4627af99001f473e841) )
	ROM_LOAD16_BYTE( "ui12.bin", 0x00000, 0x40000, CRC(caf5a7e1) SHA1(b521b2f06a804a52dad1b07657db2a29e1411844) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1.bin", 0x00000, 0x10000 , CRC(07f50947) SHA1(83740655ab5f677bd009191bb0de60e237aaa11c) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this contains the code for 2 of the IRQ functions, but the game only uses one of them, the other is
	   executed from ROM.  The version in ROM is slightly patched version so maybe there is an earlier revision
	   which uses the code provided by the MCU instead */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(0704e6c7) SHA1(22387257db569990378c304af9677e6dc1436207) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15.bin", 0x00000, 0x20000, CRC(a3bacfd7) SHA1(d015d8bd26d0189fc13d09fefcb9b8baaaacec8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4.bin", 0x000000, 0x80000, CRC(a9577bcf) SHA1(9918d982ebee1c88bd203fa2b3ce2468c160fb95) )
	ROM_LOAD( "ua5.bin", 0x080000, 0x80000, CRC(7179d32d) SHA1(dae7126401b5bb7f99689587e05a8bf5033ec06e) )
	ROM_LOAD( "ua6.bin", 0x100000, 0x80000, CRC(4834e5b1) SHA1(cd8a4c329b2bfe1a9c2dea9d72ca09b71366c60a) )
	ROM_LOAD( "ua7.bin", 0x180000, 0x80000, CRC(663dd099) SHA1(84b52af54ac49e8b4bae23995e3cf94494be2bb3) )
ROM_END

ROM_START( moremore )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52.bin",  0x00001, 0x40000, CRC(cea4b246) SHA1(5febcb5dda6581caccfe9079b28c2366dfc1db2b) )
	ROM_LOAD16_BYTE( "u74.bin",  0x00000, 0x40000, CRC(2acdcb88) SHA1(74d661d07752bbccab7eab151209a414e9bf7675) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35.bin", 0x00000, 0x10000 , CRC(92dc95fc) SHA1(f04e63cc680835458246989532faf5657e28db13) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(782dd2aa) SHA1(2587734271e0c85cb76bcdee171366c4e6fc9f81) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14.bin", 0x00000, 0x40000, CRC(90580088) SHA1(c64de2c0db95ab4ce06fc0a29c0cc8b7f3deeb28) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75.bin", 0x000000, 0x80000, CRC(d671815c) SHA1(a7e8d3bf688ce51b5d9a2b306cc557974328c322) )
	ROM_LOAD( "u76.bin", 0x080000, 0x80000, CRC(e0d479e8) SHA1(454b53949664aca07a86229d3b6c9ce4e9449ea6) )
	ROM_LOAD( "u77.bin", 0x100000, 0x80000, CRC(60a281da) SHA1(3f268f8b1cd8efd3e32d0fcdba5483c93122800e) )
	ROM_LOAD( "u78.bin", 0x180000, 0x80000, CRC(e2723b4e) SHA1(6b4ba1e2e937b3231d76526af3f5a4a67144e4d5) )
ROM_END

ROM_START( moremorp )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mmp_u52.bin",  0x00001, 0x40000, CRC(66baf9b2) SHA1(f1d383a94ef4313cb02c59ace17b9562eddcfb3c) )
	ROM_LOAD16_BYTE( "mmp_u74.bin",  0x00000, 0x40000, CRC(7c6fede5) SHA1(41bc539a6efe9eb2304243701857b972d2170bcf) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "mmp_u35.bin", 0x00000, 0x10000 , CRC(4d098cad) SHA1(a79d417e7525a25dd6697da9f3d1de269e759d2e) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(782dd2aa) SHA1(2587734271e0c85cb76bcdee171366c4e6fc9f81) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "mmp_u14.bin", 0x00000, 0x40000, CRC(211a2566) SHA1(48138547822a8e76c101dd4189d581f80eee1e24) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "mmp_u75.bin", 0x000000, 0x80000, CRC(af9e824e) SHA1(2b68813bf025a34b8958033108e4f8d39fd618cb) )
	ROM_LOAD( "mmp_u76.bin", 0x080000, 0x80000, CRC(c42af064) SHA1(f9d755e7cb52828d8594f7871932daf11443689f) )
	ROM_LOAD( "mmp_u77.bin", 0x100000, 0x80000, CRC(1d7396e1) SHA1(bde7e925051408dd2371b5da8235a6a4cae8cf6a) )
	ROM_LOAD( "mmp_u78.bin", 0x180000, 0x80000, CRC(5508d80b) SHA1(1b9a70a502b237fa11d1d55dce761e2def18873a) )
ROM_END

ROM_START( 3in1semi ) /* SemiCom Ser-4331-4 PCB */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52",  0x00001, 0x40000, CRC(b0e4a0f7) SHA1(e1f8b8ef020a85fcd7817814cf6c5d560e9e608d) )
	ROM_LOAD16_BYTE( "u74",  0x00000, 0x40000, CRC(266862c4) SHA1(2c5c513fee99bdb6e0ae3e0e644e516bdaddd629) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(85deba7c) SHA1(44c6d9306b4f8e47182f4740a18971c49a8df8db) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14", 0x00000, 0x40000, CRC(c83c11be) SHA1(c05d96d61e5b8245232c85cbbcb7cc1e4e066492) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75", 0x000000, 0x80000, CRC(b66a0db6) SHA1(a4e604eb3c0a5b16b4b0bb99219045bf2146287c) )
	ROM_LOAD( "u76", 0x080000, 0x80000, CRC(5f4b48ea) SHA1(e9dd1100d55b021b060990988c1e5271ce1ae35b) )
	ROM_LOAD( "u77", 0x100000, 0x80000, CRC(d44211e3) SHA1(53af19dec03e76912632450414cdbcbb31cc094c) )
	ROM_LOAD( "u78", 0x180000, 0x80000, CRC(af596afc) SHA1(875d7a51ff5c741cae4483d8da33df9cae8de52a) )
ROM_END

ROM_START( 3in1semia ) // SemiCom Ser-4331-4 PCB
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u52.bin",  0x00001, 0x40000, CRC(388334a8) SHA1(93823917682f1658a443d23ceb58eafa530d1854) )
	ROM_LOAD16_BYTE( "u74.bin",  0x00000, 0x40000, CRC(555ae716) SHA1(9a3d7d81d7c6fb8443d263087bb7dca7bc3918b6) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Z80 Code
	ROM_LOAD( "u35.bin", 0x00000, 0x10000 , CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x10000, "cpu2", 0 ) // Intel 87C52 MCU Code
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) // Data from Shared RAM
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(85deba7c) SHA1(44c6d9306b4f8e47182f4740a18971c49a8df8db) )

	ROM_REGION( 0x040000, "oki", 0 ) // Samples
	ROM_LOAD( "u14.bin", 0x00000, 0x40000, CRC(c83c11be) SHA1(c05d96d61e5b8245232c85cbbcb7cc1e4e066492) )

	ROM_REGION( 0x200000, "gfx1", 0 ) // Sprites
	ROM_LOAD( "u75.bin", 0x000000, 0x80000, CRC(9d705249) SHA1(99e0adca1f3f285692bf15c988b25d87f6b1dd90) )
	ROM_LOAD( "u76.bin", 0x080000, 0x80000, CRC(b65f5d79) SHA1(a9f19cf54f5ecdeca58f853f3053a686b89fea63) )
	ROM_LOAD( "u77.bin", 0x100000, 0x80000, CRC(b9728be9) SHA1(4ea7940f9d1d7c01cfcab92184b6513e4b0a83d7) )
	ROM_LOAD( "u78.bin", 0x180000, 0x20000, CRC(aefad49e) SHA1(246ea713eaaa6a01290eded36377437cb7e79fa8) ) // MX26C1000APC
ROM_END

/*

Ma Cheon Ru
SemiCom (c) 1999

PCB:  Ser-4331-4

  CPU: 68000, Z80
Sound: OKIM6295, YM2151+YM3012 (rebadged as KA51+BS902)
Video: QuickLogic QL2003-XPL84C
  OSC: 16MHz, 12MHz

Measurements:
   68000 12MHz
     Z80 4MHz (16MHz/4)
  YM2151 4MHz (16MHz/4)
OKI 6295 1MHz (16MHz/16, pin 7 High)
*/

ROM_START( mcheonru ) /* SemiCom Ser-4331-4 PCB */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52",  0x00001, 0x40000, CRC(63fd8a9b) SHA1(53054d8072322842c32625ab38e7d62dc0e75627) )
	ROM_LOAD16_BYTE( "u74",  0x00000, 0x40000, CRC(3edb17ce) SHA1(0c6ea239f57eca114d75c173b77b2c8ef43d63a2) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35", 0x00000, 0x10000 , CRC(79e965b4) SHA1(268df67ec6ea828ae01a6e4d2da9ad2a08a837f1) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD( "protdata.bin", 0x00000, 0x200 , CRC(d61f4f07) SHA1(29485bce6e3a7ed2ace540bb81fe028456ae1ae9) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14", 0x00000, 0x40000, CRC(28a62d23) SHA1(24dbe6229647032599e02225de57650205dce5c3) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75", 0x000000, 0x80000, CRC(0142c085) SHA1(9d948a63e4edcdb7f5cef0d2d7f118e6cbf4779e) )
	ROM_LOAD( "u76", 0x080000, 0x80000, CRC(27fda507) SHA1(87820348470979da730956066d3c820faedbec13) )
	ROM_LOAD( "u77", 0x100000, 0x80000, CRC(4dfe0550) SHA1(9a227114765ea5327b64061e4b4e4d4f19bd3293) )
	ROM_LOAD( "u78", 0x180000, 0x80000, CRC(f1b74978) SHA1(d1ac1bc212050d4f1a861045ab612115c73d3fd0) )
ROM_END

/*

Cookie & Bibi 2
SemiCom (c) 1996

  CPU: MC68000P10, Z8400B Z80
Sound: OKIM6295, YM2151+YM3012 (rebadged as AD-65, KA51+K-662)
Video: Lattice pLSI 1032-60LJ (square 84 pin socketed)
  OSC: 16MHz, 12MHz
*/

ROM_START( cookbib2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "unico_02.uh12",  0x00001, 0x40000, CRC(b2909460) SHA1(2438638af870cfc105631d2b5e5a27a64ab5394d) ) // 27C020
	ROM_LOAD16_BYTE( "unico_01.ui12",  0x00000, 0x40000, CRC(65aafde2) SHA1(01f9f261527c35182f0445d641d987aa86ad750f) ) // 27C020

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "unico_07.u1", 0x00000, 0x10000 , CRC(f59f1c9a) SHA1(2830261fd55249e015514fcb4cf8392e83b7fd0d) ) // 27C512

	ROM_REGION( 0x2000, "protection", 0 ) /* P87C52EBPN (XSC6407A) Code (8052) */
	ROM_LOAD( "p87c52ebpn.bin", 0x0000, 0x2000 , CRC(ef042cef) SHA1(3089d5a3cb5ed663a8d89d59e427a06cffcd6219) ) /* dumped via laser glitching */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "unico_06.uj15", 0x00000, 0x20000, CRC(5e6f76b8) SHA1(725800143dfeaa6093ed5fcc5b9f15678ae9e547) ) // 27C010

	ROM_REGION( 0x140000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "unico_05.ua4", 0x000000, 0x80000, CRC(89fb38ce) SHA1(1b39dd9c2743916b8d8af590bd92fe4819c2454b) ) // 27C040
	ROM_LOAD( "unico_04.ua5", 0x080000, 0x80000, CRC(f240111f) SHA1(b2c3b6e3d916fc68e1fd258b1279b6c39e1f0108) ) // 27C040
	ROM_LOAD( "unico_03.ua6", 0x100000, 0x40000, CRC(e1604821) SHA1(bede6bdd8331128b9f2b229d718133470bf407c9) ) // 27C020
	/* UA7 is unpopulated */
ROM_END

ROM_START( cookbib2a )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "unico.uh12",  0x00001, 0x40000, CRC(19421631) SHA1(00dfc474cee7d21d4b6bdbae2eabcdfde3584307) ) // non descriptive UNICO label with no numbers
	ROM_LOAD16_BYTE( "unico.ui12",  0x00000, 0x40000, CRC(0d09ecf5) SHA1(16f787638041d38ee2567ca958ca4405324cf5fa) ) // both program ROMs are 27C020

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "unico_07.u1", 0x00000, 0x10000 , CRC(f59f1c9a) SHA1(2830261fd55249e015514fcb4cf8392e83b7fd0d) ) // 27C512

	ROM_REGION( 0x2000, "protection", 0 ) /* P87C52EBPN (XSC6407A) Code (8052) */
	ROM_LOAD( "p87c52ebpn.bin", 0x0000, 0x2000 , CRC(ef042cef) SHA1(3089d5a3cb5ed663a8d89d59e427a06cffcd6219) ) /* dumped via laser glitching */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "unico_06.uj15", 0x00000, 0x20000, CRC(5e6f76b8) SHA1(725800143dfeaa6093ed5fcc5b9f15678ae9e547) ) // 27C010

	ROM_REGION( 0x140000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "unico_05.ua4", 0x000000, 0x80000, CRC(89fb38ce) SHA1(1b39dd9c2743916b8d8af590bd92fe4819c2454b) ) // 27C040
	ROM_LOAD( "unico_04.ua5", 0x080000, 0x80000, CRC(f240111f) SHA1(b2c3b6e3d916fc68e1fd258b1279b6c39e1f0108) ) // 27C040
	ROM_LOAD( "unico_03.ua6", 0x100000, 0x40000, CRC(e1604821) SHA1(bede6bdd8331128b9f2b229d718133470bf407c9) ) // 27C020
	/* UA7 is unpopulated */
ROM_END

ROM_START( cookbib2b )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "uh12.020",  0x00001, 0x40000, CRC(a44ec1f8) SHA1(0c741bf38f5cf667586cd1925417b6e17dbb8916) )
	ROM_LOAD16_BYTE( "ui12.020",  0x00000, 0x40000, CRC(bdbcd0d1) SHA1(9a6a85a492c21f6dd5daef964071a8a1c62f73c8) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1.512", 0x00000, 0x10000 , CRC(f59f1c9a) SHA1(2830261fd55249e015514fcb4cf8392e83b7fd0d) )

	ROM_REGION( 0x2000, "protection", 0 ) /* P87C52EBPN (XSC6407A) Code (8052) */
	ROM_LOAD( "87c52.mcu", 0x0000, 0x2000 , NO_DUMP ) /* not dumped yet */

	ROM_REGION( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there

	   this one is hacked from the cookbib2 one, absolute code jump needed to be changed at least */
	ROM_LOAD16_WORD_SWAP( "protdata_alt.bin", 0x000, 0x200, BAD_DUMP CRC(bc136ead) SHA1(96459c2ccf7f95880421ba082c2414fa1040f3ed) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15.010", 0x00000, 0x20000, CRC(5e6f76b8) SHA1(725800143dfeaa6093ed5fcc5b9f15678ae9e547) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4.040", 0x000000, 0x80000, CRC(f458d52e) SHA1(f6a145aaa57c64557479e63bb95732a98a7b8b85) )
	ROM_LOAD( "ua6.040", 0x080000, 0x80000, CRC(249e89b4) SHA1(2100eea2c3cee84a046ba7ff6cec1027966b895c) )
	ROM_LOAD( "ua8.040", 0x100000, 0x80000, CRC(caa25138) SHA1(784111255777f5774abf4d34c0a95b5e23a14c9f) )
ROM_END

// PCB is marked: "YFTE3" and "951121" on component side
// EPROMs are labelled: "UNICO"
ROM_START( cookbib2c ) // no ROM matches any of the other sets
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "unico.uh12",  0x00001, 0x40000, CRC(be021efa) SHA1(87e169e4987f4c31bcda147d0bf5f7fdbaa52f38) )
	ROM_LOAD16_BYTE( "unico.ui12",  0x00000, 0x40000, CRC(ed49a0e5) SHA1(fbb6c45559d82ec84253aac5e4e6ff0a8c4d04e0) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "unico.u1", 0x00000, 0x10000 , CRC(f1100b20) SHA1(92c78e028e9020743b68744a759a11638cf2c971) )

	ROM_REGION( 0x2000, "protection", 0 ) /* P87C52EBPN (XSC6407A) Code (8052) */
	ROM_LOAD( "87c52.mcu", 0x0000, 0x2000 , NO_DUMP ) /* not dumped yet */

	ROM_REGION( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there

	   this one is hacked from the old cookbib2 one, absolute code jump needed to be changed at least */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x000, 0x200, BAD_DUMP CRC(b956f056) SHA1(aa9a73e8546b027ae8ef30b03524d302d07cae92) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "unico.uj15", 0x00000, 0x20000, CRC(ae5cc9e5) SHA1(f001c00fea76795ad2d7f4f0e436abecbc7ff00d) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "unico.ua4", 0x000000, 0x80000, CRC(3de7a813) SHA1(fdf7e0a092a2056bd7e6443e035c86dde94a2300) )
	ROM_LOAD( "unico.ua5", 0x080000, 0x80000, CRC(6d543788) SHA1(efcd4f45e2bcdffcdd650c6e3bd543e877ce205a) )
	ROM_LOAD( "unico.ua6", 0x100000, 0x40000, CRC(13cc9bf4) SHA1(dec3561d953699ced17231b76980663fdcd6e155) )
ROM_END

ROM_START( cookbib3 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52.bin",  0x00001, 0x40000, CRC(65134893) SHA1(b1f26794d1a85893aedf55adb2195ad244f90132) )
	ROM_LOAD16_BYTE( "u74.bin",  0x00000, 0x40000, CRC(c4ab8435) SHA1(7f97d3deafb3eb5412a44308ef20d3317405e94c) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35.bin", 0x0c000, 0x4000 ,  CRC(5dfd2a98) SHA1(193c0cd9272144c25cbc3660967424d34d0da185) ) /* bit strange but verified, not the first time SemiCom have done this, see bcstory.. */
	ROM_CONTINUE(0x8000,0x4000)
	ROM_CONTINUE(0x4000,0x4000)
	ROM_CONTINUE(0x0000,0x4000)

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(c819b9a8) SHA1(1d425e8c9940c0e691149e5406dd71808bd73832) )
	/* the 'empty' pattern continued after 0x200 but the game doesn't use it or attempt to decrypt it */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14.bin", 0x00000, 0x20000, CRC(e5bf9288) SHA1(12fb9542f9105fe1a21a74a08cda4d6372b984ee) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u75.bin", 0x000000, 0x80000, CRC(cbe4d9c8) SHA1(81b043bd2b45ab2a8c9df0ba599c6220ed0c9fbf) )
	ROM_LOAD( "u76.bin", 0x080000, 0x80000, CRC(1be17b57) SHA1(57b58cc094d6b47ed6136266f1d34b8bad3f421f) )
	ROM_LOAD( "u77.bin", 0x100000, 0x80000, CRC(7823600d) SHA1(90d431f324b71758c49f3a72ee07701ceb76403f) )
ROM_END

ROM_START( 4in1boot ) /* snow bros, tetris, hyperman 1, pacman 2 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u52",  0x00001, 0x80000, CRC(71815878) SHA1(e3868f5687c1d8ec817671c50ade6c56ee83bfa1) )
	ROM_LOAD16_BYTE( "u74",  0x00000, 0x80000, CRC(e22d3fa2) SHA1(020ab92d8cbf37a9f8186a81934abb97088c16f9) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u35", 0x00000, 0x10000 , CRC(c894ac80) SHA1(ee896675b5205ab2dbd0cbb13db16aa145391d06) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u14", 0x00000, 0x40000, CRC(94b09b0e) SHA1(414de3e36eff85126038e8ff74145b35076e0a43) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u78", 0x000000, 0x200000, CRC(6c1fbc9c) SHA1(067f32cae89fd4d57b90be659d2d648e557c11df) )
ROM_END

ROM_START( snowbro3 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ur4",  0x00000, 0x20000, CRC(19c13ffd) SHA1(4f9db70354bd410b7bcafa96be4591de8dc33d90) )
	ROM_LOAD16_BYTE( "ur3",  0x00001, 0x20000, CRC(3f32fa15) SHA1(1402c173c1df142ff9dd7b859689c075813a50e5) )

	/* the sound is driven by an MCU */
	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "sound.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ua5",        0x000000, 0x80000, CRC(0604e385) SHA1(96acbc65a8db89a7be100f852dc07ba9a0313167) )   /* 16x16 tiles */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* 16x16 BG Tiles */
	ROM_LOAD( "un7",        0x000000, 0x200000, CRC(4a79da4c) SHA1(59207d116d39b9ee25c51affe520f5fdff34e536) )
	ROM_LOAD( "un8",        0x200000, 0x200000, CRC(7a4561a4) SHA1(1dd823369c09368d1f0ec8e1cb85d700f10ff448) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "us5",     0x00000, 0x20000, CRC(7c6368ef) SHA1(53393c570c605f7582b61c630980041e2ed32e2d) )
	ROM_CONTINUE(0x80000,0x60000)
ROM_END

ROM_START( ballboy )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "27c010.ur4",  0x00000, 0x20000, CRC(5fb51b99) SHA1(07e12a3bb51fbb3f192b81497460231c7b609290) )
	ROM_LOAD16_BYTE( "27c010.ur3",  0x00001, 0x20000, CRC(a9c1fdda) SHA1(efb7eaab993f99d89d3b9c159c3b8eb18ace9c2c) )

	/* the sound is driven by an MCU */
	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "sound.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "27c040.ua5",        0x000000, 0x80000, CRC(0604e385) SHA1(96acbc65a8db89a7be100f852dc07ba9a0313167) )   /* 16x16 tiles */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* 16x16 BG Tiles */
	ROM_LOAD( "27c160.un7",        0x000000, 0x200000, CRC(4a79da4c) SHA1(59207d116d39b9ee25c51affe520f5fdff34e536) )
	ROM_LOAD( "27c160.un8",        0x200000, 0x200000, CRC(bfef8c44) SHA1(86930cfcaedbd111d5b985e87a76d2211d2ce2ec) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "27c040.us5",     0x00000, 0x20000, CRC(7c6368ef) SHA1(53393c570c605f7582b61c630980041e2ed32e2d) )
	ROM_CONTINUE(0x80000,0x60000)
ROM_END


ROM_START( ballboy3p ) //PCB etched JOYCUS1B and 2001927
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ur4",  0x00000, 0x20000, CRC(32153d8f) SHA1(1fa698b93507fb775dfff6da8701ab65c986cac5) )
	ROM_LOAD16_BYTE( "ur3",  0x00001, 0x20000, CRC(4d462a75) SHA1(30a84a618bea5c64201329d02847382c2d0c84ba) )

	// the sound is driven by an MCU
	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "sound.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ua5",        0x000000, 0x80000, CRC(fc72011f) SHA1(f1f10b34fd3365c6542299bd0224dad926d650b4) )   // 16x16 tiles

	ROM_REGION( 0x400000, "gfx2", 0 ) // 16x16 BG Tiles
	ROM_LOAD( "un7",        0x000000, 0x400000, CRC(fe427e9d) SHA1(6932ad18b6807af860f8430e2a00e959d6c36a23) )

	ROM_REGION( 0x100000, "oki", 0 )    // OKIM6295 samples
	ROM_LOAD( "us5",     0x00000, 0x20000, CRC(7c6368ef) SHA1(53393c570c605f7582b61c630980041e2ed32e2d) ) // only ROM identical to the 2 player version
	ROM_CONTINUE(0x80000,0x60000)
ROM_END


/*

Information from Korean arcade gaming magazine

name : Final Tetris
author : Jeil computer system
year : 1993.08.24

*/

ROM_START( finalttr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "10.7o",    0x00000, 0x20000, CRC(eecc83e5) SHA1(48088a2fae8852a73a325a9659c24b241515eac3) )
	ROM_LOAD16_BYTE( "9.5o",     0x00001, 0x20000, CRC(58d3640e) SHA1(361bc64174a6c7b15a13e0d1f048c7ea270182ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "12.5r",    0x00000, 0x10000, CRC(4bc21361) SHA1(dab9bea665c0f2fd7cee8ab7f3762e427911bcca) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from
	   shared ram, the MCU puts it there */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x00000, 0x200 , CRC(d5bbb006) SHA1(2f9ce6c4f4f5a304a807134da9c85c68a7b49200) )
	/* after 0xc7 the data read seems meaningless garbage, it doesn't appear to
	   stop at 0x102200, might be worth going back and checking if its simply random
	   values due to ram not being cleared, or actual data */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "11.7p",    0x00000, 0x20000, CRC(2e331022) SHA1(1e74c66d16eb9c8ae04acecbb4040dea037492cc) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "5.1d",     0x00000, 0x40000, CRC(64a450f3) SHA1(d0560f68fe1527fda7852269ec39237ace66ab32) )
	ROM_LOAD( "6.1f",     0x40000, 0x40000, CRC(7281a3cc) SHA1(3f2ed7893bd7c5ff25ecb6eabce78ab66fe532a7) )
	ROM_LOAD( "7.1g",     0x80000, 0x40000, CRC(ec80f442) SHA1(870e44d28490a324f74af554604b9daa8422b86f) )
	ROM_LOAD( "9.1h",     0xc0000, 0x40000, CRC(2ebd316d) SHA1(2f1249ebd2a0bb0cc15259f7187201576a365fa6) )
ROM_END



ROM_START( suhosong )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "su_ho_sung.uh12",  0x00001, 0x20000, CRC(6bd8bd08) SHA1(668398c9c77cc4cc52858daefd3cb13fbaf29a37) )
	ROM_LOAD16_BYTE( "su_ho_sung.ui12",  0x00000, 0x20000, CRC(79a4806e) SHA1(a4080ea70fa588ada384ffa9877f5cf965fb68df) )


	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "su_ho_sung.u1", 0x00000, 0x10000 ,  CRC(509ce74e) SHA1(a93add5ab674671078b55128281dcf9b0db46617) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASE00 ) /* Data from Shared RAM */
	ROM_LOAD16_WORD( "protdata.bin", 0x000, 0x200 , CRC(4478e251) SHA1(08489d6bfe5503c8eb62909e56a07193a922b4c1) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "su_ho_sung.uj15", 0x00000, 0x40000, CRC(266fcae8) SHA1(0f15f880bde0c12b5c663ed387f9353c13b731b6) )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "su_ho_sung.ua4", 0x000000, 0x80000, CRC(bc83a944) SHA1(fbd46648107c66f328b0a61c74b6b82c718e6f4b) )
	ROM_LOAD( "su_ho_sung.ua5", 0x080000, 0x80000, CRC(a1907ea4) SHA1(e21c29d12e50cce1434dbaff0929c207bfd33344) )
	ROM_LOAD( "su_ho_sung.ua6", 0x100000, 0x80000, CRC(92fea02c) SHA1(946c7bf55354875a1581ce484cb185b640f74166) )
ROM_END


ROM_START( yutnori )
	ROM_REGION( 0x80000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "sys_d0-d7",    0x000001, 0x20000, CRC(d5c853da) SHA1(578f29c3a307f82fcaa23a7fe2931c0a673e777e) )
	ROM_LOAD16_BYTE( "sys_d8-d15",   0x000000, 0x20000, CRC(bf108119) SHA1(e64f64ddb577d6750cbc3a6c9d8d2ec4482cafe0) )

	ROM_REGION( 0x4010, "mcu", 0 )    /* PIC code */
	ROM_LOAD( "pic16c64a-04-p",    0x000000, 0x4010, BAD_DUMP CRC(46fd3671) SHA1(54cf7a38f7743cdad73a2741183b2720ee42e6c8) ) // dump seems to be 99% empty, protected, only configuration bytes dumped?

	ROM_REGION( 0x120000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "graphics_rom_1",    0x000000, 0x80000, CRC(d4881b49) SHA1(e169b7eca48a0bd66ad55fe21197a4bb491198bb) )
	ROM_LOAD( "graphics_rom_2",    0x080000, 0x80000, CRC(8cd9ce60) SHA1(d1db929ca6128ec2ebe983e6161f200ba421bd31) )
	ROM_LOAD( "graphics_rom_3",    0x100000, 0x20000, CRC(04f7c2ac) SHA1(927fa0f76ff1801845776d47aa5311f485b0b809) )

	ROM_REGION(0x80000, "oki1", 0 ) // 2 banks, 1 large sample in each (music)
	ROM_LOAD("sound rom", 0x000000, 0x080000, CRC(d24c2e43) SHA1(5c7a130048463558d695857ffc056a95a8072219) )

	ROM_REGION(0x40000, "oki2", 0 )
	ROM_LOAD("voice_rom", 0x000000, 0x040000, CRC(25e85201) SHA1(6c0001e2942f49b62e1bbf3a68c59abad1e2f94c) )
ROM_END


ROM_START( sutjarod ) // 숫자로 해요 Deluxe
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ul12_am27c512.bin",    0x00000, 0x10000, CRC(50e2563c) SHA1(d02e0805ae5aa5058e8c096f4265603006de2a6d) )
	ROM_LOAD16_BYTE( "uh12_am27c512.bin",    0x00001, 0x10000, CRC(dc7e036c) SHA1(90a9b29b3f776e26303ccfa74800501152e05ceb) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 Code */
	ROM_LOAD( "u1_am27c512.bin",    0x00000, 0x10000, CRC(c6e31c4a) SHA1(178e10432d9d1096ac0d11b0da9e5441279f5ffe) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real ROM but instead the data extracted from
	   shared RAM - the MCU puts it there

	   this is taken from cookbib3, the key in the program ROM is
	   patched in our init function to decrypt it, real data is
	   still needed.
	   */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, BAD_DUMP CRC(c819b9a8) SHA1(1d425e8c9940c0e691149e5406dd71808bd73832) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uj15_tms27c010a.bin",    0x00000, 0x20000, CRC(da9dea73) SHA1(b72da82686054dbbe423724fecf0e58966b0ea45) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "ua4_am27c040.bin",     0x00000, 0x80000, CRC(1e87d377) SHA1(b4b076026ff60bfb25172d5cc3b2b410599583ea) )
	ROM_LOAD( "ua5_am27c040.bin",     0x80000, 0x80000, CRC(b286427d) SHA1(ee7f98a9d451b901c211326bff6b3ed4e2c1b76f) )
ROM_END



void snowbros_state::init_cookbib2()
{
	save_item(NAME(m_semicom_prot_offset));

	m_semicom_prot_base = 0xf000 / 2;
}


uint16_t snowbros_state::_4in1_02_read()
{
	return 0x0202;
}

void snowbros_state::init_4in1boot()
{
	uint8_t *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* strange order */
	{
		std::vector<uint8_t> buffer(len);
		for (int i = 0;i < len; i++)
			if (i & 1) buffer[i] = bitswap<8>(src[i],6,7,5,4,3,2,1,0);
			else buffer[i] = src[i];

		memcpy(src, &buffer[0], len);
	}

	src = memregion("soundcpu")->base();
	len = memregion("soundcpu")->bytes();

	/* strange order */
	{
		std::vector<uint8_t> buffer(len);
		for (int i = 0;i < len; i++)
			buffer[i] = src[i^0x4000];
		memcpy(src,&buffer[0],len);
	}
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x200001, read16smo_delegate(*this, FUNC(snowbros_state::_4in1_02_read)));
}

void snowbros_state::init_snowbro3()
{
	uint8_t *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* strange order */
	{
		std::vector<uint8_t> buffer(len);
		for (int i = 0;i < len; i++)
			buffer[i] = src[bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,3,4,1,2,0)];
		memcpy(src,&buffer[0],len);
	}

	save_item(NAME(m_sb3_music_is_playing));
	save_item(NAME(m_sb3_music));
}

void snowbros_state::init_ballboy3p()
{
	init_snowbro3();

	m_maincpu->space(AS_PROGRAM).unmap_write(0x400000, 0x400001); // unmap flipscreen as the DSW has been removed in favor of the controls for the 3rd player
}

uint16_t snowbros_state::_3in1_read()
{
	return 0x000a;
}

void snowbros_state::init_3in1semi()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x200001, read16smo_delegate(*this, FUNC(snowbros_state::_3in1_read)));
}


uint16_t snowbros_state::cookbib3_read()
{
	return 0x2a2a;
}

void snowbros_state::init_cookbib3()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x200001, read16smo_delegate(*this, FUNC(snowbros_state::cookbib3_read)));
}

void snowbros_state::init_sutjarod()
{
	uint16_t *rom = (uint16_t*)memregion("maincpu")->base();
	// patch RAM decrypt key in ROM to be the same as cookbib3
	rom[0xb994 / 2] = 0x434b;
	rom[0xb996 / 2] = 0x3345;

	// anti-tamper checksum - keep checksum the same by putting a value in an unused area at the start of ROM
	// (the game does a ROM checksum between scenes, expects result to be 0, or goes off the rails)
	rom[0x54 / 2] = 0x2508;
}

void snowbros_state::init_pzlbreak()
{
	m_pandora->set_bg_pen(0xc0);
}



void snowbros_state::init_toto()
{
	// every single rom has bits 0x10 and 0x08 swapped
	uint8_t *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	for (int i = 0; i < len; i++)
	{
		src[i] = bitswap<8>(src[i], 7, 6, 5, 3, 4, 2, 1, 0);
	}

	src = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();

	for (int i = 0; i < len; i++)
	{
		src[i] = bitswap<8>(src[i], 7, 6, 5, 3, 4, 2, 1, 0);
	}

	src = memregion("soundcpu")->base();
	len = memregion("soundcpu")->bytes();

	for (int i = 0; i < len; i++)
	{
		src[i] = bitswap<8>(src[i], 7, 6, 5, 3, 4, 2, 1, 0);
	}

	// protection? (just return 0x07)
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x500006, 0x500007, read16s_delegate(*this, FUNC(snowbros_state::toto_read)));
}

void snowbros_state::init_hyperpac()
{
	save_item(NAME(m_semicom_prot_offset));
	m_semicom_prot_base = 0xe000 / 2;
}


void snowbros_state::init_yutnori()
{
	m_yutnori_prot_val = 0;
	m_pandora->set_bg_pen(0xf0);
	save_item(NAME(m_yutnori_prot_val));
}


GAME( 1990, snowbros,   0,        snowbros,    snowbros, snowbros_state, empty_init,    ROT0, "Toaplan",                        "Snow Bros. - Nick & Tom (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbrosa,  snowbros, snowbros,    snowbros, snowbros_state, empty_init,    ROT0, "Toaplan",                        "Snow Bros. - Nick & Tom (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbrosb,  snowbros, snowbros,    snowbros, snowbros_state, empty_init,    ROT0, "Toaplan",                        "Snow Bros. - Nick & Tom (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbrosc,  snowbros, snowbros,    snowbros, snowbros_state, empty_init,    ROT0, "Toaplan",                        "Snow Bros. - Nick & Tom (set 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbrosj,  snowbros, snowbros,    snowbroj, snowbros_state, empty_init,    ROT0, "Toaplan",                        "Snow Bros. - Nick & Tom (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbrosd,  snowbros, snowbros,    snowbroj, snowbros_state, empty_init,    ROT0, "Toaplan (Dooyong license)",      "Snow Bros. - Nick & Tom (Dooyong license)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, wintbob,    snowbros, wintbob,     snowbros, snowbros_state, empty_init,    ROT0, "bootleg (Sakowa Project Korea)", "The Winter Bobble (bootleg of Snow Bros.)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, snowbroswb, snowbros, wintbob,     snowbros, snowbros_state, empty_init,    ROT0, "bootleg",                        "Snow Bros. - Nick & Tom (The Winter Bobble hardware bootleg)", MACHINE_SUPPORTS_SAVE ) // this was probably unhacked back from the more common Winter Bobble to make it look more original

GAME( 1996, toto,       0,        snowbros,    snowbros, snowbros_state, init_toto,     ROT0, "SoftClub",                       "Come Back Toto", MACHINE_SUPPORTS_SAVE ) // modified from 'snowbros' code

// none of the games below are on genuine SnowBros hardware, but they clone the functionality of it.

// SemiCom / Jeil titles are protected, a dumb MCU copies code into RAM at startup, some also check for a specific return value from an address on startup.
GAME( 1993, finalttr,   0,        finalttr,    finalttr, snowbros_state, empty_init,    ROT0, "Jeil Computer System", "Final Tetris", MACHINE_SUPPORTS_SAVE )
GAME( 1995, hyperpac,   0,        semicom_mcu, hyperpac, snowbros_state, init_hyperpac, ROT0, "SemiCom",              "Hyper Pacman", MACHINE_SUPPORTS_SAVE )
GAME( 1995, hyperpacb,  hyperpac, semicom,     hyperpac, snowbros_state, empty_init,    ROT0, "bootleg",              "Hyper Pacman (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, cookbib2,   0,        semicom_mcu, cookbib2, snowbros_state, init_cookbib2, ROT0, "SemiCom",              "Cookie & Bibi 2 (English, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, cookbib2a,  cookbib2, semicom_mcu, cookbib2, snowbros_state, init_cookbib2, ROT0, "SemiCom",              "Cookie & Bibi 2 (English, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, cookbib2b,  cookbib2, semiprot,    cookbib2, snowbros_state, init_cookbib2, ROT0, "SemiCom",              "Cookie & Bibi 2 (English, set 3)", MACHINE_SUPPORTS_SAVE ) // older? test mode looks even worse on this, but neither shows the correct dip info anyway
GAME( 1996, cookbib2c,  cookbib2, semiprot,    cookbib2c,snowbros_state, init_cookbib2, ROT0, "SemiCom",              "Cookie & Bibi 2 (English / Korean)", MACHINE_SUPPORTS_SAVE ) // in this set the language switch actually works but its effects are inverted to what test mode shows. Sticker on PCB shows 95 so maybe earliest set?
GAME( 1996, toppyrap,   0,        semiprot,    toppyrap, snowbros_state, empty_init,    ROT0, "SemiCom",              "Toppy & Rappy", MACHINE_SUPPORTS_SAVE )
GAME( 1996, sutjarod,   0,        semiprot,    sutjarod, snowbros_state, init_sutjarod, ROT0, "SemiCom",              "Sutjaro Haeyo Deluxe", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cookbib3,   0,        semiprot,    cookbib3, snowbros_state, init_cookbib3, ROT0, "SemiCom",              "Cookie & Bibi 3", MACHINE_SUPPORTS_SAVE )
GAME( 1997, pzlbreak,   0,        semiprot,    pzlbreak, snowbros_state, init_pzlbreak, ROT0, "SemiCom / Tirano",     "Puzzle Break (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, pzlbreaka,  pzlbreak, semiprot,    pzlbreak, snowbros_state, init_pzlbreak, ROT0, "SemiCom / Tirano",     "Puzzle Break (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, suhosong,   0,        semiprot,    suhosong, snowbros_state, empty_init,    ROT0, "SemiCom",              "Su Ho Seong", MACHINE_SUPPORTS_SAVE )
GAME( 1997, twinkle,    0,        semiprot,    twinkle,  snowbros_state, empty_init,    ROT0, "SemiCom / Tirano",     "Twinkle (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, twinklea,   twinkle,  semiprot,    twinkle,  snowbros_state, empty_init,    ROT0, "SemiCom / Tirano",     "Twinkle (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, 3in1semi,   0,        semiprot,    moremore, snowbros_state, init_3in1semi, ROT0, "SemiCom / XESS",       "New HyperMan (3-in-1 with Cookie & Bibi & HyperMan) (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, 3in1semia,  3in1semi, semiprot,    moremore, snowbros_state, init_3in1semi, ROT0, "SemiCom / XESS",       "New HyperMan (3-in-1 with Cookie & Bibi & HyperMan) (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, mcheonru,   0,        semiprot,    mcheonru, snowbros_state, init_3in1semi, ROT0, "SemiCom / AceVer",     "Ma Cheon Ru", MACHINE_SUPPORTS_SAVE ) // a flyer exists for an English version called Arirang, AceVer team logo is displayed on it
GAME( 1999, moremore,   0,        semiprot,    moremore, snowbros_state, init_3in1semi, ROT0, "SemiCom / Exit",       "More More", MACHINE_SUPPORTS_SAVE )
GAME( 1999, moremorp,   0,        semiprot,    moremore, snowbros_state, init_3in1semi, ROT0, "SemiCom / Exit",       "More More Plus", MACHINE_SUPPORTS_SAVE )
// This is very similar to the SemiCom titles, but unprotected.
GAME( 2002, 4in1boot,   0,        _4in1,       4in1boot, snowbros_state, init_4in1boot, ROT0, "K1 Soft", "Puzzle King (PacMan 2, Tetris, HyperMan 2, Snow Bros.)" , MACHINE_SUPPORTS_SAVE )

GAME( 1995, honeydol,   0,        honeydol,    honeydol, snowbros_state, empty_init,    ROT0, "Barko Corp.", "Honey Doll", MACHINE_SUPPORTS_SAVE ) // based on snowbros code..

GAME( 1995, twinadv,    0,        twinadv,     twinadv,  snowbros_state, empty_init,    ROT0, "Barko Corp.", "Twin Adventure (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinadvk,   twinadv,  twinadv,     twinadv,  snowbros_state, empty_init,    ROT0, "Barko Corp.", "Twin Adventure (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, multi96,    twinadv,  twinadv,     twinadv,  snowbros_state, empty_init,    ROT0, "Barko Corp.", "Multi Game '96 (Italy)", MACHINE_SUPPORTS_SAVE )

// The Korean games database shows an earlier version of this called Ball Boy with a different title screen to the version of Ball Boy we have
// http://mamedev.emulab.it/undumped/images/Ballboy.jpg
// it is possible this 'ball boy' is the original bootleg, with snwobro3 being a hack of that, and the ballboy set we have a further hack of that
// these use an MCU to drive the sound
GAME( 2002, snowbro3,   0,        snowbro3,    snowbroj,  snowbros_state, init_snowbro3,  ROT0, "Syrmex",  "Snow Brothers 3 - Magical Adventure", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // hacked from SnowBros code but released as an original game
GAME( 2003, ballboy,    snowbro3, snowbro3,    snowbroj,  snowbros_state, init_snowbro3,  ROT0, "bootleg", "Ball Boy (2 players)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2004, ballboy3p,  snowbro3, snowbro3,    ballboy3p, snowbros_state, init_ballboy3p, ROT0, "bootleg", "Ball Boy (3 players)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
// protection appears to handle the sound, should check if it's just a block of code that is conditionally executed like some of the Semicom titles.
GAME( 1999, yutnori,    0,        yutnori,     yutnori,   snowbros_state, init_yutnori,   ROT0, "Nunal",   "Puzzle Yutnori (Korea)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND ) // Nunal is apparently Korean slang for Eyeball, hence the logo.  Some places report 'JCC Soft' as the manufacturer
