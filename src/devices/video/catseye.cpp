// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "catseye.h"

#define LOG_VRAM  (LOG_GENERAL << 1)
#define LOG_REG   (LOG_GENERAL << 2)
#define LOG_WMOVE (LOG_GENERAL << 3)

//#define VERBOSE (LOG_WMOVE|LOG_REG|LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CATSEYE, catseye_device, "catseye", "HP Catseye ASIC")

catseye_device::catseye_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t{mconfig, type, tag, owner, clock},
	m_vram{*this, { "^vram_video", "^vram_overlay"}},
	m_int_write_func{*this},
	m_blink_timer{nullptr},
	m_changed{false},
	m_blink_state{false},
	m_plane_mask_l{0},
	m_plane_mask_h{0},
	m_write_enable{0},
	m_read_enable{0},
	m_fb_enable{0},
	m_display_enable{0},
	m_blink_enable{0},
	m_in_vblank{0},
	m_wm_int_pending{0},
	m_vblank_int_pending{0},
	m_wm_int_enable{0},
	m_vblank_int_enable{0},
	m_rugsc{0},
	m_misc{0},
	m_wmx{0},
	m_wmwidth{0},
	m_wmheight{0},
	m_wmsourcex{0},
	m_wmsourcey{0},
	m_wmdestx{0},
	m_wmdesty{0},
	m_wmclipleft{0},
	m_wmclipright{0},
	m_wmcliptop{0},
	m_wmclipbottom{0},
	m_patterns{{{0}}},
	m_linepath{0},
	m_linetype{0},
	m_prr{0},
	m_wrr{0},
	m_trr{0},
	m_trrctl{0},
	m_color{0},
	m_vb{0},
	m_acntrl{0},
	m_planemode{0},
	m_status{0}
{
}

catseye_device::catseye_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	catseye_device(mconfig, CATSEYE, tag, owner, clock)
{
}


void catseye_device::device_start()
{
	m_int_write_func.resolve_safe();

	m_blink_timer = timer_alloc(FUNC(catseye_device::blink_callback), this);
	m_blink_timer->adjust(attotime::from_hz(3));

	save_item(NAME(m_changed));
	save_item(NAME(m_blink_state));
	save_item(NAME(m_write_enable), 2);
	save_item(NAME(m_read_enable), 2);
	save_item(NAME(m_fb_enable), 2);
	save_item(NAME(m_display_enable));
	save_item(NAME(m_blink_enable));
	save_item(NAME(m_sync_enable));
	save_item(NAME(m_in_vblank));
	save_item(NAME(m_wm_int_pending));
	save_item(NAME(m_wm_int_enable));
	save_item(NAME(m_vblank_int_enable));
	save_item(NAME(m_vblank_int_pending));
	save_item(NAME(m_rugsc));
	save_item(NAME(m_misc));
	save_item(NAME(m_wmx));
	save_item(NAME(m_wmwidth));
	save_item(NAME(m_wmheight));
	save_item(NAME(m_wmsourcex));
	save_item(NAME(m_wmsourcey));
	save_item(NAME(m_wmdestx));
	save_item(NAME(m_wmdesty));
	save_item(NAME(m_wmclipleft));
	save_item(NAME(m_wmclipright));
	save_item(NAME(m_wmcliptop));
	save_item(NAME(m_wmclipbottom));
	save_item(NAME(m_patterns));
	save_item(NAME(m_linepath));
	save_item(NAME(m_linetype));
	save_item(NAME(m_prr), 2);
	save_item(NAME(m_wrr), 2);
	save_item(NAME(m_trr), 2);
	save_item(NAME(m_trrctl), 2);
	save_item(NAME(m_color), 2);
	save_item(NAME(m_vb), 2);
	save_item(NAME(m_acntrl));
	save_item(NAME(m_planemode), 2);
	save_item(NAME(m_status));
}

void catseye_device::device_reset()
{
	LOG("%s\n", __func__);

	for (int i = 0; i < 2; i++) {
		m_read_enable[i] = 0;
		m_write_enable[i] = 0;
		m_fb_enable[i] = 0;

		m_prr[i] = 0;
		m_wrr[i] = 0;
		m_trr[i] = 0;
		m_trrctl[i] = 0;
		m_color[i] = 0;
		m_vb[i] = 0;
		m_planemode[i] = 0;
		std::fill(m_patterns[i].begin(), m_patterns[i].end(), 0);
	}
	m_acntrl = 0;
	m_blink_enable = 0;
	m_display_enable = m_plane_mask_h;
	m_rugsc = 0;
	m_wmwidth = 0;
	m_wmheight = 0;
	m_wmsourcex = 0;
	m_wmsourcey = 0;
	m_wmdestx = 0;
	m_wmdesty = 0;
	m_wmclipleft = 0;
	m_wmclipright = 0;
	m_wmcliptop = 0;
	m_wmclipbottom = 0;

	m_linepath = 0;
	m_linetype = 0;
}

void catseye_device::update_int()
{
	LOG("%s: pending %d, enabled %d\n", __func__, m_wm_int_pending, m_wm_int_enable);
	if ((m_wm_int_enable && m_wm_int_pending ) ||
		(m_vblank_int_enable && m_vblank_int_pending))
		m_int_write_func(m_plane);
	else
		m_int_write_func(0);
}

TIMER_CALLBACK_MEMBER(catseye_device::blink_callback)
{
	m_blink_timer->adjust(attotime::from_hz(5));
	m_blink_state ^= true;
	m_changed = true;
}

void catseye_device::execute_rule(const bool src, const int rule, bool &dst) const
{
	switch (rule & 0x0f) {
	case CATSEYE_REPLACE_RULE_CLEAR:
		dst = false;
		break;

	case CATSEYE_REPLACE_RULE_SRC_AND_DST:
		dst &= src;
		break;

	case CATSEYE_REPLACE_RULE_SRC_AND_NOT_DST:
		dst = !dst & src;
		break;

	case CATSEYE_REPLACE_RULE_SRC:
		dst = src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC_AND_DST:
		dst &= !src;
		break;

	case CATSEYE_REPLACE_RULE_NOP:
		break;

	case CATSEYE_REPLACE_RULE_SRC_XOR_DST:
		dst ^= src;
		break;

	case CATSEYE_REPLACE_RULE_SRC_OR_DST:
		dst |= src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC_AND_NOT_DST:
		dst = !dst & !src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC_XOR_DST:
		dst ^= !src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_DST:
		dst ^= true;
		break;

	case CATSEYE_REPLACE_RULE_SRC_OR_NOT_DST:
		dst = src | !dst;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC:
		dst = !src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC_OR_DST:
		dst |= !src;
		break;

	case CATSEYE_REPLACE_RULE_NOT_SRC_OR_NOT_DST:
		dst = !src | !dst;
		break;

	case CATSEYE_REPLACE_RULE_SET:
		dst = true;
		break;

	}
}

template<int Idx>
void catseye_device::window_move()
{

	if (!m_fb_enable[Idx])
		return;

	LOGMASKED(LOG_WMOVE, "%s%d: %3ux%3u -> %3ux%3u / %3ux%3u planemode %04x wrr %04x, trr %04x m_trrctl %04x acntr %04x\n",
			__func__, Idx, m_wmsourcex, m_wmsourcey, m_wmdestx, m_wmdesty, m_wmwidth, m_wmheight, m_planemode[Idx],
			m_wrr[Idx], m_trr[Idx], m_trrctl[Idx], m_acntrl);

	int line, endline, lineincr;

	if (m_wmdesty > m_wmsourcey) {
		/* move down */
		line = m_wmheight-1;
		endline = -1;
		lineincr = -1;
	} else {
		/* move up */
		line = 0;
		endline = m_wmheight;
		lineincr = 1;
	}

	int startcolumn, endcolumn, columnincr;
	if (m_wmdestx > m_wmsourcex) {
		/* move right */
		startcolumn = m_wmwidth-1;
		endcolumn = -1;
		columnincr = -1;
	} else {
		/* move left */
		startcolumn = 0;
		endcolumn = m_wmwidth;
		columnincr = 1;
	}

	for ( ; line != endline; line += lineincr) {
		for (int column = startcolumn; column != endcolumn; column += columnincr) {
			const int sx = m_wmsourcex + column;
			const int sy = m_wmsourcey + line;
			const int dx = m_wmdestx + column;
			const int dy = m_wmdesty + line;

			if ((m_misc & CATSEYE_MISC_ENABLE_CLIP) &&
					(dy < m_wmcliptop ||
					(dy > m_wmclipbottom) ||
					(dx < m_wmclipleft) ||
					(dx > m_wmclipright))) {
				continue;
			}

			bool dst = false;

			if (m_trrctl[Idx] & 0x0100) {
				bool bit = m_patterns[Idx][(m_plane << 4) | (dy & 0xf)] & (1 << (dx & 0xf));
				switch (m_trr[Idx] >> 8) {

				case 0xf0:
					dst = bit;
					break;

				default:
					logerror("%s: unknown trr %02x\n", __func__, m_trr[Idx] >> 8);
					break;
				}
			} else {
				bool const src = get_vram_pixel_plane<Idx>(sx, sy);
				dst = get_vram_pixel<Idx>(dx, dy);
				execute_rule(src, m_wrr[Idx] & 0x0f, dst);
			}
			modify_vram<Idx>(dx, dy, dst);
		}
	}
}


template<int Idx>
void catseye_device::draw_pixel(int x, int y, int color)
{
	if ((m_misc & CATSEYE_MISC_ENABLE_CLIP) &&
		(x < m_wmclipleft ||
		x > m_wmclipright ||
		y < m_wmcliptop ||
		y > m_wmclipbottom))
			return;

	bool src = color & m_plane_mask_l;
	bool dst = get_vram_pixel<Idx>(x, y);
	execute_rule(src, (m_prr[Idx] >> 8) & 0x0f, dst);
	modify_vram<Idx>(x, y, dst);
	m_status |= CATSEYE_STATUS_UNCLIPPED;
}

template<int Idx>
void catseye_device::draw_line()
{
	const int color = m_color[Idx] >> 8;
	int x1 = m_wmsourcex;
	int y1 = m_wmsourcey;
	int x2 = m_wmdestx;
	int y2 = m_wmdesty;
	int dx, dy, t, e, x, y, incy, diago, horiz;
	bool c1;

	LOGMASKED(LOG_WMOVE, "%s%d %dx%d -> %dx%d, color %d\n",
			__func__, Idx, m_wmsourcex, m_wmsourcey, m_wmdestx, m_wmdesty, color);

	c1 = false;
	incy = 1;

	if (x2 > x1)
		dx = x2 - x1;
	else
		dx = x1 - x2;

	if (y2 > y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	if (dy > dx) {
		t = y2;
		y2 = x2;
		x2 = t;

		t = y1;
		y1 = x1;
		x1 = t;

		t = dx;
		dx = dy;
		dy = t;

		c1 = true;
	}

	if (x1 > x2) {
		t = y2;
		y2 = y1;
		y1 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	horiz = dy << 1;
	diago = (dy - dx) << 1;
	e = (dy << 1) - dx;

	if (y1 <= y2)
		incy = 1;
	else
		incy = -1;

	x = x1;
	y = y1;

	if (c1) {
		do {
			draw_pixel<Idx>(y, x, color);

			if (e > 0) {
				y  += incy;
				e  += diago;
			} else {
				e += horiz;
			}
			x++;
		} while (x <= x2);
	} else {
		do {
			draw_pixel<Idx>(x, y, color);
			if (e > 0) {
				y += incy;
				e += diago;
			} else {
				e += horiz;
			}
			x++;
		} while (x <= x2);
	}

}

template<int Idx>
void catseye_device::trigger_wm()
{
	if (!m_fb_enable[Idx])
		return;

	if ((m_vb[Idx] & CATSEYE_VB_VECTOR) && (m_rugsc == 0x10))
		draw_line<Idx>();
	else if (m_rugsc == 0x90)
		window_move<Idx>();
	else
		logerror("%s: unsupported rugcmd: %04x vb %04x\n", __func__, m_rugsc, m_vb[Idx]);
}

template<int Idx>
u16 catseye_device::vram_r_bit(offs_t offset)
{
	u16 ret = 0;

	offset &= ~0x7;

	for (int i = 0; i < 16; i++)
		ret |= get_vram_offset_plane<Idx>(offset * 2 + 15 - i) ? (1 << i) : 0;
	return ret;
}

template<int Idx>
u16 catseye_device::vram_r_word(offs_t offset, u16 mem_mask)
{
	u16 ret = 0;

	if (mem_mask & m_plane_mask_l)
		ret |= get_vram_offset<Idx>(offset * 2 + 1);

	if (mem_mask & m_plane_mask_h)
		ret |= get_vram_offset<Idx>(offset * 2) << 8;

	return ret;
}

template<int Idx>
u16 catseye_device::vram_r(offs_t offset, u16 mem_mask)
{
	if (m_acntrl & 0x100)
		return vram_r_bit<Idx>(offset);
	else
		return vram_r_word<Idx>(offset, mem_mask);
}

uint16_t catseye_device::vram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0;

	if (m_acntrl & 0x200)
		ret = vram_r<VRAM_OVERLAY_PLANE>(offset, mem_mask);
	else
		ret = vram_r<VRAM_VIDEO_PLANE>(offset, mem_mask);

	LOGMASKED(LOG_VRAM, "%s: %04x: %04x(%04x)\n", __func__, offset << 1, ret, mem_mask);
	return ret;
}

template<int Idx>
void catseye_device::vram_w_bit(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= ~0x7;

	for (int i = 0; i < 16; i++) {

		const int bit = (1 << i);

		if (!(mem_mask & bit))
			continue;

		bool dst = get_vram_offset<Idx>(offset * 2 + 15 - i);
		execute_rule(data & bit, (m_prr[Idx] >> 8) & 0x0f, dst);
		modify_vram_offset<Idx>(offset * 2 + 15 - i, dst);
	}
}

template<int Idx>
void catseye_device::vram_w_word(offs_t offset, u16 data, u16 mem_mask)
{
	if (mem_mask & m_plane_mask_l) {
		const bool src = data & m_plane_mask_l;
		bool dst = get_vram_offset<Idx>(offset * 2 + 1);
		execute_rule(src, (m_prr[Idx] >> 8) & 0x0f, dst);
		modify_vram_offset<Idx>(offset * 2 + 1, dst);
	}

	if (mem_mask & m_plane_mask_h) {
		const bool src = data & m_plane_mask_h;
		bool dst = get_vram_offset<Idx>(offset * 2);
		execute_rule(src, (m_prr[Idx] >> 8) & 0x0f, dst);
		modify_vram_offset<Idx>(offset * 2, dst);
	}
}

template<int Idx>
void catseye_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_acntrl & 0x100)
		vram_w_bit<Idx>(offset, data, mem_mask);
	else
		vram_w_word<Idx>(offset, data, mem_mask);
}

void catseye_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_VRAM, "%s: %04x: %04x(%04x)\n", __func__, offset << 1, data, mem_mask);

	if (m_fb_enable[0])
		vram_w<VRAM_VIDEO_PLANE>(offset, data, mem_mask);

	if (m_fb_enable[1])
		vram_w<VRAM_OVERLAY_PLANE>(offset, data, mem_mask);
}

void catseye_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	bool trigger = false;

	if (m_write_enable[0] && offset >= 0x200 && offset < 0x280) {
		COMBINE_DATA(&m_patterns[0][offset - 0x200]);
		LOGMASKED(LOG_REG, "W PATTERNS1[%03d] = %04x\n", offset - 0x200, m_patterns[0][offset - 0x200]);
		return;
	}

	if (m_write_enable[1] && offset >= 0x300 && offset < 0x380) {
		COMBINE_DATA(&m_patterns[1][offset - 0x300]);
		LOGMASKED(LOG_REG, "W PATTERNS2[%03d] = %04x\n", offset - 0x300, m_patterns[1][offset - 0x300]);
		return;
	}

	switch(offset) {
	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		if (mem_mask & 0xff00)
			m_blink_enable = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "enable blink plane: %04x\n", m_blink_enable);
		break;

	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		if (mem_mask & 0xff00)
			m_display_enable = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "display enable %04x\n", m_display_enable);
		break;

	case TOPCAT_REG_SYNC_ENABLE:
		if (mem_mask & 0xff00)
			m_sync_enable = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "sync enable: %04x\n", m_sync_enable);
		break;

	case TOPCAT_REG_WMOVE_INTRQ:
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMOVE_INTRQ = %04x\n", data);
		m_wm_int_pending = 0;
		update_int();
		break;

	case TOPCAT_REG_WMOVE_IE:
		if (mem_mask & 0xff00)
			m_wm_int_enable = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMOVE_IE = %04x\n", m_wm_int_enable);
		update_int();
		break;

	case TOPCAT_REG_VBLANK_IE:
		if (mem_mask & 0xff00)
			m_vblank_int_enable = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMOVE_IE = %04x\n", m_vblank_int_enable);
		update_int();
		break;

	case CATSEYE_REG_WMX:
		COMBINE_DATA(&m_wmx);
		COMBINE_DATA(&m_wmsourcex);
		COMBINE_DATA(&m_wmdestx);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMX = %04x\n", m_wmx);
		break;

	case CATSEYE_REG_RUG_SC:
		COMBINE_DATA(&m_rugsc);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_RUG_SC = %04x\n", m_rugsc);
		break;

	case TOPCAT_REG_WMWIDTH:
	case CATSEYE_REG_WMWIDTH:
		COMBINE_DATA(&m_wmwidth);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMWIDTH = %04x\n", m_wmwidth);
		break;

	case TOPCAT_REG_WMHEIGHT:
	case CATSEYE_REG_WMHEIGHT:
		COMBINE_DATA(&m_wmheight);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMHEIGHT = %04x\n", m_wmheight);
		break;

	case CATSEYE_REG_LINEPATH:
		COMBINE_DATA(&m_linepath);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_LINEPATH = %04x\n", m_linepath);
		break;

	case CATSEYE_REG_LINETYPE:
		COMBINE_DATA(&m_linetype);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_LINETYPE = %04x\n", m_linetype);
		break;


	case TOPCAT_REG_WMSOURCEX:
	case CATSEYE_REG_WMSOURCEX:
		COMBINE_DATA(&m_wmsourcex);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMSOURCEX = %04x\n", m_wmsourcex);
		break;

	case TOPCAT_REG_WMSOURCEY:
	case CATSEYE_REG_WMSOURCEY:
		COMBINE_DATA(&m_wmsourcey);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMSOURCEY = %04x\n", m_wmsourcey);
		break;

	case TOPCAT_REG_WMDESTX:
	case CATSEYE_REG_WMDESTX:
		COMBINE_DATA(&m_wmdestx);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMDESTX = %04x\n", m_wmdestx);
		break;

	case TOPCAT_REG_WMDESTY:
	case CATSEYE_REG_WMDESTY:
		COMBINE_DATA(&m_wmdesty);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMDESTY = %04x\n", m_wmdesty);
		break;

	case CATSEYE_REG_WMCLIPLEFT:
		COMBINE_DATA(&m_wmclipleft);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMCLIPLEFT = %04x\n", m_wmclipleft);
		break;

	case CATSEYE_REG_WMCLIPRIGHT:
		COMBINE_DATA(&m_wmclipright);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMCLIPRIGHT = %04x\n", m_wmclipright);
		break;

	case CATSEYE_REG_WMCLIPTOP:
		COMBINE_DATA(&m_wmcliptop);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMCLIPTOP = %04x\n", m_wmcliptop);
		break;

	case CATSEYE_REG_WMCLIPBOTTOM:
		COMBINE_DATA(&m_wmclipbottom);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WMCLIPBOTTOM = %04x\n", m_wmclipbottom);
		break;

	case CATSEYE_REG_TWMWIDTH:
		COMBINE_DATA(&m_wmwidth);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMWIDTH = %04x\n", m_wmwidth);
		trigger = true;
		break;

	case CATSEYE_REG_TWMHEIGHT:
		COMBINE_DATA(&m_wmheight);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMHEIGHT = %04x\n", m_wmheight);
		trigger = true;
		break;

	case CATSEYE_REG_TWMSOURCEX:
		COMBINE_DATA(&m_wmsourcex);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMSOURCEX = %04x\n", m_wmsourcex);
		trigger = true;
		break;

	case CATSEYE_REG_TWMSOURCEY:
		COMBINE_DATA(&m_wmsourcey);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMSOURCEY = %04x\n", m_wmsourcey);
		trigger = true;
		break;

	case CATSEYE_REG_TWMDESTX:
		COMBINE_DATA(&m_wmdestx);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMDESTX = %04x\n", m_wmdestx);
		trigger = true;
		break;

	case CATSEYE_REG_TWMDESTY:
		COMBINE_DATA(&m_wmdesty);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TWMDESTY = %04x\n", m_wmdesty);
		trigger = true;
		break;

	case TOPCAT_REG_START_WMOVE:
		if (m_write_enable[0]) {
			LOGMASKED(LOG_REG, "W TOPCAT_REG_START_WMOVE = %04x\n", data);
			window_move<0>();
			if (m_wm_int_enable) {
				m_wm_int_pending = m_plane_mask_h;
				update_int();
			}
		}
		break;

	case CATSEYE_REG_STATUS:
		COMBINE_DATA(&m_status);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_STATUS = %04x\n", m_status);
		break;

	case CATSEYE_REG_STATUS+1:
	case CATSEYE_REG_STATUS+2:
	// unknown
			break;


	// TC 1 registers
	case TOPCAT_REG_READ_ENABLE_PLANE:
	case CATSEYE_REG_TCREN1:
		if (mem_mask & m_plane_mask_h)
			m_read_enable[0] = data & m_plane_mask_l;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_READ_ENABLE1 = %04x\n", m_read_enable[0]);
		break;

	case TOPCAT_REG_WRITE_ENABLE_PLANE:
	case CATSEYE_REG_TCWEN1:
		if (mem_mask & m_plane_mask_h)
			m_write_enable[0] = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WRITE_ENABLE1 = %04x\n", m_write_enable[0]);
		break;

	case TOPCAT_REG_FB_WRITE_ENABLE:
	case CATSEYE_REG_FBEN1:
		if (mem_mask & m_plane_mask_h)
			m_fb_enable[0] = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_FBEN1 = %04x\n", m_fb_enable[0]);
		break;

	case TOPCAT_REG_PRR:
	case CATSEYE_REG_PRR1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_prr[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_PRR1 = %04x\n", m_prr[0]);
		break;

	case TOPCAT_REG_WRR:
	case CATSEYE_REG_WRR1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_wrr[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WRR1 = %04x\n", m_wrr[0]);
		break;

	case CATSEYE_REG_TRR1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_trr[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TRR1 = %04x\n", m_trr[0]);
		break;

	case CATSEYE_REG_VB1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_vb[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_VB1 = %04x\n", m_vb[0]);
		break;

	case CATSEYE_REG_COLOR1:
		COMBINE_DATA(&m_color[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_COLOR1 = %04x\n", m_color[0]);
		break;

	case CATSEYE_REG_TRRCTL1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_trrctl[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TRRCTL1 = %04x\n", m_trrctl[0]);
		break;

	case CATSEYE_REG_ACNTL1:
		if (!m_write_enable[0])
			break;
		COMBINE_DATA(&m_acntrl);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_ACNTL1 = %04x\n", m_acntrl);
		break;

	case CATSEYE_REG_PLANEMODE1:
		COMBINE_DATA(&m_planemode[0]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_PLANEMODE1 = %04x\n", m_planemode[0]);
		break;

	// TC 2 registers
	case CATSEYE_REG_TCREN2:
		if (mem_mask & 0xff00)
			m_read_enable[1] = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_READ_ENABLE2 = %04x\n", m_read_enable[1]);
		break;

	case CATSEYE_REG_TCWEN2:
		if (mem_mask & 0xff00)
			m_write_enable[1] = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_READ_ENABLE2 = %04x\n", m_write_enable[1]);
		break;

	case CATSEYE_REG_FBEN2:
		if (mem_mask & 0xff00)
			m_fb_enable[1] = data & m_plane_mask_h;
		LOGMASKED(LOG_REG, "W CATSEYE_REG_READ_ENABLE2 = %04x\n", m_fb_enable[1]);
		break;

	case CATSEYE_REG_PRR2:
		if (!m_write_enable[1])
			break;
		COMBINE_DATA(&m_prr[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_PRR2 = %04x\n", m_prr[1]);
		break;

	case CATSEYE_REG_WRR2:
		if (!m_write_enable[1])
			break;
		COMBINE_DATA(&m_wrr[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_WRR2 = %04x\n", m_wrr[1]);
		break;

	case CATSEYE_REG_TRR2:
		if (!m_write_enable[1])
			break;
		COMBINE_DATA(&m_trr[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TRR2 = %04x\n", m_trr[1]);
		break;

	case CATSEYE_REG_VB2:
		if (!m_write_enable[1])
			break;

		COMBINE_DATA(&m_vb[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_VB2 = %04x\n", m_vb[1]);
		break;

	case CATSEYE_REG_COLOR2:
		COMBINE_DATA(&m_color[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_COLOR2 = %04x\n", m_color[1]);
		break;

	case CATSEYE_REG_TRRCTL2:
		if (!m_write_enable[1])
			break;

		COMBINE_DATA(&m_trrctl[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_TRRCTL2 = %04x\n", m_trrctl[1]);
		break;

	case CATSEYE_REG_ACNTL2:
		if (!m_write_enable[1])
			break;

		COMBINE_DATA(&m_acntrl);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_ACNTL2 = %04x\n", m_acntrl);
		break;

	case CATSEYE_REG_PLANEMODE2:
		if (!m_write_enable[1])
			break;

		COMBINE_DATA(&m_planemode[1]);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_PLANEMODE2 = %04x\n", m_planemode[1]);
		break;
	case CATSEYE_REG_RUG_REV:
		COMBINE_DATA(&m_misc);
		LOGMASKED(LOG_REG, "W CATSEYE_REG_MISC = %04x\n", m_misc);
		break;
	default:
		logerror("%s: unknown: %04x = %04x (mask %04x)\n", __func__, offset << 1, data, mem_mask);
		return;
	}

	if (m_fb_enable[0] && trigger)
		trigger_wm<VRAM_VIDEO_PLANE>();

	if (m_fb_enable[1] && trigger)
		trigger_wm<VRAM_OVERLAY_PLANE>();

}

uint16_t catseye_device::ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t ret;

	if (offset >= 0x200 && offset < 0x280) {
		LOGMASKED(LOG_REG, "R PATTERNS1[%03d] = %04x\n", offset - 0x200, m_patterns[0][offset - 0x200]);
		return m_patterns[0][offset - 0x200];
	}

	if (offset >= 0x300 && offset < 0x380) {
		LOGMASKED(LOG_REG, "R PATTERNS2[%03d] = %04x\n", offset - 0x300, m_patterns[1][offset - 0x300]);
		return m_patterns[1][offset - 0x300];
	}

	switch(offset) {
	case TOPCAT_REG_WMOVE_ACTIVE:
		ret = 0;
		break;

	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		ret = m_blink_enable;
		break;

	case TOPCAT_REG_SYNC_ENABLE:
		ret = m_sync_enable;
		break;

	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		ret = m_display_enable;
		break;

	case TOPCAT_REG_VBLANK:
		ret = m_in_vblank;
		LOGMASKED(LOG_REG, "R VBLANK = %04x\n", ret);
		break;

	case TOPCAT_REG_WMOVE_INTRQ:
		ret = m_wm_int_pending;
		LOGMASKED(LOG_REG, "R WMOVE_INTRQ = %04x\n", ret);
		break;

	case TOPCAT_REG_VBLANK_INTRQ:
		ret = m_vblank_int_pending;
		LOGMASKED(LOG_REG, "R VBLANK_INTRQ = %04x\n", ret);
		break;

	case TOPCAT_REG_WMOVE_IE:
		ret = m_wm_int_enable;
		LOGMASKED(LOG_REG, "R WMOVE_IE = %04x\n", ret);
		break;

	case TOPCAT_REG_VBLANK_IE:
		ret = m_vblank_int_enable;
		LOGMASKED(LOG_REG, "R VBLANK_IE = %04x\n", ret);
		break;

	case CATSEYE_REG_RUG_SC:
		ret = m_rugsc;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_RUG_SC = %04x\n", ret);
		break;

	case TOPCAT_REG_WMWIDTH:
	case CATSEYE_REG_WMWIDTH:
		ret = m_wmwidth;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMWIDTH = %04x\n", ret);
		break;

	case TOPCAT_REG_WMHEIGHT:
	case CATSEYE_REG_WMHEIGHT:
		ret = m_wmheight;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMHEIGHT = %04x\n", ret);
		break;

	case CATSEYE_REG_LINEPATH:
		ret = m_linepath;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_LINEPATH = %04x\n", ret);
		break;

	case CATSEYE_REG_LINETYPE:
		ret = m_linetype;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_LINETYPE = %04x\n", ret);
		break;

	case TOPCAT_REG_WMSOURCEX:
	case CATSEYE_REG_WMSOURCEX:
		ret = m_wmsourcex;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMSOURCEX = %04x\n", ret);
		break;

	case TOPCAT_REG_WMSOURCEY:
	case CATSEYE_REG_WMSOURCEY:
		ret = m_wmsourcey;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMSOURCEY = %04x\n", ret);
		break;

	case TOPCAT_REG_WMDESTX:
	case CATSEYE_REG_WMDESTX:
		ret = m_wmdestx;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMDESTX = %04x\n", ret);
		break;

	case TOPCAT_REG_WMDESTY:
	case CATSEYE_REG_WMDESTY:
		ret = m_wmdesty;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMDESTY = %04x\n", ret);
		break;

	case CATSEYE_REG_WMCLIPLEFT:
		ret = m_wmclipleft;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMCLIPLEFT = %04x\n", ret);
		break;

	case CATSEYE_REG_WMCLIPRIGHT:
		ret = m_wmclipright;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMCLIPRIGHT = %04x\n", ret);
		break;

	case CATSEYE_REG_WMCLIPTOP:
		ret = m_wmcliptop;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMCLIPTOP = %04x\n", ret);
		break;

	case CATSEYE_REG_WMCLIPBOTTOM:
		ret = m_wmclipbottom;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WMCLIPBOTTOM = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMWIDTH:
		ret = m_wmwidth;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMWIDTH = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMHEIGHT:
		ret = m_wmheight;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMHEIGHT = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMSOURCEX:
		ret = m_wmsourcex;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMSOURCEX = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMSOURCEY:
		ret = m_wmsourcey;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMSOURCEY = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMDESTX:
		ret = m_wmdestx;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMDESTX = %04x\n", ret);
		break;

	case CATSEYE_REG_TWMDESTY:
		ret = m_wmdesty;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TWMDESTY = %04x\n", ret);
		break;

	case CATSEYE_REG_STATUS:
		ret = CATSEYE_STATUS_READY | CATSEYE_STATUS_NO_DAUGHTERBOARD;
		ret |= m_status & ~CATSEYE_STATUS_VBLANK;
		if (m_in_vblank)
			ret |= CATSEYE_STATUS_VBLANK;

		LOGMASKED(LOG_REG, "R CATSEYE_REG_STATUS = %04x\n", ret);
		break;

	case CATSEYE_REG_STATUS+1:
	case CATSEYE_REG_STATUS+2:
	// unknown
		ret = 0;
		break;

	case TOPCAT_REG_START_WMOVE:
		m_wm_int_pending = 0;
		update_int();
		ret = 0;
		break;

	case CATSEYE_REG_RUG_REV:
		ret = m_misc;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_RUG_REV = %04x\n", ret);
		break;

	// TC1
	case TOPCAT_REG_FB_WRITE_ENABLE:
	case CATSEYE_REG_FBEN1:
		ret = m_fb_enable[0];
		break;

	case TOPCAT_REG_READ_ENABLE_PLANE:
	case CATSEYE_REG_TCREN1:
		ret = m_read_enable[0];
		break;

	case TOPCAT_REG_WRITE_ENABLE_PLANE:
	case CATSEYE_REG_TCWEN1:
		ret = m_write_enable[0];
		break;

	case TOPCAT_REG_PRR:
	case CATSEYE_REG_PRR1:
		if (m_read_enable[0])
			return 0;

		ret = m_prr[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_PRR1 = %04x\n", ret);
		break;

	case TOPCAT_REG_WRR:
	case CATSEYE_REG_WRR1:
		if (m_read_enable[0])
			return 0;

		ret = m_wrr[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WRR1 = %04x\n", ret);
		break;

	case CATSEYE_REG_TRR1:
		if (m_read_enable[0])
			return 0;

		ret = m_trr[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TRR1 = %04x\n", ret);
		break;

	case CATSEYE_REG_COLOR1:
		if (m_read_enable[0])
			return 0;

		ret = m_color[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_COLOR1 = %04x\n", ret);
		break;

	case CATSEYE_REG_VB1:
		if (m_read_enable[0])
			return 0;

		ret = m_vb[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_VB1 = %04x\n", ret);
		break;

	case CATSEYE_REG_TRRCTL1:
		if (m_read_enable[0])
			return 0;

		ret = m_trrctl[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TRRCTL1 = %04x\n", ret);
		break;

	case CATSEYE_REG_ACNTL1:
		if (m_read_enable[0])
			return 0;

		ret = m_acntrl;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_ACNTL1 = %04x\n", ret);
		break;

	case CATSEYE_REG_PLANEMODE1:
		if (m_read_enable[0])
			return 0;

		ret = m_planemode[0];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_PLANEMODE1 = %04x\n", ret);
		break;

	case CATSEYE_REG_BARC_REV1:
			ret = 0x0100;
			break;

	// TC2
	case CATSEYE_REG_FBEN2:
		ret = m_fb_enable[1];
		break;

	case CATSEYE_REG_TCREN2:
		ret = m_read_enable[1];
		break;

	case CATSEYE_REG_TCWEN2:
		ret = m_write_enable[1];
		break;

	case CATSEYE_REG_PRR2:
		if (m_read_enable[1])
			return 0;

		ret = m_prr[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_PRR2 = %04x\n", ret);
		break;

	case CATSEYE_REG_WRR2:
		if (m_read_enable[1])
			return 0;

		ret = m_wrr[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_WRR2 = %04x\n", ret);
		break;

	case CATSEYE_REG_TRR2:
		if (m_read_enable[1])
			return 0;

		ret = m_trr[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TRR2 = %04x\n", ret);
		break;

	case CATSEYE_REG_COLOR2:
		if (m_read_enable[1])
			return 0;

		ret = m_color[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_COLOR2 = %04x\n", ret);
		break;

	case CATSEYE_REG_VB2:
		if (m_read_enable[1])
			return 0;

		ret = m_vb[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_VB2 = %04x\n", ret);
		break;

	case CATSEYE_REG_TRRCTL2:
		if (m_read_enable[1])
			return 0;

		ret = m_trrctl[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_TRRCTL2 = %04x\n", ret);
		break;

	case CATSEYE_REG_ACNTL2:
		if (m_read_enable[1])
			return 0;

		ret = m_acntrl;
		LOGMASKED(LOG_REG, "R CATSEYE_REG_ACNTL2 = %04x\n", ret);
		break;

	case CATSEYE_REG_PLANEMODE2:
		if (m_read_enable[1])
			return 0;

		ret = m_planemode[1];
		LOGMASKED(LOG_REG, "R CATSEYE_REG_PLANEMODE2 = %04x\n", ret);
		break;

	case CATSEYE_REG_BARC_REV2:
		ret = 0x0100;
		break;

	default:
		logerror("%s: unknown: %04x (mask %04x)\n", __func__, offset << 1, mem_mask);
		return space.unmap();
	}
	return ret;
}

WRITE_LINE_MEMBER(catseye_device::vblank_w)
{
	m_in_vblank = state ? m_plane_mask_h : 0;
	if (state && m_vblank_int_enable)
		m_vblank_int_pending = m_plane_mask_h;
}
