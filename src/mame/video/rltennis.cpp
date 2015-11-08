// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/****************************************************************************************
 Reality Tennis - (c) 1993 TCH



Blitter registers description (reg/bit names selected arbitrary ) :

700000 - BLT_X_START
     JOANA  JOAQUIN
  fedcba98 76543210
  -------- xxxxxxxx dst x start
  xxxxxxxx -------- src x start


700002 - BLT_X_END
     JOANA  JOAQUIN
  fedcba98 76543210
  -------- xxxxxxxx dst x end
  xxxxxxxx -------- src x end


700004 - BLT_Y_START
     JOANA  JOAQUIN
  fedcba98 76543210    x start
  -------- xxxxxxxx
  xxxxxxxx --------


700006 - BLT_Y_END
     JOANA  JOAQUIN
  fedcba98 76543210    x start
  -------- xxxxxxxx
  xxxxxxxx --------


700008 - BLT_FLAGS
     JOANA  JOAQUIN
  fedcba98 76543210
  -------- -------x BLTFLAG_DST_X_DIR    x dst direction (step inc or dec)
  -------- ------x- BLTFLAG_DST_Y_DIR    y dst direction
  -------- -----x-- BLTFLAG_DST_LR       LR and UD controlls the quarter of framebuffer to use
  -------- ----x--- BLTFLAG_DST_UD       /
  -------- ---?----                      unknown
  -------- --x----- BLTFLAG_DISPLAY_UD   display buffer select
  -------- ??------                      unknown
  -------x -------- BLTFLAG_SRC_X_DIR    x src direction
  ------x- -------- BLTFLAG_SRC_Y_DIR    y src direction
  -----x-- -------- BLTFLAG_SRC_LR       LR and UD controlls the quarter of src buffer to use
  ----x--- -------- BLTFLAG_SRC_UD       /
  xxxx---- --------                      src ROM num


70000a - BLT_UNK
     JOANA  JOAQUIN
  fedcba98 76543210
  ???????? ???????? unknown (used during gameplay ... zoom factors ?)


70000c - BLT_START
  fedcba98 76543210
  --?????- ???????? unknown
  -------x -------- BLTSTRT_ROM_MSB     src ROM MSB
  -x------ -------- BLTSTRT_TRIGGER     blit start
  x------- -------- BLTSTRT_LAYER       FG or BG layer of framebuffer

70000e - BLT_UNK2
     JOANA  JOAQUIN
  fedcba98 76543210
  ???????? ???????? unknown (set to 0 @ boot)

****************************************************************************************/
#include "emu.h"
#include "includes/rltennis.h"

enum
{
	BLT_X_START = 0,
	BLT_X_END,
	BLT_Y_START,
	BLT_Y_END,
	BLT_FLAGS,
	BLT_UNK,
	BLT_START,
	BLT_UNK2
};

enum
{
	BITMAP_BG=0,
	BITMAP_FG_1,
	BITMAP_FG_2,
	BITMAP_FG_DISPLAY
};

#define BLTFLAG_DST_X_DIR   (1<<0)
#define BLTFLAG_DST_Y_DIR   (1<<1)
#define BLTFLAG_DST_LR      (1<<2)
#define BLTFLAG_DST_UD      (1<<3)
#define BLTFLAG_DISPLAY_UD  (1<<5)

#define BLTFLAG_SRC_X_DIR   (1<<8)
#define BLTFLAG_SRC_Y_DIR   (1<<9)
#define BLTFLAG_SRC_LR      (1<<10)
#define BLTFLAG_SRC_UD      (1<<11)


#define BLTSTRT_ROM_MSB     (1<<8)
#define BLTSTRT_TRIGGER     (1<<14)
#define BLTSTRT_LAYER       (1<<15)

#define SRC_SHIFT           8

WRITE16_MEMBER(rltennis_state::blitter_w)
{
	int old_data=m_blitter[offset];
	COMBINE_DATA(&m_blitter[offset]);
	int new_data=m_blitter[offset];

	if(offset==BLT_FLAGS && ((new_data^old_data) & BLTFLAG_DISPLAY_UD) )  /* visible page flip and clear */
	{
		if(new_data & BLTFLAG_DISPLAY_UD)
		{
			copybitmap(*m_tmp_bitmap[BITMAP_FG_DISPLAY], *m_tmp_bitmap[BITMAP_FG_1], 0, 0, 0, 0, m_tmp_bitmap[BITMAP_FG_DISPLAY]->cliprect());
			m_tmp_bitmap[BITMAP_FG_1]->fill(0);
		}
		else
		{
			copybitmap(*m_tmp_bitmap[BITMAP_FG_DISPLAY], *m_tmp_bitmap[BITMAP_FG_2], 0, 0, 0, 0, m_tmp_bitmap[BITMAP_FG_DISPLAY]->cliprect());
			m_tmp_bitmap[BITMAP_FG_2]->fill(0);
		}
	}

	if(offset == BLT_START && (((new_data ^ old_data ) & new_data) & BLTSTRT_TRIGGER))  /* blit strobe 0->1 */
	{
		m_maincpu->set_input_line(1, HOLD_LINE);

		int src_x0=(m_blitter[BLT_X_START]>>SRC_SHIFT)+((m_blitter[BLT_FLAGS] & BLTFLAG_SRC_LR)?256:0);
		int src_y0=(m_blitter[BLT_Y_START]>>SRC_SHIFT)+((m_blitter[BLT_FLAGS]>>3)&0xff00)+(((m_blitter[BLT_START]) & BLTSTRT_ROM_MSB)?(1<<0xd):0);

		int dst_x0=(m_blitter[BLT_X_START]&0xff);
		int dst_y0=(m_blitter[BLT_Y_START]&0xff);

		int dst_x1=(m_blitter[BLT_X_END]&0xff);
		int dst_y1=(m_blitter[BLT_Y_END]&0xff);

		int src_x1=((m_blitter[BLT_X_END]>>SRC_SHIFT)&0xff)+((m_blitter[BLT_FLAGS] & BLTFLAG_SRC_LR)?256:0);
		int src_y1=((m_blitter[BLT_Y_END]>>SRC_SHIFT)&0xff)+((m_blitter[BLT_FLAGS]>>3)&0xff00)+(((m_blitter[BLT_START]) & BLTSTRT_ROM_MSB)?(1<<0xd):0);

		int x_dst_step=(m_blitter[BLT_FLAGS] & BLTFLAG_DST_X_DIR)?1:-1;
		int y_dst_step=(m_blitter[BLT_FLAGS] & BLTFLAG_DST_Y_DIR)?1:-1;

		int x_src_step=(m_blitter[BLT_FLAGS] & BLTFLAG_SRC_X_DIR)?1:-1;
		int y_src_step=(m_blitter[BLT_FLAGS] & BLTFLAG_SRC_Y_DIR)?1:-1;

		int x,y;

		int idx_x,idx_y;

		int blit_w=src_x1-src_x0;
		int blit_h=src_y1-src_y0;

		int blit_w1=dst_x1-dst_x0;
		int blit_h1=dst_y1-dst_y0;

		if(blit_w1<0) blit_w1=(-blit_w1)^0xff; /* is it correct ? game does that when flips images */
		if(blit_h1<0) blit_h1=-blit_h1;

		if(blit_w<0) blit_w=-blit_w;
		if(blit_h<0) blit_h=-blit_h;

		{
			/* wrong, causes gfx glitches (wrong size , but gives (so far) the best results */
			if(blit_w1<blit_w) blit_w1=blit_w;
			if(blit_h1<blit_h) blit_h1=blit_h;
		}

		int layer=(m_blitter[BLT_START] & BLTSTRT_LAYER )?BITMAP_BG:BITMAP_FG_1;

		if(layer==BITMAP_FG_1)
		{
			if(m_blitter[BLT_FLAGS] & BLTFLAG_DST_UD )
			{
				layer=BITMAP_FG_2;
			}
		}

		bool force_blit=false;

		if(blit_w==1 && blit_h==1) /* seems to be bg layer color fill */
		{
			force_blit=true;
		}

		for( x=dst_x0, idx_x=0 ; idx_x<=blit_w1; x+=x_dst_step, idx_x++ )
		{
			for( y=dst_y0, idx_y=0 ; idx_y<=blit_h1;y+=y_dst_step, idx_y++)
			{
				int xx=src_x0+(x_src_step*idx_x);
				int yy=src_y0+(y_src_step*idx_y);

				if(force_blit)
				{
					xx=src_x0;
					yy=src_y0;
				}

				int address=yy*512+xx;

				int pix = m_gfx[ address & 0x0ffffff ];
				int screen_x=(x&0xff)+((m_blitter[BLT_FLAGS] & BLTFLAG_DST_LR )?256:0);

				if((pix || force_blit)&& screen_x >0 && y >0 && screen_x < 512 && y < 256 )
				{
					m_tmp_bitmap[layer]->pix16(y  , screen_x ) = pix;
				}
			}
		}
	}
}

void rltennis_state::video_start()
{
	m_tmp_bitmap[BITMAP_BG] = auto_bitmap_ind16_alloc(machine(), 512, 256);
	m_tmp_bitmap[BITMAP_FG_1] = auto_bitmap_ind16_alloc(machine(), 512, 256);
	m_tmp_bitmap[BITMAP_FG_2] = auto_bitmap_ind16_alloc(machine(), 512, 256);
	m_tmp_bitmap[BITMAP_FG_DISPLAY] = auto_bitmap_ind16_alloc(machine(), 512, 256);

	save_item(NAME(m_blitter));
}

UINT32 rltennis_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_tmp_bitmap[BITMAP_BG], 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, *m_tmp_bitmap[BITMAP_FG_DISPLAY], 0, 0, 0, 0, cliprect, 0);
	return 0;
}
