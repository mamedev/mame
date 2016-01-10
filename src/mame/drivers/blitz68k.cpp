// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, Tomasz Slanina, Luca Elia
/*************************************************************************************************************

Blitter based gambling games
68000 CPU + 8bit MCUs (68HC7058/87C748), CRT controller, RAMDAC

Preliminary driver by David Haywood, Angelo Salese, Tomasz Slanina, Luca Elia

----------------------------------------------------------------------
Year  Game                        Manufacturer
----------------------------------------------------------------------
1990  Mega Double Poker           Blitz Systems Inc.
1990  Mega Double Poker Jackpot   Blitz Systems Inc.
1993  Bank Robbery                Entertainment Technology Corp.
1993? Poker 52                    Blitz Systems Inc.
1993  Strip Teaser                <unknown>
1995  Dual Games (proto)          Labtronix Technologies
1995  The Hermit                  Dugamex
1997  Deuces Wild 2               <unknown>
1998  Funny Fruit                 Cadillac Jack
1998  Triple Play                 Cadillac Jack
199?  Il Pagliaccio               <unknown>
----------------------------------------------------------------------

Notes:

- ilpag: at start-up a "initialize request" pops up. Press Service Mode and the Service switch, and
  reset with F3 for doing it.
- cjffruit: at start-up a "need coin adjustment" pops up. Press menu, go to page 1 with start, move to
  "price coin #1" with big, and set it with small, then exit with menu.
- "I/O TEST" is available among the statistics pages.
- ilpag: based on pSOS+ S68000 V1.2.3 (Integrated Systems).

To Do:

- ilpag: protection not yet checked at all. My guess is it communicates via irq 3 and/or 6;
- steaser: understand the shared ram inputs better and find the coin chutes;
- ilpag, steaser: some minor issues with the blitter emulation;
- ilpag: sound uses a MP7524 8-bit DAC (bottom right, by the edge connector -PJB),  but the MCU controls the sound writes?
- steaser: sound uses an OkiM6295 (controlled by the sub MCU), check if it can be simulated;
- deucesw2: colour cycling effect on attract mode is ugly (background should be blue, it's instead a MAME-esque
  palette), protection?

*****************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mc6845.h"
#include "sound/dac.h"
#include "sound/saa1099.h"
#include "machine/nvram.h"
#include "video/ramdac.h"

class blitz68k_state : public driver_device
{
public:
	blitz68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_frame_buffer(*this, "frame_buffer"),
			m_blit_romaddr(*this, "blit_romaddr"),
			m_blit_attr1_ram(*this, "blit_attr1_ram"),
			m_blit_dst_ram_loword(*this, "blitram_loword"),
			m_blit_attr2_ram(*this, "blit_attr2_ram"),
			m_blit_dst_ram_hiword(*this, "blitram_hiword"),
			m_blit_vregs(*this, "blit_vregs"),
			m_blit_transpen(*this, "blit_transpen"),
			m_leds0(*this, "leds0"),
			m_leds1(*this, "leds1"),
			m_leds2(*this, "leds2") ,
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	optional_shared_ptr<UINT16> m_nvram;
	std::unique_ptr<UINT8[]> m_blit_buffer;
	optional_shared_ptr<UINT16> m_frame_buffer;
	optional_shared_ptr<UINT16> m_blit_romaddr;
	optional_shared_ptr<UINT16> m_blit_attr1_ram;
	optional_shared_ptr<UINT16> m_blit_dst_ram_loword;
	optional_shared_ptr<UINT16> m_blit_attr2_ram;
	optional_shared_ptr<UINT16> m_blit_dst_ram_hiword;
	optional_shared_ptr<UINT16> m_blit_vregs;
	optional_shared_ptr<UINT16> m_blit_transpen;
	optional_shared_ptr<UINT16> m_leds0;
	optional_shared_ptr<UINT16> m_leds1;
	optional_shared_ptr<UINT16> m_leds2;
	DECLARE_WRITE16_MEMBER(blit_copy_w);
	DECLARE_READ8_MEMBER(blit_status_r);
	DECLARE_WRITE8_MEMBER(blit_x_w);
	DECLARE_WRITE8_MEMBER(blit_y_w);
	DECLARE_WRITE8_MEMBER(blit_xy_w);
	DECLARE_WRITE8_MEMBER(blit_w_w);
	DECLARE_WRITE8_MEMBER(blit_h_w);
	DECLARE_WRITE8_MEMBER(blit_wh_w);
	DECLARE_WRITE8_MEMBER(blit_addr0_w);
	DECLARE_WRITE8_MEMBER(blit_addr1_w);
	DECLARE_WRITE8_MEMBER(blit_addr01_w);
	DECLARE_WRITE8_MEMBER(blit_addr2_w);
	DECLARE_WRITE8_MEMBER(blit_pens_w);
	DECLARE_WRITE8_MEMBER(blit_pen0_w);
	DECLARE_WRITE8_MEMBER(blit_pen1_w);
	DECLARE_WRITE8_MEMBER(blit_pen2_w);
	DECLARE_WRITE8_MEMBER(blit_pen3_w);
	DECLARE_WRITE8_MEMBER(blit_flag0_w);
	DECLARE_WRITE8_MEMBER(blit_flag1_w);
	DECLARE_WRITE8_MEMBER(blit_flipx_w);
	DECLARE_WRITE8_MEMBER(blit_flipy_w);
	DECLARE_WRITE8_MEMBER(blit_solid_w);
	DECLARE_WRITE8_MEMBER(blit_trans_w);
	DECLARE_WRITE8_MEMBER(blit_flag6_w);
	DECLARE_WRITE8_MEMBER(blit_flag7_w);
	DECLARE_WRITE8_MEMBER(blit_flags_w);
	DECLARE_WRITE8_MEMBER(blit_draw_w);
	DECLARE_WRITE8_MEMBER(blit_hwyxa_draw_w);
	DECLARE_READ16_MEMBER(blitter_status_r);
	DECLARE_WRITE16_MEMBER(lamps_w);
	DECLARE_READ16_MEMBER(test_r);
	DECLARE_READ8_MEMBER(bankrob_mcu1_r);
	DECLARE_READ8_MEMBER(bankrob_mcu2_r);
	DECLARE_READ8_MEMBER(bankrob_mcu_status_read_r);
	DECLARE_READ8_MEMBER(bankrob_mcu_status_write_r);
	DECLARE_WRITE8_MEMBER(bankrob_mcu1_w);
	DECLARE_WRITE8_MEMBER(bankrob_mcu2_w);
	DECLARE_READ8_MEMBER(bankroba_mcu1_r);
	DECLARE_READ8_MEMBER(bankroba_mcu2_r);
	DECLARE_READ8_MEMBER(bankroba_mcu1_status_write_r);
	DECLARE_READ8_MEMBER(bankroba_mcu2_status_write_r);
	DECLARE_WRITE8_MEMBER(bankroba_mcu1_w);
	DECLARE_WRITE8_MEMBER(bankroba_mcu2_w);
	DECLARE_WRITE16_MEMBER(cjffruit_leds1_w);
	DECLARE_WRITE16_MEMBER(cjffruit_leds2_w);
	DECLARE_WRITE16_MEMBER(cjffruit_leds3_w);
	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_READ16_MEMBER(cjffruit_mcu_r);
	DECLARE_WRITE16_MEMBER(cjffruit_mcu_w);
	DECLARE_READ16_MEMBER(deucesw2_mcu_r);
	DECLARE_WRITE16_MEMBER(deucesw2_mcu_w);
	DECLARE_WRITE16_MEMBER(deucesw2_leds1_w);
	DECLARE_WRITE16_MEMBER(deucesw2_leds2_w);
	DECLARE_WRITE16_MEMBER(deucesw2_leds3_w);
	DECLARE_READ8_MEMBER(dualgame_mcu1_r);
	DECLARE_READ8_MEMBER(dualgame_mcu2_r);
	DECLARE_READ8_MEMBER(dualgame_mcu_status_read_r);
	DECLARE_READ8_MEMBER(dualgame_mcu_status_write_r);
	DECLARE_WRITE8_MEMBER(dualgame_mcu1_w);
	DECLARE_WRITE8_MEMBER(dualgame_mcu2_w);
	DECLARE_READ16_MEMBER(hermit_mcu_r);
	DECLARE_WRITE16_MEMBER(hermit_mcu_w);
	DECLARE_WRITE16_MEMBER(hermit_leds1_w);
	DECLARE_WRITE16_MEMBER(hermit_leds2_w);
	DECLARE_READ16_MEMBER(hermit_track_r);
	DECLARE_READ8_MEMBER(maxidbl_mcu1_r);
	DECLARE_READ8_MEMBER(maxidbl_mcu2_r);
	DECLARE_READ8_MEMBER(maxidbl_mcu_status_read_r);
	DECLARE_READ8_MEMBER(maxidbl_mcu_status_write_r);
	DECLARE_WRITE8_MEMBER(maxidbl_mcu1_w);
	DECLARE_WRITE8_MEMBER(maxidbl_mcu2_w);
	void show_leds123();
	void show_leds12();
	DECLARE_WRITE16_MEMBER(crtc_lpen_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync_irq1);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync_irq3);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync_irq5);
	DECLARE_DRIVER_INIT(bankrob);
	DECLARE_DRIVER_INIT(cjffruit);
	DECLARE_DRIVER_INIT(deucesw2);
	DECLARE_DRIVER_INIT(megadble);
	DECLARE_DRIVER_INIT(bankroba);
	DECLARE_DRIVER_INIT(maxidbl);
	DECLARE_DRIVER_INIT(cj3play);
	DECLARE_DRIVER_INIT(megadblj);
	DECLARE_DRIVER_INIT(hermit);
	DECLARE_DRIVER_INIT(dualgame);
	DECLARE_VIDEO_START(blitz68k);
	DECLARE_VIDEO_START(blitz68k_addr_factor1);
	UINT32 screen_update_blitz68k(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_blitz68k_noblit(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(steaser_mcu_sim);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

/*************************************************************************************************************

    Video

*************************************************************************************************************/

struct blit_t
{
	UINT8 x, y;
	UINT8 w, h;
	UINT8 addr[3];
	UINT8 pen[4];
	UINT8 flag[8];
	UINT8 flipx, flipy;
	UINT8 solid;
	UINT8 trans;
	int addr_factor;
} blit;

VIDEO_START_MEMBER(blitz68k_state,blitz68k)
{
	m_blit_buffer = std::make_unique<UINT8[]>(512*256);
	blit.addr_factor = 2;
}

VIDEO_START_MEMBER(blitz68k_state,blitz68k_addr_factor1)
{
	VIDEO_START_CALL_MEMBER(blitz68k);
	blit.addr_factor = 1;
}

UINT32 blitz68k_state::screen_update_blitz68k(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;

	UINT8 *src = m_blit_buffer.get();

	for(y = 0; y < 256; y++)
	{
		for(x = 0; x < 512; x++)
		{
			bitmap.pix32(y, x) = m_palette->pen(*src++);
		}
	}

	return 0;
}

// Blitter-less board (SPI-68K)


UINT32 blitz68k_state::screen_update_blitz68k_noblit(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;

	UINT16 *src = m_frame_buffer;

	for(y = 0; y < 256; y++)
	{
		for(x = 0; x < 512; )
		{
			UINT16 pen = *src++;
			bitmap.pix32(y, x++) = m_palette->pen((pen >>  8) & 0xf);
			bitmap.pix32(y, x++) = m_palette->pen((pen >> 12) & 0xf);
			bitmap.pix32(y, x++) = m_palette->pen((pen >>  0) & 0xf);
			bitmap.pix32(y, x++) = m_palette->pen((pen >>  4) & 0xf);
		}
	}

	return 0;
}

/*************************************************************************************************************

    Blitter (ilpag, steaser)

    To do:
    - register names should be properly renamed
      "transpen" 8/2 is for layer clearance;
      "transpen" 10/2 is the trasparency pen;
      "vregs" are pen selects, they select the proper color to render, and are tied to the first three gfx offsets;
    - shrinking bit? (some pictures in steaser, "00" on top of ilpag)
    - draw direction (card choose in steaser, in service mode)
    - line draw? (for the "Game Over" msg in steaser)
    - "random" pens? (9d0000 read in steaser)
    - anything else?

*************************************************************************************************************/


WRITE16_MEMBER(blitz68k_state::blit_copy_w)
{
	UINT8 *blit_rom = memregion("blitter")->base();
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;
	int x,y,x_size,y_size;
	UINT32 src;

	logerror("blit copy %04x %04x %04x %04x %04x\n", m_blit_romaddr[0], m_blit_attr1_ram[0], m_blit_dst_ram_loword[0], m_blit_attr2_ram[0], m_blit_dst_ram_hiword[0] );
	logerror("blit vregs %04x %04x %04x %04x\n",m_blit_vregs[0/2],m_blit_vregs[2/2],m_blit_vregs[4/2],m_blit_vregs[6/2]);
	logerror("blit transpen %04x %04x %04x %04x %04x %04x %04x %04x\n",m_blit_transpen[0/2],m_blit_transpen[2/2],m_blit_transpen[4/2],m_blit_transpen[6/2],
																	m_blit_transpen[8/2],m_blit_transpen[10/2],m_blit_transpen[12/2],m_blit_transpen[14/2]);

	blit_dst_xpos = (m_blit_dst_ram_loword[0] & 0x00ff)*2;
	blit_dst_ypos = ((m_blit_dst_ram_loword[0] & 0xff00)>>8);

	y_size = (0x100-((m_blit_attr2_ram[0] & 0xff00)>>8));
	x_size = (m_blit_attr2_ram[0] & 0x00ff)*2;

	/* rounding around for 0 size */
	if(x_size == 0) { x_size = 0x200; }

	/* TODO: used by steaser "Game Over" msg on attract mode*/
//  if(y_size == 1) { y_size = 32; }

	src = m_blit_romaddr[0] | (m_blit_attr1_ram[0] & 0x1f00)<<8;
//  src|= (m_blit_transpen[0xc/2] & 0x0100)<<12;

	for(y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			int drawx = (blit_dst_xpos+x)&0x1ff;
			int drawy = (blit_dst_ypos+y)&0x0ff;
			if(m_blit_transpen[0x8/2] & 0x100)
				m_blit_buffer[drawy*512+drawx] = ((m_blit_vregs[0] & 0xf00)>>8);
			else
			{
				UINT8 pen_helper;

				pen_helper = blit_rom[src] & 0xff;
				if(m_blit_transpen[0xa/2] & 0x100) //pen is opaque register
				{
					if(pen_helper)
						m_blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((m_blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
				}
				else
					m_blit_buffer[drawy*512+drawx] = ((pen_helper & 0xff) <= 3) ? ((m_blit_vregs[pen_helper] & 0xf00)>>8) : blit_rom[src];
			}

			src++;
		}
	}
}

/*************************************************************************************************************

    Blitter (cjffruit)
    To do: merge with above

*************************************************************************************************************/

READ8_MEMBER(blitz68k_state::blit_status_r)
{
	return 0;   // bit 0 = blitter busy
}

WRITE8_MEMBER(blitz68k_state::blit_x_w)
{
	blit.x = data;
}
WRITE8_MEMBER(blitz68k_state::blit_y_w)
{
	blit.y = data;
}
WRITE8_MEMBER(blitz68k_state::blit_xy_w)
{
	if (offset)
		blit_x_w(space, offset, data);
	else
		blit_y_w(space, offset, data);
}


WRITE8_MEMBER(blitz68k_state::blit_w_w)
{
	blit.w = data;
}
WRITE8_MEMBER(blitz68k_state::blit_h_w)
{
	blit.h = data;
}
WRITE8_MEMBER(blitz68k_state::blit_wh_w)
{
	if (offset)
		blit_w_w(space, offset, data-1);
	else
		blit_h_w(space, offset, -data-1);
}


WRITE8_MEMBER(blitz68k_state::blit_addr0_w)
{
	blit.addr[0] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_addr1_w)
{
	blit.addr[1] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_addr01_w)
{
	if (offset)
		blit_addr0_w(space, offset, data);
	else
		blit_addr1_w(space, offset, data);
}
WRITE8_MEMBER(blitz68k_state::blit_addr2_w)
{
	blit.addr[2] = data;
}


WRITE8_MEMBER(blitz68k_state::blit_pens_w)
{
	blit.pen[offset] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_pen0_w)
{
	blit.pen[0] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_pen1_w)
{
	blit.pen[1] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_pen2_w)
{
	blit.pen[2] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_pen3_w)
{
	blit.pen[3] = data;
}


WRITE8_MEMBER(blitz68k_state::blit_flag0_w)
{
	blit.flag[0] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_flag1_w)
{
	blit.flag[1] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_flipx_w)
{
	blit.flipx = data;
}
WRITE8_MEMBER(blitz68k_state::blit_flipy_w)
{
	blit.flipy = data;
}
WRITE8_MEMBER(blitz68k_state::blit_solid_w)
{
	blit.solid = data;
}
WRITE8_MEMBER(blitz68k_state::blit_trans_w)
{
	blit.trans = data;
}
WRITE8_MEMBER(blitz68k_state::blit_flag6_w)
{
	blit.flag[6] = data;
}
WRITE8_MEMBER(blitz68k_state::blit_flag7_w)
{
	blit.flag[7] = data;
}

WRITE8_MEMBER(blitz68k_state::blit_flags_w)
{
	switch(offset)
	{
		case 0: blit_flipx_w(space, offset, data);  break;
		case 1: blit_flipy_w(space, offset, data);  break;
		case 2: blit_solid_w(space, offset, data);  break;
		case 3: blit_trans_w(space, offset, data);  break;
	}
}

WRITE8_MEMBER(blitz68k_state::blit_draw_w)
{
	UINT8 *blit_rom  = memregion("blitter")->base();
	int blit_romsize = memregion("blitter")->bytes();
	UINT32 blit_dst_xpos;
	UINT32 blit_dst_ypos;
	int x, y, x_size, y_size;
	UINT32 src;

	logerror("%s: blit x=%02x y=%02x w=%02x h=%02x addr=%02x%02x%02x pens=%02x %02x %02x %02x flag=%02x %02x %02x %02x - %02x %02x %02x %02x\n", machine().describe_context(),
				blit.x,  blit.y, blit.w, blit.h,
				blit.addr[2], blit.addr[1], blit.addr[0],
				blit.pen[0], blit.pen[1], blit.pen[2], blit.pen[3],
				blit.flag[0], blit.flag[1], blit.flipx, blit.flipy,     blit.solid, blit.trans, blit.flag[6], blit.flag[7]
	);

	x_size = (blit.w + 1) * 2;
	y_size = (blit.h + 1);

	blit_dst_ypos = (blit.y);
	blit_dst_xpos = (blit.x);

	blit_dst_xpos *= 2;

	src = (blit.addr[2] << 16) | (blit.addr[1] << 8) | blit.addr[0];

	UINT8 pen = 0;
	if (blit.solid)
	{
		pen = src & 0xff;
	}

	src *= blit.addr_factor;

	int flipx = (blit.flipx == 0);
	int flipy = (blit.flipy == 0);

	for (y = 0; y < y_size; y++)
	{
		for (x = 0; x < x_size; x++)
		{
			int drawx = (blit_dst_xpos + (flipx ? -x+1 : x)) & 0x1ff;
			int drawy = (blit_dst_ypos + (flipy ? -y+1 : y)) & 0x0ff;

			if (!blit.solid)
			{
				src %= blit_romsize;
				pen = blit_rom[src];
				src++;
			}

			if (pen || !blit.trans)
			{
				if (pen <= 3)
					pen = blit.pen[pen] & 0xf;

				m_blit_buffer[drawy * 512 + drawx] = pen;
			}
		}
	}

	// used by cjffruit in service mode (girl select screen)
	blit_dst_xpos += (flipx ?        +1 :      0);
	blit_dst_ypos += (flipy ? -y_size+1 : y_size);
	blit.x = blit_dst_xpos / 2;
	blit.y = blit_dst_ypos;
}

WRITE8_MEMBER(blitz68k_state::blit_hwyxa_draw_w)
{
	switch (offset)
	{
		case 0: blit_h_w        (space, offset, data);  break;
		case 1: blit_w_w        (space, offset, data);  break;
		case 2: blit_y_w        (space, offset, data);  break;
		case 3: blit_x_w        (space, offset, data);  break;
		case 4: blit_addr2_w    (space, offset, data);  break;
		case 5: blit_addr1_w    (space, offset, data);  break;
		case 6: blit_addr0_w    (space, offset, data);  break;
		case 7: blit_draw_w     (space, offset, data);  break;
	}
}

/*************************************************************************************************************
    Outputs
*************************************************************************************************************/

void blitz68k_state::show_leds123()
{
#ifdef MAME_DEBUG
	popmessage("led %02x %02x %02x", m_leds0[0]>>8, m_leds1[0]>>8, m_leds2[0]>>8);
#endif
}
void blitz68k_state::show_leds12()
{
#ifdef MAME_DEBUG
	popmessage("led %02x %02x", m_leds0[0]>>8, m_leds1[0]>>8);
#endif
}

/*************************************************************************************************************

    Memory Maps

*************************************************************************************************************/

/*bit 0 is the blitter busy flag*/
READ16_MEMBER(blitz68k_state::blitter_status_r)
{
	return 0;
}

/*TODO*/
WRITE16_MEMBER(blitz68k_state::lamps_w)
{
//  popmessage("%02x",data);
}

READ16_MEMBER(blitz68k_state::test_r)
{
	return 0xffff;//machine().rand();
}

#if 0
WRITE16_MEMBER(blitz68k_state::irq_callback_w)
{
//  popmessage("%02x",data);
	m_maincpu->set_input_line(3, HOLD_LINE );
}

WRITE16_MEMBER(blitz68k_state::sound_write_w)
{
	popmessage("%02x",data);
	dac_data_w(0, data & 0x0f);     /* Sound DAC? */
}
#endif

static ADDRESS_MAP_START( ilpag_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blitter", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_SHARE("nvram")

//  AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x880000, 0x880001) AM_READ(test_r)

	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0xff00 )
	AM_RANGE(0x900004, 0x900005) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_SHARE("blit_transpen") //video registers for the blitter write
	AM_RANGE(0x990000, 0x990007) AM_RAM AM_SHARE("blit_vregs") //pens
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_SHARE("blit_romaddr")
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_SHARE("blit_attr1_ram")
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_SHARE("blitram_loword")
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_SHARE("blit_attr2_ram")
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE(blit_copy_w ) AM_SHARE("blitram_hiword")
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)

	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xc00180, 0xc00181) AM_READ_PORT("IN2")
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
	AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( steaser_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM AM_REGION("blitter", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x800000, 0x800001) AM_READ(test_r)
//  AM_RANGE(0x840000, 0x840001) AM_WRITE(sound_write_w)
	AM_RANGE(0x880000, 0x880001) AM_READ(test_r)
//  AM_RANGE(0x8c0000, 0x8c0001) AM_WRITE(sound_write_w)

	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0xff00 )
	AM_RANGE(0x900004, 0x900005) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )
	AM_RANGE(0x940000, 0x940001) AM_WRITENOP //? Seems a dword write for some read, written consecutively
	AM_RANGE(0x980000, 0x98000f) AM_RAM AM_SHARE("blit_transpen")//probably transparency pens
	AM_RANGE(0x990000, 0x990005) AM_RAM AM_SHARE("blit_vregs")
	AM_RANGE(0x998000, 0x998001) AM_RAM AM_SHARE("blit_romaddr")
	AM_RANGE(0x9a0000, 0x9a0001) AM_RAM AM_SHARE("blit_attr1_ram")
	AM_RANGE(0x9a8000, 0x9a8001) AM_RAM AM_SHARE("blitram_loword")
	AM_RANGE(0x9b0000, 0x9b0001) AM_RAM AM_SHARE("blit_attr2_ram")
	AM_RANGE(0x9b8000, 0x9b8001) AM_RAM_WRITE(blit_copy_w ) AM_SHARE("blitram_hiword")
	AM_RANGE(0x9c0002, 0x9c0003) AM_READNOP //pen control?
	AM_RANGE(0x9d0000, 0x9d0001) AM_READNOP //?
	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)
	AM_RANGE(0x9f0000, 0x9f0001) AM_WRITENOP //???

//  AM_RANGE(0xc00000, 0xc00001) AM_WRITE(lamps_w)
	AM_RANGE(0xbd0000, 0xbd0001) AM_READ(test_r)
//  AM_RANGE(0xc00200, 0xc00201) AM_WRITE(sound_write_w)
//  AM_RANGE(0xc00380, 0xc00381) AM_READ_PORT("IN3")
//  AM_RANGE(0xc00300, 0xc00301) AM_WRITE(irq_callback_w)
ADDRESS_MAP_END

/*************************************************************************************************************
    Bank Robbery
*************************************************************************************************************/

// MCU simulation (to be done)
READ8_MEMBER(blitz68k_state::bankrob_mcu1_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu1 reads %02x\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(blitz68k_state::bankrob_mcu2_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu2 reads %02x\n", machine().describe_context(), ret);
	return ret;
}

READ8_MEMBER(blitz68k_state::bankrob_mcu_status_read_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

READ8_MEMBER(blitz68k_state::bankrob_mcu_status_write_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

WRITE8_MEMBER(blitz68k_state::bankrob_mcu1_w)
{
	logerror("%s: mcu1 written with %02x\n", machine().describe_context(), data);
}
WRITE8_MEMBER(blitz68k_state::bankrob_mcu2_w)
{
	logerror("%s: mcu2 written with %02x\n", machine().describe_context(), data);
}

static ADDRESS_MAP_START( bankrob_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM

	AM_RANGE(0x220000, 0x220001) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x220002, 0x220003) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0xff00 )

	AM_RANGE(0x240000, 0x240001) AM_WRITE8(blit_addr0_w, 0xff00)
	AM_RANGE(0x240002, 0x240003) AM_WRITE8(blit_addr1_w, 0xff00)
	AM_RANGE(0x240004, 0x240005) AM_WRITE8(blit_addr2_w, 0xff00)

	AM_RANGE(0x240006, 0x240007) AM_WRITE8(blit_x_w, 0xff00)
	AM_RANGE(0x240008, 0x240009) AM_WRITE8(blit_y_w, 0xff00)

	AM_RANGE(0x24000a, 0x24000b) AM_WRITE8(blit_w_w, 0xff00)
	AM_RANGE(0x24000c, 0x24000d) AM_WRITE8(blit_h_w, 0xff00)

	AM_RANGE(0x24000e, 0x24000f) AM_WRITE8(blit_draw_w, 0xff00)

	AM_RANGE(0x260000, 0x260001) AM_WRITE8(blit_pen0_w, 0xff00)
	AM_RANGE(0x260002, 0x260003) AM_WRITE8(blit_pen1_w, 0xff00)
	AM_RANGE(0x260004, 0x260005) AM_WRITE8(blit_pen2_w, 0xff00)
	AM_RANGE(0x260006, 0x260007) AM_WRITE8(blit_pen3_w, 0xff00)

	AM_RANGE(0x280000, 0x280001) AM_READ(blitter_status_r)

	AM_RANGE(0x2c0000, 0x2c0001) AM_WRITENOP    // 1->0

	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE8(blit_flag0_w, 0xff00)
	AM_RANGE(0x2e0002, 0x2e0003) AM_WRITE8(blit_flag1_w, 0xff00)
	AM_RANGE(0x2e0004, 0x2e0005) AM_WRITE8(blit_flipx_w, 0xff00)
	AM_RANGE(0x2e0006, 0x2e0007) AM_WRITE8(blit_flipy_w, 0xff00)
	AM_RANGE(0x2e0008, 0x2e0009) AM_WRITE8(blit_solid_w, 0xff00)
	AM_RANGE(0x2e000a, 0x2e000b) AM_WRITE8(blit_trans_w, 0xff00)
	AM_RANGE(0x2e000c, 0x2e000d) AM_WRITE8(blit_flag6_w, 0xff00)
	AM_RANGE(0x2e000e, 0x2e000f) AM_WRITE8(blit_flag7_w, 0xff00)

	AM_RANGE(0x300000, 0x300001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x300002, 0x300003) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0xff00 )
	AM_RANGE(0x300004, 0x300005) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )

	AM_RANGE(0x400000, 0x400001) AM_READ8(bankrob_mcu_status_write_r, 0x00ff)
	AM_RANGE(0x400002, 0x400003) AM_READ8(bankrob_mcu_status_read_r,  0x00ff)

	AM_RANGE(0x400004, 0x400005) AM_READWRITE8(bankrob_mcu1_r, bankrob_mcu1_w, 0x00ff)
	AM_RANGE(0x400006, 0x400007) AM_READWRITE8(bankrob_mcu2_r, bankrob_mcu2_w, 0xff00)

	AM_RANGE(0x800000, 0x800001) AM_DEVREADWRITE8("crtc", mc6845_device, status_r,   address_w,  0xff00)    // triggered by MCU?
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
ADDRESS_MAP_END

// bankroba:

// MCU simulation (to be done)
READ8_MEMBER(blitz68k_state::bankroba_mcu1_r)
{
	UINT8 ret = machine().rand();   // machine().rand() gives "interesting" results
	logerror("%s: mcu1 reads %02x\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(blitz68k_state::bankroba_mcu2_r)
{
	UINT8 ret = machine().rand();   // machine().rand() gives "interesting" results
	logerror("%s: mcu2 reads %02x\n", machine().describe_context(), ret);
	return ret;
}

READ8_MEMBER(blitz68k_state::bankroba_mcu1_status_write_r)
{
	return 0x00;    // bit 0 = MCU1. Active low.
}
READ8_MEMBER(blitz68k_state::bankroba_mcu2_status_write_r)
{
	return 0x01;    // bit 0 = MCU2. Active high.
}

WRITE8_MEMBER(blitz68k_state::bankroba_mcu1_w)
{
	logerror("%s: mcu1 written with %02x\n", machine().describe_context(), data);
}
WRITE8_MEMBER(blitz68k_state::bankroba_mcu2_w)
{
	logerror("%s: mcu2 written with %02x\n", machine().describe_context(), data);
}

static ADDRESS_MAP_START( bankroba_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x800000, 0x800001) AM_READ8(bankroba_mcu1_r, 0x00ff)  // lev 4
	AM_RANGE(0x840000, 0x840001) AM_WRITE8(bankroba_mcu1_w, 0x00ff)

	AM_RANGE(0x880000, 0x880001) AM_READ8(bankroba_mcu2_r, 0x00ff)  // lev 3
	AM_RANGE(0x8c0000, 0x8c0001) AM_WRITE8(bankroba_mcu2_w, 0x00ff)

	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0xff00 )
	AM_RANGE(0x900004, 0x900005) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )

//  AM_RANGE(0x940000, 0x940001) AM_WRITE   // lev 6

	AM_RANGE(0x980000, 0x980001) AM_WRITE8(blit_flag0_w, 0xff00)
	AM_RANGE(0x980002, 0x980003) AM_WRITE8(blit_flag1_w, 0xff00)
	AM_RANGE(0x980004, 0x980005) AM_WRITE8(blit_flipx_w, 0xff00)
	AM_RANGE(0x980006, 0x980007) AM_WRITE8(blit_flipy_w, 0xff00)
	AM_RANGE(0x980008, 0x980009) AM_WRITE8(blit_solid_w, 0xff00)
	AM_RANGE(0x98000a, 0x98000b) AM_WRITE8(blit_trans_w, 0xff00)
	AM_RANGE(0x98000c, 0x98000d) AM_WRITE8(blit_flag6_w, 0xff00)
	AM_RANGE(0x98000e, 0x98000f) AM_WRITE8(blit_flag7_w, 0xff00)

	AM_RANGE(0x990000, 0x990001) AM_WRITE8(blit_pen0_w, 0xff00)
	AM_RANGE(0x990002, 0x990003) AM_WRITE8(blit_pen1_w, 0xff00)
	AM_RANGE(0x990004, 0x990005) AM_WRITE8(blit_pen2_w, 0xff00)
	AM_RANGE(0x990006, 0x990007) AM_WRITE8(blit_pen3_w, 0xff00)

	AM_RANGE(0x998000, 0x998001) AM_WRITE8(blit_addr01_w, 0xffff)
	AM_RANGE(0x9a0000, 0x9a0001) AM_WRITE8(blit_addr2_w,  0xff00)

	AM_RANGE(0x9a8000, 0x9a8001) AM_WRITE8(blit_xy_w, 0xffff)

	AM_RANGE(0x9b0000, 0x9b0001) AM_WRITE8(blit_wh_w, 0xffff)

	AM_RANGE(0x9b8000, 0x9b8001) AM_WRITE8(blit_draw_w, 0x00ff)

	AM_RANGE(0x9c0000, 0x9c0001) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x9c0002, 0x9c0003) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0xff00 )

	AM_RANGE(0x9d0000, 0x9d0001) AM_READ8(bankroba_mcu1_status_write_r, 0xff00)

	AM_RANGE(0x9e0000, 0x9e0001) AM_READ(blitter_status_r)

	AM_RANGE(0x9f0000, 0x9f0001) AM_WRITENOP // 1

	AM_RANGE(0xbd0000, 0xbd0001) AM_READ8(bankroba_mcu2_status_write_r, 0xff00)

	// CRTC connected to MCU?
ADDRESS_MAP_END

/*************************************************************************************************************
    Funny Fruit
*************************************************************************************************************/

WRITE16_MEMBER(blitz68k_state::cjffruit_leds1_w)
{
	data = COMBINE_DATA(m_leds0);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x0100);    // coin in
		output().set_led_value(0, data & 0x0200);    // win???
//                                     1  data & 0x0400     // win???
		output().set_led_value(2, data & 0x0800);    // small
		output().set_led_value(3, data & 0x1000);    // big
		output().set_led_value(4, data & 0x2000);    // take
		output().set_led_value(5, data & 0x4000);    // double up
		output().set_led_value(6, data & 0x8000);    // cancel
		show_leds123();
	}
}

WRITE16_MEMBER(blitz68k_state::cjffruit_leds2_w)
{
	data = COMBINE_DATA(m_leds1);
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value( 7, data & 0x0100);   // start
		output().set_led_value( 8, data & 0x0200);   // bet
		output().set_led_value( 9, data & 0x0400);   // hold 5
		output().set_led_value(10, data & 0x0800);   // hold 4
		output().set_led_value(11, data & 0x1000);   // hold 3
		output().set_led_value(12, data & 0x2000);   // hold 2
		output().set_led_value(13, data & 0x4000);   // collect
		output().set_led_value(14, data & 0x8000);   // call attendant
		show_leds123();
	}
}

WRITE16_MEMBER(blitz68k_state::cjffruit_leds3_w)
{
	data = COMBINE_DATA(m_leds2);
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value(15, data & 0x0100);   // hopper coins?
		output().set_led_value(16, data & 0x0400);   // coin out?
		show_leds123();
	}
}

// CRTC
READ8_MEMBER(blitz68k_state::crtc_r)
{
	mc6845_device *mc6845 = machine().device<mc6845_device>("crtc");
	if (offset)
		return mc6845->register_r(space, 0);
	else
		return mc6845->status_r(space, 0);
}

WRITE8_MEMBER(blitz68k_state::crtc_w)
{
	mc6845_device *mc6845 = machine().device<mc6845_device>("crtc");
	if (offset)
		mc6845->register_w(space, 0, data);
	else
		mc6845->address_w(space, 0, data);
}

WRITE16_MEMBER(blitz68k_state::crtc_lpen_w)
{
	device_t *device = machine().device("crtc");
	// 8fe0006: 0->1
	if (ACCESSING_BITS_8_15 && (data & 0x0100))
		downcast<mc6845_device *>(device)->assert_light_pen_input();
	// 8fe0007: 1->0 (MCU irq?)
}

// MCU simulation (to be done)
READ16_MEMBER(blitz68k_state::cjffruit_mcu_r)
{
	UINT8 ret = 0x00;   // machine().rand() gives "interesting" results
	logerror("%s: mcu reads %02x\n", machine().describe_context(), ret);
	return ret << 8;
}

WRITE16_MEMBER(blitz68k_state::cjffruit_mcu_w)
{
	logerror("%s: mcu written with %02x\n", machine().describe_context(),data >> 8);
}

static ADDRESS_MAP_START( cjffruit_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x480000, 0x4807ff) AM_RAM

	AM_RANGE(0x820000, 0x820007) AM_WRITE8(blit_hwyxa_draw_w, 0xffff)

	AM_RANGE(0x850000, 0x850001) AM_READ(cjffruit_mcu_r )

	AM_RANGE(0x870000, 0x870001) AM_READ_PORT("IN0")
	AM_RANGE(0x872000, 0x872001) AM_READ_PORT("IN1")
	AM_RANGE(0x874000, 0x874001) AM_READ_PORT("IN2")
	AM_RANGE(0x876000, 0x876001) AM_READ_PORT("DSW")

	AM_RANGE(0x880000, 0x880001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x880000, 0x880001) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0x00ff )
	AM_RANGE(0x880002, 0x880003) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )
	AM_RANGE(0x880000, 0x880001) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x880000, 0x880001) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0x00ff )

	AM_RANGE(0x8a0000, 0x8a0007) AM_WRITE8(blit_hwyxa_draw_w, 0xffff)

	AM_RANGE(0x8b0000, 0x8b0003) AM_WRITE8(blit_pens_w, 0xffff)

	AM_RANGE(0x8e0000, 0x8e0001) AM_WRITE(cjffruit_mcu_w )

	AM_RANGE(0x8f8000, 0x8f8001) AM_WRITE(cjffruit_leds1_w) AM_SHARE("leds0")
	AM_RANGE(0x8fa000, 0x8fa001) AM_WRITE(cjffruit_leds2_w) AM_SHARE("leds1")
	AM_RANGE(0x8fc000, 0x8fc001) AM_WRITE(cjffruit_leds3_w) AM_SHARE("leds2")

	AM_RANGE(0x8fe000, 0x8fe003) AM_WRITE8(blit_flags_w, 0xffff)    // flipx,y,solid,trans
	AM_RANGE(0x8fe004, 0x8fe005) AM_WRITEONLY
	AM_RANGE(0x8fe006, 0x8fe007) AM_WRITE(crtc_lpen_w)  // 0x8fe006: 0->1, 0x8fe007: 1->0

	AM_RANGE(0xc40000, 0xc40001) AM_READWRITE8(crtc_r, crtc_w, 0xffff)
ADDRESS_MAP_END

/*************************************************************************************************************
    Deuces Wild 2
*************************************************************************************************************/

// MCU simulation (to be done)
READ16_MEMBER(blitz68k_state::deucesw2_mcu_r)
{
	UINT8 ret = 0x00;   // machine().rand() gives "interesting" results
	logerror("%s: mcu reads %02x\n", machine().describe_context(), ret);
	return ret << 8;
}

WRITE16_MEMBER(blitz68k_state::deucesw2_mcu_w)
{
	logerror("%s: mcu written with %02x\n", machine().describe_context(),data >> 8);
}

WRITE16_MEMBER(blitz68k_state::deucesw2_leds1_w)
{
	data = COMBINE_DATA(m_leds0);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x0100);    // coin in
		output().set_led_value(0, data & 0x0200);    // win???
//                                     1  data & 0x0400     // win???
		output().set_led_value(2, data & 0x0800);    // small
		output().set_led_value(3, data & 0x1000);    // big
		output().set_led_value(4, data & 0x2000);    // take
		output().set_led_value(5, data & 0x4000);    // double up
		output().set_led_value(6, data & 0x8000);    // cancel
		show_leds123();
	}
}

WRITE16_MEMBER(blitz68k_state::deucesw2_leds2_w)
{
	data = COMBINE_DATA(m_leds1);
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value( 7, data & 0x0100);   // start
		output().set_led_value( 8, data & 0x0200);   // bet
		output().set_led_value( 9, data & 0x0400);   // hold 5
		output().set_led_value(10, data & 0x0800);   // hold 4
		output().set_led_value(11, data & 0x1000);   // hold 3
		output().set_led_value(12, data & 0x2000);   // hold 2
		output().set_led_value(13, data & 0x4000);   // hold 1
		output().set_led_value(14, data & 0x8000);   // call attendant
		show_leds123();
	}
}

WRITE16_MEMBER(blitz68k_state::deucesw2_leds3_w)
{
	data = COMBINE_DATA(m_leds2);
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value(15, data & 0x0100);   // hopper coins?
		output().set_led_value(16, data & 0x0400);   // coin out?
		show_leds123();
	}
}

static ADDRESS_MAP_START( deucesw2_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x800000, 0x800007) AM_WRITE8(blit_hwyxa_draw_w, 0xffff)

	AM_RANGE(0x812000, 0x812001) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x812000, 0x812001) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0x00ff )

	AM_RANGE(0x830000, 0x830001) AM_READ(deucesw2_mcu_r )

	AM_RANGE(0x840000, 0x840001) AM_READ_PORT("IN0")
	AM_RANGE(0x850000, 0x850001) AM_READ_PORT("IN1")
	AM_RANGE(0x860000, 0x860001) AM_READ_PORT("IN2")
	AM_RANGE(0x870000, 0x870001) AM_READ_PORT("DSW")

	AM_RANGE(0x880000, 0x880007) AM_WRITE8(blit_hwyxa_draw_w, 0xffff)

	AM_RANGE(0x890000, 0x890001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x890000, 0x890001) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0x00ff )
	AM_RANGE(0x890002, 0x890003) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )

	AM_RANGE(0x894000, 0x894003) AM_WRITE8(blit_pens_w, 0xffff)

	AM_RANGE(0x896000, 0x896001) AM_WRITE(deucesw2_mcu_w )

	AM_RANGE(0x898000, 0x898001) AM_WRITE(deucesw2_leds1_w) AM_SHARE("leds0")
	AM_RANGE(0x89a000, 0x89a001) AM_WRITE(deucesw2_leds2_w) AM_SHARE("leds1")
	AM_RANGE(0x89c000, 0x89c001) AM_WRITE(deucesw2_leds3_w) AM_SHARE("leds2")

	AM_RANGE(0x89e000, 0x89e003) AM_WRITE8(blit_flags_w, 0xffff)    // flipx,y,solid,trans
	AM_RANGE(0x89e004, 0x89e005) AM_WRITEONLY
	AM_RANGE(0x89e006, 0x89e007) AM_WRITE(crtc_lpen_w)  // 0x89e006: 0->1, 0x89e007: 1->0

	AM_RANGE(0xc00000, 0xc00001) AM_READWRITE8(crtc_r, crtc_w, 0xffff)
ADDRESS_MAP_END

/*************************************************************************************************************
    Dual Games
*************************************************************************************************************/

// MCU simulation (to be done)
READ8_MEMBER(blitz68k_state::dualgame_mcu1_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu1 reads %02x\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(blitz68k_state::dualgame_mcu2_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu2 reads %02x\n", machine().describe_context(), ret);
	return ret;
}

READ8_MEMBER(blitz68k_state::dualgame_mcu_status_read_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

READ8_MEMBER(blitz68k_state::dualgame_mcu_status_write_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

WRITE8_MEMBER(blitz68k_state::dualgame_mcu1_w)
{
	logerror("%s: mcu1 written with %02x\n", machine().describe_context(), data);
}
WRITE8_MEMBER(blitz68k_state::dualgame_mcu2_w)
{
	logerror("%s: mcu2 written with %02x\n", machine().describe_context(), data);
}

static ADDRESS_MAP_START( dualgame_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM

	AM_RANGE(0x220002, 0x220003) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x220002, 0x220003) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0x00ff )

	AM_RANGE(0x240000, 0x240001) AM_WRITE8(blit_addr0_w, 0xff00)
	AM_RANGE(0x240002, 0x240003) AM_WRITE8(blit_addr1_w, 0xff00)
	AM_RANGE(0x240004, 0x240005) AM_WRITE8(blit_addr2_w, 0xff00)

	AM_RANGE(0x240006, 0x240007) AM_WRITE8(blit_x_w, 0xff00)
	AM_RANGE(0x240008, 0x240009) AM_WRITE8(blit_y_w, 0xff00)

	AM_RANGE(0x24000a, 0x24000b) AM_WRITE8(blit_w_w, 0xff00)
	AM_RANGE(0x24000c, 0x24000d) AM_WRITE8(blit_h_w, 0xff00)

	AM_RANGE(0x24000e, 0x24000f) AM_WRITE8(blit_draw_w, 0xff00)

	AM_RANGE(0x260000, 0x260001) AM_WRITE8(blit_pen0_w, 0xff00)
	AM_RANGE(0x260002, 0x260003) AM_WRITE8(blit_pen1_w, 0xff00)
	AM_RANGE(0x260004, 0x260005) AM_WRITE8(blit_pen2_w, 0xff00)
	AM_RANGE(0x260006, 0x260007) AM_WRITE8(blit_pen3_w, 0xff00)

	AM_RANGE(0x280000, 0x280001) AM_READ8(blit_status_r, 0xff00)

	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(crtc_lpen_w)
	AM_RANGE(0x2a0000, 0x2a0001) AM_READNOP

	AM_RANGE(0x2c0000, 0x2c0001) AM_WRITENOP    // 1->0 (MCU related?)

	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE8(blit_flag0_w, 0xff00)
	AM_RANGE(0x2e0002, 0x2e0003) AM_WRITE8(blit_flag1_w, 0xff00)
	AM_RANGE(0x2e0004, 0x2e0005) AM_WRITE8(blit_flipx_w, 0xff00)    // flipx
	AM_RANGE(0x2e0006, 0x2e0007) AM_WRITE8(blit_flipy_w, 0xff00)    // flipy
	AM_RANGE(0x2e0008, 0x2e0009) AM_WRITE8(blit_solid_w, 0xff00)    // solid
	AM_RANGE(0x2e000a, 0x2e000b) AM_WRITE8(blit_trans_w, 0xff00)    // transparency
	AM_RANGE(0x2e000c, 0x2e000d) AM_WRITE8(blit_flag6_w, 0xff00)
	AM_RANGE(0x2e000e, 0x2e000f) AM_WRITE8(blit_flag7_w, 0xff00)

	AM_RANGE(0x300000, 0x300001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x300002, 0x300003) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0xff00 )
	AM_RANGE(0x300004, 0x300005) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )

	AM_RANGE(0x400000, 0x400001) AM_READ8(dualgame_mcu_status_write_r, 0x00ff)
	AM_RANGE(0x400002, 0x400003) AM_READ8(dualgame_mcu_status_read_r,  0x00ff)

	AM_RANGE(0x400004, 0x400005) AM_READWRITE8(dualgame_mcu1_r, dualgame_mcu1_w, 0x00ff)
	AM_RANGE(0x400006, 0x400007) AM_READWRITE8(dualgame_mcu2_r, dualgame_mcu2_w, 0xff00)

	AM_RANGE(0x800000, 0x800001) AM_DEVREADWRITE8("crtc", mc6845_device, status_r,   address_w,  0xff00)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
ADDRESS_MAP_END

/*************************************************************************************************************
    The Hermit
*************************************************************************************************************/

// MCU simulation (to be done)
READ16_MEMBER(blitz68k_state::hermit_mcu_r)
{
	UINT8 ret = 0x00;   // machine().rand() gives "interesting" results
	logerror("%s: mcu reads %02x\n", machine().describe_context(), ret);
	return ret << 8;
}

WRITE16_MEMBER(blitz68k_state::hermit_mcu_w)
{
	logerror("%s: mcu written with %02x\n", machine().describe_context(),data >> 8);
}

WRITE16_MEMBER(blitz68k_state::hermit_leds1_w)
{
	data = COMBINE_DATA(m_leds0);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x0100);    // coin in
		show_leds12();
	}
}

WRITE16_MEMBER(blitz68k_state::hermit_leds2_w)
{
	data = COMBINE_DATA(m_leds1);
	if (ACCESSING_BITS_8_15)
	{
		output().set_led_value( 7, data & 0x0100);   // button
		show_leds12();
	}
}

READ16_MEMBER(blitz68k_state::hermit_track_r)
{
#ifdef MAME_DEBUG
//  popmessage("track %02x %02x", ioport("TRACK_X")->read(), ioport("TRACK_Y")->read());
#endif

	return
		((0xf - ((ioport("TRACK_Y")->read() + 0x7) & 0xf)) << 12) |
		((0xf - ((ioport("TRACK_X")->read() + 0x7) & 0xf)) << 8)  ;
}

static ADDRESS_MAP_START( hermit_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x800000, 0x800001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x800000, 0x800001) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0x00ff )
	AM_RANGE(0x800002, 0x800003) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )
	AM_RANGE(0x840000, 0x840001) AM_DEVWRITE8("ramdac",ramdac_device, index_w, 0xff00 )
	AM_RANGE(0x840000, 0x840001) AM_DEVWRITE8("ramdac",ramdac_device, pal_w, 0x00ff )
	AM_RANGE(0x840002, 0x840003) AM_DEVWRITE8("ramdac",ramdac_device, mask_w, 0xff00 )
	AM_RANGE(0x840000, 0x840001) AM_DEVREAD8("ramdac",ramdac_device, index_r, 0xff00 )
	AM_RANGE(0x840000, 0x840001) AM_DEVREAD8("ramdac",ramdac_device, pal_r, 0x00ff )


	AM_RANGE(0x8c0000, 0x8c0003) AM_WRITE8(blit_pens_w, 0xffff )

	AM_RANGE(0x940000, 0x940001) AM_READ(hermit_mcu_r )
	AM_RANGE(0x980000, 0x980001) AM_WRITE(hermit_mcu_w )

	AM_RANGE(0x9c0000, 0x9c0001) AM_READ_PORT("IN0")
	AM_RANGE(0x9c8000, 0x9c8001) AM_READ(hermit_track_r )
	AM_RANGE(0x9d0000, 0x9d0001) AM_READ_PORT("IN2")
	AM_RANGE(0x9d8000, 0x9d8001) AM_READ_PORT("DSW")

	AM_RANGE(0x9e0000, 0x9e0001) AM_WRITE(hermit_leds1_w) AM_SHARE("leds0")
	AM_RANGE(0x9e8000, 0x9e8001) AM_WRITE(hermit_leds2_w) AM_SHARE("leds1")

	AM_RANGE(0x9f0000, 0x9f0003) AM_WRITE8(blit_flags_w, 0xffff)    // flipx,y,solid,trans
	AM_RANGE(0x9f0004, 0x9f0005) AM_WRITEONLY
	AM_RANGE(0x9f0006, 0x9f0007) AM_WRITE(crtc_lpen_w)  // 0x9f0006: 0->1, 0x9f0007: 1->0

	AM_RANGE(0xb00000, 0xb00001) AM_READWRITE8(crtc_r, crtc_w, 0xffff)  // triggered by MCU?

	AM_RANGE(0xc80000, 0xc80007) AM_WRITE8(blit_hwyxa_draw_w, 0xffff)
ADDRESS_MAP_END

/*************************************************************************************************************
    Maxi Double Poker
*************************************************************************************************************/

// MCU simulation (to be done)
READ8_MEMBER(blitz68k_state::maxidbl_mcu1_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu1 reads %02x\n", machine().describe_context(), ret);
	return ret;
}
READ8_MEMBER(blitz68k_state::maxidbl_mcu2_r)
{
	UINT8 ret = 0;  // machine().rand() gives "interesting" results
	logerror("%s: mcu2 reads %02x\n", machine().describe_context(), ret);
	return ret;
}

READ8_MEMBER(blitz68k_state::maxidbl_mcu_status_read_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

READ8_MEMBER(blitz68k_state::maxidbl_mcu_status_write_r)
{
	return 0x03;    // bit 0 = MCU1, bit 1 = MCU2. Active high.
}

WRITE8_MEMBER(blitz68k_state::maxidbl_mcu1_w)
{
	logerror("%s: mcu1 written with %02x\n", machine().describe_context(), data);
}
WRITE8_MEMBER(blitz68k_state::maxidbl_mcu2_w)
{
	logerror("%s: mcu2 written with %02x\n", machine().describe_context(), data);
}

static ADDRESS_MAP_START( maxidbl_map, AS_PROGRAM, 16, blitz68k_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_SHARE("frame_buffer")

	AM_RANGE(0x30000c, 0x30000d) AM_WRITENOP    // 0->1 (IRQ3 ack.?)
	AM_RANGE(0x30000e, 0x30000f) AM_WRITENOP    // 1->0 (MCU related?)

	AM_RANGE(0x500000, 0x500001) AM_READ8(maxidbl_mcu_status_write_r, 0x00ff)
	AM_RANGE(0x500002, 0x500003) AM_READ8(maxidbl_mcu_status_read_r,  0x00ff)

	AM_RANGE(0x500004, 0x500005) AM_READWRITE8(maxidbl_mcu1_r, maxidbl_mcu1_w, 0x00ff)
	AM_RANGE(0x500006, 0x500007) AM_READWRITE8(maxidbl_mcu2_r, maxidbl_mcu2_w, 0xff00)

	AM_RANGE(0x600000, 0x600001) AM_DEVREADWRITE8("crtc", mc6845_device, status_r,   address_w,  0xff00)    // triggered by MCU?
	AM_RANGE(0x600002, 0x600003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
ADDRESS_MAP_END



/*************************************************************************************************************

    Inputs

*************************************************************************************************************/

static INPUT_PORTS_START( bankrob )
	// not hooked up
	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_DIPNAME( 0x0200, 0x0200, "Clock?" )                PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "10 MHz" )
	PORT_DIPSETTING(      0x0200, "11 MHz" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cjffruit )
	// Inputs for L74 pinout
	PORT_START("IN0")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1        ) // coin 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2        ) // coin 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1     ) PORT_NAME("Recall")
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW     ) // menu
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK  ) // stats
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_POKER_HOLD1  ) // hold 1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET   ) // bet
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL  ) // start / deal

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   ) // take
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) // double up
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) // small (show pay table)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) // big
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_CANCEL  ) // cancel
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // collect
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) // hold 2
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) // hold 3

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_POKER_HOLD4 ) // hold 4
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_POKER_HOLD5 ) // hold 5
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // hopper coin
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_DOOR ) PORT_NAME("Cash Door") PORT_CODE(KEYCODE_D) PORT_TOGGLE
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // blitter busy
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // 5] 0 = mcu response ready
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // 6] 0 = mcu busy
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SERVICE3    ) PORT_NAME("Call Attendant")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_DIPNAME( 0x0e00, 0x0800, "Pinout" )                PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0e00, "8L6 (Cherry Master)" )
	PORT_DIPSETTING(      0x0c00, "8L7"  )
	PORT_DIPSETTING(      0x0a00, "8L10" )
	PORT_DIPSETTING(      0x0800, "L74 (Funny Fruit)"   )
	PORT_DIPSETTING(      0x0600, "CYB"  )
	PORT_DIPSETTING(      0x0400, "PMMG" )
	PORT_DIPSETTING(      0x0200, "Invalid #1" )
	PORT_DIPSETTING(      0x0000, "Invalid #2" )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Logic Door") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_DIPNAME( 0x2000, 0x2000, "Factory Default" )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Main Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( deucesw2 )
	// Inputs for L74 pinout
	PORT_START("IN0")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5)    // coin 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5)    // coin 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3         ) PORT_IMPULSE(5)    // coin 3
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW      ) // menu
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   ) // stats
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // collect
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET    ) // bet
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL   ) // start / deal

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   ) // take
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) // double up
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) // small
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) // big
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_CANCEL  ) // cancel
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) // hold 1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) // hold 2
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) // hold 3

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_POKER_HOLD4 ) // hold 4
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_POKER_HOLD5 ) // hold 5
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // hopper coin
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_DOOR ) PORT_NAME("Cash Door") PORT_CODE(KEYCODE_D) PORT_TOGGLE
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // blitter busy
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // 5] 0 = mcu response ready
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // 6] 0 = mcu busy
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SERVICE3    ) PORT_NAME("Call Attendant")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Logic Door") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Main Door")  PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_DIPNAME( 0x3800, 0x2000, "Pinout" )                PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x3800, "8L6 (Cherry Master)" )
	PORT_DIPSETTING(      0x3000, "8L7"   )
	PORT_DIPSETTING(      0x2800, "8L10"  )
	PORT_DIPSETTING(      0x2000, "L74"   )
	PORT_DIPSETTING(      0x1800, "Invalid #1" )
	PORT_DIPSETTING(      0x1000, "Invalid #2" )
	PORT_DIPSETTING(      0x0800, "Invalid #3" )
	PORT_DIPSETTING(      0x0000, "Invalid #4" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Factory Default" )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dualgame )
	// not hooked up
	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_DIPNAME( 0x0200, 0x0200, "Clock?" )                PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "10 MHz" )
	PORT_DIPSETTING(      0x0200, "11 MHz" )
	PORT_DIPNAME( 0x0400, 0x0400, "V-Sync" )                PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "V-" )
	PORT_DIPSETTING(      0x0400, "V+" )
	PORT_DIPNAME( 0x0800, 0x0800, "H-Sync" )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, "H-" )
	PORT_DIPSETTING(      0x0800, "H+" )
INPUT_PORTS_END

static INPUT_PORTS_START( hermit )
	PORT_START("IN0")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1        ) PORT_IMPULSE(5) // coin 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN      ) // -
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN      ) // -
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW     ) // menu
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK  ) // stats
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN      ) // -
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN      ) // -
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1      ) // -

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN     ) // -
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN     ) // -
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN     ) // -
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_UNKNOWN     ) // -
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // blitter busy
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL     ) // 5] 0 = mcu response ready
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_SPECIAL     ) // 6] 0 = mcu busy
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN     ) // -

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_DIPNAME( 0x0200, 0x0200, "Clock?" )                PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "20 MHz" )
	PORT_DIPSETTING(      0x0200, "22 MHz" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Trackball Alt #1" )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Trackball Alt #2" )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Factory Default" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("TRACK_X")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_RESET

	PORT_START("TRACK_Y")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_RESET
INPUT_PORTS_END

static INPUT_PORTS_START( ilpag )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Gioco (Bet)")
	PORT_DIPNAME( 0x0020, 0x0020, "IN3" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( maxidbl )
	// not hooked up
	PORT_START("DSW_SUB")
	PORT_DIPNAME( 0x0100, 0x0100, "V-Sync" )                PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "V-" )
	PORT_DIPSETTING(      0x0100, "V+" )
	PORT_DIPNAME( 0x0200, 0x0200, "H-Sync" )                PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "H-" )
	PORT_DIPSETTING(      0x0800, "H+" )
	PORT_DIPNAME( 0x0400, 0x0400, "Comp" )                  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "En-V" )                  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0100, 0x0100, "Screen Refresh" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, "50 Hz" )
	PORT_DIPSETTING(      0x0100, "60 Hz" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( steaser )
	PORT_START("MENU")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)

	PORT_START("STAT")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_IMPULSE(1)

	PORT_START("BET_DEAL")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_IMPULSE(1)

	PORT_START("TAKE_DOUBLE")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score") PORT_IMPULSE(1)

	PORT_START("SMALL_BIG")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Big") PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) PORT_NAME("Small") PORT_IMPULSE(1)

	PORT_START("CANCEL_HOLD1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_IMPULSE(1)

	PORT_START("HOLD2_HOLD3")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_IMPULSE(1)

	PORT_START("HOLD4_HOLD5")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_IMPULSE(1)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_IMPULSE(1)
INPUT_PORTS_END

/*************************************************************************************************************

    Machine Drivers

*************************************************************************************************************/

// R6845AP used for video sync signals only

WRITE_LINE_MEMBER(blitz68k_state::crtc_vsync_irq1)
{
	m_maincpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(blitz68k_state::crtc_vsync_irq3)
{
	m_maincpu->set_input_line(3, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(blitz68k_state::crtc_vsync_irq5)
{
	m_maincpu->set_input_line(5, state ? ASSERT_LINE : CLEAR_LINE);
}

MC6845_ON_UPDATE_ADDR_CHANGED(blitz68k_state::crtc_addr)
{
}

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, blitz68k_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( ilpag, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, 11059200 )  // ?
	MCFG_CPU_PROGRAM_MAP(ilpag_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state, irq4_line_hold) //3 & 6 used, mcu comms?

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)

	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/*
20089f = 1 -> menu
2008a0 = 1 -> stat
2008a1 = 1 -> (unused)
2008a2 = 1 -> bet
2008a3 = 1 -> deal
2008a4 = 1 -> take
2008a5 = 1 -> double
2008a6 = 1 -> small
2008a7 = 1 -> big
2008a8 = 1 -> cancel
2008a9 = 1 -> hold 1
2008aa = 1 -> hold 2
2008ab = 1 -> hold 3
2008ac = 1 -> hold 4
2008ad = 1 -> hold 5
*/

TIMER_DEVICE_CALLBACK_MEMBER(blitz68k_state::steaser_mcu_sim)
{
//  static int i;
	/*first off, signal the "MCU is running" flag*/
	m_nvram[0x932/2] = 0xffff;
	/*clear the inputs (they are impulsed)*/
//  for(i=0;i<8;i+=2)
//      m_nvram[((0x8a0)+i)/2] = 0;
	/*finally, read the inputs*/
	m_nvram[0x89e/2] = ioport("MENU")->read() & 0xffff;
	m_nvram[0x8a0/2] = ioport("STAT")->read() & 0xffff;
	m_nvram[0x8a2/2] = ioport("BET_DEAL")->read() & 0xffff;
	m_nvram[0x8a4/2] = ioport("TAKE_DOUBLE")->read() & 0xffff;
	m_nvram[0x8a6/2] = ioport("SMALL_BIG")->read() & 0xffff;
	m_nvram[0x8a8/2] = ioport("CANCEL_HOLD1")->read() & 0xffff;
	m_nvram[0x8aa/2] = ioport("HOLD2_HOLD3")->read() & 0xffff;
	m_nvram[0x8ac/2] = ioport("HOLD4_HOLD5")->read() & 0xffff;
}


static MACHINE_CONFIG_DERIVED( steaser, ilpag )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(steaser_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state, irq5_line_hold) //3, 4 & 6 used, mcu comms?

	MCFG_TIMER_DRIVER_ADD_PERIODIC("coinsim", blitz68k_state, steaser_mcu_sim, attotime::from_hz(10000))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cjffruit, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22_1184MHz/2)
	MCFG_CPU_PROGRAM_MAP(cjffruit_map)

	// MC68HC705C8P (Sound MCU)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", R6545_1, "screen", XTAL_22_1184MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq1))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bankrob, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_11_0592MHz)
	MCFG_CPU_PROGRAM_MAP(bankrob_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state,  irq3_line_hold)   // protection prevents correct irq frequency by crtc
	// irq 2 reads from MCUs

	// MC68HC705C8P (MCU1)

	// MC68HC705C8P (MCU2)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+4, 256-1-4)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_11_0592MHz/4)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq3))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bankroba, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_11_0592MHz )
	MCFG_CPU_PROGRAM_MAP(bankroba_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state,  irq5_line_hold)   // protection prevents correct irq frequency by crtc
	// irq 3,4 read from MCUs

	// MC68HC705C8P (MCU)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+7, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_11_0592MHz/4)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq5))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k_addr_factor1)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( deucesw2, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22_1184MHz / 2)
	MCFG_CPU_PROGRAM_MAP(deucesw2_map)
	// irq 2 reads from MCUs

	// MC68HC705C8P (MCU)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", R6545_1, "screen", XTAL_22_1184MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq3))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( dualgame, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_11_0592MHz )
	MCFG_CPU_PROGRAM_MAP(dualgame_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state,  irq2_line_hold) // lev 2 = MCUs, lev 3 = vblank

	// MC68HC705C8P (MCU1)

	// MC68HC705C8P (MCU2)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+4, 256-1-4)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_11_0592MHz/4)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq3))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hermit, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22_1184MHz/2 )
	MCFG_CPU_PROGRAM_MAP(hermit_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state,  irq1_line_hold)   // protection prevents correct irq frequency by crtc

	// MC68HC705C8P (MCU)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+4, 256-1-4)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k)

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_22_1184MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq1))

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_VIDEO_START_OVERRIDE(blitz68k_state,blitz68k)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( maxidbl, blitz68k_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_11_0592MHz)
	MCFG_CPU_PROGRAM_MAP(maxidbl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz68k_state,  irq3_line_hold)   // protection prevents correct irq frequency by crtc
	// irq 2 reads from MCUs

	// MC68HC705C8P (MCU1)

	// MC68HC705C8P (MCU2)

	// MC68HC705C8P (MCU3)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz68k_state, screen_update_blitz68k_noblit)

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_11_0592MHz/4)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)
	MCFG_MC6845_ADDR_CHANGED_CB(blitz68k_state, crtc_addr)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(blitz68k_state, crtc_vsync_irq3))

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SAA1099_ADD("saa", XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************************************************************************************

    ROMs Loading

*************************************************************************************************************/

/*************************************************************************************************************

Bank Robbery (Ver. 3.32)
(c) 1993 Entertainment Technology Corp.
(BLITZ SYSTEM INC April 1995 in the ROMs)

Main Board -------------------

  MPI-68K REV-0 LEV-1
  Made in CANADA
  Copyright 1991, Blitz System Inc.

CPUs:
  MC68HC000P12 (Main CPU)
  Osc. 11.0592 MHz
  2 x MC68HC705C8A (MCUs, internal ROMs not dumped)
  Osc. 4.000? MHz

Video:
  HD46505SP-2 HD68B45SP (CRT Controller)
  Bt471KPJ80 (RAMDAC)

ROMs:
  2 x 27C040 (Gfx)
  2 x 27C010 (Code)
  2 Unpop. Socket (EPROM #E, #F)

Other:
  2 x RS-232 (3 wire, labeled COM1 and COM2)
  MAX232CPE
  Flat cable connector (Hopper Board?)
  10 pin connector (Sub Board)
  Jumpers for ROMs size
  DSW4
  3.6V Lithium battery

Sub Board --------------------

  Unknown

*************************************************************************************************************/

ROM_START( bankrob )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "unknown_label.u21", 0x00000, 0x20000, CRC(a043a651) SHA1(798f7b7b04bf6ef5333b07d329fadc0264da00e9) )
	ROM_LOAD16_BYTE( "unknown_label.u20", 0x00001, 0x20000, CRC(31dcfd41) SHA1(d23f6a6d57d917ba1c4e202c341b35ab0eeaef42) )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "xpi-1_2.6.u6", 0x0000, 0x2000, NO_DUMP ) // missing label

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "xpi-2_2.6.u7", 0x0000, 0x2000, NO_DUMP ) // "for SPI & MPI 06/08/1995"

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "unknown_label.u70", 0x00000, 0x80000, CRC(35225bf6) SHA1(cd3176ab43c0678c6b9a92b9fafea116babdd534) )
	ROM_LOAD16_BYTE( "unknown_label.u54", 0x00001, 0x80000, CRC(c7c0c2d1) SHA1(3b3b6954fbded65418492374aaa94e3c60af69c5) )

//  ROM_REGION( 0x20000, "samples", 0 ) // 8 bit unsigned

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "palce16v8h.u10", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u18", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u19", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u33", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u34", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u39", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u60", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Bank Robbery (Ver. 2.00)
(c) 1993 Entertainment Technology Corp.
(BLITZ SYSTEM INC MAY 10TH, 1993 in the ROMs)

Main Board -------------------

  DK-B REV-B LEV-2
  Made in CANADA
  Copyright 1991 Blitz System inc.

CPUs:
  68000 (Main CPU)
  Osc. 11.0592 MHz
  MC68HC705C8A? (MCU, internal ROM not dumped)

Video:
  HD46505SP-2 HD68B45SP (CRT Controller)
  Bt476KPJ35 (RAMDAC)

ROMs:
  2 x 27C040 (Gfx)
  2 x 27C010 (Code)
  2 Unpop. Socket (EPROM #E, #F)

Other:
  MAX695CPE
  2 x 20 pin connector (Sub Board?)
  24 pin connector
  Jumpers for ROMs size
  DSW4
  3.6V Lithium battery

Sub Board --------------------

  Unknown

*************************************************************************************************************/

ROM_START( bankroba )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "dkbkus_2.64-b.u32", 0x00000, 0x20000, CRC(03ddde43) SHA1(c24ed9419726ca7bd96a92651705043da545512f) )
	ROM_LOAD16_BYTE( "dkbkus_2.64-a.u31", 0x00001, 0x20000, CRC(8906f5b6) SHA1(b9dbecfac299bdd1dba5fe22cda3485b3202e074) )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "c8-bank_2.51.u2", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "bankroba_sub.mcu", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD( "dkbkus_2.31-c.u46", 0x00000, 0x80000, CRC(d94a3ead) SHA1(e599b8d110bae16f83b3969834aa9b01076e2310) )
	ROM_LOAD( "dkbkus_2.31-d.u51", 0x80000, 0x80000, CRC(834b63bb) SHA1(da6b5e2fc1626044ecddf438c696e606a72d6164) )

	ROM_REGION( 0x80000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "unknown_label.u18", 0x00000, 0x80000, CRC(37f5862d) SHA1(8053c9ea30bb304982ef7e2c67d94454df520dfd) ) // = bank_2.31-g.u17 (dualgame)

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "pal20x6acns.u12",  0x000, 0x0cc, NO_DUMP )   // size?
	ROM_LOAD( "palce16v8h.u22",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "pal20x10acns.u40", 0x000, 0x0cc, NO_DUMP )   // size?
	ROM_LOAD( "pal20x6acns.u43",  0x000, 0x0cc, NO_DUMP )   // size?
	ROM_LOAD( "palce16v8h.u44",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "pal20x6acns.u48",  0x000, 0x0cc, NO_DUMP )   // size?
	ROM_LOAD( "pal20x6acns.u52",  0x000, 0x0cc, NO_DUMP )   // size?
	ROM_LOAD( "palce16v8h.u53",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u56",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u57",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u69",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u71",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u72",   0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Triple Play
(c) 1997-1998 Cadillac Jack
(FEBRUARY 24TH, 1999 in the ROMS)

Board:
  CJ-8L REV-B LEV-1 (USA)

CPUs:
  MC68EC000FN12 (Main CPU)
  Osc. 22.1184 MHz
  MC68HC705C8A (Sound MCU, internal ROM not dumped)
  Osc. 4.000 MHz

Video:
  R6545AP (CRT Controller)
  Bt476KPJ50 (RAMDAC)

ROMs:
  6 x 27C040 (JP6-9 configure the rom sizes for program, sound and graphics)
  5 x GAL16V8d-15LP (read protected?)

Other:
  DSW8
  DS14C232CN (Serial)
  RS-232 (4 pin)
  CDP68HC68T1E (Real-Time clock plus RAM with serial interface)
  3.6V Lithium battery
  36/72 pin edge connector
  10/20 pin edge connector

Note:
  Game supports a Centronics iDP-3541 printer (and many others)

*************************************************************************************************************/

ROM_START( cj3play )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD( "cjtripleply-cj_1.10-a.u65", 0x00000, 0x80000, CRC(69ae3fd3) SHA1(50eed5130905b710f48b2086173448e999dc96e8) )

	ROM_REGION( 0x2000, "mcu", 0 )  // 68HC705C8A code
	ROM_LOAD( "cj-tripleplay_2.4.c8", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "cjtripleply-cj_1.10-d.u68", 0x000000, 0x80000, CRC(8bbcf296) SHA1(e7e6e88f5f3065e7df7fff45429fdda1404418d6) )
	ROM_LOAD16_BYTE( "cjtripleply-cj_1.10-c.u75", 0x000001, 0x80000, CRC(3dd101e0) SHA1(01241a880e72834282dd7447273ffc332a105ad1) )
	ROM_LOAD16_BYTE( "cjtripleply-cj_1.10-f.u51", 0x100000, 0x80000, CRC(c8ccf1a7) SHA1(7a7b0f68d6ed5894fb4deb93fbf8053aff4fdb35) )
	ROM_LOAD16_BYTE( "cjtripleply-cj_1.10-e.u61", 0x100001, 0x80000, CRC(ff59f0ae) SHA1(b9f9cdc90f44f75ace079ec08ab5d71b21ce98dd) )

	ROM_REGION( 0x80000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "cjtripleply-cj_1.10-g.u50", 0x00000, 0x80000, CRC(8129f700) SHA1(fc09e1e4694757b08570cc46c9536340fbce0ded) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d_vdp.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_vdo.u53", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck2.u64", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck1.u69", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_dec.u70", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Funny Fruit
(c) 1997-1998 Cadillac Jack
(APRIL 21ST, 1999 in the ROMs)

Board:
  CJ-8L REV-D

CPUs:
  MC68EC000FN12 (Main CPU)
  Osc. 22.1184 MHz
  MC68HC705C8P (Sound MCU, internal ROM not dumped)
  Osc. 4.000 MHz

Video:
  Xilinx chip (CRT Controller)
  Bt476KPJ35 (RAMDAC)

ROMs:
  6 x 27C040 (JP5-9 configure the rom sizes for program, sound and graphics)
  5 x GAL16V8d-15LP (read protected)

Other:
  DSW8
  RS-232C interface (4-pin)
  3.6V Lithium battery
  36/72 pin edge connector
  10/20 pin edge connector

Note:
  Game supports a Centronics iDP-3541 printer (4-pin connector) and a Deltronics DL-1275 ticket dispenser.
  It is also available on board CJ-8L REV-B LEV-1, wih a R6545AP as CRT controller.

*************************************************************************************************************/

ROM_START( cjffruit )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD( "cjfunfruit-cj_1.13-a.u65", 0x00000, 0x80000, CRC(3a74d769) SHA1(fc8804d49cc31dadf10027ed1e2458cae96d6355) )

	ROM_REGION( 0x2000, "mcu", 0 )  // 68HC705C8P code
	ROM_LOAD( "cjfunfruit_2.3.c8", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-d.u68", 0x000000, 0x80000, CRC(33ccdc3f) SHA1(8d81e25c5a38f280c6fe5710937c876dcb679e61) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-c.u75", 0x000001, 0x80000, CRC(93854506) SHA1(09fd85d60ab723883d28a12f56dbb0cb2b03907f) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-f.u51", 0x100000, 0x80000, CRC(f5de1072) SHA1(943a82899ca6a07991fa4031d2ff96f625c9d6f5) )
	ROM_LOAD16_BYTE( "cjfunfruit-cj_1.13-e.u61", 0x100001, 0x80000, CRC(7acaef9d) SHA1(5031dc22e787dc4d8dffe67382068b9926c24bef) )

	ROM_REGION( 0x80000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "cjfunfruit-cj_1.13-g.u50", 0x00000, 0x80000, CRC(5fb53d3e) SHA1(f4a37b00a9417440685d198f1375b615848e7fb6) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d_vdp.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_vdo.u53", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck2.u64", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck1.u69", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_dec.u70", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Deuces Wild 2 - American Heritage (Ver. 2.02F)
(APRIL 10TH, 1997 in the ROMS)

Board:
  CBC-8L REV-C LEV-1
  MADE IN CANADA

CPUs:
  68EC000 (Main CPU, scratched surface)
  Osc. 22.1184 MHz
  MC68HC705C8A (MCU, internal ROM not dumped)
  Osc. 4.000 MHz

Video:
  R6545AP (CRT Controller)
  Bt471KPJ80 (RAMDAC)

ROMs:
  2 x 27C020 (Gfx)
  1 x 27C010 (Sound)
  1 x 27C020 (Code)
  1 Unpop. Socket (EPROM-PRGM #B)

Other:
  RS-232 (4 pin)
  RS-232 (DB9)
  Parallel Port
  3 x RS-485
  3 x AMD232LJN
  Exar ST16C452CJ (Dual UART + CENTRONICS parallel port)
  Osc. 1.8432 MHz
  Jumpers for ROMs size
  DSW8
  3.6V Lithium battery
  36/72 pin edge connector
  10/20 pin edge connector

*************************************************************************************************************/

ROM_START( deucesw2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "cb2wild-ah-2.02f-a.u92", 0x00000, 0x40000, CRC(723140ef) SHA1(8ab9c89663ce0dd736b6e9f701f16dbf5ebb9527) )    // "for CBC ($6F6D)"

	ROM_REGION( 0x2000, "mcu", 0 )  // 68HC705C8P code
	ROM_LOAD( "cbc-8l_2.0.u31", 0x0000, 0x2000, NO_DUMP )   // "for CBC-8L REV..." (label is only partially readable)

	ROM_REGION( 0x80000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "cb2wild-ah-2.02f-d.u87", 0x00000, 0x40000, CRC(7ab3ea30) SHA1(5e435f2a6ea169b827ae0f3da6a8afda0b636d7e) ) // "for CBC ($AA97)"
	ROM_LOAD16_BYTE( "cb2wild-ah-2.02f-c.u94", 0x00001, 0x40000, CRC(5b465430) SHA1(df428e3309732376d0999ad75567e264b7db9a1c) ) // "for CBC ($465A)"

	ROM_REGION( 0x20000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "cb2wild-ah-2.02f-k.u54", 0x00000, 0x20000, CRC(1eea618b) SHA1(65f3513d1a93a8afbfaeff27ebea5f0b5348e54b) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d_vdp.u23", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck2.u62", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_vdo.u65", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_ck1.u70", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_irq.u80", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_dec.u83", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_pia.u86", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Dual Games? (demo/prototype, no payouts)
(c) 1995 Labtronix Technologies

Main Board -------------------

  MPM-8L6 REV-0 LEV-1
  (c) 1993, Entertainment Technology Corp.
  Made in CANADA

CPUs:
  TMP68HC000P-12 (Main CPU)
  Osc. 11.05920 MHz
  2 x MC68HC705C8A (MCUs, internal ROMs not dumped)
  Osc. 8.000 MHz

Video:
  HD46505SP-2 HD68B45SP (CRT Controller)
  Bt476KPJ35 (RAMDAC)

ROMs:
  2 x ??????  (Code)
  2 x 27C040? (Gfx)

Other:
  MC68HC68T1P (Real-Time clock plus RAM with serial interface)
  DSW4
  2 x RS-232C interface (3 wire, labeled COM1 & ...)
  MAX232N
  Flat cable connector (Sub Board)
  3.6V Lithium battery
  10 pin connector (Buttons)
  36/72 pin edge connector
  10/20 pin edge connector

Sub Board --------------------

  Model: DAB
  Code:  AA1

ROMs:
  1 x 27C040 (+ Unpopulated socket)

Other:
  Dallas DS166-010? (Audio Digital Resistor?)
  Osc. ??.0000 MHz

*************************************************************************************************************/

ROM_START( dualgame )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mpduga_0.01-a.u27", 0x00000, 0x20000, CRC(57b87596) SHA1(b31d83f5dbd0ad25564c876e2995bba61e1f425f) )  // "for MPI/MPM 09/05/1995"
	ROM_LOAD16_BYTE( "mpduga_0.01-b.u28", 0x00001, 0x20000, CRC(e441d895) SHA1(c026b6ebeaedece303b9361bd92c69150ea63b0a) )  // ""

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "dualgame.u8", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "dualgame.u9", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "mpduga_0.01-d.u69", 0x00000, 0x80000, CRC(2f65e87e) SHA1(ded9d75ebb46e061615dac408f86dad14df9d30b) )
	ROM_LOAD16_BYTE( "mpduga_0.01-c.u68", 0x00001, 0x80000, CRC(bc5b4738) SHA1(69bcc15d3e7524ba26dad0e29919461fbd0a8736) )

	ROM_REGION( 0x80000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "bank_2.31-g.u17", 0x00000, 0x80000, CRC(37f5862d) SHA1(8053c9ea30bb304982ef7e2c67d94454df520dfd) )   // "for DK-B or DAB 03/04/1995" (hand-written: Demo)
ROM_END

/*************************************************************************************************************

The Hermit (Ver. 1.14)
(c) 1995 Dugamex

Board:
  LC-8L REV-0 LEV-1
  Made in CANADA

CPUs:
  MC68EC000FN12 (Main CPU)
  Osc. 22.1184 MHz
  MC68HC705C8A (MCU, internal ROM not dumped)
  Osc. 4.000 MHz

Video:
  HD46505SP-2 HD68B45SP (CRT Controller)
  Bt471KPJ80 (RAMDAC)

ROMs:
  2 x 27C040 (Gfx)
  1 x 27C040 (Sound)
  1 x ?????? (Code)
  1 Unpop. Socket (EPROM-PRGM #B)

Other:
  Empty space for MC68HC68T1 (Real-Time clock)
  Empty space for RAM-Backup
  Empty space RS-232C (MAX232)
  Jumpers for ROMs size, V-Sync, H-Sync
  DSW8
  Reset Push Button
  3.6V Lithium battery
  36/72 pin edge connector
  10/20 pin edge connector

*************************************************************************************************************/

ROM_START( hermit )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD( "unknown_label.u69", 0x00000, 0x40000, CRC(5e1a37c1) SHA1(76ec6ef75ddda848d466e20a4a8bfb3e1647e876) )

	ROM_REGION( 0x2000, "mcu", 0 )  // 68HC705C8P code
	ROM_LOAD( "lc-hermit_1.00.u51", 0x0000, 0x2000, NO_DUMP )   // "for LC-8L 26/04/1995"

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD16_BYTE( "unknown_label.u2", 0x00000, 0x80000, CRC(fc8b9ec6) SHA1(c7515cf78d68a1ae7f2e8e50fe3083db2547c314) )
	ROM_LOAD16_BYTE( "unknown_label.u3", 0x00001, 0x80000, CRC(0a621d76) SHA1(66cabf4e233dc784851c9fb07f18658c10744cd7) )

	ROM_REGION( 0x80000, "samples", 0 ) // 8 bit unsigned
	ROM_LOAD( "lcherm_1.00-g.u48", 0x00000, 0x80000, CRC(1ad999de) SHA1(02beb744a0a6fb92e225c1de10672c852151eb6b) ) // "for LC-8L 26/04/1995"

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "palce16v8h.u34", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u45", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u46", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u56", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Il Pagliaccio (unknown manufacturer)

CPU

1x MC68HC000FN12 (main)
1x P87C748EBPN (80C51 family 8-bit microcontroller)
1x oscillator 11.0592MHz
1x blue resonator CSB400P

VIDEO

1x XC95144XL
2x XC9572XL
1x ADV471KP50

ROMs

4x W27E040
1x P87C748EBPN (read protected)

Note

1x 28x2 edge connector (not JAMMA)
1x trimmer (volume)
1x battery
2x pushbutton (MANAGEMENT,STATISTIC)

*************************************************************************************************************/

ROM_START( ilpag )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.7c-35.u32", 0x00000, 0x80000, CRC(ed99c884) SHA1(b3d2c9fb7765e3c8ff1e0de9c8edb6628e1c79ef) )
	ROM_LOAD16_BYTE( "2.7c-36.u31", 0x00001, 0x80000, CRC(4cd41688) SHA1(a1a15b06aa738cd4154d3c3479a7bf2da0e48426) )

	ROM_REGION( 0x800, "mcu", 0 ) // 87C748 code
	ROM_LOAD( "87c748.u132", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x100000, "blitter", 0 ) // data for the blitter
	ROM_LOAD( "graf1.u46",   0x00000, 0x80000, CRC(cf745964) SHA1(7af4a6c0b8d01c0d1b71bc5330a257d2fa712611) )
	ROM_LOAD( "graf2.u51",   0x80000, 0x80000, CRC(2d64d3b5) SHA1(8fdb943d0aedf12706ce0a772c8f5155fa03e8c7) )
ROM_END

/*************************************************************************************************************

Maxi Double Poker (Ver. 1.10)
(c) 1992 Blitz Systems Inc.

Main Board -------------------

  SPI-68K REV-C LEV-1
  Copyright 1991, Blitz System Inc.
  Made in CANADA

CPUs:
  68000 (Main CPU)
  Osc. 11.05920 MHz
  2 x MC68HC705C8A (MCUs, internal ROMs not dumped)
  Osc. 8.0000 MHz

Video:
  HD46505SP-2 HD68B45SP (CRT Controller)

Audio:
  SAA1099P (6 voice sound chip)

ROMs:
  2 x 27C010  (Code + Gfx)

Other:
  DSW4
  2 x RS-232C interface (3 wire, labeled COM-1 & COM-2)
  MAX232CPE
  MAX695CPE
  Flat cable connector (Hopper Board?)
  20 pin connector (Sub Board)
  24 pin connector
  3.6V Lithium battery

Sub Board --------------------

  Model: 8L74
  Code:  AA1
  Copyright 1992, Blitz System Inc.
  "LVI-02366" (Sticker)
  "Mega Double No/Pay N3314" (Yellow hand-written sticker)

CPUs:
  MC68HC705C8A (MCU, internal ROM not dumped)
  Osc. 4.0000 MHz

Other:
  DSW4 (Video Sync)
  Trimmer (Audio Volume)
  Trimmer (Ratio)
  10 pin connector (Buttons)
  36/72 pin edge connector
  10/20 pin edge connector

*************************************************************************************************************/

ROM_START( maxidbl )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "spm_maxi_1.1-b.u25", 0x00000, 0x20000, CRC(b0c754c2) SHA1(a664386d1813b1e06be0d9f41fc3569dfc20e395) )
	ROM_LOAD16_BYTE( "sp_maxi_1.1-a.u24",  0x00001, 0x20000, CRC(adec27b6) SHA1(3dabe86eae9781e6d8fe20e160351e5e757faeda) )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "c8_pi-2.u5", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "c8_spi-maxi.u6", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu3", 0 ) // 68HC705C8P code
	ROM_LOAD( "sub8l74_846.u18", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "16as15hb1.u2",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "16as15hb1.u13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "16as15hb1.u14", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "16as15hb1.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "16as15hb1.u19", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Mega Double Poker (Ver. 1.63 Espagnol)
(c) 1990 Blitz Systems Inc.
(NOVEMBER 1994 in the ROMs)

Main Board -------------------

  SPI-68K REV-E LEV-1
  Same as Maxi Double Poker (Ver. 1.10) but without the SAA1099P sound chip

Sub Board --------------------

  HILO S\K-B REV-B
  Copyright 1992, Blitz System Inc.
  "LVI-02932" (Sticker)

CPUs:
  MC68HC705C8A (MCU, internal ROM not dumped)
  Osc. 4.000 MHz

Other:
  MC68HC68T1P (Real-Time clock plus RAM with serial interface)
  DSW4 (Video Sync)
  Trimmer (Interface Audio Volume)
  Trimmer (Mother Board Audio Volume)
  Trimmer
  10 pin connector (Buttons)
  36/72 pin edge connector

Hopper Board -----------------

  Model: HOP?
  Code:  ?
  Copyright 1992, Blitz System Inc.

Other:
  20 pin chip labelled "HOP-DEC"
  2 x DSW4
  16 pin connector

*************************************************************************************************************/

ROM_START( megadble )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code + gfx
	ROM_LOAD16_BYTE( "spmegasp_1.62-b.u25", 0x00000, 0x20000, CRC(4562a181) SHA1(82f1034a0631f83236b163c0c1d6fed5d365bf9c) )
	ROM_LOAD16_BYTE( "spmegasp_1.62-a.u24", 0x00001, 0x20000, CRC(292fdacc) SHA1(fd8e117586569abb094d3b0ebd41292565f18c2a) )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "xpi-2_2.6.u5", 0x0000, 0x2000, NO_DUMP ) // C8-#2

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "xpi-1_2.6.u6", 0x0000, 0x2000, NO_DUMP ) // C8-#1, no label

	ROM_REGION( 0x2000, "mcu3", 0 ) // 68HC705C8P code
	ROM_LOAD( "hilo_sk-b_4.01.u18", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "palce16v8h.u2",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u14", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u19", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Mega Double Poker Jackpot (Ver. 1.26)
(c) 1990 Blitz Systems Inc.
(JUNE 28TH, 1993 in the ROMs)

Main Board -------------------

  SPI-68K REV-C LEV-1
  See Maxi Double Poker (Ver. 1.10)

Sub Board --------------------

  Same as Mega Double Poker (Ver. 1.63 Espagnol)?

*************************************************************************************************************/

ROM_START( megadblj )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code + gfx
	ROM_LOAD16_BYTE( "spmegajk_1.26-b.u25", 0x00000, 0x20000, CRC(de206213) SHA1(35d886c805424fe7b364b49f301ff8467e30c26b) )
	ROM_LOAD16_BYTE( "spmegajk_1.26-a.u24", 0x00001, 0x20000, CRC(fa0ccaea) SHA1(562324119e08d44228ac754acbf763d0781ab891) )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "c8_pi-2.u5", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "c8_spi-megajk.u6", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu3", 0 ) // 68HC705C8P code
	ROM_LOAD( "megadblj_sub.mcu", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "palce16v8h.u2",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u14", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u19", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Poker 52 (Ver. 1.2)
(COPYRIGHT 1993 BLITZ SYSTEM INC
MARCH 10TH, 1994 in the ROMs)

Main Board -------------------

  SPI-68K REV-E LEV-1
  Same as Maxi Double Poker (Ver. 1.10) but without the SAA1099P sound chip

Sub Board --------------------

  Unknown

*************************************************************************************************************/

ROM_START( poker52 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 ) // 68000 code + gfx
	ROM_LOAD16_BYTE( "unknown_label.u25", 0x00000, 0x20000, CRC(63e0dd69) SHA1(970e6363ade714a2b9a844c5683ab1365193457a) )
	ROM_LOAD16_BYTE( "unknown_label.u24", 0x00001, 0x20000, NO_DUMP )

	ROM_REGION( 0x2000, "mcu1", 0 ) // 68HC705C8P code
	ROM_LOAD( "xpi-2_2.3.u5", 0x0000, 0x2000, NO_DUMP ) // C8-#2, "for SPI or MPI 09/02/1995" (partially readable)

	ROM_REGION( 0x2000, "mcu2", 0 ) // 68HC705C8P code
	ROM_LOAD( "spi-pk52_1.6.u6", 0x0000, 0x2000, NO_DUMP )  // C8-#2

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "palce16v8h.u2",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u14", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u15", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u19", 0x000, 0x117, NO_DUMP )
ROM_END

/*************************************************************************************************************

Strip Teaser (unknown manufacturer)

lower board (Dk-B)

TS68000CP12 (main cpu)
osc. 11.0592MHz
MC68HC705C8P (MCU)
UM6845RA (CRT controller-Supports alphanumeric and graphics modes.Addresses up to 16 KB of video memory-2 MHz)
Lithium battery 3,6V


upper board (8L74) (soundboard?)

MC68HC705C8P (MCU)
osc. 4.0000MHz
non JAMMA connector
1x dipswitch (4 switch)


ROMs
1x AT27c010 (u31.1)(program)
1x AM27C010 (u32.6)(program)
4x M27C4001 (u46.2 - u51.3 - u61.4 - u66.5)(GFX)
1x M27C4001 (u18.7)(sound)

*************************************************************************************************************/

ROM_START( steaser )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u31.1", 0x00001, 0x20000, CRC(7963e960) SHA1(2a1c68265e0a3909ccd097ea784e3e179f528844) )
	ROM_LOAD16_BYTE( "u32.6", 0x00000, 0x20000, CRC(c0ab5fb1) SHA1(15b3dbf0242e885b7009c21479544a821d4e5a7d) )

	ROM_REGION( 0x1000, "mcu", 0 ) // 68705
	ROM_LOAD( "mc68hc705c8p_main.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "mcu2", 0 ) // 68705
	ROM_LOAD( "mc68hc705c8p_sub.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "blitter", 0 ) // data for the blitter
	ROM_LOAD( "u46.2", 0x000000, 0x80000, CRC(c4a5e47b) SHA1(9f3d3124c76c0bdf8cdca849e1d921a335e433b6) )
	ROM_LOAD( "u51.3", 0x080000, 0x80000, CRC(4dc57435) SHA1(7dfa6f9e35986dd48869786abbe70103f336bcb1) )
	ROM_LOAD( "u61.4", 0x100000, 0x80000, CRC(d8d8dc6f) SHA1(5a76b1fd1a3a532e5ff2de127286ace7d3567c58) )
	ROM_LOAD( "u66.5", 0x180000, 0x80000, CRC(da309671) SHA1(66baf8a83024547c471da39748ff99a9a9013ea4) )

	ROM_REGION( 0x80000, "oki", 0 ) // Sound Samples
	ROM_LOAD( "u18.7", 0x00000, 0x80000, CRC(ee942232) SHA1(b9c1fc73c6006bcad0dd177e0f30a96f1063a993) )
ROM_END


/*************************************************************************************************************
    ROM patches
*************************************************************************************************************/

DRIVER_INIT_MEMBER(blitz68k_state,bankrob)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xb5e0/2] = 0x6028;

	// crtc
	ROM[0x81d0/2] = 0x4e71;
	ROM[0x81d8/2] = 0x4e71;

	// loop
	ROM[0x1d4d4/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(blitz68k_state,bankroba)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0x11e4e/2] = 0x6028;

	// crtc
	ROM[0xf640/2] = 0x4e71;
	ROM[0xf648/2] = 0x4e71;

	// loop
	ROM[0x178ec/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(blitz68k_state,cj3play)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0x7064/2] = 0x6028;
	ROM[0xa0d2/2] = 0x6024;

	// loop
	ROM[0x2773c/2] = 0x4e71;
//  ROM[0x3491a/2] = 0x4e71;

	// ERROR CHECKSUM ROM PROGRAM
	ROM[0x20ab0/2] = 0x6050;
}

DRIVER_INIT_MEMBER(blitz68k_state,cjffruit)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xf564/2] = 0x6028;

	// ERROR CHECKSUM ROM PROGRAM
	ROM[0x1e7b8/2] = 0x6050;
}

DRIVER_INIT_MEMBER(blitz68k_state,deucesw2)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0x8fe4/2] = 0x6020;

	// ERROR CHECKSUM ROM PROGRAM
	ROM[0x12f70/2] = 0x6054;
}

DRIVER_INIT_MEMBER(blitz68k_state,dualgame)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xa518/2] = 0x6024;

	ROM[0x1739a/2] = 0x4e71;
	ROM[0x1739c/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(blitz68k_state,hermit)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xdeba/2] = 0x602e;

	// ROM: BAD
	ROM[0xdd78/2] = 0x4e71;

	// loop
	ROM[0x15508/2] = 0x4e71;

	// crtc
	ROM[0x3238/2] = 0x4e75;
}

DRIVER_INIT_MEMBER(blitz68k_state,maxidbl)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xb384/2] = 0x6036;

	// loop
	ROM[0x17ca/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(blitz68k_state,megadblj)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xe21c/2] = 0x6040;

	// loop
	ROM[0x19d4/2] = 0x4e71;
}

DRIVER_INIT_MEMBER(blitz68k_state,megadble)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	// WRONG C8 #1
	ROM[0xcfc2/2] = 0x4e71;

	// C8 #2 NOT RESPONDING
	ROM[0x1d40/2] = 0x4e71;
}



GAME( 1992,  maxidbl,  0,       maxidbl,  maxidbl, blitz68k_state,  maxidbl,  ROT0,  "Blitz Systems Inc.",             "Maxi Double Poker (Ver. 1.10)",                  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_WRONG_COLORS )
GAME( 1990,  megadblj, 0,       maxidbl,  maxidbl, blitz68k_state,  megadblj, ROT0,  "Blitz Systems Inc.",             "Mega Double Poker Jackpot (Ver. 1.26)",          MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // JUNE 28TH, 1993
GAME( 1990,  megadble, 0,       maxidbl,  maxidbl, blitz68k_state,  megadble, ROT0,  "Blitz Systems Inc.",             "Mega Double Poker (Ver. 1.63 Espagnol)",         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_WRONG_COLORS ) // NOVEMBER 1994
GAME( 1993,  steaser,  0,       steaser,  steaser, driver_device,  0,        ROT0,  "<unknown>",                      "Strip Teaser (Italy, Ver. 1.22)",                MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // In-game strings are in Italian but service mode is half English / half French?
GAME( 1993,  bankrob,  0,       bankrob,  bankrob, blitz68k_state,  bankrob,  ROT0,  "Entertainment Technology Corp.", "Bank Robbery (Ver. 3.32)",                       MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // BLITZ SYSTEM INC APRIL 1995
GAME( 1993,  bankroba, bankrob, bankroba, bankrob, blitz68k_state,  bankroba, ROT0,  "Entertainment Technology Corp.", "Bank Robbery (Ver. 2.00)",                       MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // BLITZ SYSTEM INC MAY 10TH, 1993
GAME( 1993?, poker52,  0,       maxidbl,  maxidbl, driver_device,  0,        ROT0,  "Blitz Systems Inc.",             "Poker 52 (Ver. 1.2)",                            MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                  // MARCH 10TH, 1994
GAME( 1995,  dualgame, 0,       dualgame, dualgame, blitz68k_state, dualgame, ROT0,  "Labtronix Technologies",         "Dual Games (prototype)",                         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // SEPTEMBER 5TH, 1995
GAME( 1995,  hermit,   0,       hermit,   hermit, blitz68k_state,   hermit,   ROT0,  "Dugamex",                        "The Hermit (Ver. 1.14)",                         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // APRIL 1995
GAME( 1997,  deucesw2, 0,       deucesw2, deucesw2, blitz68k_state, deucesw2, ROT0,  "<unknown>",                      "Deuces Wild 2 - American Heritage (Ver. 2.02F)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // APRIL 10TH, 1997
GAME( 1998,  cj3play,  0,       cjffruit, cjffruit, blitz68k_state, cj3play,  ROT0,  "Cadillac Jack",                  "Triple Play (Ver. 1.10)",                        MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // FEBRUARY 24TH, 1999
GAME( 1998,  cjffruit, 0,       cjffruit, cjffruit, blitz68k_state, cjffruit, ROT0,  "Cadillac Jack",                  "Funny Fruit (Ver. 1.13)",                        MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )                     // APRIL 21ST, 1999
GAME( 199?,  ilpag,    0,       ilpag,    ilpag, driver_device,    0,        ROT0,  "<unknown>",                      "Il Pagliaccio (Italy, Ver. 2.7C)",               MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND )
