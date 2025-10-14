// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina
/***************************************************************************

Legionnaire (c) Tad 1992
-----------

David Graves

Made from MAME D-con and Toki drivers (by Bryan McPhail, Jarek Parchanski)


Heated Barrel looks like a minor revision of the Legionnaire
hardware. It has a graphics banking facility, which doubles the 0xfff
different tiles available for use in the foreground layer.


Board names displayed on test mode menu:
    SYS68C2 BOARD       Seibu Cup Soccer
    SYS68C3 BOARD       SD Gundam Sangokushi Rainbow Tairiku Senki
    Denden Makamaka     Denjin Makai
    (none)              Godzilla

Heated Barrel includes only the I/O test, and Legionnaire only has a DSW
display. These can be accessed by holding down the P1 joystick at reset.
(Legionnaire's program contains remnants of a more complete test mode.)


TODO
----

Check work RAM boundaries, they are likely to be too generous right now


Legionnaire
-----------

Need 16 px off top of vis area?


Denjin Makai
------------

- Palette Ram format correct,but color offset wrong? Probably protection related,game updates paletteram with rom data (or bad program rom?)..

- Needs to patch a sound-related comm to make this to work, same as SD Gundam Psycho Salamander No Kyoui (68k never writes to port 6 for whatever reason).

- there are some ROM writes from time to time, could be a coding bug or something related to the protection.

- There's a minor gap in one of the transitions during final stage, after the conveyor belt and the horizontal elevator (the one with tons of grenades), this is known to not happen on a real PCB.

- "Mai T Joplin" text in credits is slightly off of screen area even on real HW (btanb)

- Initials can be entered into ranking screen only if player(s) never ever continues (btanb)

Godzilla
--------

The COP-MCU appears to write to the work ram area, otherwise it resets in mid-animation
of the title screen.


Preliminary COP MCU memory map
------------------------------

0x400-0x5ff   Protection related:
0x400         Sprite parameter (color)
0x414-0x412   Sprite DMA source address
0x478         Layer clearance (?)
0x48e         X limiter (SD Gundam)
0x48c         Y limiter (SD Gundam)
0x4c0-0x4a0   COP reg 0
0x4c2-0x4a2   COP reg 1
0x4c4-0x4a4   COP reg 2
0x4c6-0x4a6   COP reg 3
0x580(r)      Hit Check (bit 1 & bit 0)
0x590-0x59c(r)BCD number
0x5b0         DMA bit flag (bit 1)
0x600-0x6ff   Includes standard screen control words
0x700-0x7ff   Includes standard Seibu sound system


***************************************************************************/

#include "emu.h"
#include "legionna.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"


/*****************************************************************************/

u8 legionna_state::denjinmk_sound_comms_r(offs_t offset)
{
	// Routine at 5FDC spins indefinitely until the lowest bit becomes 1
	if (offset == 10) // ($100714)
		return 1;

	return m_seibu_sound->main_r((offset >> 1) & 7);
}

void legionna_state::legionna_cop_map(address_map &map)
{
	// grainbow sprite DMA
	// the three NOP writes are initied before every sprite DMA
	map(0x100400, 0x100401).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_param_lo_w));
	map(0x100402, 0x100403).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_param_hi_w));
	map(0x100404, 0x100405).nopw(); // $0002
	map(0x10040a, 0x10040b).nopw(); // $ffff
	map(0x10040c, 0x10040d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_size_w));
	map(0x10040e, 0x10040f).nopw(); // $0023
	map(0x100410, 0x100411).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_inc_w));
	map(0x100412, 0x100413).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_src_hi_w));
	map(0x100414, 0x100415).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_src_lo_w));

	map(0x10041c, 0x10041d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_target_w)); // angle target (for 0x6200 COP macro)
	map(0x10041e, 0x10041f).w(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_step_w));   // angle step   (for 0x6200 COP macro)

	map(0x100420, 0x100421).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_low_w));
	map(0x100422, 0x100423).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_high_w));
	map(0x100424, 0x100425).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_mode_w));
	map(0x100428, 0x100429).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_v1_w));
	map(0x10042a, 0x10042b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_v2_w));
	map(0x10042c, 0x10042d).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_prng_maxvalue_r), FUNC(raiden2cop_device::cop_prng_maxvalue_w));

	map(0x100432, 0x100433).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_data_w));
	map(0x100434, 0x100435).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_addr_w));
	map(0x100436, 0x100437).w(m_raiden2cop, FUNC(raiden2cop_device::cop_hitbox_baseadr_w));
	map(0x100438, 0x100439).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_value_w));
	map(0x10043a, 0x10043b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_mask_w));
	map(0x10043c, 0x10043d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_trigger_w));
	//map(0x10043e, 0x10043f).rw(m_raiden2cop, ...);    /*  0 in all 68k based games,   0xffff in raiden2 / raidendx,   0x2000 in zeroteam / xsedae , it's always set up just before the 0x474 register */

	map(0x100440, 0x100441).w(m_raiden2cop, FUNC(raiden2cop_device::cop_unk_param_a_w));
	map(0x100442, 0x100443).w(m_raiden2cop, FUNC(raiden2cop_device::cop_unk_param_b_w));
	map(0x100444, 0x100445).w(m_raiden2cop, FUNC(raiden2cop_device::cop_scale_w));
	map(0x100446, 0x100447).w(m_raiden2cop, FUNC(raiden2cop_device::cop_rom_addr_hi_w)); // cupsoc
	map(0x100448, 0x100449).w(m_raiden2cop, FUNC(raiden2cop_device::cop_rom_addr_lo_w)); // cupsoc
	map(0x10044a, 0x10044b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_precmd_w)); // cupsoc

	map(0x100450, 0x100451).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_ram_addr_hi_w));
	map(0x100452, 0x100453).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_ram_addr_lo_w));
	map(0x100454, 0x100455).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_lookup_hi_w));
	map(0x100456, 0x100457).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_lookup_lo_w));
	map(0x100458, 0x100459).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_param_w));
	map(0x10045a, 0x10045b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pal_brightness_val_w)); //palette DMA brightness val, used by X Se Dae / Zero Team
	map(0x10045c, 0x10045d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pal_brightness_mode_w));  //palette DMA brightness mode, used by X Se Dae / Zero Team (sets to 5)

//  map(0x100470, 0x100471).rw(FUNC(legionna_state::cop_tile_bank_2_r), FUNC(legionna_state::cop_tile_bank_2_w));
//  map(0x100474, 0x100475).w(m_raiden2cop, FUNC(raiden2cop_device::...)); // this gets set to a pointer to spriteram (relative to start of ram) on all games except raiden 2, where it isn't set
	map(0x100476, 0x100477).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_adr_rel_w));
	map(0x100478, 0x100479).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_src_w));
	map(0x10047a, 0x10047b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_size_w));
	map(0x10047c, 0x10047d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_dst_w));
	map(0x10047e, 0x10047f).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_mode_r), FUNC(raiden2cop_device::cop_dma_mode_w));

//  map(0x100488, 0x100489).nopw(); grainbow $0010
//  map(0x10048a, 0x10048b).nopw(); grainbow $0000
	map(0x10048c, 0x10048d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_abs_y_w)); // 68k
	map(0x10048e, 0x10048f).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sprite_dma_abs_x_w)); // 68k

	map(0x1004a0, 0x1004ad).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_reg_high_r), FUNC(raiden2cop_device::cop_reg_high_w));
	map(0x1004c0, 0x1004cd).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_reg_low_r), FUNC(raiden2cop_device::cop_reg_low_w));

//  map(0x100500, 0x100505).w(FUNC(legionna_state::cop_cmd_w)); // ADD ME
	map(0x100500, 0x100505).w(m_raiden2cop, FUNC(raiden2cop_device::LEGACY_cop_cmd_w)); // REMOVE ME

	map(0x100580, 0x100581).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_r));
	map(0x100582, 0x100587).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_val_r));


	map(0x100588, 0x100589).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_stat_r));
	map(0x100590, 0x100599).r(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_digits_r));

	map(0x1005a0, 0x1005a7).r(m_raiden2cop, FUNC(raiden2cop_device::cop_prng_r));

	map(0x1005b0, 0x1005b1).r(m_raiden2cop, FUNC(raiden2cop_device::cop_status_r));
	map(0x1005b2, 0x1005b3).r(m_raiden2cop, FUNC(raiden2cop_device::cop_dist_r));
	map(0x1005b4, 0x1005b5).r(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_r));

	map(0x1006fc, 0x1006fd).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_trigger_w));
	map(0x1006fe, 0x1006ff).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_dma_trig_w)); // sort-DMA trigger
}


void legionna_state::legionna_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100470, 0x100471).nopw(); // toggles 0x2000 / 0x0000, tile bank on some games
	map(0x100600, 0x10064f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x10071f).lrw8(
								 NAME([this](offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this](offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x102000, 0x1027ff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102800, 0x103fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x104000, 0x104fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");    /* palette xRRRRxGGGGxBBBBx ? */
	map(0x105000, 0x105fff).ram().share("spriteram");
	map(0x106000, 0x107fff).ram();
	map(0x108000, 0x11ffff).ram(); /* main ram */
}


void legionna_state::heatbrl_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100470, 0x100471).w(FUNC(legionna_state::heatbrl_setgfxbank));
	map(0x100600, 0x100601).nopw(); // irq ack?
	map(0x100640, 0x10068f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x1007c0, 0x1007df).lrw8(
								 NAME([this] (offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100800, 0x100fff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102000, 0x102fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x103000, 0x103fff).ram().share("spriteram");
	map(0x104000, 0x104fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");
	map(0x108000, 0x11ffff).ram();
}

void legionna_state::godzilla_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100470, 0x100471).w(FUNC(legionna_state::heatbrl_setgfxbank));
	map(0x100600, 0x10064f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x10071f).lrw8(
								 NAME([this] (offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x100800, 0x100fff).ram();
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x102000, 0x1027ff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102800, 0x103fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x104000, 0x104fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");
	map(0x105000, 0x105fff).ram().share("spriteram");
	map(0x106000, 0x1067ff).ram();
	map(0x106800, 0x106fff).ram();
	map(0x107000, 0x107fff).ram(); /*Ani-DSP ram*/
	map(0x108000, 0x11ffff).ram();
//  map(0xff0000, 0xffffff).ram(); // game reads here at Biollante stage in story mode (i.e. when Super X starts shooting).
								   // Development leftover/coding bug? Game doesn't seem to care at all anyway.
}

// additional z80 i/o port, present only in Godzilla (512KB OKI ROM vs 256KB)
// Notice Denjin Makai has a 512KB OKI ROM too but latter half is empty
void legionna_state::godzilla_oki_bank_w(u8 data)
{
	// bit 1 used, unknown purpose (always on?)
	m_oki->set_rom_bank(data & 1);
	if((data & 0xfe) != 0x02)
		printf("oki_bank_w %02x!\n",data);
}

void legionna_state::godzilla_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(legionna_state::godzilla_oki_bank_w));
}

// Denjin Makai: Looks like they specifically swapped address line A1 in this range?
// Initially thought it was a palette DMA mode 4 but it doesn't apply for Godzilla, causing color bugs in the background tilemap.
void legionna_state::palette_swap_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset^=1;
	COMBINE_DATA(&m_swappal[offset]);
}

void legionna_state::denjinmk_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100470, 0x100471).w(FUNC(legionna_state::denjinmk_setgfxbank));
	map(0x100600, 0x10064f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x10071f).r(FUNC(legionna_state::denjinmk_sound_comms_r))
			.lw8(NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x10075c, 0x10075d).portr("DSW2");
	map(0x100800, 0x100fff).ram();
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x102000, 0x1027ff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102800, 0x103fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x104000, 0x104fff).ram().w(FUNC(legionna_state::palette_swap_w)).share("swappal");
	map(0x105000, 0x105fff).ram().share("spriteram");
	map(0x106000, 0x107fff).ram();
	map(0x108000, 0x11dfff).ram();
	map(0x11e000, 0x11efff).ram();
	map(0x11f000, 0x11ffff).ram();
}

void legionna_state::grainbow_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100480, 0x100487).w(FUNC(legionna_state::grainbow_layer_config_w)); // probably a COP feature
	map(0x100600, 0x10064f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x10071f).lrw8(
								 NAME([this](offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this](offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x10075c, 0x10075d).portr("DSW2");
	map(0x100800, 0x100fff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102000, 0x102fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x103000, 0x103fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");
	map(0x104000, 0x104fff).ram(); // .w(FUNC(legionna_state::paletteram_xBBBBBGGGGGRRRRR_word_w)).share("paletteram");
	map(0x105000, 0x105fff).ram();
	map(0x106000, 0x106fff).ram();
	map(0x107000, 0x107fff).ram().share("spriteram");
	map(0x108000, 0x11ffff).ram();
}

void legionna_state::cupsoc_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100600, 0x10064f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x10071f).lrw8(
								 NAME([this] (offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100740, 0x100741).portr("DSW1");
	map(0x100744, 0x100745).portr("PLAYERS12");
	map(0x100748, 0x100749).portr("PLAYERS34");
	map(0x10074c, 0x10074d).portr("SYSTEM");
	map(0x10075c, 0x10075d).portr("DSW2");
	map(0x100800, 0x100fff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102000, 0x102fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x103000, 0x103fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");
	map(0x104000, 0x104fff).ram();
	map(0x105000, 0x106fff).ram();
	map(0x107000, 0x1077ff).ram().share("spriteram");
	map(0x107800, 0x107fff).ram(); /*Ani Dsp(?) Ram*/
	map(0x108000, 0x10ffff).ram();
	map(0x110000, 0x119fff).ram();
	map(0x11a000, 0x11dfff).ram();
	map(0x11e000, 0x11ffff).ram(); /*Stack Ram*/
}

void legionna_state::cupsocs_map(address_map &map)
{
	legionna_cop_map(map);
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1003ff).ram();
	map(0x100600, 0x10067f).lrw16(
								  NAME([this](offs_t offset) {
									  return m_crtc->read(offset ^ 0x20);
								  }),
								  NAME([this](offs_t offset, u16 data) {
									  m_crtc->write(offset ^ 0x20, data);
								  }));
	map(0x100680, 0x100681).nopw(); // irq ack?
	map(0x100700, 0x100701).portr("DSW1");
	map(0x100704, 0x100705).portr("PLAYERS12");
	map(0x100708, 0x100709).portr("PLAYERS34");
	map(0x10070c, 0x10070d).portr("SYSTEM");
	map(0x10071c, 0x10071d).portr("DSW2");
	map(0x100740, 0x10075f).lrw8(
								 NAME([this] (offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
								 NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);
	map(0x100800, 0x100fff).ram(); // .w(FUNC(legionna_state::background_w)).share("back_data");
	map(0x101000, 0x1017ff).ram(); // .w(FUNC(legionna_state::foreground_w).share("fore_data");
	map(0x101800, 0x101fff).ram(); // .w(FUNC(legionna_state::midground_w).share("mid_data");
	map(0x102000, 0x102fff).ram(); // .w(FUNC(legionna_state::text_w).share("textram");
	map(0x103000, 0x103fff).ram(); // .w("palette", FUNC(palette_device::write)).share("palette");
	map(0x104000, 0x104fff).ram();
	map(0x105000, 0x106fff).ram();
	map(0x107000, 0x1077ff).ram().share("spriteram");
	map(0x107800, 0x107fff).ram(); /*Ani Dsp(?) Ram*/
	map(0x108000, 0x10ffff).ram();
	map(0x110000, 0x119fff).ram();
	map(0x11a000, 0x11dfff).ram();
	map(0x11e000, 0x11ffff).ram(); /*Stack Ram*/
}


/*****************************************************************************/

// All inputs are defined to trigger through test mode for each game (if available).
// These inputs/dips may or may not coincide with actual in-game usage, however.

static INPUT_PORTS_START( legionna )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // ???
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // ???
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // ???
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // ???

	PORT_START("DSW1") // Note: If the joystick is held in any direction at power-on the DIP switches are shown on screen.
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" )                PORT_DIPLOCATION("SW01:1")
	PORT_DIPSETTING(      0x0001, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) )         PORT_DIPLOCATION("SW01:2,3,4,5") PORT_CONDITION("DSW1", 0x0001, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ))           PORT_DIPLOCATION("SW01:2,3") PORT_CONDITION("DSW1", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ))           PORT_DIPLOCATION("SW01:4,5") PORT_CONDITION("DSW1", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )         PORT_DIPLOCATION("SW01:6") // manual says 'NOT IN USE'
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" )                   PORT_DIPLOCATION("SW01:7")  // manual says 'NOT IN USE'
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("SW01:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )           PORT_DIPLOCATION("SW02:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0400, 0x0400, "Extend Play" )              PORT_DIPLOCATION("SW02:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )         PORT_DIPLOCATION("SW02:4") // manual says 'NOT IN USE'
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW02:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW02:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("SW02:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )

	PORT_START("PLAYERS34")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Some sort of rotational input?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ???
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Also read but not used
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( heatbrl )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN4 )    // haven't found coin4, maybe it doesn't exist
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PLAYERS34")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW1") // Note: If any player 1 button is held down at power-on the DIP switches are shown on screen for 40 seconds.
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" )                PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) )         PORT_DIPLOCATION("SW1:2,3,4,5") PORT_CONDITION("DSW1", 0x0001, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ))           PORT_DIPLOCATION("SW1:2,3") PORT_CONDITION("DSW1", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ))           PORT_DIPLOCATION("SW1:4,5") PORT_CONDITION("DSW1", 0x0001, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0060, 0x0060, "Cabinet Setting" )          PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0060, "2 Players & 1 Coin Slot" )
	PORT_DIPSETTING(      0x0040, "4 Players & 1 Coin Slot" )
	PORT_DIPSETTING(      0x0020, "4 Players (2x 2P Linked) & 1-4 Coin Slots" )
	PORT_DIPSETTING(      0x0000, "4 Players & 4 Coin Slots" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Players Start & Join" )     PORT_DIPLOCATION("SW2:3,4") // Listed as-is from the manual but test mode shows different text for 2 choices
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )     // 1 or 2 players can start
	PORT_DIPSETTING(      0x0800, "2 Start, 1 Can Join" ) // test shows 'Double Coin Start'. This is for a 4 player cab.
	PORT_DIPSETTING(      0x0400, "2 Start, 2 Can Join" ) // test shows 'Normal'... probably for a 4-player cab or 2 linked cabs with a minimum of 2 players.
	PORT_DIPSETTING(      0x0000, "1 Start, 2 Can Join" ) // test shows 'Double Coin Start'... also probably for a 4 player cab or 2 linked cabs.
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( godzilla )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("PLAYERS34")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( grainbow )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1) //debug button
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PLAYERS34")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3  ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW1") // DIP switch sheet or manual needed to improve this.
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )         PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2")
	PORT_DIPSETTING(      0x0003, "3")
	PORT_DIPSETTING(      0x0000, "5")
	PORT_DIPSETTING(      0x0001, "4")
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW1:3,4") // could be something else, not difficulty???
	PORT_DIPSETTING(      0x000c, "2" ) // Internal value stored at 0x1086cc and often used as table offset
	PORT_DIPSETTING(      0x0008, "0" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0000, "4" ) // Was one of these settings intended to be 6?
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:7") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW3:8") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( denjinmk )
	// SEIBU_COIN_INPUTS override
	PORT_START("COIN")      /* coin inputs read through sound cpu, an impulse of 4 frame is too much for this game, especially for coin 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PLAYERS34")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW1") // DIP switch sheet or manual needed to improve this.
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )                    PORT_DIPLOCATION("SW1:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Language ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Japanese ) ) // On English it skips the story entirely, so leave JP as default
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED ) //unused according to the test mode
INPUT_PORTS_END


static INPUT_PORTS_START( cupsoc )
	// p3 and p4 inputs are routed thru two 10-pins on lower-left of PCB
	// TODO: dip-conditional with coin slots... Not actually needed as there are no conditional DIPs on this game.
	// SEIBU_COIN_INPUTS override
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) //TEST
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	// 9-10 of p4 connector
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START4 )
	// 9-10 of p3 presumably routes anywhere on following (GND on manuals)
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYERS12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Pass")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Unused (Debug)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Shoot")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Pass")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Unused (Debug)")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("PLAYERS34")
	// 1-8 p3 connector
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Shoot")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Pass")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Unused (Debug)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN3 ) // p3 coin slot
	// 1-8 p4 connector
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 Shoot")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 Pass")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 Unused (Debug)")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN4 ) // p4 coin slot

	// manuals confirms all OFF as default setting
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin 1 (3)" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin 2 (4)" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:7") // x2 means at least 2 players must start which
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )                         // is similar to Heated Barrel SW2: 3,4
	PORT_DIPSETTING(      0x0000, "x2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Time vs Computer, 1 Player" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1:30" )
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0100, "2:30" )
	PORT_DIPSETTING(      0x0000, "3:00" )
	// the duplicate settings of following three aren't actually documented in manual
	// tested 2p/4p via stopwatch, matches 2min and 3min of effective time.
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time Player vs Player, 2 Players" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0c00, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "2:00 (duplicate)" )
	PORT_DIPNAME( 0x3000, 0x3000, "Time Player vs Player, 3 Players" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, "2:30" )
	PORT_DIPSETTING(      0x3000, "3:00" )
	PORT_DIPSETTING(      0x1000, "3:30" )
	PORT_DIPSETTING(      0x0000, "2:30 (duplicate)" )
	PORT_DIPNAME( 0xc000, 0xc000, "Time Player vs Player, 4 Players" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0xc000, "3:30" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "3:00 (duplicate)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Cabinet Setting" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "2 Players" )
	PORT_DIPSETTING(      0x0008, "4 Players & 4 Coin Slots" )
	PORT_DIPSETTING(      0x0004, "4 Players (2x 2P Linked) & 1-4 Coin Slots" )
	PORT_DIPSETTING(      0x0000, "4 Players & 1 Coin Slot" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8") // toggling this has no effect in test mode, always shown on.
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 4) },
	{ STEP4(3, -1), STEP4(4*4+3, -1) },
	{ STEP8(0, 4*8) },
	8*8*4
};


void legionna_state::descramble_legionnaire_gfx(u8* src)
{
	int len = 0x10000;

	/*  rearrange gfx */
	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			buffer[i] = src[bitswap<24>(i,
			23,22,21,20,
			19,18,17,16,
			6,5,15,14,13,12,
			11,10,9,8,
			7,4,
			3,2,1,0)];
		}
		memcpy(src,&buffer[0],len);
	}
}


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 4) },
	{ STEP4(3, -1), STEP4(4*4+3, -1), STEP4(4*8*16+3, -1), STEP4(4*8*16+4*4+3, -1) },
	{ STEP16(0, 4*8) },
	16*16*4
};

static GFXDECODE_START( gfx_legionna_spr )
	GFXDECODE_ENTRY( "sprite", 0, tilelayout, 64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_legionna ) // Background and Midground has shared ROM
	GFXDECODE_ENTRY( "char",   0, charlayout, 48*16, 16 )
	GFXDECODE_ENTRY( "back",   0, tilelayout,     0, 32 )
	GFXDECODE_ENTRY( "fore",   0, tilelayout, 32*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_heatbrl ) // Midground has independent ROM
	GFXDECODE_ENTRY( "char",   0, charlayout, 48*16, 16 )
	GFXDECODE_ENTRY( "back",   0, tilelayout,  0*16, 16 )
	GFXDECODE_ENTRY( "fore",   0, tilelayout, 32*16, 16 )
	GFXDECODE_ENTRY( "mid",    0, tilelayout, 16*16, 16 )
GFXDECODE_END

/*****************************************************************************/

void legionna_state::legionna(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);  /* ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::legionna_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(36*8, 36*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(legionna_state::screen_update_legionna));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_legionna);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(12'000'000), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::pri_cb));

	MCFG_VIDEO_START_OVERRIDE(legionna_state,legionna)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void legionna_state::heatbrl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);  /* ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::heatbrl_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(36*8, 36*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(legionna_state::screen_update_heatbrl));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_heatbrl);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(12'000'000), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::pri_cb));

	MCFG_VIDEO_START_OVERRIDE(legionna_state,heatbrl)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void legionna_state::godzilla(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::godzilla_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold));

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &legionna_state::godzilla_sound_io_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(61);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
//  screen.set_size(42*8, 36*8);
//  screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_raw(14318180/2,455,0,320,258,0,224); // ~61 Hz, 15.734 kHz
	screen.set_screen_update(FUNC(legionna_state::screen_update_godzilla));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));
	m_crtc->layer_scroll_base_callback().set(FUNC(legionna_state::tile_scroll_base_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_legionna);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::pri_cb));
	m_spritegen->set_gfxbank_callback(FUNC(legionna_state::godzilla_tile_cb));

	MCFG_VIDEO_START_OVERRIDE(legionna_state,godzilla)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

void legionna_state::denjinmk(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::denjinmk_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold));

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(42*8, 36*8);
	screen.set_refresh_hz(56); // <= 56 FPS, Value from doc
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_visarea(0*8, 40*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(legionna_state::screen_update_godzilla));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_heatbrl);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::pri_cb));
	m_spritegen->set_gfxbank_callback(FUNC(legionna_state::godzilla_tile_cb));

	MCFG_VIDEO_START_OVERRIDE(legionna_state,denjinmk)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

void legionna_state::grainbow(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::grainbow_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold));

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 36*8);
	screen.set_visarea(2*8, 42*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(legionna_state::screen_update_grainbow));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_legionna);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::grainbow_pri_cb));
	m_spritegen->set_offset(16, 16);

	MCFG_VIDEO_START_OVERRIDE(legionna_state,grainbow)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

void legionna_state::cupsoc(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::cupsoc_map);
	m_maincpu->set_vblank_int("screen", FUNC(legionna_state::irq4_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 14318180/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &legionna_state::seibu_sound_map);
	m_audiocpu->set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(legionna_state::videowrite_cb_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(42*8, 36*8);
	// TODO: real PCB is a bit more complex,
	// it's really 320x256 in-game but with 8px border color on top/bottom.
	// Border color is also not pure black but a very dark grey,
	// can be seen when throw-in on bottom line occurs.
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(legionna_state::screen_update_grainbow));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(legionna_state::tilemap_enable_w));
	m_crtc->layer_scroll_callback().set(FUNC(legionna_state::tile_scroll_w));
	m_crtc->reg_1a_callback().set(FUNC(legionna_state::tile_vreg_1a_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_legionna);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xBGR_555, 128*16);

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_legionna_spr);
	m_spritegen->set_pri_callback(FUNC(legionna_state::pri_cb));

	MCFG_VIDEO_START_OVERRIDE(legionna_state,cupsoc)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline(m_audiocpu, 0);
	m_seibu_sound->coin_io_callback().set_ioport("COIN");
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void legionna_state::cupsocs(machine_config &config)
{
	cupsoc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &legionna_state::cupsocs_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

// all 3 Legionnaire sets differ only by the region byte at 0x1ef in rom 4 (Japan 0x00, US 0x01, World 0x02)
// unused program code above 0x28c00 has many bad bits, probably a defective copy of a previous build
ROM_START( legionna )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1.u025",  0x00000, 0x20000, CRC(9e2d3ec8) SHA1(8af9ca349389cbbd2b541aafa09de57f87f6fd72) )
	ROM_LOAD32_BYTE( "2.u024",  0x00001, 0x20000, CRC(35c8a28f) SHA1(31a1f2f9e04dfcab4b3357d6d27c24b434a8c14b) )
	ROM_LOAD32_BYTE( "3.u026",  0x00002, 0x20000, CRC(553fc7c0) SHA1(b12a2eea6b2c9bd76c0c74ddf2765d58510f586a) )
	ROM_LOAD32_BYTE( "4a.u023", 0x00003, 0x20000, CRC(2cc36c98) SHA1(484fc6eeeed89386ec69df0f92919b742cfdd89f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "6.u1110",     0x000000, 0x08000, CRC(fe7b8d06) SHA1(1e5b52ea4b4042940e2ee2db75c7c0f24973422a) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "user1", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD16_BYTE( "7.u077", 0x000000, 0x10000, CRC(88e26809) SHA1(40ee55d3b5329b6f657e0621d93c4caf6a035fdf) )
	ROM_LOAD16_BYTE( "8.u072", 0x000001, 0x10000, CRC(06e35407) SHA1(affeeb97b7f3cfa9b65a584ebe25c16a5b2c9a89) )

	ROM_REGION( 0x010000, "char", 0 )  /* FG Tiles */
	ROM_COPY( "user1", 0x010000, 0x000000, 0x010000 )

	ROM_REGION( 0x010000, "fore", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "back", 0 )    /* 3 sets of tiles ('MBK','LBK','BK3') */
	ROM_LOAD16_WORD_SWAP( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u0910",   0x000000, 0x000200, CRC(cc6da568) SHA1(cde01291f32def1ce291d7b558f64ce0758cf379) ) /* N82S147N type BPROM */
ROM_END

ROM_START( legionnaj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1.u025",  0x00000, 0x20000, CRC(9e2d3ec8) SHA1(8af9ca349389cbbd2b541aafa09de57f87f6fd72) )
	ROM_LOAD32_BYTE( "2.u024",  0x00001, 0x20000, CRC(35c8a28f) SHA1(31a1f2f9e04dfcab4b3357d6d27c24b434a8c14b) )
	ROM_LOAD32_BYTE( "3.u026",  0x00002, 0x20000, CRC(553fc7c0) SHA1(b12a2eea6b2c9bd76c0c74ddf2765d58510f586a) )
	ROM_LOAD32_BYTE( "4.u023",  0x00003, 0x20000, CRC(4c385dc7) SHA1(75ec869a5553228369faa8f8487d92ac5df7e563) ) // sldh

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "6.u1110",     0x000000, 0x08000, CRC(fe7b8d06) SHA1(1e5b52ea4b4042940e2ee2db75c7c0f24973422a) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "user1", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD16_BYTE( "7.u077", 0x000000, 0x10000, CRC(88e26809) SHA1(40ee55d3b5329b6f657e0621d93c4caf6a035fdf) )
	ROM_LOAD16_BYTE( "8.u072", 0x000001, 0x10000, CRC(06e35407) SHA1(affeeb97b7f3cfa9b65a584ebe25c16a5b2c9a89) )

	ROM_REGION( 0x010000, "char", 0 )  /* FG Tiles */
	ROM_COPY( "user1", 0x010000, 0x000000, 0x010000 )

	ROM_REGION( 0x010000, "fore", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "back", 0 )    /* 3 sets of tiles ('MBK','LBK','BK3') */
	ROM_LOAD16_WORD_SWAP( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u0910",   0x000000, 0x000200, CRC(cc6da568) SHA1(cde01291f32def1ce291d7b558f64ce0758cf379) ) /* N82S147N type BPROM */
ROM_END

ROM_START( legionnau )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1.u025", 0x00000, 0x20000, CRC(9e2d3ec8) SHA1(8af9ca349389cbbd2b541aafa09de57f87f6fd72) )
	ROM_LOAD32_BYTE( "2.u024", 0x00001, 0x20000, CRC(35c8a28f) SHA1(31a1f2f9e04dfcab4b3357d6d27c24b434a8c14b) )
	ROM_LOAD32_BYTE( "3.u026", 0x00002, 0x20000, CRC(553fc7c0) SHA1(b12a2eea6b2c9bd76c0c74ddf2765d58510f586a) )
	ROM_LOAD32_BYTE( "4.u023", 0x00003, 0x20000, CRC(91fd4648) SHA1(8ad6d0512996b88d3c0c7a96912eebaae2333424) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "6.u1110",     0x000000, 0x08000, CRC(fe7b8d06) SHA1(1e5b52ea4b4042940e2ee2db75c7c0f24973422a) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "user1", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD16_BYTE( "7.u077", 0x000000, 0x10000, CRC(88e26809) SHA1(40ee55d3b5329b6f657e0621d93c4caf6a035fdf) )
	ROM_LOAD16_BYTE( "8.u072", 0x000001, 0x10000, CRC(06e35407) SHA1(affeeb97b7f3cfa9b65a584ebe25c16a5b2c9a89) )

	ROM_REGION( 0x010000, "char", 0 )  /* FG Tiles */
	ROM_COPY( "user1", 0x010000, 0x000000, 0x010000 )

	ROM_REGION( 0x010000, "fore", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "back", 0 )    /* 3 sets of tiles ('MBK','LBK','BK3') */
	ROM_LOAD16_WORD_SWAP( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u0910",   0x000000, 0x000200, CRC(cc6da568) SHA1(cde01291f32def1ce291d7b558f64ce0758cf379) ) /* N82S147N type BPROM */
ROM_END

ROM_START( heatbrl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1e_ver3.9k",   0x00000, 0x20000, CRC(6b41fbac) SHA1(aa987386be40439450bc02f97e57dc833b32fa63) )
	ROM_LOAD32_BYTE( "2e_ver3.9m",   0x00001, 0x20000, CRC(dd21969b) SHA1(735e6984ac7b83c10bf4a90608fa3548db62cabc) )
	ROM_LOAD32_BYTE( "3e_ver3.9f",   0x00002, 0x20000, CRC(09544a91) SHA1(5c24fbf642dd4c40ee21664bdc7b837e8a15b8bb) )
	ROM_LOAD32_BYTE( "4e_ver3.9h",   0x00003, 0x20000, CRC(ebd34559) SHA1(9d565eb144b9239769a272990c1d1e22e72e3f0c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "barrel_7.u1110", 0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, "sprite", 0 )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

ROM_START( heatbrl2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1e_ver2.9k",   0x00000, 0x20000, CRC(b30bd632) SHA1(8684dd4787929886b0bce283301e492206ade9d9) )
	ROM_LOAD32_BYTE( "2e_ver2.9m",   0x00001, 0x20000, CRC(f3a23056) SHA1(d8840468535ac59fede60ea5a2928410d9c7a33a) )
	ROM_LOAD32_BYTE( "3e_ver2.9f",   0x00002, 0x20000, CRC(a2c41715) SHA1(a15b7a35ae0792ed00c47426d2e07c445acd8b8d) )
	ROM_LOAD32_BYTE( "4e_ver2.9h",   0x00003, 0x20000, CRC(a50f4f08) SHA1(f468e4a016a53803b8404bacdef5712311c6f0ac) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "barrel_7.u1110", 0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, "sprite", 0 )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

ROM_START( heatbrl3 ) // only the maincpu and audiocpu ROMs were provided for this set. This is the only known set with a different audiocpu ROM, though it's quite similar.
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "barrel_1.u025.9k",   0x00000, 0x20000, CRC(b360c9cd) SHA1(3722c7220f99c28ac47ef5abac027ac62f838caa) )
	ROM_LOAD32_BYTE( "barrel_2.u024.9m",   0x00001, 0x20000, CRC(06730aac) SHA1(451ebe6fdc9f983b05cfd20c5b6e89f2c1e7d17a) )
	ROM_LOAD32_BYTE( "barrel_3.u026.9f",   0x00002, 0x20000, CRC(63fff651) SHA1(4bf060e3338fce8d0eecdcb7418d353ca3616382) )
	ROM_LOAD32_BYTE( "barrel_4.u023.9h",   0x00003, 0x20000, CRC(7a119fd5) SHA1(ffccb7cec9b6f420edb658705a7434041854e77e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "u1110.a6", 0x00000, 0x08000, CRC(790bdba4) SHA1(9030ddcf06b632a53ec49cbb7c94d1a5195e0316) ) // no label
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077.5t", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072.5v", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, "sprite", 0 )   /* sprites */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

ROM_START( heatbrlo )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "barrel.1h",   0x00000, 0x20000, CRC(d5a85c36) SHA1(421a42863faa940057ed5637748f791152a15502) )
	ROM_LOAD32_BYTE( "barrel.2h",   0x00001, 0x20000, CRC(5104d463) SHA1(f65ee824508da431567661804f6235b61425b2dd) )
	ROM_LOAD32_BYTE( "barrel.3h",   0x00002, 0x20000, CRC(823373a0) SHA1(1bb7f811df4f85db8ca10e59fe22137a09470def) )
	ROM_LOAD32_BYTE( "barrel.4h",   0x00003, 0x20000, CRC(19a8606b) SHA1(6e950212c532e46bb6645c3c1f8205c2a4ea2c87) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "barrel_7.u1110", 0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

/* Sprite + tilemap gfx roms not dumped, for now we use ones from heatbrlu
Readme mentions as undumped:
barrel1,2,3,4.OBJ
barrel1,2,3,4.BG */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

ROM_START( heatbrlu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "1e_ver2.9k",   0x00000, 0x20000, CRC(b30bd632) SHA1(8684dd4787929886b0bce283301e492206ade9d9) )
	ROM_LOAD32_BYTE( "2u",           0x00001, 0x20000, CRC(289dd629) SHA1(fb379e067ffee4e54d55da638e45e22d6b2ef788) )
	ROM_LOAD32_BYTE( "3e_ver2.9f",   0x00002, 0x20000, CRC(a2c41715) SHA1(a15b7a35ae0792ed00c47426d2e07c445acd8b8d) )
	ROM_LOAD32_BYTE( "4e_ver2.9h",   0x00003, 0x20000, CRC(a50f4f08) SHA1(f468e4a016a53803b8404bacdef5712311c6f0ac) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "barrel_7.u1110", 0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

ROM_START( heatbrle )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD32_BYTE( "2.u025",   0x00000, 0x20000, CRC(b34dc60c) SHA1(f9d1438469bf0d36d53d3f148bdf7f04dee5eae0) ) /* ROM type is AM27C020 */
	ROM_IGNORE( 0x20000 )   // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "1.u024",   0x00001, 0x20000, CRC(16a3754f) SHA1(3e070f2d004fc17d8ae9171955dc48ec5d14cf8a) ) /* ROM type is AM27C020 */
	ROM_IGNORE( 0x20000 )   // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "4.u026",   0x00002, 0x20000, CRC(fae85c88) SHA1(1b0316e66d4e0c5b3aa4045d6bfcc8a5464dc74e) ) /* ROM type is AM27C020 */
	ROM_IGNORE( 0x20000 )   // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "3.u023",   0x00003, 0x20000, CRC(3b035081) SHA1(b7ecbacd85102eda21dd162427a0e57cc6d24661) ) /* ROM type is AM27C020 */
	ROM_IGNORE( 0x20000 )   // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "barrel_7.u1110", 0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(               0x10000, 0x08000 )  /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000,    0x18000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )   /* chars */
	ROM_LOAD16_BYTE( "barrel_6.u077", 0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )
	ROM_LOAD16_BYTE( "barrel_5.u072", 0x000001, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD16_WORD_SWAP( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x080000, "mid", 0 )    /* LBK tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "copx", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END


/*

Godzilla
Banpresto 1993

This game runs on Seibu hardware, similar to Legionnaire.

PCB Layout
|----------------------------------------------------|
|   YM2151  M6295  PCM.922   8    Z80          PAL   |
|     YM3014                                   PAL   |
|           YM3931  SEI0220  14.31818MHz  6116       |
|         (SEI0100)                                  |
|                                OBJ1.648   OBJ3.743 |
|                      SEI0211                       |
|J                               OBJ2.756   OBJ4.757 |
|A                                                   |
|M     DSW1-8                 2.025  4.026  62256    |
|M     DSW2-8                               62256    |
|A           S68E08.844 PAL   1.024  3.023  62256    |
|      6264                   PAL  PAL      62256    |
|      6264                                          |
| SEI0200                   COPX-D2.313   SEI0300    |
| TC110G21AF                              TC25SC900AF|
|                                                    |
|BK1.618             11.620  20MHz   PAL   PAL       |
|          BK3.619   10.615          PAL   68000     |
|----------------------------------------------------|

Notes:
      Z80 clock    : 3.579545MHz
      68000 clock  : 10.000MHz
      YM2151 clock : 3.579545MHz
      M6295 clock  : 1.000MHz, sample rate = clk /132
      VSync        : 61Hz
      HSync        : 15.74kHz
      S68E08.844   : 82S147 bipolar PROM
      BK & OBJ     : 8M MASK (read as 238000)
      Main PRG 1-4 : 27C010 EPROM
      ROMs 8,10,11 : 27C512 EPROM
      COPX-D2.313  : 4M MASK (read as 234200)
      PCM          : 4M MASK (read as 27C040)

      Custom SEIBU chips:
                         SEI0211 and SEI0220: connected to OBJ ROMs
                         SEI300             : connected to PRG ROMs and 68000
                         SEI0200            : connected to BG ROMs

*/

ROM_START( godzilla )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "2.025",        0x000000, 0x020000, CRC(be9c6e5a) SHA1(9a7e49ac9cdbcc02b13b3448544cee5fe398ec16) )
	ROM_LOAD32_BYTE( "1.024",        0x000001, 0x020000, CRC(0d6b663d) SHA1(01e02999cffd2642f7a37e492fe7f83770cddd67) )
	ROM_LOAD32_BYTE( "4.026",        0x000002, 0x020000, CRC(bb8c0132) SHA1(fa8b049f590be710b3cf82f27deade63656db730) )
	ROM_LOAD32_BYTE( "3.023",        0x000003, 0x020000, CRC(bb16e5d0) SHA1(31d8941e6e297b1f410944f0063a4c9219d23f23) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "8.016",        0x000000, 0x08000, CRC(4ab76e43) SHA1(40c34fade03161c4b50f9f6a2ae61078b8d8ea6d) )
	ROM_CONTINUE(             0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "11.620",       0x000000, 0x010000, CRC(58e0e41f) SHA1(563c633eb3d4df41e467c93957c74b540a0ae43c) )
	ROM_LOAD16_BYTE( "10.615",       0x000001, 0x010000, CRC(9c22bc13) SHA1(a94d9ed63ee1f5e358ebcaf517e6a1c986fa5d96) )

	ROM_REGION( 0x800000, "sprite", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "obj1.748",     0x000000, 0x200000, CRC(0dfaf26d) SHA1(2af3ea06369c40ae89c2f8362c273f4801db8e68) )
	ROM_LOAD16_WORD_SWAP( "obj2.756",     0x200000, 0x200000, CRC(32b1516a) SHA1(4adcf4b957f6b9baf1a5b8807b381db664de632d) )
	ROM_LOAD16_WORD_SWAP( "obj3.743",     0x400000, 0x100000, CRC(5af0114e) SHA1(9362de9ade6db67ab0e3a2dfea580e688bbf7729) )
	ROM_LOAD16_WORD_SWAP( "obj4.757",     0x500000, 0x100000, CRC(7448b054) SHA1(5c08319329eb8c90b63e5393c0011bc39911ebbb) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "bg1.618",      0x000000, 0x100000, CRC(78fbbb84) SHA1(b1f5d4041bb88c5b2a561949239b11c3fd7c5fbc) )

	ROM_REGION( 0x100000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "bg2.619",      0x000000, 0x100000, CRC(8ac192a5) SHA1(54b557e81a704c70a651e6b8da70207a2a70530f) ) // 1xxxxxxxxxxxxxxxxxxx = 0xff filled

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "pcm.922",      0x000000, 0x080000, CRC(59cbef10) SHA1(6b89b7286f80f9c903dfb81dc93a03c38dff707c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d2.313",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

Denjin Makai newer set
PCB labeled BP942KS
All "denjin" ROM labels from this set are actually written in kanji chars.

*/

ROM_START( denjinmk )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "denjin_1.u025", 0x000000, 0x040000, CRC(b23a6e6f) SHA1(73ab330dfc0d2799984874f02da51deb8323b9e2) )
	ROM_LOAD32_BYTE( "denjin_2.u024", 0x000001, 0x040000, CRC(4fde59e7) SHA1(1618db14a18acc2acabdd93b8e2563d7221e643d) )
	ROM_LOAD32_BYTE( "denjin_3.u026", 0x000002, 0x040000, CRC(4f10292b) SHA1(c61c88cacc433bb9af6a4225ce5959dd0fefd084) )
	ROM_LOAD32_BYTE( "denjin_4.u023", 0x000003, 0x040000, CRC(209f1f6b) SHA1(3f6709dc79fae47ef4181d405d3aa94c2df5963a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "denjin_7.u1016",  0x000000, 0x08000, CRC(970f36dd) SHA1(010a9edeaedb9e258cd02b3e9294264d00ec7c45) )
	ROM_CONTINUE(         0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "denjin_6.u0620", 0x000000, 0x010000, CRC(e1f759b1) SHA1(ddc60e78e7791a59c59403dd4089b3f6e1ecf8cb) )
	ROM_LOAD16_BYTE( "denjin_5.u0615", 0x000001, 0x010000, CRC(cc36af0d) SHA1(69c2ae38f03be79be4d138fcc73a6a86407eb285) )

	ROM_REGION( 0x500000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj-0-3.748", 0x000000, 0x200000, CRC(67c26a67) SHA1(20543ca9dcf3fed0884968b5249b34b59a14b791) ) /* banks 0,1,2,3 */
	ROM_LOAD16_WORD_SWAP( "obj-4-5.756", 0x200000, 0x100000, CRC(01f8d4e6) SHA1(25b69da693be8c3404f750b419c330a7a56e88ec) ) /* 4,5 */
	ROM_LOAD16_WORD_SWAP( "obj-6-7.743", 0x300000, 0x100000, CRC(e5805757) SHA1(9d392c27eef7c1fcda560dac17ba9d7ae2287ac8) ) /* 6,7 */
	ROM_LOAD16_WORD_SWAP( "obj-8-9.757", 0x400000, 0x100000, CRC(c8f7e1c9) SHA1(a746d187b50a0ecdd5a7f687a2601e5dc8bfe272) ) /* 8,9 */

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "bg-1-ab.618", 0x000000, 0x100000, CRC(eaad151a) SHA1(bdd1d83ee8497efe20f21baf873e786446372bcb) )

	ROM_REGION( 0x100000, "mid", 0 )    /* BK2 used (or LBK; just identification string differs?) */
	ROM_LOAD16_WORD_SWAP( "bg-2-ab.617", 0x000000, 0x100000, CRC(40938f74) SHA1(d68b0f8245a8b390ad5d4e6ebc7514a939b8ac51) )

	ROM_REGION( 0x100000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "bg-3-ab.619", 0x000000, 0x100000, CRC(de7366ee) SHA1(0c3969d15f3cd963e579d4164b6e0a6b4012c9c6) )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "denjin_8.u0922",     0x000000, 0x040000, CRC(a11adb8f) SHA1(50e1158767b506d962bff861c2a6609246d764c4) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "s68e08.844",  0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d2.313", 0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

Denjin Makai
Banpresto, 1994

This game runs on early 90's Seibu hardware.
(i.e. Raiden II, Godzilla, Seibu Cup Soccer, Legionnaire, Heated Barrel etc).
The PCB looks to have been converted from some other game (a lot of flux traces are
left on the PCB). The game might be a simple ROM swap for some other Seibu-based game.


PCB Layout
----------

|-----------------------------------------------------|
|LA4460   YM2151   M6295  ROM6  ROM5  Z80       PAL6  |
|                                     6116      PAL9  |
|        SEI0100BU   SEI0220BP  14.31818MHz           |
|                                                     |
|        6116                                         |
|                              OBJ-0-3     OBJ-6-7    |
|J       6116      SEI0211                            |
|A                             OBJ-4-5     OBJ-8-9    |
|M                                                    |
|M                        ROM1     ROM3      62256    |
|M                                           62256    |
|                         ROM2     ROM4      62256    |
|     DSW1   S68E08  PAL7                    62256    |
|     DSW2                PAL6  PAL3                  |
|SEI0200                                              |
|     6264                                            |
|     6264                   COPX-D2                  |
|                                             SEI1000 |
|BG-1-AB            ROM7         PAL2  PAL1           |
|                                                     |
|BG-2-AB   BG-3-AB  ROM8  20MHz  PAL5  68000          |
|-----------------------------------------------------|
Notes:
      68000 clock  : 10.000MHz (20 / 2)
      Z80 clock    : 3.579545MHz (14.31818 / 4)
      YM2151 clock : 3.579545MHz (14.31818 / 4)
      M6295 clock  : 1.000MHz, sample rate = M6295 clock / 132
      VSync        : 56Hz

      62256    : 32K x8 SRAM
      6264     : 8K  x8 SRAM
      6116     : 2K  x8 SRAM
      SEI0200  : Custom Seibu QFP100 also stamped TC110G21AF
      SEI1000  : Custom Seibu QFP184 also stamped SB01-001
      SEI0211  : Custom Seibu QFP128
      SEI0100BU: Custom Seibu SDIP64 also stamped YM3931
      SEI0220BP: Custom Seibu QFP80

      ROMs 1, 2, 3, 4   Main program   27C020 EPROM
      ROM  6            OKI samples    27C020 EPROM
      ROM  5            Sound program  27C512 EPROM
      ROMs 7 and 8      Graphics       27C512 EPROM
      ROMs BG*          Graphics       8M Mask ROM
      ROMs OBJ*         Graphics       8M Mask ROM
      COPX-D2           ?              4M Mask ROM

      PAL1 : type AMI 18CV8PC,    labelled 'S68E01'
      PAL2 : type MMI PAL16L8ACN, labelled 'S68E02'
      PAL3 : type MMI PAL16L8ACN, labelled 'S68E03'
      PAL4 : type AMI 18CV8PC,    labelled 'S68E04'
      PAL5 : type AMI 18CV8PC,    labelled 'S68E05T'
      PAL6 : type AMI 18CV8PC,    labelled 'S68E06T'
      PAL7 : type AMI 18CV8PC,    labelled 'S68E07'
      PAL9 : type MMI PAL16L8ACN, labelled 'S68E09'

      S68E08 : PROM type 82S147, labelled 'S68E08'

*/

ROM_START( denjinmka )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "rom1.025", 0x000000, 0x040000, CRC(44a648e8) SHA1(a3c1721e89ac6b9fc16f80682b2f701cb24b5d76) )
	ROM_LOAD32_BYTE( "rom2.024", 0x000001, 0x040000, CRC(e5ee8fe0) SHA1(2ebff4fdbe82062fb526598e10f11358b0b5c02f) )
	ROM_LOAD32_BYTE( "rom3.026", 0x000002, 0x040000, CRC(781b942e) SHA1(f1f4ddc332de3dc29b716a1b82c2ecc2045efb3a) )
	ROM_LOAD32_BYTE( "rom4.023", 0x000003, 0x040000, CRC(502a588b) SHA1(9055b631240fe52d33b572e34275d31a9f3d290f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "rom5.016", 0x000000, 0x08000, CRC(7fe7e352) SHA1(1ceae22186751ca91dfffab7bd11f275e693451f) )
	ROM_CONTINUE(         0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "rom7.620",       0x000000, 0x010000, CRC(e1f759b1) SHA1(ddc60e78e7791a59c59403dd4089b3f6e1ecf8cb) )
	ROM_LOAD16_BYTE( "rom8.615",       0x000001, 0x010000, CRC(cc36af0d) SHA1(69c2ae38f03be79be4d138fcc73a6a86407eb285) )

	ROM_REGION( 0x500000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj-0-3.748", 0x000000, 0x200000, CRC(67c26a67) SHA1(20543ca9dcf3fed0884968b5249b34b59a14b791) ) /* banks 0,1,2,3 */
	ROM_LOAD16_WORD_SWAP( "obj-4-5.756", 0x200000, 0x100000, CRC(01f8d4e6) SHA1(25b69da693be8c3404f750b419c330a7a56e88ec) ) /* 4,5 */
	ROM_LOAD16_WORD_SWAP( "obj-6-7.743", 0x300000, 0x100000, CRC(e5805757) SHA1(9d392c27eef7c1fcda560dac17ba9d7ae2287ac8) ) /* 6,7 */
	ROM_LOAD16_WORD_SWAP( "obj-8-9.757", 0x400000, 0x100000, CRC(c8f7e1c9) SHA1(a746d187b50a0ecdd5a7f687a2601e5dc8bfe272) ) /* 8,9 */

	ROM_REGION( 0x100000, "back", 0 )   /* MBK tiles */
	ROM_LOAD16_WORD_SWAP( "bg-1-ab.618",      0x000000, 0x100000, CRC(eaad151a) SHA1(bdd1d83ee8497efe20f21baf873e786446372bcb) )

	ROM_REGION( 0x100000, "mid", 0 )    /* BK2 used (or LBK; just identification string differs?) */
	ROM_LOAD16_WORD_SWAP( "bg-2-ab.617",      0x000000, 0x100000, CRC(40938f74) SHA1(d68b0f8245a8b390ad5d4e6ebc7514a939b8ac51) )

	ROM_REGION( 0x100000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "bg-3-ab.619",      0x000000, 0x100000, CRC(de7366ee) SHA1(0c3969d15f3cd963e579d4164b6e0a6b4012c9c6) )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "rom6.922",      0x000000, 0x040000, CRC(09e13213) SHA1(9500e057104c6b83da0467938e46d9efa2f49f4c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d2.313",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

SD Gundam Sangokushi Rainbow Tairiku Senki
(c)1993 Banpresto
TYPE-R
Board made by Seibu?

CPU  : MC68000P10
Sound: Z80A YM2151 M6295 Y3014B
OSC  : 20.0000MHz (X11), 14.31818MHz (X71)

ROMs:
rb-p1.25 - Main programs (27c020)
rb-p2.24 |
rb-p3.26 |
rb-p4.23 /

rb-s.016 - Sound program (27c512)
rb-ad.922 - Sound data (27c1001)

rb-bg-01.618 - Background (TC538200AP)
rb-bg-2.619  |
rb-f1.620    | (27c512)
rb-f2.615    /

rb-spr01.748 - Sprites (TC538200AP)
rb-spr23.756 /

copx-d2.313 - ? (2M-16bit)

s68e08.844 - (N82S147N)

PALs:
s68e01.122
s68e02.310 (16L8ACN)
s68e03.322 (16L8ACN)
s68e04.551
s68e05.552
s68e06r.741 (18CV8)
s68e07.842
s68e09.015 (16L8ACN)


Custom chips:
SEI0100BU YM3931 9149 EALA
SEI0220BP JAPAN S 9208 U ("S" for "Sharp")
SEI0211 9215 ABBB
SEI0200 TC110G21AF 0076 9324EAI JAPAN
SEI300 TC25SC900AF 001 9211EAI JAPAN

*/

ROM_START( grainbow )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "rb-p1.25",     0x000000, 0x040000, CRC(0995c511) SHA1(97fb2bd7d26720552ace25e655fce09ad9a7afd7) )
	ROM_LOAD32_BYTE( "rb-p2.24",     0x000001, 0x040000, CRC(c9eb756f) SHA1(88d784a71bfab4f321d3320aed1b6b2648529979) )
	ROM_LOAD32_BYTE( "rb-p3.26",     0x000002, 0x040000, CRC(fe2f08a8) SHA1(bb95e5c113a0343b6da43c5dca1292601dec00eb) )
	ROM_LOAD32_BYTE( "rb-p4.23",     0x000003, 0x040000, CRC(f558962a) SHA1(fcfb6f2cba59effd14c76602b0f87f564235d8ef) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rb-s.016",     0x000000, 0x08000, CRC(8439bf5b) SHA1(089009b91768d64edef6639e7694723d2d1c46ff) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "rb-f1.620",    0x000000, 0x010000, CRC(792c403d) SHA1(3c606af696fe8f3d6edefdab3940bd5eb341bca9) )
	ROM_LOAD16_BYTE( "rb-f2.615",    0x000001, 0x010000, CRC(a30e0903) SHA1(b9e7646da1ccab6dadaca6beda08125b34946653) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-spr01.748", 0x000000, 0x100000, CRC(11a3479d) SHA1(4d2d06d62da02c6e9884735de8c319f37ca1715c) )
	ROM_LOAD16_WORD_SWAP( "rb-spr23.756", 0x100000, 0x100000, CRC(fd08a761) SHA1(3297a2bfaabef17ed9320e24e9a4ffa2f3eb3a44) )

	ROM_REGION( 0x100000, "back", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-bg-01.618", 0x000000, 0x100000, CRC(6a4ca7e7) SHA1(13612d29f8f04cf62b4357b69b81240dd1eceae4) )

	ROM_REGION( 0x100000, "fore", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-bg-2.619",  0x000000, 0x100000, CRC(a9b5c85e) SHA1(0ae044e05730e8080d94f1f6758f8dd051b03c41) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "rb-ad.922",    0x000000, 0x020000, CRC(a364cb42) SHA1(c527b39a1627ecee20a2c4df4cf2b5f2ba729081) )

	ROM_REGION( 0x040000, "user1", 0 )
	ROM_LOAD( "copx-d2.313",  0x0000, 0x040000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END


ROM_START( grainbowk )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "rom1.u025",   0x000000, 0x040000, CRC(686c25ea) SHA1(e4f68b0455eefacf783c9e56f542085d0027009c) )
	ROM_LOAD32_BYTE( "rom2.u024",   0x000001, 0x040000, CRC(2400662e) SHA1(251fc779c2f3f56137f07f4865483e919a101845) )
	ROM_LOAD32_BYTE( "rom3.u026",   0x000002, 0x040000, CRC(22857489) SHA1(b23d983057925b86f2a8a7ca80dbfc7464a0075f) )
	ROM_LOAD32_BYTE( "rom4.u023",   0x000003, 0x040000, CRC(ad8de15b) SHA1(0af56fc17c9b2c5e13fea67e08792577453d678a) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rb-s.016", 0x000000, 0x08000, CRC(8439bf5b) SHA1(089009b91768d64edef6639e7694723d2d1c46ff) )
	ROM_CONTINUE(         0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "rb-f1.620",   0x000000, 0x010000, CRC(792c403d) SHA1(3c606af696fe8f3d6edefdab3940bd5eb341bca9) )
	ROM_LOAD16_BYTE( "rb-f2.615",   0x000001, 0x010000, CRC(a30e0903) SHA1(b9e7646da1ccab6dadaca6beda08125b34946653) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-spr01.748", 0x000000, 0x100000, CRC(11a3479d) SHA1(4d2d06d62da02c6e9884735de8c319f37ca1715c) )
	ROM_LOAD16_WORD_SWAP( "rb-spr23.756", 0x100000, 0x100000, CRC(fd08a761) SHA1(3297a2bfaabef17ed9320e24e9a4ffa2f3eb3a44) )

	ROM_REGION( 0x100000, "back", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-bg-01.618", 0x000000, 0x100000, CRC(6a4ca7e7) SHA1(13612d29f8f04cf62b4357b69b81240dd1eceae4) )

	ROM_REGION( 0x100000, "fore", 0 )
	ROM_LOAD16_WORD_SWAP( "rb-bg-2.619",  0x000000, 0x100000, CRC(a9b5c85e) SHA1(0ae044e05730e8080d94f1f6758f8dd051b03c41) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "rb-ad.922",    0x000000, 0x020000, CRC(a364cb42) SHA1(c527b39a1627ecee20a2c4df4cf2b5f2ba729081) )

	ROM_REGION( 0x040000, "user1", 0 )
	ROM_LOAD( "copx-d2.313",  0x0000, 0x040000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END

ROM_START( cupsoc )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "scc_01.bin", 0x000000, 0x040000, CRC(c122203c) SHA1(93c0ae90c0ed3889b9159774ba89536108c9b259) )
	ROM_LOAD32_BYTE( "scc_02.bin", 0x000001, 0x040000, CRC(105511b4) SHA1(f2ebe95a10f5928f57d4f532e2d2432f13b774b2) )
	ROM_LOAD32_BYTE( "scc_03.bin", 0x000002, 0x040000, CRC(2d23d78f) SHA1(c479ded8782f2d23e123b7d00ec57c18a8f80578) )
	ROM_LOAD32_BYTE( "scc_04.bin", 0x000003, 0x040000, CRC(e8877461) SHA1(3be44459699fd455b0daaac10e8a37d1b7985607) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "scc_06.bin", 0x000000, 0x010000, CRC(f1a18ec6) SHA1(43f8ec3fc541b8dc2a17533329dd3448afadcb3b) )
	ROM_LOAD16_BYTE( "scc_05.bin", 0x000001, 0x010000, CRC(c0358503) SHA1(e87991c6a6f3e060a1b03b4899fa891510fca15f) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END

ROM_START( cupsoca )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "soca_1.bin", 0x000000, 0x040000, CRC(d5f76bd6) SHA1(5e7c3843f6f497a24b8236a9307d347fb24dd0d5) )
	ROM_LOAD32_BYTE( "soca_2.bin", 0x000001, 0x040000, CRC(34966aa1) SHA1(f07bca19bd92a60c04aa9b23e5d2d2eac073d2e4) )
	ROM_LOAD32_BYTE( "soca_3.bin", 0x000002, 0x040000, CRC(2b7934ec) SHA1(b78e7079e03b23853397a3848c93f60702ac1c33) )
	ROM_LOAD32_BYTE( "soca_4.bin", 0x000003, 0x040000, CRC(f4aa1d90) SHA1(eb94efc8cc623434b4c9a22e63d262e80ca84d83) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "soca_6.bin", 0x000000, 0x010000, CRC(a9e15910) SHA1(305541b16a87d0e38871240fa2e111bb9332e93c) )
	ROM_LOAD16_BYTE( "soca_5.bin", 0x000001, 0x010000, CRC(73a3e024) SHA1(aeb359dd2dc9eb96330f494c44123bab3f5986a4) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END

ROM_START( cupsocb )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "1-10n.bin", 0x000000, 0x040000, CRC(d4f37bf2) SHA1(af06364a602bd0ac2b9506de792bef003281e9d4) )
	ROM_LOAD32_BYTE( "2-10q.bin", 0x000001, 0x040000, CRC(f06e8743) SHA1(e2f3c9b44b2830c9780df43ce10634e4e2fcb96d) )
	ROM_LOAD32_BYTE( "3-10f.bin", 0x000002, 0x040000, CRC(226f65f9) SHA1(106a2f807aaf0f2e1fbcb1ffec6ccf4d2d7addd8) )
	ROM_LOAD32_BYTE( "4-10k.bin", 0x000003, 0x040000, CRC(8ff16a9e) SHA1(c29986cec74e183d18eaaf69ba6ca20b75590298) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "soca_6.bin", 0x000000, 0x010000, CRC(a9e15910) SHA1(305541b16a87d0e38871240fa2e111bb9332e93c) )
	ROM_LOAD16_BYTE( "soca_5.bin", 0x000001, 0x010000, CRC(73a3e024) SHA1(aeb359dd2dc9eb96330f494c44123bab3f5986a4) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END

ROM_START( cupsocs )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "1_10n.bin", 0x000000, 0x040000, CRC(b67835c5) SHA1(4fa562630d1f9cfb6f5bfff3295ebbdd227e4da5) )
	ROM_LOAD32_BYTE( "2_10q.bin", 0x000001, 0x040000, CRC(de65509c) SHA1(3362258b6d86fc63afa205712416a4aac0cf10e4) )
	ROM_LOAD32_BYTE( "3_10f.bin", 0x000002, 0x040000, CRC(c0333f0c) SHA1(ed02897724de4cf981aa8c6ce98551b9e79efff3) )
	ROM_LOAD32_BYTE( "4_10k.bin", 0x000003, 0x040000, CRC(288f11d4) SHA1(424cb3d1428f7195decce2ba6eebc1e24d9bb207) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "6_7x.bin", 0x000000, 0x010000, CRC(7981366e) SHA1(b859bf23c5ae466f4020958a06935192fa68ee8d) )
	ROM_LOAD16_BYTE( "5_7y.bin", 0x000001, 0x010000, CRC(26cbfaf0) SHA1(1ba7bc1cecb4bd06ba5c2d3eaa9c9e38e2106cd2) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "8_7a.bin", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END



ROM_START( cupsocs2 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "seibu1.10n", 0x000000, 0x040000, CRC(e91fdc95) SHA1(71c56fffabca79e73dfc61aad17bc58e09a28680) )
	ROM_LOAD32_BYTE( "seibu2.10q", 0x000001, 0x040000, CRC(7816df3c) SHA1(d5cfbf493cc00c47406b314c08e9cbf159a7f98c) )
	ROM_LOAD32_BYTE( "seibu3.10f", 0x000002, 0x040000, CRC(3be8a330) SHA1(f821080acd29c5801abc36da3341aabaea82ceb0) )
	ROM_LOAD32_BYTE( "seibu4.10k", 0x000003, 0x040000, CRC(f30167ea) SHA1(5431296e3245631c90362373027c54166f8fba16) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "seibu6.7x", 0x000000, 0x010000, CRC(21c1e1b8) SHA1(30928c8ef98bf32ba0bf795ddadba1c95fcffe9d) )
	ROM_LOAD16_BYTE( "seibu5.7y", 0x000001, 0x010000, CRC(955d9fd7) SHA1(782451e8e85f7ba285d6cacd9d3fdcf48bde60bc) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END

ROM_START( olysoc92 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "u025.1", 0x000000, 0x040000, CRC(a94e7780) SHA1(abbe328be425b4529e6b75ffa723c6771e4b6fcf) )
	ROM_LOAD32_BYTE( "u024.2", 0x000001, 0x040000, CRC(cb5f0748) SHA1(e11bf11a3766ab33c60a143867496887c6238b11) )
	ROM_LOAD32_BYTE( "u026.3", 0x000002, 0x040000, CRC(f71cc626) SHA1(7f66031509063d5fac33a3b5873b616c7ad0c25b) )
	ROM_LOAD32_BYTE( "u023.4", 0x000003, 0x040000, CRC(2ba10e6c) SHA1(d682d97426a749cfdbaf728edb219dbf84e9eef8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "seibu6.7x", 0x000000, 0x010000, CRC(21c1e1b8) SHA1(30928c8ef98bf32ba0bf795ddadba1c95fcffe9d) )
	ROM_LOAD16_BYTE( "seibu5.7y", 0x000001, 0x010000, CRC(955d9fd7) SHA1(782451e8e85f7ba285d6cacd9d3fdcf48bde60bc) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END

ROM_START( olysoc92a )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "1.u025", 0x000000, 0x040000, CRC(5191e895) SHA1(468db88eddc054a72cd739491288d03536b6b1d0) ) /* Comes from an early board with a S/N of 402 */
	ROM_LOAD32_BYTE( "2.u024", 0x000001, 0x040000, CRC(6c566f43) SHA1(7f6db2b141f09412b1c6ac41afa6b2085cfdcc11) )
	ROM_LOAD32_BYTE( "3.u026", 0x000002, 0x040000, CRC(e75bc773) SHA1(a371f0dab100f8c3a9192eabe6db4b06e070b858) )
	ROM_LOAD32_BYTE( "4.u023", 0x000003, 0x040000, CRC(6c2b037e) SHA1(280f3e0af3109be5b0d55f147a8a8cfae531961b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "6_7x.bin", 0x000000, 0x010000, CRC(7981366e) SHA1(b859bf23c5ae466f4020958a06935192fa68ee8d) )
	ROM_LOAD16_BYTE( "5_7y.bin", 0x000001, 0x010000, CRC(26cbfaf0) SHA1(1ba7bc1cecb4bd06ba5c2d3eaa9c9e38e2106cd2) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END


ROM_START( olysoc92b )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "1", 0x000000, 0x040000, CRC(d4f37bf2) SHA1(af06364a602bd0ac2b9506de792bef003281e9d4) ) // == cupsocb    Seibu Cup Soccer (set 3)
	ROM_LOAD32_BYTE( "2", 0x000001, 0x040000, CRC(6967d6f9) SHA1(292f59cf501064cdfb20acd8295fee7e93c9bc95) ) // same as Seibu Cup Soccer (set 3) except last byte is 02 instead of 01
	ROM_LOAD32_BYTE( "3", 0x000002, 0x040000, CRC(226f65f9) SHA1(106a2f807aaf0f2e1fbcb1ffec6ccf4d2d7addd8) ) // == cupsocb    Seibu Cup Soccer (set 3)
	ROM_LOAD32_BYTE( "4", 0x000003, 0x040000, CRC(8ff16a9e) SHA1(c29986cec74e183d18eaaf69ba6ca20b75590298) ) // sldh == cupsocb    Seibu Cup Soccer (set 3)

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "7",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "6", 0x000000, 0x010000, CRC(7edb1700) SHA1(db30c01fc0e5b9f3c2d6139f89ec88936bc75b38) )
	ROM_LOAD16_BYTE( "5", 0x000001, 0x010000, CRC(ec21c8dc) SHA1(93a9bf13d7c53e76762307c028f427ff2888629c) )  // sldh

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD16_WORD_SWAP( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "back", 0 )   /* 2 sets of tiles ('MBK','LBK') */
	ROM_LOAD16_WORD_SWAP( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x080000, "fore", 0 )   /* BK3 tiles */
	ROM_LOAD16_WORD_SWAP( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "8", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "copx", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "soc07.u0910", 0x000, 0x200, NO_DUMP ) // N82S147AN
ROM_END




/* Does the COP or something else on the PCB provide a rom overlay for the last part of ROM?

   In Seibu Cup Soccer Selection the only way I've found to not display debug text is by changing this
   area, and for Olympic Soccer '92 it appears to be the only way to get the Olympic Soccer '92 titles
   to be used instead of the regular Seibu Cup Soccer one (and AFAIK both dumps are confirmed to show
   that on hardware, as are all early versions with the advertising boards of dubious legality)

   You can also enable other debug menus by patching this area of ROM, but some initial tests appear to
   show those menus are still inaccessible with a patched rom on real hardware, again indicating there
   could be a rom overlay causing the patches to be ignored.

*/


// if this is 1 then P1 Button 3 during gameplay enters the 'Game Master' debug menu, with extensive
// debugging features.
#define CUPSOC_DEBUG_MODE 0

void legionna_state::init_cupsoc_debug()
{
#if CUPSOC_DEBUG_MODE
	u16 *ROM = (u16 *)memregion("maincpu")->base();
	ROM[0xffffa/2]  = 0x0000;
	ROM[0xffff6/2] ^= 0x00ff;
#endif
}

void legionna_state::init_olysoc92()
{
	u16 *ROM = (u16 *)memregion("maincpu")->base();
	ROM[0xffffe/2] ^= 0x0003; // show Olympic Soccer '92 title

	init_cupsoc_debug();
}

void legionna_state::init_cupsocs()
{
	u16 *ROM = (u16 *)memregion("maincpu")->base();
	ROM[0xffffa/2] = 0x00ff; // disable debug text (this is already 0x00ff in the bootleg sets for the same reason)

	init_cupsoc_debug();
}

void legionna_state::init_cupsoc()
{
	init_cupsoc_debug();
}



void legionna_state::init_legiongfx()
{
	descramble_legionnaire_gfx(memregion("fore")->base() );
}

void legionna_state::init_godzilla()
{
	u16 *ROM = (u16 *)memregion("maincpu")->base();
	// TODO: some game elements don't collide properly, @see seibucop.cpp
	ROM[(0xbe0e + 0x0a)/2] = 0xb000;
	ROM[(0xbe0e + 0x1a)/2] = 0xb800;
	ROM[(0xbb0a + 0x0a)/2] = 0xb000;
	ROM[(0xbb0a + 0x1a)/2] = 0xb800;
	// patch ROM checksum
	ROM[0x3fffe/2] = 0x61ba;
}

GAME( 1992, legionna,  0,        legionna, legionna, legionna_state, init_legiongfx, ROT0, "TAD Corporation",                  "Legionnaire (World)", 0 )
GAME( 1992, legionnau, legionna, legionna, legionna, legionna_state, init_legiongfx, ROT0, "TAD Corporation (Fabtek license)", "Legionnaire (US)", 0 )
GAME( 1992, legionnaj, legionna, legionna, legionna, legionna_state, init_legiongfx, ROT0, "TAD Corporation",                  "Legionnaire (Japan)", 0 )

GAME( 1992, heatbrl,   0,        heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation", "Heated Barrel (World version 3)", 0 )
GAME( 1992, heatbrl2,  heatbrl,  heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation", "Heated Barrel (World version 2)", 0 )
GAME( 1992, heatbrl3,  heatbrl,  heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation", "Heated Barrel (World version ?)", 0 )
GAME( 1992, heatbrlo,  heatbrl,  heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation", "Heated Barrel (World old version)", 0 )
GAME( 1992, heatbrlu,  heatbrl,  heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation", "Heated Barrel (US)", 0 )
GAME( 1992, heatbrle,  heatbrl,  heatbrl,  heatbrl,  legionna_state, empty_init,     ROT0, "TAD Corporation (Electronic Devices license)", "Heated Barrel (Electronic Devices license)", 0 )

GAME( 1993, godzilla,  0,        godzilla, godzilla, legionna_state, init_godzilla,  ROT0, "Banpresto", "Godzilla (Japan)", 0 )
GAME( 1993, grainbow,  0,        grainbow, grainbow, legionna_state, empty_init,     ROT0, "Banpresto", "SD Gundam Sangokushi Rainbow Tairiku Senki (Japan)", 0 )
GAME( 1993, grainbowk, grainbow, grainbow, grainbow, legionna_state, empty_init,     ROT0, "Banpresto", "SD Gundam Sangokushi Rainbow Tairiku Senki (Korea)", 0 )
GAME( 1994, denjinmk,  0,        denjinmk, denjinmk, legionna_state, empty_init,     ROT0, "Winkysoft (Banpresto license)", "Denjin Makai (set 1)", 0 )
GAME( 1994, denjinmka, denjinmk, denjinmk, denjinmk, legionna_state, empty_init,     ROT0, "Winkysoft (Banpresto license)", "Denjin Makai (set 2)", 0 )

GAME( 1992, cupsoc,    0,        cupsoc,   cupsoc,   legionna_state, init_cupsoc,    ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsoca,   cupsoc,   cupsoc,   cupsoc,   legionna_state, init_cupsoc,    ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocb,   cupsoc,   cupsoc,   cupsoc,   legionna_state, init_cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocs,   cupsoc,   cupsocs,  cupsoc,   legionna_state, init_cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer :Selection: (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocs2,  cupsoc,   cupsocs,  cupsoc,   legionna_state, init_cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer :Selection: (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92,  cupsoc,   cupsoc,   cupsoc,   legionna_state, init_olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92a, cupsoc,   cupsoc,   cupsoc,   legionna_state, init_olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92b, cupsoc,   cupsoc,   cupsoc,   legionna_state, init_olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
