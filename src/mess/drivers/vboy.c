/***************************************************************************

    Nintendo Virtual Boy

    driver by Miodrag Milanovic & Angelo Salese

    Great info at http://www.goliathindustries.com/vb/
    and http://www.vr32.de/modules/dokuwiki/doku.php?

    TODO:
	- 100us / 20us timers
	- sound
    - per-game NVRAM hook-up (wariolnd, vleague, golf, others?)
	- 3dtetris: missing gfxs on gameplay (writes to framebuffer)
    - boundh: game is way too fast
    - galactic: ball goes out of bounds sometimes?
	- golf: missing gfxs on gameplay (writes to framebuffer)
    - marioten: title screen logo is misplaced if Mario completes his animation
    - nesterfb: once that you hit the pins, animation phase takes a while to start
    - redalarm: gameplay doesn't work
    - spaceinv: Taito logo only if you press the button, framebuffer?
    - spaceinv: missing shots
    - telerobo: crashes if you die
    - telerobo: hangs after winning first match;
   	- vlab: doesn't boot (irq issue?)
    - wariolnd: brightness gets suddently darker during intro.
	- waterwld: doesn't accept start input at title screen (regression, it used to work);

****************************************************************************/

#include "emu.h"
#include "cpu/v810/v810.h"
#include "imagedev/cartslot.h"
#include "audio/vboy.h"
#include "vboy.lh"

typedef struct _vboy_regs_t vboy_regs_t;

/* FIXME: most if not all of these must be UINT8 */
struct _vboy_regs_t
{
	UINT32 lpc, lpc2, lpt, lpr;
	UINT32 khb, klb;
	UINT8 thb, tlb;
	UINT32 tcr, wcr, kcr;
};

typedef struct _vip_regs_t vip_regs_t;
struct _vip_regs_t
{

	UINT16 INTPND;
	UINT16 INTENB;
	UINT16 DPSTTS;
	UINT16 DPCTRL;
	UINT16 BRTA;
	UINT16 BRTB;
	UINT16 BRTC;
	UINT16 REST;
	UINT16 FRMCYC;
	UINT16 CTA;
	UINT16 XPSTTS;
	UINT16 XPCTRL;
	UINT16 VER;
	UINT16 SPT[4];
	UINT16 GPLT[4];
	UINT16 JPLT[4];
	UINT16 BKCOL;
};

struct _vboy_timer_t
{
	UINT16 count;
	UINT16 latch;
};

typedef struct _vboy_timer_t vboy_timer_t;

class vboy_state : public driver_device
{
public:
	vboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
		  { }

	required_device<cpu_device> m_maincpu;

	DECLARE_READ32_MEMBER(io_r);
	DECLARE_WRITE32_MEMBER(io_w);
	DECLARE_READ16_MEMBER(vip_r);
	DECLARE_WRITE16_MEMBER(vip_w);
	DECLARE_WRITE16_MEMBER(vboy_font0_w);
	DECLARE_WRITE16_MEMBER(vboy_font1_w);
	DECLARE_WRITE16_MEMBER(vboy_font2_w);
	DECLARE_WRITE16_MEMBER(vboy_font3_w);
	DECLARE_READ16_MEMBER(vboy_font0_r);
	DECLARE_READ16_MEMBER(vboy_font1_r);
	DECLARE_READ16_MEMBER(vboy_font2_r);
	DECLARE_READ16_MEMBER(vboy_font3_r);
	DECLARE_WRITE16_MEMBER(vboy_bgmap_w);
	DECLARE_READ16_MEMBER(vboy_bgmap_r);
	DECLARE_READ8_MEMBER(vboy_lfb0_r);
	DECLARE_READ8_MEMBER(vboy_lfb1_r);
	DECLARE_READ8_MEMBER(vboy_rfb0_r);
	DECLARE_READ8_MEMBER(vboy_rfb1_r);
	DECLARE_WRITE8_MEMBER(vboy_lfb0_w);
	DECLARE_WRITE8_MEMBER(vboy_lfb1_w);
	DECLARE_WRITE8_MEMBER(vboy_rfb0_w);
	DECLARE_WRITE8_MEMBER(vboy_rfb1_w);
	UINT16 *m_font;
	UINT16 *m_bgmap;
	UINT8 *m_l_frame_0;
	UINT8 *m_l_frame_1;
	UINT8 *m_r_frame_0;
	UINT8 *m_r_frame_1;
	UINT16 *m_world;
	UINT16 *m_columntab1;
	UINT16 *m_columntab2;
	UINT16 *m_objects;
	vboy_regs_t m_vboy_regs;
	vip_regs_t m_vip_regs;
	vboy_timer_t m_vboy_timer;
	int *m_bg_map;
	int *m_ovr_map;
	UINT16 m_frame_count;
	UINT8 m_displayfb;
	UINT8 m_drawfb;
	UINT8 m_row_num;
	attotime m_input_latch_time;
	void m_timer_tick(UINT8 setting);
	void m_scanline_tick(int scanline, UINT8 screen_type);
	void m_set_irq(UINT16 irq_vector);

	void m_pcg_debug(UINT16 offset,UINT16 data,UINT16 mem_mask);

	void put_obj(bitmap_ind16 &bitmap, int x, int y, UINT16 code, bool flipx, bool flipy, UINT8 pal);
	void put_char(int x, int y, UINT16 code, bool flipx, bool flipy, UINT8 pal);
	void fill_ovr_char(UINT16 code, bool flipx, bool flipy, UINT8 pal);
	void fill_bg_map(int num, UINT16 scx, UINT16 scy);
	void draw_bg_map(bitmap_ind16 &bitmap, UINT16 *vboy_paramtab, int mode, int gx, int gp, int gy, int mx, int mp, int my,int h, int w,
	                                       UINT16 x_mask, UINT16 y_mask, UINT8 ovr, bool right);
	void draw_affine_map(bitmap_ind16 &bitmap, UINT16 *vboy_paramtab, int gx, int gp, int gy, int h, int w,
	                                           UINT16 x_mask, UINT16 y_mask, UINT8 ovr, bool right);
	UINT8 display_world(int num, bitmap_ind16 &bitmap, bool right, int &cur_spt);
	void m_set_brightness(void);
};


static VIDEO_START( vboy )
{
	vboy_state *state = machine.driver_data<vboy_state>();
	//int i;

	// Allocate memory for temporary screens
	state->m_bg_map = auto_alloc_array_clear(machine, int, 0x1000*0x1000);
	state->m_ovr_map = auto_alloc_array_clear(machine, int, 8*8);

	// Allocate memory for framebuffers
	state->m_l_frame_0 = auto_alloc_array_clear(machine, UINT8, 0x6000);
	state->m_l_frame_1 = auto_alloc_array_clear(machine, UINT8, 0x6000);
	state->m_r_frame_0 = auto_alloc_array_clear(machine, UINT8, 0x6000);
	state->m_r_frame_1 = auto_alloc_array_clear(machine, UINT8, 0x6000);

	state->m_font  = auto_alloc_array_clear(machine, UINT16, 2048 * 8);
	state->m_bgmap = auto_alloc_array(machine, UINT16, 0x20000 >> 1);
	state->m_objects = state->m_bgmap + (0x1e000 >> 1);
	state->m_columntab1 = state->m_bgmap + (0x1dc00 >> 1);
	state->m_columntab2 = state->m_bgmap + (0x1de00 >> 1);
	state->m_world = state->m_bgmap + (0x1d800 >> 1);
}

void vboy_state::put_obj(bitmap_ind16 &bitmap, int x, int y, UINT16 code, bool flipx, bool flipy, UINT8 pal)
{
	UINT16 data;
	UINT8 yi, xi, dat, col;

	for (yi = 0; yi < 8; yi++)
	{
		if (!flipy)
			 data = m_font[code * 8 + yi];
		else
			 data = m_font[code * 8 + (7-yi)];

		for (xi = 0; xi < 8; xi++)
		{
			int res_x,res_y;

			if (!flipx)
				dat = ((data >> (xi << 1)) & 0x03);
			else
				dat = ((data >> ((7-xi) << 1)) & 0x03);

			res_x = x + xi;
			res_y = y + yi;

			col = (pal >> (dat*2)) & 3;

			if (dat && res_x < 384 && res_y < 224)
				bitmap.pix16((res_y), (res_x)) = machine().pens[col];
		}
	}
}

void vboy_state::put_char(int x, int y, UINT16 code, bool flipx, bool flipy, UINT8 pal)
{
	UINT16 data;
	UINT8 yi, xi, dat;
	int col;

	for (yi = 0; yi < 8; yi++)
	{
		if (!flipy)
			 data = m_font[code * 8 + yi];
		else
			 data = m_font[code * 8 + (7-yi)];

		for (xi = 0; xi < 8; xi++)
		{
			int res_x,res_y;

			if (!flipx)
				dat = ((data >> (xi << 1)) & 0x03);
			else
				dat = ((data >> ((7-xi) << 1)) & 0x03);

			res_x = x + xi;
			res_y = y + yi;

			col = (pal >> (dat*2)) & 3;

			if(dat == 0)
				col = -1;

			m_bg_map[res_y*0x1000+res_x] = col;
		}
	}
}

void vboy_state::fill_ovr_char(UINT16 code, bool flipx, bool flipy, UINT8 pal)
{
	UINT16 data;
	UINT8 yi, xi, dat;
	int col;

	for (yi = 0; yi < 8; yi++)
	{
		if (!flipy)
			 data = m_font[code * 8 + yi];
		else
			 data = m_font[code * 8 + (7-yi)];

		for (xi = 0; xi < 8; xi++)
		{
			if (!flipx)
				dat = ((data >> (xi << 1)) & 0x03);
			else
				dat = ((data >> ((7-xi) << 1)) & 0x03);

			col = (pal >> (dat*2)) & 3;

			if(dat == 0)
				col = -1;

			m_ovr_map[yi*8+xi] = col;
		}
	}
}

void vboy_state::fill_bg_map(int num, UINT16 scx, UINT16 scy)
{
	int x, y;
	UINT8 stepx, stepy;

	// Fill background map
	for (y = 0; y < scy; y++)
	{
		for (x = 0; x < scx; x++)
		{
			stepx = (x & 0x1c0) >> 6;
			stepy = ((y & 0x1c0) >> 6) * (stepx+1);
			UINT16 val = m_bgmap[(x & 0x3f) + (64 * (y & 0x3f)) + ((num + stepx + stepy) * 0x1000)];
			put_char(x * 8, y * 8, val & 0x7ff, BIT(val,13), BIT(val,12), m_vip_regs.GPLT[(val >> 14) & 3]);
		}
	}
}

void vboy_state::draw_bg_map(bitmap_ind16 &bitmap, UINT16 *vboy_paramtab, int mode, int gx, int gp, int gy, int mx, int mp, int my, int h, int w,
                                                   UINT16 x_mask, UINT16 y_mask, UINT8 ovr, bool right)
{
	int x,y;

	for(y=0;y<=h;y++)
	{
		for(x=0;x<=w;x++)
		{
			int src_x,src_y;
			INT32 y1 = (y+gy);
			INT32 x1 = (x+gx);
			int pix = 0;

			x1 += right ? -gp : gp;

			src_x = x+mx;
			if (mode==1)
				src_x += (INT16)vboy_paramtab[y*2+(right ^ 1)];

			src_y = y+my;
			src_x += right ? -mp : mp;

			if(ovr && (src_x > x_mask || src_y > y_mask || src_x < 0 || src_y < 0))
				pix = m_ovr_map[(src_y & 7)*8+(src_x & 7)];
			else
				pix = m_bg_map[(src_y & y_mask)*0x1000+(src_x & x_mask)];

			if(pix != -1)
				if (y1>=0 && y1<224)
					if (x1>=0 && x1<384)
						bitmap.pix16(y1, x1) = machine().pens[pix & 3];
		}
	}
}

void vboy_state::draw_affine_map(bitmap_ind16 &bitmap, UINT16 *vboy_paramtab, int gx, int gp, int gy, int h, int w,
                                                       UINT16 x_mask, UINT16 y_mask, UINT8 ovr, bool right)
{
	int x,y;

    for(y=0;y<=h;y++)
    {
		float h_skw = (INT16)vboy_paramtab[y*8+0] / 8.0;
		float prlx = (INT16)vboy_paramtab[y*8+1] / 8.0;
		float v_skw = (INT16)vboy_paramtab[y*8+2] / 8.0;
		float h_scl = (INT16)vboy_paramtab[y*8+3] / 512.0;
		float v_scl = (INT16)vboy_paramtab[y*8+4] / 512.0;

		h_skw += right ? -prlx : prlx;

		for(x=0;x<=w;x++)
		{
			INT32 src_x,src_y;
			INT16 y1 = (y+gy);
			INT16 x1 = (x+gx);
			int pix = 0;

			x1 += right ? -gp : gp;

			src_x = (INT32)((h_skw) + (h_scl * x));
			src_y = (INT32)((v_skw) + (v_scl * x));

			if(ovr && (src_y > y_mask || src_x > x_mask || src_x < 0 || src_y < 0))
				pix = m_ovr_map[(src_y & 7)*8+(src_x & 7)];
			else
				pix = m_bg_map[(src_y & y_mask)*0x1000+(src_x & x_mask)];

            if(pix != -1)
				if (y1>=0 && y1<224)
					if (x1>=0 && x1<384)
						bitmap.pix16(y1, x1) = machine().pens[pix & 3];
		}
	}
}

/*
x--- ---- ---- ---- [0] LON
-x-- ---- ---- ----     RON
--xx ---- ---- ----     BGM type
---- xx-- ---- ----     SCX
---- --xx ---- ----     SCY
---- ---- x--- ----     OVR
---- ---- -x-- ----     END
---- ---- --00 ----
---- ---- ---- xxxx     BGMAP_BASE
*/

UINT8 vboy_state::display_world(int num, bitmap_ind16 &bitmap, bool right, int &cur_spt)
{
	num <<= 4;
	UINT16 def = m_world[num];
	UINT8 lon = (def >> 15) & 1;
	UINT8 ron = (def >> 14) & 1;
	UINT8 mode = (def >> 12) & 3;
	UINT16 scx = 64 << ((def >> 10) & 3);
	UINT16 scy = 64 << ((def >> 8) & 3);
	UINT8 ovr = (def >> 7) & 1;
	UINT8 end = (def >> 6) & 1;
	INT16 gx  = m_world[num+1];
	INT16 gp  = m_world[num+2];
	INT16 gy  = m_world[num+3];
	INT16 mx  = m_world[num+4];
	INT16 mp  = m_world[num+5];
	INT16 my  = m_world[num+6];
	UINT16 w  = m_world[num+7];
	UINT16 h  = m_world[num+8];
	UINT16 param_base = m_world[num+9] & 0xfff0;
	UINT16 ovr_char = m_bgmap[m_world[num+10]];
	UINT8 bg_map_num = def & 0x0f;
	UINT16 *vboy_paramtab;
	int i;

	vboy_paramtab = m_bgmap + param_base;

	if(end)
		return 1;

	if (mode < 2) // Normal / HBias Mode
	{
		if(ovr)
			fill_ovr_char(ovr_char & 0x7ff,BIT(ovr_char,13), BIT(ovr_char,12), m_vip_regs.GPLT[(ovr_char >> 14) & 3]);

		if (lon && (!right))
		{
			fill_bg_map(bg_map_num, scx, scy);
			draw_bg_map(bitmap, vboy_paramtab, mode, gx, gp, gy, mx, mp, my, h,w, scx*8-1, scy*8-1, ovr, right);
		}

		if (ron && (right))
		{
			fill_bg_map(bg_map_num, scx, scy);
			draw_bg_map(bitmap, vboy_paramtab, mode, gx, gp, gy, mx, mp, my, h,w, scx*8-1, scy*8-1, ovr, right);
		}
	}
	else if (mode==2) // Affine Mode
	{
		if(ovr)
			fill_ovr_char(ovr_char & 0x7ff,BIT(ovr_char,13), BIT(ovr_char,12), m_vip_regs.GPLT[(ovr_char >> 14) & 3]);

		if (lon && (!right))
		{
			fill_bg_map(bg_map_num, scx, scy);
			draw_affine_map(bitmap, vboy_paramtab, gx, gp, gy, h,w, scx*8-1, scy*8-1, ovr, right);
		}

		if (ron && (right))
		{
			fill_bg_map(bg_map_num, scx, scy);
			draw_affine_map(bitmap, vboy_paramtab, gx, gp, gy, h,w, scx*8-1, scy*8-1, ovr, right);
		}
	}
	else if (mode==3) // OBJ Mode
	{
		int start_offs, end_offs;

		if(cur_spt == -1)
		{
			popmessage("Cur spt used with -1 pointer!");
			return 0;
		}

		start_offs = m_vip_regs.SPT[cur_spt];

		end_offs = 0x3ff;
		if(cur_spt != 0)
			end_offs = m_vip_regs.SPT[cur_spt-1];

		i = start_offs;
		do
		{
			UINT16 start_ndx = i * 4;
			INT16 jx = m_objects[start_ndx+0];
			INT16 jp = m_objects[start_ndx+1] & 0x3fff;
			INT16 jy = m_objects[start_ndx+2] & 0x1ff;
			UINT16 val = m_objects[start_ndx+3];
			UINT8 jlon = (m_objects[start_ndx+1] & 0x8000) >> 15;
			UINT8 jron = (m_objects[start_ndx+1] & 0x4000) >> 14;

			if (!right && jlon)
				put_obj(bitmap, (jx-jp) & 0x1ff, jy, val & 0x7ff, BIT(val,13), BIT(val,12), m_vip_regs.JPLT[(val>>14) & 3]);

			if(right && jron)
				put_obj(bitmap, (jx+jp) & 0x1ff, jy, val & 0x7ff, BIT(val,13), BIT(val,12), m_vip_regs.JPLT[(val>>14) & 3]);

			i --;
			i &= 0x3ff;
		}while(i != end_offs);

		if((lon && !right) || (ron && right))
			cur_spt --;
	}

	return 0;
}

static SCREEN_UPDATE_IND16( vboy_left )
{
	vboy_state *state = screen.machine().driver_data<vboy_state>();
	bitmap.fill(screen.machine().pens[state->m_vip_regs.BKCOL], cliprect);
	int cur_spt;

	if(!(state->m_vip_regs.DPCTRL & 2)) /* Don't bother if screen is off */
		return 0;

	cur_spt = 3;
	for(int i=31; i>=0; i--)
		if (state->display_world(i, bitmap, false, cur_spt)) break;

	if(0)
	{
		int x,y;

		for(y=0;y<224;y++)
		{
			for(x=0;x<384;x++)
			{
				UINT8 pen;
				UINT8 pix;
				int yi;

				pen = state->m_l_frame_1[(x*0x40)+(y >> 2)];
				yi = ((y & 0x3)*2);
				pix = (pen >> yi) & 3;

				bitmap.pix16(y, x) = screen.machine().pens[pix & 3];
			}
		}
	}

	return 0;
}

static SCREEN_UPDATE_IND16( vboy_right )
{
	vboy_state *state = screen.machine().driver_data<vboy_state>();
	bitmap.fill(screen.machine().pens[state->m_vip_regs.BKCOL], cliprect);
	int cur_spt;

	if(!(state->m_vip_regs.DPCTRL & 2)) /* Don't bother if screen is off */
		return 0;

	cur_spt = 3;
	for(int i=31; i>=0; i--)
		if (state->display_world(i, bitmap, true, cur_spt)) break;

	return 0;
}

/**********************************
 *
 * I/O
 *
 *********************************/

READ32_MEMBER( vboy_state::io_r )
{
	UINT32 value = 0x00;

	switch ((offset << 2))
	{
		case 0x10:	// KLB (Keypad Low Byte)
			value = m_vboy_regs.klb;	// 0x02 is always 1
			break;
		case 0x14:	// KHB (Keypad High Byte)
			value = m_vboy_regs.khb;
			break;
		case 0x18:	// TLB (Timer Low Byte)
			value = m_vboy_regs.tlb;
			break;
		case 0x1c:	// THB (Timer High Byte)
			value = m_vboy_regs.thb;
			break;
		case 0x20:	// TCR (Timer Control Reg)
			value = m_vboy_regs.tcr;
			break;
		case 0x24:	// WCR (Wait State Control Reg)
			value = m_vboy_regs.wcr;
			break;
		case 0x28:	// KCR (Keypad Control Reg)
			{
//              attotime new_time = machine().time();

//              if((new_time - m_input_latch_time) < m_maincpu->cycles_to_attotime(640))
					value |= machine().rand() & 2;

				value = m_vboy_regs.kcr | 0x4c;
			}
			break;
		case 0x00:	// LPC (Link Port Control Reg)
		case 0x04:	// LPC2 (Link Port Control Reg)
		case 0x08:	// LPT (Link Port Transmit)
		case 0x0c:	// LPR (Link Port Receive)
		default:
			logerror("Unemulated read: offset %08x\n", 0x02000000 + (offset << 2));
			break;
	}
	return value;
}

WRITE32_MEMBER( vboy_state::io_w )
{
	switch (offset<<2)
	{
		case 0x0c:	// LPR (Link Port Receive)
		case 0x10:	// KLB (Keypad Low Byte)
		case 0x14:	// KHB (Keypad High Byte)
			//logerror("Ilegal write: offset %02x should be only read\n", offset);
			break;
		case 0x18:	// TLB (Timer Low Byte)
			m_vboy_regs.tlb = data;
			m_vboy_timer.latch = m_vboy_regs.tlb | (m_vboy_timer.latch & 0xff00);
			break;
		case 0x1c:	// THB (Timer High Byte)
			m_vboy_regs.thb = data;
			m_vboy_timer.latch = (m_vboy_regs.thb<<8) | (m_vboy_timer.latch & 0xff);
			break;
		case 0x20:	// TCR (Timer Control Reg)
			/*
                111- ---- always 1
                ---x ---- timer select (1=20 us, 0=100 us)
                ---- x--- timer irq
                ---- -x-- resets timer zero flag
                ---- --x- timer is zero flag
                ---- ---x enables timer
            */
			if(data & 1)
			{
				m_vboy_regs.tlb = m_vboy_timer.latch & 0xff;
				m_vboy_regs.thb = m_vboy_timer.latch >> 8;
				m_vboy_timer.count = m_vboy_timer.latch;
			}

			m_vboy_regs.tcr = (data & 0xfd) | (0xe4) | (m_vboy_regs.tcr & 2);	// according to docs: bits 5, 6 & 7 are unused and set to 1, bit 1 is read only.
			if(data & 4)
				m_vboy_regs.tcr &= 0xfd;
			break;
		case 0x24:	// WCR (Wait State Control Reg)
			m_vboy_regs.wcr = data | 0xfc;	// according to docs: bits 2 to 7 are unused and set to 1.
			break;
		case 0x28:	// KCR (Keypad Control Reg)
			if (data & 0x04 )
			{
				m_vboy_regs.klb = (ioport("INPUT")->read() & 0x00ff);
				m_vboy_regs.khb = (ioport("INPUT")->read() & 0xff00) >> 8;
				//m_input_latch_time = machine().time();
			}

			if (data & 1)
			{
				m_vboy_regs.klb = 0;
				m_vboy_regs.khb = 0;
				//m_input_latch_time = attotime::zero;
			}


			m_vboy_regs.kcr = (data | 0x48) & 0xfd;	// according to docs: bit 6 & bit 3 are unused and set to 1, bit 1 is read only.
			break;
		case 0x00:	// LPC (Link Port Control Reg)
		case 0x04:	// LPC2 (Link Port Control Reg)
		case 0x08:	// LPT (Link Port Transmit)
		default:
			logerror("Unemulated write: offset %08x, data %04x\n", 0x02000000 + (offset << 2), data);
			break;
	}
}


/**********************************
 *
 * VIP
 *
 *********************************/
/*
TODO: brightness presumably isn't a linear algorythm, also REST needs to be taken into account (needs a working example)
*/
void vboy_state::m_set_brightness(void)
{
	int a,b,c;

	//d = (m_vip_regs.BRTA + m_vip_regs.BRTB + m_vip_regs.BRTC + m_vip_regs.REST);
	a = (0xff * (m_vip_regs.BRTA)) / 0x80;
	b = (0xff * (m_vip_regs.BRTA + m_vip_regs.BRTB)) / 0x80;
	c = (0xff * (m_vip_regs.BRTA + m_vip_regs.BRTB + m_vip_regs.BRTC)) / 0x80;

	if(a < 0) { a = 0; }
	if(b < 0) { b = 0; }
	if(c < 0) { c = 0; }
	if(a > 0xff) { a = 0xff; }
	if(b > 0xff) { b = 0xff; }
	if(c > 0xff) { c = 0xff; }

//	popmessage("%02x %02x %02x %02x",m_vip_regs.BRTA,m_vip_regs.BRTB,m_vip_regs.BRTC,m_vip_regs.REST);
	palette_set_color_rgb(machine(), 1, a,0,0);
	palette_set_color_rgb(machine(), 2, b,0,0);
	palette_set_color_rgb(machine(), 3, c,0,0);
}

READ16_MEMBER( vboy_state::vip_r )
{
	switch(offset << 1) {
		case 0x00:	//INTPND
					return m_vip_regs.INTPND;
		case 0x02:	//INTENB
					return m_vip_regs.INTENB;
		case 0x04:	//INTCLR
					logerror("Error reading INTCLR\n");
					break;
/*
		---- -x-- ---- ---- LOCK (status column table address (CTA) lock)
		---- --x- ---- ---- SYNCE (status of sync signal enable)
		---- ---x ---- ---- RE (status of memory refresh cycle)
		---- ---- x--- ---- FCLK
        ---- ---- -x-- ---- SCANRDY (active low)
		---- ---- --xx xx-- DPBSY (current framebuffer displayed)
		---- ---- --10 00-- RFB1
		---- ---- --01 00-- LFB1
		---- ---- --00 10-- RFB0
		---- ---- --00 01-- LFB0
		---- ---- ---- --x- DISP
*/
		case 0x20:	//DPSTTS
		{
			UINT16 res;

			res = m_vip_regs.DPCTRL & 0x0702;

			if(m_vip_regs.DPCTRL & 2)
			{
				if(m_row_num < 224/8)
				{
					if(m_displayfb == 0)
						res |= 0x0c;
					else
						res |= 0x30;
				}
			}

			res |= 0x40;
			//printf("%04x\n",res);
			return res;
		}
		case 0x22:	//DPCTRL
					return m_vip_regs.DPCTRL;
		case 0x24:	//BRTA
					return m_vip_regs.BRTA;
		case 0x26:	//BRTB
					return m_vip_regs.BRTB;
		case 0x28:	//BRTC
					return m_vip_regs.BRTC;
		case 0x2A:	//REST
					return m_vip_regs.REST;
		case 0x2E:	//FRMCYC
					return m_vip_regs.FRMCYC;
		case 0x30:	//CTA
					printf("Read CTA\n");
					return m_vip_regs.CTA;
		case 0x40:	//XPSTTS, piXel Processor STaTuS
		{
			/*
			x--- ---- ---- ---- SBOUT
			---x xxxx ---- ---- SBCOUNT
			---- ---- ---x ---- OVERTIME (process overflow)
            ---- ---- ---- x--- XPBSY1 (second framebuffer busy flag)
            ---- ---- ---- -x-- XPBSY0 (first framebfuffer busy flag)
            ---- ---- ---- --x- XPEN (starts drawing at beginning of game frame)
            ---- ---- ---- ---x XPRST (force drawing process to idle)
            */
			UINT16 res;

			//printf("%d\n",row_num);

			res =  m_vip_regs.XPSTTS & 0x00f3; // empty ^^'
			res |= m_drawfb << 2;

			if(m_row_num < 224/8)
			{
				res |= 0x8000;
				res |= m_row_num<<8;
			}

			return res;
		}
		case 0x42:	//XPCTRL
					return m_vip_regs.XPCTRL;
		case 0x44:	//VER
					printf("%08x read VER\n",cpu_get_pc(m_maincpu));
					return m_vip_regs.VER;
		case 0x48:	//SPT0
					return m_vip_regs.SPT[0];
		case 0x4A:	//SPT1
					return m_vip_regs.SPT[1];
		case 0x4C:	//SPT2
					return m_vip_regs.SPT[2];
		case 0x4E:	//SPT3
					return m_vip_regs.SPT[3];
		case 0x60:	//GPLT0
					return m_vip_regs.GPLT[0];
		case 0x62:	//GPLT1
					return m_vip_regs.GPLT[1];
		case 0x64:	//GPLT2
					return m_vip_regs.GPLT[2];
		case 0x66:	//GPLT3
					return m_vip_regs.GPLT[3];
		case 0x68:	//JPLT0
					return m_vip_regs.JPLT[0];
		case 0x6A:	//JPLT1
					return m_vip_regs.JPLT[1];
		case 0x6C:	//JPLT2
					return m_vip_regs.JPLT[2];
		case 0x6E:	//JPLT3
					return m_vip_regs.JPLT[3];
		case 0x70:	//BKCOL
					return m_vip_regs.BKCOL;
		default:
					logerror("Unemulated read: addr %08x\n", offset * 2 + 0x0005f800);
					break;
	}
	return 0xffff;
}

WRITE16_MEMBER( vboy_state::vip_w )
{
	if(mem_mask != 0xffff)
		printf("%04x %02x\n",mem_mask,offset*2);

	switch(offset << 1) {
		/*
            x--- ---- ---- ---- TIME_ERR
            -x-- ---- ---- ---- XP_END
            --x- ---- ---- ---- SB_HIT
            ---- ---- ---x ---- FRAME_START
            ---- ---- ---- x--- GAME_START
            ---- ---- ---- -x-- RFB_END
            ---- ---- ---- --x- LFB_END
            ---- ---- ---- ---x SCAN_ERR
        */
		case 0x00:	//INTPND
					logerror("Error writing INTPND\n");
					break;
		case 0x02:	//INTENB
					m_vip_regs.INTENB = data;
					m_set_irq(0);
					//printf("%04x ENB\n",data);
					break;
		case 0x04:	//INTCLR
					m_vip_regs.INTPND &= ~data;
					m_set_irq(0);
					//else
					//  printf("%04x\n",m_vip_regs.INTPND);
					break;
		case 0x20:	//DPSTTS
					logerror("Error writing DPSTTS\n");
					break;
/*
		---- -x-- ---- ---- LOCK (status column table address (CTA) lock)
		---- --x- ---- ---- SYNCE (status of sync signal enable)
		---- ---x ---- ---- RE (status of memory refresh cycle)
		---- ---- ---- --x- DISP
		---- ---- ---- ---x DPRST (Resets the VIP internal counter)
*/
		case 0x22:	//DPCTRL
					m_vip_regs.DPCTRL = data & 0x0702;

					if(data & 1)
					{
						m_vip_regs.INTPND &= 0xe000; // reset FRAME_START, GAME_START, RFB_END, LFB_END and SCAN_ERR irqs
						m_set_irq(0);
					}
					break;
		case 0x24:	//BRTA
					m_vip_regs.BRTA = data;
					m_set_brightness();
					break;
		case 0x26:	//BRTB
					m_vip_regs.BRTB = data;
					m_set_brightness();
					break;
		case 0x28:	//BRTC
					m_vip_regs.BRTC = data;
					m_set_brightness();
					break;
		case 0x2A:	//REST
					m_vip_regs.REST = data;
					m_set_brightness();
					if(data)
						printf("%04x REST\n",data);
					break;
		case 0x2E:	//FRMCYC
					//printf("%d\n",data);
					m_vip_regs.FRMCYC = data;
					break;
		case 0x30:	//CTA
					m_vip_regs.CTA = data;
					printf("%04x CTA\n",data);
					break;
		case 0x40:	//XPSTTS
					logerror("Error writing XPSTTS\n");
					break;
		case 0x42:	//XPCTRL, w/o
					/*
                    ---- ---- ---- --x-
                    ---- ---- ---- ---x Reset Pixel Processor
                    */
					m_vip_regs.XPCTRL = data & 0x1f02;

					//if(data & 0x1f00)
					//  printf("%04x SBCMP\n",data);

					if(data & 1)
					{
						m_vip_regs.INTPND &= 0x1fff; // reset SB_HIT, XP_END and TIME_ERR irqs
						m_set_irq(0);
					}
					break;
		case 0x44:	//VER
					//m_vip_regs.VER = data;
					break;
		case 0x48:	//SPT0
					m_vip_regs.SPT[0] = data & 0x3ff;
					break;
		case 0x4A:	//SPT1
					m_vip_regs.SPT[1] = data & 0x3ff;
					break;
		case 0x4C:	//SPT2
					m_vip_regs.SPT[2] = data & 0x3ff;
					break;
		case 0x4E:	//SPT3
					m_vip_regs.SPT[3] = data & 0x3ff;
					break;
		case 0x60:	//GPLT0
					m_vip_regs.GPLT[0] = data;
					break;
		case 0x62:	//GPLT1
					m_vip_regs.GPLT[1] = data;
					break;
		case 0x64:	//GPLT2
					m_vip_regs.GPLT[2] = data;
					break;
		case 0x66:	//GPLT3
					m_vip_regs.GPLT[3] = data;
					break;
		case 0x68:	//JPLT0
					m_vip_regs.JPLT[0] = data & 0xfc;
					break;
		case 0x6A:	//JPLT1
					m_vip_regs.JPLT[1] = data & 0xfc;
					break;
		case 0x6C:	//JPLT2
					m_vip_regs.JPLT[2] = data & 0xfc;
					break;
		case 0x6E:	//JPLT3
					m_vip_regs.JPLT[3] = data & 0xfc;
					break;
		case 0x70:	//BKCOL
					m_vip_regs.BKCOL = data & 3;
					break;
		default:
					logerror("Unemulated write: addr %08x, data %04x\n", offset * 2 + 0x0005f800, data);
					break;
	}
}


void vboy_state::m_pcg_debug(UINT16 offset,UINT16 data,UINT16 mem_mask)
{
	UINT8 *pcg_ram = memregion("pcg")->base();
	int i;

	if(mem_mask & 0x00ff)
		pcg_ram[(offset<<1)+1] = (data & 0x00ff);

	if(mem_mask & 0xff00)
		pcg_ram[(offset<<1)+0] = (data & 0xff00) >> 8;

	gfx_element_mark_dirty(machine().gfx[0], offset >> 4);
	for(i=0;i<0x800;i++)
		gfx_element_mark_dirty(machine().gfx[0], i);

}

WRITE16_MEMBER( vboy_state::vboy_font0_w )
{
	m_font[offset] = data | (m_font[offset] & (mem_mask ^ 0xffff));

	if(1)
		m_pcg_debug(offset,data,mem_mask);
}

WRITE16_MEMBER( vboy_state::vboy_font1_w )
{
	m_font[offset + 0x1000] = data | (m_font[offset + 0x1000] & (mem_mask ^ 0xffff));

	if(1)
		m_pcg_debug(offset+0x1000,data,mem_mask);
}

WRITE16_MEMBER( vboy_state::vboy_font2_w )
{
	m_font[offset + 0x2000] = data | (m_font[offset + 0x2000] & (mem_mask ^ 0xffff));

	if(1)
		m_pcg_debug(offset+0x2000,data,mem_mask);
}

WRITE16_MEMBER( vboy_state::vboy_font3_w )
{
	m_font[offset + 0x3000] = data | (m_font[offset + 0x3000] & (mem_mask ^ 0xffff));

	if(1)
		m_pcg_debug(offset+0x3000,data,mem_mask);
}

READ16_MEMBER( vboy_state::vboy_font0_r )
{
	return m_font[offset];
}

READ16_MEMBER( vboy_state::vboy_font1_r )
{
	return m_font[offset + 0x1000];
}

READ16_MEMBER( vboy_state::vboy_font2_r )
{
	return m_font[offset + 0x2000];
}

READ16_MEMBER( vboy_state::vboy_font3_r )
{
	return m_font[offset + 0x3000];
}

WRITE16_MEMBER( vboy_state::vboy_bgmap_w )
{
	m_bgmap[offset] = data | (m_bgmap[offset] & (mem_mask ^ 0xffff));
}

READ16_MEMBER( vboy_state::vboy_bgmap_r )
{
	return m_bgmap[offset];
}

READ8_MEMBER( vboy_state::vboy_lfb0_r ) { return m_l_frame_0[offset]; }
READ8_MEMBER( vboy_state::vboy_lfb1_r ) { return m_l_frame_1[offset]; }
READ8_MEMBER( vboy_state::vboy_rfb0_r ) { return m_r_frame_0[offset]; }
READ8_MEMBER( vboy_state::vboy_rfb1_r ) { return m_r_frame_1[offset]; }
WRITE8_MEMBER( vboy_state::vboy_lfb0_w ) { m_l_frame_0[offset] = data; }
WRITE8_MEMBER( vboy_state::vboy_lfb1_w ) { m_l_frame_1[offset] = data; }
WRITE8_MEMBER( vboy_state::vboy_rfb0_w ) { m_r_frame_0[offset] = data; }
WRITE8_MEMBER( vboy_state::vboy_rfb1_w ) { m_r_frame_1[offset] = data; }


static ADDRESS_MAP_START( vboy_mem, AS_PROGRAM, 32, vboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0x07ffffff)
	AM_RANGE( 0x00000000, 0x00005fff ) AM_READWRITE8(vboy_lfb0_r,vboy_lfb0_w,0xffffffff) // L frame buffer 0
	AM_RANGE( 0x00006000, 0x00007fff ) AM_READWRITE16(vboy_font0_r, vboy_font0_w, 0xffffffff) // Font 0-511
	AM_RANGE( 0x00008000, 0x0000dfff ) AM_READWRITE8(vboy_lfb1_r,vboy_lfb1_w,0xffffffff) // L frame buffer 1
	AM_RANGE( 0x0000e000, 0x0000ffff ) AM_READWRITE16(vboy_font1_r, vboy_font1_w, 0xffffffff) // Font 512-1023
	AM_RANGE( 0x00010000, 0x00015fff ) AM_READWRITE8(vboy_rfb0_r,vboy_rfb0_w,0xffffffff)  // R frame buffer 0
	AM_RANGE( 0x00016000, 0x00017fff ) AM_READWRITE16(vboy_font2_r, vboy_font2_w, 0xffffffff) // Font 1024-1535
	AM_RANGE( 0x00018000, 0x0001dfff ) AM_READWRITE8(vboy_rfb1_r,vboy_rfb1_w,0xffffffff)  // R frame buffer 1
	AM_RANGE( 0x0001e000, 0x0001ffff ) AM_READWRITE16(vboy_font3_r, vboy_font3_w, 0xffffffff) // Font 1536-2047

	AM_RANGE( 0x00020000, 0x0003ffff ) AM_READWRITE16(vboy_bgmap_r,vboy_bgmap_w, 0xffffffff) // VIPC memory

	//AM_RANGE( 0x00040000, 0x0005ffff ) AM_RAM // VIPC
	AM_RANGE( 0x0005f800, 0x0005f87f ) AM_READWRITE16(vip_r, vip_w, 0xffffffff)

	AM_RANGE( 0x00078000, 0x00079fff ) AM_READWRITE16(vboy_font0_r, vboy_font0_w, 0xffffffff) // Font 0-511 mirror
	AM_RANGE( 0x0007a000, 0x0007bfff ) AM_READWRITE16(vboy_font1_r, vboy_font1_w, 0xffffffff) // Font 512-1023 mirror
	AM_RANGE( 0x0007c000, 0x0007dfff ) AM_READWRITE16(vboy_font2_r, vboy_font2_w, 0xffffffff) // Font 1024-1535 mirror
	AM_RANGE( 0x0007e000, 0x0007ffff ) AM_READWRITE16(vboy_font3_r, vboy_font3_w, 0xffffffff) // Font 1536-2047 mirror

	AM_RANGE( 0x01000000, 0x010005ff ) AM_DEVREADWRITE8("vbsnd", vboysnd_device, read, write, 0xffffffff)
	AM_RANGE( 0x02000000, 0x0200002b ) AM_MIRROR(0x0ffff00) AM_READWRITE(io_r, io_w) // Hardware control registers mask 0xff
	//AM_RANGE( 0x04000000, 0x04ffffff ) // Expansion area
	AM_RANGE( 0x05000000, 0x0500ffff ) AM_MIRROR(0x0ff0000) AM_RAM AM_SHARE("wram")// Main RAM - 64K mask 0xffff
	AM_RANGE( 0x06000000, 0x06003fff ) AM_RAM AM_SHARE("nvram") // Cart RAM - 8K NVRAM
	AM_RANGE( 0x07000000, 0x071fffff ) AM_MIRROR(0x0e00000) AM_ROM AM_REGION("user1", 0) /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( vboy_io, AS_IO, 32, vboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0x07ffffff)
	AM_RANGE( 0x00000000, 0x00005fff ) AM_RAM AM_SHARE("l_frame_0") // L frame buffer 0
	AM_RANGE( 0x00006000, 0x00007fff ) AM_READWRITE16(vboy_font0_r, vboy_font0_w, 0xffffffff) // Font 0-511
	AM_RANGE( 0x00008000, 0x0000dfff ) AM_RAM AM_SHARE("l_frame_1") // L frame buffer 1
	AM_RANGE( 0x0000e000, 0x0000ffff ) AM_READWRITE16(vboy_font1_r, vboy_font1_w, 0xffffffff) // Font 512-1023
	AM_RANGE( 0x00010000, 0x00015fff ) AM_RAM AM_SHARE("r_frame_0") // R frame buffer 0
	AM_RANGE( 0x00016000, 0x00017fff ) AM_READWRITE16(vboy_font2_r, vboy_font2_w, 0xffffffff) // Font 1024-1535
	AM_RANGE( 0x00018000, 0x0001dfff ) AM_RAM AM_SHARE("r_frame_1") // R frame buffer 1
	AM_RANGE( 0x0001e000, 0x0001ffff ) AM_READWRITE16(vboy_font3_r, vboy_font3_w, 0xffffffff) // Font 1536-2047

	AM_RANGE( 0x00020000, 0x0003ffff ) AM_READWRITE16(vboy_bgmap_r,vboy_bgmap_w, 0xffffffff) // VIPC memory

	//AM_RANGE( 0x00040000, 0x0005ffff ) AM_RAM // VIPC
	AM_RANGE( 0x0005f800, 0x0005f87f ) AM_READWRITE16(vip_r, vip_w, 0xffffffff)

	AM_RANGE( 0x00078000, 0x00079fff ) AM_READWRITE16(vboy_font0_r, vboy_font0_w, 0xffffffff) // Font 0-511 mirror
	AM_RANGE( 0x0007a000, 0x0007bfff ) AM_READWRITE16(vboy_font1_r, vboy_font1_w, 0xffffffff) // Font 512-1023 mirror
	AM_RANGE( 0x0007c000, 0x0007dfff ) AM_READWRITE16(vboy_font2_r, vboy_font2_w, 0xffffffff) // Font 1024-1535 mirror
	AM_RANGE( 0x0007e000, 0x0007ffff ) AM_READWRITE16(vboy_font3_r, vboy_font3_w, 0xffffffff) // Font 1536-2047 mirror

	AM_RANGE( 0x01000000, 0x010005ff ) AM_DEVREADWRITE8("vbsnd", vboysnd_device, read, write, 0xffffffff)
	AM_RANGE( 0x02000000, 0x0200002b ) AM_MIRROR(0x0ffff00) AM_READWRITE(io_r, io_w) // Hardware control registers mask 0xff
//  AM_RANGE( 0x04000000, 0x04ffffff ) // Expansion area
	AM_RANGE( 0x05000000, 0x0500ffff ) AM_MIRROR(0x0ff0000) AM_RAM AM_SHARE("wram") // Main RAM - 64K mask 0xffff
	AM_RANGE( 0x06000000, 0x06003fff ) AM_RAM AM_SHARE("nvram") // Cart RAM - 8K NVRAM
	AM_RANGE( 0x07000000, 0x071fffff ) AM_MIRROR(0x0e00000) AM_ROM AM_REGION("user1", 0) /* ROM */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vboy )
	PORT_START("INPUT")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("L") PORT_PLAYER(1) // Left button on back
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("R") PORT_PLAYER(1) // Right button on back
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(1) // B button (Mario Clash Jump button)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(1) // A button
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNUSED ) // Always 1
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) // Battery low
INPUT_PORTS_END


static MACHINE_RESET(vboy)
{
	vboy_state *state = machine.driver_data<vboy_state>();

	/* Initial values taken from Reality Boy, to be verified when emulation improves */
	state->m_vboy_regs.lpc = 0x6d;
	state->m_vboy_regs.lpc2 = 0xff;
	state->m_vboy_regs.lpt = 0x00;
	state->m_vboy_regs.lpr = 0x00;
	state->m_vboy_regs.klb = 0x00;
	state->m_vboy_regs.khb = 0x00;
	state->m_vboy_regs.tlb = 0xff;
	state->m_vboy_regs.thb = 0xff;
	state->m_vboy_regs.tcr = 0xe4;
	state->m_vboy_regs.wcr = 0xfc;
	state->m_vboy_regs.kcr = 0x4c;
	state->m_vip_regs.DPCTRL = 2; // ssquash relies on this at boot otherwise no frame_start irq is fired
	state->m_displayfb = 0;
	state->m_drawfb = 0;
}


void vboy_state::m_timer_tick(UINT8 setting)
{
	if(m_vboy_regs.tcr & 1 && ((m_vboy_regs.tcr & 0x10) == setting))
	{
		if(m_vboy_timer.count != 0)
		{
			m_vboy_timer.count--;
			m_vboy_regs.tlb = m_vboy_timer.count & 0xff;
			m_vboy_regs.thb = m_vboy_timer.count >> 8;
		}
		else
		{
			m_vboy_timer.count = m_vboy_timer.latch;
			//printf("Zero timer trigger\n");
			m_vboy_regs.tcr |= 0x02;
			if(m_vboy_regs.tcr & 8)
				device_set_input_line(m_maincpu, 1, HOLD_LINE);
		}
	}
}

static TIMER_DEVICE_CALLBACK( timer_100us_tick )
{
	vboy_state *state = timer.machine().driver_data<vboy_state>();

	state->m_timer_tick(0x00);
}

static TIMER_DEVICE_CALLBACK( timer_20us_tick )
{
	vboy_state *state = timer.machine().driver_data<vboy_state>();

	state->m_timer_tick(0x10);
}

static PALETTE_INIT( vboy )
{
	palette_set_color(machine, 0, RGB_BLACK);
	palette_set_color(machine, 1, RGB_BLACK);
	palette_set_color(machine, 2, RGB_BLACK);
	palette_set_color(machine, 3, RGB_BLACK);
}

void vboy_state::m_set_irq(UINT16 irq_vector)
{
	m_vip_regs.INTPND |= irq_vector;

	if(m_vip_regs.INTENB & m_vip_regs.INTPND)
		device_set_input_line(m_maincpu, 4, ASSERT_LINE);

	if((m_vip_regs.INTENB & m_vip_regs.INTPND) == 0)
		device_set_input_line(m_maincpu, 4, CLEAR_LINE);
}

/* TODO: obviously all of this needs clean-ups and better implementation ... */
void vboy_state::m_scanline_tick(int scanline, UINT8 screen_type)
{
	//int frame_num = machine().primary_screen->frame_number();

	if(screen_type == 0)
		m_row_num = (scanline / 8) & 0x1f;

	if(scanline == 0)
	{
		if(m_vip_regs.DPCTRL & 2)
			m_set_irq(0x0010); // FRAME_START

		m_frame_count++;

		if(m_frame_count > m_vip_regs.FRMCYC)
		{
			m_set_irq(0x0008); // GAME_START
			m_frame_count = 0;
		}

		if(m_vip_regs.DPCTRL & 2)
			m_displayfb ^= 1;
	}

	if(scanline == 224)
	{
		if(m_displayfb)
			m_drawfb = 1;
		else
			m_drawfb = 2;
		m_set_irq(0x4000); // XPEND
	}

	if(scanline == 232)
	{
		m_drawfb = 0;
		m_set_irq(0x0002); // LFBEND
	}

	if(scanline == 240)
	{
		m_set_irq(0x0004); // RFBEND
	}

	if(m_row_num == ((m_vip_regs.XPCTRL & 0x1f00) >> 8))
	{
		m_set_irq(0x2000); // SBHIT
	}

}

static TIMER_DEVICE_CALLBACK( vboy_scanlineL )
{
	vboy_state *state = timer.machine().driver_data<vboy_state>();
	int scanline = param;

	state->m_scanline_tick(scanline,0);
}

#if 0
static TIMER_DEVICE_CALLBACK( vboy_scanlineR )
{
	vboy_state *state = timer.machine().driver_data<vboy_state>();
	int scanline = param;

	//state->m_scanline_tick(scanline,1);
}
#endif


static const gfx_layout vboy_pcg_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{ 7*2, 6*2, 5*2, 4*2, 3*2, 2*2, 1*2, 0*2 },
	{ STEP8(0,8*2) },
	8*8*2
};

/* decoded for debugging purpose, this will be nuked in the end... */
static GFXDECODE_START( vboy )
	GFXDECODE_ENTRY( "pcg",     0x00000, vboy_pcg_8x8,      0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( vboy, vboy_state )

	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", V810, XTAL_20MHz )
	MCFG_CPU_PROGRAM_MAP(vboy_mem)
	MCFG_CPU_IO_MAP(vboy_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer_l", vboy_scanlineL, "3dleft", 0, 1)
	//MCFG_TIMER_ADD_SCANLINE("scantimer_r", vboy_scanlineR, "3dright", 0, 1)

	MCFG_MACHINE_RESET(vboy)

	MCFG_TIMER_ADD_PERIODIC("timer_100us", timer_100us_tick, attotime::from_usec(1000))
	MCFG_TIMER_ADD_PERIODIC("timer_20us", timer_20us_tick, attotime::from_msec(20))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_vboy)
	MCFG_VIDEO_START(vboy)
	MCFG_PALETTE_LENGTH(4)
	MCFG_PALETTE_INIT(vboy)

	/* Left screen */
	MCFG_SCREEN_ADD("3dleft", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20MHz/2,757,0,384,264,0,224)
	MCFG_SCREEN_UPDATE_STATIC(vboy_left)

	/* Right screen */
	MCFG_SCREEN_ADD("3dright", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20MHz/2,757,0,384,264,0,224)
	MCFG_SCREEN_UPDATE_STATIC(vboy_right)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("vb,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("vboy_cart")

	MCFG_GFXDECODE(vboy)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","vboy")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_VBOYSND_ADD("vbsnd")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vboy )
	ROM_REGION( 0x200000, "user1", 0 )
	ROM_CART_LOAD("cart", 0x0000, 0x200000, ROM_MIRROR)

	ROM_REGION( 0x8000, "pcg", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY     FULLNAME       FLAGS */
CONS( 1995, vboy,   0,      0,       vboy,      vboy, driver_device,    0,    "Nintendo", "Virtual Boy", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND)
