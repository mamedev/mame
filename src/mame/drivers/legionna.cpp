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

- There are a bunch of unemulated registers, one of them seems to be a brightness control of some sort. Needs a PCB side-by-side test.

- backdrop color is ugly, especially noticeable in the port harbour stage (level 4). It should be dark blue or black but it's currently grey.

- there are some ROM writes from time to time, could be a coding bug or something related to the protection.

Godzilla
--------

The COP-MCU appears to write to the work ram area,otherwise it resets in mid-animation
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
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "cpu/m68000/m68000.h"
#include "machine/seicop.h"
#include "includes/legionna.h"

/*****************************************************************************/

READ16_MEMBER(legionna_state::sound_comms_r)
{
	return m_seibu_sound->main_word_r(space,(offset >> 1) & 7,0xffff);
}

WRITE16_MEMBER(legionna_state::sound_comms_w)
{
	m_seibu_sound->main_word_w(space,(offset >> 1) & 7,data,0x00ff);
}

static ADDRESS_MAP_START( legionna_cop_mem, AS_PROGRAM, 16, legionna_state )
	AM_RANGE(0x100400, 0x100401) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_param_lo_w) // grainbow
	AM_RANGE(0x100402, 0x100403) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_param_hi_w) // grainbow
	AM_RANGE(0x10040c, 0x10040d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_size_w) // grainbow
	AM_RANGE(0x100410, 0x100411) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_inc_w) // grainbow
	AM_RANGE(0x100412, 0x100413) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_src_hi_w) // grainbow
	AM_RANGE(0x100414, 0x100415) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sprite_dma_src_lo_w) // grainbow

	AM_RANGE(0x10041c, 0x10041d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_angle_target_w) // angle target (for 0x6200 COP macro)
	AM_RANGE(0x10041e, 0x10041f) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_angle_step_w)   // angle step   (for 0x6200 COP macro)

	AM_RANGE(0x100420, 0x100421) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_low_w)
	AM_RANGE(0x100422, 0x100423) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_high_w)
	AM_RANGE(0x100424, 0x100425) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_digit_count_w)
	AM_RANGE(0x100428, 0x100429) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_v1_w)
	AM_RANGE(0x10042a, 0x10042b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_v2_w)
	AM_RANGE(0x10042c, 0x10042d) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_prng_maxvalue_r, cop_prng_maxvalue_w)

	AM_RANGE(0x100432, 0x100433) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_data_w)
	AM_RANGE(0x100434, 0x100435) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_addr_w)
	AM_RANGE(0x100436, 0x100437) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_hitbox_baseadr_w)
	AM_RANGE(0x100438, 0x100439) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_value_w)
	AM_RANGE(0x10043a, 0x10043b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_mask_w)
	AM_RANGE(0x10043c, 0x10043d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_trigger_w)
	//AM_RANGE(0x10043e, 0x10043f) AM_DEVWRITE("raiden2cop", raiden2cop_device,)    /*  0 in all 68k based games,   0xffff in raiden2 / raidendx,   0x2000 in zeroteam / xsedae , it's always set up just before the 0x474 register */

	AM_RANGE(0x100444, 0x100445) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_scale_w)
	AM_RANGE(0x100446, 0x100447) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_rom_addr_unk_w) // 68k
	AM_RANGE(0x100448, 0x100449) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_rom_addr_lo_w) // 68k
	AM_RANGE(0x10044a, 0x10044b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_rom_addr_hi_w) // 68k

	AM_RANGE(0x100450, 0x100451) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_ram_addr_hi_w)
	AM_RANGE(0x100452, 0x100453) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_ram_addr_lo_w)
	AM_RANGE(0x100454, 0x100455) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_lookup_hi_w)
	AM_RANGE(0x100456, 0x100457) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_lookup_lo_w)
	AM_RANGE(0x100458, 0x100459) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_param_w)
	AM_RANGE(0x10045a, 0x10045b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pal_brightness_val_w) //palette DMA brightness val, used by X Se Dae / Zero Team
	AM_RANGE(0x10045c, 0x10045d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pal_brightness_mode_w)  //palette DMA brightness mode, used by X Se Dae / Zero Team (sets to 5)

//  AM_RANGE(0x100470, 0x100471) AM_READWRITE(cop_tile_bank_2_r,cop_tile_bank_2_w)
//  AM_RANGE(0x100474, 0x100475) AM_DEVWRITE("raiden2cop", raiden2cop_device,) // this gets set to a pointer to spriteram (relative to start of ram) on all games excecpt raiden 2, where it isn't set
	AM_RANGE(0x100476, 0x100477) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_adr_rel_w)
	AM_RANGE(0x100478, 0x100479) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_src_w)
	AM_RANGE(0x10047a, 0x10047b) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_size_w)
	AM_RANGE(0x10047c, 0x10047d) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_dst_w)
	AM_RANGE(0x10047e, 0x10047f) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_dma_mode_r, cop_dma_mode_w)

	AM_RANGE(0x10048c, 0x10048d) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_sprite_dma_abs_y_w) // 68k
	AM_RANGE(0x10048e, 0x10048f) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_sprite_dma_abs_x_w) // 68k

	AM_RANGE(0x1004a0, 0x1004ad) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_reg_high_r, cop_reg_high_w)
	AM_RANGE(0x1004c0, 0x1004cd) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_reg_low_r, cop_reg_low_w)

//  AM_RANGE(0x100500, 0x100505) AM_WRITE(cop_cmd_w) // ADD ME
	AM_RANGE(0x100500, 0x100505) AM_DEVWRITE("raiden2cop", raiden2cop_device,LEGACY_cop_cmd_w) // REMOVE ME

	AM_RANGE(0x100580, 0x100581) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_r)
	AM_RANGE(0x100582, 0x100587) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_val_r)


	AM_RANGE(0x100588, 0x100589) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_stat_r)
	AM_RANGE(0x100590, 0x100599) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_itoa_digits_r)

	AM_RANGE(0x1005a0, 0x1005a7) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_prng_r)

	AM_RANGE(0x1005b0, 0x1005b1) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_status_r)
	AM_RANGE(0x1005b2, 0x1005b3) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_dist_r)
	AM_RANGE(0x1005b4, 0x1005b5) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_angle_r)

	AM_RANGE(0x1006fc, 0x1006fd) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_trigger_w)
	AM_RANGE(0x1006fe, 0x1006ff) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_sort_dma_trig_w) // sort-DMA trigger
ADDRESS_MAP_END


static ADDRESS_MAP_START( legionna_map, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100470, 0x100471) AM_WRITENOP // toggles 0x2000 / 0x0000, tile bank on some games
	AM_RANGE(0x100600, 0x10063f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100680, 0x100681) AM_WRITENOP // writes 0x0000
	AM_RANGE(0x100700, 0x10071f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x102000, 0x1027ff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102800, 0x1037ff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x104000, 0x104fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* palette xRRRRxGGGGxBBBBx ? */
	AM_RANGE(0x105000, 0x105fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x106000, 0x107fff) AM_RAM
	AM_RANGE(0x108000, 0x11ffff) AM_RAM /* main ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( heatbrl_map, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100470, 0x100471) AM_WRITE(heatbrl_setgfxbank)
	AM_RANGE(0x100640, 0x10068f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1007c0, 0x1007df) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100800, 0x100fff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102000, 0x102fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103000, 0x103fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x104000, 0x104fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x108000, 0x11ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( godzilla_map, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100470, 0x100471) AM_WRITE(denjinmk_setgfxbank)
	AM_RANGE(0x100600, 0x10063f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x10071f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x100800, 0x100fff) AM_RAM
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x102000, 0x1027ff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102800, 0x1037ff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103800, 0x103fff) AM_RAM // check?
	AM_RANGE(0x104000, 0x104fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x105000, 0x105fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x106000, 0x1067ff) AM_RAM
	AM_RANGE(0x106800, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x107fff) AM_RAM /*Ani-DSP ram*/
	AM_RANGE(0x108000, 0x11ffff) AM_RAM
ADDRESS_MAP_END


/* did they swap the lines, or does the protection device swap the words during the DMA?? */
WRITE16_MEMBER(legionna_state::wordswapram_w)
{
	offset^=1;
	COMBINE_DATA(&m_wordswapram[offset]);
}


static ADDRESS_MAP_START( denjinmk_map, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100470, 0x100471) AM_WRITE(denjinmk_setgfxbank)
	AM_RANGE(0x100600, 0x10063f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x10071f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x10075c, 0x10075d) AM_READ_PORT("DSW2")
	AM_RANGE(0x100800, 0x100fff) AM_RAM
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x102000, 0x1027ff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102800, 0x103fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x104000, 0x104fff) AM_RAM_WRITE(wordswapram_w) AM_SHARE("wordswapram")
	AM_RANGE(0x105000, 0x105fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x106000, 0x107fff) AM_RAM
	AM_RANGE(0x108000, 0x11dfff) AM_RAM
	AM_RANGE(0x11e000, 0x11efff) AM_RAM
	AM_RANGE(0x11f000, 0x11ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( grainbow_map, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100600, 0x10063f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x10071f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x10075c, 0x10075d) AM_READ_PORT("DSW2")
	AM_RANGE(0x100800, 0x100fff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102000, 0x102fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103000, 0x103fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x104000, 0x104fff) AM_RAM//_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x105000, 0x105fff) AM_RAM
	AM_RANGE(0x106000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x107fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x108000, 0x11ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsoc_mem, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100600, 0x10063f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x10071f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100740, 0x100741) AM_READ_PORT("DSW1")
	AM_RANGE(0x100744, 0x100745) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100748, 0x100749) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10074c, 0x10074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x10075c, 0x10075d) AM_READ_PORT("DSW2")
	AM_RANGE(0x100800, 0x100fff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102000, 0x102fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103000, 0x103fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x104000, 0x104fff) AM_RAM
	AM_RANGE(0x105000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x1077ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x107800, 0x107fff) AM_RAM /*Ani Dsp(?) Ram*/
	AM_RANGE(0x108000, 0x10ffff) AM_RAM
	AM_RANGE(0x110000, 0x119fff) AM_RAM
	AM_RANGE(0x11a000, 0x11dfff) AM_RAM
	AM_RANGE(0x11e000, 0x11ffff) AM_RAM /*Stack Ram*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsocs_mem, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100600, 0x10060f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)//?
	AM_RANGE(0x100640, 0x10067f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x100701) AM_READ_PORT("DSW1")
	AM_RANGE(0x100704, 0x100705) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100708, 0x100709) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10070c, 0x10070d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x10071c, 0x10071d) AM_READ_PORT("DSW2")
	AM_RANGE(0x100740, 0x10075f) AM_READWRITE(sound_comms_r,sound_comms_w)
	AM_RANGE(0x100800, 0x100fff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102000, 0x102fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103000, 0x103fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x104000, 0x104fff) AM_RAM
	AM_RANGE(0x105000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x1077ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x107800, 0x107fff) AM_RAM /*Ani Dsp(?) Ram*/
	AM_RANGE(0x108000, 0x10ffff) AM_RAM
	AM_RANGE(0x110000, 0x119fff) AM_RAM
	AM_RANGE(0x11a000, 0x11dfff) AM_RAM
	AM_RANGE(0x11e000, 0x11ffff) AM_RAM /*Stack Ram*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsocbl_mem, AS_PROGRAM, 16, legionna_state )
	AM_IMPORT_FROM( legionna_cop_mem )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	//AM_RANGE(0x100000, 0x1003ff) AM_RAM
	AM_RANGE(0x100600, 0x10060f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)//?
	AM_RANGE(0x100640, 0x10067f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0x100700, 0x100701) AM_READ_PORT("DSW1")
	AM_RANGE(0x100704, 0x100705) AM_READ_PORT("PLAYERS12")
	AM_RANGE(0x100708, 0x100709) AM_READ_PORT("PLAYERS34")
	AM_RANGE(0x10070c, 0x10070d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x100000, 0x1007ff) AM_DEVREADWRITE("seibucop_boot", seibu_cop_bootleg_device, copdxbl_0_r,copdxbl_0_w) AM_SHARE("cop_mcu_ram")
	AM_RANGE(0x100800, 0x100fff) AM_RAM // _WRITE(legionna_background_w) AM_SHARE("back_data")
	AM_RANGE(0x101000, 0x1017ff) AM_RAM // _WRITE(legionna_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x101800, 0x101fff) AM_RAM // _WRITE(legionna_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x102000, 0x102fff) AM_RAM // _WRITE(legionna_text_w) AM_SHARE("textram")
	AM_RANGE(0x103000, 0x103fff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x104000, 0x104fff) AM_RAM
	AM_RANGE(0x105000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x1077ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x107800, 0x107fff) AM_RAM /*Ani Dsp(?) Ram*/
	AM_RANGE(0x108000, 0x10ffff) AM_RAM
	AM_RANGE(0x110000, 0x119fff) AM_RAM
	AM_RANGE(0x11a000, 0x11dfff) AM_RAM
	AM_RANGE(0x11e000, 0x11ffff) AM_RAM /*Stack Ram*/
ADDRESS_MAP_END


WRITE8_MEMBER(legionna_state::okim_rombank_w)
{
//  popmessage("%08x",0x40000 * (data & 0x07));
	m_oki->set_bank_base(0x40000 * (data & 0x7));
}

static ADDRESS_MAP_START( cupsocbl_sound_mem, AS_PROGRAM, 8, legionna_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(okim_rombank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

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
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001e, "A 1/1 B 1/2" )
	PORT_DIPSETTING(      0x0014, "A 2/1 B 1/3" )
	PORT_DIPSETTING(      0x000a, "A 3/1 B 1/5" )
	PORT_DIPSETTING(      0x0000, "A 5/1 B 1/6" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Extend" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("PLAYERS34")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001e, "A 1/1 B 1/2" )
	PORT_DIPSETTING(      0x0014, "A 2/1 B 1/3" )
	PORT_DIPSETTING(      0x000a, "A 3/1 B 1/5" )
	PORT_DIPSETTING(      0x0000, "A 5/1 B 1/6" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0400, 0x0400, "Extend" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0002, "2")
	PORT_DIPSETTING(      0x0003, "3")
	PORT_DIPSETTING(      0x0000, "5")
	PORT_DIPSETTING(      0x0001, "4")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( denjinmk )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Language ) ) // it actually skips the story entirely, so just remain JP as default
	PORT_DIPSETTING(      0x0080, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0600, "3 Coins / 5 Credits" )
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
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x6000, "3 Coins / 5 Credits" )
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
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) //TEST
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
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
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN2 )

	PORT_START("PLAYERS34")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //TEST
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin 1 (3)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin 2 (4)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Staring Coin" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "x2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Time vs Computer" )
	PORT_DIPSETTING(      0x0200, "1:30" )
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0100, "2:30" )
	PORT_DIPSETTING(      0x0000, "3:00" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time vs Player, 2 Players" )
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0c00, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "x:xx" )
	PORT_DIPNAME( 0x3000, 0x3000, "Time vs Player, 3 Players" )
	PORT_DIPSETTING(      0x2000, "2:30" )
	PORT_DIPSETTING(      0x3000, "3:00" )
	PORT_DIPSETTING(      0x1000, "3:30" )
	PORT_DIPSETTING(      0x0000, "x:xx" )
	PORT_DIPNAME( 0xc000, 0xc000, "Time vs Player, 4 Players" )
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0xc000, "3:30" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "x:xx" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x0000, "Players / Coin Mode" )
	PORT_DIPSETTING(      0x0000, "4 Players / 1 Coin Slot" )
	PORT_DIPSETTING(      0x0004, "4 Players / 4 Coin Slots" )
	PORT_DIPSETTING(      0x0008, "4 Players / 2 Coin Slots" )
	PORT_DIPSETTING(      0x000c, "2 Players" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// probably not mapped, service lists 3*8 dips
	PORT_DIPNAME( 0xff00, 0xff00, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0xff00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*****************************************************************************/

static const gfx_layout legionna_new_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 4, 8, 12 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


void legionna_state::descramble_legionnaire_gfx(UINT8* src)
{
	int len = 0x10000;

	/*  rearrange gfx */
	dynamic_buffer buffer(len);
	{
		int i;
		for (i = 0;i < len; i++)
		{
			buffer[i] = src[BITSWAP24(i,
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


static const gfx_layout legionna_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 2*4, 3*4, 0*4, 1*4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
		64*8+3, 64*8+2, 64*8+1, 64*8+0, 64*8+16+3, 64*8+16+2, 64*8+16+1, 64*8+16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout legionna_tilelayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0*4, 1*4, 2*4, 3*4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
		64*8+3, 64*8+2, 64*8+1, 64*8+0, 64*8+16+3, 64*8+16+2, 64*8+16+1, 64*8+16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout legionna_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 2*4, 3*4, 0*4, 1*4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
		64*8+3, 64*8+2, 64*8+1, 64*8+0, 64*8+16+3, 64*8+16+2, 64*8+16+1, 64*8+16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static GFXDECODE_START( legionna )
	GFXDECODE_ENTRY( "char", 0, legionna_new_charlayout, 48*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, legionna_tilelayout,      0*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, legionna_tilelayout,     32*16, 16 )
	GFXDECODE_ENTRY( "sprite", 0, legionna_spritelayout,    0*16, 8*16 )
	GFXDECODE_ENTRY( "gfx5", 0, legionna_tilelayout2,   32*16, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( heatbrl )
	GFXDECODE_ENTRY( "char", 0, legionna_new_charlayout,    48*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, legionna_tilelayout,    0*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, legionna_tilelayout,   32*16, 16 ) /* unused */
	GFXDECODE_ENTRY( "sprite", 0, legionna_spritelayout,  0*16, 8*16 )
	GFXDECODE_ENTRY( "gfx5", 0, legionna_tilelayout,   32*16, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( cupsoc )
	GFXDECODE_ENTRY( "char", 0, legionna_new_charlayout,    48*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, legionna_tilelayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx4", 0, legionna_tilelayout,   32*16, 16 ) /* unused */
	GFXDECODE_ENTRY( "sprite", 0, legionna_spritelayout,  0*16, 8*16 )
	GFXDECODE_ENTRY( "gfx5", 0, legionna_tilelayout,   32*16, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END


static const gfx_layout cupsocsb_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24, 512+4, 512+0, 512+12, 512+8, 512+20, 512+16, 512+28, 512+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static const gfx_layout cupsocsb_8x8_tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 0,3,2,1,16,19,18,17 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout cupsocsb_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 0,3,2,1,16,19,18,17,  512+0,512+3,512+2,512+1,512+16,512+19,512+18,512+17 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};


static GFXDECODE_START( heatbrl_csb )
	GFXDECODE_ENTRY( "char", 0, cupsocsb_8x8_tilelayout,    48*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, cupsocsb_tilelayout,        0*16, 32 )
	GFXDECODE_ENTRY( "gfx4", 0, cupsocsb_tilelayout,        32*16, 16 ) /* unused */
	GFXDECODE_ENTRY( "sprite", 0, cupsocsb_spritelayout,      0*16, 8*16 )
	GFXDECODE_ENTRY( "gfx5", 0, cupsocsb_tilelayout,        32*16, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, cupsocsb_tilelayout,        16*16, 16 )
GFXDECODE_END


static GFXDECODE_START( grainbow )
	GFXDECODE_ENTRY( "char", 0, legionna_new_charlayout,    48*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, legionna_tilelayout,        0*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, legionna_tilelayout,        32*16, 16 ) /* unused */
	GFXDECODE_ENTRY( "sprite", 0, legionna_spritelayout,      0*16, 8*16 )
	GFXDECODE_ENTRY( "gfx5", 0, legionna_tilelayout,        32*16, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, legionna_tilelayout,        16*16, 16 )
GFXDECODE_END

/*****************************************************************************/

static MACHINE_CONFIG_START( legionna, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,20000000/2)  /* ??? */
	MCFG_CPU_PROGRAM_MAP(legionna_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(36*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_legionna)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", legionna)
	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,legionna)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( heatbrl, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,20000000/2)  /* ??? */
	MCFG_CPU_PROGRAM_MAP(heatbrl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(36*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_legionna)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", heatbrl)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,legionna)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( godzilla, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000/2)
	MCFG_CPU_PROGRAM_MAP(godzilla_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(61)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
//  MCFG_SCREEN_SIZE(42*8, 36*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_RAW_PARAMS(14318180/2,455,0,320,258,0,224) // ~61 Hz, 15.734 kHz
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_godzilla)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_BASE_CB(WRITE16(legionna_state, tile_scroll_base_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", heatbrl)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,godzilla)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( denjinmk, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000/2)
	MCFG_CPU_PROGRAM_MAP(denjinmk_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(42*8, 36*8)
	MCFG_SCREEN_REFRESH_RATE(61)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_godzilla)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", heatbrl)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))


	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,denjinmk)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( grainbow, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000/2)
	MCFG_CPU_PROGRAM_MAP(grainbow_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(64*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 42*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_grainbow)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", grainbow)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,grainbow)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( cupsoc, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,20000000/2)
	MCFG_CPU_PROGRAM_MAP(cupsoc_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(42*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_grainbow)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cupsoc)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,cupsoc)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cupsocs, cupsoc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cupsocs_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cupsocbl, legionna_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,12000000)
	MCFG_CPU_PROGRAM_MAP(cupsocbl_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", legionna_state,  irq4_line_hold) /* VBL */

	MCFG_SEIBU_COP_ADD("seibucop_boot")
	MCFG_LEGIONNACOP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(legionna_state, videowrite_cb_w))


	/*Different Sound hardware*/
	//SEIBU_SOUND_SYSTEM_CPU(14318180/4)
	MCFG_CPU_ADD("audiocpu", Z80,14318180/4)
	MCFG_CPU_PROGRAM_MAP(cupsocbl_sound_mem)
	//MCFG_PERIODIC_INT("screen", nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(42*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(legionna_state, screen_update_grainbow)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(legionna_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(legionna_state, tile_scroll_w))
	MCFG_SEIBU_CRTC_REG_1A_CB(WRITE16(legionna_state, tile_vreg_1a_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", heatbrl_csb)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 128*16)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(legionna_state,cupsoc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

// all 3 Legionnaire sets differ only by the region byte at 0x1ef in rom 4 (Japan 0x00, US 0x01, World 0x02)
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

	ROM_REGION( 0x010000, "gfx5", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "user2", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )    /* 3 sets of tiles ('MBK','LBK','BK3') */

	ROM_REGION( 0x80000, "gfx3", 0 )  /* MBK */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x80000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_ERASEFF )
	/* Not Used */

	ROM_REGION( 0x80000, "gfx6", 0 )    /* LBK */
	ROM_COPY( "user2", 0x080000, 0x000000, 0x78000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u091",   0x000000, 0x000200, NO_DUMP ) /* N82S147N type BPROM */
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

	ROM_REGION( 0x010000, "gfx5", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "user2", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )    /* 3 sets of tiles ('MBK','LBK','BK3') */

	ROM_REGION( 0x80000, "gfx3", 0 )  /* MBK */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x80000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_ERASEFF )
	/* Not Used */

	ROM_REGION( 0x80000, "gfx6", 0 )    /* LBK */
	ROM_COPY( "user2", 0x080000, 0x000000, 0x78000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u091",   0x000000, 0x000200, NO_DUMP ) /* N82S147N type BPROM */
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

	ROM_REGION( 0x010000, "gfx5", 0 )  /* BK3 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x010000 ) /* decrambled in INIT */

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD( "legionnire_obj1.u0815", 0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )   /* sprites */
	ROM_LOAD( "legionnire_obj2.u0814", 0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, "user2", 0 ) /* load the tiles here so we can split them up into the required regions by hand */
	ROM_LOAD( "legionnire_back.u075", 0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )    /* 3 sets of tiles ('MBK','LBK','BK3') */

	ROM_REGION( 0x80000, "gfx3", 0 )  /* MBK */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x80000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_ERASEFF )
	/* Not Used */

	ROM_REGION( 0x80000, "gfx6", 0 )    /* LBK */
	ROM_COPY( "user2", 0x080000, 0x000000, 0x78000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "5.u106", 0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )

	ROM_REGION( 0x080000, "copx", 0 ) /* SEI300 data rom */
	ROM_LOAD( "copx-d1.u0330",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) ) /* not dumped from this PCB assumed to be the same */

	ROM_REGION( 0x200, "proms", 0 ) /* Priority? */
	ROM_LOAD( "leg007.u091",   0x000000, 0x000200, NO_DUMP ) /* N82S147N type BPROM */
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
	ROM_LOAD( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, "gfx4", 0 )   /* not used? */
	ROM_COPY( "char", 0x010000, 0x000000, 0x010000 ) // this is just corrupt tiles if we decode it

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_LOAD( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "user1", 0 )  /* SEI300 data rom */
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
	ROM_LOAD( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, "gfx4", 0 )   /* not used? */
	ROM_COPY( "char", 0x010000, 0x000000, 0x010000 ) // this is just corrupt tiles if we decode it

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_LOAD( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "user1", 0 )  /* SEI300 data rom */
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
	ROM_LOAD( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_LOAD( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "user1", 0 )  /* SEI300 data rom */
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
	ROM_LOAD( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_LOAD( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "user1", 0 )  /* SEI300 data rom */
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
	ROM_LOAD( "heated-barrel_obj1.u085",  0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )
	ROM_LOAD( "heated-barrel_obj2.u0814", 0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "heated-barrel_bg-1.u075", 0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, "gfx4", 0 )   /* not used? */
	ROM_COPY( "char", 0x010000, 0x000000, 0x010000 ) // this is just corrupt tiles if we decode it

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "heated-barrel_bg-3.u076", 0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_LOAD( "heated-barrel_bg-2.u074", 0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM samples */
	ROM_LOAD( "barrel_8.u106",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )

	ROM_REGION( 0x200, "proms", 0 )     /* Priority */
	ROM_LOAD( "heat07.u0910",   0x000000, 0x000200, CRC(265eccc8) SHA1(cf650c69f97b887251b5079e5518497721692af3) ) /* N82S147N type BPROM */

	ROM_REGION( 0x080000, "user1", 0 )  /* SEI300 data rom */
	ROM_LOAD( "copx-d2.u0339",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) ) /* not dumped from this PCB assumed to be the same */
ROM_END

/*

Godzilla
Banpresto 1993

This game runs on Seibu hardware, similar to Legionairre.

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
	ROM_LOAD( "obj1.748",     0x000000, 0x200000, CRC(0dfaf26d) SHA1(2af3ea06369c40ae89c2f8362c273f4801db8e68) )
	ROM_LOAD( "obj2.756",     0x200000, 0x200000, CRC(32b1516a) SHA1(4adcf4b957f6b9baf1a5b8807b381db664de632d) )
	ROM_LOAD( "obj3.743",     0x400000, 0x100000, CRC(5af0114e) SHA1(9362de9ade6db67ab0e3a2dfea580e688bbf7729) )
	ROM_LOAD( "obj4.757",     0x500000, 0x100000, CRC(7448b054) SHA1(5c08319329eb8c90b63e5393c0011bc39911ebbb) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "bg1.618",      0x000000, 0x100000, CRC(78fbbb84) SHA1(b1f5d4041bb88c5b2a561949239b11c3fd7c5fbc) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x100000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "bg2.619",      0x000000, 0x100000, CRC(8ac192a5) SHA1(54b557e81a704c70a651e6b8da70207a2a70530f) )

	ROM_REGION( 0x100000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx3", 0x80000, 0x00000, 0x080000 )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "pcm.922",      0x000000, 0x080000, CRC(59cbef10) SHA1(6b89b7286f80f9c903dfb81dc93a03c38dff707c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d2.313",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

Denjin Makai
Banpresto, 1994

This game runs on early 90's Seibu hardware.
(i.e. Raiden II, Godzilla, Seibu Cup Soccer, Legionairre, Heated Barrel etc).
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

ROM_START( denjinmk )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "rom1.025",        0x000000, 0x040000, CRC(44a648e8) SHA1(a3c1721e89ac6b9fc16f80682b2f701cb24b5d76) )
	ROM_LOAD32_BYTE( "rom2.024",        0x000001, 0x040000, CRC(e5ee8fe0) SHA1(2ebff4fdbe82062fb526598e10f11358b0b5c02f) )
	ROM_LOAD32_BYTE( "rom3.026",        0x000002, 0x040000, CRC(781b942e) SHA1(f1f4ddc332de3dc29b716a1b82c2ecc2045efb3a) )
	ROM_LOAD32_BYTE( "rom4.023",        0x000003, 0x040000, CRC(502a588b) SHA1(9055b631240fe52d33b572e34275d31a9f3d290f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "rom5.016",        0x000000, 0x08000, CRC(7fe7e352) SHA1(1ceae22186751ca91dfffab7bd11f275e693451f) )
	ROM_CONTINUE(             0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "rom7.620",       0x000000, 0x010000, CRC(e1f759b1) SHA1(ddc60e78e7791a59c59403dd4089b3f6e1ecf8cb) )
	ROM_LOAD16_BYTE( "rom8.615",       0x000001, 0x010000, CRC(cc36af0d) SHA1(69c2ae38f03be79be4d138fcc73a6a86407eb285) )

	ROM_REGION( 0x500000, "sprite", 0 )
	ROM_LOAD( "obj-0-3.748",     0x000000, 0x200000, CRC(67c26a67) SHA1(20543ca9dcf3fed0884968b5249b34b59a14b791) ) /* banks 0,1,2,3 */
	ROM_LOAD( "obj-4-5.756",     0x200000, 0x100000, CRC(01f8d4e6) SHA1(25b69da693be8c3404f750b419c330a7a56e88ec) ) /* 4,5 */
	ROM_LOAD( "obj-6-7.743",     0x300000, 0x100000, CRC(e5805757) SHA1(9d392c27eef7c1fcda560dac17ba9d7ae2287ac8) ) /* 6,7 */
	ROM_LOAD( "obj-8-9.757",     0x400000, 0x100000, CRC(c8f7e1c9) SHA1(a746d187b50a0ecdd5a7f687a2601e5dc8bfe272) ) /* 8,9 */

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "bg-1-ab.618",      0x000000, 0x100000, CRC(eaad151a) SHA1(bdd1d83ee8497efe20f21baf873e786446372bcb) )

	ROM_REGION( 0x100000, "gfx4", 0 )   /* BK2 used */
	ROM_LOAD( "bg-2-ab.617",      0x000000, 0x100000, CRC(40938f74) SHA1(d68b0f8245a8b390ad5d4e6ebc7514a939b8ac51) )

	ROM_REGION( 0x100000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "bg-3-ab.619",      0x000000, 0x100000,  CRC(de7366ee) SHA1(0c3969d15f3cd963e579d4164b6e0a6b4012c9c6) )

	ROM_REGION( 0x100000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx4", 0x000000, 0x00000, 0x100000 )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "rom6.922",      0x000000, 0x040000, CRC(09e13213) SHA1(9500e057104c6b83da0467938e46d9efa2f49f4c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, "user1", 0 )
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
	ROM_LOAD( "rb-spr01.748", 0x000000, 0x100000, CRC(11a3479d) SHA1(4d2d06d62da02c6e9884735de8c319f37ca1715c) )
	ROM_LOAD( "rb-spr23.756", 0x100000, 0x100000, CRC(fd08a761) SHA1(3297a2bfaabef17ed9320e24e9a4ffa2f3eb3a44) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "rb-bg-01.618", 0x000000, 0x100000, CRC(6a4ca7e7) SHA1(13612d29f8f04cf62b4357b69b81240dd1eceae4) )

	ROM_REGION( 0x040000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x100000, "gfx5", 0 )
	ROM_LOAD( "rb-bg-2.619",  0x000000, 0x100000, CRC(a9b5c85e) SHA1(0ae044e05730e8080d94f1f6758f8dd051b03c41) )

	ROM_REGION( 0x100000, "gfx6", 0 )
//#define ROM_COPY(rgn,srcoffset,offset,length)
	ROM_COPY( "gfx3", 0x80000, 0x00000, 0x80000 )

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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "8_7a.bin", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
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
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu8.7a", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
ROM_END


ROM_START( olysoc92b )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD32_BYTE( "1", 0x000000, 0x040000, CRC(d4f37bf2) SHA1(af06364a602bd0ac2b9506de792bef003281e9d4) ) // == cupsocb    Seibu Cup Soccer (set 3)
	ROM_LOAD32_BYTE( "2", 0x000001, 0x040000, CRC(6967d6f9) SHA1(292f59cf501064cdfb20acd8295fee7e93c9bc95) ) // same as Seibu Cup Soccer (set 3) except last byte is 02 instead of 01
	ROM_LOAD32_BYTE( "3", 0x000002, 0x040000, CRC(226f65f9) SHA1(106a2f807aaf0f2e1fbcb1ffec6ccf4d2d7addd8) ) // == cupsocb    Seibu Cup Soccer (set 3)
	ROM_LOAD32_BYTE( "4", 0x000003, 0x040000, CRC(8ff16a9e) SHA1(c29986cec74e183d18eaaf69ba6ca20b75590298) ) // == cupsocb    Seibu Cup Soccer (set 3)

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "7",   0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "6", 0x000000, 0x010000, CRC(7edb1700) SHA1(db30c01fc0e5b9f3c2d6139f89ec88936bc75b38) )
	ROM_LOAD16_BYTE( "5", 0x000001, 0x010000, CRC(ec21c8dc) SHA1(93a9bf13d7c53e76762307c028f427ff2888629c) )

	ROM_REGION( 0x200000, "sprite", 0 )
	ROM_LOAD( "obj.8c", 0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, "gfx3", 0 )   /* MBK tiles */
	ROM_LOAD( "back-1.4y", 0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, "gfx4", ROMREGION_ERASEFF )   /* not used */

	ROM_REGION( 0x080000, "gfx5", 0 )   /* BK3 tiles */
	ROM_LOAD( "back-2.6y", 0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, "gfx6", 0 )   /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "8", 0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, "user1", 0 )
	ROM_LOAD( "copx-d1.bin", 0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
ROM_END

/*

Seibu Cup Soccer - Seibu - Bootleg

2 boards

1st board

(snd)
1 x z80
1 x oki 6295
sc_01 (prg)
sc_02 and sc_03 (data)

(prg)
1 x 68000
sc_04 and sc_05

(gfx)
2 x ti tpc1020
from sc_06 to sc_11

2nd board

(gfx)
1 x actel pl84c
from sc_12 to sc_15

*/

ROM_START( cupsocsb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "sc_04.bin", 0x00001, 0x80000, CRC(22566087) SHA1(4392f46ca50cc9947823a5190aa25f5e9654aa0d) )
	ROM_LOAD16_BYTE( "sc_05.bin", 0x00000, 0x80000, CRC(2f977dff) SHA1(4d8d6e7d06ce17bb7292072965911f8b1f1067e2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "sc_01.bin",    0x000000, 0x08000, CRC(cea39d6d) SHA1(f0b79c03ffafdd1e57673d6d4836becbe415110b) )
	ROM_CONTINUE(             0x000000, 0x08000 )

	ROM_REGION( 0x200000, "sprite", ROMREGION_INVERT ) /* bootleg sprite gfx */
	ROM_LOAD( "sc_07.bin", 0x000000, 0x080000, CRC(dcb29d01) SHA1(72b4234622605f0ab03f21fdb6a61c6dac36000d) )
	ROM_LOAD( "sc_06.bin", 0x080000, 0x080000, CRC(2dc70e05) SHA1(f1d0beb8428a7e1d7c7818e6719abdc543b2fa80) )
	ROM_COPY( "sprite", 0x000000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "test1", 0 ) /* bootleg tile gfx */
	ROM_LOAD16_BYTE( "sc_09.bin", 0x000000, 0x080000, CRC(695b6342) SHA1(dfccb43789021ba2568b9284ae61e64f7f89b152) )
	ROM_LOAD16_BYTE( "sc_10.bin", 0x000001, 0x080000, CRC(27e172b8) SHA1(ed86db2f42c8061607d46f2407b0130aaf692a02) )
	ROM_LOAD16_BYTE( "sc_08.bin", 0x100000, 0x080000, CRC(637120f3) SHA1(b4b2ad192e46ff80d4cb440d7fb6dac215a353ed) )
	ROM_LOAD16_BYTE( "sc_11.bin", 0x100001, 0x080000, CRC(0cd5ca5e) SHA1(a59665e543e9383355de2576e6693348ec356591) )

	ROM_REGION( 0x020000, "char", ROMREGION_INVERT )
	ROM_COPY( "test1", 0x080000, 0x00000, 0x020000 )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )    /* MBK tiles */
	ROM_COPY( "test1", 0x000000, 0x00000, 0x080000 )
	ROM_COPY( "test1", 0x100000, 0x80000, 0x080000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT )    /* not used */
	ROM_COPY("gfx3",0x000000,0x00000,0x100000)

	ROM_REGION( 0x080000, "gfx5", ROMREGION_INVERT )    /* BK3 tiles */
	ROM_COPY( "test1", 0x180000, 0x00000, 0x080000 )

	ROM_REGION( 0x080000, "gfx6", ROMREGION_INVERT )    /* LBK tiles */
	ROM_COPY( "gfx5", 0x000000, 0x00000, 0x080000 )

	ROM_REGION( 0x100000, "adpcm", ROMREGION_ERASEFF )  /* ADPCM samples */
	ROM_LOAD( "sc_02.bin",    0x000000, 0x020000, CRC(a70d4f03) SHA1(c2482e624c8a828a94206a36d10c1021ad8ca1d0) )
	ROM_LOAD( "sc_03.bin",    0x080000, 0x080000, CRC(6e254d12) SHA1(857779dbd276b688201a8ea3afd5817e38acad2e) )

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASEFF )
	ROM_COPY( "adpcm", 0x00000, 0x000000, 0x20000 ) //bank 0
	ROM_COPY( "adpcm", 0x00000, 0x020000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x100000, 0x20000 ) //bank 4
	ROM_COPY( "adpcm", 0x80000, 0x120000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x140000, 0x20000 ) //bank 5
	ROM_COPY( "adpcm", 0xa0000, 0x160000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x180000, 0x20000 ) //bank 6
	ROM_COPY( "adpcm", 0xc0000, 0x1a0000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x1c0000, 0x20000 ) //bank 7
	ROM_COPY( "adpcm", 0xe0000, 0x1e0000, 0x20000 )


	/* these are maths tables, for whatever COP replacement the bootlegs use */
	ROM_REGION( 0x500000, "unknown0", 0 )
	ROM_LOAD16_BYTE( "sc_13.bin", 0x00000, 0x010000, CRC(229bddd8) SHA1(0924bf29db9c5a970546f154e7752697fdce6a58) )
	ROM_LOAD16_BYTE( "sc_12.bin", 0x00001, 0x010000, CRC(dabfa826) SHA1(0db587c846755491b169ef7751ba8e7cdc2607e6) )
	ROM_REGION( 0x500000, "unknown1", 0 )
	ROM_LOAD16_BYTE( "sc_15.bin", 0x00000, 0x080000, CRC(8fd87e65) SHA1(acc9fd0289fa9ab60bec16d3e642039380e5180a) )
	ROM_LOAD16_BYTE( "sc_14.bin", 0x00001, 0x080000, CRC(566086c2) SHA1(b7d09ce978f99ecc0d1975b31330ed49317701d5) )
ROM_END

/* slight changes in the program roms compared to above set, all remaining roms were the same */
ROM_START( cupsocsb2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "4", 0x00001, 0x80000, CRC(83db76f8) SHA1(ffcd0a728de58871b945c15cc27da374b587e170) )
	ROM_LOAD16_BYTE( "5", 0x00000, 0x80000, CRC(c01e88c6) SHA1(8f90261792343c92ddd877ab8a2480b5aac82961) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "sc_01.bin",    0x000000, 0x08000, CRC(cea39d6d) SHA1(f0b79c03ffafdd1e57673d6d4836becbe415110b) )
	ROM_CONTINUE(             0x000000, 0x08000 )

	ROM_REGION( 0x200000, "sprite", ROMREGION_INVERT ) /* bootleg sprite gfx */
	ROM_LOAD( "sc_07.bin", 0x000000, 0x080000, CRC(dcb29d01) SHA1(72b4234622605f0ab03f21fdb6a61c6dac36000d) )
	ROM_LOAD( "sc_06.bin", 0x080000, 0x080000, CRC(2dc70e05) SHA1(f1d0beb8428a7e1d7c7818e6719abdc543b2fa80) )
	ROM_COPY( "sprite", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "test1", 0 ) /* bootleg tile gfx */
	ROM_LOAD16_BYTE( "sc_09.bin", 0x000000, 0x080000, CRC(695b6342) SHA1(dfccb43789021ba2568b9284ae61e64f7f89b152) )
	ROM_LOAD16_BYTE( "sc_10.bin", 0x000001, 0x080000, CRC(27e172b8) SHA1(ed86db2f42c8061607d46f2407b0130aaf692a02) )
	ROM_LOAD16_BYTE( "sc_08.bin", 0x100000, 0x080000, CRC(637120f3) SHA1(b4b2ad192e46ff80d4cb440d7fb6dac215a353ed) )
	ROM_LOAD16_BYTE( "sc_11.bin", 0x100001, 0x080000, CRC(0cd5ca5e) SHA1(a59665e543e9383355de2576e6693348ec356591) )

	ROM_REGION( 0x020000, "char", ROMREGION_INVERT )
	ROM_COPY( "test1", 0x080000, 0x00000, 0x020000 )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )    /* MBK tiles */
	ROM_COPY( "test1", 0x00000, 0x00000, 0x080000 )
	ROM_COPY( "test1", 0x100000, 0x80000, 0x080000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT )    /* not used */
	ROM_COPY("gfx3",0x00000,0x00000,0x100000)

	ROM_REGION( 0x080000, "gfx5", ROMREGION_INVERT )    /* BK3 tiles */
	ROM_COPY( "test1", 0x180000, 0x00000, 0x080000 )

	ROM_REGION( 0x080000, "gfx6", ROMREGION_INVERT )    /* LBK tiles */
	ROM_COPY( "gfx5", 0x00000, 0x00000, 0x080000 )

	ROM_REGION( 0x100000, "adpcm", ROMREGION_ERASEFF )  /* ADPCM samples */
	ROM_LOAD( "sc_02.bin",    0x000000, 0x020000, CRC(a70d4f03) SHA1(c2482e624c8a828a94206a36d10c1021ad8ca1d0) )
	ROM_LOAD( "sc_03.bin",    0x080000, 0x080000, CRC(6e254d12) SHA1(857779dbd276b688201a8ea3afd5817e38acad2e) )

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASEFF )
	ROM_COPY( "adpcm", 0x00000, 0x000000, 0x20000 ) //bank 0
	ROM_COPY( "adpcm", 0x00000, 0x020000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x100000, 0x20000 ) //bank 4
	ROM_COPY( "adpcm", 0x80000, 0x120000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x140000, 0x20000 ) //bank 5
	ROM_COPY( "adpcm", 0xa0000, 0x160000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x180000, 0x20000 ) //bank 6
	ROM_COPY( "adpcm", 0xc0000, 0x1a0000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x1c0000, 0x20000 ) //bank 7
	ROM_COPY( "adpcm", 0xe0000, 0x1e0000, 0x20000 )


	/* these are maths tables, for whatever COP replacement the bootlegs use */
	ROM_REGION( 0x500000, "unknown0", 0 )
	ROM_LOAD16_BYTE( "sc_13.bin", 0x00000, 0x010000, CRC(229bddd8) SHA1(0924bf29db9c5a970546f154e7752697fdce6a58) )
	ROM_LOAD16_BYTE( "sc_12.bin", 0x00001, 0x010000, CRC(dabfa826) SHA1(0db587c846755491b169ef7751ba8e7cdc2607e6) )
	ROM_REGION( 0x500000, "unknown1", 0 )
	ROM_LOAD16_BYTE( "sc_15.bin", 0x00000, 0x080000, CRC(8fd87e65) SHA1(acc9fd0289fa9ab60bec16d3e642039380e5180a) )
	ROM_LOAD16_BYTE( "sc_14.bin", 0x00001, 0x080000, CRC(566086c2) SHA1(b7d09ce978f99ecc0d1975b31330ed49317701d5) )
ROM_END

ROM_START( cupsocsb3 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "4.bin", 0x00001, 0x80000, CRC(f615058f) SHA1(f7c0eb6b9f8dcdc8b13f8e5b03f46252a87a6c0f) ) // sldh
	ROM_LOAD16_BYTE( "5.bin", 0x00000, 0x80000, CRC(6500edf2) SHA1(1a617b18b4997c24af53601c98e9a0efbe637a4b) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "sc_01.bin",    0x000000, 0x08000, CRC(cea39d6d) SHA1(f0b79c03ffafdd1e57673d6d4836becbe415110b) )
	ROM_CONTINUE(             0x000000, 0x08000 )

	ROM_REGION( 0x200000, "sprite", ROMREGION_INVERT ) /* bootleg sprite gfx */
	ROM_LOAD( "sc_07.bin", 0x000000, 0x080000, CRC(dcb29d01) SHA1(72b4234622605f0ab03f21fdb6a61c6dac36000d) )
	ROM_LOAD( "sc_06.bin", 0x080000, 0x080000, CRC(2dc70e05) SHA1(f1d0beb8428a7e1d7c7818e6719abdc543b2fa80) )
	ROM_COPY( "sprite", 0x00000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "test1", 0 ) /* bootleg tile gfx */
	ROM_LOAD16_BYTE( "sc_09.bin", 0x000000, 0x080000, CRC(695b6342) SHA1(dfccb43789021ba2568b9284ae61e64f7f89b152) )
	ROM_LOAD16_BYTE( "sc_10.bin", 0x000001, 0x080000, CRC(27e172b8) SHA1(ed86db2f42c8061607d46f2407b0130aaf692a02) )
	ROM_LOAD16_BYTE( "sc_08.bin", 0x100000, 0x080000, CRC(637120f3) SHA1(b4b2ad192e46ff80d4cb440d7fb6dac215a353ed) )
	ROM_LOAD16_BYTE( "sc_11.bin", 0x100001, 0x080000, CRC(0cd5ca5e) SHA1(a59665e543e9383355de2576e6693348ec356591) )

	ROM_REGION( 0x020000, "char", ROMREGION_INVERT )
	ROM_COPY( "test1", 0x080000, 0x00000, 0x020000 )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )    /* MBK tiles */
	ROM_COPY( "test1", 0x00000, 0x00000, 0x080000 )
	ROM_COPY( "test1", 0x100000, 0x80000, 0x080000 )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT )    /* not used */
	ROM_COPY("gfx3",0x00000,0x00000,0x100000)

	ROM_REGION( 0x080000, "gfx5", ROMREGION_INVERT )    /* BK3 tiles */
	ROM_COPY( "test1", 0x180000, 0x00000, 0x080000 )

	ROM_REGION( 0x080000, "gfx6", ROMREGION_INVERT )    /* LBK tiles */
	ROM_COPY( "gfx5", 0x00000, 0x00000, 0x080000 )

	ROM_REGION( 0x100000, "adpcm", ROMREGION_ERASEFF )  /* ADPCM samples */
	ROM_LOAD( "sc_02.bin",    0x000000, 0x020000, CRC(a70d4f03) SHA1(c2482e624c8a828a94206a36d10c1021ad8ca1d0) )
	ROM_LOAD( "sc_03.bin",    0x080000, 0x080000, CRC(6e254d12) SHA1(857779dbd276b688201a8ea3afd5817e38acad2e) )

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASEFF )
	ROM_COPY( "adpcm", 0x00000, 0x000000, 0x20000 ) //bank 0
	ROM_COPY( "adpcm", 0x00000, 0x020000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x100000, 0x20000 ) //bank 4
	ROM_COPY( "adpcm", 0x80000, 0x120000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x140000, 0x20000 ) //bank 5
	ROM_COPY( "adpcm", 0xa0000, 0x160000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x180000, 0x20000 ) //bank 6
	ROM_COPY( "adpcm", 0xc0000, 0x1a0000, 0x20000 )
	ROM_COPY( "adpcm", 0x00000, 0x1c0000, 0x20000 ) //bank 7
	ROM_COPY( "adpcm", 0xe0000, 0x1e0000, 0x20000 )

	/* these are maths tables, for whatever COP replacement the bootlegs use */
	ROM_REGION( 0x500000, "unknown0", 0 )
	ROM_LOAD16_BYTE( "sc_13.bin", 0x00000, 0x010000, CRC(229bddd8) SHA1(0924bf29db9c5a970546f154e7752697fdce6a58) )
	ROM_LOAD16_BYTE( "sc_12.bin", 0x00001, 0x010000, CRC(dabfa826) SHA1(0db587c846755491b169ef7751ba8e7cdc2607e6) )
	ROM_REGION( 0x500000, "unknown1", 0 )
	ROM_LOAD16_BYTE( "sc_15.bin", 0x00000, 0x080000, CRC(8fd87e65) SHA1(acc9fd0289fa9ab60bec16d3e642039380e5180a) )
	ROM_LOAD16_BYTE( "sc_14.bin", 0x00001, 0x080000, CRC(566086c2) SHA1(b7d09ce978f99ecc0d1975b31330ed49317701d5) )
ROM_END



/* Does the COP or something else on the PCB provide a rom overlay for the last part of ROM?

   In Seibu Cup Soccer Selection the only way I've found to not display debug text is by changing this
   area, and for Olympic Soccer '92 it appears to be the only way to get the Olympic Soccer '92 titles
   to be used instead of the reagular Seibu Cup Soccer one (and AFAIK both dumps are confirmed to show
   that on hardware, as are all early versions with the advertising boards of dubious legality)

   You can also enable other debug menus by patching this area of ROM, but some initial tests appear to
   show those menus are still inaccessible with a patched rom on real hardware, again indicating there
   could be a rom overlay causing the patches to be ignored.

*/


// if this is 1 then P1 Button 3 during gameplay enters the 'Game Master' debug menu, with extensive
// debugging features.
#define CUPSOC_DEBUG_MODE 0

DRIVER_INIT_MEMBER(legionna_state, cupsoc_debug)
{
#if CUPSOC_DEBUG_MODE
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	ROM[0xffffa/2]  = 0x0000;
	ROM[0xffff6/2] ^= 0x00ff;
#endif
}

DRIVER_INIT_MEMBER(legionna_state, olysoc92)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	ROM[0xffffe/2] ^= 0x0003; // show Olympic Soccer '92 title

	DRIVER_INIT_CALL(cupsoc_debug);
}

DRIVER_INIT_MEMBER(legionna_state, cupsocs)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	ROM[0xffffa/2] = 0x00ff; // disable debug text (this is already 0x00ff in the bootleg sets for the same reason)

	DRIVER_INIT_CALL(cupsoc_debug);
}

DRIVER_INIT_MEMBER(legionna_state,cupsoc)
{
	DRIVER_INIT_CALL(cupsoc_debug);
}



DRIVER_INIT_MEMBER(legionna_state,denjinmk)
{
	/* problem with audio comms? */
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	ROM[0x5fe4/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(legionna_state,legiongfx)
{
	descramble_legionnaire_gfx(memregion("gfx5")->base() );
}



GAME( 1992, legionna, 0,        legionna, legionna, legionna_state, legiongfx, ROT0, "TAD Corporation",                  "Legionnaire (World)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, legionnau,legionna, legionna, legionna, legionna_state, legiongfx, ROT0, "TAD Corporation (Fabtek license)", "Legionnaire (US)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, legionnaj,legionna, legionna, legionna, legionna_state, legiongfx, ROT0, "TAD Corporation",                  "Legionnaire (Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )

GAME( 1992, heatbrl,  0,        heatbrl,  heatbrl, driver_device,  0,         ROT0, "TAD Corporation", "Heated Barrel (World version 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, heatbrl2, heatbrl,  heatbrl,  heatbrl, driver_device,  0,         ROT0, "TAD Corporation", "Heated Barrel (World version 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, heatbrlo, heatbrl,  heatbrl,  heatbrl, driver_device,  0,         ROT0, "TAD Corporation", "Heated Barrel (World old version)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, heatbrlu, heatbrl,  heatbrl,  heatbrl, driver_device,  0,         ROT0, "TAD Corporation", "Heated Barrel (US)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, heatbrle, heatbrl,  heatbrl,  heatbrl, driver_device,  0,         ROT0, "TAD Corporation (Electronic Devices license)", "Heated Barrel (Electronic Devices license)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )

GAME( 1993, godzilla, 0,        godzilla, godzilla, driver_device, 0,         ROT0, "Banpresto", "Godzilla (Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1993, grainbow, 0,        grainbow, grainbow, driver_device, 0,         ROT0, "Banpresto", "SD Gundam Sangokushi Rainbow Tairiku Senki", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1993, denjinmk, 0,        denjinmk, denjinmk, legionna_state,denjinmk,  ROT0, "Winkysoft (Banpresto license)", "Denjin Makai", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, cupsoc,   0,        cupsoc,   cupsoc, legionna_state,  cupsoc,    ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsoca,  cupsoc,   cupsoc,   cupsoc, legionna_state,  cupsoc,    ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocb,  cupsoc,   cupsoc,   cupsoc, legionna_state,  cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer (set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocs,  cupsoc,   cupsocs,  cupsoc, legionna_state,  cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer :Selection: (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocs2, cupsoc,   cupsocs,  cupsoc, legionna_state,  cupsocs,   ROT0, "Seibu Kaihatsu", "Seibu Cup Soccer :Selection: (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocsb, cupsoc,   cupsocbl, cupsoc, legionna_state,  cupsoc,    ROT0, "bootleg", "Seibu Cup Soccer :Selection: (bootleg, set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocsb2,cupsoc,   cupsocbl, cupsoc, legionna_state,  cupsoc,    ROT0, "bootleg", "Seibu Cup Soccer :Selection: (bootleg, set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, cupsocsb3,cupsoc,   cupsocbl, cupsoc, legionna_state,  cupsoc,    ROT0, "bootleg", "Seibu Cup Soccer :Selection: (bootleg, set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92, cupsoc,   cupsoc,   cupsoc, legionna_state,  olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92a,cupsoc,   cupsoc,   cupsoc, legionna_state,  olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1992, olysoc92b,cupsoc,   cupsoc,   cupsoc, legionna_state,  olysoc92,  ROT0, "Seibu Kaihatsu", "Olympic Soccer '92 (set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
