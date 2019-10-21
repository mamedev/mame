// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/*

STV - VDP1

the vdp1 draws to the FRAMEBUFFER which is mapped in memory

-------------------------- WARNING WARNING WARNING --------------------------
This is a legacy core, all game based notes are for a future device rewrite.
Please don't remove them if for no reason you truly want to mess with this.
-------------------------- WARNING WARNING WARNING --------------------------

Framebuffer todo:
- finish manual erase
- add proper framebuffer erase
- 8 bpp support - now we always draw as 16 bpp, but this is not a problem since
  VDP2 interprets framebuffer as 8 bpp in these cases

*/


#include "emu.h"
#include "includes/saturn.h" // FIXME: this is a dependency from devices on MAME


#define VDP1_LOG 0


enum { FRAC_SHIFT = 16 };

struct spoint {
	int32_t x, y;
	int32_t u, v;
};

struct shaded_point
{
	int32_t x,y;
	int32_t r,g,b;
};

#define RGB_R(_color)   (_color & 0x1f)
#define RGB_G(_color)   ((_color >> 5) & 0x1f)
#define RGB_B(_color)   ((_color >> 10) & 0x1f)

#define SWAP_INT32(_a,_b) \
	{ \
		int32_t t; \
		t = _a; \
		_a = _b; \
		_b = t; \
	}

#define SWAP_INT32PTR(_p1, _p2) \
	{ \
		int32_t *p; \
		p = _p1; \
		_p1 = _p2; \
		_p2 = p; \
	}

/*TV Mode Selection Register */
/*
   xxxx xxxx xxxx ---- | UNUSED
   ---- ---- ---- x--- | VBlank Erase/Write (VBE)
   ---- ---- ---- -xxx | TV Mode (TVM)
   TV-Mode:
   This sets the Frame Buffer size,the rotation of the Frame Buffer & the bit width.
   bit 2 HDTV disable(0)/enable(1)
   bit 1 non-rotation/rotation(1)
   bit 0 16(0)/8(1) bits per pixel
   Size of the Frame Buffer:
   7 invalid
   6 invalid
   5 invalid
   4 512x256
   3 512x512
   2 512x256
   1 1024x256
   0 512x256
*/

/*Frame Buffer Change Mode Register*/
/*
   xxxx xxxx xxx- ---- | UNUSED
   ---- ---- ---x ---- | Even/Odd Coordinate Select Bit (EOS)
   ---- ---- ---- x--- | Double Interlace Mode (DIE)
   ---- ---- ---- -x-- | Double Interlace Draw Line (DIL)
   ---- ---- ---- --x- | Frame Buffer Change Trigger (FCM)
   ---- ---- ---- ---x | Frame Buffer Change Mode (FCT)
*/
#define STV_VDP1_FBCR ((m_vdp1_regs[0x002/2] >> 0)&0xffff)
#define STV_VDP1_EOS ((STV_VDP1_FBCR & 0x0010) >> 4)
#define STV_VDP1_DIE ((STV_VDP1_FBCR & 0x0008) >> 3)
#define STV_VDP1_DIL ((STV_VDP1_FBCR & 0x0004) >> 2)
#define STV_VDP1_FCM ((STV_VDP1_FBCR & 0x0002) >> 1)
#define STV_VDP1_FCT ((STV_VDP1_FBCR & 0x0001) >> 0)

/*Plot Trigger Register*/
/*
   xxxx xxxx xxxx xx-- | UNUSED
   ---- ---- ---- --xx | Plot Trigger Mode (PTM)

   Plot Trigger Mode:
   3 Invalid
   2 Automatic draw
   1 VDP1 draw by request
   0 VDP1 Idle (no access)
*/
#define STV_VDP1_PTMR ((m_vdp1_regs[0x004/2])&0xffff)
#define STV_VDP1_PTM  ((STV_VDP1_PTMR & 0x0003) >> 0)
#define PTM_0         m_vdp1_regs[0x004/2]&=~0x0001

/*
    Erase/Write Data Register
    16 bpp = data
    8 bpp = erase/write data for even/odd X coordinates
*/
#define STV_VDP1_EWDR ((m_vdp1_regs[0x006/2])&0xffff)

/*Erase/Write Upper-Left register*/
/*
   x--- ---- ---- ---- | UNUSED
   -xxx xxx- ---- ---- | X1 register
   ---- ---x xxxx xxxx | Y1 register

*/
#define STV_VDP1_EWLR ((m_vdp1_regs[0x008/2])&0xffff)
#define STV_VDP1_EWLR_X1 ((STV_VDP1_EWLR & 0x7e00) >> 9)
#define STV_VDP1_EWLR_Y1 ((STV_VDP1_EWLR & 0x01ff) >> 0)
/*Erase/Write Lower-Right register*/
/*
   xxxx xxx- ---- ---- | X3 register
   ---- ---x xxxx xxxx | Y3 register

*/
#define STV_VDP1_EWRR ((m_vdp1_regs[0x00a/2])&0xffff)
#define STV_VDP1_EWRR_X3 ((STV_VDP1_EWRR & 0xfe00) >> 9)
#define STV_VDP1_EWRR_Y3 ((STV_VDP1_EWRR & 0x01ff) >> 0)
/*Transfer End Status Register*/
/*
   xxxx xxxx xxxx xx-- | UNUSED
   ---- ---- ---- --x- | CEF
   ---- ---- ---- ---x | BEF

*/
#define STV_VDP1_EDSR ((m_vdp1_regs[0x010/2])&0xffff)
#define STV_VDP1_CEF  (STV_VDP1_EDSR & 2)
#define STV_VDP1_BEF  (STV_VDP1_EDSR & 1)
/**/



READ16_MEMBER( saturn_state::saturn_vdp1_regs_r )
{
	//logerror ("%s VDP1: Read from Registers, Offset %04x\n", machine().describe_context(), offset);

	switch(offset)
	{
		case 0x02/2:
			return 0;
		case 0x10/2:
			break;
		case 0x12/2: return m_vdp1.lopr;
		case 0x14/2: return m_vdp1.copr;
		/* MODR register, read register for the other VDP1 regs
		   (Shienryu SS version abuses of this during intro) */
		case 0x16/2:
			uint16_t modr;

			modr = 0x1000; //vdp1 VER
			modr |= (STV_VDP1_PTM >> 1) << 8; // PTM1
			modr |= STV_VDP1_EOS << 7; // EOS
			modr |= STV_VDP1_DIE << 6; // DIE
			modr |= STV_VDP1_DIL << 5; // DIL
			modr |= STV_VDP1_FCM << 4; //FCM
			modr |= STV_VDP1_VBE << 3; //VBE
			modr |= STV_VDP1_TVM & 7; //TVM

			return modr;
		default:
			if(!machine().side_effects_disabled())
				logerror("%s VDP1: Read from Registers, Offset %04x\n", machine().describe_context(), offset*2);
			break;
	}

	return m_vdp1_regs[offset]; //TODO: write-only regs should return open bus or zero
}

/* TODO: TVM & 1 is just a kludgy work-around, the VDP1 actually needs to be rewritten from scratch. */
/* Daisenryaku Strong Style (daisenss) uses it */
void saturn_state::stv_clear_framebuffer( int which_framebuffer )
{
	int start_x, end_x, start_y, end_y;

	start_x = STV_VDP1_EWLR_X1 * ((STV_VDP1_TVM & 1) ? 16 : 8);
	start_y = STV_VDP1_EWLR_Y1 * (m_vdp1.framebuffer_double_interlace+1);
	end_x = STV_VDP1_EWRR_X3 * ((STV_VDP1_TVM & 1) ? 16 : 8);
	end_y = (STV_VDP1_EWRR_Y3+1) * (m_vdp1.framebuffer_double_interlace+1);
//  popmessage("%d %d %d %d %d",STV_VDP1_EWLR_X1,STV_VDP1_EWLR_Y1,STV_VDP1_EWRR_X3,STV_VDP1_EWRR_Y3,m_vdp1.framebuffer_double_interlace);

	if(STV_VDP1_TVM & 1)
	{
		for(int y=start_y;y<end_y;y++)
			for(int x=start_x;x<end_x;x++)
				m_vdp1.framebuffer[ which_framebuffer ][((x&1023)+(y&511)*1024)] = m_vdp1.ewdr;
	}
	else
	{
		for(int y=start_y;y<end_y;y++)
			for(int x=start_x;x<end_x;x++)
				m_vdp1.framebuffer[ which_framebuffer ][((x&511)+(y&511)*512)] = m_vdp1.ewdr;
	}

	if ( VDP1_LOG ) logerror( "Clearing %d framebuffer\n", m_vdp1.framebuffer_current_draw );
//  memset( m_vdp1.framebuffer[ which_framebuffer ], m_vdp1.ewdr, 1024 * 256 * sizeof(uint16_t) * 2 );
}


void saturn_state::stv_prepare_framebuffers( void )
{
	int i,rowsize;

	rowsize = m_vdp1.framebuffer_width;
	if ( m_vdp1.framebuffer_current_draw == 0 )
	{
		for ( i = 0; i < m_vdp1.framebuffer_height; i++ )
		{
			m_vdp1.framebuffer_draw_lines[i] = &m_vdp1.framebuffer[0][ i * rowsize ];
			m_vdp1.framebuffer_display_lines[i] = &m_vdp1.framebuffer[1][ i * rowsize ];
		}
		for ( ; i < 512; i++ )
		{
			m_vdp1.framebuffer_draw_lines[i] = &m_vdp1.framebuffer[0][0];
			m_vdp1.framebuffer_display_lines[i] = &m_vdp1.framebuffer[1][0];
		}
	}
	else
	{
		for ( i = 0; i < m_vdp1.framebuffer_height; i++ )
		{
			m_vdp1.framebuffer_draw_lines[i] = &m_vdp1.framebuffer[1][ i * rowsize ];
			m_vdp1.framebuffer_display_lines[i] = &m_vdp1.framebuffer[0][ i * rowsize ];
		}
		for ( ; i < 512; i++ )
		{
			m_vdp1.framebuffer_draw_lines[i] = &m_vdp1.framebuffer[1][0];
			m_vdp1.framebuffer_display_lines[i] = &m_vdp1.framebuffer[0][0];
		}

	}

	for ( ; i < 512; i++ )
	{
		m_vdp1.framebuffer_draw_lines[i] = &m_vdp1.framebuffer[0][0];
		m_vdp1.framebuffer_display_lines[i] = &m_vdp1.framebuffer[1][0];
	}

}

void saturn_state::stv_vdp1_change_framebuffers( void )
{
	m_vdp1.framebuffer_current_display ^= 1;
	m_vdp1.framebuffer_current_draw ^= 1;
	// "this bit is reset to 0 when the frame buffers are changed"
	CEF_0;
	if ( VDP1_LOG ) logerror( "Changing framebuffers: %d - draw, %d - display\n", m_vdp1.framebuffer_current_draw, m_vdp1.framebuffer_current_display );
	stv_prepare_framebuffers();
}

void saturn_state::stv_set_framebuffer_config( void )
{
	if ( m_vdp1.framebuffer_mode == STV_VDP1_TVM &&
			m_vdp1.framebuffer_double_interlace == STV_VDP1_DIE ) return;

	if ( VDP1_LOG ) logerror( "Setting framebuffer config\n" );
	m_vdp1.framebuffer_mode = STV_VDP1_TVM;
	m_vdp1.framebuffer_double_interlace = STV_VDP1_DIE;
	switch( m_vdp1.framebuffer_mode )
	{
		case 0: m_vdp1.framebuffer_width = 512; m_vdp1.framebuffer_height = 256; break;
		case 1: m_vdp1.framebuffer_width = 1024; m_vdp1.framebuffer_height = 256; break;
		case 2: m_vdp1.framebuffer_width = 512; m_vdp1.framebuffer_height = 256; break;
		case 3: m_vdp1.framebuffer_width = 512; m_vdp1.framebuffer_height = 512; break;
		case 4: m_vdp1.framebuffer_width = 512; m_vdp1.framebuffer_height = 256; break;
		default: logerror( "Invalid framebuffer config %x\n", STV_VDP1_TVM ); m_vdp1.framebuffer_width = 512; m_vdp1.framebuffer_height = 256; break;
	}
	if ( STV_VDP1_DIE ) m_vdp1.framebuffer_height *= 2; /* double interlace */

	m_vdp1.framebuffer_current_draw = 0;
	m_vdp1.framebuffer_current_display = 1;
	stv_prepare_framebuffers();
}

WRITE16_MEMBER( saturn_state::saturn_vdp1_regs_w )
{
	COMBINE_DATA(&m_vdp1_regs[offset]);

	switch(offset)
	{
		case 0x00/2:
			stv_set_framebuffer_config();
			if ( VDP1_LOG ) logerror( "VDP1: Access to register TVMR = %1X\n", STV_VDP1_TVMR );

			break;
		case 0x02/2:
			stv_set_framebuffer_config();
			if ( VDP1_LOG ) logerror( "VDP1: Access to register FBCR = %1X\n", STV_VDP1_FBCR );
			m_vdp1.fbcr_accessed = 1;
			break;
		case 0x04/2:
			if ( VDP1_LOG ) logerror( "VDP1: Access to register PTMR = %1X\n", STV_VDP1_PTM );
			if ( STV_VDP1_PTMR == 1 )
				stv_vdp1_process_list();

			break;
		case 0x06/2:
			if ( VDP1_LOG ) logerror( "VDP1: Erase data set %08X\n", data );

			m_vdp1.ewdr = STV_VDP1_EWDR;
			break;
		case 0x08/2:
			if ( VDP1_LOG ) logerror( "VDP1: Erase upper-left coord set: %08X\n", data );
			break;
		case 0x0a/2:
			if ( VDP1_LOG ) logerror( "VDP1: Erase lower-right coord set: %08X\n", data );
			break;
		case 0x0c/2:
		case 0x0e/2: // After Burner 2 / Out Run / Fantasy Zone writes here with a dword ...
			if ( VDP1_LOG ) logerror( "VDP1: Draw forced termination register write: %08X %08X\n", offset*2, data );
			break;
		default:
			printf("Warning: write to unknown VDP1 reg %08x %08x\n",offset*2,data);
			break;
	}

}

READ32_MEMBER ( saturn_state::saturn_vdp1_vram_r )
{
	return m_vdp1_vram[offset];
}


WRITE32_MEMBER ( saturn_state::saturn_vdp1_vram_w )
{
	uint8_t *vdp1 = m_vdp1.gfx_decode.get();

	COMBINE_DATA (&m_vdp1_vram[offset]);

//  if (((offset * 4) > 0xdf) && ((offset * 4) < 0x140))
//  {
//      logerror("%s: VRAM dword write to %08X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
//  }

	data = m_vdp1_vram[offset];
	/* put in gfx region for easy decoding */
	vdp1[offset*4+0] = (data & 0xff000000) >> 24;
	vdp1[offset*4+1] = (data & 0x00ff0000) >> 16;
	vdp1[offset*4+2] = (data & 0x0000ff00) >> 8;
	vdp1[offset*4+3] = (data & 0x000000ff) >> 0;
}

WRITE32_MEMBER ( saturn_state::saturn_vdp1_framebuffer0_w )
{
	//popmessage ("STV VDP1 Framebuffer 0 WRITE offset %08x data %08x",offset, data);
	if ( STV_VDP1_TVM & 1 )
	{
		/* 8-bit mode */
		//printf("VDP1 8-bit mode %08x %02x\n",offset,data);
		if ( ACCESSING_BITS_24_31 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] &= 0x00ff;
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] |= data & 0xff00;
		}
		if ( ACCESSING_BITS_16_23 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] &= 0xff00;
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] |= data & 0x00ff;
		}
		if ( ACCESSING_BITS_8_15 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] &= 0x00ff;
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] |= data & 0xff00;
		}
		if ( ACCESSING_BITS_0_7 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] &= 0xff00;
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] |= data & 0x00ff;
		}
	}
	else
	{
		/* 16-bit mode */
		if ( ACCESSING_BITS_16_31 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] = (data >> 16) & 0xffff;
		}
		if ( ACCESSING_BITS_0_15 )
		{
			m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] = data & 0xffff;
		}
	}
}

READ32_MEMBER ( saturn_state::saturn_vdp1_framebuffer0_r )
{
	uint32_t result = 0;
	//popmessage ("STV VDP1 Framebuffer 0 READ offset %08x",offset);
	if ( STV_VDP1_TVM & 1 )
	{
		/* 8-bit mode */
		//printf("VDP1 8-bit mode %08x\n",offset);
		if ( ACCESSING_BITS_24_31 )
			result |= ((m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] & 0xff00) << 16);
		if ( ACCESSING_BITS_16_23 )
			result |= ((m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] & 0x00ff) << 16);
		if ( ACCESSING_BITS_8_15 )
			result |= ((m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] & 0xff00));
		if ( ACCESSING_BITS_0_7 )
			result |= ((m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1] & 0x00ff));
	}
	else
	{
		/* 16-bit mode */
		if ( ACCESSING_BITS_16_31 )
		{
			result |= (m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2] << 16);
		}
		if ( ACCESSING_BITS_0_15 )
		{
			result |= (m_vdp1.framebuffer[m_vdp1.framebuffer_current_draw][offset*2+1]);
		}

	}

	return result;
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER ( saturn_state::saturn_vdp1_framebuffer1_w )
{
	//popmessage ("STV VDP1 Framebuffer 1 WRITE offset %08x data %08x",offset, data);
}

READ32_MEMBER ( saturn_state::saturn_vdp1_framebuffer1_r )
{
	//popmessage ("STV VDP1 Framebuffer 1 READ offset %08x",offset);
	return 0xffff;
}
#endif


/*

there is a command every 0x20 bytes
the first word is the control word
the rest are data used by it

---
00 CMDCTRL
   e--- ---- ---- ---- | end bit (15)
   -jjj ---- ---- ---- | jump select bits (12-14)
   ---- zzzz ---- ---- | zoom point / hotspot (8-11)
   ---- ---- 00-- ---- | UNUSED
   ---- ---- --dd ---- | character read direction (4,5)
   ---- ---- ---- cccc | command bits (0-3)

02 CMDLINK
   llll llll llll ll-- | link
   ---- ---- ---- --00 | UNUSED

04 CMDPMOD
   m--- ---- ---- ---- | MON (looks at MSB and apply shadows etc.)
   -00- ---- ---- ---- | UNUSED
   ---h ---- ---- ---- | HSS (High Speed Shrink)
   ---- p--- ---- ---- | PCLIP (Pre Clipping Disable)
   ---- -c-- ---- ---- | CLIP (Clipping Mode Bit)
   ---- --m- ---- ---- | CMOD (User Clipping Enable Bit)
   ---- ---M ---- ---- | MESH (Mesh Enable Bit)
   ---- ---- e--- ---- | ECD (End Code Disable)
   ---- ---- -S-- ---- | SPD (Transparent Pixel Disable)
   ---- ---- --cc c--- | Colour Mode
   ---- ---- ---- -CCC | Colour Calculation bits

06 CMDCOLR
   mmmm mmmm mmmm mmmm | Colour Bank, Colour Lookup /8

08 CMDSRCA (Character Address)
   aaaa aaaa aaaa aa-- | Character Address
   ---- ---- ---- --00 | UNUSED

0a CMDSIZE (Character Size)
   00-- ---- ---- ---- | UNUSED
   --xx xxxx ---- ---- | Character Size (X)
   ---- ---- yyyy yyyy | Character Size (Y)

0c CMDXA (used for normal sprite)
   eeee ee-- ---- ---- | extension bits
   ---- --xx xxxx xxxx | x position

0e CMDYA (used for normal sprite)
   eeee ee-- ---- ---- | extension bits
   ---- --yy yyyy yyyy | y position

10 CMDXB
12 CMDYB
14 CMDXC
16 CMDYC
18 CMDXD
1a CMDYD
1c CMDGRDA (Gouraud Shading Table)
1e UNUSED
---


*/

void saturn_state::stv_clear_gouraud_shading(void)
{
	memset( &stv_gouraud_shading, 0, sizeof( stv_gouraud_shading ) );
}

uint8_t saturn_state::stv_read_gouraud_table( void )
{
	int gaddr;

	if ( (stv2_current_sprite.CMDPMOD & 0x7) == 4 )
	{
		gaddr = stv2_current_sprite.CMDGRDA * 8;
		stv_gouraud_shading.GA = (m_vdp1_vram[gaddr/4] >> 16) & 0xffff;
		stv_gouraud_shading.GB = (m_vdp1_vram[gaddr/4] >> 0) & 0xffff;
		stv_gouraud_shading.GC = (m_vdp1_vram[gaddr/4 + 1] >> 16) & 0xffff;
		stv_gouraud_shading.GD = (m_vdp1_vram[gaddr/4 + 1] >> 0) & 0xffff;
		return 1;
	}
	else
	{
		return 0;
	}
}

static inline int32_t _shading( int32_t color, int32_t correction )
{
	correction = (correction >> 16) & 0x1f;
	color += (correction - 16);

	if ( color < 0 ) color = 0;
	if ( color > 0x1f ) color = 0x1f;

	return color;
}

uint16_t saturn_state::stv_vdp1_apply_gouraud_shading( int x, int y, uint16_t pix )
{
	int32_t r,g,b, msb;

	msb = pix & 0x8000;

#ifdef MAME_DEBUG
	if ( (stv_vdp1_shading_data->scanline[y].x[0] >> 16) != x )
	{
		logerror( "ERROR in computing x coordinates (line %d, x = %x, %d, xc = %x, %d)\n", y, x, x, stv_vdp1_shading_data->scanline[y].x[0], stv_vdp1_shading_data->scanline[y].x[0] >> 16 );
	};
#endif

	b = RGB_B(pix);
	g = RGB_G(pix);
	r = RGB_R(pix);

	b = _shading( b, stv_vdp1_shading_data->scanline[y].b[0] );
	g = _shading( g, stv_vdp1_shading_data->scanline[y].g[0] );
	r = _shading( r, stv_vdp1_shading_data->scanline[y].r[0] );

	stv_vdp1_shading_data->scanline[y].b[0] += stv_vdp1_shading_data->scanline[y].db;
	stv_vdp1_shading_data->scanline[y].g[0] += stv_vdp1_shading_data->scanline[y].dg;
	stv_vdp1_shading_data->scanline[y].r[0] += stv_vdp1_shading_data->scanline[y].dr;

	stv_vdp1_shading_data->scanline[y].x[0] += 1 << FRAC_SHIFT;

	return msb | b << 10 | g << 5 | r;
}

void saturn_state::stv_vdp1_setup_shading_for_line(int32_t y, int32_t x1, int32_t x2,
											int32_t r1, int32_t g1, int32_t b1,
											int32_t r2, int32_t g2, int32_t b2)
{
	int xx1 = x1>>FRAC_SHIFT;
	int xx2 = x2>>FRAC_SHIFT;


	if ( xx1 > xx2 )
	{
		SWAP_INT32(xx1, xx2);
		SWAP_INT32(r1, r2);
		SWAP_INT32(g1, g2);
		SWAP_INT32(b1, b2);
	}

	if ( (y >= 0) && (y < 512) )
	{
		int32_t  dx;
		int32_t   gbd, ggd, grd;

		dx = xx2 - xx1;

		if ( dx == 0 )
		{
			gbd = ggd = grd = 0;
		}
		else
		{
			gbd = abs(b2 - b1) / dx;
			if (b2 < b1) gbd = -gbd;
			ggd = abs(g2 - g1) / dx;
			if (g2 < g1) ggd = -ggd;
			grd = abs(r2 - r1) / dx;
			if (r2 < r1) grd = -grd;
		}

		stv_vdp1_shading_data->scanline[y].x[0] = x1;
		stv_vdp1_shading_data->scanline[y].x[1] = x2;

		stv_vdp1_shading_data->scanline[y].b[0] = b1;
		stv_vdp1_shading_data->scanline[y].g[0] = g1;
		stv_vdp1_shading_data->scanline[y].r[0] = r1;
		stv_vdp1_shading_data->scanline[y].b[1] = b2;
		stv_vdp1_shading_data->scanline[y].g[1] = g2;
		stv_vdp1_shading_data->scanline[y].r[1] = r2;

		stv_vdp1_shading_data->scanline[y].db = gbd;
		stv_vdp1_shading_data->scanline[y].dg = ggd;
		stv_vdp1_shading_data->scanline[y].dr = grd;

	}
}

void saturn_state::stv_vdp1_setup_shading_for_slope(
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t r1, int32_t r2, int32_t slr1, int32_t slr2, int32_t *nr1, int32_t *nr2,
							int32_t g1, int32_t g2, int32_t slg1, int32_t slg2, int32_t *ng1, int32_t *ng2,
							int32_t b1, int32_t b2, int32_t slb1, int32_t slb2, int32_t *nb1, int32_t *nb2,
							int32_t _y1, int32_t y2)
{
	if(x1 > x2 || (x1==x2 && sl1 > sl2)) {
		SWAP_INT32(x1,x2);
		SWAP_INT32(sl1,sl2);
		SWAP_INT32PTR(nx1, nx2);
		SWAP_INT32(r1,r2);
		SWAP_INT32(slr1, slr2);
		SWAP_INT32PTR(nr1, nr2);
		SWAP_INT32(g1, g2);
		SWAP_INT32(slg1, slg2);
		SWAP_INT32PTR(ng1, ng2);
		SWAP_INT32(b1, b2);
		SWAP_INT32(slb1, slb2);
		SWAP_INT32PTR(nb1, nb2);
	}

	while(_y1 < y2)
	{
		stv_vdp1_setup_shading_for_line(_y1, x1, x2, r1, g1, b1, r2, g2, b2);
		x1 += sl1;
		r1 += slr1;
		g1 += slg1;
		b1 += slb1;

		x2 += sl2;
		r2 += slr2;
		g2 += slg2;
		b2 += slb2;
		_y1++;
	}
	*nx1 = x1;
	*nr1 = r1;
	*ng1 = g1;
	*nb1 = b1;

	*nx2 = x2;
	*nr2 = r2;
	*nb2 = b2;
	*ng2 = g2;
}

void saturn_state::stv_vdp1_setup_shading(const struct spoint* q, const rectangle &cliprect)
{
	int32_t x1, x2, delta, cury, limy;
	int32_t r1, g1, b1, r2, g2, b2;
	int32_t sl1, slg1, slb1, slr1;
	int32_t sl2, slg2, slb2, slr2;
	int pmin, pmax, i, ps1, ps2;
	struct shaded_point p[8];
	uint16_t gd[4];

	if ( stv_read_gouraud_table() == 0 ) return;

	gd[0] = stv_gouraud_shading.GA;
	gd[1] = stv_gouraud_shading.GB;
	gd[2] = stv_gouraud_shading.GC;
	gd[3] = stv_gouraud_shading.GD;

	for(i=0; i<4; i++) {
		p[i].x = p[i+4].x = q[i].x << FRAC_SHIFT;
		p[i].y = p[i+4].y = q[i].y;
		p[i].r = p[i+4].r = RGB_R(gd[i]) << FRAC_SHIFT;
		p[i].g = p[i+4].g = RGB_G(gd[i]) << FRAC_SHIFT;
		p[i].b = p[i+4].b = RGB_B(gd[i]) << FRAC_SHIFT;
	}

	pmin = pmax = 0;
	for(i=1; i<4; i++) {
		if(p[i].y < p[pmin].y)
			pmin = i;
		if(p[i].y > p[pmax].y)
			pmax = i;
	}

	cury = p[pmin].y;
	limy = p[pmax].y;

	stv_vdp1_shading_data->sy = cury;
	stv_vdp1_shading_data->ey = limy;

	if(cury == limy) {
		x1 = x2 = p[0].x;
		ps1 = ps2 = 0;
		for(i=1; i<4; i++) {
			if(p[i].x < x1) {
				x1 = p[i].x;
				ps1 = i;
			}
			if(p[i].x > x2) {
				x2 = p[i].x;
				ps2 = i;
			}
		}
		stv_vdp1_setup_shading_for_line(cury, x1, x2, p[ps1].r, p[ps1].g, p[ps1].b, p[ps2].r, p[ps2].g, p[ps2].b);
		goto finish;
	}

	ps1 = pmin+4;
	ps2 = pmin;

	goto startup;

	for(;;) {
		if(p[ps1-1].y == p[ps2+1].y) {
			stv_vdp1_setup_shading_for_slope(
							x1, x2, sl1, sl2, &x1, &x2,
							r1, r2, slr1, slr2, &r1, &r2,
							g1, g2, slg1, slg2, &g1, &g2,
							b1, b2, slb1, slb2, &b1, &b2,
							cury, p[ps1-1].y);
			cury = p[ps1-1].y;
			if(cury >= limy)
				break;
			ps1--;
			ps2++;

		startup:
			while(p[ps1-1].y == cury)
				ps1--;
			while(p[ps2+1].y == cury)
				ps2++;
			x1 = p[ps1].x;
			r1 = p[ps1].r;
			g1 = p[ps1].g;
			b1 = p[ps1].b;
			x2 = p[ps2].x;
			r2 = p[ps2].r;
			g2 = p[ps2].g;
			b2 = p[ps2].b;

			delta = cury-p[ps1-1].y;
			sl1 = (x1-p[ps1-1].x)/delta;
			slr1 = (r1-p[ps1-1].r)/delta;
			slg1 = (g1-p[ps1-1].g)/delta;
			slb1 = (b1-p[ps1-1].b)/delta;

			delta = cury-p[ps2+1].y;
			sl2 = (x2-p[ps2+1].x)/delta;
			slr2 = (r2-p[ps2+1].r)/delta;
			slg2 = (g2-p[ps2+1].g)/delta;
			slb2 = (b2-p[ps2+1].b)/delta;
		} else if(p[ps1-1].y < p[ps2+1].y) {
			stv_vdp1_setup_shading_for_slope(
							x1, x2, sl1, sl2, &x1, &x2,
							r1, r2, slr1, slr2, &r1, &r2,
							g1, g2, slg1, slg2, &g1, &g2,
							b1, b2, slb1, slb2, &b1, &b2,
							cury, p[ps1-1].y);
			cury = p[ps1-1].y;
			if(cury >= limy)
				break;
			ps1--;
			while(p[ps1-1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			r1 = p[ps1].r;
			g1 = p[ps1].g;
			b1 = p[ps1].b;

			delta = cury-p[ps1-1].y;
			sl1 = (x1-p[ps1-1].x)/delta;
			slr1 = (r1-p[ps1-1].r)/delta;
			slg1 = (g1-p[ps1-1].g)/delta;
			slb1 = (b1-p[ps1-1].b)/delta;
		} else {
			stv_vdp1_setup_shading_for_slope(
							x1, x2, sl1, sl2, &x1, &x2,
							r1, r2, slr1, slr2, &r1, &r2,
							g1, g2, slg1, slg2, &g1, &g2,
							b1, b2, slb1, slb2, &b1, &b2,
							cury, p[ps2+1].y);
			cury = p[ps2+1].y;
			if(cury >= limy)
				break;
			ps2++;
			while(p[ps2+1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			r2 = p[ps2].r;
			g2 = p[ps2].g;
			b2 = p[ps2].b;

			delta = cury-p[ps2+1].y;
			sl2 = (x2-p[ps2+1].x)/delta;
			slr2 = (r2-p[ps2+1].r)/delta;
			slg2 = (g2-p[ps2+1].g)/delta;
			slb2 = (b2-p[ps2+1].b)/delta;
		}
	}
	if(cury == limy)
		stv_vdp1_setup_shading_for_line(cury, x1, x2, r1, g1, b1, r2, g2, b2 );

finish:

	if ( stv_vdp1_shading_data->sy < 0 ) stv_vdp1_shading_data->sy = 0;
	if ( stv_vdp1_shading_data->sy >= 512 ) return;
	if ( stv_vdp1_shading_data->ey < 0 ) return;
	if ( stv_vdp1_shading_data->ey >= 512 ) stv_vdp1_shading_data->ey = 511;

	for ( cury = stv_vdp1_shading_data->sy; cury <= stv_vdp1_shading_data->ey; cury++ )
	{
		while( (stv_vdp1_shading_data->scanline[cury].x[0] >> 16) < cliprect.min_x )
		{
			stv_vdp1_shading_data->scanline[cury].x[0] += (1 << FRAC_SHIFT);
			stv_vdp1_shading_data->scanline[cury].b[0] += stv_vdp1_shading_data->scanline[cury].db;
			stv_vdp1_shading_data->scanline[cury].g[0] += stv_vdp1_shading_data->scanline[cury].dg;
			stv_vdp1_shading_data->scanline[cury].r[0] += stv_vdp1_shading_data->scanline[cury].dr;
		}
	}

}

/* note that if we're drawing
to the framebuffer we CAN'T frameskip the vdp1 drawing as the hardware can READ the framebuffer
and if we skip the drawing the content could be incorrect when it reads it, although i have no idea
why they would want to */



void saturn_state::drawpixel_poly(int x, int y, int patterndata, int offsetcnt)
{
	/* Capcom Collection Dai 4 uses a dummy polygon to clear VDP1 framebuffer that goes over our current max size ... */
	if(x >= 1024 || y >= 512)
		return;

	m_vdp1.framebuffer_draw_lines[y][x] = stv2_current_sprite.CMDCOLR;
}

void saturn_state::drawpixel_8bpp_trans(int x, int y, int patterndata, int offsetcnt)
{
	uint16_t pix;

	pix = m_vdp1.gfx_decode[patterndata+offsetcnt] & 0xff;
	if ( pix != 0 )
	{
		m_vdp1.framebuffer_draw_lines[y][x] = pix | m_sprite_colorbank;
	}
}

void saturn_state::drawpixel_4bpp_notrans(int x, int y, int patterndata, int offsetcnt)
{
	uint16_t pix;

	pix = m_vdp1.gfx_decode[patterndata+offsetcnt/2];
	pix = offsetcnt&1 ? (pix & 0x0f) : ((pix & 0xf0)>>4);
	m_vdp1.framebuffer_draw_lines[y][x] = pix | m_sprite_colorbank;
}

void saturn_state::drawpixel_4bpp_trans(int x, int y, int patterndata, int offsetcnt)
{
	uint16_t pix;

	pix = m_vdp1.gfx_decode[patterndata+offsetcnt/2];
	pix = offsetcnt&1 ? (pix & 0x0f) : ((pix & 0xf0)>>4);
	if ( pix != 0 )
		m_vdp1.framebuffer_draw_lines[y][x] = pix | m_sprite_colorbank;
}

void saturn_state::drawpixel_generic(int x, int y, int patterndata, int offsetcnt)
{
	int pix,transpen, spd = stv2_current_sprite.CMDPMOD & 0x40;
//  int mode;
	int mesh = stv2_current_sprite.CMDPMOD & 0x100;
	int raw,endcode;

	if ( mesh && !((x ^ y) & 1) )
	{
		return;
	}

	if(x >= 1024 || y >= 512)
		return;

	if ( stv2_current_sprite.ispoly )
	{
		raw = pix = stv2_current_sprite.CMDCOLR&0xffff;

		transpen = 0;
		endcode = 0xffff;
		#if 0
		if ( pix & 0x8000 )
		{
			mode = 5;
		}
		else
		{
			mode = 1;
		}
		#endif
	}
	else
	{
		switch (stv2_current_sprite.CMDPMOD&0x0038)
		{
			case 0x0000: // mode 0 16 colour bank mode (4bits) (hanagumi blocks)
				// most of the shienryu sprites use this mode
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt/2) & 0xfffff];
				raw = offsetcnt&1 ? (raw & 0x0f) : ((raw & 0xf0)>>4);
				pix = raw+((stv2_current_sprite.CMDCOLR&0xfff0));
				//mode = 0;
				transpen = 0;
				endcode = 0xf;
				break;
			case 0x0008: // mode 1 16 colour lookup table mode (4bits)
				// shienryu explosions (and some enemies) use this mode
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt/2) & 0xfffff];
				raw = offsetcnt&1 ? (raw & 0x0f) : ((raw & 0xf0)>>4);
				pix = raw&1 ?
				((((m_vdp1_vram[(((stv2_current_sprite.CMDCOLR&0xffff)*8)>>2)+((raw&0xfffe)/2)])) & 0x0000ffff) >> 0):
				((((m_vdp1_vram[(((stv2_current_sprite.CMDCOLR&0xffff)*8)>>2)+((raw&0xfffe)/2)])) & 0xffff0000) >> 16);
				//mode = 5;
				transpen = 0;
				endcode = 0xf;
				break;
			case 0x0010: // mode 2 64 colour bank mode (8bits) (character select portraits on hanagumi)
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt) & 0xfffff] & 0xff;
				//mode = 2;
				pix = raw+(stv2_current_sprite.CMDCOLR&0xffc0);
				transpen = 0;
				endcode = 0xff;
				// Notes of interest:
				// Scud: the disposable assassin wants transparent pen on 0
				// sasissu: racing stage background clouds
				break;
			case 0x0018: // mode 3 128 colour bank mode (8bits) (little characters on hanagumi use this mode)
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt) & 0xfffff] & 0xff;
				pix = raw+(stv2_current_sprite.CMDCOLR&0xff80);
				transpen = 0;
				endcode = 0xff;
				//mode = 3;
				break;
			case 0x0020: // mode 4 256 colour bank mode (8bits) (hanagumi title)
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt) & 0xfffff] & 0xff;
				pix = raw+(stv2_current_sprite.CMDCOLR&0xff00);
				transpen = 0;
				endcode = 0xff;
				//mode = 4;
				break;
			case 0x0028: // mode 5 32,768 colour RGB mode (16bits)
				raw = m_vdp1.gfx_decode[(patterndata+offsetcnt*2+1) & 0xfffff] | (m_vdp1.gfx_decode[(patterndata+offsetcnt*2) & 0xfffff]<<8);
				//mode = 5;
				// TODO: 0x1-0x7ffe reserved (color bank)
				pix = raw;
				transpen = 0;
				endcode = 0x7fff;
				break;
			case 0x0038: // invalid
				// game tengoku uses this on hi score screen (tate mode)
				// according to Charles, reads from VRAM address 0
				raw = pix = m_vdp1.gfx_decode[1] | (m_vdp1.gfx_decode[0]<<8) ;
				// TODO: check transpen
				transpen = 0;
				endcode = -1;
				break;
			default: // other settings illegal
				pix = machine().rand();
				raw = pix & 0xff; // just mimic old driver behavior
				//mode = 0;
				transpen = 0;
				endcode = 0xff;
				popmessage("Illegal Sprite Mode %02x, contact MAMEdev",stv2_current_sprite.CMDPMOD&0x0038);
		}


		// preliminary end code disable support
		if ( ((stv2_current_sprite.CMDPMOD & 0x80) == 0) &&
			(raw == endcode) )
		{
			return;
		}
	}

	/* MSBON */
	// TODO: does this always applies to the frame buffer regardless of the mode?
	pix |= stv2_current_sprite.CMDPMOD & 0x8000;
	/*
	TODO: from docs:
	"Except for the color calculation of replace and shadow, color calculation can only be performed when the color code of the original picture is RGB code.
	Color calculation can be executed when the color code is color bank code, but the results are not guaranteed."
	Currently no idea about the "result not guaranteed" part, let's disable this branch for the time being ...
	*/
	#if 0
	if ( mode != 5 )
	{
		if ( (raw != transpen) || spd )
		{
			m_vdp1.framebuffer_draw_lines[y][x] = pix;
		}
	}
	else
	#endif
	{
		if ( (raw != transpen) || spd )
		{
			if ( stv2_current_sprite.CMDPMOD & 0x4 ) /* Gouraud shading */
				pix = stv_vdp1_apply_gouraud_shading( x, y, pix );

			switch( stv2_current_sprite.CMDPMOD & 0x3 )
			{
				case 0: /* replace */
					m_vdp1.framebuffer_draw_lines[y][x] = pix;
					break;
				case 1: /* shadow */
					if ( m_vdp1.framebuffer_draw_lines[y][x] & 0x8000 )
					{
						m_vdp1.framebuffer_draw_lines[y][x] = ((m_vdp1.framebuffer_draw_lines[y][x] & ~0x8421) >> 1) | 0x8000;
					}
					break;
				case 2: /* half luminance */
					m_vdp1.framebuffer_draw_lines[y][x] = ((pix & ~0x8421) >> 1) | 0x8000;
					break;
				case 3: /* half transparent */
					if ( m_vdp1.framebuffer_draw_lines[y][x] & 0x8000 )
					{
						m_vdp1.framebuffer_draw_lines[y][x] = alpha_blend_r16( m_vdp1.framebuffer_draw_lines[y][x], pix, 0x80 ) | 0x8000;
					}
					else
					{
						m_vdp1.framebuffer_draw_lines[y][x] = pix;
					}
					break;
				//case 4: /* Gouraud shading */
				// TODO: Pro Yakyuu Team mo Tsukurou (during team creation, on PR girl select)
				//case 6:
				//  break;
				//case 7: /* Gouraud-shading + half-transparent */
					// Lupin the 3rd Pyramid no Kenja enemy shadows
					// Death Crimson lives indicators
					// TODO: latter looks really bad.
				default:
					// TODO: mode 5: prohibited, mode 6: gouraud shading + half-luminance, mode 7: gouraud-shading + half-transparent
					popmessage("VDP1 PMOD = %02x, contact MAMEdev",stv2_current_sprite.CMDPMOD & 0x7);
					m_vdp1.framebuffer_draw_lines[y][x] = pix;
					break;
			}
		}
	}
}


void saturn_state::stv_vdp1_set_drawpixel( void )
{
	int sprite_type = stv2_current_sprite.CMDCTRL & 0x000f;
	int sprite_mode = stv2_current_sprite.CMDPMOD&0x0038;
	int spd = stv2_current_sprite.CMDPMOD & 0x40;
	int mesh = stv2_current_sprite.CMDPMOD & 0x100;
	int ecd = stv2_current_sprite.CMDPMOD & 0x80;

	if ( mesh || !ecd || ((stv2_current_sprite.CMDPMOD & 0x7) != 0) )
	{
		drawpixel = &saturn_state::drawpixel_generic;
		return;
	}

	if(stv2_current_sprite.CMDPMOD & 0x8000)
	{
		drawpixel = &saturn_state::drawpixel_generic;
		return;
	}

	// polygon / polyline / line with replace case
	if (sprite_type & 4 && ((stv2_current_sprite.CMDPMOD & 0x7) == 0))
	{
		drawpixel = &saturn_state::drawpixel_poly;
	}
	else if ( (sprite_mode == 0x20) && !spd )
	{
		m_sprite_colorbank = (stv2_current_sprite.CMDCOLR&0xff00);
		drawpixel = &saturn_state::drawpixel_8bpp_trans;
	}
	else if ((sprite_mode == 0x00) && spd)
	{
		m_sprite_colorbank = (stv2_current_sprite.CMDCOLR&0xfff0);
		drawpixel = &saturn_state::drawpixel_4bpp_notrans;
	}
	else if (sprite_mode == 0x00 && !spd )
	{
		m_sprite_colorbank = (stv2_current_sprite.CMDCOLR&0xfff0);
		drawpixel = &saturn_state::drawpixel_4bpp_trans;
	}
	else
	{
		drawpixel = &saturn_state::drawpixel_generic;
	}
}


void saturn_state::vdp1_fill_slope(const rectangle &cliprect, int patterndata, int xsize,
							int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t *nx1, int32_t *nx2,
							int32_t u1, int32_t u2, int32_t slu1, int32_t slu2, int32_t *nu1, int32_t *nu2,
							int32_t v1, int32_t v2, int32_t slv1, int32_t slv2, int32_t *nv1, int32_t *nv2,
							int32_t _y1, int32_t y2)
{
	if(_y1 > cliprect.max_y)
		return;

	if(y2 <= cliprect.min_y) {
		int delta = y2-_y1;
		*nx1 = x1+delta*sl1;
		*nu1 = u1+delta*slu1;
		*nv1 = v1+delta*slv1;
		*nx2 = x2+delta*sl2;
		*nu2 = u2+delta*slu2;
		*nv2 = v2+delta*slv2;
		return;
	}

	if(y2 > cliprect.max_y)
		y2 = cliprect.max_y+1;

	if(_y1 < cliprect.min_y) {
		int delta = cliprect.min_y - _y1;
		x1 += delta*sl1;
		u1 += delta*slu1;
		v1 += delta*slv1;
		x2 += delta*sl2;
		u2 += delta*slu2;
		v2 += delta*slv2;
		_y1 = cliprect.min_y;
	}

	if(x1 > x2 || (x1==x2 && sl1 > sl2)) {
		int32_t t, *tp;
		t = x1;
		x1 = x2;
		x2 = t;
		t = sl1;
		sl1 = sl2;
		sl2 = t;
		tp = nx1;
		nx1 = nx2;
		nx2 = tp;

		t = u1;
		u1 = u2;
		u2 = t;
		t = slu1;
		slu1 = slu2;
		slu2 = t;
		tp = nu1;
		nu1 = nu2;
		nu2 = tp;

		t = v1;
		v1 = v2;
		v2 = t;
		t = slv1;
		slv1 = slv2;
		slv2 = t;
		tp = nv1;
		nv1 = nv2;
		nv2 = tp;
	}

	while(_y1 < y2) {
		if(_y1 >= cliprect.min_y) {
			int32_t slux = 0, slvx = 0;
			int xx1 = x1>>FRAC_SHIFT;
			int xx2 = x2>>FRAC_SHIFT;
			int32_t u = u1;
			int32_t v = v1;
			if(xx1 != xx2) {
				int delta = xx2-xx1;
				slux = (u2-u1)/delta;
				slvx = (v2-v1)/delta;
			}
			if(xx1 <= cliprect.max_x || xx2 >= cliprect.min_x) {
				if(xx1 < cliprect.min_x) {
					int delta = cliprect.min_x-xx1;
					u += slux*delta;
					v += slvx*delta;
					xx1 = cliprect.min_x;
				}
				if(xx2 > cliprect.max_x)
					xx2 = cliprect.max_x;

				while(xx1 <= xx2) {
					(this->*drawpixel)(xx1,_y1, patterndata, (v>>FRAC_SHIFT)*xsize+(u>>FRAC_SHIFT));
					xx1++;
					u += slux;
					v += slvx;
				}
			}
		}

		x1 += sl1;
		u1 += slu1;
		v1 += slv1;
		x2 += sl2;
		u2 += slu2;
		v2 += slv2;
		_y1++;
	}
	*nx1 = x1;
	*nu1 = u1;
	*nv1 = v1;
	*nx2 = x2;
	*nu2 = u2;
	*nv2 = v2;
}

void saturn_state::vdp1_fill_line(const rectangle &cliprect, int patterndata, int xsize, int32_t y,
							int32_t x1, int32_t x2, int32_t u1, int32_t u2, int32_t v1, int32_t v2)
{
	int xx1 = x1>>FRAC_SHIFT;
	int xx2 = x2>>FRAC_SHIFT;

	if(y > cliprect.max_y || y < cliprect.min_y)
		return;

	if(xx1 <= cliprect.max_x || xx2 >= cliprect.min_x) {
		int32_t slux = 0, slvx = 0;
		int32_t u = u1;
		int32_t v = v1;
		if(xx1 != xx2) {
			int delta = xx2-xx1;
			slux = (u2-u1)/delta;
			slvx = (v2-v1)/delta;
		}
		if(xx1 < cliprect.min_x) {
			int delta = cliprect.min_x-xx1;
			u += slux*delta;
			v += slvx*delta;
			xx1 = cliprect.min_x;
		}
		if(xx2 > cliprect.max_x)
			xx2 = cliprect.max_x;

		while(xx1 <= xx2) {
			(this->*drawpixel)(xx1,y,patterndata,(v>>FRAC_SHIFT)*xsize+(u>>FRAC_SHIFT));
			xx1++;
			u += slux;
			v += slvx;
		}
	}
}

void saturn_state::vdp1_fill_quad(const rectangle &cliprect, int patterndata, int xsize, const struct spoint *q)
{
	int32_t sl1, sl2, slu1, slu2, slv1, slv2, cury, limy, x1, x2, u1, u2, v1, v2, delta;
	int pmin, pmax, i, ps1, ps2;
	struct spoint p[8];

	for(i=0; i<4; i++) {
		p[i].x = p[i+4].x = q[i].x << FRAC_SHIFT;
		p[i].y = p[i+4].y = q[i].y;
		p[i].u = p[i+4].u = q[i].u << FRAC_SHIFT;
		p[i].v = p[i+4].v = q[i].v << FRAC_SHIFT;
	}

	pmin = pmax = 0;
	for(i=1; i<4; i++) {
		if(p[i].y < p[pmin].y)
			pmin = i;
		if(p[i].y > p[pmax].y)
			pmax = i;
	}

	cury = p[pmin].y;
	limy = p[pmax].y;

	if(cury == limy) {
		x1 = x2 = p[0].x;
		u1 = u2 = p[0].u;
		v1 = v2 = p[0].v;
		for(i=1; i<4; i++) {
			if(p[i].x < x1) {
				x1 = p[i].x;
				u1 = p[i].u;
				v1 = p[i].v;
			}
			if(p[i].x > x2) {
				x2 = p[i].x;
				u2 = p[i].u;
				v2 = p[i].v;
			}
		}
		vdp1_fill_line(cliprect, patterndata, xsize, cury, x1, x2, u1, u2, v1, v2);
		return;
	}

	if(cury > cliprect.max_y)
		return;
	if(limy <= cliprect.min_y)
		return;

	if(limy > cliprect.max_y)
		limy = cliprect.max_y;

	ps1 = pmin+4;
	ps2 = pmin;

	goto startup;

	for(;;) {
		if(p[ps1-1].y == p[ps2+1].y) {
			vdp1_fill_slope(cliprect, patterndata, xsize,
							x1, x2, sl1, sl2, &x1, &x2,
							u1, u2, slu1, slu2, &u1, &u2,
							v1, v2, slv1, slv2, &v1, &v2,
							cury, p[ps1-1].y);
			cury = p[ps1-1].y;
			if(cury >= limy)
				break;
			ps1--;
			ps2++;

		startup:
			while(p[ps1-1].y == cury)
				ps1--;
			while(p[ps2+1].y == cury)
				ps2++;
			x1 = p[ps1].x;
			u1 = p[ps1].u;
			v1 = p[ps1].v;
			x2 = p[ps2].x;
			u2 = p[ps2].u;
			v2 = p[ps2].v;

			delta = cury-p[ps1-1].y;
			sl1 = (x1-p[ps1-1].x)/delta;
			slu1 = (u1-p[ps1-1].u)/delta;
			slv1 = (v1-p[ps1-1].v)/delta;

			delta = cury-p[ps2+1].y;
			sl2 = (x2-p[ps2+1].x)/delta;
			slu2 = (u2-p[ps2+1].u)/delta;
			slv2 = (v2-p[ps2+1].v)/delta;
		} else if(p[ps1-1].y < p[ps2+1].y) {
			vdp1_fill_slope(cliprect, patterndata, xsize,
							x1, x2, sl1, sl2, &x1, &x2,
							u1, u2, slu1, slu2, &u1, &u2,
							v1, v2, slv1, slv2, &v1, &v2,
							cury, p[ps1-1].y);
			cury = p[ps1-1].y;
			if(cury >= limy)
				break;
			ps1--;
			while(p[ps1-1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			u1 = p[ps1].u;
			v1 = p[ps1].v;

			delta = cury-p[ps1-1].y;
			sl1 = (x1-p[ps1-1].x)/delta;
			slu1 = (u1-p[ps1-1].u)/delta;
			slv1 = (v1-p[ps1-1].v)/delta;
		} else {
			vdp1_fill_slope(cliprect, patterndata, xsize,
							x1, x2, sl1, sl2, &x1, &x2,
							u1, u2, slu1, slu2, &u1, &u2,
							v1, v2, slv1, slv2, &v1, &v2,
							cury, p[ps2+1].y);
			cury = p[ps2+1].y;
			if(cury >= limy)
				break;
			ps2++;
			while(p[ps2+1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			u2 = p[ps2].u;
			v2 = p[ps2].v;

			delta = cury-p[ps2+1].y;
			sl2 = (x2-p[ps2+1].x)/delta;
			slu2 = (u2-p[ps2+1].u)/delta;
			slv2 = (v2-p[ps2+1].v)/delta;
		}
	}
	if(cury == limy)
		vdp1_fill_line(cliprect, patterndata, xsize, cury, x1, x2, u1, u2, v1, v2);
}

int saturn_state::x2s(int v)
{
	return (int32_t)(int16_t)v + m_vdp1.local_x;
}

int saturn_state::y2s(int v)
{
	return (int32_t)(int16_t)v + m_vdp1.local_y;
}

void saturn_state::stv_vdp1_draw_line(const rectangle &cliprect)
{
	struct spoint q[4];

	q[0].x = x2s(stv2_current_sprite.CMDXA);
	q[0].y = y2s(stv2_current_sprite.CMDYA);
	q[1].x = x2s(stv2_current_sprite.CMDXB);
	q[1].y = y2s(stv2_current_sprite.CMDYB);
	q[2].x = x2s(stv2_current_sprite.CMDXA);
	q[2].y = y2s(stv2_current_sprite.CMDYA);
	q[3].x = x2s(stv2_current_sprite.CMDXB);
	q[3].y = y2s(stv2_current_sprite.CMDYB);

	q[0].u = q[3].u = q[1].u = q[2].u = 0;
	q[0].v = q[1].v = q[2].v = q[3].v = 0;

	vdp1_fill_quad(cliprect, 0, 1, q);
}

void saturn_state::stv_vdp1_draw_poly_line(const rectangle &cliprect)
{
	struct spoint q[4];

	q[0].x = x2s(stv2_current_sprite.CMDXA);
	q[0].y = y2s(stv2_current_sprite.CMDYA);
	q[1].x = x2s(stv2_current_sprite.CMDXB);
	q[1].y = y2s(stv2_current_sprite.CMDYB);
	q[2].x = x2s(stv2_current_sprite.CMDXA);
	q[2].y = y2s(stv2_current_sprite.CMDYA);
	q[3].x = x2s(stv2_current_sprite.CMDXB);
	q[3].y = y2s(stv2_current_sprite.CMDYB);

	q[0].u = q[3].u = q[1].u = q[2].u = 0;
	q[0].v = q[1].v = q[2].v = q[3].v = 0;

	vdp1_fill_quad(cliprect, 0, 1, q);

	q[0].x = x2s(stv2_current_sprite.CMDXB);
	q[0].y = y2s(stv2_current_sprite.CMDYB);
	q[1].x = x2s(stv2_current_sprite.CMDXC);
	q[1].y = y2s(stv2_current_sprite.CMDYC);
	q[2].x = x2s(stv2_current_sprite.CMDXB);
	q[2].y = y2s(stv2_current_sprite.CMDYB);
	q[3].x = x2s(stv2_current_sprite.CMDXC);
	q[3].y = y2s(stv2_current_sprite.CMDYC);

	q[0].u = q[3].u = q[1].u = q[2].u = 0;
	q[0].v = q[1].v = q[2].v = q[3].v = 0;

	vdp1_fill_quad(cliprect, 0, 1, q);

	q[0].x = x2s(stv2_current_sprite.CMDXC);
	q[0].y = y2s(stv2_current_sprite.CMDYC);
	q[1].x = x2s(stv2_current_sprite.CMDXD);
	q[1].y = y2s(stv2_current_sprite.CMDYD);
	q[2].x = x2s(stv2_current_sprite.CMDXC);
	q[2].y = y2s(stv2_current_sprite.CMDYC);
	q[3].x = x2s(stv2_current_sprite.CMDXD);
	q[3].y = y2s(stv2_current_sprite.CMDYD);

	q[0].u = q[3].u = q[1].u = q[2].u = 0;
	q[0].v = q[1].v = q[2].v = q[3].v = 0;

	vdp1_fill_quad(cliprect, 0, 1, q);

	q[0].x = x2s(stv2_current_sprite.CMDXD);
	q[0].y = y2s(stv2_current_sprite.CMDYD);
	q[1].x = x2s(stv2_current_sprite.CMDXA);
	q[1].y = y2s(stv2_current_sprite.CMDYA);
	q[2].x = x2s(stv2_current_sprite.CMDXD);
	q[2].y = y2s(stv2_current_sprite.CMDYD);
	q[3].x = x2s(stv2_current_sprite.CMDXA);
	q[3].y = y2s(stv2_current_sprite.CMDYA);

	q[0].u = q[3].u = q[1].u = q[2].u = 0;
	q[0].v = q[1].v = q[2].v = q[3].v = 0;

	stv_vdp1_setup_shading(q, cliprect);
	vdp1_fill_quad(cliprect, 0, 1, q);

}

void saturn_state::stv_vdp1_draw_distorted_sprite(const rectangle &cliprect)
{
	struct spoint q[4];

	int xsize, ysize;
	int direction;
	int patterndata;

	direction = (stv2_current_sprite.CMDCTRL & 0x0030)>>4;

	if ( stv2_current_sprite.ispoly )
	{
		xsize = ysize = 1;
		patterndata = 0;
	}
	else
	{
		xsize = (stv2_current_sprite.CMDSIZE & 0x3f00) >> 8;
		xsize = xsize * 8;
		if (xsize == 0) return; /* setting prohibited */

		ysize = (stv2_current_sprite.CMDSIZE & 0x00ff);
		if (ysize == 0) return; /* setting prohibited */

		patterndata = (stv2_current_sprite.CMDSRCA) & 0xffff;
		patterndata = patterndata * 0x8;

	}


	q[0].x = x2s(stv2_current_sprite.CMDXA);
	q[0].y = y2s(stv2_current_sprite.CMDYA);
	q[1].x = x2s(stv2_current_sprite.CMDXB);
	q[1].y = y2s(stv2_current_sprite.CMDYB);
	q[2].x = x2s(stv2_current_sprite.CMDXC);
	q[2].y = y2s(stv2_current_sprite.CMDYC);
	q[3].x = x2s(stv2_current_sprite.CMDXD);
	q[3].y = y2s(stv2_current_sprite.CMDYD);

	if(direction & 1) { // xflip
		q[0].u = q[3].u = xsize-1;
		q[1].u = q[2].u = 0;
	} else {
		q[0].u = q[3].u = 0;
		q[1].u = q[2].u = xsize-1;
	}
	if(direction & 2) { // yflip
		q[0].v = q[1].v = ysize-1;
		q[2].v = q[3].v = 0;
	} else {
		q[0].v = q[1].v = 0;
		q[2].v = q[3].v = ysize-1;
	}

	stv_vdp1_setup_shading(q, cliprect);
	vdp1_fill_quad(cliprect, patterndata, xsize, q);
}

void saturn_state::stv_vdp1_draw_scaled_sprite(const rectangle &cliprect)
{
	struct spoint q[4];

	int xsize, ysize;
	int direction;
	int patterndata;
	int zoompoint;
	int x,y;
	int x2,y2;
	int screen_width,screen_height,screen_height_negative = 0;

	direction = (stv2_current_sprite.CMDCTRL & 0x0030)>>4;

	xsize = (stv2_current_sprite.CMDSIZE & 0x3f00) >> 8;
	xsize = xsize * 8;

	ysize = (stv2_current_sprite.CMDSIZE & 0x00ff);

	patterndata = (stv2_current_sprite.CMDSRCA) & 0xffff;
	patterndata = patterndata * 0x8;

	zoompoint = (stv2_current_sprite.CMDCTRL & 0x0f00)>>8;

	x = stv2_current_sprite.CMDXA;
	y = stv2_current_sprite.CMDYA;

	screen_width = (int16_t)stv2_current_sprite.CMDXB;
	if ( (screen_width < 0) && zoompoint)
	{
		screen_width = -screen_width;
		direction |= 1;
	}

	screen_height = (int16_t)stv2_current_sprite.CMDYB;
	if ( (screen_height < 0) && zoompoint )
	{
		screen_height_negative = 1;
		screen_height = -screen_height;
		direction |= 2;
	}

	x2 = stv2_current_sprite.CMDXC; // second co-ordinate set x
	y2 = stv2_current_sprite.CMDYC; // second co-ordinate set y

	switch (zoompoint)
	{
		case 0x0: // specified co-ordinates
			break;
		case 0x5: // up left
			break;
		case 0x6: // up center
			x -= screen_width/2 ;
			break;
		case 0x7: // up right
			x -= screen_width;
			break;

		case 0x9: // center left
			y -= screen_height/2 ;
			break;
		case 0xa: // center center
			y -= screen_height/2 ;
			x -= screen_width/2 ;

			break;

		case 0xb: // center right
			y -= screen_height/2 ;
			x -= screen_width;
			break;

		case 0xd: // center left
			y -= screen_height;
			break;

		case 0xe: // center center
			y -= screen_height;
			x -= screen_width/2 ;
			break;

		case 0xf: // center right
			y -= screen_height;
			x -= screen_width;
			break;

		default: // illegal
			break;

	}

	/*  0----1
	    |    |
	    |    |
	    3----2   */

	if (zoompoint)
	{
		q[0].x = x2s(x);
		q[0].y = y2s(y);
		q[1].x = x2s(x)+screen_width;
		q[1].y = y2s(y);
		q[2].x = x2s(x)+screen_width;
		q[2].y = y2s(y)+screen_height;
		q[3].x = x2s(x);
		q[3].y = y2s(y)+screen_height;

		if ( screen_height_negative )
		{
			q[0].y += screen_height;
			q[1].y += screen_height;
			q[2].y += screen_height;
			q[3].y += screen_height;
		}
	}
	else
	{
		q[0].x = x2s(x);
		q[0].y = y2s(y);
		q[1].x = x2s(x2);
		q[1].y = y2s(y);
		q[2].x = x2s(x2);
		q[2].y = y2s(y2);
		q[3].x = x2s(x);
		q[3].y = y2s(y2);
	}


	if(direction & 1) { // xflip
		q[0].u = q[3].u = xsize-1;
		q[1].u = q[2].u = 0;
	} else {
		q[0].u = q[3].u = 0;
		q[1].u = q[2].u = xsize-1;
	}
	if(direction & 2) { // yflip
		q[0].v = q[1].v = ysize-1;
		q[2].v = q[3].v = 0;
	} else {
		q[0].v = q[1].v = 0;
		q[2].v = q[3].v = ysize-1;
	}

	stv_vdp1_setup_shading(q, cliprect);
	vdp1_fill_quad(cliprect, patterndata, xsize, q);
}




void saturn_state::stv_vdp1_draw_normal_sprite(const rectangle &cliprect, int sprite_type)
{
	int y, ysize, drawypos;
	int x, xsize, drawxpos;
	int direction;
	int patterndata;
	uint8_t shading;
	int su, u, dux, duy;
	int maxdrawypos, maxdrawxpos;

	x = x2s(stv2_current_sprite.CMDXA);
	y = y2s(stv2_current_sprite.CMDYA);

	direction = (stv2_current_sprite.CMDCTRL & 0x0030)>>4;

	xsize = (stv2_current_sprite.CMDSIZE & 0x3f00) >> 8;
	xsize = xsize * 8;

	ysize = (stv2_current_sprite.CMDSIZE & 0x00ff);

	patterndata = (stv2_current_sprite.CMDSRCA) & 0xffff;
	patterndata = patterndata * 0x8;

	if (VDP1_LOG) logerror ("Drawing Normal Sprite x %04x y %04x xsize %04x ysize %04x patterndata %06x\n",x,y,xsize,ysize,patterndata);

	if ( x > cliprect.max_x ) return;
	if ( y > cliprect.max_y ) return;

	shading = stv_read_gouraud_table();
	if ( shading )
	{
		struct spoint q[4];
		q[0].x = x; q[0].y = y;
		q[1].x = x + xsize; q[1].y = y;
		q[2].x = x + xsize; q[2].y = y + ysize;
		q[3].x = x; q[3].y = y + ysize;

		stv_vdp1_setup_shading( q, cliprect );
	}

	u = 0;
	dux = 1;
	duy = xsize;
	if ( direction & 0x1 ) //xflip
	{
		dux = -1;
		u = xsize - 1;
	}
	if ( direction & 0x2 ) //yflip
	{
		duy = -xsize;
		u += xsize*(ysize-1);
	}
	if ( y < cliprect.min_y ) //clip y
	{
		u += xsize*(cliprect.min_y - y);
		ysize -= (cliprect.min_y - y);
		y = cliprect.min_y;
	}
	if ( x < cliprect.min_x ) //clip x
	{
		u += dux*(cliprect.min_x - x);
		xsize -= (cliprect.min_x - x);
		x = cliprect.min_x;
	}
	maxdrawypos = std::min(y+ysize-1,cliprect.max_y);
	maxdrawxpos = std::min(x+xsize-1,cliprect.max_x);
	for (drawypos = y; drawypos <= maxdrawypos; drawypos++ )
	{
		//destline = m_vdp1.framebuffer_draw_lines[drawypos];
		su = u;
		for (drawxpos = x; drawxpos <= maxdrawxpos; drawxpos++ )
		{
			(this->*drawpixel)( drawxpos, drawypos, patterndata, u );
			u += dux;
		}
		u = su + duy;
	}
}

TIMER_CALLBACK_MEMBER(saturn_state::vdp1_draw_end )
{
	/* set CEF to 1*/
	CEF_1;

	// TODO: temporary for Batman Forever, presumably anonymous timer not behaving well.
	#if 0
	if(!(m_scu.ism & IRQ_VDP1_END))
	{
		m_maincpu->set_input_line_and_vector(0x2, HOLD_LINE, 0x4d); // SH2
		scu_do_transfer(6);
	}
	else
		m_scu.ist |= (IRQ_VDP1_END);
	#endif
}


void saturn_state::stv_vdp1_process_list( void )
{
	int position;
	int spritecount;
	int vdp1_nest;
	rectangle *cliprect;

	spritecount = 0;
	position = 0;

	if (VDP1_LOG) logerror ("Sprite List Process START\n");

	vdp1_nest = -1;

	stv_clear_gouraud_shading();

	/*Set CEF bit to 0*/
	CEF_0;

	// TODO: is there an actual limit for this?
	while (spritecount<16383) // max 16383 with texture or max 16384 without texture - vitrually unlimited
	{
		int draw_this_sprite;

		draw_this_sprite = 1;

	//  if (position >= ((0x80000/0x20)/4)) // safety check
	//  {
	//      if (VDP1_LOG) logerror ("Sprite List Position Too High!\n");
	//      position = 0;
	//  }

		spritecount++;

		stv2_current_sprite.CMDCTRL = (m_vdp1_vram[position * (0x20/4)+0] & 0xffff0000) >> 16;

		if (stv2_current_sprite.CMDCTRL == 0x8000)
		{
			if (VDP1_LOG) logerror ("List Terminator (0x8000) Encountered, Sprite List Process END\n");
			goto end; // end of list
		}

		stv2_current_sprite.CMDLINK = (m_vdp1_vram[position * (0x20/4)+0] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDPMOD = (m_vdp1_vram[position * (0x20/4)+1] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDCOLR = (m_vdp1_vram[position * (0x20/4)+1] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDSRCA = (m_vdp1_vram[position * (0x20/4)+2] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDSIZE = (m_vdp1_vram[position * (0x20/4)+2] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDXA   = (m_vdp1_vram[position * (0x20/4)+3] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDYA   = (m_vdp1_vram[position * (0x20/4)+3] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDXB   = (m_vdp1_vram[position * (0x20/4)+4] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDYB   = (m_vdp1_vram[position * (0x20/4)+4] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDXC   = (m_vdp1_vram[position * (0x20/4)+5] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDYC   = (m_vdp1_vram[position * (0x20/4)+5] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDXD   = (m_vdp1_vram[position * (0x20/4)+6] & 0xffff0000) >> 16;
		stv2_current_sprite.CMDYD   = (m_vdp1_vram[position * (0x20/4)+6] & 0x0000ffff) >> 0;
		stv2_current_sprite.CMDGRDA = (m_vdp1_vram[position * (0x20/4)+7] & 0xffff0000) >> 16;
//      stv2_current_sprite.UNUSED  = (m_vdp1_vram[position * (0x20/4)+7] & 0x0000ffff) >> 0;

		/* proecess jump / skip commands, set position for next sprite */
		switch (stv2_current_sprite.CMDCTRL & 0x7000)
		{
			case 0x0000: // jump next
				if (VDP1_LOG) logerror ("Sprite List Process + Next (Normal)\n");
				position++;
				break;
			case 0x1000: // jump assign
				if (VDP1_LOG) logerror ("Sprite List Process + Jump Old %06x New %06x\n", position, (stv2_current_sprite.CMDLINK>>2));
				position= (stv2_current_sprite.CMDLINK>>2);
				break;
			case 0x2000: // jump call
				if (vdp1_nest == -1)
				{
					if (VDP1_LOG) logerror ("Sprite List Process + Call Old %06x New %06x\n",position, (stv2_current_sprite.CMDLINK>>2));
					vdp1_nest = position+1;
					position = (stv2_current_sprite.CMDLINK>>2);
				}
				else
				{
					if (VDP1_LOG) logerror ("Sprite List Nested Call, ignoring\n");
					position++;
				}
				break;
			case 0x3000:
				if (vdp1_nest != -1)
				{
					if (VDP1_LOG) logerror ("Sprite List Process + Return\n");
					position = vdp1_nest;
					vdp1_nest = -1;
				}
				else
				{
					if (VDP1_LOG) logerror ("Attempted return from no subroutine, aborting\n");
					position++;
					goto end; // end of list
				}
				break;
			case 0x4000:
				draw_this_sprite = 0;
				position++;
				break;
			case 0x5000:
				if (VDP1_LOG) logerror ("Sprite List Skip + Jump Old %06x New %06x\n", position, (stv2_current_sprite.CMDLINK>>2));
				draw_this_sprite = 0;
				position= (stv2_current_sprite.CMDLINK>>2);

				break;
			case 0x6000:
				draw_this_sprite = 0;
				if (vdp1_nest == -1)
				{
					if (VDP1_LOG) logerror ("Sprite List Skip + Call To Subroutine Old %06x New %06x\n",position, (stv2_current_sprite.CMDLINK>>2));

					vdp1_nest = position+1;
					position = (stv2_current_sprite.CMDLINK>>2);
				}
				else
				{
					if (VDP1_LOG) logerror ("Sprite List Nested Call, ignoring\n");
					position++;
				}
				break;
			case 0x7000:
				draw_this_sprite = 0;
				if (vdp1_nest != -1)
				{
					if (VDP1_LOG) logerror ("Sprite List Skip + Return from Subroutine\n");

					position = vdp1_nest;
					vdp1_nest = -1;
				}
				else
				{
					if (VDP1_LOG) logerror ("Attempted return from no subroutine, aborting\n");
					position++;
					goto end; // end of list
				}
				break;
		}

		/* continue to draw this sprite only if the command wasn't to skip it */
		if (draw_this_sprite ==1)
		{
			if ( stv2_current_sprite.CMDPMOD & 0x0400 )
			{
				//if(stv2_current_sprite.CMDPMOD & 0x0200) /* TODO: Bio Hazard inventory screen uses outside cliprect */
				//  cliprect = &m_vdp1.system_cliprect;
				//else
					cliprect = &m_vdp1.user_cliprect;
			}
			else
			{
				cliprect = &m_vdp1.system_cliprect;
			}

			stv_vdp1_set_drawpixel();

			switch (stv2_current_sprite.CMDCTRL & 0x000f)
			{
				case 0x0000:
					if (VDP1_LOG) logerror ("Sprite List Normal Sprite (%d %d)\n",stv2_current_sprite.CMDXA,stv2_current_sprite.CMDYA);
					stv2_current_sprite.ispoly = 0;
					stv_vdp1_draw_normal_sprite(*cliprect, 0);
					break;

				case 0x0001:
					if (VDP1_LOG) logerror ("Sprite List Scaled Sprite (%d %d)\n",stv2_current_sprite.CMDXA,stv2_current_sprite.CMDYA);
					stv2_current_sprite.ispoly = 0;
					stv_vdp1_draw_scaled_sprite(*cliprect);
					break;

				case 0x0002:
				case 0x0003: // used by Hardcore 4x4
					if (VDP1_LOG) logerror ("Sprite List Distorted Sprite\n");
					if (VDP1_LOG) logerror ("(A: %d %d)\n",stv2_current_sprite.CMDXA,stv2_current_sprite.CMDYA);
					if (VDP1_LOG) logerror ("(B: %d %d)\n",stv2_current_sprite.CMDXB,stv2_current_sprite.CMDYB);
					if (VDP1_LOG) logerror ("(C: %d %d)\n",stv2_current_sprite.CMDXC,stv2_current_sprite.CMDYC);
					if (VDP1_LOG) logerror ("(D: %d %d)\n",stv2_current_sprite.CMDXD,stv2_current_sprite.CMDYD);
					if (VDP1_LOG) logerror ("CMDPMOD = %04x\n",stv2_current_sprite.CMDPMOD);

					stv2_current_sprite.ispoly = 0;
					stv_vdp1_draw_distorted_sprite(*cliprect);
					break;

				case 0x0004:
					if (VDP1_LOG) logerror ("Sprite List Polygon\n");
					stv2_current_sprite.ispoly = 1;
					stv_vdp1_draw_distorted_sprite(*cliprect);
					break;

				case 0x0005:
//              case 0x0007: // mirror? Baroque uses it, crashes for whatever reason
					if (VDP1_LOG) logerror ("Sprite List Polyline\n");
					stv2_current_sprite.ispoly = 1;
					stv_vdp1_draw_poly_line(*cliprect);
					break;

				case 0x0006:
					if (VDP1_LOG) logerror ("Sprite List Line\n");
					stv2_current_sprite.ispoly = 1;
					stv_vdp1_draw_line(*cliprect);
					break;

				case 0x0008:
//              case 0x000b: // mirror? Bug 2
					if (VDP1_LOG) logerror ("Sprite List Set Command for User Clipping (%d,%d),(%d,%d)\n", stv2_current_sprite.CMDXA, stv2_current_sprite.CMDYA, stv2_current_sprite.CMDXC, stv2_current_sprite.CMDYC);
					m_vdp1.user_cliprect.set(stv2_current_sprite.CMDXA, stv2_current_sprite.CMDXC, stv2_current_sprite.CMDYA, stv2_current_sprite.CMDYC);
					break;

				case 0x0009:
					if (VDP1_LOG) logerror ("Sprite List Set Command for System Clipping (0,0),(%d,%d)\n", stv2_current_sprite.CMDXC, stv2_current_sprite.CMDYC);
					m_vdp1.system_cliprect.set(0, stv2_current_sprite.CMDXC, 0, stv2_current_sprite.CMDYC);
					break;

				case 0x000a:
					if (VDP1_LOG) logerror ("Sprite List Local Co-Ordinate Set (%d %d)\n",(int16_t)stv2_current_sprite.CMDXA,(int16_t)stv2_current_sprite.CMDYA);
					m_vdp1.local_x = (int16_t)stv2_current_sprite.CMDXA;
					m_vdp1.local_y = (int16_t)stv2_current_sprite.CMDYA;
					break;

				default:
					popmessage ("VDP1: Sprite List Illegal %02x (%d), contact MAMEdev",stv2_current_sprite.CMDCTRL & 0xf,spritecount);
					m_vdp1.lopr = (position * 0x20) >> 3;
					//m_vdp1.copr = (position * 0x20) >> 3;
					// prematurely kill the VDP1 process if an illegal opcode is executed
					// Sexy Parodius calls multiple illegals and expects VDP1 irq to be fired anyway!
					goto end;
			}
		}

	}


	end:
	m_vdp1.copr = (position * 0x20) >> 3;


	/* TODO: what's the exact formula? Guess it should be a mix between number of pixels written and actual command data fetched. */
	// if spritecount = 10000 don't send a vdp1 draw end
//  if(spritecount < 10000)
	machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(spritecount*16), timer_expired_delegate(FUNC(saturn_state::vdp1_draw_end),this));

	if (VDP1_LOG) logerror ("End of list processing!\n");
}

void saturn_state::video_update_vdp1( void )
{
	int framebuffer_changed = 0;

//  int enable;
//  if (machine.input().code_pressed (KEYCODE_R)) VDP1_LOG = 1;
//  if (machine.input().code_pressed (KEYCODE_T)) VDP1_LOG = 0;

//  if (machine.input().code_pressed (KEYCODE_Y)) VDP1_LOG = 0;
//  {
//      FILE *fp;
//
//      fp=fopen("vdp1_ram.dmp", "w+b");
//      if (fp)
//      {
//          fwrite(stv_vdp1, 0x00100000, 1, fp);
//          fclose(fp);
//      }
//  }
	if (VDP1_LOG) logerror("video_update_vdp1 called\n");
	if (VDP1_LOG) logerror( "FBCR = %0x, accessed = %d\n", STV_VDP1_FBCR, m_vdp1.fbcr_accessed );

	if(STV_VDP1_CEF)
		BEF_1;
	else
		BEF_0;

	if ( m_vdp1.framebuffer_clear_on_next_frame )
	{
		if ( ((STV_VDP1_FBCR & 0x3) == 3) &&
			m_vdp1.fbcr_accessed )
		{
			stv_clear_framebuffer(m_vdp1.framebuffer_current_display);
			m_vdp1.framebuffer_clear_on_next_frame = 0;
		}
	}

	switch( STV_VDP1_FBCR & 0x3 )
	{
		case 0: /* Automatic mode */
			stv_vdp1_change_framebuffers();
			stv_clear_framebuffer(m_vdp1.framebuffer_current_draw);
			framebuffer_changed = 1;
			break;
		case 1: /* Setting prohibited */
			break;
		case 2: /* Manual mode - erase */
			if ( m_vdp1.fbcr_accessed )
			{
				m_vdp1.framebuffer_clear_on_next_frame = 1;
			}
			break;
		case 3: /* Manual mode - change */
			if ( m_vdp1.fbcr_accessed )
			{
				stv_vdp1_change_framebuffers();
				if ( STV_VDP1_VBE )
				{
					stv_clear_framebuffer(m_vdp1.framebuffer_current_draw);
				}
				/* TODO: Slam n Jam 96 & Cross Romance doesn't like this, investigate. */
				framebuffer_changed = 1;
			}
	//      framebuffer_changed = 1;
			break;
	}
	m_vdp1.fbcr_accessed = 0;

	if (VDP1_LOG) logerror( "PTM = %0x, TVM = %x\n", STV_VDP1_PTM, STV_VDP1_TVM );
	/*Set CEF bit to 0*/
	//CEF_0;
	switch(STV_VDP1_PTM & 3)
	{
		case 0:/*Idle Mode*/
			/*Set CEF bit to 0*/
			//CEF_0;
			break;
		case 1:/*Draw by request*/
			/*Set CEF bit to 0*/
			//CEF_0;
			break;
		case 2:/*Automatic Draw*/
			if ( framebuffer_changed || VDP1_LOG )
			{
				/*set CEF to 1*/
				stv_vdp1_process_list();
			}
			break;
		case 3: /*<invalid>*/
			logerror("Warning: Invalid PTM mode set for VDP1!\n");
			break;
	}
	//popmessage("%04x %04x",STV_VDP1_EWRR_X3,STV_VDP1_EWRR_Y3);
}

void saturn_state::stv_vdp1_state_save_postload( void )
{
	uint8_t *vdp1 = m_vdp1.gfx_decode.get();
	int offset;
	uint32_t data;

	m_vdp1.framebuffer_mode = -1;
	m_vdp1.framebuffer_double_interlace = -1;

	stv_set_framebuffer_config();

	for (offset = 0; offset < 0x80000/4; offset++ )
	{
		data = m_vdp1_vram[offset];
		/* put in gfx region for easy decoding */
		vdp1[offset*4+0] = (data & 0xff000000) >> 24;
		vdp1[offset*4+1] = (data & 0x00ff0000) >> 16;
		vdp1[offset*4+2] = (data & 0x0000ff00) >> 8;
		vdp1[offset*4+3] = (data & 0x000000ff) >> 0;
	}
}

int saturn_state::stv_vdp1_start ( void )
{
	m_vdp1_regs = make_unique_clear<uint16_t[]>(0x020/2 );
	m_vdp1_vram = make_unique_clear<uint32_t[]>(0x100000/4 );
	m_vdp1.gfx_decode = std::make_unique<uint8_t[]>(0x100000 );

	stv_vdp1_shading_data = std::make_unique<struct stv_vdp1_poly_scanline_data>();

	m_vdp1.framebuffer[0] = std::make_unique<uint16_t[]>(1024 * 256 * 2 ); /* *2 is for double interlace */
	m_vdp1.framebuffer[1] = std::make_unique<uint16_t[]>(1024 * 256 * 2 );

	m_vdp1.framebuffer_display_lines = auto_alloc_array(machine(), uint16_t *, 512);
	m_vdp1.framebuffer_draw_lines = auto_alloc_array(machine(), uint16_t *, 512);

	m_vdp1.framebuffer_width = m_vdp1.framebuffer_height = 0;
	m_vdp1.framebuffer_mode = -1;
	m_vdp1.framebuffer_double_interlace = -1;
	m_vdp1.fbcr_accessed = 0;
	m_vdp1.framebuffer_current_display = 0;
	m_vdp1.framebuffer_current_draw = 1;
	stv_clear_framebuffer(m_vdp1.framebuffer_current_draw);
	m_vdp1.framebuffer_clear_on_next_frame = 0;

	m_vdp1.system_cliprect.set(0, 0, 0, 0);
	/* Kidou Senshi Z Gundam - Zenpen Zeta no Kodou loves to use the user cliprect vars in an undefined state ... */
	m_vdp1.user_cliprect.set(0, 512, 0, 256);

	// save state
	save_pointer(NAME(m_vdp1_regs), 0x020/2);
	save_pointer(NAME(m_vdp1_vram), 0x100000/4);
	save_item(NAME(m_vdp1.fbcr_accessed));
	save_item(NAME(m_vdp1.framebuffer_current_display));
	save_item(NAME(m_vdp1.framebuffer_current_draw));
	save_item(NAME(m_vdp1.framebuffer_clear_on_next_frame));
	save_item(NAME(m_vdp1.local_x));
	save_item(NAME(m_vdp1.local_y));
	machine().save().register_postload(save_prepost_delegate(FUNC(saturn_state::stv_vdp1_state_save_postload), this));
	return 0;
}
