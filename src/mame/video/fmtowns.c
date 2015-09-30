// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

/*
 * FM Towns video hardware
 *
 * Resolution: from 320x200 to 768x512
 *
 * Up to two graphics layers
 *
 * Sprites
 *
 * CRTC registers (16-bit):
 *
 *  0:  HSync width 1
 *  1:  HSync width 2
 *  4:  HSync total
 *  5:  VSync width 1
 *  6:  VSync width 2
 *  7:  Equalising pulse accountable time (what?)
 *  8:  VSync total
 *
 *  9:
 *  10: Graphic layer 0 horizontal start/end
 *  11:
 *  12: Graphic layer 1 horizontal start/end
 *
 *  13:
 *  14: Graphic layer 0 vertical start/end
 *  15:
 *  16: Graphic layer 1 vertical start/end
 *
 *  17: Graphic layer 0 initial address?
 *  18: Graphic layer 0 horizontal adjust
 *  19: Graphic layer 0 field indirect address offset
 *  20: Graphic layer 0 line indirect address offset
 *
 *  21-24: As above, but for Graphic layer 1
 *
 *  27: Layer zoom.     bit 0 = x2 horizontal zoom layer 0
 *  to be confirmed     bit 5 = x2 vertical zoom layer 0
 *                      bit 9 = x2 horizontal zoom layer 1
 *                      bit 13 = x2 vertical zoom layer 1
 *
 *  28: Control register 0
 *      VSync enable (bit 15) (blank display?)
 *      Scroll type (layer 0 = bit 4, layer 1 = bit 5)
 *          0 = spherical scroll, 1 = cylindrical scroll
 *
 *  29: Control register 1
 *      Dot clock (bits 1 and 0)
 *      0x00 = 28.6363MHz
 *      0x01 = 24.5454MHz
 *      0x02 = 25.175MHz
 *      0x03 = 21.0525MHz (default?)
 *
 *  30: Dummy register
 *
 *  31: Control register 2
 *
 *  Video registers:
 *
 *  0:  Graphic layer(s) type: (others likely exist)
 *      bit 4 = 2 layers
 *      bits 2-3 = layer 1 mode
 *      bits 0-1 = layer 0 mode
 *          mode: 1 = 16 colours, 2 = 256 colours, 3 = highcolour (16-bit)
 *                0 = disabled?
 *
 *  1:  Layer reverse (priority?) (bit 0)
 *      YM (bit 2) - unknown
 *      peltype (bits 4 and 5)
 *
 *
 *  Sprite registers:
 *
 *  0,1:    Maximum sprite (last one to render?) (10-bit)
 *
 *  1 (bit 7):  Enable sprite display
 *
 *  2,3:    X offset (9-bit)
 *
 *  4,5:    Y offset (9-bit)
 *
 *  6 (bit 4):  VRAM location (0=0x40000,1=0x60000)
 *
 */

#include "emu.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "includes/fmtowns.h"

//#define CRTC_REG_DISP 1
//#define SPR_DEBUG 1

//static UINT32 pshift;  // for debugging

void towns_state::towns_crtc_refresh_mode()
{
	unsigned int width,height;

	rectangle scr(0, m_video.towns_crtc_reg[4], 0, m_video.towns_crtc_reg[8] / 2);

	// layer 0
	width = m_video.towns_crtc_reg[10] - m_video.towns_crtc_reg[9];
	height = (m_video.towns_crtc_reg[14] - m_video.towns_crtc_reg[13]) / 2;
	m_video.towns_crtc_layerscr[0].min_x = scr.xcenter() - (width / 2);
	m_video.towns_crtc_layerscr[0].min_y = scr.ycenter() - (height / 2);
	m_video.towns_crtc_layerscr[0].max_x = scr.xcenter() + (width / 2);
	m_video.towns_crtc_layerscr[0].max_y = scr.ycenter() + (height / 2);

	// layer 1
	width = m_video.towns_crtc_reg[12] - m_video.towns_crtc_reg[11];
	height = (m_video.towns_crtc_reg[16] - m_video.towns_crtc_reg[15]) / 2;
	m_video.towns_crtc_layerscr[1].min_x = scr.xcenter() - (width / 2);
	m_video.towns_crtc_layerscr[1].min_y = scr.ycenter() - (height / 2);
	m_video.towns_crtc_layerscr[1].max_x = scr.xcenter() + (width / 2);
	m_video.towns_crtc_layerscr[1].max_y = scr.ycenter() + (height / 2);

	// sanity checks
	if(scr.max_x == 0 || scr.max_y == 0)
		return;
	if(scr.max_x <= scr.min_x || scr.max_y <= scr.min_y)
		return;

	machine().first_screen()->configure(scr.max_x+1,scr.max_y+1,scr,HZ_TO_ATTOSECONDS(60));
}

READ8_MEMBER( towns_state::towns_gfx_high_r )
{
	return m_towns_gfxvram[offset];
}

WRITE8_MEMBER( towns_state::towns_gfx_high_w )
{
	m_towns_gfxvram[offset] = data;
}

READ8_MEMBER( towns_state::towns_gfx_r )
{
	UINT8 ret = 0;

	if(m_towns_mainmem_enable != 0)
		return m_messram->pointer()[offset+0xc0000];

	offset = offset << 2;

	if(m_video.towns_vram_page_sel != 0)
		offset += 0x20000;

	ret = (((m_towns_gfxvram[offset] >> m_video.towns_vram_rplane) << 7) & 0x80)
		| (((m_towns_gfxvram[offset] >> m_video.towns_vram_rplane) << 2) & 0x40)
		| (((m_towns_gfxvram[offset+1] >> m_video.towns_vram_rplane) << 5) & 0x20)
		| (((m_towns_gfxvram[offset+1] >> m_video.towns_vram_rplane)) & 0x10)
		| (((m_towns_gfxvram[offset+2] >> m_video.towns_vram_rplane) << 3) & 0x08)
		| (((m_towns_gfxvram[offset+2] >> m_video.towns_vram_rplane) >> 2) & 0x04)
		| (((m_towns_gfxvram[offset+3] >> m_video.towns_vram_rplane) << 1) & 0x02)
		| (((m_towns_gfxvram[offset+3] >> m_video.towns_vram_rplane) >> 4) & 0x01);

	return ret;
}

WRITE8_MEMBER( towns_state::towns_gfx_w )
{
	if(m_towns_mainmem_enable != 0)
	{
		m_messram->pointer()[offset+0xc0000] = data;
		return;
	}
	offset = offset << 2;
	if(m_video.towns_vram_page_sel != 0)
		offset += 0x20000;
	if(m_video.towns_vram_wplane & 0x08)
	{
		m_towns_gfxvram[offset] &= ~0x88;
		m_towns_gfxvram[offset] |= ((data & 0x80) >> 4) | ((data & 0x40) << 1);
		m_towns_gfxvram[offset + 1] &= ~0x88;
		m_towns_gfxvram[offset + 1] |= ((data & 0x20) >> 2) | ((data & 0x10) << 3);
		m_towns_gfxvram[offset + 2] &= ~0x88;
		m_towns_gfxvram[offset + 2] |= ((data & 0x08)) | ((data & 0x04) << 5);
		m_towns_gfxvram[offset + 3] &= ~0x88;
		m_towns_gfxvram[offset + 3] |= ((data & 0x02) << 2) | ((data & 0x01) << 7);
	}
	if(m_video.towns_vram_wplane & 0x04)
	{
		m_towns_gfxvram[offset] &= ~0x44;
		m_towns_gfxvram[offset] |= ((data & 0x80) >> 5) | ((data & 0x40));
		m_towns_gfxvram[offset + 1] &= ~0x44;
		m_towns_gfxvram[offset + 1] |= ((data & 0x20) >> 3) | ((data & 0x10) << 2);
		m_towns_gfxvram[offset + 2] &= ~0x44;
		m_towns_gfxvram[offset + 2] |= ((data & 0x08) >> 1) | ((data & 0x04) << 4);
		m_towns_gfxvram[offset + 3] &= ~0x44;
		m_towns_gfxvram[offset + 3] |= ((data & 0x02) << 1) | ((data & 0x01) << 6);
	}
	if(m_video.towns_vram_wplane & 0x02)
	{
		m_towns_gfxvram[offset] &= ~0x22;
		m_towns_gfxvram[offset] |= ((data & 0x80) >> 6) | ((data & 0x40) >> 1);
		m_towns_gfxvram[offset + 1] &= ~0x22;
		m_towns_gfxvram[offset + 1] |= ((data & 0x20) >> 4) | ((data & 0x10) << 1);
		m_towns_gfxvram[offset + 2] &= ~0x22;
		m_towns_gfxvram[offset + 2] |= ((data & 0x08) >> 2) | ((data & 0x04) << 3);
		m_towns_gfxvram[offset + 3] &= ~0x22;
		m_towns_gfxvram[offset + 3] |= ((data & 0x02)) | ((data & 0x01) << 5);
	}
	if(m_video.towns_vram_wplane & 0x01)
	{
		m_towns_gfxvram[offset] &= ~0x11;
		m_towns_gfxvram[offset] |= ((data & 0x80) >> 7) | ((data & 0x40) >> 2);
		m_towns_gfxvram[offset + 1] &= ~0x11;
		m_towns_gfxvram[offset + 1] |= ((data & 0x20) >> 5) | ((data & 0x10));
		m_towns_gfxvram[offset + 2] &= ~0x11;
		m_towns_gfxvram[offset + 2] |= ((data & 0x08) >> 3) | ((data & 0x04) << 2);
		m_towns_gfxvram[offset + 3] &= ~0x11;
		m_towns_gfxvram[offset + 3] |= ((data & 0x02) >> 1) | ((data & 0x01) << 4);
	}
}

void towns_state::towns_update_kanji_offset()
{
	// this is a little over the top...
	if(m_video.towns_kanji_code_h < 0x30)
	{
		m_video.towns_kanji_offset = ((m_video.towns_kanji_code_l & 0x1f) << 4)
							| (((m_video.towns_kanji_code_l - 0x20) & 0x20) << 8)
							| (((m_video.towns_kanji_code_l - 0x20) & 0x40) << 6)
							| ((m_video.towns_kanji_code_h & 0x07) << 9);
	}
	else if(m_video.towns_kanji_code_h < 0x70)
	{
		m_video.towns_kanji_offset = ((m_video.towns_kanji_code_l & 0x1f) << 4)
							+ (((m_video.towns_kanji_code_l - 0x20) & 0x60) << 8)
							+ ((m_video.towns_kanji_code_h & 0x0f) << 9)
							+ (((m_video.towns_kanji_code_h - 0x30) & 0x70) * 0xc00)
							+ 0x8000;
	}
	else
	{
		m_video.towns_kanji_offset = ((m_video.towns_kanji_code_l & 0x1f) << 4)
							| (((m_video.towns_kanji_code_l - 0x20) & 0x20) << 8)
							| (((m_video.towns_kanji_code_l - 0x20) & 0x40) << 6)
							| ((m_video.towns_kanji_code_h & 0x07) << 9)
							| 0x38000;
	}
}

READ8_MEMBER( towns_state::towns_video_cff80_r )
{
	UINT8* ROM = m_user->base();

	switch(offset)
	{
		case 0x00:  // mix register
			return m_video.towns_crtc_mix;
		case 0x01:  // read/write plane select (bit 0-3 write, bit 6-7 read)
			return ((m_video.towns_vram_rplane << 6) & 0xc0) | m_video.towns_vram_wplane;
		case 0x02:  // display planes (bits 0-2,5), display page select (bit 4)
			return m_video.towns_display_plane | m_video.towns_display_page_sel;
		case 0x03:  // VRAM page select (bit 5)
			if(m_video.towns_vram_page_sel != 0)
				return 0x10;
			else
				return 0x00;
		case 0x06:
			if(m_video.towns_vblank_flag != 0)
				return 0x10;
			else
				return 0x00;
		case 0x16:  // Kanji character data
			return ROM[(m_video.towns_kanji_offset << 1) + 0x180000];
		case 0x17:  // Kanji character data
			return ROM[(m_video.towns_kanji_offset++ << 1) + 0x180001];
		case 0x19:  // ANK CG ROM
			if(m_towns_ankcg_enable != 0)
				return 0x01;
			else
				return 0x00;
		default:
			logerror("VGA: read from invalid or unimplemented memory-mapped port %05x\n",0xcff80+offset*4);
	}

	return 0;
}

WRITE8_MEMBER( towns_state::towns_video_cff80_w )
{
	switch(offset)
	{
		case 0x00:  // mix register
			m_video.towns_crtc_mix = data;
			break;
		case 0x01:  // read/write plane select (bit 0-3 write, bit 6-7 read)
			m_video.towns_vram_wplane = data & 0x0f;
			m_video.towns_vram_rplane = (data & 0xc0) >> 6;
			towns_update_video_banks(space);
			//logerror("VGA: VRAM wplane select = 0x%02x\n",towns_vram_wplane);
			break;
		case 0x02:  // display plane (bits 0-2), display page select (bit 4)
			m_video.towns_display_plane = data & 0x27;
			m_video.towns_display_page_sel = data & 0x10;
			break;
		case 0x03:  // VRAM page select (bit 4)
			m_video.towns_vram_page_sel = data & 0x10;
			break;
		case 0x14:  // Kanji offset (high)
			m_video.towns_kanji_code_h = data & 0x7f;
			towns_update_kanji_offset();
			//logerror("VID: Kanji code set (high) = %02x %02x\n",towns_kanji_code_h,towns_kanji_code_l);
			break;
		case 0x15:  // Kanji offset (low)
			m_video.towns_kanji_code_l = data & 0x7f;
			towns_update_kanji_offset();
			//logerror("VID: Kanji code set (low) = %02x %02x\n",towns_kanji_code_h,towns_kanji_code_l);
			break;
		case 0x19:  // ANK CG ROM
			m_towns_ankcg_enable = data & 0x01;
			towns_update_video_banks(space);
			break;
		default:
			logerror("VGA: write %08x to invalid or unimplemented memory-mapped port %05x\n",data,0xcff80+offset);
	}
}

READ8_MEMBER( towns_state::towns_video_cff80_mem_r )
{
	if(m_towns_mainmem_enable != 0)
		return m_messram->pointer()[offset+0xcff80];

	return towns_video_cff80_r(space,offset);
}

WRITE8_MEMBER( towns_state::towns_video_cff80_mem_w )
{
	if(m_towns_mainmem_enable != 0)
	{
		m_messram->pointer()[offset+0xcff80] = data;
		return;
	}
	towns_video_cff80_w(space,offset,data);
}

/*
 *  port 0x440-0x443 - CRTC
 *      0x440 = register select
 *      0x442/3 = register data (16-bit)
 *      0x448 = shifter register select
 *      0x44a = shifter register data (8-bit)
 *
 */
READ8_MEMBER(towns_state::towns_video_440_r)
{
	UINT8 ret = 0;
	UINT16 xpos,ypos;

	switch(offset)
	{
		case 0x00:
			return m_video.towns_crtc_sel;
		case 0x02:
//          logerror("CRTC: reading register %i (0x442) [%04x]\n",towns_crtc_sel,towns_crtc_reg[towns_crtc_sel]);
			if(m_video.towns_crtc_sel == 30)
					return 0x00;
			return m_video.towns_crtc_reg[m_video.towns_crtc_sel] & 0x00ff;
		case 0x03:
//          logerror("CRTC: reading register %i (0x443) [%04x]\n",towns_crtc_sel,towns_crtc_reg[towns_crtc_sel]);
			if(m_video.towns_crtc_sel == 30)
			{
				// check video position
				xpos = space.machine().first_screen()->hpos();
				ypos = space.machine().first_screen()->vpos();

				if(xpos < (m_video.towns_crtc_reg[0] & 0xfe))
					ret |= 0x02;
				if(ypos < (m_video.towns_crtc_reg[6] & 0x1f))
					ret |= 0x04;
				if(xpos < m_video.towns_crtc_layerscr[0].max_x && xpos > m_video.towns_crtc_layerscr[0].min_x)
					ret |= 0x10;
				if(xpos < m_video.towns_crtc_layerscr[1].max_x && xpos > m_video.towns_crtc_layerscr[1].min_x)
					ret |= 0x20;
				if(ypos < m_video.towns_crtc_layerscr[0].max_y && ypos > m_video.towns_crtc_layerscr[0].min_y)
					ret |= 0x40;
				if(ypos < m_video.towns_crtc_layerscr[1].max_y && ypos > m_video.towns_crtc_layerscr[1].min_y)
					ret |= 0x80;

				return ret;
			}
			return (m_video.towns_crtc_reg[m_video.towns_crtc_sel] & 0xff00) >> 8;
		case 0x08:
			return m_video.towns_video_sel;
		case 0x0a:
			logerror("Video: reading register %i (0x44a) [%02x]\n",m_video.towns_video_sel,m_video.towns_video_reg[m_video.towns_video_sel]);
			return m_video.towns_video_reg[m_video.towns_video_sel];
		case 0x0c:
			if(m_video.towns_dpmd_flag != 0)
			{
				m_video.towns_dpmd_flag = 0;
				ret |= 0x80;
			}
			ret |= (m_video.towns_sprite_flag ? 0x02 : 0x00);  // Sprite drawing flag
			ret |= m_video.towns_sprite_page & 0x01;
			return ret;
		case 0x10:
			return m_video.towns_sprite_sel;
		case 0x12:
			logerror("SPR: reading register %i (0x452) [%02x]\n",m_video.towns_sprite_sel,m_video.towns_sprite_reg[m_video.towns_sprite_sel]);
			return m_video.towns_sprite_reg[m_video.towns_sprite_sel];
		//default:
			//logerror("VID: read port %04x\n",offset+0x440);
	}
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_video_440_w)
{
	switch(offset)
	{
		case 0x00:
			m_video.towns_crtc_sel = data;
			break;
		case 0x02:
//          logerror("CRTC: writing register %i (0x442) [%02x]\n",towns_crtc_sel,data);
			m_video.towns_crtc_reg[m_video.towns_crtc_sel] =
				(m_video.towns_crtc_reg[m_video.towns_crtc_sel] & 0xff00) | data;
			towns_crtc_refresh_mode();
			break;
		case 0x03:
//          logerror("CRTC: writing register %i (0x443) [%02x]\n",towns_crtc_sel,data);
			m_video.towns_crtc_reg[m_video.towns_crtc_sel] =
				(m_video.towns_crtc_reg[m_video.towns_crtc_sel] & 0x00ff) | (data << 8);
			towns_crtc_refresh_mode();
			break;
		case 0x08:
			m_video.towns_video_sel = data & 0x01;
			break;
		case 0x0a:
			logerror("Video: writing register %i (0x44a) [%02x]\n",m_video.towns_video_sel,data);
			m_video.towns_video_reg[m_video.towns_video_sel] = data;
			break;
		case 0x10:
			m_video.towns_sprite_sel = data & 0x07;
			break;
		case 0x12:
			logerror("SPR: writing register %i (0x452) [%02x]\n",m_video.towns_sprite_sel,data);
			m_video.towns_sprite_reg[m_video.towns_sprite_sel] = data;
			break;
		default:
			logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0x440);
	}
}

READ8_MEMBER(towns_state::towns_video_5c8_r)
{
	//logerror("VID: read port %04x\n",offset+0x5c8);
	switch(offset)
	{
		case 0x00:  // 0x5c8 - disable TVRAM?
		if(m_video.towns_tvram_enable != 0)
		{
			m_video.towns_tvram_enable = 0;
			return 0x80;
		}
		else
			return 0x00;
	}
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_video_5c8_w)
{
	pic8259_device* dev = m_pic_slave;

	switch(offset)
	{
		case 0x02:  // 0x5ca - VSync clear?
			dev->ir3_w(0);
			if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set low\n");
			//towns_vblank_flag = 0;
			break;
	}
	logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0x5c8);
}

void towns_state::towns_update_palette()
{
	UINT8 entry = m_video.towns_palette_select;
	UINT8 r = m_video.towns_palette_r[entry];
	UINT8 g = m_video.towns_palette_g[entry];
	UINT8 b = m_video.towns_palette_b[entry];
	m_palette->set_pen_color(entry, r, g, b);
}

/* Video/CRTC
 *
 * 0xfd90 - palette colour select
 * 0xfd92/4/6 - BRG value
 * 0xfd98-9f  - degipal(?)
 */
READ8_MEMBER(towns_state::towns_video_fd90_r)
{
	UINT8 ret = 0;
	UINT16 xpos;

//    logerror("VID: read port %04x\n",offset+0xfd90);
	switch(offset)
	{
		case 0x00:
			return m_video.towns_palette_select;
		case 0x02:
			return m_video.towns_palette_b[m_video.towns_palette_select];
		case 0x04:
			return m_video.towns_palette_r[m_video.towns_palette_select];
		case 0x06:
			return m_video.towns_palette_g[m_video.towns_palette_select];
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			return m_video.towns_degipal[offset-0x08];
		case 0x10:  // "sub status register"
			// check video position
			xpos = space.machine().first_screen()->hpos();

			if(xpos < m_video.towns_crtc_layerscr[0].max_x && xpos > m_video.towns_crtc_layerscr[0].min_x)
				ret |= 0x02;
			if(m_video.towns_vblank_flag)
				ret |= 0x01;
			return ret;
	}
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_video_fd90_w)
{
	switch(offset)
	{
		case 0x00:
			m_video.towns_palette_select = data;
			break;
		case 0x02:
			m_video.towns_palette_b[m_video.towns_palette_select] = data;
			towns_update_palette();
			break;
		case 0x04:
			m_video.towns_palette_r[m_video.towns_palette_select] = data;
			towns_update_palette();
			break;
		case 0x06:
			m_video.towns_palette_g[m_video.towns_palette_select] = data;
			towns_update_palette();
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			m_video.towns_degipal[offset-0x08] = data;
			m_video.towns_dpmd_flag = 1;
			break;
		case 0x10:
			m_video.towns_layer_ctrl = data;
			break;
	}
	logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0xfd90);
}

READ8_MEMBER(towns_state::towns_video_ff81_r)
{
	return ((m_video.towns_vram_rplane << 6) & 0xc0) | m_video.towns_vram_wplane;
}

WRITE8_MEMBER(towns_state::towns_video_ff81_w)
{
	m_video.towns_vram_wplane = data & 0x0f;
	m_video.towns_vram_rplane = (data & 0xc0) >> 6;
	towns_update_video_banks(space);
	logerror("VGA: VRAM wplane select (I/O) = 0x%02x\n",m_video.towns_vram_wplane);
}

READ8_MEMBER(towns_state::towns_video_unknown_r)
{
	return 0x00;
}

/*
 *  Sprite RAM, low memory
 *  Writing to 0xc8xxx or 0xcaxxx activates TVRAM
 *  Writing to I/O port 0x5c8 disables TVRAM
 *     (bit 7 returns high if TVRAM was previously active)
 *
 *  In TVRAM mode:
 *    0xc8000-0xc8fff: ASCII text (2 bytes each: ISO646 code, then attribute)
 *    0xca000-0xcafff: JIS code
 */
READ8_MEMBER(towns_state::towns_spriteram_low_r)
{
	UINT8* RAM = m_messram->pointer();
	UINT8* ROM = m_user->base();

	if(offset < 0x1000)
	{  // 0xc8000-0xc8fff
		if(m_towns_mainmem_enable == 0)
		{
//          if(towns_tvram_enable == 0)
//              return towns_sprram[offset];
//          else
				return m_towns_txtvram[offset];
		}
		else
			return RAM[offset + 0xc8000];
	}
	if(offset >= 0x1000 && offset < 0x2000)
	{  // 0xc9000-0xc9fff
		return RAM[offset + 0xc9000];
	}
	if(offset >= 0x2000 && offset < 0x3000)
	{  // 0xca000-0xcafff
		if(m_towns_mainmem_enable == 0)
		{
			if(m_towns_ankcg_enable != 0 && offset < 0x2800)
				return ROM[0x180000 + 0x3d000 + (offset-0x2000)];
//          if(towns_tvram_enable == 0)
//              return m_towns_sprram[offset];
//          else
				return m_towns_txtvram[offset];
		}
		else
			return RAM[offset + 0xca000];
	}
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_spriteram_low_w)
{
	UINT8* RAM = m_messram->pointer();

	if(offset < 0x1000)
	{  // 0xc8000-0xc8fff
		m_video.towns_tvram_enable = 1;
		if(m_towns_mainmem_enable == 0)
			m_towns_txtvram[offset] = data;
		else
			RAM[offset + 0xc8000] = data;
	}
	if(offset >= 0x1000 && offset < 0x2000)
	{
		RAM[offset + 0xc9000] = data;
	}
	if(offset >= 0x2000 && offset < 0x3000)
	{  // 0xca000-0xcafff
		m_video.towns_tvram_enable = 1;
		if(m_towns_mainmem_enable == 0)
			m_towns_txtvram[offset] = data;
		else
			RAM[offset + 0xca000] = data;
	}
}

READ8_MEMBER( towns_state::towns_spriteram_r )
{
	return m_towns_txtvram[offset];
}

WRITE8_MEMBER( towns_state::towns_spriteram_w )
{
	m_towns_txtvram[offset] = data;
}

/*
 *  Sprites
 *
 *  Max. 1024, 16x16, 16 colours per sprite
 *  128kB Sprite RAM (8kB attributes, 120kB pattern/colour data)
 *  Sprites are rendered directly to VRAM layer 1 (VRAM offset 0x40000 or 0x60000)
 *
 *  Sprite RAM format:
 *      4 words per sprite
 *      +0: X position (10-bit)
 *      +2: Y position (10-bit)
 *      +4: Sprite Attribute
 *          bit 15: enforce offsets (regs 2-5)
 *          bit 12,13: flip sprite
 *          bits 10-0: Sprite RAM offset containing sprite pattern
 *          TODO: other attributes (zoom?)
 *      +6: Sprite Colour
 *          bit 15: use colour data in located in sprite RAM offset in bits 11-0 (x32)
 */
void towns_state::render_sprite_4(UINT32 poffset, UINT32 coffset, UINT16 x, UINT16 y, UINT16 xflip, UINT16 yflip, const rectangle* rect)
{
	UINT16 xpos,ypos;
	UINT16 col,pixel;
	UINT32 voffset;
	UINT16 xstart,xend,ystart,yend;
	int xdir,ydir;
	int width = (m_video.towns_crtc_reg[12] - m_video.towns_crtc_reg[11]) / (((m_video.towns_crtc_reg[27] & 0x0f00) >> 8)+1);
	int height = (m_video.towns_crtc_reg[16] - m_video.towns_crtc_reg[15]) / (((m_video.towns_crtc_reg[27] & 0xf000) >> 12)+2);

	if(xflip)
	{
		xstart = x+14;
		xend = x-2;
		xdir = -2;
	}
	else
	{
		xstart = x+1;
		xend = x+17;
		xdir = 2;
	}
	if(yflip)
	{
		ystart = y+15;
		yend = y-1;
		ydir = -1;
	}
	else
	{
		ystart = y;
		yend = y+16;
		ydir = 1;
	}
	xstart &= 0x1ff;
	xend &= 0x1ff;
	ystart &= 0x1ff;
	yend &= 0x1ff;
	poffset &= 0x1ffff;

	for(ypos=ystart;ypos!=yend;ypos+=ydir,ypos&=0x1ff)
	{
		for(xpos=xstart;xpos!=xend;xpos+=xdir,xpos&=0x1ff)
		{
			if(m_video.towns_sprite_page != 0)
				voffset = 0x20000;
			else
				voffset = 0x00000;
			pixel = (m_towns_txtvram[poffset] & 0xf0) >> 4;
			col = m_towns_txtvram[coffset+(pixel*2)] | (m_towns_txtvram[coffset+(pixel*2)+1] << 8);
			voffset += (m_video.towns_crtc_reg[24] * 4) * (ypos & 0x1ff);  // scanline size in bytes * y pos
			voffset += (xpos & 0x1ff) * 2;
			if((m_video.towns_sprite_page != 0 && voffset > 0x1ffff && voffset < 0x40000)
					|| (m_video.towns_sprite_page == 0 && voffset < 0x20000))
			{
				if(xpos < width && ypos < height && pixel != 0)
				{
					m_towns_gfxvram[0x40000+voffset+1] = (col & 0xff00) >> 8;
					m_towns_gfxvram[0x40000+voffset] = col & 0x00ff;
				}
			}
			if(xflip)
				voffset+=2;
			else
				voffset-=2;
			pixel = m_towns_txtvram[poffset] & 0x0f;
			col = m_towns_txtvram[coffset+(pixel*2)] | (m_towns_txtvram[coffset+(pixel*2)+1] << 8);
			if((m_video.towns_sprite_page != 0 && voffset > 0x1ffff && voffset < 0x40000)
					|| (m_video.towns_sprite_page == 0 && voffset < 0x20000))
			{
				if(xpos < width && ypos < height && pixel != 0)
				{
					m_towns_gfxvram[0x40000+voffset+1] = (col & 0xff00) >> 8;
					m_towns_gfxvram[0x40000+voffset] = col & 0x00ff;
				}
			}
			poffset++;
			poffset &= 0x1ffff;
		}
	}
}

void towns_state::render_sprite_16(UINT32 poffset, UINT16 x, UINT16 y, UINT16 xflip, UINT16 yflip, const rectangle* rect)
{
	UINT16 xpos,ypos;
	UINT16 col;
	UINT32 voffset;
	UINT16 xstart,ystart,xend,yend;
	int xdir,ydir;
	int width = (m_video.towns_crtc_reg[12] - m_video.towns_crtc_reg[11]) / (((m_video.towns_crtc_reg[27] & 0x0f00) >> 8)+1);
	int height = (m_video.towns_crtc_reg[16] - m_video.towns_crtc_reg[15]) / (((m_video.towns_crtc_reg[27] & 0xf000) >> 12)+2);

	if(xflip)
	{
		xstart = x+16;
		xend = x;
		xdir = -1;
	}
	else
	{
		xstart = x+1;
		xend = x+17;
		xdir = 1;
	}
	if(yflip)
	{
		ystart = y+15;
		yend = y-1;
		ydir = -1;
	}
	else
	{
		ystart = y;
		yend = y+16;
		ydir = 1;
	}
	xstart &= 0x1ff;
	xend &= 0x1ff;
	ystart &= 0x1ff;
	yend &= 0x1ff;
	poffset &= 0x1ffff;

	for(ypos=ystart;ypos!=yend;ypos+=ydir,ypos&=0x1ff)
	{
		for(xpos=xstart;xpos!=xend;xpos+=xdir,xpos&=0x1ff)
		{
			if(m_video.towns_sprite_page != 0)
				voffset = 0x20000;
			else
				voffset = 0x00000;
			col = m_towns_txtvram[poffset] | (m_towns_txtvram[poffset+1] << 8);
			voffset += (m_video.towns_crtc_reg[24] * 4) * (ypos & 0x1ff);  // scanline size in bytes * y pos
			voffset += (xpos & 0x1ff) * 2;
			if((m_video.towns_sprite_page != 0 && voffset > 0x1ffff && voffset < 0x40000)
					|| (m_video.towns_sprite_page == 0 && voffset < 0x20000))
			{
				if(xpos < width && ypos < height && col < 0x8000)
				{
					m_towns_gfxvram[0x40000+voffset+1] = (col & 0xff00) >> 8;
					m_towns_gfxvram[0x40000+voffset] = col & 0x00ff;
				}
			}
			poffset+=2;
			poffset &= 0x1ffff;
		}
	}
}

void towns_state::draw_sprites(const rectangle* rect)
{
	UINT16 sprite_limit = (m_video.towns_sprite_reg[0] | (m_video.towns_sprite_reg[1] << 8)) & 0x3ff;
	int n;
	UINT16 x,y,attr,colour;
	UINT16 xoff = (m_video.towns_sprite_reg[2] | (m_video.towns_sprite_reg[3] << 8)) & 0x1ff;
	UINT16 yoff = (m_video.towns_sprite_reg[4] | (m_video.towns_sprite_reg[5] << 8)) & 0x1ff;
	UINT32 poffset,coffset;

	if(!(m_video.towns_sprite_reg[1] & 0x80))
		return;

	// clears VRAM for each frame?
	if(m_video.towns_sprite_page == 0)
		memset(m_towns_gfxvram+0x40000,0x80,0x20000);
	else
		memset(m_towns_gfxvram+0x60000,0x80,0x20000);

	for(n=sprite_limit;n<1024;n++)
	{
		x = m_towns_txtvram[8*n] | (m_towns_txtvram[8*n+1] << 8);
		y = m_towns_txtvram[8*n+2] | (m_towns_txtvram[8*n+3] << 8);
		attr = m_towns_txtvram[8*n+4] | (m_towns_txtvram[8*n+5] << 8);
		colour = m_towns_txtvram[8*n+6] | (m_towns_txtvram[8*n+7] << 8);
		if(attr & 0x8000)
		{
			x += xoff;
			y += yoff;
		}
		x &= 0x1ff;
		y &= 0x1ff;

		if(colour & 0x8000)
		{
			poffset = (attr & 0x3ff) << 7;
			coffset = (colour & 0xfff) << 5;
#ifdef SPR_DEBUG
			printf("Sprite4 #%i, X %i Y %i Attr %04x Col %04x Poff %08x Coff %08x\n",
				n,x,y,attr,colour,poffset,coffset);
#endif
			if(!(colour & 0x2000))
				render_sprite_4((poffset)&0x1ffff,coffset,x,y,attr&0x2000,attr&0x1000,rect);
		}
		else
		{
			poffset = (attr & 0x3ff) << 7;
#ifdef SPR_DEBUG
			printf("Sprite16 #%i, X %i Y %i Attr %04x Col %04x Poff %08x",
				n,x,y,attr,colour,poffset);
#endif
			if(!(colour & 0x2000))
				render_sprite_16((poffset)&0x1ffff,x,y,attr&0x2000,attr&0x1000,rect);
		}
	}

	if(m_video.towns_sprite_page == 0)  // flip VRAM page
		m_video.towns_sprite_page = 1;
	else
		m_video.towns_sprite_page = 0;

	m_video.towns_sprite_flag = 1;  // we are now drawing
	m_video.sprite_timer->adjust(m_maincpu->cycles_to_attotime(128 * (1025-sprite_limit)));
}

void towns_state::towns_crtc_draw_scan_layer_hicolour(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline)
{
	UINT32 off = 0;
	int x;
	UINT16 colour;
	int hzoom = 1;
	int linesize;
	UINT32 scroll;

	if(layer == 0)
		linesize = m_video.towns_crtc_reg[20] * 4;
	else
		linesize = m_video.towns_crtc_reg[24] * 4;

	if(m_video.towns_display_page_sel != 0)
		off = 0x20000;

//  if((layer == 1) && (m_video.towns_sprite_reg[1] & 0x80) && (m_video.towns_sprite_page == 1))
//      off = 0x20000;

	if(layer != 0)
	{
		if(!(m_video.towns_video_reg[0] & 0x10))
			return;
		if(!(m_video.towns_crtc_reg[28] & 0x10))
			off += (m_video.towns_crtc_reg[21]) << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[21] & 0xfc00) << 2) | (((m_video.towns_crtc_reg[21] & 0x3ff) << 2));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[22]);
		hzoom = ((m_video.towns_crtc_reg[27] & 0x0f00) >> 8) + 1;
	}
	else
	{
		if(!(m_video.towns_crtc_reg[28] & 0x20))
			off += (m_video.towns_crtc_reg[17]) << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[17] & 0xfc00) << 2) | (((m_video.towns_crtc_reg[17] & 0x3ff) << 2));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]);
		hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
	}

	off += line * linesize;
	off &= ~0x01;

	if(hzoom == 1)
	{
		for(x=rect->min_x;x<rect->max_x;x++)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer

			colour = (m_towns_gfxvram[off+(layer*0x40000)+1] << 8) | m_towns_gfxvram[off+(layer*0x40000)];
			if(colour < 0x8000)
			{
				bitmap.pix32(scanline, x) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
			}
			off+=2;
		}
	}

	if(hzoom == 2)
	{  // x2 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=2)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = (m_towns_gfxvram[off+(layer*0x40000)+1] << 8) | m_towns_gfxvram[off+(layer*0x40000)];
			if(colour < 0x8000)
			{
				bitmap.pix32(scanline, x) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+1) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
			}
			off+=2;
		}
	}

	if(hzoom == 3)
	{  // x3 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=3)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = (m_towns_gfxvram[off+(layer*0x40000)+1] << 8) | m_towns_gfxvram[off+(layer*0x40000)];
			if(colour < 0x8000)
			{
				bitmap.pix32(scanline, x) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+1) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+2) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
			}
			off+=2;
		}
	}

	if(hzoom == 4)
	{  // x4 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=4)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = (m_towns_gfxvram[off+(layer*0x40000)+1] << 8) | m_towns_gfxvram[off+(layer*0x40000)];
			if(colour < 0x8000)
			{
				bitmap.pix32(scanline, x) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+1) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+2) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+3) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
			}
			off+=2;
		}
	}

	if(hzoom == 5)
	{  // x5 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=5)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = (m_towns_gfxvram[off+(layer*0x40000)+1] << 8) | m_towns_gfxvram[off+(layer*0x40000)];
			if(colour < 0x8000)
			{
				bitmap.pix32(scanline, x) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+1) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+2) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+3) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
				bitmap.pix32(scanline, x+4) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
			}
			off+=2;
		}
	}
}

void towns_state::towns_crtc_draw_scan_layer_256(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline)
{
	int off = 0;
	int x;
	UINT8 colour;
	int hzoom = 1;
	int linesize;
	UINT32 scroll;

	if(m_video.towns_display_page_sel != 0)
		off = 0x20000;

//  if((layer == 1) && (m_video.towns_sprite_reg[1] & 0x80) && (m_video.towns_sprite_page == 1))
//      off = 0x20000;

	if(layer == 0)
		linesize = m_video.towns_crtc_reg[20] * 8;
	else
		linesize = m_video.towns_crtc_reg[24] * 8;

	if(layer != 0)
	{
		if(!(m_video.towns_video_reg[0] & 0x10))
			return;
		if(!(m_video.towns_crtc_reg[28] & 0x10))
			off += m_video.towns_crtc_reg[21] << 3;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[21] & 0xfc00) << 3) | (((m_video.towns_crtc_reg[21] & 0x3ff) << 3));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[22]);
		hzoom = ((m_video.towns_crtc_reg[27] & 0x0f00) >> 8) + 1;
	}
	else
	{
		if(!(m_video.towns_crtc_reg[28] & 0x20))
			off += m_video.towns_crtc_reg[17] << 3;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[17] & 0xfc00) << 3) | (((m_video.towns_crtc_reg[17] & 0x3ff) << 3));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]);
		hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
	}

	off += line * linesize;

	if(hzoom == 1)
	{
		for(x=rect->min_x;x<rect->max_x;x++)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)];
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 2)
	{  // x2 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=2)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)+1];
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 3)
	{  // x3 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=3)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)+1];
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 4)
	{  // x4 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=4)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)+1];
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 5)
	{  // x5 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=5)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)+1];
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+4) = m_palette->pen(colour);
			}
			off++;
		}
	}
}

void towns_state::towns_crtc_draw_scan_layer_16(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline)
{
	int off = 0;
	int x;
	UINT8 colour;
	int hzoom = 1;
	int linesize;
	UINT32 scroll;

	if(m_video.towns_display_page_sel != 0)
		off = 0x20000;

//  if((layer == 1) && (m_video.towns_sprite_reg[1] & 0x80) && (m_video.towns_sprite_page == 1))
//      off = 0x20000;

	if(layer == 0)
		linesize = m_video.towns_crtc_reg[20] * 4;
	else
		linesize = m_video.towns_crtc_reg[24] * 4;

	if(layer != 0)
	{
		if(!(m_video.towns_video_reg[0] & 0x10))
			return;
		if(!(m_video.towns_crtc_reg[28] & 0x10))
			off += m_video.towns_crtc_reg[21];  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[21] & 0xfc00)<<2) | (((m_video.towns_crtc_reg[21] & 0x3ff)<<2));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[22]);
		hzoom = ((m_video.towns_crtc_reg[27] & 0x0f00) >> 8) + 1;
	}
	else
	{
		if(!(m_video.towns_crtc_reg[28] & 0x20))
			off += m_video.towns_crtc_reg[17];  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[17] & 0xfc00)<<2) | (((m_video.towns_crtc_reg[17] & 0x3ff)<<2));
			off += scroll;
		}
		off += (m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]);
		hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
	}

	off += line * linesize;

	if(hzoom == 1)
	{
		for(x=rect->min_x;x<rect->max_x;x+=2)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
			}
			colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 2)
	{  // x2 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=4)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
			}
			colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 3)
	{  // x3 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=6)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+4) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+5) = m_palette->pen(colour);
			}
			colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 4)
	{  // x4 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=8)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x+4) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+5) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+6) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+7) = m_palette->pen(colour);
			}
			colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
			}
			off++;
		}
	}

	if(hzoom == 5)
	{  // x5 horizontal zoom
		for(x=rect->min_x;x<rect->max_x;x+=10)
		{
			if(m_video.towns_video_reg[0] & 0x10)
				off &= 0x3ffff;  // 2 layers
			else
				off &= 0x7ffff;  // 1 layer
			colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x+5) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+6) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+7) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+8) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+9) = m_palette->pen(colour);
			}
			colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
			if(colour != 0)
			{
				bitmap.pix32(scanline, x) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+1) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+2) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+3) = m_palette->pen(colour);
				bitmap.pix32(scanline, x+4) = m_palette->pen(colour);
			}
			off++;
		}
	}
}

void towns_state::towns_crtc_draw_layer(bitmap_rgb32 &bitmap,const rectangle* rect,int layer)
{
	int line;
	int scanline;
	int height;

	if(layer == 0)
	{
		scanline = rect->min_y;
		height = (rect->max_y - rect->min_y);
		if(m_video.towns_crtc_reg[27] & 0x0010)
			height /= 2;
		switch(m_video.towns_video_reg[0] & 0x03)
		{
			case 0x01:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x0010)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
			case 0x02:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_256(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x0010)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_256(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
			case 0x03:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x0010)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
		}
	}
	else
	{
		scanline = rect->min_y;
		height = (rect->max_y - rect->min_y);
		if(m_video.towns_crtc_reg[27] & 0x1000)
			height /= 2;
		switch(m_video.towns_video_reg[0] & 0x0c)
		{
			case 0x04:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x1000)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
			case 0x08:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_256(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x1000)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_256(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
			case 0x0c:
				for(line=0;line<height;line++)
				{
					towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
					scanline++;
					if(m_video.towns_crtc_reg[27] & 0x1000)  // vertical zoom
					{
						towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
						scanline++;
					}
				}
				break;
		}
	}
}

void towns_state::render_text_char(UINT8 x, UINT8 y, UINT8 ascii, UINT16 jis, UINT8 attr)
{
	UINT32 rom_addr;
	UINT32 vram_addr;
	UINT16 linesize = m_video.towns_crtc_reg[24] * 4;
	UINT8 code_h,code_l;
	UINT8 colour;
	UINT8 data;
	UINT8 temp;
	UINT8* font_rom = m_user->base();
	int a,b;

	// all characters are 16 pixels high
	vram_addr = (x * 4) + (y * (linesize * 16));

	if((attr & 0xc0) == 0)
		rom_addr = 0x3d800 + (ascii * 128);
	else
	{
		code_h = (jis & 0xff00) >> 8;
		code_l = jis & 0x00ff;
		if(code_h < 0x30)
		{
			rom_addr = ((code_l & 0x1f) << 4)
								| (((code_l - 0x20) & 0x20) << 8)
								| (((code_l - 0x20) & 0x40) << 6)
								| ((code_h & 0x07) << 9);
		}
		else if(code_h < 0x70)
		{
			rom_addr = ((code_l & 0x1f) << 4)
								+ (((code_l - 0x20) & 0x60) << 8)
								+ ((code_h & 0x0f) << 9)
								+ (((code_h - 0x30) & 0x70) * 0xc00)
								+ 0x8000;
		}
		else
		{
			rom_addr = ((code_l & 0x1f) << 4)
								| (((code_l - 0x20) & 0x20) << 8)
								| (((code_l - 0x20) & 0x40) << 6)
								| ((code_h & 0x07) << 9)
								| 0x38000;
		}
	}
	colour = attr & 0x07;
	if(attr & 0x20)
		colour |= 0x08;

	for(a=0;a<16;a++)  // for each scanline
	{
		if((attr & 0xc0) == 0)
			data = font_rom[0x180000 + rom_addr + a];
		else
		{
			if((attr & 0xc0) == 0x80)
				data = font_rom[0x180000 + rom_addr + (a*2)];
			else
				data = font_rom[0x180000 + rom_addr + (a*2) + 1];
		}

		if(attr & 0x08)
			data = ~data;  // inverse

		// and finally, put the data in VRAM
		for(b=0;b<8;b+=2)
		{
			temp = 0;
			if(data & (1<<b))
				temp |= ((colour & 0x0f) << 4);
			if(data & (1<<(b+1)))
				temp |= (colour & 0x0f);
			//m_towns_gfxvram[0x40000+vram_addr+(b/2)] = temp;
		}

		vram_addr += linesize;
		vram_addr &= 0x3ffff;
	}
}

void towns_state::draw_text_layer()
{
/*
 *  Text format
 *  2 bytes per character at both 0xc8000 and 0xca000
 *  0xc8xxx: Byte 1: ASCII character
 *           Byte 2: Attributes
 *             bits 2-0: GRB (or is it BRG?)
 *             bit 3: Inverse
 *             bit 4: Blink
 *             bit 5: high brightness
 *             bits 7-6: Kanji high/low
 *
 *  If either bits 6 or 7 are high, then a fullwidth Kanji character is displayed
 *  at this location.  The character displayed is represented by a 2-byte
 *  JIS code at the same offset at 0xca000.
 *
 *  The video hardware renders text to VRAM layer 1, there is no separate text layer
 */
	int x,y,c = 0;

	for(y=0;y<40;y++)
	{
		for(x=0;x<80;x++)
		{
			render_text_char(x,y,m_towns_txtvram[c],((m_towns_txtvram[c+0x2000] << 8)|(m_towns_txtvram[c+0x2001])),m_towns_txtvram[c+1]);
			c+=2;
		}
	}
}

TIMER_CALLBACK_MEMBER(towns_state::towns_sprite_done)
{
	// sprite drawing is complete, lower flag
	m_video.towns_sprite_flag = 0;
	if(m_video.towns_sprite_page != 0)
		m_video.towns_crtc_reg[21] |= 0x8000;
	else
		m_video.towns_crtc_reg[21] &= ~0x8000;
}

TIMER_CALLBACK_MEMBER(towns_state::towns_vblank_end)
{
	// here we'll clear the vsync signal, I presume it goes low on it's own eventually
	device_t* dev = (device_t*)ptr;
	downcast<pic8259_device *>(dev)->ir3_w(0);  // IRQ11 = VSync
	if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set low\n");
	m_video.towns_vblank_flag = 0;
}

INTERRUPT_GEN_MEMBER(towns_state::towns_vsync_irq)
{
	pic8259_device* dev = m_pic_slave;
	dev->ir3_w(1);  // IRQ11 = VSync
	if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set high\n");
	m_video.towns_vblank_flag = 1;
	machine().scheduler().timer_set(machine().first_screen()->time_until_vblank_end(), timer_expired_delegate(FUNC(towns_state::towns_vblank_end),this), 0, (void*)dev);
	if(m_video.towns_tvram_enable)
		draw_text_layer();
	if(m_video.towns_sprite_reg[1] & 0x80)
		draw_sprites(&m_video.towns_crtc_layerscr[1]);
}

void towns_state::video_start()
{
	m_video.towns_vram_wplane = 0x00;
	m_video.towns_sprite_page = 0;
	m_video.sprite_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(towns_state::towns_sprite_done),this));
}

UINT32 towns_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x00000000, cliprect);

	if(!(m_video.towns_video_reg[1] & 0x01))
	{
		if(!machine().input().code_pressed(KEYCODE_Q))
		{
			if((m_video.towns_layer_ctrl & 0x03) != 0)
				towns_crtc_draw_layer(bitmap,&m_video.towns_crtc_layerscr[1],1);
		}
		if(!machine().input().code_pressed(KEYCODE_W))
		{
			if((m_video.towns_layer_ctrl & 0x0c) != 0)
				towns_crtc_draw_layer(bitmap,&m_video.towns_crtc_layerscr[0],0);
		}
	}
	else
	{
		if(!machine().input().code_pressed(KEYCODE_Q))
		{
			if((m_video.towns_layer_ctrl & 0x0c) != 0)
				towns_crtc_draw_layer(bitmap,&m_video.towns_crtc_layerscr[0],0);
		}
		if(!machine().input().code_pressed(KEYCODE_W))
		{
			if((m_video.towns_layer_ctrl & 0x03) != 0)
				towns_crtc_draw_layer(bitmap,&m_video.towns_crtc_layerscr[1],1);
		}
	}

#if 0
#ifdef SPR_DEBUG
	if(machine().input().code_pressed(KEYCODE_O))
		pshift+=0x80;
	if(machine().input().code_pressed(KEYCODE_I))
		pshift-=0x80;
	popmessage("Pixel shift = %08x",pshift);
#endif
#endif

#ifdef CRTC_REG_DISP
	popmessage("CRTC: %i %i %i %i %i %i %i %i %i\n%i %i %i %i | %i %i %i %i\n%04x %i %i %i | %04x %i %i %i\nZOOM: %04x\nVideo: %02x %02x\nText=%i Spr=%02x\nReg28=%04x",
		m_video.towns_crtc_reg[0],m_video.towns_crtc_reg[1],m_video.towns_crtc_reg[2],m_video.towns_crtc_reg[3],
		m_video.towns_crtc_reg[4],m_video.towns_crtc_reg[5],m_video.towns_crtc_reg[6],m_video.towns_crtc_reg[7],
		m_video.towns_crtc_reg[8],
		m_video.towns_crtc_reg[9],m_video.towns_crtc_reg[10],m_video.towns_crtc_reg[11],m_video.towns_crtc_reg[12],
		m_video.towns_crtc_reg[13],m_video.towns_crtc_reg[14],m_video.towns_crtc_reg[15],m_video.towns_crtc_reg[16],
		m_video.towns_crtc_reg[17],m_video.towns_crtc_reg[18],m_video.towns_crtc_reg[19],m_video.towns_crtc_reg[20],
		m_video.towns_crtc_reg[21],m_video.towns_crtc_reg[22],m_video.towns_crtc_reg[23],m_video.towns_crtc_reg[24],
		m_video.towns_crtc_reg[27],m_video.towns_video_reg[0],m_video.towns_video_reg[1],m_video.towns_tvram_enable,m_video.towns_sprite_reg[1] & 0x80,
		m_video.towns_crtc_reg[28]);
#endif

	return 0;
}
