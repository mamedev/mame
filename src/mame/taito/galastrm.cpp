// license:BSD-3-Clause
// copyright-holders: Hau

/*

Galactic Storm
(c)1992 Taito

----------------------------------------------------------
MAIN PCB
CPU:MC68EC020RP25
TC0480SCP
TC0100SCN
TC0510NIO
TC0580PIV x2
TC0110PCR
TC0470LIN x2
TC0570SPC
TC0610
ADC0809CCN

OSC1:32MHz
OSC2:20MHz
----------------------------------------------------------
SOUND BOARD
CPU:MC68000P12F,MC68681P
ENSONIQ 5701,5510,5505

OSC1:16MHz
OSC2:30.47618MHz
----------------------------------------------------------
based on driver from taito/gunbustr.cpp by Bryan McPhail & David Graves
Written by Hau
07/03/2008


tips
$300.b debugmode
$305.b invincibility

TODO:
- device-ify TC0610? (no other known users)
- full screen rotation is incorrect in taito logo, end of stage, etc... (see https://youtu.be/lzPnO8Kej20)

*/


#include "emu.h"

#include "taito_en.h"
#include "taitoio.h"
#include "tc0100scn.h"
#include "tc0110pcr.h"
#include "tc0480scp.h"

#include "cpu/m68000/m68020.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_BADSPRITES     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_BADSPRITES)

#include "logmacro.h"

#define LOGBADSPRITES(...)     LOGMASKED(LOG_BADSPRITES,     __VA_ARGS__)


namespace {

class galastrm_renderer;


class galastrm_state : public driver_device
{
	friend class galastrm_renderer;

public:
	galastrm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0480scp(*this, "tc0480scp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_spriteram(*this,"spriteram"),
		m_spritemap_rom(*this, "sprmaprom")
	{ }

	void galastrm(machine_config &config);
	int frame_counter_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_shared_ptr<u32> m_spriteram;
	required_region_ptr<u16> m_spritemap_rom;

	struct gs_tempsprite
	{
		u8 gfx = 0U;
		u32 code = 0U;
		u32 color = 0U;
		bool flipx = false;
		bool flipy = false;
		int x = 0;
		int y = 0;
		int zoomx = 0;
		int zoomy = 0;
		u32 primask = 0U;
	};

	u16 m_frame_counter = 0U;
	u16 m_tc0610_addr[2]{};
	s16 m_tc0610_ctrl_reg[2][8]{};
	std::unique_ptr<gs_tempsprite[]> m_spritelist;
	struct gs_tempsprite *m_sprite_ptr_pre;
	bitmap_ind16 m_tmpbitmaps;
	std::unique_ptr<galastrm_renderer> m_poly;

	s16 m_rsxb = 0;
	s16 m_rsyb = 0;
	s32 m_rsxoffs = 0;
	s32 m_rsyoffs = 0;

	static constexpr u8 X_OFFSET = 96;
	static constexpr u8 Y_OFFSET = 60;

	template<int Chip> void tc0610_w(offs_t offset, u16 data);
	void coin_word_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites_pre(int x_offs, int y_offs);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int priority);

	void main_map(address_map &map) ATTR_COLD;
};


struct gs_poly_data
{
	bitmap_ind16* texbase = nullptr;
};

class galastrm_renderer : public poly_manager<float, gs_poly_data, 2>
{
public:
	galastrm_renderer(galastrm_state &state);

	void tc0610_draw_scanline(s32 scanline, const extent_t& extent, const gs_poly_data& object, int threadid);
	void tc0610_rotate_draw(bitmap_ind16 &srcbitmap, const rectangle &clip);

	bitmap_ind16 &screenbits() { return m_screenbits; }

private:
	galastrm_state& m_state;
	bitmap_ind16 m_screenbits;
};

galastrm_renderer::galastrm_renderer(galastrm_state& state)
	: poly_manager<float, gs_poly_data, 2>(state.machine())
	, m_state(state)
	, m_screenbits(state.m_screen->width(), state.m_screen->height())
{
}


/******************************************************************/

void galastrm_state::video_start()
{
	m_spritelist = std::make_unique<gs_tempsprite[]>(0x4000);

	m_poly = std::make_unique<galastrm_renderer>(*this);

	m_screen->register_screen_bitmap(m_tmpbitmaps);
	m_screen->register_screen_bitmap(m_poly->screenbits());

	save_item(NAME(m_rsxb));
	save_item(NAME(m_rsyb));
	save_item(NAME(m_rsxoffs));
	save_item(NAME(m_rsyoffs));
	save_item(NAME(m_frame_counter));
	save_item(NAME(m_tc0610_addr));
	save_item(NAME(m_tc0610_ctrl_reg));
}


/************************************************************
            SPRITE DRAW ROUTINES

We draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap ROM provides the lookup
table for this. The game hardware looks up 16x16 sprite chunks
from the spritemap ROM, creating a 64x64 sprite like this:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15

(where the number is the word offset into the spritemap ROM).
It can also create 32x32 sprites.

NB: unused portions of the spritemap ROM contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

Heavy use is made of sprite zooming.

        ***

    Sprite table layout (4 long words per entry)

    ------------------------------------------
     0 | ........ x....... ........ ........ | Flip X
     0 | ........ .xxxxxxx ........ ........ | ZoomX
     0 | ........ ........ .xxxxxxx xxxxxxxx | Sprite Tile
       |                                     |
     2 | ........ ....xx.. ........ ........ | Sprite/tile priority [*]
     2 | ........ ......xx xxxxxx.. ........ | Palette bank
     2 | ........ ........ ......xx xxxxxxxx | X position
       |                                     |
     3 | ........ .....x.. ........ ........ | Sprite size (0=32x32, 1=64x64)
     3 | ........ ......x. ........ ........ | Flip Y
     3 | ........ .......x xxxxxx.. ........ | ZoomY
     3 | ........ ........ ......xx xxxxxxxx | Y position
    ------------------------------------------

    [* 00=over BG1; 01=BG2; 10=BG3; 11=over text ???]

********************************************************/

void galastrm_state::draw_sprites_pre(int x_offs, int y_offs)
{
	int sprites_flipscreen = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	m_sprite_ptr_pre = m_spritelist.get();

	for (int offs = (m_spriteram.bytes() / 4 - 4); offs >= 0; offs -= 4)
	{
		u32 data = m_spriteram[offs + 0];
		int flipx =          (data & 0x00800000) >> 23;
		int zoomx =          (data & 0x007f0000) >> 16;
		const u32 tilenum =  (data & 0x00007fff);

		if (!tilenum) continue;

		data = m_spriteram[offs + 2];
		const int priority = (data & 0x000c0000) >> 18;
		int color =          (data & 0x0003fc00) >> 10;
		int x =              (data & 0x000003ff);

		data = m_spriteram[offs + 3];
		const int dblsize =  (data & 0x00040000) >> 18;
		int flipy =          (data & 0x00020000) >> 17;
		int zoomy =          (data & 0x0001fc00) >> 10;
		int y =              (data & 0x000003ff);

		int bad_chunks = 0;
		const int dimension = ((dblsize * 2) + 2);  // 2 or 4
		const int total_chunks = ((dblsize * 3) + 1) << 2;  // 4 or 16
		const u32 map_offset = tilenum << 2;

		zoomx += 1;
		zoomy += 1;

		if (x > 713) x -= 1024;     // 1024 x 512
		if (y < 117) y += 512;

		y = (-y & 0x3ff);
		x -= x_offs;
		y += y_offs;
		if (flipy) y += (128 - zoomy);

		for (int sprite_chunk = 0; sprite_chunk < total_chunks; sprite_chunk++)
		{
			const int j = sprite_chunk / dimension;   // rows
			const int k = sprite_chunk % dimension;   // chunks per row

			int px = k;
			int py = j;
			// pick tiles back to front for x and y flips
			if (flipx)  px = dimension - 1 - k;
			if (flipy)  py = dimension - 1 - j;

			const u16 code = m_spritemap_rom[map_offset + px + (py << (dblsize + 1))];

			if (code == 0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			int curx = x + ((k * zoomx) / dimension);
			int cury = y + ((j * zoomy) / dimension);

			const int zx = x + (((k + 1) * zoomx) / dimension) - curx;
			const int zy = y + (((j + 1) * zoomy) / dimension) - cury;

			if (sprites_flipscreen)
			{
				/* -zx/y is there to fix zoomed sprite coords in screenflip.
				    drawgfxzoom does not know to draw from flip-side of sprites when
				    screen is flipped; so we must correct the coords ourselves. */

				curx = 320 - curx - zx;
				cury = 256 - cury - zy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_sprite_ptr_pre->gfx = 0;
			m_sprite_ptr_pre->code = code;
			m_sprite_ptr_pre->color = color;
			m_sprite_ptr_pre->flipx = !flipx;
			m_sprite_ptr_pre->flipy = flipy;
			m_sprite_ptr_pre->x = curx;
			m_sprite_ptr_pre->y = cury;
			m_sprite_ptr_pre->zoomx = zx << 12;
			m_sprite_ptr_pre->zoomy = zy << 12;
			m_sprite_ptr_pre->primask = priority;

			m_sprite_ptr_pre++;
		}
		if (bad_chunks)
			LOGBADSPRITES("Sprite number %04x had %02x invalid chunks\n", tilenum, bad_chunks);
	}
}

void galastrm_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int priority)
{
	struct gs_tempsprite *sprite_ptr = m_sprite_ptr_pre;

	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		if ((priority != 0 && sprite_ptr->primask != 0) ||
			(priority == 0 && sprite_ptr->primask == 0))
		{
			m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap, cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx, sprite_ptr->flipy,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->zoomx, sprite_ptr->zoomy,
				screen.priority(), primasks[sprite_ptr->primask], 0);
		}
	}
}


/**************************************************************
                POLYGON RENDERER
**************************************************************/

void galastrm_renderer::tc0610_draw_scanline(s32 scanline, const extent_t& extent, const gs_poly_data& object, int threadid)
{
	u16 *const framebuffer = &m_screenbits.pix(scanline);
	const s32 dudx = extent.param[0].dpdx;
	const s32 dvdx = extent.param[1].dpdx;

	s32 u = extent.param[0].start;
	s32 v = extent.param[1].start;
	for (int x = extent.startx; x < extent.stopx; x++)
	{
		framebuffer[x] = object.texbase->pix(v >> 16, u >> 16);
		u += dudx;
		v += dvdx;
	}
}

void galastrm_renderer::tc0610_rotate_draw(bitmap_ind16 &srcbitmap, const rectangle &clip)
{
	struct polyVert
	{
		float x;
		float y;
//      float z;
	} tmpz[4];

	vertex_t vert[4];
	int rsx = m_state.m_tc0610_ctrl_reg[1][0];
	int rsy = m_state.m_tc0610_ctrl_reg[1][1];
	const int rzx = m_state.m_tc0610_ctrl_reg[1][2];
	const int rzy = m_state.m_tc0610_ctrl_reg[1][3];
	const int ryx = m_state.m_tc0610_ctrl_reg[1][5];
	const int ryy = m_state.m_tc0610_ctrl_reg[1][4];
	const int lx  = srcbitmap.width();
	const int ly  = srcbitmap.height();

	int pxx = 0;
	int pxy = 0;
	int pyx = 0;
	int pyy = 0;
	int zx  = 0;
	int zy  = 0;

	if (rzx != 0 || rzy != 0)
	{
		while (sqrtf(powf((float)pxx / 4096.0f, 2.0f) + powf((float)pxy / 4096.0f, 2.0f)) < (float)(lx / 2))
		{
			pxx += rzx;
			pxy += rzy;
			zx++;
		}
		while (sqrtf(powf((float)pyy / 4096.0f, 2.0f) + powf((float)pyx / 4096.0f, 2.0f)) < (float)(ly / 2))
		{
			pyy += rzx;
			pyx += -rzy;
			zy++;
		}
	}
	float zsn = ((float)pyx / 4096.0f) / (float)(ly / 2);
	float zcs = ((float)pxx / 4096.0f) / (float)(lx / 2);


	if ((rsx == -240 && rsy == 1072) || !m_state.m_tc0610_ctrl_reg[1][7])
	{
		m_state.m_rsxoffs = 0;
		m_state.m_rsyoffs = 0;
	}
	else
	{
		if (rsx > m_state.m_rsxb && m_state.m_rsxb < 0 && rsx-m_state.m_rsxb > 0x8000)
		{
			if (m_state.m_rsxoffs == 0)
				m_state.m_rsxoffs = -0x10000;
			else
				m_state.m_rsxoffs = 0;
		}
		if (rsx < m_state.m_rsxb && m_state.m_rsxb > 0 && m_state.m_rsxb-rsx > 0x8000)
		{
			if (m_state.m_rsxoffs == 0)
				m_state.m_rsxoffs = 0x10000 - 1;
			else
				m_state.m_rsxoffs = 0;
		}
		if (rsy > m_state.m_rsyb && m_state.m_rsyb < 0 && rsy-m_state.m_rsyb > 0x8000)
		{
			if (m_state.m_rsyoffs == 0)
				m_state.m_rsyoffs = -0x10000;
			else
				m_state.m_rsyoffs = 0;
		}
		if (rsy < m_state.m_rsyb && m_state.m_rsyb > 0 && m_state.m_rsyb-rsy > 0x8000)
		{
			if (m_state.m_rsyoffs == 0)
				m_state.m_rsyoffs = 0x10000 - 1;
			else
				m_state.m_rsyoffs = 0;
		}
	}
	m_state.m_rsxb = rsx;
	m_state.m_rsyb = rsy;
	if (m_state.m_rsxoffs) rsx += m_state.m_rsxoffs;
	if (m_state.m_rsyoffs) rsy += m_state.m_rsyoffs;
	if (rsx < -0x14000 || rsx >= 0x14000) m_state.m_rsxoffs = 0;
	if (rsy < -0x14000 || rsy >= 0x14000) m_state.m_rsyoffs = 0;


	pxx = 0;
	pxy = 0;
	pyx = 0;
	pyy = 0;
	int yx  = 0;
	//int yy  = 0;
	//float ssn = 0.0;
	//float scs = 0.0;
	//float ysn = 0.0;
	//float ycs = 0.0;

	if (m_state.m_tc0610_ctrl_reg[1][7])
	{
		if (ryx != 0 || ryy != 0)
		{
			while (sqrtf(powf((float)pxx / 4096.0f, 2.0f) + powf((float)pxy / 4096.0f, 2.0f)) < (float)(lx / 2))
			{
				pxx += ryx;
				pxy += ryy;
				yx++;
			}
			while (sqrtf(powf((float)pyy / 4096.0f, 2.0f) + powf((float)pyx / 4096.0f, 2.0f)) < (float)(ly / 2))
			{
				pyy += ryx;
				pyx += -ryy;
				//yy++;
			}
			if (yx >= 0.0)
			{
				yx = (int)((8.0 - log((double)yx) / log(2.0)) * 6.0);
				//ysn = sin(DEGREE_TO_RADIAN(yx));
				//ycs = 1.0 - ysn*ysn;
			}
		}

		pxx = 0;
		pxy = 0;
		pyx = 0;
		pyy = 0;

		if (rsx != 0 || rsy != 0)
		{
			while (sqrtf(powf((float)pxx / 65536.0f, 2.0) + powf((float)pxy / 65536.0f, 2.0f)) < (float)(lx / 2))
			{
				pxx += rsx;
				pxy += rsy;
			}
			while (sqrtf(powf((float)pyy / 65536.0f, 2.0f) + powf((float)pyx / 65536.0f, 2.0f)) < (float)(ly / 2))
			{
				pyy += rsx;
				pyx += -rsy;
			}
		}
		//ssn = ((float)pxy/65536.0) / (float)(lx / 2);
		//scs = ((float)pyy/65536.0) / (float)(ly / 2);
	}

	{
//      polyVert tmpz[4];
		tmpz[0].x = ((float)(-zx)    * zcs) - ((float)(-zy)    * zsn);
		tmpz[0].y = ((float)(-zx)    * zsn) + ((float)(-zy)    * zcs);
//      tmpz[0].z = 0.0;
		tmpz[1].x = ((float)(-zx)    * zcs) - ((float)(zy - 1) * zsn);
		tmpz[1].y = ((float)(-zx)    * zsn) + ((float)(zy - 1) * zcs);
//      tmpz[1].z = 0.0;
		tmpz[2].x = ((float)(zx - 1) * zcs) - ((float)(zy - 1) * zsn);
		tmpz[2].y = ((float)(zx - 1) * zsn) + ((float)(zy - 1) * zcs);
//      tmpz[2].z = 0.0;
		tmpz[3].x = ((float)(zx - 1) * zcs) - ((float)(-zy)    * zsn);
		tmpz[3].y = ((float)(zx - 1) * zsn) + ((float)(-zy)    * zcs);
//      tmpz[3].z = 0.0;

		vert[0].x = tmpz[0].x + (float)(lx / 2);
		vert[0].y = tmpz[0].y + (float)(ly / 2);
		vert[1].x = tmpz[1].x + (float)(lx / 2);
		vert[1].y = tmpz[1].y + (float)(ly / 2);
		vert[2].x = tmpz[2].x + (float)(lx / 2);
		vert[2].y = tmpz[2].y + (float)(ly / 2);
		vert[3].x = tmpz[3].x + (float)(lx / 2);
		vert[3].y = tmpz[3].y + (float)(ly / 2);
	}

	vert[0].p[0] = 0.0;
	vert[0].p[1] = 0.0;
	vert[1].p[0] = 0.0;
	vert[1].p[1] = (float)(ly - 1) * 65536.0f;
	vert[2].p[0] = (float)(lx - 1) * 65536.0f;
	vert[2].p[1] = (float)(ly - 1) * 65536.0f;
	vert[3].p[0] = (float)(lx - 1) * 65536.0f;
	vert[3].p[1] = 0.0;

	gs_poly_data& extra = object_data().next();
	extra.texbase = &srcbitmap;

	render_polygon<4, 2>(clip, render_delegate(&galastrm_renderer::tc0610_draw_scanline, this), vert);
	wait();
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

u32 galastrm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 pivlayer[3];
	static const u32 primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};
	bitmap_ind8 &priority_bitmap = screen.priority();

	rectangle clip(0, screen.width() -1, 0, screen.height()  - 1);

	m_tc0100scn->tilemap_update();
	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();
	layer[0] = (priority & 0xf000) >> 12;   // tells us which bg layer is bottom
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   // tells us which is top
	layer[4] = 4;   // text layer always over bg layers

	pivlayer[0] = m_tc0100scn->bottomlayer();
	pivlayer[1] = pivlayer[0] ^ 1;
	pivlayer[2] = 2;

	bitmap.fill(0, cliprect);
	priority_bitmap.fill(0, clip);
	m_tmpbitmaps.fill(0, clip);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[0], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[1], 0, 0);

#if 0
	if (layer[0] == 0 && layer[1] == 3 && layer[2] == 2 && layer[3] == 1)
	{
		if (!machine().input().code_pressed(KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		if (!machine().input().code_pressed(KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 4);
	}
	else
	{
		if (!machine().input().code_pressed(KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		if (!machine().input().code_pressed(KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 2);
		if (!machine().input().code_pressed(KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 8);
	}

	if (layer[0] == 3 && layer[1] == 0 && layer[2] == 1 && layer[3] == 2)
	{
		for (int y = 0; y < priority_bitmap.height; y++)
		{
			for (int x = 0; x < priority_bitmap.width; x++)
			{
				u8 *pri = &priority_bitmap.pix(y, x);
				if (!(*pri & 0x02) && m_tmpbitmaps.pix(y, x))
					*pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(machine(), 42 - X_OFFSET, -571 + Y_OFFSET);
	draw_sprites(screen, m_tmpbitmaps, clip, primasks, 1);

	copybitmap_trans(bitmap, m_polybitmap, 0, 0, 0, 0, cliprect, 0);
	m_polybitmap->fill(0, clip);
	tc0610_rotate_draw(machine(), m_polybitmap, m_tmpbitmaps, cliprect);

	priority_bitmap.fill(0, cliprect);
	draw_sprites(screen, bitmap, cliprect, primasks, 0);

	if (!machine().input().code_pressed(KEYCODE_B)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);
	if (!machine().input().code_pressed(KEYCODE_M)) m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[2], 0, 0);

#else
	if (layer[0] == 0 && layer[1] == 3 && layer[2] == 2 && layer[3] == 1)
	{
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 4);
	}
	else
	{
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 8);
	}

	if (layer[0] == 3 && layer[1] == 0 && layer[2] == 1 && layer[3] == 2)
	{
		for (int y=0; y < priority_bitmap.height(); y++)
		{
			for (int x=0; x < priority_bitmap.width(); x++)
			{
				u8 *pri = &priority_bitmap.pix(y, x);
				if (!(*pri & 0x02) && m_tmpbitmaps.pix(y, x))
					*pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(42 - X_OFFSET, -571 + Y_OFFSET);
	draw_sprites(screen, m_tmpbitmaps, clip, primasks, 1);

	copybitmap_trans(bitmap, m_poly->screenbits(), 0, 0, 0, 0, cliprect, 0);
	m_poly->screenbits().fill(0, clip);
	m_poly->tc0610_rotate_draw(m_tmpbitmaps, cliprect);

	priority_bitmap.fill(0, cliprect);
	draw_sprites(screen, bitmap, cliprect, primasks, 0);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[2], 0, 0);
#endif

	return 0;
}


/*********************************************************************/

INTERRUPT_GEN_MEMBER(galastrm_state::interrupt)
{
	m_frame_counter ^= 1;
	device.execute().set_input_line(5, HOLD_LINE);
}

template<int Chip>
void galastrm_state::tc0610_w(offs_t offset, u16 data)
{
	if (offset == 0)
		m_tc0610_addr[Chip] = data;
	else if (m_tc0610_addr[Chip] < 8)
		m_tc0610_ctrl_reg[Chip][m_tc0610_addr[Chip]] = data;
}


int galastrm_state::frame_counter_r()
{
	return m_frame_counter;
}

void galastrm_state::coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void galastrm_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x21ffff).ram(); // main RAM
	map(0x300000, 0x303fff).ram().share(m_spriteram);
	map(0x400000, 0x400007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x40fff0, 0x40fff3).nopw();
	map(0x500000, 0x500007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask32(0xffffffff);
	map(0x600000, 0x6007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)); // Sound shared RAM
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));        // tilemaps
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x900003).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));
	map(0xb00000, 0xb00003).w(FUNC(galastrm_state::tc0610_w<0>));
	map(0xc00000, 0xc00003).w(FUNC(galastrm_state::tc0610_w<1>));
	map(0xd00000, 0xd0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));        // piv tilemaps
	map(0xd20000, 0xd2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}


/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( galastrm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(galastrm_state::frame_counter_r))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // Freeze input
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("IN2")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,1),
	4,  // 4 bits per pixel
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	64*16   // every sprite takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_galastrm )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout, 0, 4096 / 16 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void galastrm_state::galastrm(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 32_MHz_XTAL / 2); // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &galastrm_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(galastrm_state::interrupt)); // VBL

	EEPROM_93C46_16BIT(config, "eeprom");

	adc0809_device &adc(ADC0809(config, "adc", 500'000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 6);
	adc.in_callback<0>().set_ioport("STICKX");
	adc.in_callback<1>().set_ioport("STICKY");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_2_callback().set_ioport("IN0");
	tc0510nio.read_3_callback().set_ioport("IN1");
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(galastrm_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("IN2");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 50*8);
	m_screen->set_visarea(0+96, 40*8-1+96, 3*8+60, 32*8-1+60);
	m_screen->set_screen_update(FUNC(galastrm_state::screen_update));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_galastrm);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(-48, -56);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_tc0110pcr);
	m_tc0480scp->set_offsets(-40, -3);

	TC0110PCR(config, m_tc0110pcr, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "speaker", 1.0, 0);
	taito_en.add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************/

ROM_START( galastrm )
	ROM_REGION( 0x100000, "maincpu", 0 )    // for 68020 code (CPU A)
	ROM_LOAD32_BYTE( "c99_15.ic105", 0x00000, 0x40000,  CRC(7eae8efd) SHA1(6bbb3da697dfcd93337b53895678e2a4ff2de457) )
	ROM_LOAD32_BYTE( "c99_12.ic102", 0x00001, 0x40000,  CRC(e059d1ee) SHA1(560951f95f270f0559b5289dda7f4ba74538cfcb) )
	ROM_LOAD32_BYTE( "c99_13.ic103", 0x00002, 0x40000,  CRC(885fcb35) SHA1(be10e109c461c1f776e98efa1b2a4d588aa0c41c) )
	ROM_LOAD32_BYTE( "c99_14.ic104", 0x00003, 0x40000,  CRC(457ef6b1) SHA1(06c2613d46addacd380a0f2413cd795b17ac9474) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "c99_23.ic8",  0x100000, 0x20000,  CRC(5718ee92) SHA1(33cfa60c5bceb1525498f27b598067d2dc620431) )
	ROM_LOAD16_BYTE( "c99_22.ic7",  0x100001, 0x20000,  CRC(b90f7c42) SHA1(e2fa9ee10ad61ae1a672c3357c0072b79ec7fbcb) )

	ROM_REGION( 0x100, "tc0100scn", ROMREGION_ERASE00 ) // no ROMs for tc0100scn, dummy

	ROM_REGION( 0x200000, "tc0480scp", 0 ) // SCR 16x16 tiles
	ROM_LOAD32_WORD( "c99-05.ic1",  0x000000, 0x100000, CRC(a91ffba4) SHA1(467af9646ddad5fbb520b6bc13517ed4deacf479) )
	ROM_LOAD32_WORD( "c99-06.ic2",  0x000002, 0x100000, CRC(812ed3ae) SHA1(775904dd42643d0e3a30890590d5f8eac1fe78db) )

	ROM_REGION( 0x400000, "sprites", 0 ) // OBJ 16x16 tiles
	ROM_LOAD64_WORD_SWAP( "c99-02.ic50", 0x000000, 0x100000, CRC(81e9fc6f) SHA1(4495a7d130b755b5a48eaa814d884d6bb8243bcb) )
	ROM_LOAD64_WORD_SWAP( "c99-01.ic51", 0x000002, 0x100000, CRC(9dda1267) SHA1(c639ba064496dcadf5f1e55332a12bb442e9dc86) )
	ROM_LOAD64_WORD_SWAP( "c99-04.ic66", 0x000004, 0x100000, CRC(a681760f) SHA1(23d4fc7eb778c8a25c4bc7cee1d0c8cdd828a996) )
	ROM_LOAD64_WORD_SWAP( "c99-03.ic67", 0x000006, 0x100000, CRC(a2807a27) SHA1(977e395ea2ab2fb82807d3cf5fe5f1dbbde99da0) )

	ROM_REGION16_LE( 0x80000, "sprmaprom", 0 ) // STY
	ROM_LOAD16_WORD( "c99-11.ic90",  0x00000,  0x80000, CRC(26a6926c) SHA1(918860e2829131e9ecfe983b2ae3e49e1c9ecd72) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c99-08.ic3",  0x000000, 0x100000, CRC(fedb4187) SHA1(83563e4af795a0dfeb261a62c31b6fed72f45a4d) )
	ROM_LOAD16_BYTE( "c99-09.ic4",  0x200000, 0x100000, CRC(ba70b86b) SHA1(ffbb9547d6b6e47a3ef23206b5f40c57f3ea7619) )
	ROM_LOAD16_BYTE( "c99-10.ic5",  0x400000, 0x100000, CRC(da016f1e) SHA1(581ef158c6f6576618dd75429b1d3aa92cd3581d) )
	ROM_LOAD16_BYTE( "c99-07.ic2",  0x680000, 0x040000, CRC(4cc3136f) SHA1(d9d7556bbe6af161fa0651b1fbd72e7dbf0a8e82) )
	ROM_CONTINUE( 0x600000, 0x040000 )
	ROM_CONTINUE( 0x780000, 0x040000 )
	ROM_CONTINUE( 0x700000, 0x040000 )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-galastrm.bin", 0x0000, 0x0080, CRC(94efa7a6) SHA1(5870b988cb364065e8bd779efbdadca8d3ffc17c) )

	ROM_REGION( 0x1200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "c99-16.bin", 0x0000, 0x0104, CRC(9340e376) SHA1(3795063e44a1da5947e8695532b6d6277af5e873) )
	ROM_LOAD( "c99-17.bin", 0x0200, 0x0144, CRC(81d55be5) SHA1(dc7302eced7c5a519aa882a1e11cf44809c2fc50) )
	ROM_LOAD( "c99-18.bin", 0x0400, 0x0149, CRC(eca1501d) SHA1(d62823a77d1a76921a07889d8ded593b03cc3eca) )
	ROM_LOAD( "c99-19.bin", 0x0600, 0x0104, CRC(6310ef1d) SHA1(cb61b0a5fe9aca42a06090c0332b8e013f1c4d8f) )
	ROM_LOAD( "c99-20.bin", 0x0800, 0x0144, CRC(5d527b8b) SHA1(7e7d8a5c37d602b4e802e4d18edafb31f6182b1a) )
	ROM_LOAD( "c99-21.bin", 0x0a00, 0x0104, CRC(eb2407a1) SHA1(bfe2a06ccadac3205ae6d9cd85d434ab12088ce9) )
	ROM_LOAD( "c99-24.bin", 0x0c00, 0x0144, CRC(a0ec9b49) SHA1(2f283a271a4f47d28a9421c7dadf272a6b4d167e) )
	ROM_LOAD( "c99-25.bin", 0x0e00, 0x0144, CRC(d7cbb8be) SHA1(daeb1cb3b5a5c0445be8b18f9e80f048e1818fda) )
	ROM_LOAD( "c99-26.bin", 0x1000, 0x0144, CRC(d65cbcb9) SHA1(e4579d15d9fbc300b736948dbc322c1c6aa4aa2a) )
ROM_END

} // anonymous namespace


GAME( 1992, galastrm, 0, galastrm, galastrm, galastrm_state, empty_init, ROT0, "Taito Corporation", "Galactic Storm (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
