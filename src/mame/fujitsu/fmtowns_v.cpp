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
 *  5:  VSync period 1
 *  6:  VSync period 2
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
 *  17: Graphic layer 0 frame start address
 *  18: Graphic layer 0 horizontal adjust
 *  19: Graphic layer 0 field indirect address offset
 *  20: Graphic layer 0 line indirect address offset
 *
 *  21-24: As above, but for Graphic layer 1
 *
 *  27: Layer zoom.     bits 0-3 = horizontal zoom layer 0
 *  (0 = x1, 1 = x2,    bits 4-7 = vertical zoom layer 0
 *    2 = x3...)        bits 8-11 = horizontal zoom layer 1
 *                      bits 12-15 = vertical zoom layer 1
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
 *  0:  Control register:
 *      bit 4 = PMODE - enable 2 layers
 *      bits 2-3 = layer 1 mode
 *      bits 0-1 = layer 0 mode
 *          mode: 1 = 16 colours (PMODE=1 only), 2 = 256 colours (PMODE=0 only),
 *                3 = highcolour (16-bit), 0 = display off
 *
 *  1:  Priority register (bit 0)
 *      bit 0: PR1 - layer priority (0 = layer 0 before)
 *      bit 2: YM - screen brightness (0 = high brightness)
 *      bit 3: YS - 0 = valid.  Used for 256 colour external sync
 *      bits 4-5: Palette select
 *        0 = layer 0, 16 colours, 1 or 3 = 256 colours
 *        2 = layer 1, 16 colours
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
#include "fmtowns.h"

#include "machine/pic8259.h"
#include "machine/ram.h"
#include "screen.h"


//#define CRTC_REG_DISP 1
//#define SPR_DEBUG 1
#define LAYER_DISABLE 0  // for debugging, allow the Q and W keys to be used for disabling graphic layers
#define LOG_VID 0

//static uint32_t pshift;  // for debugging

void towns_state::towns_crtc_refresh_mode()
{
	rectangle scr(0, m_video.towns_crtc_reg[4] - m_video.towns_crtc_reg[0], 0, m_video.towns_crtc_reg[8] / 2);

	// layer 0
	m_video.towns_crtc_layerscr[0].min_x = m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[0];
	m_video.towns_crtc_layerscr[0].min_y = (m_video.towns_crtc_reg[13] - m_video.towns_crtc_reg[6]) / 2;
	m_video.towns_crtc_layerscr[0].max_x = m_video.towns_crtc_reg[10] - m_video.towns_crtc_reg[0];
	m_video.towns_crtc_layerscr[0].max_y = ((m_video.towns_crtc_reg[14] - m_video.towns_crtc_reg[6]) / 2) - 1;

	// layer 1
	m_video.towns_crtc_layerscr[1].min_x = m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[0];
	m_video.towns_crtc_layerscr[1].min_y = (m_video.towns_crtc_reg[15] - m_video.towns_crtc_reg[6]) / 2;
	m_video.towns_crtc_layerscr[1].max_x = m_video.towns_crtc_reg[12] - m_video.towns_crtc_reg[0];
	m_video.towns_crtc_layerscr[1].max_y = ((m_video.towns_crtc_reg[16] - m_video.towns_crtc_reg[6]) / 2) - 1;

	// sanity checks
	if(scr.max_x == 0 || scr.max_y == 0)
		return;
	if(scr.max_x <= scr.min_x || scr.max_y <= scr.min_y)
		return;

	m_screen->configure(scr.max_x+1,scr.max_y+1,scr,HZ_TO_ATTOSECONDS(60));
}

uint8_t towns_state::towns_gfx_high_r(offs_t offset)
{
	return m_towns_gfxvram[offset];
}

void towns_state::towns_gfx_high_w(offs_t offset, uint8_t data)
{
	u8 mask = m_vram_mask[offset & 3];
	u8 mem = m_towns_gfxvram[offset];
	m_towns_gfxvram[offset] = (mem & ~mask) | (data & mask);
}

uint8_t towns_state::towns_gfx_packed_r(offs_t offset)
{
	return m_towns_gfxvram[bitswap<19>(offset,2,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,1,0)];
}

void towns_state::towns_gfx_packed_w(offs_t offset, uint8_t data)
{
	u8 mask = m_vram_mask[offset & 3];
	offset = bitswap<19>(offset,2,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,1,0);
	u8 mem = m_towns_gfxvram[offset];
	m_towns_gfxvram[offset] = (mem & ~mask) | (data & mask);
}


uint8_t towns_state::towns_gfx_r(offs_t offset)
{
	uint8_t ret = 0;

	if(m_towns_mainmem_enable != 0)
		return m_ram->pointer()[offset+0xc0000];

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

void towns_state::towns_gfx_w(offs_t offset, uint8_t data)
{
	if(m_towns_mainmem_enable != 0)
	{
		m_ram->pointer()[offset+0xc0000] = data;
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
		m_video.towns_kanji_offset = (((m_video.towns_kanji_code_l & 0x1f) << 5)
							+ (((m_video.towns_kanji_code_l - 0x20) & 0x60) << 9)
							+ ((m_video.towns_kanji_code_h & 0x0f) << 10)
							+ (((m_video.towns_kanji_code_h - 0x30) & 0x70) * 0xc00)
							+ 0x8000) >> 1;
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


uint8_t towns_state::towns_video_cff80_r(offs_t offset)
{
	uint8_t const* const ROM = m_user->base();

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
			{
				uint8_t ret = 0x10;
				uint16_t const xpos = m_screen->hpos();
				if(xpos < m_video.towns_crtc_layerscr[0].max_x && xpos > m_video.towns_crtc_layerscr[0].min_x)
					ret |= 0x80;
				if(m_video.towns_vblank_flag != 0)
					ret |= 0x04;
				return ret;
			}
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

void towns_state::towns_video_cff80_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x00:  // mix register
			m_video.towns_crtc_mix = data;
			break;
		case 0x01:  // read/write plane select (bit 0-3 write, bit 6-7 read)
			m_video.towns_vram_wplane = data & 0x0f;
			m_video.towns_vram_rplane = (data & 0xc0) >> 6;
			towns_update_video_banks();
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
			//if(LOG_VID) logerror("VID: Kanji code set (high) = %02x %02x\n",towns_kanji_code_h,towns_kanji_code_l);
			break;
		case 0x15:  // Kanji offset (low)
			m_video.towns_kanji_code_l = data & 0x7f;
			towns_update_kanji_offset();
			//if(LOG_VID) logerror("VID: Kanji code set (low) = %02x %02x\n",towns_kanji_code_h,towns_kanji_code_l);
			break;
		case 0x19:  // ANK CG ROM
			m_towns_ankcg_enable = data & 0x01;
			towns_update_video_banks();
			break;
		default:
			logerror("VID: write %08x to invalid or unimplemented memory-mapped port %05x\n",data,0xcff80+offset);
	}
}

uint8_t towns_state::towns_video_cff80_mem_r(offs_t offset)
{
	if(m_towns_mainmem_enable != 0)
		return m_ram->pointer()[offset+0xcff80];

	return towns_video_cff80_r(offset);
}

void towns_state::towns_video_cff80_mem_w(offs_t offset, uint8_t data)
{
	if(m_towns_mainmem_enable != 0)
	{
		m_ram->pointer()[offset+0xcff80] = data;
		return;
	}
	towns_video_cff80_w(offset,data);
}

/*
 *  port 0x440-0x443 - CRTC
 *      0x440 = register select
 *      0x442/3 = register data (16-bit)
 *      0x448 = shifter register select
 *      0x44a = shifter register data (8-bit)
 *
 */
uint8_t towns_state::towns_video_440_r(offs_t offset)
{
	uint8_t ret = 0;
	uint16_t xpos,ypos;

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
				xpos = m_screen->hpos();
				ypos = m_screen->vpos();

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
			if(LOG_VID) logerror("Video: reading register %i (0x44a) [%02x]\n",m_video.towns_video_sel,m_video.towns_video_reg[m_video.towns_video_sel]);
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
			if(LOG_VID) logerror("SPR: reading register %i (0x452) [%02x]\n",m_video.towns_sprite_sel,m_video.towns_sprite_reg[m_video.towns_sprite_sel]);
			if(m_video.towns_sprite_sel == 6)
				return m_video.towns_sprite_page & 0x01 ? 0x10 : 0;
			return m_video.towns_sprite_reg[m_video.towns_sprite_sel];
		case 0x18:
			return m_vram_mask_addr;
		case 0x1a:
		case 0x1b:
		{
			int idx = (m_vram_mask_addr << 1) + offset - 0x1a;
			return m_vram_mask[idx];
		}
		//default:
			//if(LOG_VID) logerror("VID: read port %04x\n",offset+0x440);
	}
	return 0x00;
}

void towns_state::towns_video_440_w(offs_t offset, uint8_t data)
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
			if((m_video.towns_crtc_sel == 21) && (m_video.towns_sprite_reg[1] & 0x80))
			{
				m_video.towns_crtc_reg[m_video.towns_crtc_sel] =
					(m_video.towns_crtc_reg[m_video.towns_crtc_sel] & 0x80ff) | ((data & 0x7f) << 8);
				return;
			}
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
			if(m_video.towns_sprite_sel == 6)
				m_video.towns_sprite_page = data & 0x80 ? 1 : 0;
			else
				m_video.towns_sprite_reg[m_video.towns_sprite_sel] = data;
			break;
		case 0x18:
			m_vram_mask_addr = data & 1;
			break;
		case 0x1a:
		case 0x1b:
		{
			int idx = (m_vram_mask_addr << 1) + offset - 0x1a;
			m_vram_mask[idx] = data;
			break;
		}
		default:
			if(LOG_VID) logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0x440);
	}
}

uint8_t towns_state::towns_video_5c8_r(offs_t offset)
{
	//if(LOG_VID) logerror("VID: read port %04x\n",offset+0x5c8);
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

void towns_state::towns_video_5c8_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x02:  // 0x5ca - VSync clear?
			m_pic_slave->ir3_w(0);
			if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set low\n");
			//towns_vblank_flag = 0;
			break;
	}
	if(LOG_VID) logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0x5c8);
}

void towns_state::towns_update_palette()
{
	uint8_t entry = m_video.towns_palette_select;
	uint8_t r = m_video.towns_palette_r[entry];
	uint8_t g = m_video.towns_palette_g[entry];
	uint8_t b = m_video.towns_palette_b[entry];
	switch(m_video.towns_video_reg[1] & 0x30)  // Palette select
	{
		case 0x00:
		case 0x20:
			m_palette16[(m_video.towns_video_reg[1] & 0x20) >> 5]->set_pen_color(entry & 0x0f, r, g, b);
			break;
		case 0x10:
		case 0x30:
			m_palette->set_pen_color(entry, r, g, b);
			break;
	}
	// chasehq wants the + 8, the real hardware appears to be less consistent, revisit if other software doesn't like it
	if(!m_screen->vblank())
		m_screen->update_partial(m_screen->vpos() + 4);
}

/* Video/CRTC
 *
 * 0xfd90 - palette colour select
 * 0xfd92/4/6 - BRG value
 * 0xfd98-9f  - Digital palette registers (FMR-50 compatibility)
 */
uint8_t towns_state::towns_video_fd90_r(offs_t offset)
{
	uint8_t ret = 0;
	uint16_t xpos;
	palette_device* pal;

	if(m_video.towns_video_reg[1] & 0x10)
		pal = m_palette;
	else
		pal = m_palette16[(m_video.towns_video_reg[1] & 0x20) >> 5];
//    if(LOG_VID) logerror("VID: read port %04x\n",offset+0xfd90);
	switch(offset)
	{
		case 0x00:
			return m_video.towns_palette_select;
		case 0x02:
			return pal->pen_color(m_video.towns_palette_select).b();
		case 0x04:
			return pal->pen_color(m_video.towns_palette_select).r();
		case 0x06:
			return pal->pen_color(m_video.towns_palette_select).g();
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
			xpos = m_screen->hpos();

			if(xpos < m_video.towns_crtc_layerscr[0].max_x && xpos > m_video.towns_crtc_layerscr[0].min_x)
				ret |= 0x02;
			if(m_video.towns_vblank_flag)
				ret |= 0x01;
			return ret;
	}
	return 0x00;
}

void towns_state::towns_video_fd90_w(offs_t offset, uint8_t data)
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
	if(LOG_VID) logerror("VID: wrote 0x%02x to port %04x\n",data,offset+0xfd90);
}

uint8_t towns_state::towns_video_ff81_r()
{
	return ((m_video.towns_vram_rplane << 6) & 0xc0) | m_video.towns_vram_wplane;
}

void towns_state::towns_video_ff81_w(uint8_t data)
{
	m_video.towns_vram_wplane = data & 0x0f;
	m_video.towns_vram_rplane = (data & 0xc0) >> 6;
	towns_update_video_banks();
	logerror("VID: VRAM wplane select (I/O) = 0x%02x\n",m_video.towns_vram_wplane);
}

uint8_t towns_state::towns_video_unknown_r()
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
uint8_t towns_state::towns_spriteram_low_r(offs_t offset)
{
	uint8_t* RAM = m_ram->pointer();
	uint8_t* ROM = m_user->base();

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

void towns_state::towns_spriteram_low_w(offs_t offset, uint8_t data)
{
	uint8_t* RAM = m_ram->pointer();

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

uint8_t towns_state::towns_spriteram_r(offs_t offset)
{
	return m_towns_txtvram[offset];
}

void towns_state::towns_spriteram_w(offs_t offset, uint8_t data)
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
 *          bit 12,13,14: flip / rotate sprite
 *              When the rotate bit (14) is set, the X and Y coordinates are swapped when rendering the sprite to VRAM.
 *              By combining it with the flip bits, the sprite can be rotated in 90 degree increments, both mirrored and unmirrored.
 *          bits 10,11: half-size
 *          bits 9-0: Sprite RAM offset containing sprite pattern
 *      +6: Sprite Colour
 *          bit 15: use colour data in located in sprite RAM offset in bits 11-0 (x32)
 */
void towns_state::render_sprite_4(uint32_t poffset, uint32_t coffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect)
{
	uint16_t xpos,ypos;
	uint16_t col,pixel;
	uint32_t vbase = m_video.towns_sprite_page ? 0x20000 : 0, voffset;
	uint16_t xstart,xend,ystart,yend;
	int linesize = m_video.towns_crtc_reg[24] * 4;
	int xdir,ydir;

	if (rotation)
	{
		std::swap (x,y);
		std::swap (xflip,yflip);
	}

	if(xflip)
	{
		if (xhalfsize)
		{
			xstart = x+6;
			xdir = -1;
		}
		else
		{
			xstart = x+14;
			xdir = -2;
		}
		xend = x-2;
	}
	else
	{
		xstart = x+1;
		if (xhalfsize)
		{
			xend = x+9;
			xdir = 1;
		}
		else
		{
			xend = x+17;
			xdir = 2;
		}
	}
	if(yflip)
	{
		if (yhalfsize)
			ystart = y+7;
		else
			ystart = y+15;
		yend = y-1;
		ydir = -1;
	}
	else
	{
		ystart = y;
		if (yhalfsize)
			yend = y+8;
		else
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

			voffset = 0;
			pixel = (m_towns_txtvram[poffset] & 0xf0) >> 4;
			col = (m_towns_txtvram[coffset+(pixel*2)] | (m_towns_txtvram[coffset+(pixel*2)+1] << 8)) & 0x7fff;
			if (rotation)
			{
				voffset += linesize * (xpos & 0x1ff);  // scanline size in bytes * y pos
				voffset += (ypos & 0x1ff) * 2;
			}
			else
			{
				voffset += linesize * (ypos & 0x1ff);  // scanline size in bytes * y pos
				voffset += (xpos & 0x1ff) * 2;
			}
			if(voffset < 0x20000 && xpos < 256 && ypos < 256 && pixel != 0 && voffset > linesize)
			{
				m_towns_gfxvram[0x40000+voffset+vbase+1] = (col & 0xff00) >> 8;
				m_towns_gfxvram[0x40000+voffset+vbase] = col & 0x00ff;
			}

			if (!xhalfsize)
			{
				if(xflip)
					if (rotation)
						voffset+=(m_video.towns_crtc_reg[24] * 4);
					else
						voffset+=2;
				else
					if (rotation)
						voffset-=(m_video.towns_crtc_reg[24] * 4);
					else
						voffset-=2;

				pixel = m_towns_txtvram[poffset] & 0x0f;
				col = (m_towns_txtvram[coffset+(pixel*2)] | (m_towns_txtvram[coffset+(pixel*2)+1] << 8)) & 0x7fff;
				if(voffset < 0x20000 && xpos < 256 && ypos < 256 && pixel != 0 && voffset > linesize)
				{
					m_towns_gfxvram[0x40000+voffset+vbase+1] = (col & 0xff00) >> 8;
					m_towns_gfxvram[0x40000+voffset+vbase] = col & 0x00ff;
				}
			}

			poffset++;
			poffset &= 0x1ffff;
		}
		if (yhalfsize)
		{
			poffset+=8;
		}
	}
}

void towns_state::render_sprite_16(uint32_t poffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect)
{
	uint16_t xpos,ypos;
	uint16_t col;
	uint32_t vbase = m_video.towns_sprite_page ? 0x20000 : 0, voffset;
	uint16_t xstart,ystart,xend,yend;
	int linesize = m_video.towns_crtc_reg[24] * 4;
	int xdir,ydir;

	if (rotation)
	{
		std::swap (x,y);
		std::swap (xflip,yflip);
	}

	if(xflip)
	{
		if (xhalfsize)
			xstart = x+7;
		else
			xstart = x+15;
		xend = x-1;
		xdir = -1;
	}
	else
	{
		xstart = x;
		if (xhalfsize)
			xend = x+8;
		else
			xend = x+16;
		xdir = 1;
	}
	if(yflip)
	{
		if (yhalfsize)
			ystart = y+7;
		else
			ystart = y+15;
		yend = y-1;
		ydir = -1;
	}
	else
	{
		ystart = y;
		if (yhalfsize)
			yend = y+8;
		else
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
			voffset = 0;
			col = m_towns_txtvram[poffset] | (m_towns_txtvram[poffset+1] << 8);
			if (rotation)
			{
				voffset += linesize * (xpos & 0x1ff);  // scanline size in bytes * y pos
				voffset += (ypos & 0x1ff) * 2;
			}
			else
			{
				voffset += linesize * (ypos & 0x1ff);  // scanline size in bytes * y pos
				voffset += (xpos & 0x1ff) * 2;
			}
			if(voffset < 0x20000 && xpos < 256 && ypos < 256 && col< 0x8000 && voffset > linesize)
			{
				m_towns_gfxvram[0x40000+vbase+voffset+1] = (col & 0xff00) >> 8;
				m_towns_gfxvram[0x40000+vbase+voffset] = col & 0x00ff;
			}
			if (xhalfsize)
				poffset+=4;
			else
				poffset+=2;
			poffset &= 0x1ffff;
		}

		if (yhalfsize)
			poffset+=16;
	}
}

TIMER_CALLBACK_MEMBER(towns_state::draw_sprites)
{
	uint16_t sprite_limit = (m_video.towns_sprite_reg[0] | (m_video.towns_sprite_reg[1] << 8)) & 0x3ff;
	uint16_t xoff = (m_video.towns_sprite_reg[2] | (m_video.towns_sprite_reg[3] << 8)) & 0x1ff;
	uint16_t yoff = (m_video.towns_sprite_reg[4] | (m_video.towns_sprite_reg[5] << 8)) & 0x1ff;
	uint32_t poffset,coffset;
	const rectangle *rect = &m_video.towns_crtc_layerscr[1];
	int linesize = m_video.towns_crtc_reg[24] * 4;

	// TODO: I'm not confident about this but based on the behavior of aburner and rbisland, it's probably in the ballpark
	// aburner writes the backgound color from 0 to 0x400 in both pages while rbisland from 0 to 0x800 (What's the difference?)
	// it's only written when the color changes so the sprite engine has to be prevented from writing there
	uint8_t *vram;
	if(m_video.towns_sprite_page == 0)
		vram = m_towns_gfxvram.get() + 0x40000;
	else
		vram = m_towns_gfxvram.get() + 0x60000;

	for(int i = linesize; i < 0x20000; i += linesize)
		memcpy(vram + i, vram, linesize);

	for(int n=sprite_limit;n<1024;n++)
	{
		uint16_t x = m_towns_txtvram[8*n] | (m_towns_txtvram[8*n+1] << 8);
		uint16_t y = m_towns_txtvram[8*n+2] | (m_towns_txtvram[8*n+3] << 8);
		uint16_t attr = m_towns_txtvram[8*n+4] | (m_towns_txtvram[8*n+5] << 8);
		uint16_t colour = m_towns_txtvram[8*n+6] | (m_towns_txtvram[8*n+7] << 8);
		bool xflip = (attr & 0x2000) >> 13;
		bool yflip = (attr & 0x1000) >> 12;
		bool rotation = (attr & 0x4000) >> 14;
		bool xhalfsize = (attr & 0x400) >> 10;
		bool yhalfsize = (attr & 0x800) >> 11;

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
			logerror("Sprite4 #%i, X %i Y %i Attr %04x Col %04x Poff %08x Coff %08x\n",
				n,x,y,attr,colour,poffset,coffset);
#endif
			if(!(colour & 0x2000))
				render_sprite_4((poffset)&0x1ffff,coffset,x,y,xflip,yflip,xhalfsize,yhalfsize,rotation,rect);
		}
		else
		{
			poffset = (attr & 0x3ff) << 7;
#ifdef SPR_DEBUG
			logerror("Sprite16 #%i, X %i Y %i Attr %04x Col %04x Poff %08x\n",
				n,x,y,attr,colour,poffset);
#endif
			if(!(colour & 0x2000))
				render_sprite_16((poffset)&0x1ffff,x,y,xflip,yflip,xhalfsize,yhalfsize,rotation,rect);
		}
	}
	m_video.towns_sprite_flag = 0;
}

void towns_state::towns_crtc_draw_scan_layer_hicolour(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline)
{
	uint32_t off = 0;
	uint16_t colour;
	int hzoom = 1;
	int linesize;
	uint32_t scroll;
	int page = 0;
	bool sphscroll = !(m_video.towns_crtc_reg[28] & (layer ? 0x20 : 0x10));

	if(m_video.towns_video_reg[0] & 0x10)
	{
		if(layer == 0)
			linesize = m_video.towns_crtc_reg[20] * 4;
		else
			linesize = m_video.towns_crtc_reg[24] * 4;
	}
	else
		linesize = m_video.towns_crtc_reg[20] * 8;

	if(m_video.towns_display_page_sel != 0)
	{
		off = 0x20000;
		page = 1;
	}

//  if((layer == 1) && (m_video.towns_sprite_reg[1] & 0x80) && (m_video.towns_sprite_page == 1))
//      off = 0x20000;

	if(layer != 0)
	{
		if(!(m_video.towns_video_reg[0] & 0x10))
			return;
		if(!sphscroll)
			off += (m_video.towns_crtc_reg[21]) << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[21] & 0xfc00) << 2) | (((m_video.towns_crtc_reg[21] & 0x3ff) << 2));
			off += scroll;
		}
		hzoom = ((m_video.towns_crtc_reg[27] & 0x0f00) >> 8) + 1;
		off += (float)(m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[22]) * (2.0f / (float)hzoom);
	}
	else
	{
		if(!sphscroll)
			off += (m_video.towns_crtc_reg[17]) << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[17] & 0xfc00) << 2) | (((m_video.towns_crtc_reg[17] & 0x3ff) << 2));
			off += scroll;
		}
		hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
		off += (float)(m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]) * (2.0f / (float)hzoom);
	}

	off += line * linesize;
	off &= ~1;

	for(int x=rect->min_x;x<rect->max_x;x+=hzoom)
	{
		int offpage;
		int curoff;
		if(m_video.towns_video_reg[0] & 0x10)
		{
			curoff = off & 0x3ffff;
			offpage = layer;
		}
		else
		{
			offpage = (off & 4) >> 2;
			curoff = ((off & 0x7fff8) >> 1) | (off & 3);
		}
		colour = (m_towns_gfxvram[curoff+(offpage*0x40000)+1] << 8) | m_towns_gfxvram[curoff+(offpage*0x40000)];
		if(!(m_video.towns_video_reg[0] & 0x10) || (m_video.towns_video_reg[1] & 0x01) != layer || colour < 0x8000)
		{
			for (int pixel = 0; pixel < hzoom; pixel++)
				bitmap.pix(scanline, x+pixel) =
					((colour & 0x001f) << 3)
					| ((colour & 0x7c00) << 1)
					| ((colour & 0x03e0) << 14);
		}
		off+=2;
		if ((off - (page * 0x20000)) % linesize == 0 && sphscroll)
			off -= linesize;
	}
}

void towns_state::towns_crtc_draw_scan_layer_256(bitmap_rgb32 &bitmap,const rectangle* rect,int line,int scanline)
{
	int off = 0;
	uint8_t colour;
	int hzoom = 1;
	int linesize;
	uint32_t scroll;
	int page = 0;
	bool sphscroll = !(m_video.towns_crtc_reg[28] & 0x10);

	if(m_video.towns_display_page_sel != 0)
	{
		off = 0x20000;
		page = 1;
	}

	linesize = m_video.towns_crtc_reg[20] * 4;

	if(!sphscroll)
		off += m_video.towns_crtc_reg[17] << 2;  // initial offset
	else
	{
		scroll = ((m_video.towns_crtc_reg[17] & 0xfc00) << 2) | (((m_video.towns_crtc_reg[17] & 0x3ff) << 2));
		off += scroll;
	}
	hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
	int subpix = (m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]) / hzoom;

	off += line * linesize;
	off += (subpix >> 1) & ~3;
	subpix = subpix & 7;

	for(int x=rect->min_x;x<rect->max_x;x+=hzoom)
	{
		off &= 0x3ffff;
		colour = m_towns_gfxvram[off+(subpix >= 4 ? (subpix & 3)+0x40000 : subpix)];
		for (int pixel = 0; pixel < hzoom; pixel++)
			bitmap.pix(scanline, x+pixel) = m_palette->pen(colour);
		subpix++;
		if(subpix == 8)
		{
			off += 4;
			subpix = 0;
		}
		if ((off - (page * 0x20000)) % linesize == 0 && subpix == 0 && sphscroll)
			off -= linesize;
	}
}

void towns_state::towns_crtc_draw_scan_layer_16(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline)
{
	int off = 0;
	int hzoom = 1;
	uint32_t scroll;
	int page = 0;
	palette_device* pal = m_palette16[layer];
	bool sphscroll = !(m_video.towns_crtc_reg[28] & (layer ? 0x20 : 0x10));

	if(m_video.towns_display_page_sel != 0)
	{
		off = 0x20000;
		page = 1;
	}

	bool bottom_layer = (m_video.towns_video_reg[1] & 0x01) != layer;

//  if((layer == 1) && (m_video.towns_sprite_reg[1] & 0x80) && (m_video.towns_sprite_page == 1))
//      off = 0x20000;

	int linesize;
	if(layer == 0)
		linesize = m_video.towns_crtc_reg[20] * 4;
	else
		linesize = m_video.towns_crtc_reg[24] * 4;

	if(layer != 0)
	{
		if(!(m_video.towns_video_reg[0] & 0x10))
			return;
		if(!sphscroll)
			off += m_video.towns_crtc_reg[21] << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[21] & 0xfc00)<<2) | (((m_video.towns_crtc_reg[21] & 0x3ff)<<2));
			off += scroll;
		}
		hzoom = ((m_video.towns_crtc_reg[27] & 0x0f00) >> 8) + 1;
		off += (m_video.towns_crtc_reg[11] - m_video.towns_crtc_reg[22]) / (hzoom * 2);
	}
	else
	{
		if(!sphscroll)
			off += m_video.towns_crtc_reg[17] << 2;  // initial offset
		else
		{
			scroll = ((m_video.towns_crtc_reg[17] & 0xfc00)<<2) | (((m_video.towns_crtc_reg[17] & 0x3ff)<<2));
			off += scroll;
		}
		hzoom = (m_video.towns_crtc_reg[27] & 0x000f) + 1;
		off += (m_video.towns_crtc_reg[9] - m_video.towns_crtc_reg[18]) / (hzoom * 2);
	}

	off += line * linesize;

	for(int x=rect->min_x;x<rect->max_x;x+=hzoom*2)
	{
		if(m_video.towns_video_reg[0] & 0x10)
			off &= 0x3ffff;  // 2 layers
		else
			off &= 0x7ffff;  // 1 layer
		uint8_t colour;
		colour = m_towns_gfxvram[off+(layer*0x40000)] >> 4;
		if(colour != 0 || bottom_layer)
		{
			for (int pixel = 0; pixel < hzoom; pixel++)
				bitmap.pix(scanline, x+hzoom+pixel) = pal->pen(colour);
		}
		colour = m_towns_gfxvram[off+(layer*0x40000)] & 0x0f;
		if(colour != 0 || bottom_layer)
		{
			for (int pixel = 0; pixel < hzoom; pixel++)
				bitmap.pix(scanline, x+pixel) = pal->pen(colour);
		}
		off++;
		if ((off - (page * 0x20000)) % linesize == 0 && sphscroll)
			off -= linesize;
	}
}

void towns_state::towns_crtc_draw_layer(bitmap_rgb32 &bitmap,const rectangle* rect,int layer)
{
	int scanline;
	int bottom;
	int top;
	uint8_t zoom;
	uint8_t count;

	if(layer == 0)
	{
		scanline = rect->min_y;
		top = (scanline - m_video.towns_crtc_layerscr[0].min_y);
		bottom = (rect->max_y - rect->min_y) + top;
		zoom = ((m_video.towns_crtc_reg[27] & 0x00f0) >> 4) + 1;
		count = top % zoom;
		bottom /= zoom;
		top /= zoom;
		switch(m_video.towns_video_reg[0] & 0x03)
		{
			case 0x01:
				for(int line=top;line<=bottom;line++)
				{
					do
					{
						towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
						scanline++;
						count++;
					} while(count < zoom);
					count = 0;
				}
				break;
			case 0x02:
				for(int line=top;line<=bottom;line++)
				{
					do
					{
						towns_crtc_draw_scan_layer_256(bitmap,rect,line,scanline);
						scanline++;
						count++;
					} while(count < zoom);
					count = 0;
				}
				break;
			case 0x03:
				for(int line=top;line<=bottom;line++)
				{
					do
					{
						towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
						scanline++;
						count++;
					} while(count < zoom);
					count = 0;
				}
				break;
		}
	}
	else
	{
		scanline = rect->min_y;
		top = (scanline - m_video.towns_crtc_layerscr[1].min_y);
		bottom = (rect->max_y - rect->min_y) + top;
		zoom = ((m_video.towns_crtc_reg[27] & 0xf000) >> 12) + 1;
		count = top % zoom;
		bottom /= zoom;
		top /= zoom;
		switch(m_video.towns_video_reg[0] & 0x0c)
		{
			case 0x04:
				for(int line=top;line<=bottom;line++)
				{
					do
					{
						towns_crtc_draw_scan_layer_16(bitmap,rect,layer,line,scanline);
						scanline++;
						count++;
					} while(count < zoom);
					count = 0;
				}
				break;
			case 0x0c:
				for(int line=top;line<=bottom;line++)
				{
					do
					{
						towns_crtc_draw_scan_layer_hicolour(bitmap,rect,layer,line,scanline);
						scanline++;
						count++;
					} while(count < zoom);
					count = 0;
				}
				break;
		}
	}
}

void towns_state::render_text_char(uint8_t x, uint8_t y, uint8_t ascii, uint16_t jis, uint8_t attr)
{
#if 0
	uint32_t rom_addr;
	uint32_t vram_addr;
	uint16_t linesize = m_video.towns_crtc_reg[24] * 4;
	uint8_t code_h,code_l;
	uint8_t* font_rom = m_user->base();

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
	uint8_t colour = attr & 0x07;
	if(attr & 0x20)
		colour |= 0x08;

	for(int a=0;a<16;a++)  // for each scanline
	{
		uint8_t data;
		if((attr & 0xc0) == 0)
			data = font_rom[0x180000 + rom_addr + a];
		else if((attr & 0xc0) == 0x80)
			data = font_rom[0x180000 + rom_addr + (a*2)];
		else
			data = font_rom[0x180000 + rom_addr + (a*2) + 1];

		if(attr & 0x08)
			data = ~data;  // inverse

		// and finally, put the data in VRAM
		for(int b=0;b<8;b+=2)
		{
			uint8_t temp = 0;
			if(data & (1<<b))
				temp |= ((colour & 0x0f) << 4);
			if(data & (1<<(b+1)))
				temp |= (colour & 0x0f);
			m_towns_gfxvram[0x40000+vram_addr+(b/2)] = temp;
		}

		vram_addr += linesize;
		vram_addr &= 0x3ffff;
	}
#endif
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
	int c = 0;

	for(int y=0;y<40;y++)
	{
		for(int x=0;x<80;x++)
		{
			render_text_char(x,y,m_towns_txtvram[c],((m_towns_txtvram[c+0x2000] << 8)|(m_towns_txtvram[c+0x2001])),m_towns_txtvram[c+1]);
			c+=2;
		}
	}
}

void towns_state::towns_sprite_start()
{
	uint16_t sprite_limit = (m_video.towns_sprite_reg[0] | (m_video.towns_sprite_reg[1] << 8)) & 0x3ff;
	m_video.towns_sprite_flag = 1;  // we are now drawing
	m_video.sprite_timer->adjust(attotime::from_usec(32 + 75 * (1024 - sprite_limit)));
}

TIMER_CALLBACK_MEMBER(towns_state::towns_vblank_end)
{
	// here we'll clear the vsync signal, I presume it goes low on it's own eventually
	m_pic_slave->ir3_w(0);  // IRQ11 = VSync
	if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set low\n");
	m_video.towns_vblank_flag = 0;
}

INTERRUPT_GEN_MEMBER(towns_state::towns_vsync_irq)
{
	m_pic_slave->ir3_w(1);  // IRQ11 = VSync
	if(IRQ_LOG) logerror("PIC: IRQ11 (VSync) set high\n");
	m_video.towns_vblank_flag = 1;
	m_video.vblank_end_timer->adjust(m_screen->time_until_vblank_end());
	if(m_video.towns_tvram_enable)
		draw_text_layer();
	if((m_video.towns_sprite_reg[1] & 0x80) && !m_video.towns_sprite_flag)
	{
		if(m_video.towns_sprite_page == 0)  // flip VRAM page
		{
			m_video.towns_sprite_page = 1;
			m_video.towns_crtc_reg[21] &= ~0x8000;
		}
		else
		{
			m_video.towns_sprite_page = 0;
			m_video.towns_crtc_reg[21] |= 0x8000;
		}
		towns_sprite_start();
	}
}

void towns_state::video_start()
{
	m_video.towns_vram_wplane = 0x00;
	m_video.towns_sprite_page = 0;
	m_video.sprite_timer = timer_alloc(FUNC(towns_state::draw_sprites), this);
	m_video.vblank_end_timer = timer_alloc(FUNC(towns_state::towns_vblank_end), this);
}

uint32_t towns_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool layer1_en = true, layer2_en = true;
	bitmap.fill(0x00000000, cliprect);

	if(LAYER_DISABLE)
	{
		if(machine().input().code_pressed(KEYCODE_Q))
			layer1_en = false;
		if(machine().input().code_pressed(KEYCODE_W))
			layer2_en = false;
	}

	rectangle cliplayer0 = m_video.towns_crtc_layerscr[0];
	cliplayer0 &= cliprect;
	rectangle cliplayer1 = m_video.towns_crtc_layerscr[1];
	cliplayer1 &= cliprect;

	if(!(m_video.towns_video_reg[1] & 0x01))
	{
		if((m_video.towns_layer_ctrl & 0x03) != 0 && layer1_en)
			towns_crtc_draw_layer(bitmap,&cliplayer1,1);
		if((m_video.towns_layer_ctrl & 0x0c) != 0 && layer2_en)
			towns_crtc_draw_layer(bitmap,&cliplayer0,0);
	}
	else
	{
		if((m_video.towns_layer_ctrl & 0x0c) != 0 && layer1_en)
			towns_crtc_draw_layer(bitmap,&cliplayer0,0);
		if((m_video.towns_layer_ctrl & 0x03) != 0 && layer2_en)
			towns_crtc_draw_layer(bitmap,&cliplayer1,1);
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
